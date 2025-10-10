<head>
<script type="text/x-mathjax-config">
  MathJax.Hub.Config({
  extensions: ["tex2jax.js"],
  jax: ["input/TeX", "output/HTML-CSS"],
  tex2jax: {
   inlineMath: [ ['$','$'], ["\\(","\\)"] ],
   displayMath: [ ['$$','$$'], ["\\[","\\]"] ],
   processEscapes: true
  },
  "HTML-CSS": { availableFonts: ["TeX"] }
  });
</script>
  <script type="text/javascript"
   src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
</script>
</head>

4.1.1. Single-Zone Variable Air Volume (SZVAV) Controls for Other Zone-Level Equipment
================

**Richard Raustad, Florida Solar Energy Center**

 - NFP Final draft submitted November 22, 2016
 -


## Justification for New Feature ##

New feature added to ZoneHVAC:FourPipeFanCoil object in V8.6. Users request similar control for other zone equipment types. The next logical equipment type would be unitary systems using water coils.

ASHRAE Standard 90.1 requirements for capacity and fan speed control as well as provision of minimum outdoor air requirements based on Standard 62.1.

    Section 6.4.3.10 (“Single Zone Variable-Air-Volume Controls”) of ASHRAE Standard 90.1-2010. HVAC systems shall have variable airflow controls as follows:
    (a) Air-handling and fan-coil units with chilled-water cooling coils and supply fans with motors
        greater than or equal to 5 hp shall have their supply fans controlled by two-speed motors or
        variable-speed drives. At cooling demands less than or equal to 50%, the supply fan controls
        shall be able to reduce the airflow to no greater than the larger of the following:
        • One-half of the full fan speed, or
        • The volume of outdoor air required to meet the ventilation requirements of Standard 62.1.
    (b) Effective January 1, 2012, all air-conditioning equipment and air-handling units with direct
        expansion cooling and a cooling capacity at AHRI conditions greater than or equal to 110,000
        Btu/h that serve single zones shall have their supply fans controlled by two-speed motors or
        variable-speed drives. At cooling demands less than or equal to 50%, the supply fan controls
        shall be able to reduce the airflow to no greater than the larger of the following:
        • Two-thirds of the full fan speed, or
        • The volume of outdoor air required to meet the ventilation requirements of Standard 62.1.

*Note from the author: In part (a) above, the phrase, “At cooling demands less than or equal to 50%…” may be confusing, so the Standard 90.1 User’s Manual clarifies by stating the following:*

        “The term ‘cooling demand’ refers to the zone sensible cooling load.That is, when the zone
        sensible cooling load decreases to 50% of the design sensible cooling load for the zone, the
        supply fan controls shall have reduced airflow to the threshold described.”
        In addition, the User’s Manual clarifies that the supply fan can be controlled by either a
        two-speed motor, an electronically commutated motor (ECM), or a variable-frequency drive (VFD).

## E-mail and  Conference Call Conclusions ##

E-mail notes:

MW - Elementary question - I have to wonder if this control is really necessary for PTACs and PTHPs.  Doesn't a two-speed (or more) PTAC/PTHP already meet the intent of the 90.1 specification?

RR - Thinking now that maybe a better place to start is with AirloopHVAC:UnitarySystem since I have seen more chatter with this new object, and it can do both air loop and zone simulations? Same issue applies here also with 2-sp+ DX coils already capable of this control? So I guess I would target chilled water coils in that object?

TH - I echo Mike's argument. PTACs and PTHPs are usually small systems (less than 5 ton = 60kBtu/h), and two-speed controls would be adequate.


## Overview ##

The Trane Engineers Newsletter describes various control strategies for single-zone equipment. This proposal is closely based on that description. This document is located in the EnergyPlusDevSupport\DesignDocuments\Proposals folder for review as necessary.

Based on the ASHRAE Standard 90.1 description in Section 6.4.3.10, the control of zone equipment, specifically fan speed, is accomplished through modulation of the supply air temperature. In EnergyPlus, capacity regulation is primarily based on water flow rate modulation, and secondarily based on air flow modulation through fan speed adjustment (either single speed, two-speed, or variable-speed). In addition, EnergyPlus zone equipment uses a load-based control approach where supply air temperature is a result of the control, not a primary mechanism for control.

