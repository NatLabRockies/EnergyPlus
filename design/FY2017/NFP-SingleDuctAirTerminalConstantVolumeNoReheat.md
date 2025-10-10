Task: 4.1.1 Single Duct Constant Volume Air Terminal Without Reheat

Single Duct Constant Volume Air Terminal NoReheat New Object
================

**Bereket Nigusse, Florida Solar Energy Center**

 - November 20, 2017 (submitted initial NFP)
 - December 4, 2017 (submitted Original Final NFP)
 - December 7, 2017 (submitted Revised Final NFP)

## Justification for New Feature ##

The existing AirTerminal:SingleDuct:Uncontrolled object is different from all other terminal units in EnergyPlus and has the following limitations:

- It only has a single node and it is implemented directly on the zone equipment list instead of being referenced by a ZoneHVAC:AirDistributionUnit object. This causes problem to interface developers.
- Prevents this terminal unit from using the simple duct-leakage model in ZoneHVAC:AirDistributionUnit.

The new air terminal object will solve these problems.


## E-mail and  Conference Call Conclusions ##

Conference call: December 6, 2017

@mjwitte commented on this pull request.

This looks good in general. I've noted some editorial changes here and there. Also, AirTerminal:SingleDuct:Uncontrolled has an EMS actuator for mass flow rate, so this terminal unit will need that also.

Mike Witte provided detailed review comments on github pull request.

Larry: can we keep the *Uncontrolled object for longer period (e.g. up to 2 years) before removing it.
Mike W. Adding "Deprecated" to the IDD causes warning error messaged but we can add that at the last minutes to avoid huge warning error messages.

Brent: suggested that a new object should use oo design approach. He said consider the HVACFourPipeBeam as an example.

Bereket: OO design will be considered in the design of the implementation.

Agreed to keep the *Uncontrolled object for a couple of years before fully removing it. Needs a transition plan.
Mike also offered to help in the transition effort.


## Overview ##

The scope of work for this new feature is:

- add new EnergyPlus object AirTerminal:SingleDuct:ConstantVolume:NoReheat
- will have inlet and outlet air nodes.
- allow the new object to be referenced by ZoneHVAC:AirDistributionUnit
- will have capabilities of AirTerminal:SingleDuct:Uncontrolled object.

The expectation is that the AirTerminal:SingleDuct:ConstantVolume:NoReheat will replace AirTerminal:SingleDuct:Uncontrolled object.

## Approach ##

- creates the new object AirTerminal:SingleDuct:ConstantVolume:NoReheat and adds to the ADU equipment list
- the new object will have inlet and outlet air node (see below the new IDD and idf objects)
- adds new get input section in GetSysInput() function under SingleDuct.cc module
- adds new sim function for air terminal Single Duct CV No Reheat object
- simulated by calling from "SimulateSingleDuct" function
- allow all capabilities of AirTerminal:SingleDuct:Uncontrolled in this new object
- update reference documentations as needed
- the AirTerminal:SingleDuct:Uncontrolled object will be marked as deprecated

## Testing/Validation/Data Sources ##

Compare outputs with the AirTerminal:SingleDuct:Uncontrolled object. Transitioned Example files are not expected to show diffs.

## Input Output Reference Documentation ##

Add a section for this new object in the Input Output Reference under Group - Air Distribution Equipment

The *AirTerminal:SingleDuct:ConstantVolume:NoReheat* object creates the capability of supplying central system air directly to a zone without any zone level thermostat control. The supply air temperature is controlled by the central system controller. It is typically used with a unitary system which controls the system supply temperature and flow rate with continuous or cycling fan. When used without the Design Specification Outdoor Air Object Name, the terminal unit is passive and accepts any flow rate supplied by the central system, but will never exceed the maximum air flow rate. This object allows the program to know what zone this branch of the air system is attached to, and input fields for availability schedule, air inlet and outlet nodes, the maximum air flow rate, and other two optional input fields. The air inlet node should be the same as one of the *AirLoopHVAC:ZoneSplitter* or *AirLoopHVAC:SupplyPlenum* component  outlet nodes. The air outlet node name should be same as zone air inlet node name and the air distribution unit air outlet node name. The last two optional input fields: *Design Specification Outdoor Air Object Name*, and *Per Person Ventilation Rate Mode* are used for modulating the outdoor air requirement of an air terminal unit depending on the method.

