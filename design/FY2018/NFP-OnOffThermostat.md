OnOff Thermostat
================

**Lixing Gu**

**Florida Solar Energy Center**

 - First revision
 - 11/7/17

 - Original, 10/20/17



## Justification for New Feature ##

In general, real cycling systems do not cycle several times in a short period. EnergyPlys simulates cycle systems by using part load approach to ensure the system part load capacity to meet the system load in a time step. In order to simulate real system performance, on/off stage in a time step is required. Since it reduces iteration when a system cycles, it is expected to reduce execution time.

## E-mail and  Conference Call Conclusions ##

### E-mail communications

Enclosed are E-mail communications among Tianzhen, Mike, Rich and Gu after submission of the original NFP.

Gu:
I am concerned that the approach of setting the current zone load to a high value will be confusing and not control well, especially with more than one piece of equipment serving a zone.
It would seem better to build on the existing ZoneControl:Thermostat:StagedDualSetpoint (and SetpointManager:SingleZone:OneStageHeating/Cooling) which already establishes a framework for signalling on/off and stage (or speed) to AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed (and maybe from source code this works with AirLoopHVAC:UnitarySystem).
It uses ZoneSysEnergyDemand( ActualZoneNum ).StageNum (+ for heating stage, - for cooling stage, 0 for off) and DataZoneControls::StageZoneLogic to communicate the current control state to the equipment.
This task could use the same logic and extend it to other equipment types.
Mike
p.s.  Please open a pull requests to allow comments on github.

On 11/3/2017 1:54 PM, Tianzhen Hong wrote:
Thanks Gu for the clarifications.
More comments as follows.
Tianzhen

On Fri, Nov 3, 2017 at 6:10 AM, Lixing Gu <gu@fsec.ucf.edu> wrote:
Tianzhen:

Thanks for your quick response. Here are my answers to your questions:

1. For equipment with multi-stage or VFD controls, they can run at different capacity, do you need to overwrite or specify its operation if it is forced to cycle on?

A: The equipment will run full capacity, since the predicted load will be set to very high value such as 1.0E20.

In this case, would force the equipment to run at the lowest stage/capacity be a better approach to avoid overcooling or overheating?

2. Do manufacturers have minimal cycling on/off time for their equipment? Or rules of thumb? Would those be a user input. Currently you tie it to the simulation time step.

A: Yes. The thermostat is tied to each time step. Minimum operation time can be added. If added, it will be user input. However, it may not belong to the thermostat. Instead, it should belong to each individual system, because different systems may have different operation time restriction.

Agree. This is a system level control.

3. If forced to cycle on/off for full time step, the zone temperature may be overcooled or underheated. How to flag this in the report?

A: Yes, the zone may be got overcooled or overheated. I can report a temperature difference. Do you have any suggestions.

Maybe adding a time-series report variable to indicate the status of cycling on/off.

Please let me know any more concerns.

Thanks.

Gu

From: energyplusdevteam@googlegroups.com [mailto:energyplusdevteam@googlegroups.com] On Behalf Of Tianzhen Hong
Sent: Thursday, November 02, 2017 6:02 PM
To: Lixing Gu <gu@fsec.ucf.edu>
Cc: energyplusdevteam@googlegroups.com
Subject: Re: [energyplusdevteam] NFP OnOff Thermostat

Gu,
This is a nice feature especially for residential systems. I had some discussion with our residential group recently on this topic.

Can you clarify the following?

1. For equipment with multi-stage or VFD controls, they can run at different capacity, do you need to overwrite or specify its operation if it is forced to cycle on?

2. Do manufacturers have minimal cycling on/off time for their equipment? Or rules of thumb? Would those be a user input. Currently you tie it to the simulation time step.

3. If forced to cycle on/off for full time step, the zone temperature may be overcooled or underheated. How to flag this in the report?

Thanks,
Tianzhen

On Thu, Nov 2, 2017 at 10:34 AM, Lixing Gu <gu@fsec.ucf.edu> wrote:
Team:

An NFP to allow OnOff Thermostat.

https://github.com/NREL/EnergyPlus/blob/OnOffThermostat/design/FY2018/NFP-OnOffThermostat.md

Comments are welcome via e-mail or github.  Please let me know if you wish to be a reviewer for this task.

Thanks.

Gu


Rich's E-mail

Right now predictor sends a load signal to the cooling and heating SP. This does not include the load to reach the cut-out zone temperature. What if predictor, if an OnOff thermostat were used, sent the load needed to get to the zone cut-out temperature. That way the equipment would still used the same load variables, run 100% each time step until the load is met and then cycle off sometime during the final time step. If more than 1 zone equipment is used, then this "load" would not be correct if the other equipment did not use the On/Off thermostat, but there is only 1 thermostat allowed in each zone so this shouldn't be a problem.

