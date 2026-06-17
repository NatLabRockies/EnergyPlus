Modeling HVAC Operational Faults – Phase III
================

 **Rongpeng Zhang, Tianzhen Hong.**
 **Lawrence Berkeley National Laboratory**

 - Original Date: May 25, 2016
 - Updated Date: Jun 20, 2016


## Justification for New Feature ##

Most of the buildings, either new or old, have operational faults in the sensors, controllers, meters, equipment and systems. Being able to model and simulate these faults and their impact on energy performance of buildings is crucial to improve accuracy of building simulations and to support the retrofit of buildings.

To date, the main practitioner use of EnergyPlus has been for new construction design. With the new high priority attached by DOE to retrofit and improved operation of existing buildings, there is a need to extend the capabilities of EnergyPlus to model existing buildings, including faulty operation:

-	Retrofit analysis: starts with calibrated simulation; the ability to estimate the severity of common faults is expected to improve the accuracy and transparency of the calibrated model and hence increase the accuracy of analyzing different retrofit measures.
-	Commissioning providers can use the fault models to demonstrate the savings to be expected from fixing faults found in retro-commissioning
-	Support for building operation by using the calibrated model, including unfixed faults, as a real-time reference model to detect, and verify the diagnosis of, newly occurring faults.

The users in these cases will be practitioners, not the building modeling experts, so it is planned to implement the fault models using conventional EnergyPlus objects rather than the EMS, which, in any case, could only be used to implement a subset of the faults models.   Additional use cases are noted in Enhancement List document 'General_2009_08.doc'.

A literature review of operational faults in buildings was carried out and a number of common HVAC equipment faults were identified. These faults were then ranked for both complexity of implementation and the severity of the associated energy penalty. Based on the ranking, four types of occurring faults have been implemented in EnergyPlus in Phase I and Phase II, including:

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

Compared with Phase I and II that focused on the faults related with air loop, Phase III will include the faults in the plant loop. Four new types of fault will be implemented in Phase III, namely
1.	Chiller Supply Water Temperature Sensor Offset
2.	Condenser Supply Water Temperature Sensor Offset
3.	Cooling Tower Scaling
4.	Coil Supply Air Temperature Sensor Offset


## Comments and Reply ##

**Comment:**

Lawrence Scheier, Jun 6

The approach you are proposing seems sound and, just as important, easy to input but I have two basic concerns:

1) Effect on Sizing:
You state: "the fault model will only be applied at real weather simulations instead of the sizing and warm-up simulations". Would the faults be applied during the "Do HVAC Sizing Simulation for Sizing Periods" simulation? I am not sure how this would effect Brent's coincident sizing calcs. It seems like there would be a potential for the coincident plant size to be larger than the sum-of-the-peaks plant size since the faults would not be applied during the normal plant sizing. This might be a good thing or a bad thing. Not sure.

2) Effect on plant/airside convergence:
it seems like these operational faults could very easily cause the plant/airside controllers to totally lose control at some point, causing both plant temperatures, airside and space temperatures to oscillate wildly (and to unrealistic extremes). This happens often enough under normal circumstances (or bad user input) without adding operational faults to the mix. I can foresee a lot of situations where only the first few day's results are valid and the rest of the year becomes nonsensical because the mathematics go haywire. This is why I am not enamored with operational faults, in general. I think our models too sensitive in many cases to be purposefully throwing a wrench into the works. But perhaps testing will show me to be wrong on this count.

**Reply:**

1) Effect on Sizing:
The proposed models aim to simulate and evaluate the faults occurred at the "operation" phase. We assume that these faults don't exist during the "design" phase, and therefore, the fault model should have no effect on any sizing related simulations, including the traditional sizing calculations and the coincident sizing calculations.

2) Effect on plant/airside convergence:
For most cases, the chiller faults should only affect the plant loop performance and have no impact on the air loop performance or zone temperatures. Although the chiller supply temperature and chiller PLR may differ from the fault-free cases, the plant controls (e.g., bypass control) should be able to ensure the correct heat transfer to the air side. We think the faults will lead to a situation that is similar to that in an over-sized or under-sized system. Since the over-sized and under-sized situations can be well handled by the current plant/airside controllers, the faulty-sensor cases should also be well handled and hopefully don't influence the plant/airside convergence. We will keep an eye on the convergence during the test. In addition, we will try to avoid any unrealistic extremes, e.g., set a limit for the sensor offset.

**Comment:**

