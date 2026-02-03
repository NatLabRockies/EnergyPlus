Plant Equipment Autosizing Gaps
================

**B. Griffith, Energy Archmage Company, for SEI/TRANE**

 - first draft 6/20/2016
 - second draft 7/5/2016, with changes from review.
 - third draft 7/11/2016, add new input field for turbine eff

 ## Justification for New Feature ##

 The ability to autosize input is a very powerful and important capability in EnergyPlus.  However there remain gaps where fields are not yet autosizable but they should be.  This contribution will fill some gaps by making it possible to autosize ten existing fields across four plant component models. Addressing gaps involving missing autosize capabilities has been a long-standing request by a major interface developer. Users and interface developers who apply EnergyPlus for design sizing and model diverse plant systems will benefit from the ability to autosize these fields.

 ## E-mail and  Conference Call Conclusions ##

 Discussed and approved during sizing subgroup conference call on 6-22-2016.  Use "Reference" instead of "Rated" to rename fields in heat pump objects.

 ## Overview ##

This contribution will make it possible to autosize the following ten fields in four different objects:

     Chiller:CombustionTurbine,
       N45, \field Design Heat Recovery Water Flow Rate  >> need autosizable

     Chiller:EngineDriven,
       N26, \field Design Heat Recovery Water Flow Rate  >> need autosizable

     HeatPump:WaterToWater:EquationFit:Cooling and
     HeatPump:WaterToWater:EquationFit:Heating
       N1,  \field Rated Load Side Flow Rate  >> need autosizable
       N2,  \field Rated Source Side Flow Rate  >> need autosizable
       N3,  \field Rated Cooling Capacity  >> need autosizable
       N4,  \field Rated Cooling Power Consumption need autosizable

The project will not modify the simulation models beyond sizing calculations.  It is expected that additional input fields will need to be added to provide data for sizing calculations and to provide control over scaling of the sizing results.


## Approach ##

Calculations for the design heat recovery flow rate for the two chillers will be added to their respective sizing routines.  A new input field will be added to each to allow scaling the flow rate and capacity of heat recovery section relative to the condenser, as is already being done for other chillers.

The combustion turbine chiller will add a new field for turbine efficiency for scalable Gas Turbine Engine Capacity when autosizing.

The equation fit water to water heat pumps will be modified to add a new sizing routine to calculate sizes for the four main design values, flow rates.  New input for a nominal COP will be added to scale design power from design capacity, because the models as formulated have no input for efficiency, effectiveness, COP, etc.  New Sizing factor fields will be added to scale each component as a fraction of the overall loop sizing result.

The implementation will follow current code practice in the affected modules; no significant refactoring is planned.

## Testing/Validation/Data Sources ##

Autosize input will be compared to equivalent hard size input to verify.

## Input Output Reference Documentation ##

Draft I/O ref changes have been made in the document source file called "group-plant-equipment.tex."  Those input fields with changes are excerpted below.

###HeatPump:WaterToWater:EquationFit:Cooling

####1.22.16.1.6 Field: Reference Load Side Flow Rate
This numeric field contains the design volume flow rate on the load side of the heat pump in
m3/s. This corresponds to the highest load side heat transfer rate listed in the catalog data. This
field is autosizable.

####1.22.16.1.7 Field: Reference Source Side Flow Rate
This numeric field contains the design volume flow rate on the source side of the heat pump in
m3/s. This corresponds to the highest load side heat transfer rate listed in the catalog data. This
field is autosizable.

####1.22.16.1.8 Field: Reference Cooling Capacity
This numeric field contains the design cooling capacity of the heat pump in W. This corresponds
to the highest load side heat transfer rate listed in the catalog data. This field is autosizable.

####1.22.16.1.9 Field: Reference Cooling Power Consumption

This numeric field is the design electric power consumption for cooling, in W. This corresponds
to the electic power use at the Design Cooling Capacity. This field is autosizable. When autosized,
the field called Reference Coefficient of Performance must be used.

####1.22.16.1.12 Field: Reference Coefficient of Performance
This field is required if the Reference Cooling Power Consumption is set to autosize. The nominal
COP is defined by the Reference Cooling Capacity divided by the corresponding Reference Cooling Power
Consumption and is non-dimensional. This field is only used for sizing; if the Reference Cooling
Power Consumption is set to a fixed value then COP of the component during simulation will be
determined by the ratio of Reference Cooling Capacity divided by the corresponding Reference Cooling
Power Consumption and not by the value in this field.

####1.22.16.1.13 Field: Sizing Factor
This optional numeric field allows the user to specify a sizing factor for this component. The
sizing factor is used when the component design inputs are autosized: the autosizing calculations
are performed as usual and the results are multiplied by the sizing factor. For this component
the inputs that would be altered by the sizing factor are: Reference Load Side Flow Rate, Reference
Source Side Flow Rate, Reference Cooling Capacity, and Reference Cooling Power Consumption. The
Sizing Factor allows the user to size a component to meet part of the plant loop's design load while
continuing to use the autosizing feature. For example if there are two heat pumps on the supply
side, each one could be sized to be half of the design load.