### Gu's responses

1. Will remove the proposed new field as OnOff Thermal Control Flag in the ZoneControl:Thermostat object
2. Will add a new field as Temperature Difference between Cutout and Setpoint in the in the ZoneControl:Thermostat object. The proposed approach will not cause possible overheating or overcooling.
3. If time and budget allows, will add two new optional fields in the AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed object: Minimum HVAC Operation Time and Minimum HVAC Off Time.
If there is no load and HVAC system is required to turn on, the first speed operation is forced.

It should be noted that the addition of Item 3 is not related to the cut-out temperature.

## Overview ##

When a cycling system is called, the corresponding system module calculates its full capacity and compares the system load. When the system load is less than the system full capacity, a par load performance is assumed and part load penalty is applied in a time step, no matter how small a time step is. In addition, iteration is needed to calculate part load ratio, so that the part load capacity meets the system load. In general, a real system would not be on/off several times during several minutes. Instead, the real system turns on in several minutes, and turns off in several minutes. Therefore, EnergyPlus does not simulate real cycling system performance.

## Approach ##

The proposed approach adds a new optional field to allow users to input temperature difference between cut-out and setpoint. The temperature difference will be applied to both heating and cooling. The setpoint temperature will be used to start HVAC system operation, while the cut-out temperature will be used to turn off HVAC system.

When the cut-out temperature is applied, it will avoid possible overheating or overcooling.


### Revise ZoneControl:Thermostat ###

An optional field as the last field will be added to allow systems to operate either cut-out operation with input > 0 or normal thermostat operation with blank or 0.

	  N1 ; \field Temperature Difference between Cutout and Setpoint
       \units deltaC
       \type real

### Revise GetZoneAirSetPoints ###

The function of GetZoneAirSetPoints in the ZoneTempPredictorCorrector will be revised to add a reading section to read an additional field of the ZoneControl:Thermostat and to set up the temperature difference. The temperature difference will be used in both heating and cooling.

### Revise CalcZoneAirTempSetPoints ###

When the temperature difference (dt) is greater than 0, the following calculation procedures will be performed in 4 control types:

#### Heating

1. When the zone air temperature at the previous time step <= setpoint + dt

   Calculate the setpoint based on the heating setpoint + dt and request heating

2. When the zone air temperature > setpoint + dt

   Calculate the setpoint based on the heating setpoint and request heating

#### Cooling

1. When the zone air temperature at the previous time step >= setpoint - dt

   Calculate the setpoint based on the cooling setpoint - dt and request cooling

2. When the zone air temperature < setpoint - dt

   Calculate the setpoint based on the cooling setpoint - dt and request cooling

This procedures will be applied to 3 control types: SingleHeating, SingleCooling, and DualSetpoint. However, the procedure will not be applied to a control type of SinglHeatingAndCooling.


### Add min operation time in the AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed

If time and budget allows, modification of the AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed will be performed. Two new optional fields in the AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed object: Minimum HVAC Operation Time and Minimum HVAC Off Time will be added.

 	 N19, \field Minimum HVAC Operation Time
       \note Minimum operation time when HVAC system is forced on.
       \type real
       \units minutes
       \minimum 0.0
       \default 0.0
  	N20; \field Minimum HVAC Off Time
       \note Minimum HVAC system off time.
       \type real
       \units minutes
       \minimum 0.0
       \default 0.0


If there is no load and HVAC system is required to turn on, the first speed operation is forced.

It should be pointed out that it would be better to require every system type with min operation times. This is a starting point to test system performance with minimum operation time restriction.

## Testing/Validation/Data Sources ##

Multiple tests will be performed to ensure a single system only turn on in a whole time step.

## Input Output Reference Documentation ##

\subsection{ZoneControl:Thermostat}\label{zonecontrolthermostat}

The thermostatic zone control object is used to control a zone to a specified temperature. ZoneControl:Thermostat references a control type schedule and one or more control type objects which in turn reference one or more setpoint schedules. The example at the end of this section illustrates a complete zone thermostat specification including the control type and setpoint schedules.

The control type schedule and the list of control type/name pairs are directly related. The schedule defines the type of control that is to be used throughout the simulation. Valid Control Types are:

0 - Uncontrolled (No specification or default)

1 - Single Heating Setpoint

2 - Single Cooling SetPoint

3 - Single Heating/Cooling Setpoint

4 - Dual Setpoint (Heating and Cooling) with deadband