To allow zone equipment to maintain a constant minimum flow rate up to 50% of the **design zone load**, the supply air temperature must be adjusted to meet the zone load and zone air temperature set point while maintaining a fixed minimum equipment flow rate. Since it is possible to control both water and DX coils to a temperature set point, it is proposed that a new control method be added to zone equipment, specifically Fan Coils. This new control method can then be added to other equipment models. Special care will be used to develop a portable model that can easily be applied to other equipment models. The level of effort needed for this task will dictate if this new model can be applied to other equipment models within the time frame and budget allotted for this task.

This example depicts quite well the intended result of the new control method. Maintain a fixed minimum supply air flow rate for all loads within 50% of the design zone load while modulating supply air temperature to meet the zone load. When the zone load exceeds 50% of the design zone load, the supply air flow rate is increased while maintaining a maximum SAT for heating and minimum SAT for cooling.

![](Figure2_Trane.png)

As shown in the figure, there are 3 distinct air flow control scenarios:

  - minimum fan flow where coil capacity is modulated to meet the zone load (center of figure)
  - modulated air flow and/or coil capacity to meet the zone load (increasing air flow rates on left and right side of figure)
  - maximum fan flow were coil capacity is again modulated to meet the zone load (upper left and lower right of figure)

To accomplish this control method, a new key was added to the four pipe fan coil Capacity Control Method field to specify this new ASHRAE90.1 control method.

    ZoneHVAC:FourPipeFanCoil,
    A3 , \field Capacity Control Method
         \required-field
         \type choice
         \key ConstantFanVariableFlow
         \key CyclingFan
         \key VariableFanVariableFlow
         \key VariableFanConstantFlow
         \key MultiSpeedFan
         \key ASHRAE90VariableFan

The maximum and minimum fan flow threshold is determined using existing inputs for Maximum Supply Air Flow Rate and Low Speed Supply Air Flow Ratio fields.

    ZoneHVAC:FourPipeFanCoil,
    N1 , \field Maximum Supply Air Flow Rate
         \required-field
         \autosizable
         \units m3/s
    N2 , \field Low Speed Supply Air Flow Ratio
         \type real
         \minimum> 0.0
         \default 0.33

This allows modeling of two-speed and variable-speed fans. The medium speed supply air flow ratio input field is not used at this time. The 2nd degree of freedom is the range of zone loads to be met. As described in the Standard, a portion of the **zone design sensible load** is used as the critical point. Using existing model inputs, the Low Speed Supply Air Flow Ratio input field will be used to denote this threshold. For water coils, a value of 50% is typically used. For DX equipment, a value of 67% is typically used.

Finally, based on conference call feedback, two additional input fields were added to the end of the fan coil object such that the user could control the temperature limits and also avoid the need for autosizing.

NOTE: Units are missing in these fields "\units C".

    N11, \field Minimum Supply Air Temperature in Cooling Mode
      \note For Capacity Control Method = ASHRAE90VariableFan, enter the minimum air temperature in cooling mode.
      \note Leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1. In this case, a zone sizing simulation is required.
      \type real
      \minimum 0.0
    N12; \field Maximum Supply Air Temperature in Heating Mode
      \note For Capacity Control Method = ASHRAE90VariableFan, enter the maximum air temperature in heating mode.
      \note Leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1. In this case, a zone sizing simulation is required.
      \type real
      \minimum 0.0

The result of this effort provides a water coil model that closely replicates the expected control.

