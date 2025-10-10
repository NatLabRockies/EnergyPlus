A New Versatile Fan
================

**B. Griffith, Energy Archmage Company, for SEI/TRANE**

 - first draft 11/10/2015
 - second draft 11/23/2015


## Justification for New Feature ##

The existing set of fan objects is confusing to users and a burden to interface developers.  Fan:ConstantVolume and Fan:OnOff appear to be for the same sort of applications and it has been suggested that they be combined into one.  Fan:OnOff has curve options for performance at varying flow which seems to be the domain of Fan:VariableVolume. Fan:VariableVolume has fixed input fields for curve coefficients rather than referencing an external performance curve which is current practice for other HVAC component models. There is no clarity on which fan is for two-speed applications.

ASHRAE Standard 90.1 has new requirements that will lead to two-speed fans in many (all?) applications which makes in more important than ever that EnergyPlus has a clear and accurate way of modeling two-speed (and multi-speed) fans. Fan:OnOff was expanded circa 2009 to add curves for performance at different flow rates to improve modeling multi-speed fans in unitary systems (see NFP_OnOffFan_MultispeedPowerAdjustments.doc). However, PNNL researchers  point out that the flow-weighted average approach introduces up to 20% error in power consumption compared to a time-weighted approach (see Hart, Athalye, and Wang. 2013. Improving Simulation of Outside Air Economizers and Fan Control for Unitary Air Conditioners). The problem with the current approach is that it assumes that the fan is really running at the average flow rate when calculating power. When the average flow rate does not match one of the actual speed levels that the fan can operate at, the fan is actually cycling between speed levels with some fraction of the time spent at each speed level.  Because fan power is highly non-linear with flow, it is important to average the power results using a time-weighted approach rather than simply use the average flow in the power calculation.  It is therefore justified that fan models be improved to include input data for the discrete speed levels that the fans can actually operate at so that the current problem can be corrected. The following figure diagrams the issue.

![PNNL figure on power calculation problem]( Two-Speed_FanPowerProblem_PNNL.png)
Figure 1. PNNL graphic showing fan power calculation problem PNNL-SA-100271

Because fans are present in the vast majority of input files, making substantial changes to existing fan objects, or outright forcing two or more to be combined, would create transition and usability burdens for users and interface developers.  It is therefore helpful to introduce an entirely new fan object that can coexist for a time with the older fan objects while gradually replacing them over one or more release cycles.  A new fan object is more valuable if designed to allow for older fan input objects to eventually be transitioned to the new object. (However transition and removal of older fans is not being proposed for the current project, and would be a separate project for a future release cycle.)

Scalable units for sizing fan power of the form W/cfm and W/cfm-inH2O have been requested by a major interface developer because it simplifies the creation of library data and communicating with users in straightforward engineering units.  However, the existing fan input objects do not have an input for design electric power which is unlike HVAC component models elsewhere in EnergyPlus. After examining the options for adding scalable fan sizing it was concluded that an entirely new fan input object would be justified.

A new, do-all fan model that can be substituted for Fan:VariableVolume, Fan:ConstantVolume, or Fan:OnOff, and not require a separate FanPerformance:NightVentilation object, is justified because it will enable a major revamp of usability issues related to fans, a simpler solution for managing libraries, more accurate two- and multi-speed modeling, allow using separate curves and tables to describe VAV performance, etc. etc.


## E-mail and  Conference Call Conclusions ##

An early-phase first draft design document was circulated among the sizing subgroup and discussed during a conference call on November 11, 2015. Email comments were provided by three reviewers. The overall idea of a new fan object was viewed favorably but with caution and concerns about transition and policy towards yet another fan. Follow up discussions resulted in a recommendation to move forward with a full proposal, specific suggestions to improve the design of the input object for transition, and a request to use new-style OO implementation in a separate source code file.

A second draft of the design document was produced that incorporates comments.

## Overview ##

This proposal is for adding an entirely new fan input object, to be called Fan:SystemModel, that can serve as a direct substitute for Fan:VariableVolume, Fan:ConstantVolume, or Fan:OnOff. In addition it will incorporate input fields needed for night ventilation operation mode so that a separate FanPerformance:NightVentilation is not required when using the night ventilation availability manager. The core fan model will not be significantly changed, but rather the simple (not component model) model variants will be all be available within a single input object.  The exception is that for fans with discrete speed levels, the power calculation will be done using a time-weighted method instead of a flow-weighted method. Also, motor losses not added to the air stream can be rejected to the mechanical room. The older fan objects will be retained for some number of release cycles with the plan that they will eventually be deprecated and transitioned to use the new fan object.

## Approach ##

A new input object with proposed class name *Fan* will be added that has a design electric power consumption, separate performance modifier curves, scalable sizing factors, the ability to be used as a single-speed, two-speed, multi-speed, or VAV.  New input for discrete speed levels will be included and fan power calculations improved for time-weighted averaging. The underlying engineering model will be largely retained.  Sizing calculations will determine and report a Design Electric Power Consumption. Energy balance will be improved by allowing the option of entering a name of the zone where the motor is located that will receive motor heat losses that are not added to the air stream as internal gains. Parent objects that are not destined to be deprecated and contain a fan will all be modified to accept the new fan.  The new fan will be designed to allow for eventual transition of older fan objects into the new object.