Thus, if the schedule referenced in the ZoneControl:Thermostat statement has a control type of 4 for a particular time period, this indicates that during that time period ``Dual Setpoint with deadband'' control is to be used. The specific ``Dual Setpoint (Heating and Cooling) with deadband'' control object to be used is specified in the list of control type/name pairs. Then the specific control type objects reference the thermostat setpoint temperature schedule to be used. Because only one control can be specified for each control type in a ZoneControl:Thermostat statement, there are only four pairs possible in a particular ZoneControl:Thermostat type/name list. This is because individual controls can be defined throughout the simulation, thus giving the user a full range of flexibility. Since putting in the name of the control type directly in the schedule would be very cumbersome, the control types are assigned a number which is used in the schedule profile.

\subsubsection{Inputs}\label{inputs-054}

\paragraph{Field: Name}\label{field-name-052}

Unique identifying name for the thermostat.

\paragraph{Field: Zone or ZoneList Name}\label{field-zone-or-zonelist-name-001}

Name of the zone or set of zones that is being controlled. When the ZoneList option is used then this thermostat definition is applied to each of the zones in the zone list effecting a global definition for thermostatic control in the zone.

\paragraph{Field: Control Type Schedule Name}\label{field-control-type-schedule-name}

Schedule which defines what type of control is active during the simulation. Valid Control Types are:

0 - Uncontrolled (No specification or default)

1 - Single Heating Setpoint

2 - Single Cooling SetPoint

3 - Single Heating Cooling Setpoint

4 - Dual Setpoint with Deadband (Heating and Cooling)

Each non-zero control type which is used in this schedule must appear in the following fields which list the specific thermostat control objects to be used for this zone.

\paragraph{Field Set (Control Object Type, Control Name)}\label{field-set-control-object-type-control-name}

Up to four pairs of Control Object Type and Control Name may be listed to specify which control objects are used for this zone. This list is not order-dependent, and the position in this list has no impact on the control type schedule. In the control type schedule, a value of 1 always means ``Single Heating Setpoint'', even if that control type is not first in this list.

\paragraph{Field: Control Object \textless{}x\textgreater{}Type}\label{field-control-object-xtype}

This field specifies the control type to be used for this zone. Available control types are:

ThermostatSetpoint:SingleHeating

ThermostatSetpoint:SingleCooling

ThermostatSetpoint:SingleHeatingOrCooling

ThermostatSetpoint:DualSetpoint

\paragraph{Field: Control \textless{}x\textgreater{} Name}\label{field-control-x-name}

The corresponding control type name. The name is used in an object with the name of the control type and specifies the schedule.


	  N1 ; \field Temperature Difference Between Cutout And Setpoint
       \units deltaC
       \type real
       \minimum= 0

<span style="color:red;">**Field: Temperature Difference Between Cutout And Setpoint}\label{field-temperature-difference-between-cutout-and-Setpoint}**<span>

<span style="color:red;">This optional choice field provides a temperature difference between cut-out temperature and setpoint. The temperature difference is applied to both heating and cooling. When the zone air temperature at the previous time step is below the heating setpoint + temperature difference, the setpoint is the heating setpoint + temperature difference, When the zone air temperature is higher than the heating setpoint + temperature difference, the setpoint is the heating setpoint. When the zone air temperature at the previous time step is above the cooling setpoint - temperature difference, the setpoint is the cooling setpoint - temperature difference, When the zone air temperature is below the cooling setpoint - temperature difference, the setpoint is the cooling setpoint.


An example of this statement in an IDF is:

\begin{lstlisting}

ZoneControl:Thermostat, Zone 3 Thermostat, NORTH ZONE,
  Zone Control Type Sched,
  DUAL SETPOINT WITH DEADBAND, VAV Setpoints;
\end{lstlisting}

## Input Description ##

