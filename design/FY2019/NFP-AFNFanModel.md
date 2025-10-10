﻿Support System Fan Object in Airflow Networks
================

**Lixing Gu**

**Florida Solar Energy Center**

 - Second revision
 - Address comments and add design document
 - First revision
 - Based on comments from Jason DeGraw on 4/8/19
 - Original version
 - 3/11/19



## Justification for New Feature ##

A new fan object model was recently added to EnergyPlus which will become the only fan object type, eliminating and simplifying the various other options, as it is capable of simulating the same physics and controls as all of the other objects. Currently, the fan is available in most parent model objects in EnergyPlus, but it is not available in airflow network simulations. Since this is supposed to be the new primary fan object, it must be also available in airflow network simulations.

## E-mail and  Conference Call Conclusions ##

###GitHub communication between Jason and Gu

Jason

+The name identifying an AirflowNetwork constant volume fan in an air distribution system. This name must be the same as the name of the associated Fan:ConstantVolume,~ Fan:OnOff or Fan:VariableVolume object. This name will be referenced by an AirflowNetwork:Distribution:Linkage object to represent a component.
This sentence should probably be edited to either include Fan:SystemModel or to remove the types all together (since that comes up in the next field).

jasondegraw 25 minutes ago  Member
This sentence should probably be edited to either include Fan:SystemModel or to remove the types all together (since that comes up in the next field).

  @lgu1234
lgu1234 11 minutes ago  Author Member
@jasondegraw I am going to add Fan:SystemModel. Thanks.

Jason:

Two local functions of GetFanInletNodeNum and GetFanOutletNodeNum will be created to output node numbers. The functions will be called from GetAirflowNetworkInput.
  @jasondegraw
jasondegraw 24 minutes ago  Member
Will these two functions be located with the fans in their namespace or will they be in the AFN namespace?

  @lgu1234
lgu1234 6 minutes ago  Author Member
@jasondegraw These two functions will be created in the HVACFan module.

jasondegraw 25 minutes ago  Member
Replace with "NA"?

  @lgu1234
lgu1234 5 minutes ago  Author Member
@jasondegraw Done.

###GitHub communication between Mike, Rich and Gu

mjwitte  Apr 9, 2019    Member This user is a member of the National Renewable Energy Laboratory organization.


@lgu1234 A multi-speed fan also uses Discrete. From the Fan:System model I/O Reference "A constant volume or on-off fan should use Discrete with Number of Speeds set to 1". So this will need to also check Number of Speeds to determine if this is an on-off fan.

But, do you even need to check this? Doesn't OnOffFanPartLoadFraction = 1.0 for a vav fan?

rraustad  Apr 9, 2019    Member This user is a member of the National Renewable Energy Laboratory organization.

VAV fan doesn't use the OnOffFanPartLoadFraction global variable (which is set in the coil objects) because it doesn't cycle. Only OnOff fan and SystemFan use that global. We have also discussed changing the use of this global variable to eliminate the extra iterations needed to pass that global variable to the fan (e.g., blow-thru fan configuration...fan is modeled, coil is modeled and sets the global, then the fan has to be simulated again) by allowing the fan to calculate the PLF instead of waiting for the coil to do that. The best way would be to push the PLF curve index to the fan on the parent GetInput.

lgu1234  Apr 11, 2019    Author You are the author of this pull request. Member You are a member of the National Renewable Energy Laboratory organization.

@mjwitte I will check speed number to determine fan type.
@rraustad I will think to push the PLF curve index to the fan and will try it in my another E+ project (EliminateIteration branch).

## Overview ##

The Fan:SystemModel object models fans of various types using a relatively simple engineering model. This fan can be used in variable air volume, constant volume, on-off cycling, two-speed, or multi-speed applications. It was designed as a replacement for Fan:ConstantVolume, Fan:OnOff, Fan:VariableVolume, and FanPerformance:NightVentilation. In order to make the airflow network model accept the Fan:SystemModel object, an additional choice of fan type input and internal code revision are needed as follows.

- Additional input choice

A new choice key will be added in the Supply Fan Object Type field of the AirflowNetwork:Distribution:Component:Fan object as Fan:SystemModel.

- Code revision

Two modules will be revised to accommodate the new feature: AirflowNetworkBalanceManager and HVACFan.


## Approach ##

The following revisions will be performed.  Any new additions will be highlighted in red.

### AirflowNetworkBalanceManager

GetAirflowNetworkInput

The function will be modified to accept a new choice of fan type as Fan:SystemModel. Then the code will check the input choice of the Speed Control Method field in the Fan:SystemModel object. If the input is Continuous, the AFN model will consider this fan is a VAV fan. If the input is Discrete, the AFN model will consider this fan is on/off fan. the difference between on/off and constant volume fan is based the value of LoopOnOffFanPartLoadRatio.

### HAVCFan