## Implementation ##
Fan models are currently implemented in Fans.hh and Fans.cc.  The different fan models all share the same data structure and many of the same routines.  The new fan will be implemented in a new source file, called  HVACFan.hh/HVACFan.cc, to allow for new style programming and a clean separation from the older fan objects to help with eventual removal of them from the code.  When motor efficiency and motor in air stream fractions are both less than 1.0 and a motor zone name is input, then the losses will be added to the zone as internal gains as is now done for pump.

### OO Design###
The new fan will be implemented as a class, called FanSystem, using Object Oriented programming. The existing fan code in Fans.hh/Fans.cc will be used as guide to the engineering formulation but reimplemented using OO techniques.  The style will largely follow the pattern in HVACFourPipeBeam.cc. The fan object instances will be held in standard library smart pointers.  Although most of the functionality will be inside the FanSystem class, there will also be routines in a namespace called HVACFan that will be used to perform lookup and checks across object instances.

### Data Structures ###
The data structure called FanEquipConditions will serve as a starting point for data in the new fan class. New variables will be added for input for scalable units sizing factors, sizing method switches, design electric power, discrete speed levels, and zone gains.

## Testing/Validation/Data Sources ##

The new fan will be compared to the older fans with equivalent inputs and results compared.

## Input Output Reference Documentation ##

The fans are described in the file "01d-InputOutputReference.md"  However the I/o reference will be updated throughout wherever fan objects are referenced and the new fan can be used. The Fan I/O description follows:

### Fan:SystemModel
This object models fans of various types using a relatively simple engineering model. This fan can be used in variable air volume, constant volume, on-off cycling, two-speed, or multi-speed applications.  It was designed as a replacement for Fan:ConstantVolume, Fan:OnOff, Fan:VariableVolume, and FanPerformance:NightVentilation. The electric power consumed by the fan can be directly input or autosized using one of three optional methods.  For fans that can vary the volume flow rate the performance can be described using a separate performance curve or table object. Or for fans with discrete speed control the power fraction at each speed can be input directly with no need for a performance curve.

#### Field: Name
A unique name for this fan.  Any reference to this fan by another object will use this name.

#### Field: Availability Schedule Name
The name of an availability schedule for this fan. Schedule values greater than zero means the fan is available. If this field is blank, the fan is always available.

#### Field: Air Inlet Node Name
The name of an air system node at the inlet to the fan. This field is required.

#### Field: Air Outlet Node Name
The name of an air system node at the outlet of the fan.  This field is required.

#### Field: Design Maximum Air Flow Rate
This numeric field is the design volume flow rate of fan as installed in the HVAC system, in m<sup>3</sup>/s. This volume flow rate will be converted to a mass flow rate using an altitude-adjusted standard density of dry air at 20 &deg;C drybulb. This the full-speed flow rate and serves as the upper limit for fans that can vary their flow rate. This field can be autosized.

#### Field: Speed Control Method
This field is used to select how the fan fan speed can be varied. There are two choices, Continuous or Discrete.  Discrete indicates that the fan can operate only at specific speed settings and cannot be continuously varied.  Continuous indicates that the fan speed is variable and can change smoothly up to the Design Maximum Air Flow Rate.  This input informs the program how power should be calculated with Discrete control using time-weighted averaging and Continuous using flow-weighted averaging. A constant volume or on-off fan should use Discrete with Number of Speeds set to 1.  A variable air volume fan should use Continuous.

#### Field: Electric Power Minimum Flow Rate Fraction
This numeric field is used to describe how low a variable speed fan can be operated. This value is used to calculate the fan power at low flow rates but does not enforce a lower end of the air flow during simulation. The value is a fraction of Design Maximum Air Flow Rate and should be between 0 and 1.  This field is only used when Speed Control Method is set to Continuous.

#### Field: Design Pressure Rise
This numeric field is the total system pressure rise experienced by the fan in Pascals at full flow rate and altitude-adjusted standard density of dry air at 20 &deg;C drybulb. This field is required.

#### Field: Motor Efficiency
This numeric field describes the electric motor that drives the fan.  Efficiency is the shaft power divided by the electric power consumed by the motor.  The value must be between 0 and 1. The default is 0.9.

#### Field: Motor In Air Stream Fraction
This numeric field is the fraction of the motor heat that is added to the air stream.  The value must be between 0 and 1.  A value of 0 means fan motor is located completely outside of air stream and none of the motor's heat loss is added to the air stream.  A value of 1.0 means the motor is located completely inside of air stream and all of the motor's heat loss is added to the air stream.  Note that regardless of the value here there will be heat added to the air stream as a result of the work done to move the air, this field is only describing what happens to the heat generated as a result of the motor's inefficiency. The heat lost from the motor that is not added to the air stream can be added to the surrounding thermal zone where the motor is located by entering a zone name in the input field called Motor Loss Zone Name below.

#### Field: Design Electric Power Consumption
This numeric field is the electric power consumption at the full Design Maximum Air Flow Rate and Design Pressure Rise, in Watts.  The value entered in this field is used to determine the fan efficiency. This field is autosizable.  When autosized there are three different options available for the method used to size the design power and can be selected in the following field.