Lixing Gu, Jun 8

1) I checked the equation (10.1) in Eng. Ref (Thermostat/Humidistat Offset). It only uses "-", instead of "±". What is real calculation in the proposed code?
2) What is a range of values [for Factor describing the fouling severity in Fault: Cooling Tower Scaling]? Can it be zero?

**Reply:**
1) We have corrected these equations to be consistent with the existing fault models. Now the offset can be either positive or negative. Positive offsets mean that the readings is higher than the actual air conditions.
2) We will set a range in the IDD. It should be greater than 0 and less than or equal to 1.


## Overview ##

### Fault: Chiller Supply Water Temperature Sensor Offset ###
#### Symptom ####
The chiller supply water temperature readings deviate from the actual water temperature levels due to sensor offset at the evaporator outlet. This can lead to incorrect chiller supply water temperature, and thus the inappropriate and inefficient chiller operations.
#### Model inputs ####
Chiller name, reference offset value, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)
#### Approach ####
The fault applies to a number of chiller types, namely:
-	Chiller:Electric,
-	Chiller:Electric:EIR,
-	Chiller:Electric:ReformulatedEIR
-	Chiller:ConstantCOP,
-	Chiller:EngineDriven,
-	Chiller:CombustionTurbine,
-	Chiller:Absorption,
-	Chiller:Absorption:Indirect.

These chillers can have different flow modes:
-	"ConstantFlow" for constant pumping with flow controlled by chiller to operate at full design flow rate.
-	"LeavingSetpointModulated" for variable pumping with flow controlled by chiller to vary flow to target a leaving temperature setpoint.
-	"NotModulated" for either variable or constant pumping with flow controlled by the external plant system.

**(a)**
For the chillers with “ConstantFlow” and “NotModulated”, local control is provided by resetting the leaving water temperature. The actual evaporator outlet water temperature value at faulty operations can be obtained via:

<div>$$ T_{evap-o,f} = T_{evap-o,ff} - \Delta T $$</div>

where
<span>$ T_{evap-o,f} $</span> 	evaporator outlet temperature in the faulty case (actual value)
<span>$ T_{evap-o,ff} $</span> 	evaporator outlet temperature in the fault-free case (reading value)
<span>$ \Delta T $</span> 	difference between the temperature reading and the actual temperature

Then the evaporator capacity can be calculated with:

<div>$$ Q_{evap,f} = m_{evap} \times C_p \times (T_{evap-i} - T_{evap-o,f} ) $$</div>

where
<span>$ m_{evap}$</span> 	evaporator water flow rate (design value, actual value)
<span>$ Q_{evap,f} $</span> 	actual evaporator capacity in the faulty case
<span>$ T_{evap-o,f} $</span> 	evaporator outlet temperature value in the faulty case (actual value)
<span>$ T_{evap-i} $</span> 	evaporator inlet temperature value

**(b)**
For the variable flow chillers with internal water flow rate controls to target a leaving temperature setpoint (type "LeavingSetpointModulated"), the actual evaporator outlet water temperature value at faulty operations can be obtained via:

<div>$$ T_{evap-o,f} = T_{evap-o,ff} - \Delta T $$</div>

where
<span>$ T_{evap-o,f} $</span> 	evaporator outlet temperature in the faulty case (actual value)
<span>$ T_{evap-o,ff} $</span> 	evaporator outlet temperature in the fault-free case (reading, design value)
<span>$ \Delta T $</span> 	difference between the temperature reading and the actual temperature

The water flow rate at faulty operations can be obtained via:

<div>$$ m_{evap,f} = Q_{evap,ff} / ( C_p \times (T_{evap-i} - T_{evap-o,ff} ) )$$</div>

where
<span>$ m_{evap,f} $</span> 	evaporator water flow rate in the faulty case (actual value)
<span>$ Q_{evap,ff} $</span> 	evaporator capacity in the fault-free case (required value)
<span>$ Q_{evap,f} $</span> 	actual evaporator capacity in the faulty case (actual value)
<span>$ T_{evap-o,ff} $</span> 	evaporator outlet temperature in the fault-free case (reading, design value)
<span>$ T_{evap-i} $</span> 	evaporator inlet temperature value
<span>$ \Delta T $</span> 	difference between the temperature reading and the actual temperature

Then the evaporator capacity can be calculated with:

