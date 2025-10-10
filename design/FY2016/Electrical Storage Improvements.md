Electric Storage Improvements
================

**B. Griffith, Energy Archmage Company, for DOE/NREL/GARD **

 - November 9, 2015
 -

 FY2016 subtask 4.2.1

## Justification for New Feature ##

Electric storage capabilities in EnergyPlus are too limited. The current capabilities for storage were only intended for storing electrical power produced by on-site generators and the charge and discharge supervisory controls are hard-coded with no options.  New capabilities to handle grid-connected storage and more versatile storage control capabilities are justified to allow EnergyPlus users to model such systems and to compare the performance of thermal energy storage to electric power storage.  The improvements planned will address the following list of applications for electric storage that EnergyPlus is not currently able to model:

- Electric storage charge power is taken from the grid with no on-site generation.
- Electric storage charge power combines grid-supplied electricity with on-site generation.
- Electric storage charge power uses all of the on-site generation regardless of facility demand.
- Electric storage discharge controlled to regulate facility demand on utility service.
- Electric storage discharge controlled to follow a prescribed schedule
- Electric storage discharge controlled to follow a prescribed meter
- Electric storage discharge controlled to regulate net export to the grid for time-of-day net metering or distribution grid voltage regulation.

In addition, EnergyPlus lacks EMS actuators needed to allow users to write their own custom supervisory control programs using the EMS.

A number of inter-related github issues need to be addressed as part of project that is larger than individual bug fixes.


## E-mail and  Conference Call Conclusions ##

- S. Frank of NREL confirmed need for AC-to-DC converter in 11/5/2015 email.

## Overview ##

The scope of intended applications for the ElectricLoadCenter in EnergyPlus will be expanded to include grid interactions in addition to expanding supervisory control options for electric storage. Controls over storage management will be expanded while retaining legacy intended behavior as default. A collection of changes and bug fixes will be addressed to improve electrical storage.  EMS actuators will be added to facilitate custom control beyond what will be available with native methods.

## Approach ##

The input object ElectricLoadCenter:Distribution will be expanded to add new input fields to improve control over charging and discharging of electrical storage and allow grid interaction.

New EMS actuators will be added to allow users to write their own control programs using EnergyPlus Runtime Language.

A new component ElectricLoadCenter:Storage:Converter will be added to model power conversion losses when converting AC to DC for grid supplied power to charge DC storage device.

A number of known issues related to storage will be addressed as part of a general rewrite of code and comprehensive edit of documentation, including:
	1. Issue #3211 -- capacity limited to on-site generator charging.
	2. issue #5004 -- battery won't charge at its max rate
	3. Issue #4273 -- documentation of storage rules and behavior.
	4. issue #3531 -- documentation of tariffs with on-site production, storage etc. Change example files to use meters and tariffs as currently intended.
	5. Issue #5299 -- Warn if thermal to electric power ratio not set for FollowThermal* generator operation schemes
	6. Issue #5302, detect and warn if multiple load centers and one battery (appears to work with simple storage).
	7. Issue #5301, fatal kinetic battery with no information, was hard crash in  v8.1.
	8. issue #4921, false object not found error, units in kinetic battery differ from simple storage.
	9. Issue #4639, missing ElectricLoadCenter:Distribution object should lead to error message.
	10. Issue #4113, RDD and documentation don't match for whole-building electric reports
	11. Issue #5303, Sign reversed in kinetic battery model Electric Storage Production Decrement Energy

## Testing/Validation/Data Sources ##

<insert text>

## Input Output Reference Documentation ##

Introductory text will be added at the Group level to help explain how the various ElectricLoadCenter:* and Generator:* objects relate to one another to provide higher level guidance.

The ElectricLoadCenter:Distribution object will be modified to add the following new input fields at the end of the object.  All of the new inputs will be setup so if they are omitted the legacy behavior, or at least intended behavior were there no issues, will be retained.

#### Field: Storage Charge Power Source
This field is used to determine the power source used to charge the electric storage device. There are four choices: OnSiteGenerators, OnSiteGeneratorsSurplus, ScheduledGridSupply, and OnSiteGeneratorSurplusPlusScheduledGridSupply.