#### Field: Design Power Sizing Method
This field is used to select how the fan's Design Electric Power Consumption is sized when the previous field is set to autosize.  There are three choices: PowerPerFlow, PowerPerFlowPerPressure, or TotalEfficiency.  The default is PowerPerFlowPerPressure.

When PowerPerFlow is selected, the value entered in the input field called Electric Power Per Unit Flow Rate is used to size the Design Electric Power Consumption.  This method is useful during early-phase design modeling when little information is available for determining the Design Pressure Rise.  Although the pressure rise is not used to size the Design Electric Power Consumption it is still used to determine the heat added to the air stream as a result of the work done by the fan.

When PowerPerFlowPerPressure is selected, the value entered in the input field called Electric Power Per Unit Flow Rate Per Unit Pressure is used to size the power.  This method takes into account the Design Pressure Rise when sizing the Design Electric Power Consumption.

When TotalEfficiency is selected, the value entered in the input field called Fan Total Efficiency is used to size the power.  This is the legacy method used by the older fan objects prior to verson 8.5.

#### Field: Electric Power Per Unit Flow Rate
This numeric field is used when the Design Power Sizing Method is set to PowerPerFlow and the Design Electric Power Consumption is set to Autosize.  This value, in W/(m<sup>3</sup>/s), is used to scale the Design Electric Power Consumption directly from the Design Maximum Air Flow Rate.  This scaling factor is defined such that Design Electric Power Consumption = (Electric Power Per Unit Flow Rate) * (Design Maximum Air Flow Rate).

#### Field: Electric Power Per Unit Flow Rate Per Unit Pressure
This numeric field is used when the Design Power Sizing Method is set to PowerPerFlowPerPressure and the Design Electric Power Consumption is set to Autosize.  This value, in W/((m<sup>3</sup>/s)-Pa), is used to scale the Design Electric Power Consumption from the Design Maximum Air Flow Rate and the Design Pressure Rise.  This scaling factor is defined such that Design Electric Power Consumption = (Electric Power Per Unit Flow Rate Per Unit Pressure) * (Design Maximum Air Flow Rate) * (Design Pressure Rise).  The default is 1.66667.

#### Field: Fan Total Efficiency
This numeric field is used when the Design Power Sizing Method is set to TotalEfficiency and the Design Electric Power Consumption is set to Autosize.  This value is used to determine the Design Electric Power Consumption from the Design Maximum Air Flow Rate and the Design Pressure Rise.  The total efficiency is defined such that the Design Electric Power Consumption = (Design Maximum Air Flow Rate) * (Design Pressure Rise) / (Fan Total Efficiency).  The default is 0.7.

#### Field: Electric Power Function of Flow Fraction Curve Name
This field is the name of performance curve or table that describes how electric power consumption varies with air flow rate.  The independent "x" variable of the performance curve or look up table is a normalized flow fraction defined as the current flow rate divided by the Design Maximum Air Flow Rate.  The model actually uses the ratio of (moist) air mass flow rates with the numerator taking account of humidity and barometric pressure.  The dependent variable that is the result of the performance curve or lookup table is a fraction that is multiplied by the Design Electric Power Consumption to determine the electric power use as a function of flow rate.  Any of the single-independent-variable curves can be used.  This field is required if the Speed Control Method is set to Continuous.  This field is used when the Speed Control Method is set to Discrete and the Number of Speeds is greater than 1 and the input fields Speed "n" Electric Power Fraction are left blank.  Note that the fourth order polynomial in Curve:Quartic can be used with the coefficients listed above to replicate the formulation used in the older Fan:VariableVolume input object prior to version 8.5

#### Field: Night Ventilation Mode Pressure Rise
This optional numeric field is the total system pressure rise experienced by the fan when operating in night mode using AvailabilityManager:NightVentilation, in Pascals.  This field allows modeling the fan device with a different system pressure that might occur when implementing a special strategy to precool a building at night using outdoor air with dampers fully open.  This field is only used when an AvailabilityManager:NightVentilation object is used that specifies the fan's availability schedule.  This field and the next one replace the FanPerformance:NightVentilation object which is not needed with this fan.

#### Field: Night Ventilation Mode Flow Fraction
This optional numeric field is the air flow fraction for the fan speed used when operating in night mode using AvailabilityManager:NighVentilation.  This is fraction between 0 and 1 and describes the speed level for the fan relative to the Design Maximum Air Flow Rate. This field is only used when an AvailabilityManager:NightVentilation object is used that specifies the fan's availability schedule.  This field and the previous one replace the FanPerformance:NightVentilation object which is not needed with this fan.

#### Field: Motor Loss Zone Name
This optional field can be used to input the name of the Zone in which the fan motor is located.  If the fan is outdoors, or the motor's thermal losses are not to be modeled then leave this field blank.  If a valid Zone name is entered then the portion of the motor's thermal losses that are not added to the air stream are added to the surrounding thermal zone as internal heat gains.

#### Field:Motor Loss Radiative Fraction
This optional numeric field is used when a Zone name is entered in the previous field to determine the split between thermal radiation and thermal convection for the heat losses from the fan motor. If this field is left blank then all the losses will be convective. Values should be between 0 and 1.

#### Field: End-Use Subcategory
This optional field allows entering a user-defined name for the end use subcategory that  will be used to meter this fan's electric energy consumption. If this field is omitted or left blank the fan will be assigned to the "General" end use subcategory.  End use subcategories are helpful to organize reports in the tabular summary table and appear on special meter outputs.

