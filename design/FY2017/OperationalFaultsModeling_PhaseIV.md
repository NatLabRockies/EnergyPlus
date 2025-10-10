Modeling HVAC Operational Faults – Phase IV
================

 **Rongpeng Zhang, Tianzhen Hong**
 **Lawrence Berkeley National Laboratory**

 - Original Date: Nov 7, 2016
 - Updated Date: Dec 7, 2016


## Justification for New Feature ##

Most of the buildings, either new or old, have operational faults in the sensors, controllers, meters, equipment and systems. Being able to model and simulate these faults and their impact on energy performance of buildings is crucial to improve accuracy of building simulations and to support the retrofit of buildings.

To date, the main practitioner use of EnergyPlus has been for new construction design. With the new high priority attached by DOE to retrofit and improved operation of existing buildings, there is a need to extend the capabilities of EnergyPlus to model existing buildings, including faulty operation:

-	Retrofit analysis: starts with calibrated simulation; the ability to estimate the severity of common faults is expected to improve the accuracy and transparency of the calibrated model and hence increase the accuracy of analyzing different retrofit measures.
-	Commissioning providers can use the fault models to demonstrate the savings to be expected from fixing faults found in retro-commissioning
-	Support for building operation by using the calibrated model, including unfixed faults, as a real-time reference model to detect, and verify the diagnosis of, newly occurring faults.

The users in these cases will be practitioners, not the building modeling experts, so it is planned to implement the fault models using conventional EnergyPlus objects rather than the EMS, which, in any case, could only be used to implement a subset of the faults models.   Additional use cases are noted in Enhancement List document 'General_2009_08.doc'.

A literature review of operational faults in buildings was carried out and a number of common HVAC equipment faults were identified. These faults were then ranked for both complexity of implementation and the severity of the associated energy penalty. Based on the ranking, the following types of occurring faults have been implemented in EnergyPlus in Phase I to III:

1.	Economizer-related Faults
	a.	damper leakage
	b.	temperature sensor offset
	c.	enthalpy sensor offset
	d.	humidity sensor offset
	e.	pressure sensor offset
2.	Thermostat/Humidistat Offset
3.	Coil Fouling
	a.	heating coils
	b.	cooling coils
4.	Air Filter Fouling
5.	Cooling Tower Scaling (under review)
6.	Chiller Supply Water Temperature Sensor Offset (under review)
7.	Condenser Supply Water Temperature Sensor Offset (under review)
8.	Coil Supply Air Temperature Sensor Offset  (under review)

Phase IV in FY17 will focus more on the faults in the plant loop, particularly on the fouling of water-based cooling and heating equipment. Fouling fault occurs when deposits get clogged, usually caused by poor water quality and treatment. It can considerably reduce the overall heat transfer coefficient and thus cause deficiency of equipment efficiency and waste of energy.

We propose to add the following fouling fault models to EnergyPlus:

1.	Fouling of hot-water boilers,
2.	Fouling of water-cooled chillers,
3.	Fouling of evaporative coolers.

This is a continuous effort from current and previous FYs to build a list of common and impactful fault models to EnergyPlus to improve modeling of existing buildings for retrofit or operational improvements to save energy.


## Comments and Reply ##
N/A

## Overview ##

### Fault: Fouling of Hot-water Boilers ###

#### Symptom ####

The fouling of the hot water boilers changes the nominal capacity of the boiler. This further impacts the boiler operations by changing the part load ratio and the related operation/performance parameters.

#### Model inputs ####

Boiler name, reference reduction factor of nominal capacity, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)

#### Approach ####

The fault applies to the hot water boiler model described by the object Boiler:HotWater. It does not apply to the steam boilers which do not have water-based heat exchangers.

Note that operational faults only affect the HVAC operations, not the system design. Therefore, the fault model will only be applied at real weather simulations instead of the sizing and warm-up simulations.

### Fault: Fouling of Water-cooled Chillers ###

#### Symptom ####

The fouling of the water-cooled condenser at chillers changes the nominal capacity of the chiller. This further impacts the chiller operations by changing the part load ratio and the related operation/performance parameters.

#### Model inputs ####

Chiller name, reference reduction factor of nominal capacity, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)

#### Approach ####

The fault applies to a number of chiller types that can have water-cooled condensers, namely:
-	Chiller:Electric,
-	Chiller:Electric:EIR
-	Chiller:Electric:ReformulatedEIR,
-	Chiller:ConstantCOP,
-	Chiller:EngineDriven,
-	Chiller:CombustionTurbine.

The fault does not apply to the absorption chillers that do not have water-based heat exchangers.

### Fault: Fouling of Evaporative Coolers ###

#### Symptom ####

The fouling of the indirect wet-coil evaporative coolers changes the design effectiveness of the tube. This further impacts the cooler operations by changing the part load ratio and the related operation/performance parameters.

#### Model inputs ####

Evaporative cooler name, reference reduction factor of nominal efficiency, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)

#### Approach ####

The fault applies to the wetted coil evaporative cooler described by object EvaporativeCooler:Indirect:WetCoil, where the cooling water is sprayed directly on the tubes. The fault does not apply to direct evaporative coolers or the dry coil indirect evaporative coolers where there is no water-cooled coil.


## IDD Object (New) ##
New objects will be created for the proposed fault types, namely:
-	FaultModel:Fouling:Boiler
-	FaultModel:Fouling:Chiller
-	FaultModel:Fouling:EvaporativeCooler