- OnSiteGenerators key choice indicates that all the power produced on-site is used to charge the storage regardless of the current facility power demand. The rate of charging will depend on how the generators are controlled.
- OnSiteGeneratorsSurplus key choice indicates that the power produced on-site that exceeds the facility demand is used to charge the storage.  The rate of charging will depend on how the generators are controlled and the current level of power consumed by the building and its systems. This was the intended legacy behavior prior to version 8.5 and is therefore the default.
- ScheduledGridSupply key choice indicates that the power used to charge will be drawn from the utility service connection.  The rate of charging will depend on the power level set in the input field Maximum Storage Charge Grid Supply Power as modified by a fractional schedule named in the input field called Storage Charge Grid Supply Power Fraction Schedule Name. This is the only choice available when electric storage is used without any on-site generators. If generators are present they will not be used to charge storage.
- OnSiteGeneratorSurplusPlusScheduledGridSupply key choice indicates that the power used to charge will be drawn from both utility service connection and what is produced on-site that exceeds the facility demand.  The rate of charging will depend on the power level set in the input field Maximum Storage Charge Grid Supply Power as modified by a fractional schedule named in the input field called Storage Charge Grid Supply Power Fraction Schedule Name, and on how the generators are controlled and the current level of power consumed by the building and its systems.

####Field: Maximum Storage State of Charge Fraction
This numeric field specifies the fraction of storage capacity used as upper limit for controlling charging.  Charging will be constrained so that charging will stop the once this limit is reached. This fraction is the state of charge of the storage device where 1.0 is completely full and 0.0 is completely empty.  This allows supervisory control over charging to model behavior intended to protect the battery from damage. The legacy behavior prior to version 8.5 was to charge to full capacity and therefore the default is 1.0.

####Field: Minimum Storage State of Charge Fraction
This numeric field specifies the fraction of storage capacity used as lower limit for controlling discharging.  Discharging will be constrained so that discharging will stop once this limit is reached.  This fraction is the state of charge of the storage device where 1.0 is completely full and 0.0 is completely empty.  This allows supervisory control over discharging to model behavior intended to protect the battery from damage. The legacy behavior prior to version 8.5 was to discharge to empty and therefore the default is 0.0.

#### Field: Maximum Storage Charge Grid Supply Power
This numeric field specifies the maximum rate that electric power taken from grid to charge storage, in Watts.  This field is used when the Storage Charge Operation Scheme is set to ScheduledGridSupply or OnSiteGeneratorSurplusPlusScheduledGridSupply. The actual rate of utility service draw will be this value multiplied by the value in the schedule named in the following field.  This is the power level as viewed from grid. If a transformer and/or rectifier is involved the power actually going into storage will be adjusted downward as a result of conversion losses.

#### Field: Storage Charge Grid Supply Power Fraction Schedule Name
This field is the name of a schedule that is used to control the timing and magnitude of charging from grid supply. This field is only used, and is required, when Storage Charge Operation Scheme is set to ScheduledGridSupply or OnSiteGeneratorSurplusPlusScheduledGridSupply. Schedule values should be fractions between 0.0 and 1.0, inclusive, and are multiplied by the grid supply power in the previous field to control the rate of charging from the grid.

#### Field: Storage Converter Name
 This field is the name of an ElectricLoadCenter:Storage:Converter object defined elsewhere in the input file that describes the performance of converting convert AC to DC when charging DC storage from grid supply. This field is required when using DC storage (buss type DirectCurrentWithInverterDCStorage) with grid supplied charging power (Storage Charge Operation Scheme is set to ScheduledGridSupply or OnSiteGeneratorSurplusPlusScheduledGridSupply.) Although some inverter devices are bidirectional a separate converter object is needed to describe AC to DC performance.

#### Field: Storage Operation Scheme
This field is used to determine how storage charge and discharge is controlled.  There are five choices:  FacilityDemandLimit, TrackFacilityElectricDemand, TrackSchedule, TrackMeter, and ScheduledGridExport.