#### Field: Number of Speeds
This numeric field is used to specify the number of different speed levels available when Speed Control Method is set to Discrete.  This field and the remaining field sets are not used when the Speed Control Method is set to Continuous.  For a constant volume fan enter a value of 1.0.  A value of 1.0 will use the fan's maximum design and no additional field sets are needed.  When set to a value greater than 1 then a pair of flow and power fraction inputs are provided for each speed in the remaining input fields.

#### Field Set: (Speed Flow Fraction, Speed Electric Power Fraction)
This input object is extensible with a set of two fields for each speed that the fan can take.  A field set is pair of values for the flow fraction and electric power fraction at each speed. The sets should be arranged in increasing order so that the flow fractions become larger in subsequent field sets.  Typically the highest speed level will match the design maximum and have fractions of 1.0.

#### Field: Speed <#> Flow Fraction
This is the flow fraction for the fan speed.  This value is multiplied by the Design Maximum Air Flow Rate to obtain the flow rate when operating at this speed.

#### Field: Speed <#> Electric Power Fraction
This field is the electric power fraction for the fan speed.  This value is multiplied by the Design Electric Power Consumption to obtain the power consumption when operating at this speed.  This field is optional if a performance curve is used in the input field Electric Power Function of Flow Fraction Curve Name.  If omitted and the performance curve is entered, the power at this speed will be determined using the curve or table.  If the power fraction is entered in this field it will be used instead of the curve or table.  This allows either overriding the curve for particular speeds or removes the necessity of creating a curve for discrete speed control.

### Input Description ###

The IDD object for the proposed new fan input follows

    Fan,
         \memo Versatile simple fan that can be used in variable air volume, constant volume, or on-off cycling applications.
         \memo Performance at different flow rates, or speed levels, is determined using separate performance curve or table
         \memo or prescribed power fractions at discrete speed levels for two-speed or multi-speed fans.
         \min-fields 13
         \extensible:2
    A1 , \field Name
         \required-field
         \reference Fans
         \reference FansCV
         \reference FansCVandOnOff
         \reference FansCVandOnOffandVAV
    A2 , \field Availability Schedule Name
         \note Availability schedule name for this fan. Schedule value > 0 means the fan is available.
         \note If this field is blank, the fan is always available.
         \type object-list
         \object-list ScheduleNames
    A3 , \field Air Inlet Node Name
         \type node
         \required-field
    A4 , \field Air Outlet Node Name
         \type node
         \required-field
    N1 , \field Design Maximum Air Flow Rate
         \required-field
         \type real
         \units m3/s
         \minimum> 0.0
         \autosizable
    A5 , \field Speed Control Method
         \type choice
         \key Variable
         \key Discrete
         \default Discrete
    N2 , \field Electric Power Minimum Flow Rate Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
         \default 0.2
    N3 , \field Design Pressure Rise
         \type real
         \units Pa
         \ip-units inH2O
         \required-field
    N4 , \field Motor Efficiency
         \type real
         \default 0.9
         \minimum> 0.0
         \maximum 1.0
    N5 , \field Motor In Air Stream Fraction
         \note 0.0 means fan motor outside of air stream, 1.0 means motor inside of air stream
         \type real
         \default 1.0
         \minimum 0.0
         \maximum 1.0
    N6 , \field Design Electric Power Consumption
         \type real
         \units W
         \autosizable
         \note Fan power consumption at maximum air flow rate.
         \note If autosized the method used to scale power is chosen in the following field
    A6 , \field Design Power Sizing Method
         \type choice
         \key PowerPerFlow
         \key PowerPerFlowPerPressure
         \key TotalEfficiency
         \default PowerPerFlowPerPressure
    N7 , \field Electric Power Per Unit Flow Rate
         \type real
         \units W/(m3/s)
         \ip-units W/(ft3/min)
         \default ???
    N8 , \field Electric Power Per Unit Flow Rate Per Unit Pressure
         \type real
         \units W/((m3/s) -Pa)
         \ip-units W/((ft3/min)-inH2O)
         \default 1.66667
    N9,  \field Fan Total Efficiency
         \type real
         \default 0.7
         \minimum>0.0
         \maximum 1.0
    A7 , \field Electric Power Function of Flow Fraction Curve Name
         \note independent variable is normalized flow rate, current flow divided by Design Maximum Flow Rate.
         \note dependent variable is modification factor multiplied by Design Power Consumption.
         \note field is required if Speed Control Method is set to Continuous
         \type object-list
         \object-list UniVariateCurves
         \object-list UniVariateTables
    N10, \field Night Ventilation Mode Pressure Rise
         \note pressure rise to use when in night mode using AvailabilityManager:NightVentilation
         \type real
         \units Pa
         \ip-units inH2O
    N11, \field Night Ventilation Mode Flow Fraction
         \note Fraction of Design Maximum Air Flow Rate to use when in night mode using AvailabilityManager:NightVentilation
         \type real
         \minimum 0.0
         \maximum 1.0
    A8 , \field Motor Loss Zone Name
          \note optional, if used fan motor heat losses that not added to air stream are transferred to zone as internal gains
          \type object-list
          \object-list ZoneNames
    N12, \field Motor Loss Radiative Fraction
          \note optional. If zone identified in previous field then this determines
          \note the split between convection and radiation for the fan motor's skin losses
          \type real
          \minimum 0.0
          \maximum 1.0
    A9 , \field End-Use Subcategory
         \type alpha
         \retaincase
         \default General
    N13, \field Number of Speeds
         \note number of different speed levels available when Speed Control Method is set to Discrete
         \note Speed need to be arranged in increasing order in remaining field sets.
         \note If set to 1, or omitted, and Speed Control Method is Discrete then constant fan speed is the design maximum.
         \type integer
         \default 1
    N14, \field Speed 1 Flow Fraction
         \begin-extensible
         \type real
         \minimum 0.0
         \maximum 1.0
    N15, \field Speed 1 Electric Power Fraction
         \note if left blank then use Electric Power Function of Flow Fraction Curve
         \type real
         \minimum 0.0
         \maximum 1.0
    N16, \field Speed 2 Flow Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
    N17, \field Speed 2 Electric Power Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
    N18, \field Speed 3 Flow Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
    N19, \field Speed 3 Electric Power Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
    N20, \field Speed n Flow Fraction
         \type real
         \minimum 0.0
         \maximum 1.0
    N21; \field Speed n Electric Power Fraction
         \type real
         \minimum 0.0
         \maximum 1.0