<div>$$ Q_{evap,f} = m_{evap,f} \times C_p \times (T_{evap-i} - T_{evap-o,f} ) $$</div>
where
<span>$ m_{evap,f}	 $</span> evaporator water flow rate in the faulty case (actual value)
<span>$ Q_{evap,f}	 $</span> actual evaporator capacity in the faulty case (actual value)
<span>$ T_{evap-o,f} $</span> 	evaporator outlet temperature value in the faulty case (actual value)
<span>$ T_{evap-i} $</span> 	evaporator inlet temperature value

Note that:
(1) Operational faults only affect the HVAC operations, not the system design. Therefore, the fault model will only be applied at real weather simulations instead of the sizing and warm-up simulations.
(2) If the faulty sensor leads to a supply water temperature level that goes beyond the limits defined in the chiller object, the predefined bound values will be used as the actual supply water temperature $T_{evap-o,f}$.
(3) It is possible that the faulty sensor leads to a heat transfer rate that goes beyond the maximum chiller evaporator capacity. In this case, the maximum capacity will be used to reversely calculate the actual supply water temperature.
(4) For both the constant and variable flow chillers, actual evaporator outlet temperature value $T_{evap-o,f}$ is used for the chiller efficiency calculations.


### Fault: Condenser Supply Water Temperature Sensor Offset ###
#### Symptom ####
The condenser supply water temperature readings deviate from the actual water temperature levels due to sensor offset at the condenser inlet. Because this is usually used as the condenser loop temperature setpoint, the fault may affect the actual performance of cooling tower and condenser. It can results in inappropriate tower operations such as fan and pump cycling and water bypass.
#### Model inputs ####
Tower name, reference offset value, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)
#### Approach ####
The fault applies to a number of tower types, namely:
-	CoolingTower:SingleSpeed
-	CoolingTower:TwoSpeed
-	CoolingTower:VariableSpeed
-	CoolingTower:VariableSpeed:MERKEL

 The effect of an offset in a condenser supply water temperature sensor whose sole use is for calculation of the difference between the set-points and the actual values can be modeled as an equal and opposite offset:

<div>$$ T_{tower-o,f} = T_{tower-o,ff} - \Delta T $$</div>
where
<span>$ T_{tower-o,f} $</span> 	tower outlet temperature in the faulty case (actual value)
<span>$ T_{tower-o,ff}	 $</span> tower outlet temperature in the fault-free case (reading value)
<span>$ \Delta T	 $</span> difference between the temperature reading and the actual temperature

Note that the fault affects the tower in both the free convection cooling mode when fan is off and normal cooling mode when fan is on.

Also note that if the faulty sensor temperature goes beyond the sensor bounds (e.g., min/max condenser loop temperature defined in object “CondenserLoop”, or the min/max setpoint values defined in object “SetpointManager:FollowOutdoorAirTemperature”), the predefined bound values will be used as the actual temperature.

### Fault: Cooling Tower Scaling ###
#### Symptom ####
The fault of scaling widely exists in the cooling tower operations. It occurs when deposits get clogged, usually caused by poor water quality and treatment. It is reported that the removal of scale deposits is one of the biggest expenses in the cooling tower maintenance. Scale deposits can reduce the overall heat transfer coefficient (UA), affecting both the tower effectiveness and energy efficiency.
#### Model inputs ####
Name of the tower with fault, name of availability schedule of the fault, name of the fan severity schedule describing the variations of overall heat transfer coefficient due to tower fouling
#### Approach ####
The fault applies to a number of tower types, namely:
-	CoolingTower:SingleSpeed
-	CoolingTower:TwoSpeed
-	CoolingTower:VariableSpeed
-	CoolingTower:VariableSpeed:MERKEL.

The fault model allows the user to describe the fouling information in either of the two methods: FouledUARated or FoulingFactor. Using FouledUARated method, user specifies the value of UAfouled directly. Using FoulingFactor method user specifies air/water side fouling factor, and the UAfouled value is further calculated via:

<div>$$ UA_{tower,f} = UA_{tower,ff} \times F_{UA} $$</div>
where
<span>$ UA_{tower,f} $</span> 	U-factor times area values in the faulty case
<span>$ UA_{tower,ff} $</span> 	U-factor times area values in the fault-free case
<span>$ F_{UA} $</span>  	Factor describing the tower UA reduction due to fouling; applicable to both the design UA and free convection UA of the tower


