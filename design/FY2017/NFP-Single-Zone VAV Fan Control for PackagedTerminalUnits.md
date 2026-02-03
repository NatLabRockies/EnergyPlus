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

4.1.1. Single-Zone Variable Air Volume (SZVAV) Controls for Zone-Level Packaged Terminal Equipment
================

**Richard Raustad, Florida Solar Energy Center**

 - NFP Initial draft submitted November 15, 2017
 - Design document conference call on Nov 27, 2017 (EL, MW, JD, RR)
 - Update to this document based on design team conference call and review
 -
 - As of 12/5/17, this Final NFP document now incorporates all expected code changes.



## Justification for New Feature ##

New feature added to ZoneHVAC:FourPipeFanCoil object in V8.6 and AirloopHVAC:UnitarySystem object in V8.7. Users request similar control for other zone equipment types. The next logical equipment type would be packaged air conditioners or heat pumps. This control requires two-speed or variable speed coils and fans. The ZoneHVAC:PackagedTerminalAirConditioner or ZoneHVAC:PackagedTerminalHeatPump both allow variable speed coils with corresponding adjustments to air flow rate.

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

E-mail notes: na


## Overview ##

The Trane Engineers Newsletter describes various control strategies for single-zone equipment. This proposal is closely based on that description. This document is located in the EnergyPlusDevSupport\DesignDocuments\Proposals folder for review as necessary.

Based on the ASHRAE Standard 90.1 description in Section 6.4.3.10, the control of zone equipment, specifically fan speed, is accomplished through modulation of the supply air temperature. In EnergyPlus, capacity regulation is primarily based on water flow rate modulation, and secondarily based on air flow modulation through fan speed adjustment (either single speed, two-speed, or variable-speed). In addition, EnergyPlus zone equipment uses a load-based control approach where supply air temperature is a result of the control, not a primary mechanism for control.

To allow zone equipment to maintain a constant minimum flow rate up to 50% of the **design zone load**, the supply air temperature must be adjusted to meet the zone load and zone air temperature set point while maintaining a fixed minimum equipment flow rate. Since it is possible to control both water and DX coils to a temperature set point, it is proposed that a new control method be added to zone equipment, specifically Fan Coils. This new control method can then be added to other equipment models. Special care will be used to develop a portable model that can easily be applied to other equipment models. The level of effort needed for this task will dictate if this new model can be applied to other equipment models within the time frame and budget allotted for this task.

This example depicts quite well the intended result of the new control method. Maintain a fixed minimum supply air flow rate for all loads within 50% of the design zone load while modulating supply air temperature to meet the zone load. When the zone load exceeds 50% of the design zone load, the supply air flow rate is increased while maintaining a maximum SAT for heating and minimum SAT for cooling.

![](Figure2_Trane.png)

As shown in the figure, there are 3 distinct air flow control scenarios:

  - minimum fan flow where coil capacity is modulated to meet the zone load (center of figure)
  - modulated air flow and/or coil capacity to meet the zone load (increasing air flow rates on left and right side of figure)
  - maximum fan flow were coil capacity is again modulated to meet the zone load (upper left and right of figure)

## SZVAV History in EnergyPlus
The history of the SZVAV model began with the fan coil unit and was then added to the unitary system. Each of these additions required new or revised inputs to each object and specific code to each model.

# Fan Coil Model #

A new key was added to the four pipe fan coil Capacity Control Method field to specify this new ASHRAE90.1 control method.

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

This allows modeling of two-speed and variable-speed fans. As described in the Standard, a portion of the **zone design sensible load** is used as the critical point. Using existing model inputs, the Low Speed Supply Air Flow Ratio input field is used to denote this threshold. For water coils, a value of 50% is typically used. For DX equipment, a value of 67% is typically used.

