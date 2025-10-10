Economizer Integration Enhancements
================

**Lixing Gu**

**Florida Solar Energy Center**


 - First revision of Design Document based on Edwin's comments by adding IO Ref and Eng. Ref sections on 12/2/15
 - Design document
 - First revision based on the conference call on 10/20/15 (10/23/15)
 - Original version on 10/15/15



## Justification for New Feature ##

The EnergyPlus economizer simulation does not align fully with ASHRAE Standard 90.1-2013 requirements for typical DX and variable speed fan systems. Pacific Northwest National Laboratory (PNNL) developed EnergyPlus Energy Management System (EMS) code to better match these requirements from the ASHRAE Standard1. This NFP proposes an approach to implement the same EMS functionality in C++ and adapted to become functionally equivalent internal control capabilities within EnergyPlus.

## E-mail and  Conference Call Conclusions ##

###E-mails###

There are lot of E-mail communications before and after the conference call. It is better to provide summary based on conclusions.

###Conference call###

The first conference call was held on 10/20/15. Mike Witte, Brent Griffith, Reid Hart, Weiming Wang and Lixing Gu attended the conference call.

###Communication Summary###

1 First draft NFP

The first draft NPF mainly presented approaches how to implement the EMS code exactly in EnergyPlus. The review team thought it would be better to take advantages of native code to make better approaches.

2 Consensus

Implementation of same EMS functionality may not be enough to show native code. It is better to implement the code based on original standard and algorithms.

3 Multiple mode in a single time step

The EMS code assume multiple modes, such as cooling and economizer, may occur in a single time. It is suggested that when a smaller time step is used, a single mode is sufficient. This can be accomplished using small time steps. PNNL agreed that if a smaller system time step is performed, a single mode approach in a time step should be OK.

4 Fan power calculation

The exponent function of fan power ratio used in the ASHRAE paper can be achieved by exiting Fan:OnOff object. The object has a field to calculate power ration based on an exponential function.

5 Economizer operation

The EMS code uses the economizer effectiveness regression curves to calculate the maximum outdoor air fraction to comply 90.1 code requirements:

<span style="color:red;">the outdoor air damper is at the 100% open position when mechanical cooling is on, and the outdoor air damper does not begin to close to prevent coil freezing due to minimum compressor run time until the leaving air temperature is less than 45°F.</span>

The team thinks to use native code to accomplish the goal to ensure the minimum leaving air temperature is 45F by using a modified SetpointManager:MixedAir.


###Actions after the first conference call###

Revise approaches to make native code to accomplish the scope.

####Comments received after the first revision####

Nagappan Chidambaram, Trane


Thanks Gu. I think so this is a good change but I see the proposal focusses only on Multi Speed Unitary Object. It would be great if we can extend to built-up  Airloop HVAC objects. I am assuming this would work with Variable Speed DX objects too because the same principle can be applied.


Gu's reply

It is assumed that AirLoopHVAC:UnitarySystem is able to take over other up  Airloop HVAC objects, so that any effort will be focused on the AirLoopHVAC:UnitarySystem. Regarding your concern of Variable Speed DX objects, since I need to deliver this new feature in Dec., it will be added later.


## Overview ##

This new feature will implement two major capabilities to calculate fan power more accurately with multiple modes in a single time step and reduces outdoor air flow rate when the temperature at the cooling coil outlet is less than 45F.

1 Economizer operation

The main goal is to meet code requirement defined in Section 6.5.1.3 a of 90.1 to reduce outdoor air flow rate when the temperature at the cooling coil outlet is less than 45F.

6.5.1.3
a. Unit controls shall have the mechanical cooling capacity control interlocked with the air economizer controls such that the outdoor air damper is at the 100% open position when mechanical cooling is on, and the outdoor air damper does not begin to close to prevent coil freezing due to minimum compressor run time until the leaving air temperature is less than 45°F.

The EMS code and ASHRAE paper uses economizer effectiveness regression curves to calculate the maximum outdoor air fraction.

After extensive discussion, it is agreed that a modified setpoint manager will do a better job to perform native code way to meet 90.1 requirements. Based on the outlet temperature of a cooling coil or an AirLoop, the mixed air temperature at the outlet of an OA mixer is calculated as a setpoint temperature. Based on the setpoint temperature of the OA mixer outlet, the maximum outdoor air fraction can be calculated. Therefore, no economizer effectiveness regression curves are needed. It is guaranteed that coil freezing can be prevented.

2 Fan power calculation

The existing EMS code calculates fan power use by assuming different modes are applied in a single time step.

After extensive discussion, it is agreed that if a smaller system time step, such 1 - 6 minutes, is used, it is not necessary to have multiple modes in a single time step. If a Fan:OnOff is used, the existing fan power curve can make power ratio calculation correctly during coil operation. In addition, when a flow rate with no cooling and heating is set correctly, it should provide more accurate power use during coil off time. In this way, a single operation mode is assume in a time step. The existing capability can provide such performance with modifications.

## Approach ##

This section covers approaches to calculate fan power more accurately with multiple modes in a single time step and reduces outdoor air flow rate when the temperature at the cooling coil outlet is less than 45F.

1	Economizer operation

The proposed modification of a setpoint manager is SetpointManager:MixedAir. The current object provides a setpoint manager that takes an already established setpoint (usually the supply air outlet node setpoint temperature), subtracts the supply fan heat gain, and applies the result as the setpoint temperature at the mixed air node (or any other node the user specifies). Three optional fields will be added to input cooling coil information and a limit of the cooling coil outlet temperature.

  	A7, \field Cooling Coil Inlet Node Name
 	A8, \field Cooling coil Outlet Node Name
 	N1; \field Minimum Temperature at Cooling Coil Outlet Node

When a draw-thro supply fan is used:
Setpoint = MAX (  (reference node setpoint  + coil depression), (N1 + coil depression)  )

When a blow-thro supply fan is used:
Setpoint = MAX (  (reference node setpoint – fan rise + coil depression), (N1 – fan rise + coil depression)  )

The proposed method is simpler and more native code way to meet 90.1 requirements.


2 Fan power calculation

We propose a revision to make more accurate fan power calculation by adding a new filed in the  UnitarySystemPerformance:Multispeed object called by the AirLoopHVAC:UnitarySystem object. The main function is to ensure a single mode operation is applied with given speed number, so that fan power calculation during cooling operation mode is accomplished. The existing code calculate fan power based on two adjacent speeds.

Multiple speed coils

When zone loads vary with time, it is better to have multiple speed coil to provide energy efficient HVAC operation. The current coil control is based on two categories. If the zone load is less than lowest speed capacity, the coil will be on/off to meet loads by keeping the same fan flow rate, similar performance to a single mode operation coil. When the zone load is above the minimum capacity, the coil will require two consecutive speed operation. The proposed change will allow each speed has its independent operation with given speed number as single mode operation. For example, when the zone load is between capacities between speed 2 and speed 3, the speed 3 alone operation is called.


## Testing/Validation/Data Sources ##

Compare simulation results with spread sheet calculations.

## Input Output Reference Documentation ##

Revisions or additions to the sections are noted as <span style="color:red;">red</span>.

The input Output Reference has several sections related to the modified inputs:  SetpointManager:MixedAir and UnitarySystemPerformance:Multispeed.

### SetpointManager:MixedAir

The Mixed Air Setpoint Manager is meant to be used in conjunction with a Controller:OutdoorAir object. This setpoint manager is used to establish a temperature setpoint at the mixed air node. The Controller:OutdoorAir then operates the outdoor air damper to attempt to meet this setpoint.

In EnergyPlus the relief and outdoor air dampers, the economizer, and any outdoor air conditioning equipment form a separate subsystem of an air loop system (ref. AirLoopHVAC). The outdoor air controller operates within the subsystem. Consequently the mixed air temperature setpoint must take into account any downstream system fan heat if it is to meet a desired system supply air leaving temperature. The Mixed Air Setpoint Manager accomplishes this by referencing a supply air setpoint set by another setpoint manager (most likely at the AirLoopHVAC outlet node). The supply fan inlet and outlet nodes are also inputs to the Mixed Air Setpoint Manager. From this information the Mixed Air Setpoint Manager calculates the supply fan air temperature rise, subtracts it from the reference setpoint temperature, and sets the result as the mixed air node setpoint temperature.