### Fault: Coil Supply Air Temperature Sensor Offset ###
#### Symptom ####
The coil supply air temperature readings deviate from the actual air temperature levels due to sensor offset at the coil outlet. Because coil outlet node is usually used as the setpoint node for coil control, the fault may affect the actual performance of the coils. It can results in inappropriate coil operations such as coil on/off mode and water-side flow rate control, and therefore affect the coil energy consumption. Since the coil outlet air temperature deviate from the design level, the operations and performance of other components (e.g., other coils) at the downstream may also be affected.

#### Model inputs ####
Name of the coil with fault, reference offset value, name of availability schedule (on/off schedule to indicate when faults are applicable), name of severity schedule (normalized schedule to indicate the degree of faults)

#### Approach ####
EnergyPlus can model a number of coil types, some of which are temperature-based control and the others are load-based control. The proposed fault model will be applied to the ones with temperature-based control, namely:
-	Coil:Heating:Electric
-	Coil:Heating:Electric:Multistage
-	Coil:Heating:Gas
-	Coil:Heating:Gas:Multistage
-	Coil:Heating:Steam
-	Coil:Heating:Desuperheater
-	Coil:Heating:Water
-	Coil:Cooling:Water
-	Coil:Cooling:Water:Detailedgeometry

The effect of an offset in a coil supply air temperature sensor whose sole use is for calculation of the difference between the set-points and the actual values can be modeled as an equal and opposite offset:

<div>$$ T_{coil-o,f} = T_{coil-o,ff} - \Delta T $$</div>
where
<span>$ T_{coil-o,f} $</span> 	coil outlet temperature in the faulty case (actual value)
<span>$ T_{coil-o,ff} $</span> 	coil outlet temperature in the fault-free case (reading value)
<span>$ \Delta T $</span> 	difference between the temperature reading and the actual temperature

Note that the fault only affect the coil operations, but doesn’t affect the coil sizing calculations. So it is possible that the faulty sensor temperature will lead to a heat transfer rate that goes beyond the maximum coil capacity. In this case, the maximum coil capacity will be used to reversely calculate the actual supply air temperature.

Also note that “Coil:Heating:Water”, “Coil:Cooling:Water”, and “Coil:Cooling:Water:Detailedgeometry” are controlled via “Controller:WaterCoil”, while the other coil types are controlled with an internal “Temperature Setpoint Node”. These two types may need to be handled separately in the fault model implementation.


## Model Implementation ##

The proposed model implementation work will try to introduce more object oriented features to the fault modeling routines. Four new classes will be added in the EnergyPlus::FaultsManager namespace, namely:
-	(1) struct FaultPropertiesChillerSWT : public FaultProperties, corresponding to the IDD object FaultModel:TemperatureSensorOffset:ChillerSupplyWater
-	(2) struct FaultPropertiesCondenserSWT : public FaultProperties, corresponding to the IDD object FaultModel:TemperatureSensorOffset:CondenserSupplyWater
-	(3) struct FaultPropertiesCoilSAT : public FaultProperties, corresponding to the IDD object FaultModel:TemperatureSensorOffset:CoilSupplyAir
-	(4) struct FaultPropertiesTowerFouling : public FaultProperties, corresponding to the IDD object FaultModel:Fouling:CoolingTower.
These classes will be inherited from Class "FaultProperties" which is the base class for all the operational fault models.
The new added functions will be defined as member functions of the corresponding fault class.

The new added fault classes will contain the information about the faults as well as the links to the affected objects. Take class "FaultPropertiesChillerSWT" for example, it uses member variables "ChillerType" and "ChillerName" to specify the chiller that is affected by the fault. Three new member variables will be added to the existing chiller class (e.g., "ReformulatedEIRChillerSpecs" in EnergyPlus::ChillerReformulatedEIR):
-	1) bool FaultyChillerSWTFlag; // True if the chiller has SWT sensor fault
-	2) int FaultyChillerSWTIndex;  // Index of the fault object corresponding to the chiller
-	3) Real64 FaultyChillerSWTOffset; // True if the chiller has SWT sensor fault.
These three variables will be initialized in the method EnergyPlus::FaultsManager::CheckAndReadFaults() which processes all the fault objects. Then these variables will be used in the chiller calculations to activate the fault calculations when the fault presents.

The model implementations will try to minimize the modification of the codes outside of the fault routine. Take the FaultModel:TemperatureSensorOffset:ChillerSupplyWater for example, all the massive calculations about the faulty chiller will be put in "FaultsManager.cc" instead of the chiller routines. This will be achieved by defining a member function "CalFaultChillerSWT" within class "FaultPropertiesChillerSWT". The chiller routines will call this function whenever the fault presents. Such implementation design will minimize the modifications of the chiller routines, and thus benefit their systematic organization and potential future maintenance.