Finally, based on conference call feedback, two additional input fields were added to the end of the fan coil object such that the user could control the temperature limits and also avoid the need for autosizing.

    N11, \field Minimum Supply Air Temperature in Cooling Mode
      \note For Capacity Control Method = ASHRAE90VariableFan, enter the minimum air temperature in cooling mode.
      \note Leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1. In this case, a zone sizing simulation is required.
      \type real
      \units C
      \minimum 0.0
      \autosizable
      \default autosize
    N12; \field Maximum Supply Air Temperature in Heating Mode
      \note For Capacity Control Method = ASHRAE90VariableFan, enter the maximum air temperature in heating mode.
      \note Leave this field blank or enter 0 to control to the zone load per ASHRAE 90.1. In this case, a zone sizing simulation is required.
      \type real
      \units C
      \minimum 0.0
      \autosizable
      \default autosize

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

# AirloopHVAC:UnitarySystem #

The SZVAV model was then added to the UnitarySystem object in a similar manner to that used for fan coils.

A new choice was added to specify the control type:

    A2,  \field Control Type
         \type choice
         \key Load
         \key SetPoint
         \key SingleZoneVAV
         \default Load

Two existing inputs were used to specify the minimum and maximum supply air temperature:

    N2,  \field Minimum Supply Air Temperature
         \type real
         \units C
         \minimum 0.0
         \maximum 20.0
         \default 2.0
         \autosizable
         \note When Use DOAS DX Cooling Coil is specified as Yes, Minimum Supply Air Temperature
         \note defines the minimum DOAS DX cooling coil leaving air temperature that should
         \note be maintained to avoid frost formation. This field is not autosizable when
         \note the input for Use DOAS DX Cooling Coil = Yes.
         \note When Control Type = SingleZoneVAV, enter the minimum air temperature limit for reduced fan speed.
    N17, \field Maximum Supply Air Temperature
         \type real
         \units C
         \autosizable
         \default 80.0
         \note Enter the maximum supply air temperature leaving the heating coil.
         \note When Control Type = SingleZoneVAV, enter the maximum air temperature limit for reduced fan speed.


 For this implementation, the SZVAV model was improved to better control the equipment capacity such that how the model performs is more like the description in the Standard. For example, the Standard describes "At cooling demands less than or equal to 50%" where the figure below shows the zone design loads and using this information the model can exactly model 1/2 the design load or any fraction of the design load desired. This improvement to the SZVAV model would have been applied to the fan coil model except that specific code was developed separately for each equipment type (i.e., this improvement was not included for the fan coil model).

![](UnitarySystem_SZVAV_watercoils.png)<br>


## Approach ##

The team will apply the previously developed SZVAV modeling methodology to the zone packaged terminal unit objects.

The SZVAV model is now included in fan coil units and unitary systems. Regarding future maintenance it would be better to provide a common function for use by all equipment models. Since the SZVAV model uses specific array variables based on equipment type, a method to pass the necessary information via a function call is warranted. For example, a call to a common routine could pass all necessary information as independent variables:

    CalcSZVAVModel( EquipType, EquipName, QZnReq, Var1, Var2, Var3, Var4, ... );
    where:
    Var1 = max supply air temp
    Var2 = min supply air temp
    Var3 = min fan flow rate
    Var4 = max fan flow rate
    etc...

However, this could lead to many variables (16+) needed in the function call.

It may be better to pass an array of variables to minimize the function parameters:

    CalcSZVAVModel( EquipType, EquipName, QZnReq, SZVAVInputs, SZVAVOutputs );
    where:
    SZVAVInputs = an Array1D of all necessary information.
    SZVAVOutputs = an Array1D of information passed back to the model

Within the common routine would be a segregation of equipment types. For example, where an equipment model would need to be called, the calc routine specific to the equipment type is called.

    void CalcSZVAVModel (arg1, arg2, ... ) {

      Some intitial calculations ...

      { auto const ModelType( EquipType );
      if ( ModelType == ZoneHVAC:FourPipeFanCoil ) {
          Calc4PipeFanCoil( FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutNoHC, 0.0 ); // needs PLR=0 for electric heating coil, otherwise will run at full capacity
      } else if ( ModelType == AirloopHVAC:UnitarySystem ) {
          CalcUnitarySystemToLoad( UnitarySysNum, AirLoopNum, FirstHVACIteration, PartLoadRatio, 0.0, OnOffAirFlowRatio, TempSensOutput, TempLatOutput, HXUnitOn, _, _, CompressorONFlag );
      } else if ( ModelType == ZoneHVAC:PackagedTerminalAirConditioner ) {
          CalcPTUnit( PTUnitNum, FirstHVACIteration, PartLoadFrac, NoCompOutput, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, HXUnitOn );
      }}

      Other calculations as necessary ...

    }