Of course any type of setpoint manager can be used to establish a temperature setpoint at the mixed air node. But the Mixed Air Setpoint Manager is likely to be the most useful.

<span style="color:red;">When optional inputs of Cooling Coil Inlet Node Name, Cooling coil Outlet Node Name, and Minimum Temperature at Cooling Coil Outlet Node are provided, the setpoint temperature at the mixed air node is calculated based on the maximum of both the reference setpoint temperature and minimum cooling coil outlet temperature, minus the cooling coil air temperature reduction and the supply fan air temperature rise if the supply fan placement is blow through.</span>

#### Field: Name

A unique, user-assigned name for an instance of a mixed air setpoint manager. Anywhere in the input that this setpoint manager is used, it is referred to by this name.

#### Field: Control Variable

The type of variable that will be controlled. There is only one choice for this type of setpoint manager: Temperature.

#### Field: Reference Setpoint Node Name

The name of an HVAC system node where the system supply air temperature is set. Normally this would be the AirLoopHVAC outlet node. The temperature setpoint at this reference node is set by a different setpoint manager.

#### Field: Fan Inlet Node Name

The name of the supply fan inlet node.

#### Field: Fan Outlet Node Name

The name of the supply fan outlet node.

#### Field: Setpoint Node or NodeList Name

The name of a NodeList object containing the names of the HVAC system nodes or the HVAC System Node Name where temperature setpoints will be established by this setpoint manager.

#### Field: Cooling Coil Inlet Node Name

The name of the cooling coil inlet node.

#### Field: Cooling Coil Outlet Node Name

The name of the cooling coil outlet node.

#### Field: Minimum Temperature at Cooling Coil Outlet Node

In order to prevent cooling coil freezing, the minimum temperature at the cooling coil outlet is required. The default value is 7.2C.


Below is an example input for a Mixed Air Setpoint Manager.

```idf
SetpointManager:SingleZone:Reheat, ! establishes the setpoint at the system outlet node
           Supply Air Temp Manager,   !- Name
           Temperature,   !- Control Variable
           13.,  !- Minimum Supply Air Temperature
           45.,  !- Maximum Supply Air Temperature
           NORTH ZONE,  !- Control Zone Name
           Zone 3 Node,  !- Zone Node Name
           Zone 3 Inlet Node,  !- Zone Inlet Node Name
          Supply Air Temp Nodes;   !- Setpoint Node or NodeList Name


SetpointManager:MixedAir, ! uses the system outlet setpoint to establish the mixed air setpoint
           Mixed Air Temp Manager 1,   !- Name
           Temperature,   !- Control Variable
           Air Loop Outlet Node,                   !- Reference Setpoint Node Name
           Heating Coil Air Outlet Node,   !- Fan Inlet Node Name
           Air Loop Outlet Node,                   !- Fan Outlet Node Name
           Mixed Air Nodes;                             !- Setpoint Node or NodeList Name

```
### UnitarySystemPerformance:Multispeed

#### Field: Name

This alpha field contains the identifying name for the multispeed performance specification.

#### Field: Number of Speeds for Heating

This field defines the number of heating speeds for the heat pump, and must match the number of heating speeds defined in the associated heating coil. The value for this input field defines the number of airflow rate ratios that must be defined for heating in the fields below. The minimum value for this field is one and the maximum value is the number specified in the coil object. If the heating coil type used in the unitary system object is not a multispeed coil type, then this field should be 1.

#### Field: Number of Speeds for Cooling

This field defines the number of cooling speeds for the heat pump, and must match the number of cooling speeds defined in the associated DX cooling coil. The value for this input field defines the number of airflow rate ratios that must be defined for cooling in the fields below. The minimum value for this field is one and the maximum value is the number specified in the coil object. If the cooling coil type used in the unitary system object is not a multispeed coil type, then this field should be 1.

#### Field: Single Mode Operation

This field specifies the coil operation mode for multiple speed DX cooling and heating coils during each HVAC time step. The allowed choice is Yes or No. The No choice allows a coil works between two adjacent speeds when a system load is greater than the coil capacity at speed 1. The Yes choice allows a coil works with a single capacity at a different speed. The speed number is determined by a system load.

#### Field: Heating Speed 1 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the heating coil and/or supplemental heater are operating at Speed 1 (lowest speed). Values must be greater than 0.

#### Field: Cooling Speed 1 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the DX cooling coil is operating at Speed 1 (lowest speed). Values must be greater than 0.

#### Field: Heating Speed 2 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the heating coil and/or supplemental heater are operating at Speed 2. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for heating speed 1.

#### Field: Cooling Speed 2 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the DX cooling coil is operating at Speed 2. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for cooling speed 1.

#### Field: Heating Speed 3 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the heating coil and/or supplemental heater are operating at Speed 3. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for heating speed 2.If the ‘Number of Speeds for Heating’ is less than 3, then this field can be left blank.

#### Field: Cooling Speed 3 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the DX cooling coil is operating at Speed 3. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for cooling speed 2.If the ‘Number of Speeds for Cooling’ is less than 3, then this field can be left blank.

#### Field: Heating Speed 4 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the DX heating coil and/or supplemental heater are operating at Speed 3. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for heating Speed 3.If the ‘Number of Speeds for Heating’ is less than 4, then this field can be left blank.

#### Field: Cooling Speed 4 Supply Air Flow Ratio

This numeric field defines the ratio of supply air flow rate leaving the unitary system to the maximum air flow rate specified in the coil object at maximum speed when the DX cooling coil is operating at Speed 4. Values must be greater than 0. The entered value must be greater or equal to the flow rate ratio specified for cooling Speed 3.If the ‘Number of Speeds for Cooling’ is less than 4, then this field can be left blank.

```idf
UnitarySystemPerformance:Multispeed,
   MyMultispeedHPSpec,      !- Name
   4,                       !- Number of Speeds for Heating
   4,                       !- Number of Speeds for Cooling
   Yes,                     !- Single Mode Operation
   0.235294118,             !- Heating Speed 1 Supply Air Flow Rate {m3/s}
   0.235294118,             !- Cooling Speed 1 Supply Air Flow Rate {m3/s}
   0.470588235,             !- Heating Speed 2 Supply Air Flow Rate {m3/s}
   0.470588235,             !- Cooling Speed 2 Supply Air Flow Rate {m3/s}
   0.705882353,             !- Heating Speed 3 Supply Air Flow Rate {m3/s}
   0.705882353,             !- Cooling Speed 3 Supply Air Flow Rate {m3/s}
   1.0,                     !- Heating Speed 4 Supply Air Flow Rate {m3/s}
   1.0;                     !- Cooling Speed 4 Supply Air Flow Rate {m3/s}
```

## Input Description ##

### Existing objects ###

Revisions to the IDD are noted as **<span style="color:red;">bold red</span>** non-blocked insertions at the appropriate location throughout the input data dictionary description.

1	SetpointManager:MixedAir

Three optional fields are added to set the OA mixer outlet temperature to prevent cooling coil freezing.

	SetpointManager:MixedAir,
     \memo The Mixed Air Setpoint Manager is meant to be used in conjunction
     \memo with a Controller:OutdoorAir object. This setpoint manager is used
     \memo to establish a temperature setpoint at the mixed air node.
  	A1 , \field Name
       \required-field
  	A2 , \field Control Variable
       \type choice
       \key Temperature
       \default Temperature
  	A3 , \field Reference Setpoint Node Name
       \required-field
       \type node
  	A4 , \field Fan Inlet Node Name
       \required-field
       \type node
  	A5 , \field Fan Outlet Node Name
       \required-field
       \type node
  	A6 , \field Setpoint Node or NodeList Name
       \required-field
       \type node
       \note Node(s) at which the temperature will be set
 <span style="color:red;">**A7 , \field Cooling Coil Inlet Node Name**</span>

       \type node
 <span style="color:red;">**A8 , \field Cooling coil Outlet Node Name**</span>

       \type node
 <span style="color:red;">**N1 ; \field Minimum Temperature at Cooling Coil Outlet Node**</span>

        \type real
        \units C
        \minimum> 0.0
        \default 7.2