Model inputs:

    ZoneHVAC:FourPipeFanCoil,
      Zone1FanCoil,            !- Name
      ASHRAE90VariableFan,     !- Capacity Control Method
      autosize,                !- Maximum Supply Air Flow Rate {m3/s}
      0.5,                     !- Low Speed Supply Air Flow Ratio
      0.5,                     !- Medium Speed Supply Air Flow Ratio
      autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}
      autosize,                !- Maximum Cold Water Flow Rate {m3/s}
      0.0,                     !- Minimum Cold Water Flow Rate {m3/s}
      autosize,                !- Maximum Hot Water Flow Rate {m3/s}
      0.0,                     !- Minimum Hot Water Flow Rate {m3/s}
      ,                        !- Minimum Supply Air Temperature in Cooling Mode
      ,                        !- Maximum Supply Air Temperature in Heating Mode

![](FanCoil_ASHRAE901_V86.png)<br>
Figure - ASHRAE 90.1 SZVAV Fan Control in V8.6

The simulation results when using the new temperature control inputs is shown in the following figure.

Apparently, this aspect of the model fell short of perfection. Although the temperature limits are enforced, the fan coil air flow rate does not increase until the load threshold is exceeded. We may consider increasing supply air flow rate once the supply air temperature reaches the limits.<br>
![](FanCoil_ASHRAE901_V86_SPc.png)<br>

## Approach ##

The team will:

 - Investigate temperature limits using ASHRAE90.1 control method for ZoneHVAC:FourPipeFanCoil
 - Add new ASHRAE90.1 control for AirloopHVAC:UnitarySystem using water coils.

## Known Issues ##

- Controlling supply air temperature **AND** air flow rate may prove troublesome

## Testing/Validation/Data Sources ##

Compare simulation results with existing equipment models. Document comparison of outlet temperature and humidity ratio and power consumption.

## Input Description ##

The previous implementation for the Fan Coil model required four key inputs.

 - Capacity Control
 - low speed fan ratio
 - Maximum Supply Air Temperature in Heating Mode
 - Minimum Supply Air Temperature in Cooling Mode

Reusing similar inputs in the Unitary System object is desirable as long as those inputs are not ambiguous.

 - For Capacity Control, an input is already available as Control Type
 - For low speed fan ratio, the input for no load supply air flow rate could be used
 - For Maximum Supply Air Temperature in Heating Mode use input for Maximum Supply Air Temperature
 - For Minimum Supply Air Temperature in Cooling Mode one input is available but is ambiguous as currently named.
   - DOAS DX Cooling Coil Leaving Minimum Air Temperature

This field could be changed to Minimum Supply Air Temperature. Otherwise one or more new inputs are required to provide the same inputs as used in the Fan Coil model.

## Outputs Description ##

No change to output reporting

## IDD - Input Data Dictionary ##

Comments are added where fields are affected.

    AirLoopHVAC:UnitarySystem,
       \memo AirloopHVAC:UnitarySystem is a generic HVAC system type that allows any
       \memo configuration of coils and/or fan. This object is a replacement of other
       \memo AirloopHVAC objects. This object can be used in outdoor air systems,
       \memo outdoor air units, air loops, and as zone equipment if desired.
       \min-fields 14
    A1,  \field Name
       \required-field
       \type alpha
       \reference DOAToZonalUnit
       \note Unique name for the Unitary System.
    A2,  \field Control Type
       \type choice
       \key Load
       \key SetPoint

add new key here for capacity control method:

       \key ASHRAE90VariableFan