The implementation of the SZVAV model is intended to make the code in UnitarySystem a stand-alone function where this function could be called by the ZoneHVAC:Packaged* equipment types. This code resides at lines 7126-7531 of HVACUnitarySystem.cc. I'm not yet sure how much extra work this would be to move this code to a common function but the rewards seem large.

Using this type of common function would allow improvements to all equipment types at the same time while keeping code logic local to a single function. Others may have ideas as how to make the SZVAV model a common routine for all equipment models. Ideas are welcome.


## Testing/Validation/Data Sources ##

Compare simulation results with existing equipment models. Document comparison of outlet temperature and humidity ratio and power consumption.

## Input Description ##

Limited changes to IDD and IO Reference documents as described herein. A single key field is added to the IDD for the PTAC/PTHP and reflected in the IO Reference document.

## Outputs Description ##

No change to output reporting

## IDD - Input Data Dictionary ##

Similar changes to ZoneHVAC:PackagedTerminalHeatPump.<br>
(Hint: no changes to existing object, new inputs are at end)


    ZoneHVAC:PackagedTerminalAirConditioner,
        \memo Packaged terminal air conditioner (PTAC).  Forced-convection heating-cooling unit
        \memo with supply fan, direct expansion (DX) cooling coil, heating coil (gas, electric, hot
        \memo water, or steam) and fixed-position outdoor air mixer.
        \min-fields 18
    A1, \field Name
        \required-field
        \type alpha
        \reference DOAToZonalUnit
        \reference ZoneEquipmentNames
        \note Unique name for this packaged terminal air conditioner object.
    A2, \field Availability Schedule Name
        \note Availability schedule name for this system. Schedule value > 0 means the system is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
        \note Schedule values of 0 denote the unit is off.
    A3, \field Air Inlet Node Name
        \required-field
        \type node
        \note Air inlet node for the PTAC must be a zone air exhaust Node.
    A4, \field Air Outlet Node Name
        \required-field
        \type node
        \note Air outlet node for the PTAC must be a zone air inlet node.
    A5, \field Outdoor Air Mixer Object Type
        \type choice
        \key OutdoorAir:Mixer
        \note Currently only one OutdoorAir:Mixer object type is available.
        \note This field should be left blank if the PTAC is connected to central
        \note dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
    A6, \field Outdoor Air Mixer Name
        \type object-list
        \object-list OutdoorAirMixers
        \note If this field is blank, the OutdoorAir:Mixer is not used.
        \note This optional field specifies the name of the OutdoorAir:Mixer object.
        \note When used, this name needs to match name of the OutdoorAir:Mixer object.
        \note This field should be left blank if the PTAC is connected to central
        \note dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
    N1,  \field Cooling Supply Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
        \note Must be less than or equal to fan size.
    N2, \field Heating Supply Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
        \note Must be less than or equal to fan size.
    N3, \field No Load Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to fan size.
        \note Only used when supply air fan operating mode schedule values specify continuous fan
        \note (schedule values greater than 0 specify continuous fan operation).
        \note This air flow rate is used when no heating or cooling is required and the cooling or
        \note heating coil is off. If this field is left blank or zero, the supply air flow rate
        \note from the previous on cycle (either cooling or heating) is used.
    N4, \field Cooling Outdoor Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to supply air flow rate during cooling operation.
        \note This field is set to zero flow when the PTAC is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
    N5, \field Heating Outdoor Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to supply air flow rate during heating operation.
        \note This field is set to zero flow when the PTAC is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
    N6, \field No Load Outdoor Air Flow Rate
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Only used when supply air fan operating mode schedule values specify continuous fan
        \note (schedule values greater than 0 specify continuous fan operation).
        \note This air flow rate is used when no heating or cooling is required and the cooling or
        \note heating coil is off. If this field is left blank or zero, the outdoor air flow rate
        \note from the previous on cycle (either cooling or heating) is used.
        \note This field is set to zero flow when the PTAC is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
    A7, \field Supply Air Fan Object Type
        \required-field
        \type choice
        \key Fan:SystemModel
        \key Fan:OnOff
        \key Fan:ConstantVolume
        \note Fan:ConstantVolume only works when continuous fan operation is used the entire
        \note simulation (all supply air fan operating mode schedule values are greater than 0).
        \note If any fan operating mode schedule values are 0 a Fan:SystemModel or Fan:OnOff object must be used.
    A8, \field Supply Air Fan Name
        \required-field
        \type object-list
        \object-list FansCVandOnOff
        \note Needs to match in the fan object.
    A9, \field Heating Coil Object Type
        \required-field
        \type choice
        \key Coil:Heating:Fuel
        \key Coil:Heating:Electric
        \key Coil:Heating:Water
        \key Coil:Heating:Steam
        \note Select the type of heating coil.
    A10, \field Heating Coil Name
        \required-field
        \type object-list
        \object-list HeatingCoilName
        \note Needs to match in the heating coil object.
    A11, \field Cooling Coil Object Type
        \required-field
        \type choice
        \key Coil:Cooling:DX:SingleSpeed
        \key Coil:Cooling:DX:VariableSpeed
        \key CoilSystem:Cooling:DX:HeatExchangerAssisted
        \note Select the type of Cooling Coil.
        \note Only works with Coil:Cooling:DX:SingleSpeed or
        \note CoilSystem:Cooling:DX:HeatExchangerAssisted or
        \note Coil:Cooling:DX:VariableSpeed.
    A12, \field Cooling Coil Name
        \required-field
        \type object-list
        \object-list CoolingCoilsDXSingleSpeed
        \object-list CoolingCoilsDXVariableSpeed
        \note Needs to match a DX cooling coil object.
    A13, \field Fan Placement
        \type choice
        \key BlowThrough
        \key DrawThrough
        \default DrawThrough
        \note Select fan placement as either blow through or draw through.
    A14, \field Supply Air Fan Operating Mode Schedule Name
        \type object-list
        \object-list ScheduleNames
        \note Enter the name of a schedule that controls fan operation. Schedule Name values of 0 denote
        \note cycling fan operation (fan cycles with cooling or heating coil). Schedule Name values greater
        \note than 0 denote constant fan operation (fan runs continually regardless of coil operation).
    A15, \field Availability Manager List Name
        \note Enter the name of an AvailabilityManagerAssignmentList object.
        \type object-list
        \object-list SystemAvailabilityManagerLists
    A16, \field Design Specification ZoneHVAC Sizing Object Name
        \note Enter the name of a DesignSpecificationZoneHVACSizing object.
        \type object-list
        \object-list DesignSpecificationZoneHVACSizingName


    New Inputs:
    A17, \field Capacity Control Method
         \type choice
         \key None
         \key SingleZoneVAV
         \default None
    N7 , \field Minimum Supply Air Temperature in Cooling Mode
         \note For Capacity Control Method = SingleZoneVAV, enter the minimum air temperature limit for reduced fan speed.
         \type real
         \minimum 0.0
         \autosizable
         \default autosize
    N8 ; \field Maximum Supply Air Temperature in Heating Mode
         \note For Capacity Control Method = SingleZoneVAV, enter the maximum air temperature limit for reduced fan speed.
         \type real
         \minimum 0.0
         \autosizable
         \default autosize