Output:

When a draw-thro supply fan is used:
Setpoint = MAX (  (reference node setpoint  + coil depression), (N1 + coil depression)  )

When a blow-thro supply fan is used:
Setpoint = MAX (  (reference node setpoint – fan rise + coil depression), (N1 – fan rise + coil depression)  )

2 UnitarySystemPerformance:Multispeed

A new optional field is added to ensure a single mode operation at any given speed number. The object is called by the AirLoopHVAC:UnitarySystem object.

	UnitarySystemPerformance:Multispeed,
       \memo The UnitarySystemPerformance object is used to specify the air flow ratio at each
       \memo operating speed. This object is primarily used for multispeed DX and water coils to allow
       \memo operation at alternate flow rates different from those specified in the coil object.
       \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
  	A1 , \field Name
       \required-field
       \reference UnitarySystemPerformaceNames
  	N1 , \field Number of Speeds for Heating
       \required-field
       \type integer
       \minimum 0
       \maximum 10
       \note Used only for Multi speed coils
       \note Enter the number of the following sets of data for air flow rates.
  	N2 , \field Number of Speeds for Cooling
       \required-field
       \type integer
       \minimum 0
       \maximum 10
       \note Used only for Multi speed coils
       \note Enter the number of the following sets of data for air flow rates.

<span style="color:red;">**A2 , \field Single Mode Operation**</span>

       \type choice
       \key Yes
       \key No
       \default No
  	N3 , \field Heating Speed 1 Supply Air Flow Ratio
       \required-field
       \type real
       \autosizable
       \minimum> 0
       \begin-extensible
       \note Used only for Multi speed coils
       \note Enter the lowest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N4 , \field Cooling Speed 1 Supply Air Flow Ratio
       \required-field
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the lowest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N5 , \field Heating Speed 2 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N6 , \field Cooling Speed 2 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N7 , \field Heating Speed 3 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N8 , \field Cooling Speed 3 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N9 , \field Heating Speed 4 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
  	N10; \field Cooling Speed 4 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.

if a test example may not work properly with multiple coils under AirLoopHVAC:UnitarySystem before modification, AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed with mutlispeed coils will be used to test single mode operation. Possible bug fix effort should not be included in this new feature effort.


## Outputs Description ##

Modifications have been applied to the following output variables in the AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed object.

### AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed

### Unitary Air to Air MultiSpeed Heat Pump (AirLoopHVAC) Outputs

* HVAC,Average,Unitary System DX Coil Cycling Ratio []

* HVAC,Average,Unitary System DX Coil Speed Ratio []


#### Unitary System DX Coil Cycling Ratio []

This output variable is the ratio of the sensible load (heating or cooling) to the steady-state capacity of the multispeed heat pump’s DX heating or cooling coil (Speed 1) for the entire system timestep. The value is between 0.0 and 1.0 when the heat pump is cycling on and off its lowest speed (Speed 1) and 1.0 when the multispeed heat pump operates at speeds above 1.

<span style="color:red;">When Single Mode Operation is specified, the value is between 0.0 and 1.0 when the heat pump is cycling on at any given speed. </span>

#### Unitary System DX Coil Speed Ratio []

This output variable is the ratio of time in a system timestep that the compressor is at rated speed between two consecutive speed numbers ( [Compressor Speed - Compressor speed at Speed i-1] / [Compressor speed at Speed i - Compressor speed at Speed i-1]). The compressor speed ratio reports (1.0 is max, 0.0 is min) and any value in between as it is averaged over the timestep. The value is 0.0 during Speed 1 operation.

The physical meaning of the speed ratio is dependent on the compressor configuration defined in the field of child coil object: Apply Part Load Fraction to Speeds greater than 1. The allowed choice is either Yes or No. When No is entered, one compressor is assumed for all speeds.  The speed ratio represents how long the higher speed runs as a fraction of the system timestep, and the lower speed runs in the rest of the system timestep. When Yes is entered, multiple compressors are assumed, and each compressor has associated speed. The speed ratio represents how long the higher speed runs as a fraction of the system timestep, and the low speed runs in a whole system timestep.

<span style="color:red;">When Single Mode Operation is specified, the speed ratio is set to 0 at Speed 1 and 1 at Speed > 1.</span>

## Engineering Reference ##

Revisions or additions to the sections are noted as <span style="color:red;">red</span>.

The Engineering Reference has several sections related to the modified calculation procedures: Setpoint Managers and multispeed DC cooling and heating coils: Coil:Heating:DX:MultiSpeed, and     Coil:Cooling:DX:MultiSpeed.

###Setpoint Managers <a name="SetpointManagers"></a>

### Mixed Air

The input object SetpointManager:MixedAir provides a setpoint manager that takes an already established setpoint (usually the supply air outlet node setpoint temperature), subtracts the supply fan heat gain, and applies the result as the setpoint temperature at the mixed air node (or any other node the user specifies).

<div>$${T_{set}} = {T_{set,ref}} - ({T_{fan,outlet}} - {T_{fan,inlet}})$$</div>

<span style="color:red;">When inputs of Cooling Coil Inlet Node Name, Cooling coil Outlet Node Name, and Minimum Temperature at Cooling Coil Outlet Node are provided, the setpoint temperature at the mixed air node is given below based on supply fan placement:

<span style="color:red;">Draw through placement:

<div>$${T_{set}} = max(T_{set,ref}, T_{min}) - ({T_{coil,outlet}} - {T_{coil,inlet}})$$</div>

<span style="color:red;">Blow through placement:

<div>$${T_{set}} = max(T_{set,ref}, T_{min}) - ({T_{coil,outlet}} - {T_{coil,inlet}}) -  - ({T_{fan,outlet}} - {T_{fan,inlet}})$$</div>

### Multi-Speed Electric DX Air Cooling Coil

#### Overview

This model (object name Coil:Cooling:DX:MultiSpeed) simulates the performance of an air-to-air direct expansion (DX) cooling system. The main difference compared to the other cooling coil models, such as Coil:Cooling:DX:SingleSpeed, is that this cooling coil allows modeling of two to four discrete compressor speeds. Each speed has a set of corresponding performance information at rated conditions along with curve fits for variations in total capacity, SHR, energy input ratio and part-load fraction to determine the performance of the unit at part-load conditions (DOE 1982).  The full load supply airflow rate is dependent on the speed number and provided by its parent object (Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed). The part-load impact on coil energy use is automatically applied to the lowest speed. A choice is provided to determine whether part-load impacts on coil energy use are applied when the coil is operating at speeds greater than speed 1.

This model simulates the thermal performance of the indoor DX cooling coil, and the power consumption of the outdoor unit (multispeed compressor, fans, and crankcase heaters). The performance of the indoor supply air fan varies widely from system to system depending on control strategy (e.g., constant fan vs. AUTO fan), fan type, fan motor efficiency and pressure losses through the air distribution system. Therefore, this DX system model does not account for the thermal effects or electric power consumption of the indoor supply air fan. EnergyPlus contains separate models for simulating the performance of various indoor fan configurations, and these models can be easily linked with the DX system model described here to simulate the entire DX system being considered. For the time being, this coil model can only be called by the parent object AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed.