![](ATSDCVNoReheat.png)

                         Figure 1 Air terminal single duct constant volume without reheat

## IDD - Input Data Dictionary ##
This new IDD object has all input fields of the *AirTerminal:SingleDuct:Uncontrolled* IDD object except that the *Design Specification Air Terminal Sizing Object Name* input field. The value for the later input field can be obtained from the parent object, ZoneHVAC:AirDistributionUnit. Also unlike the *Uncontrolled* object the new object has an inlet and an outlet air nodes input fields. Proposed idd object is shown below.

```

AirTerminal:SingleDuct:ConstantVolume:NoReheat,

       \memo Central air system terminal unit, single duct, constant volume, without reheat coil

  A1 , \field Name

       \required-field
       \reference AirTerminalUnitNames

  A2 , \field Availability Schedule Name

       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames

  A3,  \field Air Outlet Node Name

       \required-field
       \type node
       \note This is an air outlet node from the air distribution unit. This node name should be one of the
       \note supply air inlet node names of a zone served by this component.

  A4,  \field Air Inlet Node Name

       \required-field
       \type node
       \note The air inlet node name that connects the air splitter to the individual zone air distribution
       \note unit. This node should also be one of the outlet air node of an AirLoopHVAC:ZoneSplitter or
       \note AirLoopHVAC:SupplyPlenum component.

  N1,  \field Maximum Air Flow Rate

       \required-field
       \units m3/s
       \minimum 0.0
       \autosizable
       \note The design maximum volume flow rate.

   A5, \field Design Specification Outdoor Air Object Name

       \type object-list
       \object-list DesignSpecificationOutdoorAirNames
       \note This field is used to modulate the terminal unit flow rate based on the specified outdoor air
       \note requirement. When the name of a DesignSpecification:OutdoorAir object is entered, the terminal unit will
       \note adjust flow to meet this outdoor air requirement and no more. Load is still "Uncontrolled."
       \note If Outdoor Air Flow per Person is non-zero, then the outdoor air requirement will be computed based
       \note on either the current or design occupancy as specified in the Per Person Ventilation Rate Mode field.
       \note At no time will the supply air flow rate exceed the value for Maximum Air Flow Rate. The requested fl
       \note rate may not be fully met if the system is operating with cycling fan. If this field is blank, then
       \note the terminal unit will not be controlled for outdoor air flow. This field is optional.

   A6; \field Per Person Ventilation Rate Mode

       \type choice
       \key CurrentOccupancy
       \key DesignOccupancy
       \default CurrentOccupancy
       \note CurrentOccupancy uses current number of people in the zone which may vary
       \note DesignOccupancy uses the total Number of People in the zone and is constant

```

## Input Description ##

### Inputs

#### Field: Name

A unique user assigned name for this component.

#### Field: Availability Schedule Name

Availability schedule name for this system. Schedule value > 0 means the system is available or else the system is off. If this field is blank, the system is always available.

#### Field: Air Outlet Node Name

This is an air outlet node from the air distribution unit. This node name should be one of the supply air inlet node names of the zone served by this component.

#### Field: Air Inlet Node Name

The air inlet node name that connects the air splitter to the individual zone air distribution unit. This node should also be one of the outlet air node of an AirLoopHVAC:ZoneSplitter or AirLoopHVAC:SupplyPlenum component.

#### Field: Maximum Air Flow Rate

The design maximum volume flow rate~ (m\(^{3}\)/sec). This field is autosizable.

#### Field: Design Specification Outdoor Air Object Name