Question: Add to end of object or insert at a more appropriate place (e.g., before or after air flow fields)?

## Input Output Reference Documentation ##

Add new field descriptions.

## Engineering Reference ##

Add model calculations.

## Example File and Transition Changes ##

No transition required unless fields are inserted within existing object fields.

## References ##

NA

## Design Document ##

Lines 7126-7531 of HVACUnitarySystem.cc contain the latest model for SZVAV. This code will be moved to a unique function and revised to make all equipment specific data available for read/write.

For example:

The code at lines 7126-7531 will be moved to function CalcSZVAVModel. The function parameters for input and output will be passed using arrays. A reference to the equipment model array will also be passed to allow read/write of data to specific equipment models. The code in UnitarySystem starts like this. As you can see, the member variables are already set to local variables for use in the model. This is good and can preface the call to the common function in any equipment model. These variables will be written to the SZVAVInputs array as discussed later.

		// use the ASHRAE 90.1 method of reduced fan speed at low loads
		if ( UnitarySystem( UnitarySysNum ).simASHRAEModel ) {

			// set up mode specific variables to use in common function calls
			if ( CoolingLoad ) {
				maxCoilFluidFlow = UnitarySystem( UnitarySysNum ).MaxCoolCoilFluidFlow;
				maxOutletTemp = UnitarySystem( UnitarySysNum ).DOASDXCoolingCoilMinTout;
				minAirMassFlow = UnitarySystem( UnitarySysNum ).MaxNoCoolHeatAirMassFlow;
				maxAirMassFlow = UnitarySystem( UnitarySysNum ).MaxCoolAirMassFlow;
				lowSpeedFanRatio = UnitarySystem( UnitarySysNum ).LowSpeedCoolFanRatio;
				coilFluidInletNode = UnitarySystem( UnitarySysNum ).CoolCoilFluidInletNode;
				coilFluidOutletNode = UnitarySystem( UnitarySysNum ).CoolCoilFluidOutletNodeNum;
				coilLoopNum = UnitarySystem( UnitarySysNum ).CoolCoilLoopNum;

However, there are places where information needs to be passed to the model in other locations within the equipment model code. For example, in this section of the existing SZVAV model, a member variable (*CoilWaterFlowRatio) is set so it can be used elsewhere:

			if ( CoolingLoad ) { // Function CalcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
				// set the water flow ratio so water coil gets proper flow
				UnitarySystem( UnitarySysNum ).CoolCoilWaterFlowRatio = maxCoilFluidFlow / UnitarySystem( UnitarySysNum ).MaxCoolCoilFluidFlow;
				CalcUnitarySystemToLoad( UnitarySysNum, AirLoopNum, FirstHVACIteration, PartLoadRatio, 0.0, OnOffAirFlowRatio, TempSensOutput, TempLatOutput, HXUnitOn, _, _, CompressorONFlag );
			} else {
				UnitarySystem( UnitarySysNum ).HeatCoilWaterFlowRatio = maxCoilFluidFlow / UnitarySystem( UnitarySysNum ).MaxHeatCoilFluidFlow;
				CalcUnitarySystemToLoad( UnitarySysNum, AirLoopNum, FirstHVACIteration, 0.0, PartLoadRatio, OnOffAirFlowRatio, TempSensOutput, TempLatOutput, HXUnitOn, _, _, CompressorONFlag );
			}


To write to equipment model member variables, a reference to the member array would need to be passed to the function.

    auto & SZVAVModel( FanCoil[ FanCoilNum ] );
    auto & SZVAVModel( UnitarySystem[ UnitarySystemNum ] );
    auto & SZVAVModel( PTUnit[ PTUnitNum ] );

Where this reference would be passed to the function as an argument:

    void CalcSZVAVModel(
        Array1D< Real64 > SZVAVModel, // reference to equipment model
        static Array1D< Real64 > SZVAVInputs, // SZVAV inputs
        <Array1D< Real64 > SZVAVOutputs ) {} // SZVAV outputs

This use of a reference would require that the member variable names in FourPipeFanCoil, UnitarySystem, and PTUnit match. This method would also reduce or eliminate the size of or need for SZVAVInputs and SZVAVOutputs.

Once the common function is operational in UnitarySystem, the call to this new function will be added to the packaged terminal unit model.


## Design Document Conference Call##

Conference call held on Nov 27, 2017. Review of suggested changes brought up two options. Add base class to handle common variables among different equipment types or use templates to pass references of model structures.

Templates have some issues that need to be understood. One is that compilers can be picky about syntax and the other is that is can be harder to debug code since the instance of a template does not exist until runtime. For this reason, it was decided to go the route of the base class method.

To simplify the conversion, it was discussed that the code in question would be moved to a common location (e.g., General.cc). It was also suggested that implementation of the base class method not yet be undertaken until it is well understood what variables were needed in the base class for all affected equipment models (i,e., fan coil, unitary system and PTHP). The review team agreed that this method would be used going forward.