- DemandLimit indicates that storage discharge control will limit facility power demand drawn from the utility service while accounting for any on-site generation.  The rate of discharge will depend on the current level of power consumed by the building and its systems, the power generated any on-site generation, and conversion losses as well as demand limit and schedule in the next two fields. This is intended to help control demand (kW) charges in the electric service tariff.
- TrackElectric indicates that storage discharge control will follow the facility power demand while accounting for any on-site generation.  This was the intended behavior prior to version 8.5 and is therefore the default.
- TrackSchedule indicates that the storage discharge control will follow the schedule named in the input field called Storage Discharge Track Schedule Name.  This scheme does not account for any on-site generation and provides direct control over storage draws without any adjustments for conversion losses.
- TrackMeter indicates that storage discharge control will follow an electric meter named in the field called Storage Discharge Track Meter Name, while accounting for any on-site generation and conversion losses.
- ScheduledGridExport indicates that storage discharge control will attempt to export power to the utility service connection following the power level in the field called Maximum Storage Discharge Grid Export Power multiplied by the value in the schedule named in the field Storage Discharge Grid Export Fraction Schedule Name.  This scheme accounts for any on-site generation and can be used to regulate surplus power exported to the grid.  For example, when used with intermittent on-site generation, such as wind or PV, surplus power can be exported to the grid in a smooth, well-controlled manner to aid voltage regulation on the local distribution grid.

#### Field: Storage Discharge Demand Limit
This numeric field is the target utility service demand power limit, in Watts.  used to control storage discharge when using .  This field is only used, and is required, when the Storage Discharge Operation Scheme Facility is set to DemandLimit.  This value is the power as viewed from the grid.  Power conversion losses from any inverter and/or transformer located between the storage and the utility service connection are considered and the storage discharge will be adjusted higher to compensate.

#### Field: Storage Discharge Demand Limit Fraction Schedule Name
This field is the name of a schedule that can be used to vary the target utility service demand power limit over time.  If omitted a schedule value of 1.0 is used.  Schedule values should be between 0.0 and 1.0.  The values in the schedule are multiplied by the power limit level in the previous field to set a target demand limit that is used to control storage discharge.

#### Field: Storage Discharge Track Schedule Name
This field is the name of a schedule that is used control the storage discharge.  The schedule values are in Watts.  This is the power viewed from the buss connected to the storage device and does not include any power conversion outside of the storage device.  This field is only used, and is required, when the Storage Discharge Operation Scheme is set to TrackSchedule.

#### Field: Storage Discharge Track Meter Name
This field is the name of an EnergyPlus electric meter that is used to control storage discharge. The power level from the meter is considered as AC and used to control the discharge.  The rate of discharge is determined by first using an on-site generation and then meeting the remaining load by drawing from storage.  When the storage is DC the discharge rate will be adjusted upward to account for inverter losses (but not transformer losses).   This field is only used, and is required, when the Storage Discharge Operation Scheme Type is set to TrackMeter.

#### Field: Design Storage Discharge Grid Export Power
This numeric field is the target net power exported to the utility service, in Watts.  This is maximum value and is modified by the values in the schedule named in the following field.  The target export power is used in the control of storage discharge along with the current on-site generation.  This value is the power as viewed from the storage. Power conversion losses from any inverter and/or transformer located between the storage and the utility service connection are considered and the actual grid export will be adjusted lower to compensate.


#### Field: Storage Discharge Grid Export Fraction Schedule Name
This field is the name of a schedule that can be used to vary the power exported to the grid over time.  If omitted a schedule value of 1.0 is used.  Schedule values should be between 0.0 and 1.0.  The values in the schedule are multiplied by the Maximum Storage Discharge Grid Export Power in the previous field to set a target level for surplus power and is used to control storage discharge.



A new object with class name of ElectricLoadCenter:Storage:Converter is proposed to describe the performance of AC to DC conversion for grid interaction with on-site batteries. Grid-supplied charging of DC storage (storage devices on a load center with buss type set to DirectCurrentWithInverterDCStorage) requires a rectifier or AC to DC converter.  This device might the backward operating mode for a bidirectional inverter but regardless will be modeled as a separate converter component.

###  ElectricLoadCenter:Storage:Converter
This model is for converting AC to DC for grid-supplied charging of DC storage.  The model is only for power conversion and does not consider voltage. There are two methods available for determining the efficiency with which power is converted.  The efficiency is defined as the ratio of DC power output divided by AC power input.  If the name of a zone is entered the power conversion losses will be added to the zone as internal heat gains.

#### Field: Name
This field contains a unique name for the AC to DC converter.

#### Field: Availability Schedule Name
This field contains the name of a schedule that describes when the power converter is available. If power conversion is not available then electric power from the grid cannot be used to charge storage. Any non-zero value means the converter is available. If this field is blank, the schedule has a value of 1 for all time periods and the converter is always available. Standby power consumption is on when converter is available and off when the converter is not available.