This field is used to modulate the terminal unit flow rate based on the specified outdoor air requirement. When the name of a DesignSpecification:OutdoorAir object is entered, the terminal unit will adjust flow to meet this outdoor air requirement and no more. Load is still "Uncontrolled." If Outdoor Air Flow per Person is non-zero, then the outdoor air requirement will be computed based on either the current or design occupancy as specified in the *Per Person Ventilation Rate Mode* input field below. At no time will the supply air flow rate exceed the value for Maximum Air Flow Rate. The requested flow rate may not be fully met if the system is operating with cycling fan. The volume flow rate is converted to mass flow rate using the standard density of air at Pressure = 101325 Pa, Temperature = 20C, and Humidity Ratio = 0.0. If this field is blank, then the terminal unit will not be controlled for outdoor air flow. This field is optional.

#### Field: Per Person Ventilation Rate Mode

This field specifies the occupancy level to use when calculating the ventilation rate per person when a Design Specification Outdoor Air Object Name has been specified. CurrentOccupancy uses the current number of people in the zone which may vary. DesignOccupancy uses the total Number of People specified for the zone which is constant.


## New idf object based on the proposed IDD object.

#### AirTerminal:SingleDuct:ConstantVolume:NoReheat New Object

```
AirTerminal:SingleDuct:ConstantVolume:NoReheat,

      NoReheat Zone 1,                !- Name of System
      AlwaysOnFanAvailSched,          !- Availability Schedule Name
      Zone 1 Unit Air Outlet Node,    !- Air Outlet Node Name
      Zone 1 Unit Air Inlet Node,     !- Air Inlet Node Name
      0.60,                           !- Maximum Air Flow Rate {m3/s}
      ,                               !- Design Specification Outdoor Air Object Name
      ;                               !- Per Person Ventilation Rate Mode


```

## Outputs Description ##

  HVAC,Sum,Zone Air Terminal Sensible Heating Energy {[}J{]}

  HVAC,Sum,Zone Air Terminal Sensible Cooling Energy {[}J{]}

  HVAC,Average,Zone Air Terminal Sensible Heating Rate {[}W{]}

  HVAC,Average,Zone Air Terminal Sensible Cooling Rate {[}W{]}

  HVAC,Average,Zone Air Terminal Outdoor Air Volume Flow Rate {[}m3/s{]}

Zone Air Terminal Sensible Heating Energy

Zone Air Terminal Sensible Heating Rate

These outputs are the sensible heating energy and heating rate provided to the zone by the air terminal single duct constant volume no reheat object.

Zone Air Terminal Sensible Cooling Energy

Zone Air Terminal Sensible Cooling Rate

These outputs are the sensible cooling energy and cooling rate provided to the zone by the air terminal single duct constant volume no reheat object.

Zone Air Terminal Outdoor Air Volume Flow Rate

This output is the amount of outdoor air entering the zone. This is the average value over the frequency
being reported. The air terminal outdoor air volume flow rate is calculated as the terminal unit air volume flow rate multiplied by the fraction of outdoor air entering the air loop’s outside air system.

#### New EMS internal variable:
An internal variable called “AirTerminal:SingleDuct:ConstantVolume:NoReheat Maximum Mass Flow Rate” provides
information about the design flow rate for single duct constant volume no reheat air terminals.

#### New EMS Actuator:
“Mass Flow Rate” control in the “AirTerminal:SingleDuct:ConstantVolume:NoReheat” EMS actuator

## Engineering Reference ##

The input object AirTerminal:SingleDuct:ConstantVolume:NoReheat is used to pass conditioned or treated central supply air directly into a zone without any reheat. This terminal air equipment allows to supply central system air directly to a zone without any zone level control or tempering. The supply air temperature is controlled by the central system to meet the load in a controlled zone. This object is configured for use with a constant volume central air system or Furnace and variable supply air temperature. This unit allows the program to know what zone this branch of the air system is attached to, air inlet and outlet nodes, an input field for the maximum air flow rate, and other two optional input fields. The air inlet node should be the same as one of the AirLoopHVAC:ZoneSplitter or AirLoopHVAC:SupplyPlenum component outlet nodes. The air outlet node name should be same as zone air inlet node name and the air distribution unit air outlet node name. The last two input fields: *Design Specification Outdoor Air Object Name*, and *Per Person Ventilation Rate Mode* are used to compute the outdoor air requirement of an air terminal unit and the air terminal mass flow rate is set to the value calculated using these two input fields.