### Outputs Description ###

<insert text>

## Engineering Reference ##

Air System Fans <a name="Fans"></a>
---------------

### Overview

Four input objects (Fan, Fan:ConstantVolume, Fan:VariableVolume, and Fan:OnOff) provide models for fans -- the prime movers in most of the air loop and zonal air conditioning systems in EnergyPlus. The *Fan* input object was added for version 8.5 with the intent that it provide a single versatile input object that will eventually replace Fan:ConstantVolume, Fan:VariableVolume, and Fan:OnOff. For these input objects, EnergyPlus uses a simple polynomial-based curve-fit model to describe the relation between the volume flow rate and the fan electric power, with no explicit modeling of fan pressure rise. Consequently, duct-static-pressure reset strategies can only be modeled using curves that have been specially developed to approximate static reset.  For CAV and VAV central air-handling systems, a fifth input object (Fan:ComponentModel) provides a simple physics-based model for flow-dependent fan pressure rise and detailed models for fan, belt, motor, and variable-frequency-drive efficiencies and energy use. This latter input object provides the capability to analyze the effects of duct-static-pressure reset strategies, as well as changes in fan system element performance. All of these fan models calculate the fan energy usage, which is often a large fraction of HVAC energy use and a significant portion of the building energy consumption. They also calculate the temperature rise in the air stream caused by the static pressure increase as the air goes through the fan. If the motor and belt are in the air stream, waste heat from the motor and belt also acts to raise the air stream temperature.

### Model

Each of the five fan models is a forward type: the model inputs describe the fan characteristics and the conditions of the air at the fan inlet; the outputs are the fan electrical power consumption and the conditions of the air at the fan outlet. The model algorithms and data for Fan:ConstantVolume, Fan:VariableVolume, Fan:OnOff and Fan:ComponentModel are contained in the *Fans.cc* and *Fans.hh* files in EnergyPlus.  The source code for the Fan model added for version 8.5 is contained in the *FanObject.cc* and *FanObject.hh* files.

#### Inputs and Data

For the *Fan* model (added for version 8.5), the user describes the fan by entering values for the design pressure rise across the fan, the design volume flow rate, the design electric power consumption, and the fan motor efficiency. The user also needs to specify the fraction of the fan motor's waste heat that will enter the air stream (usually 0 or 1). If the fan is indoors, the name of a Zone and a fraction for the split between thermal radiation and convection can be entered so that the portion of fan motor waste heat that does not enter the air stream can be added to the thermal zone surrounding the fan. The user enters a choice for fan speed control method to be either Continuous (for VAV) or Discrete (for constant volume, on-off, two-speed, or multi-speed).  A performance curve or lookup table defined separately can be referenced by the fan to describe the variation in fan power as a function of flow rate. This curve is required for continuous speed control. However, for discrete speed control the user has the option of directly entering a power modification factor for each discrete speed and a curve is not necessary. For two- or multi-speed fans, the number of speeds is entered and pairs of values for the flow fraction and power fraction at each speed level are entered in increasing order. The design electric power consumption can be autosized using input values for one of three available scaling factors: fan total efficiency, electric power per unit flow rate, or electric power per unit flow rate per unit pressure.

For the Fan:ConstantVolume, Fan:VariableVolume, and Fan:OnOff models, the user describes the fan by entering values for the design pressure rise across the fan, the design volume flow rate, the fan total efficiency, and the fan motor efficiency.  The user also needs to specify the fraction of the fan waste heat that will enter the air stream (usually 0 or 1). For the Fan:VariableVolume model, the user must also enter the coefficients of a 4<sup>th</sup> order polynomial that relates the fan mass flow rate to the fan power consumption. The independent variable is the volume flow fraction; the dependent variable is the fan power part load ratio. For multi-speed fans, the user must enter a fan power ratio as a function of speed ratio performance curve name. Multi-speed fans can only be simulated in a parent object which allows multiple fan speeds (e.g., AirLoopHVAC:Unitary:Furnace:HeatCool, ZoneHVAC:PackagedTerminalAirConditioner, etc.). An optional efficiency curve may also be used when simulating multispeed fans to adjust the fan total efficiency as the fan speed changes.