with description:

       \default Load
       \note Load control requires a Controlling Zone name.
       \note SetPoint control requires set points at coil outlet node.
       \note ASHRAE90VariableFan control requires a Controlling Zone name, water coils and two-speed or variable-speed fan

    A3,  \field Controlling Zone or Thermostat Location
       \note Used only for Load based control
       \type object-list
       \object-list ZoneNames
       \note Zone name where thermostat is located. Required when Control Type = Load.
    A4,  \field Dehumidification Control Type
       \type choice
       \key None
       \key Multimode
       \key CoolReheat
       \default None
       \note None = meet sensible load only
       \note Multimode = activate enhanced dehumidification mode
       \note as needed and meet sensible load.  Valid only with
       \note cooling coil type Coil:Cooling:DX:TwoStageWithHumidityControlMode or CoilSystem:Cooling:DX:HeatExchangerAssisted.
       \note This control mode either switches the coil mode or allows the heat exchanger to be turned
       \note on and off based on the zone dehumidification requirements.
       \note A ZoneControl:Humidistat object is also required.
       \note CoolReheat = cool beyond the dry-bulb setpoint.
       \note as required to meet the humidity setpoint.  Valid with all
       \note cooling coil types. When a heat exchanger assisted cooling
       \note coil is used, the heat exchanger is locked on at all times.
       \note A ZoneControl:Humidistat object is also required.
    A5,  \field Availability Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \note A schedule value greater than zero (usually 1 is used) indicates that the unit is
       \note available to operate as needed. A value less than or equal to zero (usually zero
       \note is used) denotes that the unit must be off.
    A6,  \field Air Inlet Node Name
       \required-field
       \type node
       \note Enter the node name used as the inlet air node for the unitary system.
    A7,  \field Air Outlet Node Name
       \required-field
       \type node
       \note Enter the node name used as the outlet air node for the unitary system.
    A8, \field Supply Fan Object Type
       \type choice
       \key Fan:OnOff
       \key Fan:ConstantVolume
       \key Fan:VariableVolume
       \key Fan:ComponentModel
       \note Enter the type of supply air fan if included in the unitary system.
       \note Fan:ConstantVolume only works with continuous fan operating mode (i.e. supply
       \note air fan operating mode schedule values greater than 0).
       \note Specify a Fan:OnOff object when the Supply Air Fan Operating Mode Schedule Name
       \note input field above is left blank.
       \note Specify a Fan:VariableVolume when modeling VAV systems which used setpoint based control
       \note if the fan is included in the unitary system object.
       \note The ComponentModel fan type may be substituted for the ConstantVolume or VariableVolume
       \note fan types when more detailed fan modeling is required.
       \note The variable or constant volume fan may be specified on the branch instead of contained
       \note within the unitary system object (i.e., this field may be blank for certain configurations).
    A9, \field Supply Fan Name
       \type object-list
       \object-list Fans
       \note Enter the name of the supply air fan if included in the unitary system.
    A10, \field Fan Placement
       \type choice
       \key BlowThrough
       \key DrawThrough
       \note Enter the type of supply air fan if included in the unitary system.
    A11, \field Supply Air Fan Operating Mode Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note A fan operating mode schedule value of 0 indicates cycling fan mode (supply air
       \note fan cycles on and off in tandem with the cooling or heating coil).
       \note Any other schedule value indicates continuous fan mode (supply air fan operates
       \note continuously regardless of cooling or heating coil operation). Provide a schedule
       \note with non-zero values when high humidity control is specified.
       \note Leaving this schedule name blank will default to constant fan mode for the
       \note entire simulation period.
       \note This field is not used when set point based control is used where a set point
       \note controls the coil (i.e., model assumes constant fan mode operation).