## IDD Object (New) ##
New objects will be created for the proposed fault types, namely:
-	FaultModel:TemperatureSensorOffset:ChillerSupplyWater,
-	FaultModel:TemperatureSensorOffset:CondenserSupplyWater,
-	FaultModel:TemperatureSensorOffset:CoilSupplyAir,
-	FaultModel:Fouling:CoolingTower.

```
FaultModel:TemperatureSensorOffset:ChillerSupplyWater,
   \memo This object describes fault of chiller supply water temperature sensor offset
   \min-fields 3
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
       \note Enter the type of the chiller affected
       \required-field
       \type choice
       \key Chiller:Electric
       \key Chiller:Electric:EIR
       \key Chiller:Electric:ReformulatedEIR
       \key Chiller:ConstantCOP
       \key Chiller:EngineDriven
       \key Chiller:CombustionTurbine
       \key Chiller:Absorption
       \key Chiller:Absorption:Indirect
   A5, \field Chiller Object Name
       \note Enter the name of the chiller affected
       \required-field
       \type object-list
       \object-list Chillers
   N1; \field Reference Sensor Offset
       \type real
       \minimum> -10
       \maximum< 10
       \default 0.0
       \units deltaC

FaultModel:TemperatureSensorOffset:CondenserSupplyWater,
   \memo This object describes fault of condenser supply water temperature sensor offset
   \min-fields 3
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
   A4, \field Tower Object Type
       \note Enter the type of the cooling tower affected
       \required-field
       \type choice
       \key CoolingTower:SingleSpeed
       \key CoolingTower:TwoSpeed
       \key CoolingTower:VariableSpeed
       \key CoolingTower:VariableSpeed:MERKEL
   A5, \field Cooling Tower Object Name
       \note Enter the name of the cooling tower affected
       \required-field
       \type object-list
       \object-list CoolingTower:VariableSpeed
   N1; \field Reference Sensor Offset
       \type real
       \minimum> -10
       \maximum< 10
       \default 0.0
       \units deltaC

FaultModel:TemperatureSensorOffset:CoilSupplyAir,
   \memo This object describes fault of coil supply air temperature sensor offset
   \min-fields 3
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
   A4, \field Coil Object Type
       \note Enter the type of the coil affected
       \required-field
       \type choice
       \key Coil:Heating:Electric
       \key Coil:Heating:Electric:Multistage
       \key Coil:Heating:Gas
       \key Coil:Heating:Gas:Multistage
       \key Coil:Heating:Steam
       \key Coil:Heating:Desuperheater
       \key Coil:Heating:Water
       \key Coil:Cooling:Water
       \key Coil:Cooling:Water:Detailedgeometry
   A5, \field Coil Object Name
       \note Enter the name of the coil affected
       \required-field
       \type object-list
       \object-list CoolingCoilName
       \object-list HeatingCoilName
       \object-list HeatingCoilsElectricMultiStage
       \object-list HeatingCoilsElectricMultiStage
       \object-list HeatingCoilsDesuperheater
   N1; \field Reference Sensor Offset
       \type real
       \minimum> -10
       \maximum< 10
       \default 0.0
       \units deltaC

FaultModel:Fouling:CoolingTower,
   \memo This object describes the fault of fouling cooling towers
   \min-fields 3
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
   A4, \field Cooling Tower Object Type
       \note Enter the type of the cooling tower affected
       \required-field
       \type choice
       \key CoolingTower:SingleSpeed
       \key CoolingTower:TwoSpeed
       \key CoolingTower:VariableSpeed
       \key CoolingTower:VariableSpeed:MERKEL
   A5, \field Cooling Tower Object Name
       \note Enter the name of the cooling tower affected
       \required-field
       \type object-list
       \object-list CoolingTower:VariableSpeed
   N1; \field UA Reduction Factor
       \note Factor describing the tower UA reduction due to fouling
       \note It is the ratio between the UA value at fouling case and that at fault free case
       \note It is applicable to both the Design UA and Free Convection UA of the tower
       \units dimensionless
       \type real
       \minimum> 0.0
       \maximum 1.0

```

## Testing/Validation/Data Source(s) ##
Comparing simulation results with and without faults will be performed to ensure accuracy of the new features.


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