For the Fan:ComponentModel object, the user describes the fan in more detail, and also describes the duct system characteristics as seen by the fan. In particular, the user specifies a pressure rise curve with four coefficients that relates the fan total pressure rise to the volumetric flow through the fan, the duct-static-pressure set-point, and the static pressure of the spaces surrounding the ducts. If duct-static-pressure reset is used, the user enters a linear curve with two coefficients that relates the pressure set-point to the volumetric flow through the fan. For the fan itself, the user specifies the fan geometry (wheel diameter and outlet area), maximum efficiency, the Euler number corresponding to the maximum efficiency, the maximum dimensionless flow, the names of four curves with several coefficients that describe the variation of fan efficiency and dimensionless flow with the Euler number in the normal and stall operation regions of the fan, and a sizing factor (applied to the maximum flow through the fan). For the belt, the user specifies the maximum efficiency (or a curve with five coefficients that defines the maximum efficiency as a function of maximum fan shaft input power), three curves with three coefficients each that relate the belt part-load efficiency to belt fractional output torque, the motor/fan pulley diameter ratio, the belt output torque capacity, and a sizing factor (applied to the maximum output torque of the belt). For the motor, the user specifies the maximum efficiency (or a curve with three coefficients that define the maximum efficiency as a function of maximum belt input power), a curve with three coefficients that relate the motor part-load efficiency to motor fractional output power, and a sizing factor (applied to the maximum output power of the motor). For the variable-frequency-drive (VFD), the user specifies a curve with three coefficients that relate the VFD part-load efficiency to motor fractional input power or to motor fractional speed, and a sizing factor (applied to the maximum output power of the VFD).

#### Control

The models must decide whether the fan is on or off. The primary on/off trigger is the fan availability schedule. This is an on/off schedule associated with each fan: a value of 1 indicates the fan is on; a value of 0 indicates the fan is off. The fan schedule can be overruled by flags set by system availability managers. If the flag *TurnFansOn* is true, a zero fan schedule value will be overridden and the fan will be turned on. If the flag *TurnFansOff* is true the fan will be forced off. The inlet air mass flow rate must be greater than zero for the fan to be on.

Generally the fan is a passive component: it accepts the mass flow on its inlet node, uses it in its calculations of energy consumption and temperature rise, and passes it to the outlet node. However the fan maximum and minimum airflow rates act as absolute limits on the airflow rate.

For multi-speed fans, the parent object determines the fan speed ratio (i.e., the selected speed of the fan motor) and uses this value to determine flow rate and electric power consumption.  For a Fan operating with the Discrete speed control, the fan model determines the faction of time spent at the discrete speeds that bound it and will produce the average flow requested by the parent object.  Then the fan electric power consumption is determined for each speed level and combined using a time-weighted average.  For a fan operating with Continuous speed control and a fan modeled with the Fan:OnOff object, the electric power is evaluated using the power ratio performance curve at the (average) flow fraction requested by the parent object.


#### Simulation

For the *Fan* model based on design electric power consumption, the fan's total efficiency is determined at the beginning of the simulation using:

<div>$${e_{tot,max}} = \frac{({\dot V_{design,max} \cdot \Delta P  } )}{{ \dot Q_{tot, design}}}$$</div>

For a fan with discrete speed control with more than one speed and input data for Speed *x* Electric Power Fraction, the fan's total efficiency at each speed *x* is determined and stored using:

<div>$${e_{tot,x}} = \frac {({f_{flow,x}} \cdot {\dot V_{design,max} \cdot \Delta P  } )}{( {f_{power,x}} \cdot { \dot Q_{tot, design}})}$$</div>

For a fan with discrete speed control and no data for the power fraction at a speed, the power performance curve (or table) is evaluated first using the flow fraction for the speed:

<div>$${f_{power,x}} =  {func_{power curve}}({f_{flow,x}})$$</div>

The fan model and EnergyPlus's modeling of flow at air system nodes is actually calculated using mass flow rates.  User input for design maximum volume flow rate is converted to a design mass flow rate using a design density of air that is adjusted for altitude above sea level and dry air at 20 &deg;C drybulb.

<div>$${\dot m_{design,max}} = {\dot V_{design, max}} \cdot  {\rho_{air,design}}$$</div>

For a fan with discrete speed control with more than one speed, the design mass flow rate at each speed is determined and stored using:

<div>$${\dot m_{design,x}} = {f_{flow,x}} \cdot {\dot V_{design,max}}$$</div>

Simple (Single Speed) Fan Model

The following equations define the model for this fan:

<div>$${\dot Q_{tot}} = \dot m\cdot \Delta P/({e_{tot,max}}\cdot {\rho_{air}})$$</div>

<div>$${\dot Q_{shaft}} = {e_{motor}}\cdot {\dot Q_{tot}}$$</div>

<div>$${\dot Q_{toair}} = {\dot Q_{shaft}} + ({\dot Q_{tot}} - {\dot Q_{shaft}})\cdot {f_{motortoair}}$$</div>

<div>$${h_{out}} = {h_{in}} + {\dot Q_{toair}}/\dot m$$</div>

<div>$${w_{out}} = {w_{in}}$$</div>