When the model determines performance at Speed 1 (the lowest speed) or cycling between OFF and Speed 1, its performance is almost the same as the performance for the Coil:Cooling:DX:SingleSpeed model. However, the outlet conditions are calculated slightly differently. Therefore, the Coil:Cooling:DX:SingleSpeed model may be considered as a subset of the model described here. When the multispeed coil model determines performance at higher speeds (above 1), the model linearly interpolates the performance at two consecutive speeds (n-1 and n) as needed to meet the cooling load, with the fraction of time at each speed established by the speed ratio.

<span style="color:red;">When single mode operation is specified at higher speeds (above 1), its performance is almost the same as the performance for the Coil:Cooling:DX:SingleSpeed model at different flow rate and capacity with given speed number. No liner interpolation is performed between two adjacent speeds.</span>

#### Model Inputs

The model inputs are also very similar to the inputs of the Coil:Cooling:DX:SingleSpeed object. The main difference is that this multispeed model requires a set of fields at each speed, such as rated capacity, rated SHR, rated COP, two capacity modifiers, two energy input ratio modifiers, part-load correction, and latent degradation inputs. The inputs also include waste heat fraction at the rated conditions and modifier as a function of temperature to calculate recoverable waste heat for heat recovery, which are not available in the similar Coil:Cooling:DX:SingleSpeed object

#### Speed 1 Operation

The calculation procedures in this model, including defrost and crankcase heater, are indentical to the Coil:Heating:DX:SingleSpeed object (Ref: Coil:Heating:DX:SingleSpeed) with one exception: outlet node condition calculation when the supply air fan operates continuously (i.e., supply air fan operating mode schedule value is not equal to 0; Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed).

The following procedure provides the detailed description of the exception.

n Total delivered cooling capacity

The total delivered cooling capacity for speed 1 operating at the cycling ratio needed to meet the requested cooling load is:

<div>$${Q_{coil,cycling}} = {\mathop m\limits^\cdot_{Speed1}}*CycRatio*({h_{inlet}} - {h_{outlet,full}})$$</div>

where,

*Q<sub>coil,cyclingl</sub>*      = delivered total cooling capacity for Speed 1 operating at a specific cycling ratio [W]

<span>\({\mathop m\limits^\cdot_{Speed\,\,1}}\)</span>      = air mass flow rate through cooling coil at Speed 1 as set by the parent object [kg/s]

*h<sub>outlet,full</sub>*      = specific enthalpy of the coil outlet air during full-load operation at Speed 1 (no cycling) [J/kg]

*h<sub>inlet</sub>           *            = specific enthalpy of the coil inlet air [J/kg]

*CycRatio*    = cycling ratio at Speed 1, ratio of requested heating load to the full-load capacity of the coil at Speed 1 [dimensionless]

It is assumed that the coil provides no cooling capacity when the coil is OFF, even if the supply air fan continues to operate.

n Outlet air specific enthalpy

The average specific enthalpy of the coil outlet air is then calculated based on the total delivered cooling capacity and the average air mass flow rate entering the coil:

<div>$${h_{outlet,average}} = {h_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,cycling}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,cycling}}} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

where

h<sub>outlet,average</sub>    = averaged specific enthalpy at the coil outlet [J/kg]

h<sub>inlet</sub>             = specific enthalpy at the coil inlet [J/kg]

Q<sub>coil,cycling</sub>      = total capacity at full load [W]

<span>\({\mathop m\limits^\cdot_{inlet}}\)</span>         = mass flow rate at the inlet to the coil as established by the parent object (Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, Mass Flow Rate Calculation). This flow rate is the average value determined by the parent object, accounting for the specified flow rate when the cooling coil is ON and the specified flow rate when the cooling coil is OFF for the time step being simulated.

n Sensible capacity

The minimum humidity ratio (HR<sub>min</sub> ) is based on humidity ratios between inlet and full load outlet as:

HR<sub>min</sub> = Minimum(HR<sub>inlet</sub>, HR<sub>full</sub>)

where

HR<sub>inlet</sub>           = Humidity ratio at the inlet [kg/kg]

HR<sub>full</sub>            = Full load humidity ratio at the outlet [kg/kg]

The coil sensible capacity may be calculated as:

<div>$${Q_{coil,sens}} = {\mathop m\limits^\cdot_{Speed1}}*CycRatio*[{h_{inlet}}({T_{inlet}},H{R_{\min }}) - {h_{outlet,full}}({T_{outlet,full}},H{R_{\min }})]$$</div>

where

Q<sub>coil,sens</sub>        = delivered sensible cooling capacity [W]

h<sub>outlet,full</sub>         = full load specific enthalpy at the coil outlet as a function of outlet dry-bulb temperature at the full load, and the minimum humidity ratio [J/kg]

h<sub>inlet</sub>             = specific enthalpy at the coil inlet [J/kg]

n Latent capacity

The latent capacity is the difference between total and sensible capacities

<div>$${Q_{coil,latent}} = {Q_{coil,cycling}} - {Q_{coil,sens}}$$</div>

where

Q<sub>coil,latent</sub>        = delivered latent cooling capacity [W]

n Average outlet air humidity ratio

The averaged outlet HR can be calculated as:

<div>$$H{R_{outlet,average}} = H{R_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,latent}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,latent}}} {\mathop {\lambda m}\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop {\lambda m}\limits^\cdot  }$}}_{inlet}}$$</div>

where

λ     = heat of vaporization as a function of HR<sub>min</sub> and CycRatio\*T<sub>outlet,full</sub>+(1-CycRatio)\*T<sub>inlet</sub> [J/kg]

n Average outlet air temperature

Using the above averaged outlet humidity ratio and specific enthalpy, the averaged outlet temperature can be calculated using the psych function of PsyTdbFnHW.

The main reason using the above approach is that outlet conditions are calculated in the same way in low and high speed operation.

The crankcase heater defined for this DX cooling coil is enabled during the time that the compressor is not running for either heating or cooling. The crankcase heater power use from either heating or cooling is reported in the heating coil (Coil:Heating:DX:MultiSpeed).

#### Higher Speed Operation

This section describes how higher speed operation is simulated. When the required sensible load is less than the full load sensible capacity at Speed n (Speed Number &gt; 1), the following calculations are performed:

n Bypass factor at Speed n-1 and Speed n

<div>$$BypassFacto{r_n} = f(RatedBypassFacto{r_n},RatedFlowRat{e_n},ActualFowRat{e_n})$$</div>

<div>$$BypassFacto{r_{n - 1}} = f(RatedBypassFacto{r_{n - 1}},RatedFlowRat{e_{n - 1}},ActualFowRat{e_{n - 1}})$$</div>

where

BypassFactor<sub>i</sub>         = bypass factor at actual flow rate conditions at Speed i [dimensionless]

RatedBypassFactor<sub>i</sub>            = bypass factor at the rated conditions at Speed i [dimensionless]

RatedFowRate<sub>i</sub>        = air mass flow rate at the rated conditions at Speed i [kg/s]

ActualFowRate<sub>i</sub>       = actual air mass flow rate at Speed i [kg/s]

i     = Speed n or Speed n-1

The bypass factor at Speed n is a function of the bypass factor at the rated conditions, rated airflow rate, and actual flow rate at Speed n. The calculation is performed by a function, called AdjustCBF in the DXCoil module.

n Total capacity at Speed n-1 and Speed n

<span>\(TotCa{p_{n - 1}} = f(RatedCa{p_{n - 1}},TotCapTempModFa{c_{n - 1}},TotCapFlowModFa{c_{n - 1}},BypassFacto{r_{n - 1}})\)</span><span>\(TotCa{p_n} = f(RatedCa{p_n},TotCapTempModFa{c_n},TotCapFlowModFa{c_n},BypassFacto{r_n})\)</span>

where

TotCap<sub>i             </sub> = total cooling capacity at given temperatures and flow rates at Speed i [w]

RatedCap<sub>i     </sub> = cooling capacity at the rated conditions at Speed i [W]

TotCapTempModFac<sub>i                    </sub> = total cooling capacity modifier as a function of indoor web-bulb temperature and outdoor air dry-bulb temperature at Speed i