###HeatPump:WaterToWater:EquationFit:Heating
Changed and new input fields

####1.22.17.1.6 Field: Reference Load Side Flow Rate
This numeric field contains the design volume flow rate on the load side of the heat pump in
m3/s. This corresponds to the highest load side heat transfer rate listed in the catalog data. This
field is autosizable.

####1.22.17.1.7 Field: Reference Source Side Flow Rate
This numeric field contains the design volume flow rate on the source side of the heat pump in
m3/s. This corresponds to the highest load side heat transfer rate listed in the catalog data. This
field is autosizable.

####1.22.17.1.8 Field: Reference Heating Capacity
This numeric field contains the design heating capacity of the heat pump in W. This corresponds
to the highest load side heat transfer rate listed in the catalog data. This field is autosizable.

####1.22.17.1.9 Field: Reference Heating Power Consumption
This numeric field contains the design electric power consumption for heating, in W. This
corresponds to the electic power use at the Design Heating Capacity. This field is autosizable.
When autosized, the field called Reference Coefficient of Performance must be used.

####1.22.17.1.12 Field: Reference Coefficient of Performance
This field is required if the Reference Heating Power Consumption is set to autosize. The nominal
COP is defined by the Reference Heating Capacity divided by the corresponding Reference Heating Power
Consumption and is non-dimensional. This field is only used for sizing; if the Reference Heating
Power Consumption is set to a fixed value then COP of the component during simulation will be
determined by the ratio of Reference Heating Capacity divided by the corresponding Reference Heating
Power Consumption and not by the value in this field.

####1.22.17.1.13 Field: Sizing Factor
This optional numeric field allows the user to specify a sizing factor for this component. The
sizing factor is used when the component design inputs are autosized: the autosizing calculations
are performed as usual and the results are multiplied by the sizing factor. For this component
the inputs that would be altered by the sizing factor are: Reference Load Side Flow Rate, Reference
Source Side Flow Rate, Reference Heating Capacity, and Reference Heating Power Consumption. The
Sizing Factor allows the user to size a component to meet part of the plant loop's design load while
continuing to use the autosizing feature. For example if there are two heat pumps on the supply
side, each one could be sized to be half of the design load.

###Chiller:CombustionTurbine

####1.22.10.1.60 Field: Design Heat Recovery Water Flow Rate
This optional numeric field is the design heat recovery plant fluid flow rate, if the heat recovery
option is being simulated. If this value is greater than 0.0, or autosize, then a heat recovery loop
must be specified and attached to the chiller using the next two node input fields. The units are
in cubic meters per second. This field is autosizable. When autosizing, the flow rate is simply
the product of the design condenser flow rate and the Condenser Heat Recovery Relative Capacity
Fraction set in the field below.

####1.22.10.1.70 Field: Condenser Heat Recovery Relative Capacity Fraction
This field is optional. It can be used to describe the physical size of the heat recovery portion of
a split bundle condenser section. This fraction describes the relative capacity of the heat recovery
bundle of a split condenser compared to the nominal, full load heat rejection rate of the chiller.
This fraction will be applied to the full heat rejection when operating at nominal capacity and
nominal COP to model a capacity limit for the heat rejection. If this field is not entered then the
capacity fraction is set to 1.0.


####1.22.10.1.71 Field: Turbine Engine Efficiency

This optional field is the nominal turbine engine efficiency and is used when Gas Turbine Engine Capacity
is set to Autosize. Default of 0.35.


###Chiller:EngineDriven

####1.22.9.1.42 Field: Design Heat Recovery Water Flow Rate
This optional numeric field is the design heat recovery plant fluid flow rate, if the heat recovery
option is being simulated. If this value is greater than 0.0, or autosize, then a heat recovery loop
must be specified and attached to the chiller using the next two node input fields. The units are
in cubic meters per second. This field is autosizable. When autosizing, the flow rate is simply
the product of the design condenser flow rate and the Condenser Heat Recovery Relative Capacity
Fraction set in the field below.

####1.22.9.1.51 Field: Condenser Heat Recovery Relative Capacity Fraction
This field is optional. It can be used to describe the physical size of the heat recovery portion of
a split bundle condenser section. This fraction describes the relative capacity of the heat recovery
bundle of a split condenser compared to the nominal, full load heat rejection rate of the chiller.
This fraction will be applied to the full heat rejection when operating at nominal capacity and
nominal COP to model a capacity limit for the heat rejection. If this field is not entered then the
capacity fraction is set to 1.0.

## Input Description ##

see I/O ref.

## Outputs Description ##

  No new output variables.  Some new Sizing Reports will be generated when the new autosizable capabilities are used.

## Engineering Reference ##

Sizing calculation documentation will be expanded to include these new autosize fields.

## Example File and Transition Changes ##

Planning to change some existing field names to use the word "Reference" instead of "Rated" in the simple equation fit water to water heat pump models.  For the EnergyPlus team, these will just get picked up by the IDD change and not really require Transition coding.


## References ##

none