<div>$${T_{out}} = PsyTdbFnHW({h_{out}},{w_{out}})$$</div>

Fan Model for Changing Flow Rates

The fan will operate at a flow fraction that is determined by other HVAC equipment in the air system.  A parent object will control the air flow rates and the fan flow fraction is defined as the ratio of the current mass flow rate divided by the design mass flow rate.

<div>$${f_{flow, cur}} = \frac{\dot m_{cur}}{\dot m_{design,max}}$$</div>

The determination of electric power consumed at a flow fraction depends on if the speed control method is discrete or continuous.

Discrete Speed Control

For the *Fan* model using discrete speed control, with the number of fan speeds set at 1, and the flow fraction is less than one, then the fan will be modeled as cycling between "off" and "on" over the timestep.  The time fraction is equal to the flow fraction so that:

<div>$${RTF_{On}} = {f_{flow, cur}}$$</div>

<div>$${\dot Q_{tot,elec}} = {RTF_{On}} \cdot \left( \frac{\dot m\cdot \Delta P}{{e_{tot}}\cdot {\rho_{air}}}\right)$$</div>

Similarly, for two-speed and multi-speed fans, the current flow fraction will be used to find the run time fraction for each of the adjacent speed levels *x* and *x+1* that bracket the flow fraction such that

<div>$$ {f_{flow,x}} \leq {f_{flow, cur}} \leq {f_{flow,x+1}} $$</div>

The runtime fraction spent at each speed is determined using

<div>$${RFT_{x}} = \left( \frac{({f_{flow,x+1}} - {f_{flow, cur}} )}{({f_{flow,x+1}} - {f_{flow,x}})}  \right)$$</div>

<div>$${RFT_{x+1}} = \left( \frac{({f_{flow, cur}} - {f_{flow,x}} )}{({f_{flow,x+1}} - {f_{flow,x}})}  \right)$$</div>

The electric power consumption is calculated using the design mass flow rates and the total efficiency stored for each speed and combined using the runtime fraction at each speed.

<div>$${\dot Q_{tot,elec}} = {RTF_{x}} \cdot \left( \frac{\dot m_{design,x}\cdot \Delta P}{{e_{tot,x}}\cdot {\rho_{air}}}\right)  +  {RTF_{x+1}} \cdot \left( \frac{\dot m_{design,x+1}\cdot \Delta P}{{e_{tot,x+1}}\cdot {\rho_{air}}}\right) $$</div>

The temperature of the air stream leaving the fan is calculated as above using this result for total power.


Continuous Speed Control

For the *Fan* model using Continuous control the flow fraction is used to evaluate the power performance modification factor which is defined in a separate curve or table.

<div>$${f_{power,cur}} =  {func_{power curve}}({f_{flow,cur}})$$</div>

<div>$${e_{tot,cur}} = \frac {( { \dot m_{cur} \cdot \Delta P  } )}{( {f_{power,cur}} \cdot { \dot Q_{tot, design} \cdot \rho_{air}})}$$</div>

Fan:OnOff Fan Model

The on/off fan model is similar to the simple fan model with the exception that the on/off fan may cycle on and off during a simulation time step. The cycling rate of the fan is known as the run time fraction. The calculation of run time fraction accounts for the part-load losses of other equipment used in the HVAC system. A part-load factor (a.k.a. part-load ratio) is first calculated for the fan as the ratio of the actual operating mass flow rate to the maximum fan mass flow rate. The run time fraction is then calculated as the part-load factor divided by the part-load fraction. The part-load fraction is determined by other HVAC equipment in the simulation (Ref. DX coil) for use by this specific fan model.

<div>$${f_{flow}} = {\raise0.7ex\hbox{${\mathop m\limits^ \bullet  }$} \!\mathord{\left/ {\vphantom {{\mathop m\limits^ \bullet  } {{{\mathop m\limits^ \bullet  }_{\max }}}}}\right.}\!\lower0.7ex\hbox{${{{\mathop m\limits^ \bullet  }_{\max }}}$}}$$</div>

<div>$$RTF = {\raise0.7ex\hbox{${{f_{flow}}}$} \!\mathord{\left/ {\vphantom {{{f_{flow}}} {PLF}}}\right.}\!\lower0.7ex\hbox{${PLF}$}}$$</div>

The total fan power is then calculated as the maximum fan power multipled by the run time fraction.

<div>$${\mathop Q\limits^\cdot_{tot}} = RTF\left[ {\mathop m\limits^\cdot  \cdot \Delta P/\left( {{e_{tot}}\cdot {\rho_{air}}} \right)} \right]$$</div>

The remaining calculations are the same as those described in the simple single-speed fan mode.

Multi-Speed Fan Model

The model used to simulate a multi-speed fan relies on the general fan laws to correct for speed changes in the fan’s motor and the corresponding change in fan power and fan total efficiency. Two performance curves are used to define the change in power and efficiency as shown below. The power ratio curve must be used to simulate a multi-speed fan. The power ratio curve modifies the fan power based on a change in fan speed according to the cubic fan law. Exponents other than 3 are allowed. The efficiency ratio curve is used to correct for changes in nominal efficiency at alternate fan speeds. If either or both of these curves are not provided, the ratio is assumed to be 1.

The power ratio term is evaluated using an exponent performance curve. The form of the exponent curve equation is shown below.