```
FaultModel:Fouling:Boiler,
   \memo This object describes the fouling fault of the boilers with water-based heat exchangers
   \min-fields 6
   A1, \field Name
       \note Enter the name of the fault
       \required-field
       \type alpha
   A2, \field Availability Schedule Name
       \type object-list
       \object-list ScheduleNames
   A3, \field Severity Schedule Name
       \type object-list
       \object-list ScheduleNames
   A4, \field Boiler Object Type
       \note Enter the type of a boiler object
       \note The fault applies to the hot-water boilers
       \required-field
       \type choice
       \key Boiler:HotWater
   A5, \field Boiler Object Name
       \note Enter the name of a Boiler object
       \required-field
       \type object-list
       \object-list Chillers
   N1; \field Reference Fouling Factor
       \note The factor indicates the decrease of the nominal capacity
       \note It is the ratio between the nominal capacity at fouling case and that at fault free case
       \type real
       \minimum> 0
       \maximum<= 1
       \default 1
       \units dimensionless

FaultModel:Fouling:Chiller,
   \memo This object describes the fouling fault of chillers with water-cooled condensers
   \min-fields 6
   A1, \field Name
       \note Enter the name of the fault
       \required-field
       \type alpha
   A2, \field Availability Schedule Name
       \type object-list
       \object-list ScheduleNames
   A3, \field Severity Schedule Name
       \type object-list
       \object-list ScheduleNames
   A4, \field Chiller Object Type
       \note Enter the type of a chiller object
       \note The fault applies to the chillers with water-cooled condensers
       \required-field
       \type choice
       \key Chiller:Electric
       \key Chiller:Electric:EIR
       \key Chiller:Electric:ReformulatedEIR
       \key Chiller:ConstantCOP
       \key Chiller:EngineDriven
       \key Chiller:CombustionTurbine
   A5, \field Chiller Object Name
       \note Enter the name of a chiller object
       \required-field
       \type object-list
       \object-list Chillers
   N1; \field Reference Fouling Factor
       \note The factor indicates the decrease of the nominal capacity
       \note It is the ratio between the nominal capacity at fouling case and that at fault free case
       \type real
       \minimum> 0
       \maximum<= 1
       \default 1
       \units dimensionless

FaultModel:Fouling:EvaporativeCooler,
   \memo This object describes the fouling fault of the wetted coil evaporative cooler
   \min-fields 6
   A1, \field Name
       \note Enter the name of the fault
       \required-field
       \type alpha
   A2, \field Availability Schedule Name
       \type object-list
       \object-list ScheduleNames
   A3, \field Severity Schedule Name
       \type object-list
       \object-list ScheduleNames
   A4, \field Evaporative Cooler Object Type
       \note Enter the type of a Evaporative Cooler object
       \note The fault applies to the wetted coil evaporative cooler
       \note The fault does not apply to direct evaporative coolers or the dry coil indirect evaporative coolers
       \required-field
       \type choice
       \key EvaporativeCooler:Indirect:WetCoil
   A5, \field Evaporative Cooler Object Name
       \note Enter the name of aN Evaporative Cooler object
       \required-field
       \type object-list
       \object-list Chillers
   N1; \field Reference Fouling Factor
       \note The factor indicates the decrease of the indirect stage efficiency
       \type real
       \minimum> 0
       \maximum<= 1
       \default 1
       \units dimensionless
```

## Model Implementation ##

The proposed model implementation work will try to introduce more object oriented features to the fault modeling routines. Three new classes will be added in the EnergyPlus::FaultsManager namespace, namely:
-	(1) struct FaultPropertiesBoilerFouling : public FaultProperties, corresponding to the IDD object FaultModel:Fouling:Boiler
-	(2) struct FaultPropertiesChillerFouling : public FaultProperties, corresponding to the IDD object FaultModel:Fouling:Chiller
-	(3) struct FaultPropertiesEvaporativeCoolerFouling : public FaultProperties, corresponding to the IDD object FaultModel:Fouling:EvaporativeCooler
These classes will be inherited from Class "FaultProperties" which is the base class for all the operational fault models.
The new added functions will be defined as member functions of the corresponding fault class.

The new added fault classes will contain the information about the faults as well as the links to the affected objects. Take class "FaultPropertiesChillerFouling" for example, it uses member variables "ChillerType" and "ChillerName" to specify the chiller that is affected by the fouling. Three new member variables will be added to the existing chiller class (e.g., "ReformulatedEIRChillerSpecs" in EnergyPlus::ChillerReformulatedEIR):
-	1) bool FaultyChillerFoulingFlag; // True if the chiller has fouling fault
-	2) int FaultyChillerFoulingIndex;  // Index of the fouling fault object corresponding to the chiller
-	3) Real64 FaultyChillerFoulingFactor; // Fouling factor describing the decrease of nominal capacity
These three variables will be initialized in the method EnergyPlus::FaultsManager::CheckAndReadFaults() which processes all the fault objects. Then these variables will be used in the chiller calculations to activate the fault calculations when the fault presents.

The model implementations will try to minimize the modification of the codes outside of the fault routine. Take the FaultModel:Fouling:Chiller for example, all the massive calculations about the chiller fouling will be put in "FaultsManager.cc" instead of the chiller routines. This will be achieved by defining a member function within class "FaultPropertiesChillerSWT". The chiller routines will call this function whenever the fault presents. Such implementation design will minimize the modifications of the chiller routines, and thus benefit their systematic organization and potential future maintenance.

## Testing/Validation/Data Source(s) ##
Comparing simulation results with and without faults will be performed verify the new fault models.

## IDD Object(s) (Revised) ##
None

## Proposed Report Variables ##
None

## Proposed additions to Meters ##
None

## IO Ref (draft) ##
To be developed.

## EngRef (draft) ##
To be developed.

## Example File  ##
To be developed.

## Transition changes ##
N/A

## Other documents ##
N/A

## E-mail and  Conference Call Conclusions ##
N/A

## Reference ##
N/A

