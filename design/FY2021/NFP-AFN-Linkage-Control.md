# Improved AirflowNetwork Flow Linkage Controls #

Jason DeGraw, Prateek Shrestha

Oak Ridge National Laboratory

Original Date: March 31, 2021

## Justification for New Feature ##
AirflowNetwork does not currently distinguish between a control factor and a multiplier for flow linkages. One input signal is allowed that is limited to [0,1],
which precludes many use cases, including variable crack lengths, per area leakage elements, and multiple windows represented by a single path. Futhermore, it
is no longer possible to modify the IDD to allow for other uses.

## Overview ##
A multiplier factor enables an independent control of the size of a flow linkage without having to adjust the flow characteristics of the linkage component(s).
Once the user defines such a linkage characteristics, changing merely the multiplier in one location will enable the user to change the number of leakage path
openings connecting different nodes and can make the model set-up task faster and easier for the user.

## Approach ##
Previous work has added the necessary parameters to the function signatures to allow separate control and multiplier inputs.

* Add the necessary input fields 
* Separate the usage of the two values internal to AirflowNetwork 
* Add schedulability and EMS control (budget permitting) 
* Create unit tests and documentation

## Testing/Validation/Data Sources ##
Unit tests will be implemented to ensure numerical values are calculated properly.

## Input Output Reference Documentation
Additions will be made to the Input Output Reference for the multiplier associated with the flow linkage.

## Input Description ##
The current distribution linkage object will be modified to include the multiplier, control, and control schedule fields:

```
AirflowNetwork:Distribution:Linkage,
      \min-fields 4
      \memo This object defines the connection between two nodes and a component.
 A1 , \field Name
      \required-field
      \type alpha
      \note Enter a unique name for this object.
 A2 , \field Node 1 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node.
 A3 , \field Node 2 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node.
 A4 , \field Component Name
      \required-field
      \type object-list
      \object-list AirflowNetworkComponentNames
      \object-list FansCVandOnOffandVAV
      \object-list AFNCoilNames
      \object-list AFNHeatExchangerNames
      \object-list AFNTerminalUnitNames
      \note Enter the name of an AirflowNetwork component. A component is one of the
      \note following AirflowNetwork:Distribution:Component objects: Leak, LeakageRatio,
      \note Duct, ConstantVolumeFan, Coil, TerminalUnit, ConstantPressureDrop, or HeatExchanger.
 A5 , \field Thermal Zone Name
      \type object-list
      \object-list ZoneNames
      \note Only used if component = AirflowNetwork:Distribution:Component:Duct
      \note The zone name is where AirflowNetwork:Distribution:Component:Duct is exposed.
N1 ,  \field Multiplier
      \type real
      \note 
N2 ,  \field Control Value
      \type real
      \note 
A6 ;  \field Control Schedule Name
      \type object-list
      \object-list ScheduleNames
```

A new object will be added for multizone linkages:

```
AirflowNetwork:MultiZone:Linkage,
      \min-fields 4
      \memo This object defines the connection between two nodes and a component.
 A1 , \field Name
      \required-field
      \type alpha
      \note Enter a unique name for this object.
 A2 , \field Node 1 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node.
 A3 , \field Node 2 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node.
 A2 , \field Surface Name
      \required-field
      \type object-list
      \object-list SurfAndSubSurfNames
      \note Enter the name of a heat transfer surface.
 A3 , \field Leakage Component Name
      \required-field
      \type object-list
      \object-list SurfaceAirflowLeakageNames
      \note Enter the name of an Airflow Network leakage component. A leakage component is
      \note one of the following AirflowNetwork:Multizone objects:
      \note AirflowNetwork:MultiZone:Component:DetailedOpening,
      \note AirflowNetwork:MultiZone:Component:SimpleOpening,
      \note AirflowNetwork:MultiZone:Surface:Crack,
      \note AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea,
      \note AirflowNetwork:MultiZone:Component:HorizontalOpening, or
      \note AirflowNetwork:MultiZone:Component:ZoneExhaustFan.
      \note When the zone exhaust fan name is entered, any surface control fields below A3 are
      \note ignored when the zone exhaust fan turns on.
N1 ,  \field Multiplier
      \type real
      \note 
N2 ,  \field Control Value
      \type real
      \note 
A4 ;  \field Control Schedule Name
      \type object-list
      \object-list ScheduleNames
```

The planned modifications do not include the `AirflowNetwork:MultiZone:Surface` object. It will continue to work as-is. The intrazonal links object may also need to be
modified, this will be discussed with the development team.

## Outputs Description ##
No new output variables will be implemented with this work as existing variables are sufficient to gather relevant airflow information.

## Engineering Reference ##
N/A

## Example File and Transition Changes ##
Suitable example file will be generated. Since the multiplier will be incorporated within existing framework of flow linages, no transition rules are anticipated.

## References ##
N/A