TotCapFlowModFac<sub>i                      </sub> = total cooling capacity modifier as a function of the ratio of the actual flow rate across the cooling coil to the rated airflow rate at Speed i

i     = Speed n or Speed n-1



The calculation is performed by a subroutine, called CalcTotCapSHR in the DXCoil module.

n EIR at Speed n-1 and Speed n

<div>$$EI{R_{n - 1}} = RatedEI{R_{n - 1}}*EIRTempModFa{c_{n - 1}}*EIRFlowModFa{c_{n - 1}}$$</div>

<div>$$EI{R_n} = RateEI{R_n}*EIRTempModFa{c_n}*EIRFlowModFa{c_n}$$</div>

where

EIR<sub>i                        </sub> = Energy input ratio at given temperatures and flow rates at Speed i [w]

RatedEIR<sub>i      </sub> = Energy input ratio at the rated conditions at Speed i [W]

EIRTempModFac<sub>i       </sub> = Energy input ratio modifier as a function of indoor and outdoor air dry-bulb temperature at Speed i

EIRFlowModFac<sub>i         </sub> = Energy input ratio modifier as a function of ratio of the actual flow rate across the heating coil to the rated airflow rate at Speed i

i     = n or n-1

n Full load outlet conditions at Speed n-1 and Speed n

The calculation procedure of full load outlet conditions at Speed n-1 and Speed n is the same as the calculation procedure used in the Coil:Cooling:DX:SingleSpeed model (Ref. Coil:Cooling:DX:SingleSpeed). The difference is that the outlet conditions at Speed n-1 are calculated based on the total cooling capacity and mass flow rate at Speed n-1, while the outlet conditions at Speed n are calculated based on the total cooling capacity and mass flow rate at Speed n.

n Effective total cooling capacity

<div>$$\begin{array}{l}{Q_{coil,SpeedRatio}} = (SpeedRatio){\mathop m\limits^\cdot_{Speed{\kern 1pt} n}}({h_{inlet}} - {h_{outlet,full\_Speed\;n}})\\\;\;\;\;\;\;\;\;\; + (1 - SpeedRatio){\mathop m\limits^\cdot_{Speed\;n - 1}}({h_{inlet}} - {h_{outlet,full\_Speed\;n - 1}})\end{array}$$</div>

where

<span>\({Q_{coil,SpeedRatio}}\)</span><sub>                       </sub> = delivered sensible cooling capacity at a given speed ratio between two consecutive speeds [W]

<span>\({\mathop m\limits^\cdot_{Speed\,n}}\)</span>      = air mass flow rate through cooling coil at Speed n as set by the parent object [kg/s]

<span>\({\mathop m\limits^\cdot_{Speed\,\,n - 1}}\)</span>    = air mass flow rate through cooling coil at Speed 1 as set by the parent object [kg/s]

h<sub>inlet</sub>             = specific enthalpy at the coil inlet [J/kg]

h<sub>outlet,full\_Speed\\ n</sub> = full load specific enthalpy at the coil outlet at Speed n [J/kg]

h<sub>outlet,full\_Speed\\ n-1</sub>           = full load specific enthalpy at the coil outlet at Speed n-1 [J/kg]

n Average outlet air specific enthalpy

<div>$${h_{outlet,average}} = {h_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,SpeedRatio}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,SpeedRatio}}} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

where

h<sub>outlet,average</sub>    = averaged specific enthalpy at the coil outlet [J/kg]

h<sub>inlet</sub>             = specific enthalpy at the coil inlet [J/kg]

<span>\({\mathop m\limits^\cdot_{inlet}}\)</span>         = mass flow rate at the inlet to the coil as established by the parent object (Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, Mass Flow Rate Calculation). This flow rate is the average value determined by the parent object, accounting for the specified flow rate when the heating coil is at Speed n and the specified flow rate when the heating coil is at Speed n-1 for the time step being simulated.

n Effective sensible cooling capacity

The minimum humidity ratio (HR<sub>min</sub> ) is calculated as

HR<sub>min</sub> = Minimum[HR<sub>inlet</sub>, (SpeedRatio)HR<sub>full,n</sub>+(1.0-SpeedRatio)HR<sub>full,n-1</sub>)

The effective sensible cooling capacity is expressed as:

<div>$$\begin{array}{l}{Q_{coil,sens}} = {\mathop m\limits^\cdot_{Speed\;n}}(SpeedRatio)[{h_{inlet}}({T_{inlet}},H{R_{\min }}) - {h_{outlet,full\_Speed\,n}}({T_{outlet,n}},H{R_{\min }})]\\\;\;\;\;\;\;\;\;\;\;\;\;\;\; + {\mathop m\limits^\cdot_{Speed\;n - 1}}(1 - SpeedRatio)[{h_{inlet}}({T_{inlet}},H{R_{\min }}) - {h_{outlet,full\_Speed\;n - 1}}({T_{outlet,n - 1}},H{R_{\min }})]\end{array}$$</div>

where

Q<sub>coil,sens</sub>        = effective sensible cooling capacity [W]

h<sub>outlet,full\_Speed\\ n</sub> = full load specific enthalpy at the coil outlet at Speed n as a function of outlet dry-bulb temperature at the full load, and the minimum humidity ratio [J/kg]

h<sub>outlet,full\_Speed\\ n-1</sub> = full load specific enthalpy at the coil outlet at Speed n-1 as a function of outlet dry-bulb temperature at the full load, and the minimum humidity ratio [J/kg]

h<sub>inlet</sub>             = specific enthalpy at the coil inlet [J/kg]

n Aaverage outlet air humidity ratio and temperature

The effective latent cooling capacity is the difference between the total and sensible capacity:

<div>$${Q_{coil,latent}} = {Q_{coil,SpeedRatio}} - {Q_{coil,sens}}$$</div>

Q<sub>coil,latent</sub>        = effective latent cooling capacity [W]

The average outlet air HR can be calculated as:

<div>$$H{R_{outlet,average}} = H{R_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,latent}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,latent}}} {\mathop {\lambda m}\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop {\lambda m}\limits^\cdot  }$}}_{inlet}}$$</div>

where

λ     = heat of vaporization as a function of HR<sub>min</sub> and SpeedRatio\*T<sub>outlet,n</sub>+(1-SpeedRatio)\*T<sub>outlet,n-1</sub> [J/kg]

At the given averaged outlet humidity ratio and specific enthalpy, the averaged outlet temperature can be calculated using the psych function of PsyTdbFnHW.

n Calculate combined energy input

When the input for the field ‘Apply Part Load Fraction to Speeds Greater than 1’ is No in the object (equivalent to a single compressor), the combined energy output is calculated as follows:

<span>\(CoolingPower = (TotCa{p_n})(EI{R_n})(SpeedRatio) + (TotCa{p_{n - 1}})(EI{R_{n - 1}})(1.0 - SpeedRatio)\)</span> When the input for the field ’Apply Part Load Fraction to Speeds Greater than 1’ is Yes in the object (equivalent to multiple compressors), the combined energy output is calculated as follows:

<div>$$CoolingPower = (TotCa{p_n})(EI{R_n})(RTF) + (TotCa{p_{n - 1}})(EI{R_{n - 1}})(1.0 - RTF)$$</div>

where

CoolingPower         = Power used in Watt

RTF            = Run time fraction at Speed n

n Latent degradation

When the supply fan operation mode is ContinuousFanWithCyclingCompressorand the input of the Apply Latent Degradation to Speeds Greater than 1 is Yes, the latent degradation is included at Speed n. The calculation procedure is the same as one in the Coil:Cooling:DX:SingleSpeed object. The difference is that the rated values and run time fraction at Speed n are used. The adjusted SHR is used to calculate full load outlet conditions at Speed n.

It is expected to have less latent degradation at Speed n than Speed 1. Therefore, smaller values of the latent degradation inputs at Speed n than those at Speed 1 are recommended.

n Crankcase heater

There is no power need at higher speed operation.

#### Higher Speed Operation with single mode operation

