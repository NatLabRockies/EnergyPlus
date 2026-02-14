Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: Fuel type synonyms
For all Boiler:\*, WaterHeater:\* , Generator:\* ,
OtherEquipment, Exterior:FuelEquipment, HVACTemplate:System:VRF, HVACTemplate:Plant:Boiler, ZoneHVAC:HybridUnitaryHVAC
Coil:Cooling:DX:MultiSpeed, Coil:Heating:Fuel, Coil:Heating:DX:MultiSpeed, AirConditioner:VariableRefrigerantFlow, Chiller:EngineDriven, Chiller:CombustionTurbine,
ChillerHeater:Absorption:DirectFired, EnergyManagementSystem:MeteredOutputVariable, LifeCycleCost:UsePriceEscalation, LifeCycleCost:UseAdjustment,
Meter:Custom, Meter:CustomDecrement, FuelFactors and other objects with a fuel type or resource type field

Field: Fuel Type (or similar)

a) change "Gas" or "Natural Gas" to "NaturalGas"

b) change "Electric" or "Elec" to "Electricity"

c) change "FuelOil#1" or "Fuel Oil #1" or "Fuel Oil" or "Distillate Oil" to "FuelOil1"

d) change "FuelOil#2" or "Fuel Oil #2" or "Residual Oil" to "FuelOil2"

e) change "PropaneGas" or "LPG" or "Propane Gas" to "Propane"

# Object Change: GlobalGeometryRules
Field 1, Starting Vertex Position:

a) change "ULC" to "UpperLeftCorner"

b) change "LLC" to "LowerLeftCorner"

c) change "LRC" to "LowerRightCorner"

d) change "URC" to "UpperRightCorner"

Field 2, Vertex Entry Direction

a) change "CCW" to "Counterclockwise"

b) change "CW" to "Clockwise"

Field 3, Coordinate System
Field 4, Daylighting Reference Point Coordinate System
and Field 5, Rectangular Surface Coordinate System

a) change "WCS" or "WorldCoordinateSystem" or "Absolute" to "World"

b) change "Rel\*" or "Relative\*" or "Local" to "Relative"

# Object Change: ShadowCalculations
Insert field 1, Shading Calculation Method, if "External Shading Calculation Method" was specified use:

 - "Scheduled" for "ScheduledShading"
 - "Imported" for "ImportedShading"
 - "PolygonClipping" for "InternalCalculation" or if not specified

Field 2, Shading Calculation Update Frequency Method, renamed from previous field 1 "Calculation Method" and uses:

- "Periodic" for "AverageOverDaysInFrequency"
- "Timestep" for "TimestepFrequency"

Field 3, Shading Calculation Update Frequency, renamed from previous field 2 "Calculation Frequency".

Fields 4 and 5, previous fields 3 and 4. No other changes.

Insert field 6, Pixel Counting Resolution.

Field 7, previous field 5. No other changes.

Remove previous field 6 "External Shading Calculation Method".

Remaining fields 8 onward are the same as previous fields 7 onwards. No other changes.

# Object change: Output:Table:SummaryReports
Any field F1:FN

a) change "ABUPS" or "BEPS" to "AnnualBuildingUtilityPerformanceSummary"

b) change "IVRS" to "InputVerificationandResultsSummary"

c) change "CSS" to "ComponentSizingSummary"

d) change "SHAD" to "SurfaceShadowingSummary"

e) change "EIO" to "InitializationSummary"

# Object Change: `HeatPump:WaterToWater:EIR:Heating`
Object has been renamed to ```HeatPump:PlantLoop:EIR:Heating```

Fields 1-3 remain the same.
After field 3, insert 'Condenser Type'. The field has two key options of 'WaterSource' and 'AirSource', and defaults to 'WaterSource'. Shift all later fields down by 1. The old input fields F4-F15 become the new F5-F16.

# Object Change: `HeatPump:WaterToWater:EIR:Cooling`
Object has been renamed to ```HeatPump:PlantLoop:EIR:Cooling```

Fields 1-3 remain the same.
After field 3, insert 'Condenser Type'. The field has two key options of 'WaterSource' and 'AirSource', and defaults to 'WaterSource'. Shift all later fields down by 1. The old input fields F4-F15 become the new F5-F16.