revise end of previous note above for fan operating mode when ASHRAE90VariableFan control is specified

       \note controls the coil or ASHRAE90VariableFan control type is selected (i.e., model
       \note assumes constant fan mode operation).

    A12, \field Heating Coil Object Type
       \type choice
       \key Coil:Heating:DX:SingleSpeed
       \key Coil:Heating:DX:MultiSpeed
       \key Coil:Heating:DX:VariableSpeed
       \key Coil:Heating:WaterToAirHeatPump:ParameterEstimation
       \key Coil:Heating:WaterToAirHeatPump:EquationFit
       \key Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit
       \key Coil:Heating:Fuel
       \key Coil:Heating:Gas:MultiStage
       \key Coil:Heating:Electric
       \key Coil:Heating:Electric:MultiStage
       \key Coil:Heating:Water
       \key Coil:Heating:Steam
       \key Coil:Heating:Desuperheater
       \key Coil:UserDefined
       \note Enter the type of heating coil if included in the unitary system.
    A13, \field Heating Coil Name
       \type object-list
       \object-list HeatingCoilsDX
       \object-list HeatingCoilsDXMultiSpeed
       \object-list HeatingCoilsDXVariableSpeed
       \object-list HeatingCoilsWaterToAirHP
       \object-list HeatingCoilsWaterToAirVSHP
       \object-list HeatingCoilName
       \object-list HeatingCoilsElectricMultiStage
       \object-list HeatingCoilsGasMultiStage
       \object-list HeatingCoilsDesuperheater
       \object-list UserDefinedCoil
       \note Enter the name of the heating coil if included in the unitary system.
    N1 , \field DX Heating Coil Sizing Ratio
       \type real
       \default 1.0
       \minimum> 0
       \note Used to adjust heat pump heating capacity with respect to DX cooling capacity
       \note used only for heat pump configurations (i.e., a DX cooling and DX heating coil is used).
    A14, \field Cooling Coil Object Type
       \type choice
       \key Coil:Cooling:DX:SingleSpeed
       \key Coil:Cooling:DX:TwoSpeed
       \key Coil:Cooling:DX:MultiSpeed
       \key Coil:Cooling:DX:VariableSpeed
       \key Coil:Cooling:DX:TwoStageWithHumidityControlMode
       \key Coil:Cooling:DX:SingleSpeed:ThermalStorage
       \key CoilSystem:Cooling:DX:HeatExchangerAssisted
       \key Coil:Cooling:WaterToAirHeatPump:ParameterEstimation
       \key Coil:Cooling:WaterToAirHeatPump:EquationFit
       \key Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit
       \key Coil:Cooling:Water
       \key Coil:Cooling:Water:DetailedGeometry
       \key CoilSystem:Cooling:Water:HeatExchangerAssisted
       \key Coil:UserDefined
       \note Enter the type of cooling coil if included in the unitary system.
    A15, \field Cooling Coil Name
       \type object-list
       \object-list CoolingCoilsDX
       \object-list CoolingCoilsDXMultiSpeed
       \object-list CoolingCoilsDXVariableSpeed
       \object-list CoolingCoilsWaterToAirHP
       \object-list CoolingCoilsWaterToAirVSHP
       \object-list CoolingCoilsWater
       \object-list UserDefinedCoil
       \note Enter the name of the cooling coil if included in the unitary system.
    A16, \field Use DOAS DX Cooling Coil
       \type choice
       \key Yes
       \key No
       \default No
       \note If Yes, the DX cooling coil runs as 100% DOAS DX coil.
       \note If No, the DX cooling coil runs as a regular DX coil.
       \note If left blank the default is regular dx coil.

change field name of:

    N2 , \field DOAS DX Cooling Coil Leaving Minimum Air Temperature

to:

    N2 , \field Minimum Supply Air Temperature
       \type real
       \units C
       \minimum 0.0
       \maximum 7.2
       \default 2.0

revise Minimum Supply Air Temperature note section from:

       \note DX cooling coil leaving minimum air temperature defines the minimum DOAS DX cooling coil
       \note leaving air temperature that should be maintained to avoid frost formation. This input
       \note field is optional and only used along with the input field above.

