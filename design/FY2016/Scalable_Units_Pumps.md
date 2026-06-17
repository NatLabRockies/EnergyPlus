#Scalable Units for Pump Sizing Calculations#

B. Griffith

Energy Archmage Company, for SEI/Trane.

23 October, 2015
revised 26 October, 2015

##Justification for Feature Update :

Scalable units for HVAC component models allow users and interface developers to better control the outcomes from autosizing.  The general idea is to add new fields to input objects that expose parameters used to scale sizes.  The design electrical power of pumps is currently scaled from a single assumed constant (0.78 ((m3/s)-Pa)/W) for impeller efficiency that is hard-coded inside the program.  There is no way for users and interface developers to assemble a library of pumps that have varying levels of efficiency that can be used with autosizing.  Exposing the scaling factor as a new user input, while retaining the original assumed efficiency as a default for that input, is justified because it will retain legacy behavior while also allowing users and interface developers to specify pump efficiency without having to use hard sizes.

During early-phase design, the plant design has not progressed far enough for the modeler to have adequate data for pump head and motor efficiency.  Therefore an alternative sizing method that does not use pressure and motor efficiency is justified.

This is part of a long-standing request for scalable units from a major interface developer.

##Overview:
This proposal involves modest changes to all five existing pump objects. A switch will be added to provide two different methods for how the pump's power will be determined. The existing method with a hard-coded sizing factor that is used to calculate autosize values for pump "Rated Power Consumption" will be replaced by a defaulted user input.  A new method will be added that does not rely on pressure and motor efficiency will be added. For completeness, the variable speed pump will also add the capability of scaling the minimum flow rate using a new input for a sizing factor.

##Approach:

A set of new input fields will be added to the end of all five existing pump input objects: Pump:VariableSpeed, Pump:ConstantSpeed, Pump:VariableSpeed:Condensate, HeaderedPumps:ConstantSpeed, HeaderedPumps:VariableSpeed. A choice input field will allow selecting which scaling method is used to size the pump power with the legacy method as the default. For the legacy method, the hard-coded sizing parameter will then be available to users to revise if desired (but as the inverse of the old 0.78 number).  The new method will provide for a simpler scaling based on only the design flow rate and the default is selected to match the 90.1 Appendix G specification for chilled water pumping of 22 W/gpm.  The sizing calculations will be changed to use the new inputs and switch between the two methods accordingly.



##Input/Output Reference:##

Input/Output Reference changes have been made in the file 01d-InputOutputReference.md (on the branch Scalable-Units-Pumps). This is a snippet for the new fields.

#### Field: Design Power Sizing Method

This field is optional.  There are two choices PowerPerFlow and PowerPerFlowPerPressure.  If PowerPerFlow is used the pump's Design Power Consumption will be calculated using the sizing factor from the input field Design Power per Unit Flow Rate -- with Design Power Consumption = Design Maximum Flow Rate * scaling factor.  If PowerPerFlowPerPressure is used the pump's Design Power Consumption will use the sizing factor from the input field Design Power per Unit Flow Rate Per Unit Head -- with Design Power Consumption = Design Maximum Flow Rate * Design Pump Head * scaling factor / Motor Efficiency. If omitted the default method of PowerPerFlowPerPressure will be used.

#### Field: Design Electric Power per Unit Flow Rate

This field is optional.  This input is used when the input field Design Power Consumption is set to autosize and the Design Power Sizing Method is set to PowerPerFlow.  It allows setting the efficiency the pumping system's power consumption using only the design flow rate. If omitted the default value of 348701.1 W/(m<sup>3</sup>/s) ( 22 W/gpm) will be used.

#### Field: Design Shaft Power per Unit Flow Rate Per Unit Head

This field is optional.  This input is used when the input field Design Power Consumption is set to autosize and the Design Power Sizing Method is set to PowerPerFlowPerPressure.  It allows setting the efficiency of the impeller and drive assembly when calculating the pump power consumption for the design flow rate, motor efficiency, and pump head. If omitted the default value of 1.282051 W/((m<sup>3</sup>/s)-Pa) will be used.


In addition to explaining the new input fields in the usual manner, existing input fields for pumps have been improved to reflect current practice.  For example, the use of "Rated" has been changed to "Design" in many input fields. Field descriptions are improved to reflect autosizing and defaults.



## Implementation ##
The implementation of scalable units for pump objects will follow the existing patterns present in the code. Source code files Pumps.hh and Pumps.cc will be changed to read in the new input values and use them during sizing calculations.
### OO Design###
No refactoring of existing code is planned.  This will be a minimal implementation with only slight changes to get input and sizing calculations.

### Data Structures ###

The pump data structure (struct PumpSpecs) will be modified to add variables to hold the user input for the two power scaling factors and the minimum flow scaling factor.

##Input Data Dictionary##

Proposed IDD changes have been drafted in the file Energy+.idd.in (on the branch Scalable-Units-Pumps).  In addition to the new input fields, some of the existing field names have been changed and /notes improved.  The new fields have been located at the end of the input object to avoid creating transition burden.

##Output##

No new outputs are proposed.

##Engineering Reference (draft)##

Engineering reference changes have been drafted in the file 10-Sizing.md (on the branch Scalable-Units-Pumps).  The following snippet is the new section for pump sizing.

### Pump Sizing

The loop pumps' autosizable inputs are design volume flow rate and design power consumption.

#### Design Volume Flow Rate

This is set equal to the design flow rate for the loop which is obtained from summing the needs of the components on the demand side of the loop.  Each component on the plant loop registers its design flow rate and central routines sum them up.

#### Design Power Consumption

There are two methods available for calculating the design flow rate. The pump object has a input field to select which method to use.

The first, and original, method is selected by choosing PowerPerFlowPerPressure.  And the design power is calculated using


<div>$$\dot Q<sub>nom</sub> = H<sub>nom</sub> \cdot \dot V<sub>nom</sub> \cdot ScalingFactor /Eff<sub>mot</sub>$$</div>

where,

*Eff<sub>mot</sub>* is the motor efficiency, often the default value of 0.9.

*V<sub>nom</sub>* is the design volume flow rate in m<sup>3</sup>/s.

*ScalingFactor* is an input called Design Power per Unit Flow Rate per Unit Head, with a default of 1.282051 W/((m<sup>3</sup>/s)-Pa).  (This is the inverse of 0.78 for impeller efficiency that was used prior to version 8.5.)

*H<sub>nom</sub>* the nominal head, or pressure rise across the pump, is an input in Pascals.


The second method is selected by choosing PowerPerFlow.  Then the power is calculated more simply and does not use head pressure or motor efficiency

<div>$$\dot Q<sub>nom</sub> = \dot V<sub>nom</sub> \cdot ScalingFactor$$</div>

where

*ScalingFactor* is an input called Design Power per Unit Flow Rate, with a default of 348701.1 W/(m<sup>3</sup>/s) or 22 W/gpm.


##Testing/Validation/Data Source(s)##
None.

##Example File and Transition changes##

A selection of existing example files will be revised to use the new input fields.

No Transition needed as all the new input fields are located at the end of the input objects and are optional.

##Other documents:

None.
