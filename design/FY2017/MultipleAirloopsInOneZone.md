Allow Multiple Air Loops to One Thermal Zone
================

**Michael J. Witte, GARD Analytics, Inc.**

 - October 28, 2016 - Initial NFP
 - November 10, 2016 - Revised NFP
 - November 22, 2016 - Final NFP
	 - Simplified proposal for new DesignSpecification:AirTerminal:Sizing object
	 - Add new field to AirLoopHVAC to control return flows
	 - Add new fields for Lights and Refrigeration:Case objects return heat gain
	 - Add new outputs
	 - Add method to allocate return flows to multiple loops
 - January 3, 2017 - Initial Design
 	 - Add Design section at end
 	 - Add airflow windows - need to specify optional return node
 	 - *Change* - add the new field referencing `DesignSpecification:AirTerminal:Sizing` object to  `ZoneHVAC:AirDistributionUnit` and `AirTerminal:SingleDuct:Uncontrolled` (*not* to every terminal unit as previously proposed).
 - January 5, 2017 - Final Design
 - January 6, 2017 - Document additional review comments
 - January 11 and later - As-built comments

*Reviewers - Hong, Griffith, Gu, Buhl, Raustad, Horowitz, Merket, Winkler, Scheier, Lee

## Table of Contents ##

[Justification for New Feature](#justification-for-new-feature)

[E-mail and  Conference Call Conclusions](#e-mail-and-conference-call-conclusions)

[Overview](#overview)

[Approach](#approach)

[Testing/Validation/Data Sources](#testingvalidationdata-sources)

[Input Output Reference Documentation](#input-output-reference-documentation)

[Engineering Reference](#engineering-reference)

[Example File and Transition Changes](#example-file-and-transition-changes)

[Detailed Comments and Responses](#detailed-comments-and-responses)

[Design](#design)

## Justification for New Feature ##

The current limitation of one airloop (any system built using the `AirloopHVAC` object) serving a given zone has long been a barrier to modeling various system configurations. The most common situation is a multi-zone DOAS running in parallel with single or multi-zone space conditioning systems.  EnergyPlus currently offers three workarounds for modeling DOAS systems in combination with other space-conditioning equipment:

1. Model the DOAS as an airloop and the space-conditioning equipment as zone HVAC equipment (`ZoneHVAC:*`).
2. Model the space-conditioning equipment as an airloop and the DOAS as zone HVAC equipment (`ZoneHVAC:OutdoorAirUnit`).
3. Model the DOAS and the space-conditioning equipment as a dual-duct airloop where one deck is the DOAS side and the other deck is the space-conditioning side. (`AirTerminal:DualDuct:VAV:OutdoorAir`).

All of the current workarounds have limitations and none of them reflect the layout of the real systems.

Recently, the NREL Residential Group and a consultant for NRCan (Natural Resources Canada) asked how to model this common residential application:

> The configuration in question is one where zone air is re-circulated through a furnace while a DOAS, generally including a HRV/ERV, handles ventilation.  So the conditioning and ventilation loops operate in parallel.   The furnace coil must be centralized to be able to impose a capacity and calculate part-load factors: so re-heating coils are also not acceptable.

None of the available workarounds will model both systems correctly. In this particular case, one solution would be to build on #3 above with a new terminal unit that has heating and cooling capability on the VAV side.  The other solution is to use #2, but this raises issues of the HRV/ERV efficiency and defrost operation as well as fan power.

Rather than develop more workaround components, it is time to remove the restriction of one airloop per zone.

## E-mail and Conference Call Conclusions ##

1. Add output variables and/or table report to summarize how various air loops operate in a zone. E.g., what loads they meet, how many hours each loop operate.

2. Clarify that control and load sharing issues between multiple loops serving the same zone will be part of Phase II "Allow Multiple/Partial HVAC in One Zone" (also funded for FY17).

3. Minimize the amount of fields in the new DesignSpecification:AirTerminal:Sizing object.  Focus only on changes from the base Sizing:Zone values.

4. Need more details on how return flows will be determined.

5. Make it clear that the air terminal sizing factors are based on sensible heating/cooling loads. *(e-mail RR)*

6. Consider adding a new array indexed by ADU number rather than morphing the existing `TermUnitSizing` and `TermUnitFinalZoneSizing` which are currently indexed by zone. *(Sizing conference call 1/4/2017)*

7. Special sizing cases, such as chilled beam terminal units, may require special handling of the air terminal sizing factors for certain types of terminal units. *(Sizing conference call 1/4/2017)*

## Overview ##

This new feature will remove the limitation of one airloop (`AirLoopHVAC`) system per zone, and will add options for user-control of sizing and return air flow allocation. Currently, EnergyPlus issues a fatal error if both a `ZoneHVAC:AirDistributionUnit` and a `AirTerminal:SingleDuct:Uncontrolled` are connected to a given zone:

```
** Severe  ** In zone "ZONE ONE" there are too many air terminals served by AirLoopHVAC systems.
**   ~~~   ** A single zone cannot have both an AirTerminal:SingleDuct:Uncontrolled and also a second AirTerminal:* object.
**  Fatal  ** Preceding condition causes termination
```

Surprisingly, there is no check for more than one `ZoneHVAC:AirDistributionUnit` in the same zone.

Also, EnergyPlus currently allows only one Return Air Node per zone.

This feature will remove these limitations and allow any number of airloop systems to serve a given zone.

If time allows, this work will also remove the current requirement that a return path must always be described even if it does not exist in the system.  For example, most DOAS systems and direct-fired heaters only supply outdoor air and do not have any return path.

Load-sharing and control issues will be addressed in a related development project, "Allow Multiple/Partial HVAC in One Zone" (also funded for FY17).

## Approach ##

### 1. Remove error checks on number of air terminals in a zone ###

### 2. Allow more than one return air node in a zone ###
Change the `Zone Return Air Node Name` field to allow a NodeList, similar to the way that zone inlet nodes are input.

### 3. Add a new field to AirloopHVAC to specify loop return air flow fraction
To model pressurized systems and to help allocate return flows among multiple airloops, it is proposed that a new field be added to control the loop return flow rate as a fraction of the loop supply flow rate.

### 4. Revise return air flow and air loop flow balance calculations
Currently, the zone return air node is passive and receives whatever flow is left over from total supply less total unbalanced exhaust for that zone.  There is also an option to control the return air flow using the `Zone Return Air Flow Rate Fraction Schedule Name` and `Zone Return Air Flow Rate Basis Node or NodeList Name` fields. Also, each airloop checks the total supply vs unbalanced exhaust on the loop and balances the loop by adjusting return node flows proportionally to match.  The heart of these calculations is ZoneEquipmentManager::CalcZoneMassBalance.

The return air flow will be allocated as follows, similar to the current return air flow calculations with some added or modified steps:

  - Step 1 - Set known return node flows
	  - Set any return nodes that have a flow rate specified in the ZoneHVAC:EquipmentConnections object. (*existing step*)
	  - For an air loop without an outdoor air system, set the return node flow equal to the corresponding supply air inlet multiplied by the system return flow fraction (input value from AirloopHVAC). (*new step*)
  - Step 2 - Calculate remaining unallocated return air flow for each zone (*modified step*)
      - UnallocatedReturn = TotalSupply - UnbalancedExhaust - KnownReturnFlows
  - Step 3 - Set remaining return node flows (*new step*)
      - Set remaining return node flows proportional to the supply flow to that zone from the corresponding airloop.
      - For example, if Zone 1 is supplied with 0.8 m3/s from Air Loop A and 0.2 m3/s from Air Loop B, then allocate 80% of the remaining return flow to the return air node connected to Loop A and 20% to Loop B.
  - Step 4 - Allocate unbalanced exhaust air flows to each air loop (*modified step*)
      - For a given zone, the unbalanced exhaust air flow is distributed to any air loop that has an outdoor air system in proportion to the loop's supply flow to that zone. Air systems without an outdoor air inlet are excluded.
  - Step 5 - Balance each air loop (*existing step*)
      - LoopReturn0 = Sum of Return Node flow rates on the loop after Steps 1-4.
      - LoopReturn = LoopSupply - LoopExhaust
      - For each return node on the loop AdjustedReturnFlow = ReturnFlow*LoopReturn/LoopReturn0

### 5. Allocate return air heat gains to specific return air nodes
This applies to return air heat gain from lights, refrigerated case under-case return,and airflow windows.

### 6. Revise other places that assume a single airloop is associated with a zone
Preliminary code review shows that there are several places that use `ZoneEquipConfig::AirLoopNum`.  More details to follow in the design doc.

### 7. Allow different sizing specifications for different air terminal units
The present sizing inputs apply a single `Sizing:Zone` object to a given zone to calculate the design loads and airflow rates for cooling, heating, and outdoor air.

ZoneHVAC:* equipment (such as fan coils or PTACs) can reference a `DesignSpecification:ZoneHVAC:Sizing` object to customize the sizing for a given ZoneHVAC unit.  A similar approach is proposed for air terminal units.

A new object named `DesignSpecification:AirTerminal:Sizing` will be added to allow specification of any required differences from the base Sizing:Zone specifications.

A new optional field will be added at the end of the `ZoneHVAC:AirDistributionUnit` and `AirTerminal:SingleDuct:Uncontrolled` objects to reference a `DesignSpecification:AirTerminal:Sizing` object as needed.

### 8. Allow an airloop with no return path *(if budget allows)*
This would remove any checks that throw errors when there is no return path.  Beyond that the airloop should function normally with zero flow at the supply side inlet node.  Input changes would include making the following nodes optional with a blank allowed:

```
  AirLoopHVAC,
    ,  !- Demand Side Outlet Node Name

  OutdoorAir:Mixer,
    ;  !- Return Air Stream Node Name

  ZoneHVAC:EquipmentConnections,
    ;  !- Zone Return Air Node Name
```
The `AirLoopHVAC` Supply Side Inlet Node Name would still be used, because the main airloop branch uses that as its first inlet node.  This could be an outdoor air node in which case the entire outdoor air subsystem, mixer and OA controller would not be necessary.


## Testing/Validation/Data Sources ##

Build example files and check results.

## Input Output Reference Documentation ##

### Input Description ###
No transition required. One new object, and some new fields at the end of some existing objects.

### Modified Object: ZoneHVAC:EquipmentConnections ###
* Change the current `Zone Return Air Node Name` to `Zone Return Air Node or NodeList Name`.

* Change the following fields to be an extensible group:

```
Zone Return Air Flow Rate Fraction Schedule Name
Zone Return Air Flow Rate Basis Node or NodeList Name
```

### Modified Object: AirloopHVAC###
*New Field:: Design Return Air Flow Fraction of Supply Air Flow*

This field specifies the design air loop return air flow as a fraction of the supply flow when there is no exhaust flow.  It may be used to set zero return air flow for a DOAS system or to model a pressurized system where the return flow is a fraction of the supply flow. The return air flow rate will never be greater than the current supply air flow rate multiplied by this fraction.  It may be less if there is unbalanced exhaust from any zones served by this airloop. The default is 1.0.

```
  AirLoopHVAC,
    VAV Sys 1,               !- Name
    ,                        !- Controller List Name
    VAV Sys 1 Avail List,    !- Availability Manager List Name
    autosize,                !- Design Supply Air Flow Rate {m3/s}
    VAV Sys 1 Branches,      !- Branch List Name
    ,                        !- Connector List Name
    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name
    PLENUM-1 Out Node,       !- Demand Side Outlet Node Name
    Zone Eq In Node,         !- Demand Side Inlet Node Names
    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names
    0.90;                    !- Design Return Air Flow Fraction of Supply Air Flow
```

### New Object: DesignSpecification:AirTerminal:Sizing###

This object modifies the sizing of a given terminal unit given the base sizing results from the corresponding `Sizing:Zone` inputs. Any given `DesignSpecification:AirTerminal:Sizing` object is generic and may be used by multiple terminal units with similar characteristics.


*Name*

Name of the design specification air terminal sizing object. This name may be referenced by a `ZoneHVAC:AirDistributionUnit` or `AirTerminal:SingleDuct:Uncontrolled` object.

*Fraction of Design Sensible Cooling Load*

The fraction of the design sensible cooling load to be met by this terminal unit. This fraction is applied after the Zone Cooling Sizing Factor (see `Sizing:Zone`).

*Cooling Design Supply Air Temperature Difference Ratio*

This ratio adjusts the supply air temperature difference used to calculate the cooling design supply air flow rate for this terminal unit.

*Fraction of Design Sensible Heating Load*

The fraction of the design sensible heating load to be met by this terminal unit. This fraction is applied after the Zone Heating Sizing Factor (see `Sizing:Zone`).

*Heating Design Supply Air Temperature Difference Ratio*

This ratio adjusts the supply air temperature difference used to calculate the cooling design supply air flow rate for this terminal unit.

*Fraction of Minimum Outdoor Air Flow*

The fraction of the zone minimum outdoor air requirement to be met by this terminal unit.

#### Example calculations for cooling airflow rate
Results from base `Sizing:Zone` calculations:

Design sensible cooling load 1000 [W]

Design cooling supply air temperature difference 10.0 [deltaC]

Design cooling supply air flow rate 0.085 [m3/s]

Minimum outdoor air flow rate 0.1 [m3/s]

Default terminal unit flow rate would be Max(0.085, 0.1) = 0.1 [m3/s]

```
  DesignSpecification:AirTerminal:Sizing,
    DOAS Terminal Sizing,    !- Name
    0.0,                     !- Fraction of Design Cooling Load
    ,                        !- Cooling Design Supply Air Temperature Difference Ratio
    0.0,                     !- Fraction of Design Heating Load
    ,                        !- Heating Design Supply Air Temperature Difference Ratio
    1.0;                     !- Fraction of Minimum Outdoor Air Flow
```
Design cooling supply air flow rate = 0.085\*0.0 = 0.0 [m3/s]

Minimum outdoor air flow rate = 0.1\*1.0 = 0.1 [m3/s]

DOAS terminal unit flow rate = Max(0.0, 0.1) = 0.1 [m3/s]


```
  DesignSpecification:AirTerminal:Sizing,
    Recirculation System A Terminal Sizing, !- Name
    0.6,                     !- Fraction of Design Cooling Load
    0.8,                     !- Cooling Design Supply Air Temperature Difference Ratio
    1.0,                     !- Fraction of Design Heating Load
    1.0,                     !- Heating Design Supply Air Temperature Difference Ratio
    0.0;                     !- Fraction of Minimum Outdoor Air Flow
```

Design cooling supply air flow rate = 0.085\*0.6\*(1/0.8) = 0.06375 [m3/s]

Minimum outdoor air flow rate = 0.1\*0.0 = 0.0 [m3/s]

Recirculation terminal unit flow rate = Max(0.06375, 0.0) = 0.06375 [m3/s]

### Modified Objects: `ZoneHVAC:AirDistributionUnit` and `AirTerminal:SingleDuct:Uncontrolled` ###

In the `ZoneHVAC:AirDistributionUnit` and `AirTerminal:SingleDuct:Uncontrolled` objects, add the following field at the end of the object. (At some point in the future, plan to address [#4988](https://github.com/NREL/EnergyPlus/issues/4988) which would create a new AirTerminal:SingleDuct:ConstantVolume:NoReheat that sits inside an ADU just like all the other terminal units.)

*New field:  Design Specification Air Terminal Sizing Name*

This optional input field is the name of a DesignSpecification:AirTerminal:Sizing object which specifies sizing adjustments to be made for this terminal unit. See DesignSpecification:AirTerminal:Sizing for more details. If left blank, this terminal unit will be sized according to the inputs in the corresponding Sizing:Zone object.

*Additional notes*

This parallels the `Design Specification ZoneHVAC Sizing Object Name` field which is in many of the space conditioning `ZoneHVAC:*` objects:
```
ZoneHVAC:IdealLoadsAirSystem
ZoneHVAC:FourPipeFanCoil
ZoneHVAC:WindowAirConditioner
ZoneHVAC:PackagedTerminalAirConditioner
ZoneHVAC:PackagedTerminalHeatPump
ZoneHVAC:WaterToAirHeatPump
ZoneHVAC:UnitVentilator
ZoneHVAC:UnitHeater
ZoneHVAC:EvaporativeCoolerUnit
ZoneHVAC:TerminalUnit:VariableRefrigerantFlow
ZoneHVAC:VentilatedSlab
```

Objects which have a subset of the DesignSpecification:ZoneHVAC:Sizing fields embedded in them are:
```
ZoneHVAC:Baseboard:RadiantConvective:Water
ZoneHVAC:Baseboard:RadiantConvective:Steam
ZoneHVAC:Baseboard:RadiantConvective:Electric
ZoneHVAC:CoolingPanel:RadiantConvective:Water
ZoneHVAC:LowTemperatureRadiant:VariableFlow
ZoneHVAC:LowTemperatureRadiant:Electric
ZoneHVAC:HighTemperatureRadiant
```

And objects which have no scalable sizing inputs:
```
ZoneHVAC:Dehumidifier:DX
ZoneHVAC:OutdoorAirUnit
ZoneHVAC:EnergyRecoveryVentilator
ZoneHVAC:LowTemperatureRadiant:ConstantFlow

```

### Modified Object: Lights ###

*New Field: Return Air Heat Gain Node Name*

Name of the return air node for this heat gain. If left blank, defaults to the first return air node for this zone.

### Modified Object: Refrigeration:Case ###

*New Field: Under Case HVAC Return Air Node Name*

Name of the return air node for this case. If left blank, defaults to the first return air node for this zone.

### Modified Object: WindowProperty:AirflowControl ###

*New Field: Airflow Return Air Node Name*

Name of the return air node for this airflow window if the Airflow Destination is ReturnAir. If left blank, defaults to the first return air node for the zone of the window surface.

### Outputs Description ##

Existing output variables for "Zone Air Terminal * " and "Air System * " should provide details about each airloop.  For other types of zone equipment, outputs are available by type of equipment, such as "Fan Coil * " and "Zone Radiant HVAC * ". It would be useful to have a general set of output variables that report the contribution of each piece of zone equipment, regardless of type.


## Engineering Reference ##

insert text

## Example File and Transition Changes ##

No transition is anticipated.

New example files will be made to show various combinations of systems.

## Detailed Comments and Responses ##

10/30 - Hong

1. Do we allow multiple outdoor air loops (DOAS systems) to a zone?

	*MJW 1.  I don't see any reason this would not work, but the user would need to specify the OA requirements to avoid over-ventilating the zone.*

2. How do we control the operation of multiple air loops? For example, if we have two air loops to a zone with each sized to meet half of the zone peak loads, at 50% zone load, do we operate only one air loop at 100%, or two with each at 50%? We may need a control scheme for this to make it more flexible.

	*MJW 2.  That's actually a second project that is also funded for the next release "Allow Multiple/Partial HVAC in One Zone" similar, but focused on the control aspect for all equipment serving a zone, including ZoneHVAC equipment.*

3. How to avoid fight of different air loops (e..g, some doing cooling/dehumidifying while others doing heating/humidifying)? We saw this in data centers with multiple RTUs.

	*MJW 3.  That's a good question - at this point I was assuming all of the systems would work off the same thermostat, but that may not be adequate in the long run (this kind of fits into the part two project as well).*

4. Any high level report to summarize how various air loops operate in a zone? E.g., what loads they meet, how many hours each loop operate?

	*MJW 4.  That would be useful - will think about what that might contain.*


10/31 - Scheier - The proposed DesignSpecification:AirTerminal:Sizing object contains many fields that currently exist on some AirTerminal objects, but not others.

1. Should users expect the new fields to be used solely for sizing the AT or will some also have an effect on the HVAC simulation? For example, for VAV AT's, will the "
2.
3.
4.
5.  Spec OA Object Name" field on the new DesignSpecification:AirTerminal:Sizing object have an effect on the HVAC simulation of minimum airflow as is now done for the AT:SingleDuct:VAV:NoReheat and AT:SingleDuct:VAV:Reheat objects?

	*MJW 1. Good point - I didn't think about those overlaps.  The intent of the proposal was that the new sizing object would work the same way that `Sizing:Zone` currently works - just for sizing.  Maybe a better solution is to allow generic `Sizing:Zone` objects that aren't linked to a specific zone?  Or to shorten the proposed new object to be exactly like `Sizing:Zone`.*

2. It would be nice to have a table of the new fields on the new DesignSpecification:AirTerminal:Sizing object vs each AirTerminal object, i.e., which new DsnSpec:AT fields match functionality of existing AirTerminal fields, which new DsnSpec:AT fields do not apply to existing AirTerminals fields, which new DsnSpec:AT fields provide new functionality, etc.

	*MJW 2. This would be good in general for the existing sizing and terminal unit objects.  I'll work on this table and that may help guide this proposal or at least provide some discussion points.  Revised Nov 14, 2016 - this is no longer needed, because the proposed new object is very simple with no overlapping fields.*

10/31 - Gu

1. You have an option: 6. Optional - Allow an airloop with no return path. How do you handle mass balance without return path? Do you plan to use other objects?

	*MJW 1. The zone air balance would work as it currently does, except there wouldn't be a return node for that air loop.  The zone air mass balance should work the same as it currently does.  If there is more supply than exhaust (plus return to another airloop if present) then the excess air is assumed to exfiltrate and doesn't impact the zone air balance.  Of if the ZoneAirMassBalance option is active then that would take over and balance by adjusting infiltration and/or mixing.*

	*For the airloop itself without return, it would need to take in outdoor air to match the supply and complain if the OA controller didn't allow that.*

11/1 - Horowitz

1. Allowing an airloop with no return path has been identified as a high priority for modeling airflow in residential buildings. If the funding allows for this, we would definitely like to see it included in the work.

11/9 - Sizing Group Conference Call

1. Try to minimize the size of the new DesignSpecification:AirTerminal:Sizing object.  Think in terms of only what might be different from the base Sizing:Zone inputs.  Fields such as VentilationRequirement, Do/Don't use OA specifications,

## Design ##
### 1. Remove error checks on number of air terminals in a zone ###
Actually, the only existing check looks for *an air distribution unit and an AirTerminal:SingleDuct:Uncontrolled (direct air) object in the same zone.* This check will be removed.  The code already allows more than one air distribution unit without throwing an error.

#### ZoneEquipmentManager::SimZoneEquipment ####
*1/9/2017-Done*

- Delete all uses of `ZoneHasAirLoopHVACTerminal` and `ZoneHasAirLoopHVACDirectAir` which includes the error check.  That's the only purpose of these variables.

### 2. Allow more than one return air node in a zone ###

#### DataZoneEquipment.hh ####
In struct `EquipConfiguration`
*1/9/2017-NumReturnNodes and ReturnNode array added, old ReturnAirNode still there, set to first return node number*

 - Delete `int ReturnAirNode`
 - Add `int NumReturnNodes`
 - Add `Array1D_int ReturnNode`
 - Add to IDD and input processing for Return Air Node or Nodelist

 - Delete `int AirLoopNum`
 - ~~Add `int NumAirLoops` (this may not always equal `NumReturnNodes` if an airloop has no return path)~~
 - Add `Array1D_int ReturnNodeAirLoopNum`

   - *RR recommends that these align so that AirLoopPointer(6) corresponds with ReturnNode(6) to avoid mistakes. - MJW - Done*

### 3. Add a new field to AirloopHVAC to specify loop return air flow fraction

#### DataAirSystems.hh ####
*1/11/2017 - Done*

In struct `DefinePrimaryAirSystem`

 - Add `Real64 DesignReturnFlowFrac`
 - Add to IDD and input processing

#### DataAirLoop.hh
*1/11/2017 - Done*

 - Add `Real64 DesReturnFrac`
 - Added here because this is what's used primarily in CalcZoneMassBalance below, used slightly different name to avoid confusion.

### 4. Revise return air flow and air loop flow balance calculations
**NOT DONE YET**
*2/19/2017  - At this point, added design return frac, but that's all basically.*

#### ZoneEquipmentManager::CalcZoneMassBalance ####
This function may be refactored into multiple smaller functions reflecting the steps outlined in the above NFP section plus other sections currently in this function:

  - Initialize values to zero
  - Sum zone inlet, exhaust, and mixing flows
  - Step 1 - Set known return node flows
  - Step 2 - Calculate remaining unallocated return air flow for each zone (*modified step*)
  - Step 3 - Set remaining return node flows (*new step*)
  - Step 4 - Allocate unbalanced exhaust air flows to each air loop (*modified step*)
  - Adjust infiltration and mixing flows for zone air mass balance
  - Step 5 - Balance each air loop
  - Iterate until converged or limit reached


### 5. Allocate return air heat gains to specific return air nodes

#### DataHeatBalance.hh
In struct `LightsData` (Lights object)
*2/20/2017 - done*

 - Add `int ReturnNodePtr`
 - Add new field to IDD and input processing in InternalHeatGains::GetInternalHeatGainsInput

#### RefrigeratedCase.hh
In struct `RefrigCaseData` (Refrigeration:Case object)
*2/20/2017 - done*

 - Use existing `int ZoneRANode; // Node number of return node in zone`
 - Add new field to IDD and input processing in RefrigeratedCase::GetRefrigerationInput

#### DataSurfaces.hh
In struct `SurfaceWindowCalc` (WindowProperty:AirflowControl object)
*2/20/2017 - done*

 - Add `int AirflowReturnNodePtr`
 - Add new field to IDD and input processing in SurfaceGeometry::GetWindowGasAirflowControlData


#### DataZoneEquipment.hh ####
In struct `EquipConfiguration`
*2/20/2017 - done*

 - Add `bool ZoneHasAirFlowWindowReturn` to avoid [looping over every zone surface](https://github.com/NREL/EnergyPlus/blob/2592ba992c6cba84395eef038be8d3a049304067/src/EnergyPlus/ZoneEquipmentManager.cc#L4178-L4183) every iteration in `ZoneEquipmentManager::CalcZoneLeavingConditions`
 - ~Possibly add an array of just the airflow window surface numbers to avoid looping over all the surfaces in the zone even when there *is* an airflow window~

#### ZoneEquipmentManager::CalcZoneLeavingConditions ####

 - **NOT DONE** Do some minor reorganization
 - *Done* Add a for loop over all the return nodes in the zone
 - *Done* Add an if on `ZoneHasAirFlowWindow` to skip the for [loop over all the surfaces](https://github.com/NREL/EnergyPlus/blob/2592ba992c6cba84395eef038be8d3a049304067/src/EnergyPlus/ZoneEquipmentManager.cc#L4178-L4183) just to look for an airflow window
 - *Done* Add a new argument `ReturnNodeNum` to `InternalHeatGains::SumAllReturnAirConvectionGains` to allocate gains for a specific return air node
 - *Done* Also add `ReturnAirNodeNum` optional argument to `HeatBalanceInternalHeatGains::SetupZoneInternalGain` - this only applies to Lights, because refrigerated case and airflow windows don't use this system for internal gains?
 - *Done* Add a new field for `ReturnNodeName` to struct `GenericComponentZoneIntGainStruct` in `DataHeatBalance`.
 - **Not done** `InternalHeatGains::SumReturnAirConvectionGainsByTypes` may also need a ReturnNodeNum argument?
 - ~Change `QRetAir` to be an array, sized to the max number of return nodes across all zones - this may work better as an array inside `EquipConfiguration`~
 - Modify as needed to allocate heat gains to specific nodes and track multiple return temperatures

#### Room air models ####

- search for uses of SumAllReturnAirConvectionGains and adjust as needed


### 6. Revise other places that assume a single airloop is associated with a zone
Searching on `.ReturnAirNode` and `.AirLoopNum` shows relevant hits in:

 - AirflowNetworkBalanceManager.cc
 - DualDuct.cc
 - Furnaces.cc
 - HVACFourPipeBeam.cc
 - HVACManager.cc
 - HVACMultiSpeedHeatPump.cc
 - HVACUnitarySystem.cc
 - MixedAir.cc
 - PackagedTerminalHeatPump.cc
 - PurchasedAirManager.cc
 - ReportSizingManager.cc
 - RoomAirModelAirflowNetwork.cc
 - RoomAirModelManager.cc
 - RoomAirModelUserTempPattern.cc
 - SimAirServingZones.cc
 - SimulationManager.cc
 - SingleDuct.cc
 - SystemAvailabilityManager.cc
 - SystemReports.cc
 - ZoneAirLoopEquipmentManager.cc
 - ZoneContaminantPredictorCorrector.cc
 - ZoneEquipmentManager.cc
 - ZonePlenum.cc
 - ZoneTempPredictorCorrector.cc
 - And numerous unit tests

### 7. Allow different sizing specifications for different air terminal units

#### DataSizing::TermUnitFinalZoneSizing ####

- Terminal units get their final sizing data from `TermUnitFinalZoneSizing`
- TermUnitFinalZoneSizing` holds flow rates and other data which have been adjusted for sizing on ventilation load or other Sizing:System inputs such as a hard size for total air loop flow rate.
- `TermUnitFinalZoneSizing` is initialized to `DataSizing::FinalZoneSizing` in  `ZoneEquipmentManager:UpdateZoneSizing`
- `TermUnitFinalZoneSizing` is updated to adjusted `FinalZoneSizing` data in `SimAirServingZones::UpdateSysSizing`
- `TermUnitFinalZoneSizing` is further adjusted in `SimAirServingZones::UpdateSysSizing`

#### DataSizing::TermUnitSizing ####

- `TermUnitSizing` is used as input in `SimAirServingZones::UpdateSysSizing` and in `ReportSizingManager::RequestSizing`.
- `DataSizing::TermUnitSizing` is initialized in the various terminal unit sizing routines.

#### SimAirServingZones::UpdateSysSizing ####
- `UpdateSysSizing` uses `TermUnitSizing`, `ZoneSizing` and `FinalZoneSizing` as inputs.
- `UpdateSysSizing` is basically four functions in one, split up by a big case statement for `CallIndicator` = BeginDay, DuringDay, EndDay, or EndSysSizingCalc (tempted to split that into four separate functions).
- Lots of sizing calcs are done here, and the most pertinent section is that it makes final adjustments to `TermUnitFinalZoneSizing`.

#### Proposed Changes ####
- *Done* Add new struct `DataSizing::AirTerminalSizingData` to hold input for the new `DesignSpecification:AirTerminal:Sizing` object. (Similar to ZoneHVACSizingData)
- *Done* The new array will be called `AirTerminalSizingSpec`.
- *Done* Add new function `SizingManager::GetAirTerminalSizing` to process input for the new ` DesignSpecification:AirTerminal:Sizing` object.
- *Done* in `ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment` add input processing for new Design Specification Air Terminal Sizing Object Name field. And add new field to `DataDefineEquip::AirDistUnit`(ZoneAirEquip struct).
- *Done* in `DirectAirManager::GetDirectAirInput` add input processing for new Design Specification Air Terminal Sizing Object Name field. And add new field to `DirectAirManager::DirectAir` (DirectAirProps struct).
- *Done* Also add a new field `TermUnitSizingIndex` for the pointer (index) to this terminal unit's TermUnitSizing data for both ADU and direct air.  This gets set during input processing.
- *Done* And in the ZoneEquipConfig data structure add `TermUnitSizingIndex` also.  This gets set in each terminal unit's get input routine in the same place that `ZoneEquipConfig( CtrlZone ).AirDistUnitCool( SupAirIn ).InNode` gets set (that's the best I could come up with where all the right info is available).

- *Done* In `DataSizing` add a new variable for `NumAirTerminalUnits` which is the number of air terminal units.  This will be incremented as ZoneHVAC:AirDistributionUnit and AirTerminal:SingleDuct:Uncontrolled objects are created.
- *Done* In `DataSizing` add a new variable for `CurTermUnitSizingNum` which is equivalent to `CurZoneEqNum`. Use only where the TermUnitSizingIndex cannot be determined directly from the component or ADU number.

*Hmmm* `TermUnitSizing` isn't filled for everyone, only some parts are used for certain type of terminal units. Ones that currently don't rely on that don't fill it. But that may be ok.  The important info ends up in `TermUnitFinalZoneSizing`.

- *Done* Change `TermUnitSizing` and `TermUnitFinalZoneSizing` to be indexed separately, not by zone.
  - *If this proves to be too intrusive or risky, consider adding yet another array that is indexed by ADU.*
- *Done* Expand `TermUnitSizing` to include pertinent inputs from a referenced `DesignSpecification:AirTerminal:Sizing` object. These fields will default to 1.0 if there is no `DesignSpecification:AirTerminal:Sizing` object for a given ADU.
- *Done* In `UpdateSysSizing`, change some things that are indexed by CtrlZoneNum (or equivalent) to be indexed by terminal unit sizing index.
- *Done* Add two arrays to `DataAirloop::AirLoopZoneEquipConnectData`
  - `Array1D_int TermUnitCoolSizingIndex`
  - `Array1D_int TermUnitHeatSizingIndex`
  - These get filled in `SimAirServingZones::InitAirLoops`
- *Done* Wherever sizing calculations are made in the ~~terminal unit~~ system sizing routines and ~~in `RequestSizing`~~ and when filling `TermUnitFinalZoneSizing`, apply the input factors from `DesignSpecification:AirTerminal:Sizing` which are stored in `TermUnitSizing`.


### 8. Allow an airloop with no return path *(if budget allows)*

#### SimAirServingZones::GetAirPathData ####
- *Done again 2017-08-06* Modify input processing for `AirLoopHVAC` to allow the Demand Side Outlet Node Name to be blank
- ** No change needed?** Skip the check for proper return connection if the Demand Side Outlet Node Name is blank

#### ReturnAirPathManager::InitReturnAirPath  ####
- This function is currently just a placeholder and is not used.
- Add a test here to make sure that the Return Air Path Outlet Node Name is connected to an AirLoopHVAC Demand Side Outlet Node. This should detect a dangling ReturnAirPath.

#### HVACInterfaceManager::UpdateHVACInterface ####
- Add logic to allow for systems without a return path