An optional field will be added as the last field in the ZoneControl:Thermostat object, so that no transition is needed. Any new additions will be highlighted in red.

	ZoneControl:Thermostat,
   	\memo Define the Thermostat settings for a zone or list of zones.
   	\memo If you use a ZoneList in the Zone or ZoneList name field then this definition applies
   	\memo to all the zones in the ZoneList.
  	A1 , \field Name
       \required-field
       \reference ZoneControlThermostaticNames
  	A2 , \field Zone or ZoneList Name
       \required-field
       \type object-list
       \object-list ZoneAndZoneListNames
  	A3 , \field Control Type Schedule Name
       \note This schedule contains appropriate control types for thermostat.
       \note Control types are integers: 0 - Uncontrolled (floating, no thermostat), 1 = ThermostatSetpoint:SingleHeating,
       \note 2 = ThermostatSetpoint:SingleCooling, 3 = ThermostatSetpoint:SingleHeatingOrCooling, 4 = ThermostatSetpoint:DualSetpoint
       \required-field
       \type object-list
       \object-list ScheduleNames
  	A4 , \field Control 1 Object Type
       \required-field
       \type choice
       \key ThermostatSetpoint:SingleHeating
       \key ThermostatSetpoint:SingleCooling
       \key ThermostatSetpoint:SingleHeatingOrCooling
       \key ThermostatSetpoint:DualSetpoint
  	A5 , \field Control 1 Name
       \note Control names are names of individual control objects (e.g. ThermostatSetpoint:SingleHeating)
       \note Schedule values in these objects list actual setpoint temperatures for the control types
       \required-field
       \type object-list
       \object-list ControlTypeNames
  	A6 , \field Control 2 Object Type
       \type choice
       \key ThermostatSetpoint:SingleHeating
       \key ThermostatSetpoint:SingleCooling
       \key ThermostatSetpoint:SingleHeatingOrCooling
       \key ThermostatSetpoint:DualSetpoint
  	A7 , \field Control 2 Name
       \note Control names are names of individual control objects (e.g. ThermostatSetpoint:SingleHeating)
       \note Schedule values in these objects list actual setpoint temperatures for the control types
       \type object-list
       \object-list ControlTypeNames
  	A8 , \field Control 3 Object Type
       \type choice
       \key ThermostatSetpoint:SingleHeating
       \key ThermostatSetpoint:SingleCooling
       \key ThermostatSetpoint:SingleHeatingOrCooling
       \key ThermostatSetpoint:DualSetpoint
  	A9 , \field Control 3 Name
       \note Control names are names of individual control objects (e.g. ThermostatSetpoint:SingleHeating)
       \note Schedule values in these objects list actual setpoint temperatures for the control types
       \type object-list
       \object-list ControlTypeNames
  	A10, \field Control 4 Object Type
       \type choice
       \key ThermostatSetpoint:SingleHeating
       \key ThermostatSetpoint:SingleCooling
       \key ThermostatSetpoint:SingleHeatingOrCooling
       \key ThermostatSetpoint:DualSetpoint
  	A11, \field Control 4 Name
       \note Control names are names of individual control objects (e.g. ThermostatSetpoint:SingleHeating)
       \note Schedule values in these objects list actual setpoint temperatures for the control types
       \type object-list
       \object-list ControlTypeNames

<span style="color:red;">**	N1;  \field Temperature Difference Between Cutout And Setpoint **

       \units deltaC
       \type real
       \minimum= 0


## Outputs Description ##

insert text

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

One of existing example files will be modified to use this option.

No transition is needed, since an optional field is added as the last one.


## References ##

A link to the enhancement item is provided below.

https://github.com/NREL/EnergyPlusDevSupport/blob/master/DesignDocuments/EnhancementList/HVAC_General_2013_05.doc

# Design Document #

The modules modified for this new feature will be DataZoneControls and ZoneTempPredictorCorrector.

## DataZoneControls ##

A new variable will be added in the struct of ZoneTempControls to represent a new field of Temperature difference between cutout and setpoint:

  		Real64 DeltaTCutSet; // Temperature difference between cutout and setpoint


## ZoneTempPredictorCorrector ##

The function of CalcZoneAirTempSetPoints will be modified to accommodate the new feature

### SingleHeatingSetPoint ###

		if ( MAT( ActualZoneNum ) < TempZoneThermostatSetPoint( ActualZoneNum ) + DeltaTCutSet && DeltaTCutSet > 0.0 ) {
			TempZoneThermostatSetPoint( ActualZoneNum ) += DeltaTCutSet;
		}

### SingleCoolingSetPoint ###

		if ( MAT( ActualZoneNum ) > TempZoneThermostatSetPoint( ActualZoneNum ) - DeltaTCutSet && DeltaTCutSet > 0.0 ) {
			TempZoneThermostatSetPoint( ActualZoneNum ) -= DeltaTCutSet;
		}

### SingleHeatCoolSetPoint ###

No change

### DualSetPointWithDeadBand ###

		if ( MAT( ActualZoneNum ) > ZoneThermostatSetPointHi( ActualZoneNum ) - DeltaTCutSet && DeltaTCutSet > 0.0 ) {
			ZoneThermostatSetPointHi( ActualZoneNum ) -= DeltaTCutSet;
		}

		if ( MAT( ActualZoneNum ) < ZoneThermostatSetPointLo( ActualZoneNum ) + DeltaTCutSet && DeltaTCutSet > 0.0 ) {
			ZoneThermostatSetPointLo( ActualZoneNum ) += DeltaTCutSet;
			if ( ZoneThermostatSetPointHi( ActualZoneNum ) < ZoneThermostatSetPointLo( ActualZoneNum ) ) {
				ZoneThermostatSetPointLo( ActualZoneNum ) = ZoneThermostatSetPointHi( ActualZoneNum );
			}
		}

It should be noted that if the recalculated cooling setpoint is lower than the heating setpoint, the recalculated cooling setpoint will be set to the heating setpoint.