to:

       \note When USE DOAS DX Cooling Coil is specified as Yes, Minimum Supply Air Temperature
       \note defines the minimum DOAS DX cooling coil leaving air temperature that should
       \note be maintained to avoid frost formation.
       \note When ASHRAE90VariableFan Control Type is specified, this input
       \note field specifies the minimum supply air temperature in cooling mode or
       \note leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1.
       \note In this case, a zone sizing simulation is required.


    A17, \field Latent Load Control
       \type choice
       \key SensibleOnlyLoadControl
       \key LatentOnlyLoadControl
       \key LatentWithSensibleLoadControl
       \key LatentOrSensibleLoadControl
       \default SensibleOnlyLoadControl
       \note SensibleOnlyLoadControl is selected when thermostat control is used.
       \note LatentOnlyLoadControl is selected when humidistat control is used.
       \note LatentWithSensibleLoadControl is selected when thermostat control is used and
       \note dehumidification is required only when a sensible load exists.
       \note LatentOrSensibleLoadControl is selected when thermostat control is used and
       \note dehumidification is required any time the humidistat set point is exceeded.
    A18, \field Supplemental Heating Coil Object Type
       \type choice
       \key Coil:Heating:Fuel
       \key Coil:Heating:Electric
       \key Coil:Heating:Desuperheater
       \key Coil:Heating:Water
       \key Coil:Heating:Steam
       \key Coil:UserDefined
       \note Enter the type of supplemental heating coil if included in the unitary system.
       \note Only required if dehumidification control type is "CoolReheat".
    A19, \field Supplemental Heating Coil Name
       \type object-list
       \object-list HeatingCoilName
       \object-list HeatingCoilsDesuperheater
       \object-list UserDefinedCoil
       \note Enter the name of the supplemental heating coil if included in the unitary system.
       \note Only required if dehumidification control type is "CoolReheat".
    A20, \field Cooling Supply Air Flow Rate Method
       \type choice
       \key None
       \key SupplyAirFlowRate
       \key FlowPerFloorArea
       \key FractionOfAutosizedCoolingValue
       \key FlowPerCoolingCapacity
       \note Enter the method used to determine the cooling supply air volume flow rate.
       \note None is used when a cooling coil is not included in the unitary system or this field may be blank.
       \note SupplyAirFlowRate is selected when the magnitude of the supply air volume is used.
       \note FlowPerFloorArea is selected when the supply air volume flow rate is based on total floor area
       \note served by the unitary system.
       \note FractionOfAutosizedCoolingValue is selected when the supply air volume is a fraction of the
       \note value determined by the simulation.
       \note FlowPerCoolingCapacity is selected when the supply air volume is a fraction of the cooling
       \note capacity as determined by the simulation.
    N3 , \field Cooling Supply Air Flow Rate
       \type real
       \units m3/s
       \minimum 0.0
       \autosizable
       \note Enter the magnitude of the supply air volume flow rate during cooling operation.
       \note Required field when Cooling Supply Air Flow Rate Method is SupplyAirFlowRate.
       \note This field may be blank if a cooling coil is not included in the unitary system.
    N4 , \field Cooling Supply Air Flow Rate Per Floor Area
       \type real
       \units m3/s-m2
       \minimum 0.0
       \note Enter the supply air volume flow rate per total floor area fraction.
       \note Required field when Cooling Supply Air Flow Rate Method is FlowPerFloorArea.
       \note This field may be blank if a cooling coil is not included in the unitary system.
    N5 , \field Cooling Fraction of Autosized Cooling Supply Air Flow Rate
       \type real
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the cooling supply air flow rate.
       \note Required field when Cooling Supply Air Flow Rate Method is FractionOfAutosizedCoolingValue.
       \note This field may be blank if a cooling coil is not included in the unitary system.
    N6 , \field Cooling Supply Air Flow Rate Per Unit of Capacity
       \type real
       \units m3/s-W
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the cooling capacity.
       \note Required field when Cooling Supply Air Flow Rate Method is FlowPerCoolingCapacity.
       \note This field may be blank if a cooling coil is not included in the unitary system.
    A21, \field Heating Supply Air Flow Rate Method
       \type choice
       \key None
       \key SupplyAirFlowRate
       \key FlowPerFloorArea
       \key FractionOfAutosizedHeatingValue
       \key FlowPerHeatingCapacity
       \note Enter the method used to determine the heating supply air volume flow rate.
       \note None is used when a heating coil is not included in the unitary system or this field may be blank.
       \note SupplyAirFlowRate is selected when the magnitude of the supply air volume is used.
       \note FlowPerFloorArea is selected when the supply air volume flow rate is based on total floor area
       \note served by the unitary system.
       \note FractionOfAutosizedHeatingValue is selected when the supply air volume is a fraction of the
       \note value determined by the simulation.
       \note FlowPerHeatingCapacity is selected when the supply air volume is a fraction of the heating
       \note capacity as determined by the simulation.
    N7 , \field Heating Supply Air Flow Rate
       \type real
       \units m3/s
       \minimum 0.0
       \autosizable
       \note Enter the magnitude of the supply air volume flow rate during heating operation.
       \note Required field when Heating Supply Air Flow Rate Method is SupplyAirFlowRate.
       \note This field may be blank if a heating coil is not included in the unitary system.
    N8 , \field Heating Supply Air Flow Rate Per Floor Area
       \type real
       \units m3/s-m2
       \minimum 0.0
       \note Enter the supply air volume flow rate per total floor area fraction.
       \note Required field when Heating Supply Air Flow Rate Method is FlowPerFloorArea.
       \note This field may be blank if a heating coil is not included in the unitary system.
    N9 , \field Heating Fraction of Autosized Heating Supply Air Flow Rate
       \type real
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the heating supply air flow rate.
       \note Required field when Heating Supply Air Flow Rate Method is FractionOfAutosizedHeatingValue.
       \note This field may be blank if a heating coil is not included in the unitary system.
    N10, \field Heating Supply Air Flow Rate Per Unit of Capacity
       \type real
       \units m3/s-W
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the heating capacity.
       \note Required field when Heating Supply Air Flow Rate Method is FlowPerHeatingCapacity.
       \note This field may be blank if a heating coil is not included in the unitary system.