#### Field: Power Conversion Efficiency Method
This choice field is used to select which method is used to define the efficiency with which power is converted from AC to DC.  There are two options: SimpleFixed or FunctionOfPower.
- SimpleFixed indicates that power conversion efficiency is a constant with the value set in the next field. There is no need to size the converter (with a value in the field called Design Maximum Continuous Input Power) and varying levels of power do not affect the efficiency. SimpleFixed is the default.
- FunctionOfPower indicates that the power conversion efficiency is a function of the level of power being converted. This method is intended to model the characteristic that converters tend to operate less efficiently when loaded well below their design size. The converter must be sized with a value in the Design Maximum Continuous Input Power and the functional relationship described in a performance curve or look-up table.

#### Field: Simple Fixed Efficiency
This numeric field is used to set a constant efficiency for conversion of AC to DC at all power levels. This field is only used, and is required, when the Power Conversion Efficiency Method is set to SimpleFixed. The value must be greater than 0.0 and less than equal to 1.0.  The default is 0.95.

#### Field: Design Maximum Continuous Input Power
This numeric field describes the size of the power converter in terms of its design input power level, in Watts.  This is the AC power going into the converter.  This field is only used, and is required, when the the Power Conversion Efficiency Method is set to FunctionOfPower.  This input serves as an upper limit for the AC power input and is used to normalize power for use in the performance curve or table.  The AC power being converted at any given time is divided by the value in this field.

#### Field: Efficiency Function of Power Curve Name
This field is the name of a performance curve or table object that describes how efficiency varies as a function of normalized power.  The single independent "x" variable input for the curve or table is the ratio of AC input power at a given time divided by design power in the previous field.  The result of the curve should be the power conversion efficiency for that normalized power so that DC power output is the product of efficiency multiplied by the AC power input.  Any of the single-variable performance curves or lookup table objects can be used to describe performance. This field is only used, and is required, when the the Power Conversion Efficiency Method is set to FunctionOfPower.

#### Field: Ancillary Power Consumed In Standby
This numeric field describes the ancillary power consumed by the converter when it is available but not converting power, in Watts.  This field is optional and can be used with any of the efficiency methods. If this converter is really on mode of a bidirectional inverter, take care not to double count the ancillary consumption by including them in both this component and the inverter component.

#### Field: Zone Name
This field is the name of thermal zone where the converter is located.  If this field is omitted then the converter is considered outdoors.  The power lost during the conversion process is treated as heat gains and added the thermal zone named in this field.  The split between radiation and convection can be controlled in the next input field.

 #### Field: Radiative Fraction
This numeric field is the fraction of zone heat gains that are handled as infrared thermal radiation.  This field is only used if a zone is named in the previous field.  The portion of zone gains that are not radiative are added to the zone as convection. If a zone is named and this field left blank or omitted then all the zone heat gains will be convective.


## Outputs Description ##

A set of output variables will be added for the new AC-to-DC converter.

## Implementation ##
Source code changes will be made to new files that will replace the existing files ManageElectricPower.hh (1,108 lines) and ManageElectricPower.cc (3,770 lines) with new object oriented code using smart pointers and classes.

### OO Design ###
ManageElectricPower will be completely rewritten to be new-style object oriented code.  Development will proceed in two phases with the first phase being a refactor that aims to reproduce the original code behavior with the intent of showing no numerical differences in tests suites as a check that the refactor has gone well. This includes all the code for transfomer, inverters, storage batteries and all the control and generator dispatch.  Then the second phase will introduce the new features and fixes planned here implemented in the new OO code base.  The new component model for ElectricLoadCenter:Storage:Converter will be implemented using OO techniques. The large manager routine ManageElectricLoadCenters() will be broken up into a number of member functions belonging to a new object.

### Data Structures ###
ManageElectricPower.hh contains the data structures for management, supervisory control, and aggregated reporting of on-site generation , storage, and power conversion (ElecLoadCenter, WholeBldgElectSummary).  It also contains the data structures for component models for transformer, inverter, and storage devices.  These data structures will be refactored to be class data in the new objects for the OO refactoring.

## Engineering Reference ##

Engineering reference will be corrected and expanded to better explain existing and new changes.

EMS Application Guide will be updated for new actuators.

## Example File and Transition Changes ##

<insert text>

## References ##

<insert text>