<span style="color:red;">This section describes how higher speed operation is simulated. When the required sensible load is less than the full load sensible capacity at Speed n (Speed Number &gt; 1), its performance is almost the same as the performance for the Coil:Cooling:DX:SingleSpeed model at different flow rate and capacity with given Speed n. No liner interpolation is performed between two adjacent speeds.</span>

..............

### Multi-Speed Electric Heat Pump DX Air Heating Coil

#### Overview

This model (object name Coil:Heating:DX:MultiSpeed:) simulates the performance of an air-to-air direct expansion (DX) heating system. The main difference compared to the other heating coil model (Coil:Heating:DX:SingleSpeed) is that this heating coil allows modeling of two to four discrete compressor speeds. Each speed has a set of corresponding performance information at rated conditions along with curve fits for variations in total capacity, energy input ratio and part-load fraction to determine the performance of the unit at part-load conditions (DOE 1982). The full load supply airflow rate is dependent on the speed number and is set by its parent object (Ref: AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed). The part-load impact on coil energy use is automatically applied to the lowest speed. A choice is provided to determine whether part-load impacts on coil energy use are applied when the coil is operating at speeds greater than Speed 1. Adjustment factors applied to total capacity and input power to account for frost formation on the outdoor coil are calculated at each speed.

This model simulates the thermal performance of the indoor DX heating coil, and the power consumption of the outdoor unit (multispeed compressor, fans, crankcase heaters and defrost heaters). The performance of the indoor supply air fan varies widely from system to system depending on control strategy (e.g., constant fan vs. AUTO fan), fan type, fan motor efficiency and pressure losses through the air distribution system. Therefore, this DX system model does not account for the thermal effects or electric power consumption of the indoor supply air fan. EnergyPlus contains separate models for simulating the performance of various indoor fan configurations, and these models can be easily linked with the DX system model described here to simulate the entire DX system being considered. For the time being, this coil model can only be called by the parent object AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed.

When the model determines performance at Speed 1 (the lowest speed) or cycling between OFF and Speed 1, its performance is almost the same as the performance for the Coil:Heating:DX:SingleSpeed model. However, the outlet conditions are calculated slightly differently. Therefore, the Coil:Heating:DX:SingleSpeed model may be considered as a subset of the model described here. When the multispeed coil model determines performance at higher speeds (above 1), the model linearly interpolates the performance at two consecutive speeds (n-1 and n) as needed to meet the heating load, with the fraction of time at each speed established by the speed ratio.

<span style="color:red;">When single mode operation is specified at higher speeds (above 1), its performance is almost the same as the performance for the Coil:Heating:DX:SingleSpeed model at different flow rate and capacity with given speed number. No liner interpolation is performed between two adjacent speeds.</span>

#### Model Inputs

The model inputs are also very similar to the inputs of the Coil:Heating:DX:SingleSpeed object. The main difference is that this multispeed model requires a set of fields at each speed, such as rated capacity, rated COP, two capacity modifiers, two energy input ratio modifiers, and part-load correction. The inputs also include waste heat fraction and modifier as a function of temperature to calculate recoverable waste heat for heat recovery, which are not available in the similar Coil:Heating:DX:SingleSpeed object.

#### Speed 1 Operation

The calculation procedures in this model, including defrost and crankcase heater, are indentical to the Coil:Heating:DX:SingleSpeed object (Ref: Coil:Heating:DX:SingleSpeed) with one exception: outlet node condition calculation when the supply air fan operation mode is ContinuousFanWithCyclingCompressor. The following procedure provides the detailed description of the exception.

n Total delivered heating capacity

The total delivered heating capacity for speed 1 operating at the cycling ratio needed to meet the requested heating load is:

<div>$${Q_{coil,cycling}} = {\mathop m\limits^\cdot_{Speed\,1}}\left( {CycRatio} \right)({h_{inlet}} - {h_{outlet,full}})$$</div>

where,

*Q<sub>coil,cycling</sub>*      = delivered sensible heating capacity for Speed 1 operating at a specific cycling ratio [W]

<span>\(\mathop {{\rm{ }}m}\limits_{Speed{\kern 1pt} 1}^ \cdot  \)</span>      = air mass flow rate through heating coil at Speed 1 as set by the parent object [kg/s]

*h<sub>outlet,full</sub>*      = specific enthalpy of the coil outlet air during full-load operation at Speed 1 (no cycling) [J/kg]

*h<sub>inlet</sub>           *            = specific enthalpy of the coil inlet air [J/kg]

*CycRatio*    = cycling ratio at Speed 1, ratio of requested heating load to the full-load capacity of the coil at Speed 1 [dimensionless]

It is assumed that the coil provides no heating capacity when the coil is OFF, even if the supply air fan continues to operate.

n Outlet air specific enthalpy

The average specific enthalpy of the coil outlet air is then calculated based on the delivered sensible heating capacity and the average air mass flow rate entering the coil:

<div>$${h_{outlet,average}} = {h_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,cycling}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,cycling}}} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

where,

*h<sub>outlet,average</sub>*  = average specific enthalpy at the coil outlet [J/kg]

<span>\({\dot m_{inlet}}\)</span>         = mass flow rate at the inlet to the coil as established by the parent object (Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, Mass Flow Rate Calculation). This flow rate is the average value determined by the parent object, accounting for the specified flow rate when the heating coil is ON and the specified flow rate when the heating coil is OFF for the time step being simulated.

n Outlet air temperature

The heating coil’s outlet air humidity ratio equals the inlet air humidity ratio since the coil does not change the moisture content of the air. So the average outlet air temperature is calculated based on the inlet air humidity ratio and the average outlet air enthalpy using the psychrometric function PsyTdbFnHW.

The main reason for using the above approach is that outlet air conditions are calculated in the same way for all operating speeds.

The crankcase heater defined for this DX heating coil is enabled during the time that the compressor is not running for either heating or cooling. The crankcase heater power use from either heating or cooling is reported in the heating coil.

#### Higher Speed Operation

This section describes how higher speed operation is simulated. When the required sensible load is less than the full load sensible capacity at Speed n (Speed Number &gt; 1), the following calculations are performed:

n Total delivered heating capacity at Speed n-1 and Speed n

<span>\(TotCa{p_{n - 1}} = RatedCa{p_{n - 1}}\left( {TotCapTempModFa{c_{n - 1}}} \right)\left( {TotCapFlowModFa{c_{n - 1}}} \right)\)</span><span>\(TotCa{p_n} = RatedCa{p_n}\left( {TotCapTempModFa{c_n}} \right)\left( {TotCapFlowModFa{c_n}} \right)\)</span>

where,

*TotCap<sub>i</sub>*<sub>           </sub> = total delivered heating capacity at given temperatures and flow rates at Speed i [W]

*RatedCap<sub>i</sub>*<sub>   </sub> = heating capacity at the rated conditions at Speed i [W]

*TotCapTempModFac<sub>i</sub>* = total heating capacity modifier as a function of indoor and outdoor air dry-bulb temperature at Speed i

*TotCapFlowModFac<sub>i</sub>* = total heating capacity modifier as a function of the ratio of the actual flow rate across the heating coil to the rated airflow rate at Speed i

i     = Speed n or Speed n-1

n EIR at Speed n-1 and Speed n

<div>$$EI{R_{n - 1}} = RatedEI{R_{n - 1}}\left( {EIRTempModFa{c_{n - 1}}} \right)\left( {EIRFlowModFa{c_{n - 1}}} \right)$$</div>

<div>$$EI{R_n} = RateEI{R_n}\left( {EIRTempModFa{c_n}} \right)\left( {EIRFlowModFa{c_n}} \right)$$</div>

where,

*EIR<sub>i</sub>*<sub>                      </sub> = energy input ratio at given temperatures and flow rates at Speed i [W]

*RatedEIR<sub>i</sub>*<sub>    </sub> = energy input ratio at the rated conditions at Speed i [W]