Minimum flow for SZVAV operates at No Load Supply Air Flow Rate:

    A22, \field No Load Supply Air Flow Rate Method
       \type choice
       \key None
       \key SupplyAirFlowRate
       \key FlowPerFloorArea
       \key FractionOfAutosizedCoolingValue
       \key FractionOfAutosizedHeatingValue
       \key FlowPerCoolingCapacity
       \key FlowPerHeatingCapacity
       \note Enter the method used to determine the supply air volume flow rate when no cooling or heating is required.
       \note None is used when a cooling and heating coil is not included in the unitary system or this field may be blank.
       \note SupplyAirFlowRate is selected when the magnitude of the supply air volume is used.
       \note FlowPerFloorArea is selected when the supply air volume flow rate is based on total floor area
       \note served by the unitary system.
       \note FractionOfAutosizedCoolingValue is selected when the supply air volume is a fraction of the
       \note cooling value determined by the simulation.
       \note FractionOfAutosizedHeatingValue is selected when the supply air volume is a fraction of the
       \note heating value determined by the simulation.
       \note FlowPerCoolingCapacity is selected when the supply air volume is a fraction of the cooling
       \note capacity as determined by the simulation.
       \note FlowPerHeatingCapacity is selected when the supply air volume is a fraction of the heating
       \note capacity as determined by the simulation.
    N11, \field No Load Supply Air Flow Rate
       \type real
       \units m3/s
       \minimum 0.0
       \autosizable
       \note Enter the magnitude of the supply air volume flow rate during when no cooling or heating is required.
       \note Required field when No Load Supply Air Flow Rate Method is SupplyAirFlowRate.
    N12, \field No Load Supply Air Flow Rate Per Floor Area
       \type real
       \units m3/s-m2
       \minimum 0.0
       \note Enter the supply air volume flow rate per total floor area fraction.
       \note Required field when No Load Supply Air Flow Rate Method is FlowPerFloorArea.
    N13, \field No Load Fraction of Autosized Cooling Supply Air Flow Rate
       \type real
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the cooling supply air flow rate.
       \note Required field when No Load Supply Air Flow Rate Method is FractionOfAutosizedCoolingValue.
    N14, \field No Load Fraction of Autosized Heating Supply Air Flow Rate
       \type real
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the heating supply air flow rate.
       \note Required field when No Load Supply Air Flow Rate Method is FractionOfAutosizedHeatingValue.
    N15, \field No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation
       \type real
       \units m3/s-W
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the cooling capacity.
       \note Required field when No Load Supply Air Flow Rate Method is FlowPerCoolingCapacity.
    N16, \field No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation
       \type real
       \units m3/s-W
       \minimum 0.0
       \note Enter the supply air volume flow rate as a fraction of the heating capacity.
       \note Required field when No Load Supply Air Flow Rate Method is FlowPerHeatingCapacity.

Change field name to Maximum Supply Air Temperature in Heating Mode to match other objects?
Or change those to match this descriptor?

    N17, \field Maximum Supply Air Temperature
       \type real
       \units C
       \autosizable
       \default 80.0
       \note Enter the maximum supply air temperature leaving the heating coil.

