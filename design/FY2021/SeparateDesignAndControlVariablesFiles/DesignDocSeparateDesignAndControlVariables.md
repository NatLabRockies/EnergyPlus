




Separate Design and Control Variables
================

**Jermy Thomas, Dareum Nam, NREL.**


## Table of Contents ##

[Justification for New Feature](#justification-for-new-feature)

[E-mail, Conference Call, Other Communications and Conclusions](#e-mail-conference-call-other-communications-and-conclusions)

[Approach/Timeline/Design Rationale](#approachtimelinedesign-rationale)

[Testing/Validation/Data Sources](#testingvalidationdata-sources)

[Input Output Reference Documentation](#input-output-reference-documentation)

[Engineering Reference](#engineering-reference)

[Example File and Transition Changes](#example-file-and-transition-changes)

[References](#references)

[Next Steps](#next-steps)

[Changes to IDD file ](#changes-to-idd-file )

## Justification for New Feature ##

LowTemperatureRadiant and BaseBoard objects in EnergyPlus currently has design and control fields lumped together. This results in a large number of fields for these objects as seen below:
 - The `ZoneHVAC:LowTemperatureRadiant:VariableFlow` object has 34 fields
 - The `ZoneHVAC:LowTemperatureRadiant:ConstantFlow` object has 34 fields
 - The `ZoneHVAC:Baseboard:RadiantConvective:Water` object has 17+ fields*
 - The `ZoneHVAC:Baseboard:RadiantConvective:Steam` object has 17+ fields*

 Lumping of design and control parameters together results in many of the variables being repeated for every zone which could have been instead shared among the zones. Lumping everything together is also less organized and causes confusion for the users. There have been requests from users to split these objects into design and control variables.

Splitting the objects into design and control variables may result in

 1. Increased ease of creating new objects, with fewer fields to fill in.
 2. Decreased confusion from too many fields. This may also result in fewer errors caused by decreasing the many variables.

 (*The number of fields will increase with the number of surfaces)

## E-mail, Conference Call, Other Communications and Conclusions ##

University of Illinois at Urbana-Champaign and Center for the Built Environment(CBE) at the University of California Berkeley requests this new feature. The original request is like below:

"Potentially separate the lowtemp:radiant objects such that one main object contains the design parameters of the radiant system (e.g. amount of tubing, hot and chilled water loop connections), while the other contains the control parameters (e.g. two position, modulating, zone circulator pump, etc.)." (Quote from CBE)   Right now, all of the parameters associated with a radiant system in EnergyPlus are contained in a single input syntax.  The concern here is that much of the control information is probably pretty similar from system to system within a single user input file.  So, there could be less work and smaller files if the input was broken up into two separate inputs, allowing many radiant systems to re-use a single control definition.  This could potentially be applied to other input syntax beyond the low temperature radiant systems.

Conversation with E+ Team on Slack:
1. Create a design object for each radiant temp and baseboard object.
2. Maybe name the design object to something else.

## Approach/Timeline/Design Rationale ##

### Timeline:


 - Design document:
	 - Send for reviews------------------ Nov 18 2020
	 - Finalize design document-------- Dec x 2020
 - Implementation:
	 - First draft of Implementation---- Jan 31 2021
	 - Final Implementation------------- Feb 28 2021
 - Testing:
	 - Testing done, bugs worked out--- Mar 30 2021
 - Buffer: 1 month

##### Comment #####
Project to end by April 2021 since Jermy's internship ends by May 2021.

### Approach/Design Rationale ###

The proposed approach is to identify the design variables in the LowTemperatureRadiant and Baseboard objects and create separate design objects for the LowTemperatureRadiant and Baseboard objects.

<span style="display:block;text-align:center">![How the objects will be split](https://github.com/NREL/EnergyPlus/blob/Separate-Design-and-Control-Variables/design/FY2021/SeparateDesignAndControlVariablesFiles/BigSplit.jpg)

<span style="display:block;text-align:center">**Figure: How the objects will be split**

<span style="display:block;text-align:center">![How the design objects will be brokendown](https://github.com/NREL/EnergyPlus/blob/Separate-Design-and-Control-Variables/design/FY2021/SeparateDesignAndControlVariablesFiles/BKDwn1.jpg)

<span style="display:block;text-align:center">![How the design objects will be brokendown](https://github.com/NREL/EnergyPlus/blob/Separate-Design-and-Control-Variables/design/FY2021/SeparateDesignAndControlVariablesFiles/BKDwn2.jpg)

<span style="display:block;text-align:center">![How the design objects will be brokendown](https://github.com/NREL/EnergyPlus/blob/Separate-Design-and-Control-Variables/design/FY2021/SeparateDesignAndControlVariablesFiles/BKDwn3.jpg)

<span style="display:block;text-align:center">**Figure: How the design objects will be brokendown**

## Testing/Validation/Data Sources ##

Regression tests will be done to ensure that no changes to the current models have been made.

## Input Output Reference Documentation ##

### Inputs Description ###

The new objects will have the fields that were removed from the old/modified objects.

#### ZoneHVAC:LowTemperatureRadiant Objects

##### *New object* - `ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design` ####

This `ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design` object is referenced in the `ZoneHVAC:LowTemperatureRadiant:VariableFlow`  object. Multiple `ZoneHVAC:LowTemperatureRadiant:VariableFlow`  objects can be mapped to a single `ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design` object.

*Field: Name*
*Field: Fluid to Radiant Surface Heat Transfer Model*
*Field: Hydronic Tubing Inside Diameter*
*Field: Hydronic Tubing Outside Diameter*
*Field: Hydronic Tubing Conductivity*
*Field: Temperature Control Type*
*Field: Setpoint Control Type*
*Field: Heating Design Capacity Method*
*Field: Heating Design Capacity Per Floor Area*
*Field: Fraction of Autosized Heating Design Capacity*
*Field: Heating Control Throttling Range*
*Field: Heating Control Temperature Schedule Name*
*Field: Cooling Design Capacity Method*
*Field: Cooling Design Capacity Per Floor Area*
*Field: Fraction of Autosized Cooling Design Capacity*
*Field: Cooling Control Throttling Range*
*Field: Cooling Control Temperature Schedule Name*
*Field: Condensation Control Type*
*Field: Condensation Control Dewpoint Offset*
*Field: Changeover Delay Time Period Schedule*

##### *Modified object* - `ZoneHVAC:LowTemperatureRadiant:VariableFlow` ####

All of the fields in the new `ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design` will be removed from the old `ZoneHVAC:LowTemperatureRadiant:VariableFlow` object. Additionally, we add the following field and description:

*Field: Design object*

This field cannot be blank, and it should point to one  `ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design` object.

##### *New object* - `ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design` ####

This `ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design` object is referenced in the `ZoneHVAC:LowTemperatureRadiant:ConstantFlow`  object. Multiple `ZoneHVAC:LowTemperatureRadiant:ConstantFlow`  objects can be mapped to a single `ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design` object.

*Field: Name*
*Field: Fluid to Radiant Surface Heat Transfer Model*
*Field: Hydronic Tubing Inside Diameter*
*Field: Hydronic Tubing Outside Diameter*
*Field: Hydronic Tubing Conductivity*
*Field: Temperature Control Type*
*Field: Running Mean Outdoor Dry-Bulb Temperature Weighting Factor*
*Field: Motor Efficiency*
*Field: Fraction of Motor Inefficiencies to Fluid Stream*
*Field: Condensation Control Type*
*Field: Condensation Control Dewpoint Offset*
*Field: Changeover Delay Time Period Schedule*

##### *Modified object* - `ZoneHVAC:LowTemperatureRadiant:ConstantFlow` ####

All of the fields in the new `ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design` will be removed from the  old`ZoneHVAC:LowTemperatureRadiant:ConstantFlow` object. Additionally, we add the following field and description:

*Field: Design object*

This field cannot be blank, and it should point to one  `ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design` object.

#### ZoneHVAC:Baseboard:RadiantConvective Objects

##### *New object* - `ZoneHVAC:Baseboard:RadiantConvective:Water:Design` ####

This `ZoneHVAC:Baseboard:RadiantConvective:Water:Design` object is referenced in the `ZoneHVAC:Baseboard:RadiantConvective:Water` object and provides additional design parameters to it. Multiple `ZoneHVAC:Baseboard:RadiantConvective:Water` can be mapped to a single `ZoneHVAC:Baseboard:RadiantConvective:Water:Design` object.

*Field: Name*
*Field: Fraction Radiant*
*Field: Fraction of Radiant Energy Incident on People*
*Field: Heating Design Capacity Method*
*Field: Heating Design Capacity Per Floor Area {W/m2}*
*Field: Fraction of Autosized Heating Design Capacity*
*Field: Convergence Tolerance*

##### *Modified objects* - `ZoneHVAC:Baseboard:RadiantConvective:Water` ####

All of the fields in the new `ZoneHVAC:Baseboard:RadiantConvective:Water:Design` will be removed from the old `ZoneHVAC:Baseboard:RadiantConvective:Water` object. Additionally, we add the following field and description:

*Field: Design object*

This field cannot be blank, and it should point to one `ZoneHVAC:Baseboard:RadiantConvective:Water:Design` object.


##### *New object* - `ZoneHVAC:Baseboard:RadiantConvective:Steam:Design` ####

This `ZoneHVAC:Baseboard:RadiantConvective:Steam:Design` object is referenced in the `ZoneHVAC:Baseboard:RadiantConvective:Steam` object and provides additional design parameters to it. Multiple `ZoneHVAC:Baseboard:RadiantConvective:Steam` can be mapped to a single `ZoneHVAC:Baseboard:RadiantConvective:Steam:Design` object.

*Field: Name*
*Field: Fraction Radiant*
*Field: Fraction of Radiant Energy Incident on People*
*Field: Heating Design Capacity Method*
*Field: Heating Design Capacity Per Floor Area {W/m2}*
*Field: Fraction of Autosized Heating Design Capacity*
*Field: Convergence Tolerance*

##### *Modified objects* - `ZoneHVAC:Baseboard:RadiantConvective:Steam` ####

All of the fields in the new `ZoneHVAC:Baseboard:RadiantConvective:Steam:Design` will be removed from the old `ZoneHVAC:Baseboard:RadiantConvective:Steam` object. Additionally, we add the following field and description:

*Field: Design object*

This field cannot be blank, and it should point to one `ZoneHVAC:Baseboard:RadiantConvective:Steam:Design` object.

### Outputs Description ###

No changes.

## Engineering Reference ##

No changes.

## Example File and Transition Changes ##

All of the current example files that use `ZoneHVAC:LowTemperatureRadiant:VariableFlow`, `ZoneHVAC:LowTemperatureRadiant:ConstantFlow` and `ZoneHVAC:Baseboard:RadiantConvective:Water`, `ZoneHVAC:Baseboard:RadiantConvective:Steam` will be modified so that the design and parameters are different.

Appropriate changes will be made to the Fortran transition InputRulesFiles.

## References ##

None.

## Next Steps ##

 1. Get feedback from the EnergyPlus team regarding naming the design object to something else.
 2. Start coding.

## Proposed changes to the IDD file ##

The metadata to remain same, so it was taken out to improve readability.

### LowTemperatureRadiant:VariableFlow Objects ###

**ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design**

    A1 , \field Name
    A2 , \field Fluid to Radiant Surface Heat Transfer Model
    N1 , \field Hydronic Tubing Inside Diameter
    N2 , \field Hydronic Tubing Outside Diameter
    N3 , \field Hydronic Tubing Conductivity
    A3 , \field Temperature Control Type
    A4 , \field Setpoint Control Type
    A5 , \field Heating Design Capacity Method
    N4 , \field Heating Design Capacity Per Floor Area
    N5 , \field Fraction of Autosized Heating Design Capacity
    N6 , \field Heating Control Throttling Range
    A6 , \field Heating Control Temperature Schedule Name
    A7 , \field Cooling Design Capacity Method
    N7 , \field Cooling Design Capacity Per Floor Area
    N8 , \field Fraction of Autosized Cooling Design Capacity
    N9 , \field Cooling Control Throttling Range
    A8 , \field Cooling Control Temperature Schedule Name
    A9 , \field Condensation Control Type
    N10, \field Condensation Control Dewpoint Offset
    A10; \field Changeover Delay Time Period Schedule

**ZoneHVAC:LowTemperatureRadiant:VariableFlow**

    A1 , \field Name
    A2 , \field Design Object Name
    A3 , \field Availability Schedule Name
    A4 , \field Zone Name
    A5 , \field Surface Name or Radiant Surface Group Name
    N1 , \field Hydronic Tubing Length
    N2 , \field Heating Design Capacity
    N3 , \field Maximum Hot Water Flow
    A6 , \field Heating Water Inlet Node Name
    A7 , \field Heating Water Outlet Node Name
    N4 , \field Cooling Design Capacity
    N5 , \field Maximum Cold Water Flow
    A8 , \field Cooling Water Inlet Node Name
    A9 , \field Cooling Water Outlet Node Name
    A10, \field Number of Circuits
    N6 ; \field Circuit Length

### LowTemperatureRadiant:ConstantFlow Objects #####

**ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design**

       A1 , \field Name
       A2 , \field Fluid to Radiant Surface Heat Transfer Model
       N1 , \field Hydronic Tubing Inside Diameter
       N2 , \field Hydronic Tubing Outside Diameter
       N3 , \field Hydronic Tubing Conductivity
       A3 , \field Temperature Control Type
       N4 , \field Running Mean Outdoor Dry-Bulb Temperature Weighting Factor
       N5 , \field Motor Efficiency
       N6 , \field Fraction of Motor Inefficiencies to Fluid Stream
       A4 , \field Condensation Control Type
       N7 , \field Condensation Control Dewpoint Offset
       A5 ; \field Changeover Delay Time Period Schedule

**ZoneHVAC:LowTemperatureRadiant:ConstantFlow**

       A1 , \field Name
       A2 , \field Design Object Name
       A3 , \field Availability Schedule Name
       A4 , \field Zone Name
       A5 , \field Surface Name or Radiant Surface Group Name
       N1 , \field Hydronic Tubing Length
       N2 , \field Rated Flow Rate
       A6 , \field Pump Flow Rate Schedule Name
       N3 , \field Rated Pump Head
       N4 , \field Rated Power Consumption
       A7 , \field Heating Water Inlet Node Name
       A8 , \field Heating Water Outlet Node Name
       A9 , \field Heating High Water Temperature Schedule Name
       A10, \field Heating Low Water Temperature Schedule Name
       A11, \field Heating High Control Temperature Schedule Name
       A12, \field Heating Low Control Temperature Schedule Name
       A13, \field Cooling Water Inlet Node Name
       A14, \field Cooling Water Outlet Node Name
       A15, \field Cooling High Water Temperature Schedule Name
       A16, \field Cooling Low Water Temperature Schedule Name
       A17, \field Cooling High Control Temperature Schedule Name
       A18, \field Cooling Low Control Temperature Schedule Name
       A19, \field Number of Circuits
       N5 ; \field Circuit Length


### Baseboard:RadiantConvective:Water Objects ####

**ZoneHVAC:Baseboard:RadiantConvective:Water:Design**

      A1 ,  \field Name
      A2 ,  \field Heating Design Capacity Method
      N1 ,  \field Heating Design Capacity Per Floor Area
      N2 ,  \field Fraction of Autosized Heating Design Capacity
      N3 ,  \field Convergence Tolerance
      N4 ,  \field Fraction Radiant
      N5 ;  \field Fraction of Radiant Energy Incident on People

**ZoneHVAC:Baseboard:RadiantConvective:Water**

      A1, \field Name
      A2, \field Design Object Name
      A3, \field Availability Schedule Name
      A4, \field Inlet Node Name
      A5, \field Outlet Node Name
      N1, \field Rated Average Water Temperature
      N2, \field Rated Water Mass Flow Rate
      N3, \field Heating Design Capacity
      N4, \field Maximum Water Flow Rate
      A6, \field Surface 1 Name
      N5, \field Fraction of Radiant Energy to Surface 1
      A7, \field Surface 2 Name
      N6, \field Fraction of Radiant Energy to Surface 2
    .
    .
    .
     A105, \field Surface 100 Name
     N104; \field Fraction of Radiant Energy to Surface 100

### Baseboard:RadiantConvective:Steam Objects ####

**ZoneHVAC:Baseboard:RadiantConvective:Steam:Design**

      A1 ,  \field Name
      A2 ,  \field Heating Design Capacity Method
      N1 ,  \field Heating Design Capacity Per Floor Area
      N2 ,  \field Fraction of Autosized Heating Design Capacity
      N3 ,  \field Convergence Tolerance
      N4 ,  \field Fraction Radiant
      N5 ;  \field Fraction of Radiant Energy Incident on People

**ZoneHVAC:Baseboard:RadiantConvective:Steam**

      A1,  \field Name
      A2,  \field Design Object Name
      A3,  \field Availability Schedule Name
      A4,  \field Inlet Node Name
      A5,  \field Outlet Node Name
      N1,  \field Heating Design Capacity
      N2,  \field Degree of SubCooling
      N3,  \field Maximum Steam Flow Rate
      A6,  \field Surface 1 Name
      N4,  \field Fraction of Radiant Energy to Surface 1
      A7,  \field Surface 2 Name
      N5,  \field Fraction of Radiant Energy to Surface 2
    .
    .
    .
     A105, \field Surface 100 Name
     N103; \field Fraction of Radiant Energy to Surface 100