<div>$$ExponentCurveOutput = C1 + C2*{x^{C3}}$$</div>

The exponent performance curve is used to evaluate the power ratio as a function of speed ratio (x in the equation above). For typical fan performance, coefficient C1 = 0 and coefficient C2 = 1. The exponent (C3) is typically equal to 3 (cubed fan law) but other values are allowed.

<div>$$PowerRatio = f{\left( {{N_{ratio}}} \right)^n} = ExponentCurveOutput$$</div>

The efficiency ratio curve is a quadratic or cubic curve used to evaluate the efficiency ratio as a function of speed ratio (x in the following equation). The cubic equation form is shown below.

<div>$$EfficiencyCurveOutput = C1 + C2{\left( x \right)^2} + C3{\left( x \right)^3}$$</div>

The curve coefficients are determine by correlating the normalized fan total efficiency (i.e., the actual fan total efficiency divided by the nominal fan total efficiency at a speed ratio of 1) to the speed ratio (i.e., the ratio of actual air flow rate through the fan to the maximum fan air flow rate).

<div>$$EfficiencyRatio = f\left( {{N_{ratio}}} \right) = EfficiencyCurveOutput$$</div>

To account for the change in fan speed and to ensure that the run time fraction is accurately calculated, an adjustment to the previous calculation for run time fraction is made. This adjusts the calculated run time fraction to account for times when the fan speed has been reduced and the new speed is now considered the fan’s maximum speed. The speed adjusted run time fraction is then used in the fan power calculation.

<div>$$RTF = {\raise0.7ex\hbox{${RTF}$} \!\mathord{\left/ {\vphantom {{RTF} {{N_{ratio}}}}}\right.}\!\lower0.7ex\hbox{${{N_{ratio}}}$}}$$</div>

<div>$${\mathop Q\limits^ \bullet_{tot}} = RTF\left[ {\frac{{\mathop m\limits^ \bullet  \Delta P}}{{{e_{tot}}{\rho_{air}}}}} \right]\left( {\frac{{PowerRatio}}{{EfficiencyRatio}}} \right)$$</div>

Each of the performance curves described above may be used to model the performance of a multi-speed fan motor, however, the power ratio curve must be used to envoke the multi-speed simulation. These curves are used when the fan is used in an HVAC system having multiple flow rates (i.e., different flow rates in cooling and heating mode). If an HVAC system operates at the same speed in either cooling or heating mode, these curves are not required. When these curves are not used, the associated ratio term in the equation above is assumed to be 1. The remaining calculations are identical to the simple single-speed fan model described above.

Variable Speed Fan Model

The model for the variable speed fan is similar to the simple single-speed fan model except for a part load factor that multiplies the fan power consumption.

<div>$${f_{flow}} = \dot m/{\dot m_{design}}$$</div>

<div>$${f_{pl}} = {c_1} + {c_2} \cdot {f_{flow}} + {c_3} \cdot f_{flow}^2 + {c_4} \cdot f_{flow}^3 + {c_5} \cdot f_{flow}^4$$</div>

<div>$${\dot Q_{tot}} = {f_{pl}}\cdot {\dot m_{design}}\cdot \Delta P/\left( {{e_{tot}}\cdot {\rho_{air}}} \right)$$</div>

The rest of the calculation is the same as for the simple fan.

#### Nomenclature for Simple Models

<span>\({\dot Q_{tot}}\)</span> is the fan power in watts;

<span>\(\dot m\)</span>is the air mass flow in kg/s;

<span>\({\dot m_{design,max}}\)</span>is the design (maximum) air flow in kg/s;

<span>\(\Delta P\)</span>is the fan design pressure increase in Pascals;

<span>\({e_{tot}}\)</span> is the fan total efficiency;

<span>\({\rho_{air}}\)</span>is the air density at standard conditions in kg/m<sup>3</sup>;

<span>\({e_{motor}}\)</span> is the motor efficiency;

<span>\({\dot Q_{shaft}}\)</span>is the fan shaft power in watts;

<span>\({\dot Q_{toair}}\)</span>is the power entering the air in watts;

<span>\({N_{ratio}}\)</span>is the ratio of actual fan flow rate (or speed) to maximum fan flow rate (or speed)

<span>\({h_{in}},{h_{out}}\)</span> are the inlet and outlet air stream specific enthalpies in J/kg;

<span>\({w_{in}},{w_{out}}\)</span> are the inlet and outlet air stream humidity ratios;

<span>\({T_{out}}\)</span> is the outlet air temperature in degrees C;

<span>\(PsyTdbFnHW\)</span>is the EnergyPlus psychrometric routine relating enthalpy and humidity ratio to temperature;

<span>\({f_{flow}}\)</span>is the flow fraction or part-load ratio;

<span>\({f_{pl}}\)</span> is the part load factor.

## Example File and Transition Changes ##

No transition is planned for this project however feasibility of transition rules are taken into account with the intent of eventually being able to transition Fan:VariableVolume, Fan:OnOff, and Fan:ConstantVolume into the the Fan.

## References ##

Hart, R., Athalye, R., Wang, W.; 2013. Improving Simulation of Outside Air Economizer and Fan Control for Unitary Air Conditioners. ASHRAE Transactions. Vol 119 Issue 2, p1-8.