#### Assing fan part load value
The value of OnOffFanPartLoadFraction will be assigned in the update function.

#### Create two local functions to get node numbers of inlet and outlet
Two local functions of GetFanInletNodeNum and GetFanOutletNodeNum will be created to output node numbers in the HVACFan module. The functions will be called from GetAirflowNetworkInput.

## Testing/Validation/Data Sources ##

Test will be performed to ensure the additional fan type will work properly.

## Input Output Reference Documentation ##

Any new additions will be highlighted in red.

\subsection{AirflowNetwork:Distribution:Component:Fan}\label{airflownetworkdistributioncomponentfan}

This component represents a constant volume fan in the air distribution system (AirLoopHVAC). The air flow rate and air conditions (temperature and humidity) are obtained from the associated Fan:ConstantVolume, Fan:OnOff, or Fan:VariableVolume object.

\subsubsection{Inputs}\label{inputs-16-001}

\paragraph{Field: Fan Name}\label{field-fan-name-000}

The name identifying an AirflowNetwork constant volume fan in an air distribution system. This name must be the same as the name of the associated Fan:ConstantVolume,~ Fan:OnOff or Fan:VariableVolume, or Fan:SystemModel object. This name will be referenced by an AirflowNetwork:Distribution:Linkage object to represent a component.

\paragraph{Field: Supply Fan Object Type}\label{field-supply-fan-object-type}

This choice field defines the type of fan. The only valid choices are Fan:OnOff,~ Fan:ConstantVolume, and Fan:VariableVolume, <span style="color:red;">and Fan:SystemModel</style>, <span style="color:black;">with the default being Fan:ConstantVolume. Both cycling and continuous fan operating modes are allowed for Fan:OnOff. Only the continuous fan operating mode is allowed for Fan:ConstantVolume. The variable airflow rate is allowed for Fan:VariableVolume.

Note: Make sure that the volumetric air flow rates for the fan, coils, and parent components (e.g., unitary system or furnace) are the same so that fan energy and air distribution system losses/gains are properly calculated.

An IDF example is provided below:

\begin{lstlisting}

AirflowNetwork:Distribution:Component:Fan,
      Supply Fan 1,            !- Name of Constant Volume Fan
      Fan:ConstantVolume;  !- Supply fan type
\end{lstlisting}

## Input Description ##

A new field will be added in the AirflowNetwork:Distribution:Component:Fan object to allow the AFN model accept fans defined in the Fan:SystemModel object.

	AirflowNetwork:Distribution:Component:Fan,
      \min-fields 2
      \memo This object defines the name of the supply Air Fan used in an Air loop.
 	A1 , \field Fan Name
      \required-field
      \type object-list
      \object-list FansCVandOnOffandVAV
      \reference AirflowNetworkComponentNames
      \note Enter the name of the fan in the primary air loop.
	A2 ; \field Supply Fan Object Type
      \type choice
      \key Fan:OnOff
      \key Fan:ConstantVolume
      \key Fan:VariableVolume
<span style="color:red;">\key Fan:SystemModel

      \default Fan:ConstantVolume

## Outputs Description ##

NA

## Engineering Reference ##

NA

## Example File and Transition Changes ##

An existing example file will be modified. No transition is needed. However, idd change is required.

## References ##

NA

## Design Document ##

The new feature will revise two modules: AirflowNetworkBalanceManager and HVACFan

### AirflowNetworkBalanceManager ###

#### GetAirflowNetworkInput ####
The GetAirflowNetworkInput function will be modified to add a new choice Fan:SystemModel for the Supply Fan Object Type field in the AirflowNetwork:Distribution:Component:Fan object.

The allowed new SupplyFanType will be DataHVACGlobals::FanType_SystemModelObject

Call new functions of FanModel to get inlet and outlet node numbers, and get control method

```
                if (fanType_Num == FanType_SystemModelObject) {
                    inletNode = GetFanInletNode(fan_name, ErrorsFound);
                    outletNode = GetFanOutletNode(fan_name, ErrorsFound);
                    FanControlMethod = GetFanControlMethod(fan_name, ErrorsFound);
                    (Process fan model)...
                }
```

If FanControlMethod == Discrete, then check m_numSpeeds

#### Runtime check in ConstantVolumeFan::calculate ####

If m_numSpeeds > 1, then check speed number to determine if the fan operates in Speed 1 or higher
If Speed Number = 1, it will set fan mode as OnOff
If Speed Number > 1, it will set constant volume fan

If I can get information from other existing variables, this check may not be performed.

### HVACFan ###

Four possible new functions will be created to provide enough information for the AFN model:

		GetFanInletNode
		GetFanOutletNode
		GetFanControlMethod
		GetSpeedNumber

The first 3 new functions will be called from GetAirflowNetworkInput. The last function will be called during runtime from ConstantVolumeFan::calculate to determine fan operation mode.


