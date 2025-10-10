

4.1.3. Increase Speeds in UnitarySystemPerformance:Multispeed object
================

**Richard Raustad, Florida Solar Energy Center**

 - NFP Initial draft submitted April 24, 2017
 - NFP Initial draft accepted April 27, 2017
 - NFP Final: submitted May 1, 2017 (same as draft)
 - Design Doc: submitted May 1, 2017

## Draft NFP ##

## Justification for New Feature ##

Many commercial large capacity RTUs (30-200 tons) offer from 2 to 8 stages of compression. For the constant volume version of these products the indoor fan typically has 1 or 2 fan speeds. For the variable air volume version of these products, indoor fan speed is continuously variable when a VFD is used. The use of larger number of compression stages allows finer control of capacity to match load, providing better temperature and humidity control in the space. Use of larger number of compression stages also can achieve improved equipment COP at part-load conditions.

The ability to model up to 8 stages of compression with independent modulation of indoor fan will allow performance of these higher IEER products to be evaluated in EnergyPlus.


## E-mail and  Conference Call Conclusions ##

E-mail notes: no comments received other than initial draft looks acceptable


## Overview ##

The following IDD object (shortened for readability) is extensible yet includes only 4 fields for cooling and heating air flow rate ratios. This means that any user wanting to simulate more than for stages of heating and/or cooling must revised the IDD. Simlarly, the IDFEditor also required fields in the IDD to encompass the number of inputs specified in the object. For these reasons, the UnitarySystemPerformance:Multispeed object fields will be increased to include 10 speeds.

Also, there is no input for no load air flow rate ratio. If a system is autosized, there is no input for the ratio of no load operating air flow rate to maximum operating air flow rate and the result of autosizing is that the no load flow rate always equals the maximum operating air flow rate (which is unrealistic).

This makes it difficult for users and interface developers to use HVAC systems with more than 4 stages.

    UnitarySystemPerformance:Multispeed,
         \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
    A1,  \field Name
         \required-field
    N1,  \field Number of Speeds for Heating
         \required-field
         \minimum 0
         \maximum 10
    N2,  \field Number of Speeds for Cooling
         \required-field
         \minimum 0
         \maximum 10
    A2,  \field Single Mode Operation
    N3,  \field Heating Speed 1 Supply Air Flow Ratio
         \required-field
    N4,  \field Cooling Speed 1 Supply Air Flow Ratio
         \required-field
    N5,  \field Heating Speed 2 Supply Air Flow Ratio
    N6,  \field Cooling Speed 2 Supply Air Flow Ratio
    N7,  \field Heating Speed 3 Supply Air Flow Ratio
    N8,  \field Cooling Speed 3 Supply Air Flow Ratio
    N9,  \field Heating Speed 4 Supply Air Flow Ratio
    N10; \field Cooling Speed 4 Supply Air Flow Ratio


## Approach ##

The team will:

 - add a new field for No Load Supply Air Flow Rate Ratio
 - modify the IDD to include up to 10 fields for heating/cooling supply air flow ratio
 - revise function GetUnitarySystemInput to allow optional fields up to speed 10

## Known Issues ##

- none

## Testing/Validation/Data Sources ##

Compare simulation results with existing equipment models. Document comparison of outlet temperature and humidity ratio and power consumption.

## Input Description ##

Limited changes to IDD and IO Reference documents as described herein.

## Outputs Description ##

No change to output reporting (although report variables of type `Speed N` will be reviewed)

## IDD - Input Data Dictionary ##

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
    A2 , \field Single Mode Operation
       \type choice
       \key Yes
       \key No
       \default No
       \note Controls coil operation during each HVAC timestep.
       \note This choice does not apply to speed 1 operation.
       \note Yes = operate at the highest speed possible without exceeding the current load.
       \note No = allow operation at the average of two adjacent speeds to match the current load.

###New Field:###

    N3 , \field No Load Supply Air Flow Rate Ratio
       \type real
       \default 1.0
       \note Used to define no load operating air flow rate when system fan is specified to
       \note operate continuously.
<New Field>

    N4 , \field Heating Speed 1 Supply Air Flow Ratio
       \required-field
       \type real
       \autosizable
       \minimum> 0
       \begin-extensible
       \note Used only for Multi speed coils
       \note Enter the lowest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N5 , \field Cooling Speed 1 Supply Air Flow Ratio
       \required-field
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the lowest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N6 , \field Heating Speed 2 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N7 , \field Cooling Speed 2 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N8 , \field Heating Speed 3 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N9 , \field Cooling Speed 3 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N10, \field Heating Speed 4 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N11, \field Cooling Speed 4 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.

###New Fields:###

    N12, \field Heating Speed 5 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N13, \field Cooling Speed 5 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N14, \field Heating Speed 6 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N15, \field Cooling Speed 6 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N16, \field Heating Speed 7 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N17, \field Cooling Speed 7 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N18, \field Heating Speed 8 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N19, \field Cooling Speed 8 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N20, \field Heating Speed 9 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N21, \field Cooling Speed 9 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N22, \field Heating Speed 10 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during heating
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.
    N23; \field Cooling Speed 10 Supply Air Flow Ratio
       \type real
       \autosizable
       \minimum> 0
       \note Used only for Multi speed coils
       \note Enter the next highest operating supply air flow ratio during cooling
       \note operation or specify autosize. This value is the ratio of air flow
       \note at this speed to the maximum air flow rate.


## Input Output Reference Documentation ##

Add new field descriptions.

## Engineering Reference ##

Review and add model calculations as necessary.

## Example File and Transition Changes ##

Transition required.

## References ##

NA

## Final NFP ##
Same as draft NFP

## Design Doc ##

    1) Minimal new code anticipated for this feature
    2) Extend number of speeds in IDD to 10. See suggested IDD in draft NFP.
    3) Test that number of speeds < 10 works with modified IDD
    4) Test input file without UnitarySystemPerformance:Multispeed object (should already work).
    5) Test use of UnitarySystemPerformance:Multispeed object with 10-spd DX coil object