# Object Remove and Replace: `AirTerminal:SingleDuct:Uncontrolled` to `AirTerminal:SingleDuct:ConstantVolume:NoReheat`

## (1) Replace Object: replace the "AirTerminal:SingleDuct:Uncontrolled" with "AirTerminal:SingleDuct:ConstantVolume:NoReheat"

Field F1(A1): Name, no change

Field F2(A2): Availability Schedule Name, no change.

Insert Field F3(A3): Air Inlet Node Name, create a new inlet node name using the original Field F3(A3) value

          Field F4(A4 = Field F3(A3) + "ATInlet" -> example, "SPACE3-1 Supply Inlet ATInlet",

          save this new inlet node name for use as the "AirLoopHVAC:ZoneSplitter" or "AirLoopHVAC:SupplyPlenum" outlet node name

Field F4(A4): Air Outlet Node Name, use the old F3(A3).

Field F5(N1) Old Field F4: Maximum Air Flow Rate {m3/s}, no change.

Field F6(A5) Old Field F5: Design Specification Outdoor Air Object Name, no change.

Field F7(A6) Old Field F6: Per Person Ventilation Rate Mode, no change.

Save old Field F7 to use in new ZoneHVAC:AirDistribution object: Design Specification Air Terminal Sizing Object Name


## (2) Object Change: ZoneHVAC:EquipmentList

Field F3(A3): Zone Equipment 1 Object Type, replace "AirTerminal:SingleDuct:Uncontrolled" with "ZoneHVAC:AirDistributionUnit"

Field F4(A4): Zone Equipment 1 Name, create a unique new name. The new name will be created based on the name of "AirTerminal:SingleDuct:Uncontrolled" object". "AirTerminal:SingleDuct:Uncontrolled" object Name + "ADU" -> "SPACE3-1 Direct Air ADU"

save the new name of the "ZoneHVAC:AirDistributionUnit" object

save the "AirTerminal:SingleDuct:Uncontrolled" object Name for later use ( will be used as name for new air terminal no reheat object)

Field F5(N1) - F6(N2), no change.


## (3) Add Object: ZoneHVAC:AirDistributionUnit

Field F1(A1): Name, insert the unique name created for "ZoneHVAC:AirDistributionUnit" in STEP 1 above, example "SPACE3-1 Direct Air ADU"

Field F2(A2): Air Distribution Unit Outlet Node Name, use Zone Supply Air Node Name input field (A3) value of the "AirTerminal:SingleDuct:Uncontrolled" object

Field F3(A3): Air Terminal Object Type, insert "AirTerminal:SingleDuct:ConstantVolume:NoReheat"

Field F4(A4): Air Terminal Object Type, insert the Name of the "AirTerminal:SingleDuct:Uncontrolled" object, example "SPACE3-1 Direct Air"

Field >=F5(N1): Leave blank. Will use default.

Field >=F6(N2): Leave blank. Will use default.

Field >=F5(N1): Leave blank. Will use default.


## (4) Modify Object: AirLoopHVAC:ZoneSplitter

If a zone splitter references an  "AirTerminal:SingleDuct:Uncontrolled" node name (or a NodeList containing a match) . If it's a NodeList, then create a duplicate NodeList (if necessary) and change
the matching node name in the NodeList. If  duplicate NodeList is made, then change the node name in the splitter to match.

Field F1(A1): Name, no change.

Field F2(A2): Inlet Node Name, no change

    modify one of the "Outlet X Node Name" of the "AirLoopHVAC:ZoneSplitter" if it matches the Zone Supply Air Node Name input field value of current "AirTerminal:SingleDuct:Uncontrolled" object

    use the new Air Inlet Node Name created in Step 3 above for "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object

Field >=F3: Outlet X Node Name, modify the matching Outlet X Node Name, should be the same node name as the new Air Inlet Node Name of the "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object, old name + ATInlet -> example, "SPACE3-1 Supply Inlet ATInlet"

## (5) Modify Object: AirLoopHVAC:SupplyPlenum

If a supply plenum references an  "AirTerminal:SingleDuct:Uncontrolled" node name (or a NodeList containing a match). If it's a NodeList, then create a duplicate NodeList (if necessary) and change
the matching node name in the NodeList. If a duplicate NodeList is made, then change the node name in the splitter to match.

Field F1(A1): Name, no change.

Field F2(A2): Zone Name, no change

Field F3(A3): Zone Node Name, no change

Field F4(A4): Inlet Node Name, no change

    modify one of the "Outlet X Node Name" of the "AirLoopHVAC:SupplyPlenum" that matches the Zone Supply Air Node Name input field value of current "AirTerminal:SingleDuct:Uncontrolled" object

    use the new Air Inlet Node Name created in Step 3 above for "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object.

Field >=FX(AX): Outlet X Node Name, modify the matching Outlet X Node Name, should be the same node name as the new Air Inlet Node Name of "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object, old name + ATInlet -> example, "SPACE3-1 Supply Inlet ATInlet"

## (6) Modify Object: AirLoopHVAC

If F9 "Demand Side Inlet Node Names" references an  "AirTerminal:SingleDuct:Uncontrolled" node name (or a NodeList containing a match). If it's a NodeList, then create a duplicate NodeList (if necessary) and change
the matching node name in the NodeList. If a duplicate NodeList is made, then change the node name in the splitter to match.


Field F1(A1): Name, no change.

Field F2(A2): Zone Name, no change

Field F3(A3): Zone Node Name, no change

Field F4(A4): Inlet Node Name, no change

    modify one of the "Outlet X Node Name" of the "AirLoopHVAC:SupplyPlenum" that matches the Zone Supply Air Node Name input field value of current "AirTerminal:SingleDuct:Uncontrolled" object

    use the new Air Inlet Node Name created in Step 3 above for "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object.

Field >=FX(AX): Outlet X Node Name, modify the matching Outlet X Node Name, should be the same node name as the new Air Inlet Node Name of "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object, old name + ATInlet -> example, "SPACE3-1 Supply Inlet ATInlet"

## (7) Change any EMS references
Replace "AirTerminal:SingleDuct:Uncontrolled" with	"AirTerminal:SingleDuct:ConstantVolume:NoReheat"
and replace "AirTerminal:SingleDuct:Uncontrolled Maximum Mass Flow Rate" with 	"AirTerminal:SingleDuct:ConstantVolume:NoReheat Maximum Mass Flow Rate"

## (8) AirflowNetwork:Distribution:Node
If there is an "AirflowNetwork:Distribution:Node" object which references an  "AirTerminal:SingleDuct:Uncontrolled" node name, create a duplicate object
with ATInlet appended to the object name and the node name.

## (9) AirflowNetwork:Distribution:Linkage
If there is an "AirflowNetwork:Distribution:Linkage" object which references an "AirflowNetwork:Distribution:Node" that was duplicated in step 8 then modify the linkage
object and create a new one as shown in this exmple.

### Before
    AirflowNetwork:Distribution:Linkage,
      Zone3SupplyLink,         !- Name
      Zone3SupplyNode,         !- Node 1 Name
      Zone3SupplyRegisterNode, !- Node 2 Name
      Zone3Supply,             !- Component Name
      Attic Zone;              !- Thermal Zone Name

### After
    AirflowNetwork:Distribution:Linkage,
      Zone3SupplyLink ATInlet, !- Name
      Zone3SupplyRegisterNode ATInlet,  !- Node 1 Name
      Zone3SupplyRegisterNode, !- Node 2 Name
      Zone3Supply,             !- Component Name
      Attic Zone;              !- Thermal Zone Name

    AirflowNetwork:Distribution:Linkage,
      Zone3SupplyLink,         !- Name
      Zone3SupplyNode,         !- Node 1 Name
      Zone3SupplyRegisterNode ATInlet,  !- Node 2 Name
      Zone3Supply,             !- Component Name
      Attic Zone;              !- Thermal Zone Name

# EMS Function Change: Change EMS Function `@CpAirFnWTdb` to `@CpAirFnW`
For all `EnergyManagementSystem:Program` and `EnergyManagementSystem:Subroutine`, the function signature for the specific heat of air has changed from requiring both humidity ratio and drybulb temperature, to *only* requiring humidity ratio.

### Before
    SET cp_air = @CpAirFnWTdb HumRatAir TdbAir, !- Program Line

### After
    SET cp_air = @CpAirFnW HumRatAir, !- Program Line