## Example File and Transition Changes ##

New example file will be added if needed. Transition rule spans over 4 or five objects and may involve search and replace node names in at least two objects. Draft transition rule has been added.

## References ##

N/A


## Design - Single Duct Constant Volume Air Terminal No Reheat


**Bereket Nigusse, Florida Solar Energy Center**

 - Original Date: December 11, 2017 (Design)


### New Feature Proposal ##
Refer to the new feature proposal file *NFP-SingleDuctAirTerminalConstantVolumeNoReheatNFP,MD* for details on the justification, overview, and output description.

## Overview
All air terminal single duct related code is located in the ***SingleDuct.cc*** and ***SingleDuct.hh*** files. The air terminal single duct constant volume No reheat new object will be added to these files. The design document does not include any OO type implementation.

## Approach
### Changes to the **SingleDuct.cc** file
- adds new object AirTerminal:SingleDuct:ConstantVolume:NoReheat
- adds this new object to the Air Distribution Unit equipment list
- adds new get input section in **GetSysInput()** function under SingleDuct.cc module
- adds new function **SimConstVolNoReheat()** for air terminal Single Duct CV No Reheat object

	the new function **SimConstVolNoReheat()** will be a member of derived struct SysDesignParams

- adds a calling point for **SimConstVolNoReheat()** new function under "SimulateSingleDuct" function

	will be executed as a member of derived variable Sys. e.g. Sys().SimConstVolNoReheat()

- updates the sizing calculation in **SizeSys()** function to allow for the new object
- updates the flow rate calculation in **InitSys()** function depending on the air flow rate calculation method

### Changes to the **SingleDuct.hh** file


The data structures **SysDesignParams()** will not change. But a new variable will be added.

 - adds int **OAPerPersonMode** object variable for how per person rates are determined

Adds module parameter for the new object added

 - adds extern int const **SingleDuctVAVNoReheat** new module parameter


### Other modules header file change include:
 - AirTerminalUnit.hh

	Adds  **singleDuctConstVolNoReheat** parameter to the enum list for AirTerminalUnitType{}

 - DataDefineEquip.hh

	Adds  **singleDuctConstVolNoReheat** parameter to the Equipment Types in ZoneAirLoopEquipment

 - DataHVACGlobals.hh

	Adds  **ZoneEquipTypeOf_AirTerminalSingleDuctConstantVolumeNoReheat** parameter to the ZoneHVACTerminalTypes list
	extern int const ZoneEquipTypeOf_AirTerminalSingleDuctConstantVolumeNoReheat;

### Other modules cc files change include:
 - DataDefineEquip.cc

	- Adds **SingleDuctConstVolNoReheat()**  parameter

 - DataHVACGlobals.cc

	- Adds **AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT** and **"AirTerminal:SingleDuct:ConstantVolume:NoReheat"** new object names in ZoneHVACTerminalTypes

 - SystemReports.cc

	- Adds **AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT** parameter to the enum list for ComponentTypes{}
	- Adds 	{ "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT", IRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT }
	- Adds 	case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT:
		under CalcSystemEnergyUse() function

 - ZoneAirLoopEquipmentManager.cc

	- Adds AirDistUnit( AirDistUnitNum ).EquipType_Num( AirDistCompUnitNum ) = SingleDuctConstVolNoReheat; case statement in get input function GetZoneAirLoopEquipment().

	- Adds calling point for SimulateSingleDuct for SingleDuctConstVolNoReheat case in SimZoneAirLoopEquipment() function