*EIRTempModFac<sub>i</sub>* = energy input ratio modifier as a function of indoor and outdoor air dry-bulb temperature at Speed i

*EIRFlowModFac<sub>i</sub>* = energy input ratio modifier as a function of the ratio of the actual flow rate across the heating coil to the rated airflow rate at Speed i

i     = Speed n or Speed n-1

n Full load outlet air specific enthalpy at Speed n-1 and Speed n

<div>$${h_{outlet,full\_Speed\,n}} = {h_{inlet}} - {{\raise0.7ex\hbox{${\left( {TotCa{p_n}*HeatingCapacityMultiplier} \right)}$} \!\mathord{\left/ {\vphantom {{\left( {TotCa{p_n}*HeatingCapacityMultiplier} \right)} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

<div>$${h_{outlet,full\_Speed\,n - 1}} = {h_{inlet}} - {{\raise0.7ex\hbox{${\left( {TotCa{p_{n - 1}}*HeatingCapacityMultiplier} \right)}$} \!\mathord{\left/ {\vphantom {{\left( {TotCa{p_{n - 1}}*HeatingCapacityMultiplier} \right)} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

where,

*HeatingCapacityMultiplier*            = frost adjustment factor for heating capacity (See Ref. Coil:Heating:DX:SingleSpeed)

*h<sub>outlet,full\_Speed\\ n</sub>*         = specific enthalpy of the coil outlet air during full-load operation at Speed n (no cycling) [J/kg]

*h<sub>outlet,full\_Speed\\ n-1</sub>*       = specific enthalpy of the coil outlet air during full-load operation at Speed n-1 (no cycling) [J/kg]

n Effective total heating capacity

<div>$$\begin{array}{l}{Q_{coil,SpeedRatio}} = \left( {SpeedRatio} \right){\mathop m\limits^\cdot_{Speed\,n}}({h_{inlet}} - {h_{outlet,full\_Speed\,n}})\\\;\;\;\;\;\;\;\;\;\; + (1 - SpeedRatio){\mathop m\limits^\cdot_{Speed\,n - 1}}({h_{inlet}} - {h_{outlet,full\_Speed\,n - 1}})\end{array}$$</div>

where,

<span>\({{Q_{coil,SpeedRatio}}}\)</span><sub>                       </sub> = delivered sensible heating capacity at a given speed ratio between two consecutive speeds [W]

<span>\({\mathop {{\rm{ }}m}\limits_{Speed{\kern 1pt} n}^ \cdot  }\)</span>      = air mass flow rate through heating coil at Speed n as set by the parent object [kg/s]

<span>\({\mathop {{\rm{ }}m}\limits_{Speed{\kern 1pt} n - 1}^ \cdot  }\)</span>    = air mass flow rate through heating coil at Speed 1 as set by the parent object [kg/s]



n Average outlet air enthalpy

<div>$${h_{outlet,average}} = {h_{inlet}} - {{\raise0.7ex\hbox{${{Q_{coil,SpeedRatio}}}$} \!\mathord{\left/ {\vphantom {{{Q_{coil,SpeedRatio}}} {\mathop m\limits^\cdot  }}}\right.}\!\lower0.7ex\hbox{${\mathop m\limits^\cdot  }$}}_{inlet}}$$</div>

where,

*h<sub>outlet,average</sub>*  = average specific enthalpy at the coil outlet [J/kg]

*h<sub>inlet</sub>           *            = specific enthalpy of the coil inlet air [J/kg]

<span>\({\dot m_{inlet}}\)</span>         = Mass flow rate at the inlet to the coil as established by the parent object (Ref. AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, Mass Flow Rate Calculation). This flow rate is the average value determined by the parent object, accounting for the specified flow rate when the heating coil is at Speed n and the specified flow rate when the heating coil is at Speed n-1 for the time step being simulated.

n Average outlet air temperature

The heating coil’s outlet air humidity ratio equals the inlet air humidity ratio since the coil does not change the moisture content of the air. So the average outlet air temperature is calculated based on the inlet air humidity ratio and the average outlet air enthalpy using the psychrometric function PsyTdbFnHW.

n Full load energy inputs at Speed n-1 and Speed n

<div>$$HeatingPowe{r_n} = TotCa{p_n}\left( {EI{R_n}} \right)\left( {HeatingCapacityMultiplier} \right)\left( {InputPowerMultiplier} \right)$$</div>

<div>$$HeatingPowe{r_{n - 1}} = TotCa{p_{n - 1}}\left( {EI{R_{n - 1}}} \right)\left( {HeatingCapacityMultiplier} \right)\left( {InputPowerMultiplier} \right)$$</div>

where,

*InputPowerMultiplier*        = Frost adjustment factor for heating power calculation (Ref. Coil:Heating:DX:SingleSpeed)

n Calculate combined energy input

When the input for the field ‘Apply Part Load Fraction to Speeds Greater than 1’ is No (equivalent to a single compressor), the combined energy output is calculated as follows:

<div>$$HeatingPower = HeatingPowe{r_n}\left( {SpeedRatio} \right) + HeatingPowe{r_{n - 1}}(1.0 - SpeedRatio)$$</div>

When the input for the field ’Apply Part Load Fraction to Speeds Greater than 1’ is Yes (equivalent to multiple compressors), the combined energy output is calculated as follows:

<div>$$HeatingPower = HeatingPowe{r_n}\left( {RTF} \right) + HeatingPowe{r_{n - 1}}(1.0 - RTF)$$</div>

where,

HeatingPower          = Power used in Watt

RTF                        = Run time fraction (SpeedRatio/Part-load Fraction) at Speed n

n Calculate defrost power

When the defrost strategy is resistive, the power calculation is the same as Speed 1 operation (Ref. Coil:Heating:DX:SingleSpeed). When the defrost strategy is reverse-cycle, the following calculations are performed:

<div>$${Q_{defrost,n}} = 0.01({t_{frac,defrost}})(7.222 - {T_{db,o}})\left( {\frac{{{Q_{total,rated,n}}}}{{1.01667}}} \right)$$</div>

<div>$${P_{defrost,n - 1}} = DefrostEIRTempModFac\left( {\frac{{{Q_{total,rated,n - 1}}}}{{1.01667}}} \right)({t_{frac,defrost}})$$</div>

<div>$${P_{defrost,n}} = DefrostEIRTempModFac\left( {\frac{{{Q_{total,rated,n}}}}{{1.01667}}} \right)({t_{frac,defrost}})$$</div>

where:

<span>\({Q_{defrost,n}}\)</span>     = additional indoor heating load due to reverse-cycle defrost at Speed n (*W*)

<span>\({Q_{total,rated,n}}\)</span>  = total full-load heating capacity of the coil at rated conditions at Speed n (W)

<span>\({P_{defrost,n - 1}}\)</span>    = full load defrost power for the simulation time step at Speed n-1 (W)

<span>\({P_{defrost,n}}\)</span>      = full load defrost power for the simulation time step at Speed n (W)

<span>\({Q_{total,rated,n - 1}}\)</span>= capacity of the resistive defrost heating element at Speed n-1 (W)

<span>\({Q_{total,rated,n}}\)</span>= capacity of the resistive defrost heating element at Speed n (W)

DefrostEIRTempModFac     = defrost energy input ratio (EIR) modifier curve (Ref. Coil:Heating:DX:SingleSpeed).

T<sub>frac,defrost</sub>       =  fractional defrost time (Ref. Coil:Heating:DX:SingleSpeed)

When the input for the field ‘Apply Part Load Fraction to Speeds Greater than 1’ is No (equivalent to a single compressor), the average defrost power is calculated as follows:

<div>$${P_{defrost}} = {P_{defrost,n}}(SpeedRatio) + {P_{defrost,n - 1}}(1.0 - SpeedRatio)$$</div>

When the input for the field ’Apply Part Load Fraction to Speeds Greater than 1’ is Yes (equivalent to multiple compressors), the combined defrost energy is calculated as follows:

<div>$${P_{defrost}} = {P_{defrost,n}}(RTF) + {P_{defrost,n - 1}}(1.0 - RTF)$$</div>

where,

<span>\({P_{defrost}}\)</span>        = average defrost power used in Watt

RTF            = Run time fraction (SpeedRatio/Part-load Fraction) at Speed n

n Crankcase heater

There is no power need at higher speed operation.

#### Higher Speed Operation with single mode operation

<span style="color:red;">This section describes how higher speed operation is simulated. When the required sensible load is less than the full load sensible capacity at Speed n (Speed Number &gt; 1), its performance is almost the same as the performance for the Coil:Heating:DX:SingleSpeed model at different flow rate and capacity with given Speed n. No liner interpolation is performed between two adjacent speeds.</span>

..............

## Example File and Transition Changes ##

### Example files

Two example files will be provided. The first one represents the outdoor air flow rate change based on the cooling coil outlet setpoint temperature. The second represents single mode operation.

### Transition Changes

UnitarySystemPerformance:Multispeed

When an object of UnitarySystemPerformance:Multispeed is used, the transition is needed by adding a filed of Single Mode Operationwith No choice.

	UnitarySystemPerformance:Multispeed,
	   MyMultispeedHPSpec,      !- Name
	   4,                       !- Number of Speeds for Heating
	   4,                       !- Number of Speeds for Cooling

   <span style="color:red;">No,                      !- Single Mode Operation</span>

	   0.235294118,             !- Heating Speed 1 Supply Air Flow Rate {m3/s}
	   0.235294118,             !- Cooling Speed 1 Supply Air Flow Rate {m3/s}
	   0.470588235,             !- Heating Speed 2 Supply Air Flow Rate {m3/s}
	   0.470588235,             !- Cooling Speed 2 Supply Air Flow Rate {m3/s}
	   0.705882353,             !- Heating Speed 3 Supply Air Flow Rate {m3/s}
	   0.705882353,             !- Cooling Speed 3 Supply Air Flow Rate {m3/s}
	   1.0,                     !- Heating Speed 4 Supply Air Flow Rate {m3/s}
	   1.0;                     !- Cooling Speed 4 Supply Air Flow Rate {m3/s}

##Design Document##

This new feature will revise several modules: SetpointManager, MixedAir, HVACUnitarySystem and DXCoils.

###SetpointManager###

The revision is focused on SetpointManager:MixedAir.

####Add variables in SetpointManager:MixedAir####

Since three new fields are proposed to meet requirements of cooling coil outlet temperature, three new variables are added in the struct DefineMixedAirSetPointManager:

	struct DefineMixedAirSetPointManager
	{
		...
		int CoolCoilInNode; // Cooling coil inlet node number
		int CoolCoilOutNode; // Cooling coil outlet node number
		Real64 MinCoolCoilOutTemp; // The minimum temperature at cooling coil outlet node
		...
	};


####Revise DefineMixedAirSetPointManager::calculate####

When the inputs of cooling coil nodes are given, the mixed air setpoint is calculated based on fan location:

	If draw-through then
		Setpoint = MAX (  (reference node setpoint  + coil depression), (N1 + coil depression)  )
	Else (blow through)
		Setpoint = MAX (  (reference node setpoint – fan rise + coil depression), (N1 – fan rise + coil depression)  )
	End If

It should be pointed out that the fan placement can be found by comparing the temperature difference between the reference node and cooling coil outlet node. When cooling is required and the temperature difference is zero, the fan placement is blow through, and vice versa.


###MixedAir###

The revision is focused on the OA Controller. The maximum amount of outdoor air will be calculated based on the mixed air setpoint when the cooling coil outlet temperature is less than the setpoint temperature. The Maximum Fraction of Outdoor Air in the Controller:OutdoorAir object will be revised based on the maximum amount of outdoor air.

###DXCoils###

In order to perform single mode operation, an optional argument will be added in the SimDXCoilMultiSpeed function.

	void
	SimDXCoilMultiSpeed(
		std::string const & CompName, // name of the fan coil unit
		Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) /
		Real64 const CycRatio, // cycling part load ratio for variable speed
		int & CompIndex,
		Optional_int_const SpeedNum, // Speed number for multispeed cooling coil onlyn
		Optional_int_const FanOpMode, // Fan operation mode
		Optional_int_const CompressorOp, // Compressor on/off; 1=on, 0=off

<span style="color:red;">Optional_bool_const SingleModeFlag // Single mode operation </span>

	)

The additional argument will be carried by two sub-functions: CalcMultiSpeedDXCoilCooling and CalcMultiSpeedDXCoilHeating.

	void
	CalcMultiSpeedDXCoilCooling(
		int const DXCoilNum, // the number of the DX heating coil to be simulated
		Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) / (CompressorSpeedMax - CompressorSpeedMin)
		Real64 const CycRatio, // cycling part load ratio
		int const SpeedNum, // Speed number
		int const FanOpMode, // Sets fan control to CycFanCycCoil or ContFanCycCoil
		int const CompressorOp, // Compressor on/off; 1=on, 0=off
<span style="color:red;">Optional_bool_const SingleModeFlag // Single mode operation </span>

	)

	void
	CalcMultiSpeedDXCoilHeating(
		int const DXCoilNum, // the number of the DX heating coil to be simulated
		Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) / (CompressorSpeedMax - CompressorSpeedMin)
		Real64 const CycRatio, // cycling part load ratio
		int const SpeedNum, // Speed number
		int const FanOpMode // Fan operation mode
<span style="color:red;">Optional_bool_const SingleModeFlag // Single mode operation </span>

	)

The calculation procedures are provided below:

If ( SingleModeFlag ) {

	SpeedRatio = 1.0
	SpeedNumer is given and fixed
	CycRatio varies

Load = SensibleCapacity(SpeedNume) * CycRatio

}

Note: When the speed number is equal to 1, the original calculation is kept. When the load is greater than the capacity of the highest speed, no change will be made.

###AirLoopHVAC:UnitarySystem###

The main purpose is to read a new optional field of single mode operation and pass it into the sub-functions.

An additional field of Single Mode Operation will be added in the UnitarySystemPerformance:Multispeed object to accommodate the single mode operation. If this field is entered as Yes, a single mode operation is required with the fixed speed number. The performance is similar to a single capacity cooling coil whose capacity is determined by the speed number. In this way, the fan power calculation can be achieved by using exponent curve based on fan flow ratio.

Based on the additional field, a bool variable is added in the struct UnitarySystemData

	struct UnitarySystemData
	{
		....
		bool SingleModeFlag; // Single mode operation
		....
	};

####ControlUnitarySystemOutput####

One more dimension is added in the Par variable to have information on single mode operation.

				Par( 12 ) = 0.0; // FLAG, 0.0 with No single mode, 1.0 with single mode operation

####CalcUnitarySystemLoadResidual####

Add an optional argument to represent a single mode operation

		CalcUnitarySystemToLoad( UnitarySysNum, AirLoopNum, FirstHVACIteration, CoolPLR, HeatPLR, OnOffAirFlowRatio, SensOutput, LatOutput, HXUnitOn, _, _, CompressorOp, SingleModeFlag );

The optional argument of SingleModeFlag will be passed into the following two functions: CalcUnitaryCoolingSystem and CalcUnitaryHeatingSystem. The optional argument is only valid for two type of coils: CoilDX_MultiSpeedCooling and CoilDX_MultiSpeedHeating. When these two functions call a function of SimDXCoilMultiSpeed in the DXCoils, the optional argument is passed in the SimDXCoilMultiSpeed function to perform single mode operation with given speed number.


## References ##

PNNL ENergyPlus input file: ASHRAE90.1_RetailStandalone_STD2013_Chicago.idf

Reid Hart, PE; Rahul Athalye; Weimin Wang, Ph.D, 2013. "Improving Simulation of Outside Air Economizer and Fan Control for Unitary Air Conditioners," DE-13-C058, ASHRAE Transactions