Revise note for Maximum Supply Air Temperature as:

       \note Enter the maximum supply air temperature leaving the heating coil.
       \note When ASHRAE90VariableFan Control Type is selected, enter the maximum air temperature in heating mode.
       \note Leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1. In this case, a zone sizing simulation is required.

    N18, \field Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation
       \type real
       \units C
       \default 21.0
       \note Enter the maximum outdoor dry-bulb temperature for supplemental heater operation.
    A23, \field Outdoor Dry-Bulb Temperature Sensor Node Name
       \type node
       \note If this field is blank, outdoor temperature from the weather file is used.
       \note If this field is not blank, the node name specified determines the outdoor temperature used
       \note for controlling supplemental heater operation.
    N19, \field Maximum Cycling Rate
       \type real
       \units cycles/hr
       \minimum 0.0
       \maximum 5.0
       \default 2.5
       \note Used only for water source heat pump.
       \note The maximum on-off cycling rate for the compressor.
       \note Suggested value is 2.5 for a typical heat pump.
    N20, \field Heat Pump Time Constant
       \type real
       \units s
       \minimum 0.0
       \maximum 500.0
       \default 60.0
       \note Used only for water source heat pump.
       \note Time constant for the cooling coil's capacity to reach steady state after startup.
       \note Suggested value is 60 for a typical heat pump.
    N21, \field Fraction of On-Cycle Power Use
       \type real
       \minimum 0.0
       \maximum 0.05
       \default 0.01
       \note Used only for water source heat pump.
       \note The fraction of on-cycle power use to adjust the part load fraction based on
       \note the off-cycle power consumption due to crankcase heaters, controls, fans, and etc.
       \note Suggested value is 0.01 for a typical heat pump.
    N22, \field Heat Pump Fan Delay Time
       \type real
       \units s
       \minimum 0.0
       \default 60
       \note Used only for water source heat pump.
       \note Programmed time delay for heat pump fan to shut off after compressor cycle off.
       \note Only required when fan operating mode is cycling.
       \note Enter 0 when fan operating mode is continuous.
    N23, \field Ancillary On-Cycle Electric Power
       \type real
       \units W
       \minimum 0
       \default 0
       \note Enter the value of ancillary electric power for controls or other devices consumed during the on cycle.
    N24, \field Ancillary Off-Cycle Electric Power
       \type real
       \units W
       \minimum 0
       \default 0
       \note Enter the value of ancillary electric power for controls or other devices consumed during the off cycle.
    N25, \field Design Heat Recovery Water Flow Rate
       \type real
       \units m3/s
       \ip-units gal/min
       \minimum 0.0
       \default 0.0
       \note If non-zero, then the heat recovery inlet and outlet node names must be entered.
       \note Used for heat recovery to an EnergyPlus plant loop.
    N26, \field Maximum Temperature for Heat Recovery
       \type real
       \units C
       \maximum 100.0
       \minimum 0.0
       \default 80.0
       \note Enter the maximum heat recovery inlet temperature allowed for heat recovery.
    A24, \field Heat Recovery Water Inlet Node Name
       \type node
       \note Enter the name of the heat recovery water inlet node if plant water loop connections are present.
    A25, \field Heat Recovery Water Outlet Node Name
       \type node
       \note Enter the name of the heat recovery water outlet node if plant water loop connections are present.
    A26, \field Design Specification Multispeed Object Type
       \type choice
       \key UnitarySystemPerformance:Multispeed
       \note Enter the type of performance specification object used to describe the multispeed coil.
    A27; \field Design Specification Multispeed Object Name
       \type object-list
       \object-list UnitarySystemPerformaceNames
       \note Enter the name of the performance specification object used to describe the multispeed coil.


## Input Output Reference Documentation ##

Add new field descriptions.

## Engineering Reference ##

Add model calculations.

## Example File and Transition Changes ##

No transition required unless fields are inserted within existing object fields.

## References ##

NA




