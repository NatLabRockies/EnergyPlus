Improved Support for Multi-speed Fans
================

**Lixing Gu**

**Florida Solar Energy Center**

 - 2/2/23 Design Document
 - 1/12/23 (Final version) Final NFP
 - 1/9/23 (Second revision)
 - 11/30/22 (First revision)
 - 11/21/22 (Original draft)


## Justification for New Feature ##

Many Water-to-Air Heat Pump units and VRF indoor units in the market are equipped with multi-speed components to reduce energy use and noise. During actual operation, the fan air flow is coordinated with the compressor stage control. At low compression stage, the fan is going to run at low air flow rate. To support this operation, supply air flow ratio input fields (low, medium, high) should be added to the ZoneHVAC:WaterToAirHeatPump and ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects. In simulation, for each time step, the fan air flow shall cycle between adjacent speeds based on the cooling / heating capacity (or coordinate with the cooling / heating coil stage for variable speed water to air coils).

## E-mail and  Conference Call Conclusions ##

Mike Witte made comments before ther conference call. The NFP was discussed in the conference call of EnergyPlus Technicalities on 11/30/22

###Mike's comments before the conference call on 11/30/22###

@mjwitte commented on this pull request.
________________________________________
In design/FY2023/NFP_MultispeedFans.md:
> +       \type choice
+       \key FanPerformance:Multispeed
+       \note Enter the type of performance specification object used to describe the multispeed fan.
+  	A21; \field Design Specification Multispeed Fan Object Name
+       \type object-list
+       \object-list FanPerformaceNames
+       \note Enter the name of the performance specification object used to describe the multispeed fan.
</span>
+
+###FanPerformance:Multispeed###
+
+This section provides inputs of the new object.
+

+
+An alternative way is to use the existing object of UnitarySystemPerformance:Multispeed. Since this object is not used only for UnitarySystem, the name of the object may be changed.
@lgu1234 I would support changing the existing object name rather than making a new object. Of course, that will require a transition rule for the existing field in AirloopHVAC:UnitarySystem.
________________________________________
In design/FY2023/NFP_MultispeedFans.md:
> +
+
+An alternative way is to use the existing object of UnitarySystemPerformance:Multispeed. Since this object is not used only for UnitarySystem, the name of the object may be changed.
+
+It should be pointed out that since this object mainly specify airflow ratios at different speeds, it is OK to use the object with minor name change.
+
</span>
+
+
+	FanPerformance:Multispeed,
+       \memo The FanPerformance object is used to specify the air flow ratio at each
+       \memo operating speed. This object is primarily used for ZoneHVAC:WaterToAirHeatPump and
+       \memo ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects to allow operation at
+       \memo different fan flow rates.
+       \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
@rraustad If this object is truly extensible, then the fields for Number of Speeds for Heating/Cooling should not have \maximum declared in the IDD.
________________________________________
In design/FY2023/NFP_MultispeedFans.md:
> +       \note Used only for Multi speed coils
+       \note Enter the next highest operating supply air flow ratio during heating
+       \note operation or specify autosize. This value is the ratio of air flow
+       \note at this speed to the maximum air flow rate.
These notes don't need to be repeated for every speed.

</span>

### EnergyPlus Technicalities on 11/30/22 ###

The conference call focused on the issue whether we need to use the exsting object of UnitarySystemPerformance:Multispeed or not, if the FanModel object can be used to simulate multiuple fan speeds. The suggestion is to provide comparison between two objects.

###Action item:###

Add a section to compare both objects to see whether FanSystemModel can be used to specify multispeed fans.

###Action response on 11/30/22###

@mjwitte @Myoldmopar @rraustad @EnergyArchmage

Based on discussion in the conference call today, I revised the NFP by adding a section to show differences between between UnitarySystemPerformance:Multispeed and Fan:SystemModel for review.

The Fan:SystemModel does not have No Load Supply Air Flow Rate Ratio, and different fan flow ratios between heating and cooling.

Justification
No Load Supply Air Flow Rate Ratio
As long as the No Load Supply Air Flow Rate is designed in the parent object correctly, there is no need to have this field.

Different fan flow ratios between heating and cooling
The real system has only a single supply fan.

Please let me know your comments via E-mail.

Thanks.

###Email communication on 1/4/23###

####My request on 1/4/22####

All:

Happy New Year! I came back to the US after Christmas. I started to work from today after a China trip.

The new feature proposal was discussed on Nov. 30, 2022. The main topic is if we use either Fan:SystemModel or UnitarySystemPerformance:Multispeed. I submited my response to show differences between two objects in https://github.com/NatLabRockies/EnergyPlus/pull/9746:

Based on discussion in the conference call today, I revised the NFP by adding a section to show differences between between UnitarySystemPerformance:Multispeed and Fan:SystemModel for review.
The Fan:SystemModel does not have No Load Supply Air Flow Rate Ratio, and different fan flow ratios between heating and cooling.
Justification
No Load Supply Air Flow Rate Ratio
As long as the No Load Supply Air Flow Rate is designed in the parent object correctly, there is no need to have this field.
Different fan flow ratios between heating and cooling
The real system has only a single supply fan.
Please let me know your comments via E-mail.
Thanks.
I would like to have your feedback to let me know which object is selected, so that I can move forward.
Thanks.
Gu

####Mike comments on 1/4/22####

@lgu1234 Using the speeds in Fan:SystemModel to inform the parent object is desirable, because it avoids duplicate input. But it would introduce a new paradigm.

My understanding is that none of the current parent objects mine this information from the fan. The parent objects decide on a flow rate and tell the fan where it's operating. Then the fan calculates it's power use based on interpolating between the speeds. @rraustad is that correct?

Since ZoneHVAC:WaterToAirHeatPump is actually using the UnitarySystem code, the most expedient approach seems to be to use UnitarySystemPerformance:Multispeed for this one. It should require minimal new code.

If UnitarySystem does not currently throw any warnings for inconsistent flow fractions between UnitarySystemPerformance:Multispeed and the Fan:SystemModel object, then I would suggest adding these checks as part of this PR.

For ZoneHVAC:TerminalUnit:VariableRefrigerantFlow it would then make sense to keep the same approach and use UnitarySystemPerformance:Multispeed.

Another alternative is to support both methods: If UnitarySystemPerformance:Multispeed is specified, then use it. If not, check the fan inputs and use those.

@rraustad @EnergyArchmage ?

###Action after Mike's comments###

The object of UnitarySystemPerformance:Multispeed will be used in this new feature for both ZoneHVAC:WaterToAirHeatPump and ZoneHVAC:TerminalUnit:VariableRefrigerantFlow.

Warnings will be issued if UnitarySystem does not currently throw any warnings for inconsistent flow fractions between UnitarySystemPerformance:Multispeed and the Fan:SystemModel object, but speed fractions from UnitarySystemPerformance:Multispeed will be used.

The alternative method will be included if I have time left: If UnitarySystemPerformance:Multispeed is specified, then use it. If not, check the fan inputs and use those.

###Discussion in the E+ Technicalities on 1/11/23###

NFP approach was discussed one more time. After the Technicalities, Rich and I also talked about approach for VRF teminal object. At the same time, Tianzhen send me an E-mail and informed me as below:

Lixing,

The version of VRF models ( AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl) developed by us collaborating with Daikin already can handle indoor fan speed control for indoor coil/unit. Your new feature should only apply to the VRF models developed by FSEC/Rich, which currently uses constant speed fan in indoor coil/units.

Let me know if you have questions.

Article attached fyi.

Tianzhen


## Overview ##

The ZoneHVAC:FourPipeFanCoil object allows multi-speed cycling fan with constant water flow rate. MultiSpeedFan is one of choices defined in the Capacity Control Method field. The object has two fields to defind Low Speed Supply Air Flow Ratio, and Medium Speed Supply Air Flow Ratio. Therefore, it has 3 fan speeds to be applied to the object. The fan air flow cycles between adjacent speeds based on the cooling/heating capacity. It is obvious that 3 speed fan operation may not be general enough. If a user wants more or less speed operation, more fields are needed. In addition, water flow rate remains the same with different air flow rates.

The AirLoopHVAC:UnitarySystem object allows multispeed coils with two fields input: Design Specification Multispeed Object Type and Design Specification Multispeed Object Name. The advatage to introduce an object to define multispeed coils is to provide more choices for users to define fan flow ratios with heating, cooling and no load conditions.

The proposed changes, to mimic AirLoopHVAC:UnitarySystem, are to add two optional fields in both ZoneHVAC:WaterToAirHeatPump and ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects to specify UnitarySystemPerformance:Multispeed as:

Design Specification Multispeed Object Type
Design Specification Multispeed Object Name

The alternative method will be included if I hvae time left: If UnitarySystemPerformance:Multispeed is specified, then use it. If not, check the fan inputs and use those.

### Final approach ###

This section provides final approach after several rounds of discussion.


#### ZoneHVAC:WaterToAirHeatPump ####

ZoneHVAC:WaterToAirHeatPump allows two types of coils: Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit and Coil:Heating/Cooling:WaterToAirHeatPump:VariableSpeedEquationFit.

#####Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit#####

Allow multispeed airflow rate. If UnitarySystemPerformance:Multispeed is specified, then use it. If not, check the fan inputs and use those.

#####Coil:Heating/Cooling:WaterToAirHeatPump:VariableSpeedEquationFit#####

The object already defines coil properties at different speeds. I need to check the speed numbers of cooling and heating defined in the UnitarySystemPerformance:Multispeed or Fan:SystemModel to ensure they are equal. The rest of procedure will be similar to AirToAir Multispeed HP or VariableSpeedCoils in the UnitarySystem

####ZoneHVAC:TerminalUnit:VariableRefrigerantFlow####

ZoneHVAC:TerminalUnit:VariableRefrigerantFlow allows two types of coils: Coil:Heating/Cooling:DX:VariableRefrigerantFlow and Coil:HeatingCooling:DX:VariableRefrigerantFlow:FluidTemperatureControl.

#####Coil:Heating/Cooling:DX:VariableRefrigerantFlow#####

Since the current ZoneHVAC:TerminalUnit:VariableRefrigerantFlow only allows single speed coils, the multiple speed fan will be allowed after adding the UnitarySystemPerformance:Multispeed fields. The fan air flow cycles between adjacent speeds based on the cooling/heating capacity for Coil:Cooling:DX:VariableRefrigerantFlow and Coil:Heating:DX:VariableRefrigerantFlow only. This approach is simlar to Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit

#####Coil:Heating/Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl#####

Since Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl and Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl require variable speed fans, the multiple speed fans are not applied to both coils. In addition, based on description provided by LBL, the coils can handle indoor fan speed control for indoor coil/unit. No effort is needed from this new feature.


## Approach ##

As mentioned in the previous section, two new optional fields are proposed in the both ZoneHVAC:WaterToAirHeatPump and ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects.

###ZoneHVAC:WaterToAirHeatPump###

Two optional new fields are added to allow a multiple speed fan.

	...
  	A21, \field Design Specification Multispeed Object Type
       \type choice
       \key UnitarySystemPerformance:Multispeed
       \note Enter the type of performance specification object used to describe the multispeed coil or fan.
  	A22; \field Design Specification Multispeed Object Name
       \type object-list
       \object-list UnitarySystemPerformaceNames
       \note Enter the name of the performance specification object used to describe the multispeed coil or fan.

If the entered heating coil type is Coil:Heating:WaterToAirHeatPump:EquationFit, the multiple fan speeds are allowed, the full water flow rate is assumed. The fan air flow cycles between adjacent speeds based on the cooling/heating capacity.

If the entered heating coil type is Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit, the input of Number of Speeds should be the same number defined in FanPerformance:Multispeed. The fan air flow speed coordinates with the cooling/heating coil stage for variable speed water to air coils.

If the entered cooling coil type is Coil:Heating:WaterToAirHeatPump:EquationFit, the multiple fan spees are allowed, the full water flow rate is assumed. The fan air flow cycles between adjacent speeds based on the cooling/heating capacity.

If the entered cooling coil type is Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit, the input of Number of Speeds should be the same number defined in FanPerformance:Multispeed. The fan air flow speed coordinates with the cooling/heating coil stage for variable speed water to air coils.

###ZoneHVAC:TerminalUnit:VariableRefrigerantFlow###

Two optional new fields are added to allow a multiple speed fan.

	...
  	A20, \field Design Specification Multispeed Object Type
       \type choice
       \key UnitarySystemPerformance:Multispeed
       \note Enter the type of performance specification object used to describe the multispeed coil or fan.
  	A21; \field Design Specification Multispeed Fan Object Name
       \type object-list
       \object-list UnitarySystemPerformaceNames
       \note Enter the name of the performance specification object used to describe the multispeed coil or fan.

Allowed heating and cooling coils:

	Coil:Cooling:DX:VariableRefrigerantFlow
	Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl
	Coil:Heating:DX:VariableRefrigerantFlow
	Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl

Since ZoneHVAC:TerminalUnit:VariableRefrigerantFlow only allows single speed coils, listed below, no multiple speed coils will be applied. The multiple speed fan will be allowed after adding the FanPerformance:Multispeed field. The fan air flow cycles between adjacent speeds based on the cooling/heating capacity for Coil:Cooling:DX:VariableRefrigerantFlow and Coil:Heating:DX:VariableRefrigerantFlow only.

Since Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl and Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl require variable speed fans, the multiple speed fans are not applied to both coils.

###Comparison between UnitarySystemPerformance:Multispeed and Fan:SystemModel###

For simplicity, the object inputs are listed in this section.

####Input of UnitarySystemPerformance:Multispeed####

The object inputs are extracted from UnitarySystem_MultiSpeedDX.idf.

  	UnitarySystemPerformance:Multispeed,
    Sys 1 Furnace DX Cool Unitary System MultiSpeed Performance,  !- Name
    1,                       !- Number of Speeds for Heating
    2,                       !- Number of Speeds for Cooling
    No,                      !- Single Mode Operation
    ,                        !- No Load Supply Air Flow Rate Ratio
    autosize,                !- Heating Speed 1 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 1 Supply Air Flow Ratio
    autosize,                !- Heating Speed 2 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 2 Supply Air Flow Ratio
    autosize,                !- Heating Speed 3 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 3 Supply Air Flow Ratio
    autosize,                !- Heating Speed 4 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 4 Supply Air Flow Ratio
    autosize,                !- Heating Speed 5 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 5 Supply Air Flow Ratio
    autosize,                !- Heating Speed 6 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 6 Supply Air Flow Ratio
    autosize,                !- Heating Speed 7 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 7 Supply Air Flow Ratio
    autosize,                !- Heating Speed 8 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 8 Supply Air Flow Ratio
    autosize,                !- Heating Speed 9 Supply Air Flow Ratio
    autosize,                !- Cooling Speed 9 Supply Air Flow Ratio
    autosize,                !- Heating Speed 10 Supply Air Flow Ratio
    autosize;                !- Cooling Speed 10 Supply Air Flow Ratio

####Input of Fan:SystemModel####

The object inputs are extracted from ASHRAE901_RetailStripmall_STD2019_Denver.idf.

  	Fan:SystemModel,
    PSZ-AC_1:1_addAQ_Unitary_Package_fan,  !- Name
    Type1_FAN_SCH,           !- Availability Schedule Name
    PSZ-AC_1:1_OA-PSZ-AC_1:1_Unitary_PackageNode,  !- Air Inlet Node Name
    PSZ-AC_1:1_Unitary_PackageHeatCoil air inlet,  !- Air Outlet Node Name
    AUTOSIZE,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    1,                       !- Electric Power Minimum Flow Rate Fraction
    622.5,                   !- Design Pressure Rise {Pa}
    0.895,                   !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    AUTOSIZE,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.58175,                 !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    622.5,                   !- Night Ventilation Mode Pressure Rise {Pa}
    1.0,                     !- Night Ventilation Mode Flow Fraction
    ,                        !- Motor Loss Zone Name
    ,                        !- Motor Loss Radiative Fraction
    General,                 !- End-Use Subcategory
    2,                       !- Number of Speeds
    0.66,                    !- Speed 1 Flow Fraction
    0.4,                     !- Speed 1 Electric Power Fraction
    1,                       !- Speed 2 Flow Fraction
    1;                       !- Speed 2 Electric Power Fraction

####Differences####

The Fan:SystemModel does provide multiple flow fration. The UnitarySystemPerformance:Multispeed object provides more choices than the Fan:SystemModel with No Load Supply Air Flow Rate Ratio, and different fan flow ratios between heating and cooling. The latter (different flow ratios between cooling and heating) can be justified, because the real system has only a single supply fan. The real missed item is No Load Supply Air Flow Rate Ratio. As long as the No Load Supply Air Flow Rate defined in the parent object is desinged correctly, the ratio input can be avoided.

## Testing/Validation/Data Sources ##

Unit test and example file test will be performed to ensure all simulations results meet expectation.

## Input Output Reference Documentation ##

This section provides description for the two modified objects with proposed two new optional fields, and for the new object.

...

## Input Description ##

This section provides input objects, two modified objects and a new object. Any new fields in the modified objects are highlighted in red.

###ZoneHVAC:WaterToAirHeatPump###

	ZoneHVAC:WaterToAirHeatPump,
        \memo Water-to-air heat pump. Forced-convection heating-cooling unit with supply fan,
        \memo water-to-air cooling and heating coils, supplemental heating coil (gas, electric, hot
        \memo water, or steam), and fixed-position outdoor air mixer.
        \min-fields 25
   	A1,  \field Name
        \required-field
        \type alpha
        \reference DOAToZonalUnit
        \reference ZoneEquipmentNames
    A2,  \field Availability Schedule Name
        \note Availability schedule name for this system. Schedule value > 0 means the system is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
   	A3,  \field Air Inlet Node Name
        \required-field
        \type node
   	A4,  \field Air Outlet Node Name
        \required-field
        \type node
   	A5,  \field Outdoor Air Mixer Object Type
        \type choice
        \key OutdoorAir:Mixer
        \note Currently only one OutdoorAir:Mixer object type is available.
        \note This field should be left blank if the WSHP is connected to central
        \note dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
   	A6,  \field Outdoor Air Mixer Name
        \type object-list
        \object-list OutdoorAirMixers
        \note If this field is blank, the OutdoorAir:Mixer is not used.
        \note This optional field specifies the name of the OutdoorAir:Mixer object.
        \note When used, this name needs to match name of the OutdoorAir:Mixer object.
        \note This field should be left blank if the WSHP is connected to central
        \note dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
   	N1 , \field Cooling Supply Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
        \note Must be less than or equal to fan size.
   	N2 , \field Heating Supply Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
        \note Must be less than or equal to fan size.
   	N3 , \field No Load Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to fan size.
        \note Only used when heat pump fan operating mode is continuous. This air flow rate
        \note is used when no heating or cooling is required and the DX coil compressor is off.
        \note If this field is left blank or zero, the supply air flow rate from the previous
        \note on cycle (either cooling or heating) is used.
   	N4 , \field Cooling Outdoor Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to supply air flow rate during cooling operation.
        \note This field is set to zero flow when the WSHP is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
   	N5 , \field Heating Outdoor Air Flow Rate
        \required-field
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Must be less than or equal to supply air flow rate during heating operation.
        \note This field is set to zero flow when the WSHP is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
   	N6 , \field No Load Outdoor Air Flow Rate
        \type real
        \units m3/s
        \minimum 0
        \autosizable
        \note Only used when heat pump Fan operating mode is continuous. This air flow rate
        \note is used when no heating or cooling is required and the DX coil compressor is off.
        \note If this field is left blank or zero, the outdoor air flow rate from the previous
        \note on cycle (either cooling or heating) is used.
        \note This field is set to zero flow when the PTHP is connected to central
        \note dedicated outdoor air through air terminal single duct mixer object.
   	A7,  \field Supply Air Fan Object Type
        \required-field
        \type choice
        \key Fan:SystemModel
        \key Fan:OnOff
   	A8,  \field Supply Air Fan Name
        \required-field
        \type object-list
        \object-list FansOnOff
        \object-list FansSystemModel
        \note Needs to match Fan:SystemModel or Fan:OnOff object
   	A9,  \field Heating Coil Object Type
        \required-field
        \type choice
        \key Coil:Heating:WaterToAirHeatPump:EquationFit
        \key Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit
   	A10, \field Heating Coil Name
        \required-field
        \type object-list
        \object-list HeatingCoilsWaterToAirHP
        \object-list HeatingCoilsWaterToAirVSHP
        \note  Needs to match in the water-to-air heat pump heating coil object
   	A11, \field Cooling Coil Object Type
        \required-field
        \type choice
        \key Coil:Cooling:WaterToAirHeatPump:EquationFit
        \key Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit
   	A12, \field Cooling Coil Name
        \required-field
        \type object-list
        \object-list CoolingCoilsWaterToAirHP
        \object-list CoolingCoilsWaterToAirVSHP
        \note Needs to match in the water-to-air heat pump cooling coil object
   	N7,  \field Maximum Cycling Rate
        \type real
        \units cycles/hr
        \minimum 0.0
        \maximum 5.0
        \default 2.5
        \note The maximum on-off cycling rate for the compressor
        \note Suggested value is 2.5 for a typical heat pump
   	N8,  \field Heat Pump Time Constant
        \type real
        \units s
        \minimum 0.0
        \maximum 500.0
        \default 60.0
        \note Time constant for the cooling coil's capacity to reach steady state after startup
        \note Suggested value is 60 for a typical heat pump
   	N9,  \field Fraction of On-Cycle Power Use
        \minimum 0.0
        \maximum 0.05
        \default 0.01
        \note The fraction of on-cycle power use to adjust the part load fraction based on
        \note the off-cycle power consumption due to crankcase heaters, controls, fans, and etc.
        \note Suggested value is 0.01 for a typical heat pump
   	N10, \field Heat Pump Fan Delay Time
        \units s
        \minimum 0.0
        \default 60
        \note Programmed time delay for heat pump fan to shut off after compressor cycle off.
        \note Only required when fan operating mode is cycling
        \note Enter 0 when fan operating mode is continuous
   	A13, \field Supplemental Heating Coil Object Type
        \required-field
        \type choice
        \key Coil:Heating:Fuel
        \key Coil:Heating:Electric
        \key Coil:Heating:Water
        \key Coil:Heating:Steam
        \note works with gas, electric, hot water and steam heating coils
   	A14, \field Supplemental Heating Coil Name
        \required-field
        \type object-list
        \object-list HeatingCoilName
        \note  Needs to match in the supplemental heating coil object
   	N11, \field Maximum Supply Air Temperature from Supplemental Heater
        \required-field
        \type real
        \units C
        \autosizable
        \default autosize
        \note Supply air temperature from the supplemental heater will not exceed this value.
   	N12, \field Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation
        \type real
        \maximum 21.0
        \default 21.0
        \units C
   	A15, \field Outdoor Dry-Bulb Temperature Sensor Node Name
        \type node
   	A16, \field Fan Placement
        \type choice
        \key BlowThrough
        \key DrawThrough
        \default BlowThrough
   	A17, \field Supply Air Fan Operating Mode Schedule Name
        \type object-list
        \object-list ScheduleNames
        \note Enter the name of a schedule that controls fan operation. Schedule values of 0 denote
        \note cycling fan operation (fan cycles with cooling or heating coil). Schedule values greater
        \note than 0 denote constant fan operation (fan runs continually regardless of coil operation).
        \note The fan operating mode defaults to cycling fan operation if this field is left blank.
   	A18, \field Availability Manager List Name
        \note Enter the name of an AvailabilityManagerAssignmentList object.
        \type object-list
        \object-list SystemAvailabilityManagerLists
   	A19, \field Heat Pump Coil Water Flow Mode
        \type choice
        \key Constant
        \key Cycling
        \key ConstantOnDemand
        \default Cycling
        \note used only when the heat pump coils are of the type WaterToAirHeatPump:EquationFit
        \note Constant results in 100% water flow regardless of compressor PLR
        \note Cycling results in water flow that matches compressor PLR
        \note ConstantOnDemand results in 100% water flow whenever the coil is on, but is 0% whenever the coil has no load
   	A20, \field Design Specification ZoneHVAC Sizing Object Name
        \note Enter the name of a DesignSpecificationZoneHVACSizing object.
        \type object-list
        \object-list DesignSpecificationZoneHVACSizingName

<span style="color:red">

  	A21, \field Design Specification Multispeed Object Type
       \type choice
       \key FanPerformance:Multispeed
       \note Enter the type of performance specification object used to describe the multispeed fan or coil.
  	A22; \field Design Specification Multispeed Object Name
       \type object-list
       \object-list FanPerformaceNames
       \note Enter the name of the performance specification object used to describe the multispeed fan or coil.
</span>

###ZoneHVAC:TerminalUnit:VariableRefrigerantFlow###

	ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,
        \memo A terminal unit with variable refrigerant flow (VRF) DX cooling and heating coils
        \memo (air-to-air heat pump). The VRF terminal units are served by an
        \memo AirConditioner:VariableRefrigerantFlow or
        \memo AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:* system.
        \memo Terminal units can be configured as zone, air loop or outside air system equipment.
        \min-fields 19
  	A1 ,  \field Zone Terminal Unit Name
        \required-field
        \type alpha
        \reference ZoneTerminalUnitNames
        \reference DOAToZonalUnit
        \reference ZoneEquipmentNames
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
        \reference-class-name validOASysEquipmentTypes
        \reference validOASysEquipmentNames
  	A2 ,  \field Terminal Unit Availability Schedule
        \type object-list
        \object-list ScheduleNames
        \note The unit is available the entire simulation if this field is left blank
        \note Schedule values of 0 denote the unit is off.
  	A3 ,  \field Terminal Unit Air Inlet Node Name
        \required-field
        \type node
        \note the inlet node to the terminal unit
  	A4 ,  \field Terminal Unit Air Outlet Node Name
        \required-field
        \type node
        \note the outlet node of the terminal unit
  	N1 ,  \field Cooling Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
  	N2 ,  \field No Cooling Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum 0.0
        \autosizable
  	N3 ,  \field Heating Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum> 0.0
        \autosizable
  	N4 ,  \field No Heating Supply Air Flow Rate
        \type real
        \units m3/s
        \minimum 0.0
        \autosizable
  	N5 ,  \field Cooling Outdoor Air Flow Rate
        \type real
        \units m3/s
        \minimum 0.0
        \autosizable
        \note This field is used only when an oudoor air mixer is included.
        \note This field is set to zero flow when the VRF terminal unit is connected to
        \note central dedicated outdoor air through air terminal single duct mixer object.
        \note When this VRF terminal is used as air loop equipment the autosized flow
        \note rate will be set to 0 when an outdoor air system is connected to this air loop,
        \note otherwise the outdoor air flow rate will equal the maximum outdoor air flow rate.
  	N6 ,  \field Heating Outdoor Air Flow Rate
        \type real
        \units m3/s
        \minimum 0.0
        \autosizable
        \note This field is used only when an oudoor air mixer is included.
        \note This field is set to zero flow when the VRF terminal unit is connected to
        \note central dedicated outdoor air through air terminal single duct mixer object.
        \note When this VRF terminal is used as air loop equipment the autosized flow
        \note rate will be set to 0 when an outdoor air system is connected to this air loop,
        \note otherwise the outdoor air flow rate will equal the maximum outdoor air flow rate.
  	N7 ,  \field No Load Outdoor Air Flow Rate
        \type real
        \units m3/s
        \minimum 0.0
        \autosizable
        \note This field is used only when an oudoor air mixer is included.
        \note This field is set to zero flow when the VRF terminal unit is connected to
        \note central dedicated outdoor air through air terminal single duct mixer object.
        \note When this VRF terminal is used as air loop equipment the autosized flow
        \note rate will be set to 0 when an outdoor air system is connected to this air loop,
        \note otherwise the outdoor air flow rate will equal the maximum outdoor air flow rate.
  	A5 ,  \field Supply Air Fan Operating Mode Schedule Name
        \type object-list
        \object-list ScheduleNames
        \note Required for zone equipment. Leave blank if terminal unit is used in AirLoopHVAC:OutdoorAirSystem:EquipmentList.
        \note Also leave blank if terminal unit is used on main AirloopHVAC branch and terminal unit has no fan.
  	A6 ,  \field Supply Air Fan Placement
        \type choice
        \key BlowThrough
        \key DrawThrough
        \default BlowThrough
        \note Select fan placement as either blow through or draw through.
        \note Required for zone equipment. This field is ignored if the VRF terminal unit is used
        \note in AirLoopHVAC:OutdoorAirSystem:EquipmentList.
        \note This field is also ignored if VRF terminal unit is used on main AirloopHVAC branch
        \note and terminal unit has no fan.
  	A7 ,  \field Supply Air Fan Object Type
        \type choice
        \key Fan:SystemModel
        \key Fan:OnOff
        \key Fan:ConstantVolume
        \key Fan:VariableVolume
        \default Fan:ConstantVolume
        \note Supply Air Fan Object Type must be Fan:SystemModel, Fan:OnOff, or Fan:ConstantVolume
        \note if AirConditioner:VariableRefrigerantFlow is used to model VRF outdoor unit
        \note Supply Air Fan Object Type must be Fan:SystemModel or Fan:VariableVolume if
        \note AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl or
        \note AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR
        \note is used to model VRF outdoor unit
        \note Required for zone equipment. Leave blank if terminal unit is used in AirLoopHVAC:OutdoorAirSystem:EquipmentList.
        \note Also leave blank if terminal unit is used on main AirloopHVAC branch and terminal unit has no fan.
  	A8 ,  \field Supply Air Fan Object Name
        \type object-list
        \object-list FansCVandOnOffandVAV
        \object-list FansSystemModel
  	A9 ,  \field Outside Air Mixer Object Type
        \type choice
        \key OutdoorAir:Mixer
        \note Currently only one type OutdoorAir:Mixer object is available.
        \note If this field is blank, and outside air mixer is not used.
        \note This field should be left blank if the VRF terminal unit is connected to
        \note central dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
        \note This field may also be left blank when the VRF terminal is used in the air
        \note loop or outdoor air system.
  	A10,  \field Outside Air Mixer Object Name
        \type object-list
        \object-list OutdoorAirMixers
        \note If this field is blank, the OutdoorAir:Mixer is not used.
        \note This optional field specifies the name of the OutdoorAir:Mixer object.
        \note When used, this name needs to match name of the OutdoorAir:Mixer object.
        \note This field should be left blank if the VRF terminal unit is connected to
        \note central dedicated outdoor air through an AirTerminal:SingleDuct:Mixer object.
        \note This field may also be left blank when the VRF terminal is used in the air
        \note loop or outdoor air system.
  	A11,  \field Cooling Coil Object Type
        \type choice
        \key Coil:Cooling:DX:VariableRefrigerantFlow
        \key Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl
        \note Cooling Coil Type must be Coil:Cooling:DX:VariableRefrigerantFlow
        \note if AirConditioner:VariableRefrigerantFlow is used
        \note to model VRF outdoor unit
        \note Cooling Coil Type must be
        \note Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl
        \note if AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl or
        \note if AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR
        \note is used to model VRF outdoor unit
        \note This field may be left blank if heating-only mode is used
  	A12,  \field Cooling Coil Object Name
        \type object-list
        \object-list CoolingCoilsDXVarRefrigFlow
        \object-list CoolingCoilsDXVarRefrigFlowFluidTemperatureControl
        \note Cooling Coil Type must be Coil:Cooling:DX:VariableRefrigerantFlow
        \note This field may be left blank if heating-only mode is used
  	A13,  \field Heating Coil Object Type
        \type choice
        \key Coil:Heating:DX:VariableRefrigerantFlow
        \key Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl
        \note Heating Coil Type must be Coil:Heating:DX:VariableRefrigerantFlow
        \note if AirConditioner:VariableRefrigerantFlow is used
        \note to model VRF outdoor unit
        \note Heating Coil Type must be
        \note Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl
        \note if AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl or
        \note if AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR
        \note is used to model VRF outdoor unit
        \note This field may be left blank if cooling-only mode is used
  	A14,  \field Heating Coil Object Name
        \type object-list
        \object-list HeatingCoilsDXVarRefrigFlow
        \object-list HeatingCoilsDXVarRefrigFlowFluidTemperatureControl
        \note Heating Coil Type must be Coil:Heating:DX:VariableRefrigerantFlow
        \note This field may be left blank if cooling-only mode is used
  	N8 ,  \field Zone Terminal Unit On Parasitic Electric Energy Use
        \type real
        \units W
        \minimum 0
        \default 0
  	N9 ,  \field Zone Terminal Unit Off Parasitic Electric Energy Use
        \type real
        \units W
        \minimum 0
        \default 0
  	N10, \field Rated Heating Capacity Sizing Ratio
       \type real
       \units W/W
       \minimum 1.0
       \default 1.0
       \note If this terminal unit's heating coil is autosized, the heating capacity is sized
       \note to be equal to the cooling capacity multiplied by this sizing ratio.
       \note This input applies to the terminal unit heating coil and overrides the sizing
       \note ratio entered in the AirConditioner:VariableRefrigerantFlow object.
  	A15, \field Availability Manager List Name
       \note Enter the name of an AvailabilityManagerAssignmentList object.
       \type object-list
       \object-list SystemAvailabilityManagerLists
  	A16, \field Design Specification ZoneHVAC Sizing Object Name
       \note Enter the name of a DesignSpecificationZoneHVACSizing object.
       \type object-list
       \object-list DesignSpecificationZoneHVACSizingName
  	A17, \field Supplemental Heating Coil Object Type
       \type choice
       \key Coil:Heating:Fuel
       \key Coil:Heating:Electric
       \key Coil:Heating:Water
       \key Coil:Heating:Steam
       \note works with gas, electric, hot water and steam heating coil.
  	A18, \field Supplemental Heating Coil Name
       \type object-list
       \object-list HeatingCoilName
       \note Needs to match in the supplemental heating coil object.
  	N11, \field Maximum Supply Air Temperature from Supplemental Heater
       \required-field
       \type real
       \units C
       \autosizable
       \default autosize
       \note Supply air temperature from the supplemental heater will not exceed this value.
  	N12, \field Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation
       \type real
       \maximum 21.0
       \default 21.0
       \units C
       \note Supplemental heater will not operate when outdoor temperature exceeds this value.
  	A19, \field Controlling Zone or Thermostat Location
       \type object-list
       \object-list ZoneNames
       \note Used only for AirloopHVAC equipment on a main branch and defines zone name where thermostat is located.
       \note Not required for zone equipment. Leave blank if terminal unit is used in AirLoopHVAC:OutdoorAirSystem:EquipmentList.
       \note Required when terminal unit is used on main AirloopHVAC branch and coils are not set point controlled.
       \note When terminal unit is used in air loop and is load controlled, this zone's thermostat will control operation.

<span style="color:red">

  	A20, \field Design Specification Multispeed Object Type
       \type choice
       \key FanPerformance:Multispeed
       \note Enter the type of performance specification object used to describe the multispeed fan or coil.
  	A21; \field Design Specification Multispeed Object Name
       \type object-list
       \object-list FanPerformaceNames
       \note Enter the name of the performance specification object used to describe the multispeed fan or coil.
</span>

## Outputs Description ##

insert text

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

An existing eample file will be modified by adding multiple fan performance. If not found, a new example file will be created and uploded in GitHub

## References ##

• https://www.shareddocs.com/hvac/docs/1005/Public/08/50PT-12PD.pdf Carrier - on page 28, it mentioned a 3-speed fan control and the control sequence during cooling and heating operation.

• https://files.climatemaster.com/TY_Unit_Spec.doc ClimateMaster - on Page 7, it mentioned about coordinating the fan air flow speed with compressor stage.

• https://tahoeweb.daikinapplied.com/api/general/DownloadDocumentByName/media/CAT_1114-12_WS_SS_GTH-GTV_026-072_LR.pdf/ Daikin - on page 7, it mentioned that unit will operate at lower fan speed under part load to reduce noise, energy and operating cost.

• https://www.daikinac.com/content/assets/DOC/Product%20Brochures/CT-VRV-Catalog-08-15.pdf Daikin - on page 67, the FXEQ-PVJU series terminal units have a 5 speed fan.

• https://www.lg.com//global/business/download/airsolution/2020 MULTI V Catalogue_EU[20200713_184653935].pdf LG indoor unit documentation on pages 55-80 shows 3-speed capabilities across multiple product lines.

• https://www.shareddocs.com/hvac/docs/1001/Public/0A/TCTC-E20-VRF007.pdf For Carrier VRF terminals, on page 2, the indoor unit has 3 speed fan control.

• https://www.shareddocs.com/hvac/docs/1001/Public/0B/A10-1604-3.pdf Toshiba-Carrier MMDB service manual. See pages 8 and 9 for relevant unit data. See pages 17 and 18 for multi-speed fan “auto” control explanations

## Design Document ##

This document covers 3 types of coils, implemented in two modules: UnitarySystem and HVACVariableRefrigerantFlow. The UnitarySystem covers two types of coils: Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit and Coil:Heating/Cooling:WaterToAirHeatPump:VariableSpeedEquationFit. The HVACVariableRefrigerantFlow covers a single type of coil: Coil:Heating/Cooling:DX:VariableRefrigerantFlow.

### Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit ###

Several functions will be modified in the UnitarySystem module to allow multiple airflow rates with the fixed water flow. The functions included GetInput, Sizing, and Report.

#### Sizing ####

If autosize is required, linear discrete size of fan flow rates are assigned. The cooling coil air flow size code is provide below. The heating coil flow size will be similaer.

               // airflow sizing
                Real64 AirFlowRate = WaterToAirHeatPumpSimple::GetCoilAirFlowRate(
                    state, DataHVACGlobals::cAllCoilTypes(this->m_CoolingCoilType_Num), this->m_CoolingCoilName, ErrFound);
                if (this->m_NumOfSpeedCooling > 1) {
                    for (int i = 1; i <= this->m_NumOfSpeedCooling; ++i) {
                        if (state.dataUnitarySystems->designSpecMSHP[this->m_DesignSpecMSHPIndex].coolingVolFlowRatio[i] == DataSizing::AutoSize) {
                            this->m_CoolVolumeFlowRate[i] = double(i) / double(this->m_NumOfSpeedCooling) * AirFlowRate;
                        } else {
                            this->m_CoolVolumeFlowRate[i] =
                                state.dataUnitarySystems->designSpecMSHP[this->m_DesignSpecMSHPIndex].coolingVolFlowRatio[i] * AirFlowRate;
                        }
                        this->m_CoolMassFlowRate[i] = this->m_CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                    }
                }

#### Input ####

##### Read inputs of UnitarySystemPerformance:Multispeed and corresponding two fields

                    if (getPTUnitType == 3) {
                        thisSys.m_DesignSpecMultispeedHPType =
                            ip->getAlphaFieldValue(fields, objectSchemaProps, "design_specification_multispeed_object_type");
                        thisSys.m_DesignSpecMultispeedHPName =
                            ip->getAlphaFieldValue(fields, objectSchemaProps, "design_specification_multispeed_object_name");
                        if (!thisSys.m_DesignSpecMultispeedHPType.empty() && !thisSys.m_DesignSpecMultispeedHPName.empty()) {
                            int designSpecType_Num = 1;
                            DesignSpecMSHP thisDesignSpec;
                            thisSys.m_CompPointerMSHP = thisDesignSpec.factory(state, designSpecType_Num, thisSys.m_DesignSpecMultispeedHPName);
                            thisSys.m_DesignSpecMSHPIndex = getDesignSpecMSHPIndex(state, thisSys.m_DesignSpecMultispeedHPName);
                        }
                    }

##### Set up arrays to store multuiple air flows and associated flags

Use existing array of multiple speed coils to set up heaiting and cooling volumatric and mass flow rates, speed ratios

                        this->m_HeatMassFlowRate[i] = 0.0;
                        this->m_HeatVolumeFlowRate[i] = 0.0;
                        this->m_MSHeatingSpeedRatio[i] = 1.0;

In addition, set up bool varibales as true:

                        this->m_MultiSpeedHeatingCoil = true;
                        this->m_MultiOrVarSpeedHeatCoil = true;

If the UnitarySystemPerformance:Multispeed object is not available, the program will check if the fan type is Fan:SystemModel with Discrete speed control or not. If yes, the multiple fan speed operation is also applied. Since the Fan:SystemModel only provideds the number of speeds without specification of heating and cooling, the same number for heating and cooling will be applied.

##### Set up airflow rate with auto size given in UnitarySystemPerformance:Multispeed #####

If the supply heating and cooling flow rates are given in the inputs, the multiple speed airflows will be determined in the input function with or without autosize. The procedure is similar with one provided in the sizing.

##### Set up corresponding output variables

                        SetupOutputVariable(state,
                                            "Unitary System Water Coil Cycling Ratio",
                                            OutputProcessor::Unit::None,
                                            state.dataUnitarySystems->unitarySys[sysNum].m_CycRatio,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataUnitarySystems->unitarySys[sysNum].Name);
                        SetupOutputVariable(state,
                                            "Unitary System Water Coil Speed Ratio",
                                            OutputProcessor::Unit::None,
                                            state.dataUnitarySystems->unitarySys[sysNum].m_SpeedRatio,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataUnitarySystems->unitarySys[sysNum].Name);
                        SetupOutputVariable(state,
                                            "Unitary System Water Coil Speed Level",
                                            OutputProcessor::Unit::None,
                                            state.dataUnitarySystems->unitarySys[sysNum].m_SpeedNum,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataUnitarySystems->unitarySys[sysNum].Name);

##### Report output variables

Assign corresponding values to these output variables.

        case DataHVACGlobals::Coil_HeatingWaterToAirHPSimple: {
            if (this->m_NumOfSpeedHeating > 1) {
                this->m_CycRatio = max(this->m_CoolingCycRatio, this->m_HeatingCycRatio);
                this->m_SpeedRatio = max(this->m_CoolingSpeedRatio, this->m_HeatingSpeedRatio);
                this->m_SpeedNum = max(this->m_CoolingSpeedNum, this->m_HeatingSpeedNum);
            }
            if (state.dataUnitarySystems->HeatingLoad) {
                this->m_TotalAuxElecPower =
                    this->m_AncillaryOnPower * this->m_PartLoadFrac + this->m_AncillaryOffPower * (1.0 - this->m_PartLoadFrac);
                this->m_HeatingAuxElecConsumption = this->m_AncillaryOnPower * this->m_PartLoadFrac * ReportingConstant;
            }
            if (this->m_LastMode == HeatingMode) {
                this->m_HeatingAuxElecConsumption += this->m_AncillaryOffPower * (1.0 - this->m_PartLoadFrac) * ReportingConstant;
            }

            elecHeatingPower = state.dataHVACGlobal->DXElecHeatingPower;
        } break;

###Coil:Heating/Cooling:WaterToAirHeatPump:VariableSpeedEquationFit###

The example file library does not provide the test file with selected Coil:Heating/Cooling:WaterToAirHeatPump:VariableSpeedEquationFit coil under ZoneHVAC:WaterToAirHeatPump. I modified an example file of DOAToWaterToAirInlet.idf to replace Coil:Heating/Cooling:WaterToAirHeatPump:EquationFit by Cooling:WaterToAirHeatPump:VariableSpeedEquationFit. Due to mismatch of airflows, although Cooling:WaterToAirHeatPump:VariableSpeedEquationFit coils have 10 speed, the parent object does not have such capability, so that no multiple speed airflows are simulateed.

The proposed revisions to adopt inputs of multiple speed fans are described below:

#### Modify input ####

The GetInput function will be modified to read two new fields and accept the UnitarySystemPerformance:Multispeed.
Since the coil specifies the number of speed. The number of speed from the UnitarySystemPerformance:Multispeed should match the number of speed from the coil. If not, a warning is issued and the number of speed from the coil will be used.

#### Check the number speeds from both the coil and UnitarySystemPerformance:Multispeed ####

The nuymber of speeds should match each other. If not, the number of speeds from coil prevails. In other words, the number of speed provided from UnitarySystemPerformance:Multispeed will be overridden. At the same time, a warning will be posted in the err file to let users be aware of the changes.

#### Craete corresponding output variables to present the speed number, cycle ratio and speed ratio, to be consistent with other multuiple speed coils and fans.

#### Modify the OutputControl function ####

A similar procedure with multipseed AirToAirHeatPump coil will be implemented.

###Coil:Heating/Cooling:DX:VariableRefrigerantFlow###

Modifications will be performed in the HVACVariableRefrigerantFlow module

#### GetVRFInputData ####

1. Read two new fields
2. Get index of UnitarySystemPerformance:Multispeed
3. Assign number of cooling and heating speeds from UnitarySystemPerformance:Multispeed
4. Create new arrays for volumatric and mass flow rates in size of number of cooling and heating speeds.
5. When the supply airflows are not autosize, assign airflows to the new array with or without autosize

        if (!lAlphaFieldBlanks(20) && !lAlphaFieldBlanks(21)) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMultispeedHPType = cAlphaArgs(20);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMultispeedHPName = cAlphaArgs(21);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex = UnitarySystems::getDesignSpecMSHPIndex(state, cAlphaArgs(21));
            auto &designSpecFan = state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex];
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DXCoolCoilType_Num == DataHVACGlobals::CoilVRF_Cooling) {
                int NumSpeeds = designSpecFan.numOfSpeedCooling;
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate.resize(NumSpeeds + 1);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate.resize(NumSpeeds + 1);
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
                    for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling; ++i) {
                        if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .coolingVolFlowRatio[i] == DataSizing::AutoSize) {
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                                double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling) * AirFlowRate;
                        } else {
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                                state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                    .coolingVolFlowRatio[i] *
                                AirFlowRate;
                        }
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                    }
                }
            }
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DXHeatCoilType_Num == DataHVACGlobals::CoilVRF_Heating) {
                int NumSpeeds = designSpecFan.numOfSpeedHeating;
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate.resize(NumSpeeds + 1);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate.resize(NumSpeeds + 1);
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
                    for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating; ++i) {
                        if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .heatingVolFlowRatio[i] == DataSizing::AutoSize) {
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                                double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating) * AirFlowRate;
                        } else {
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                                state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                    .heatingVolFlowRatio[i] *
                                AirFlowRate;
                        }
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                    }
                }
            }

        }


#### Sizing

The sizing of supply flow rates are determined in the sizing function. The rest of flow rates at different speeds are determined by flow fraction provided in UnitarySystemPerformance:Multispeed. If the UnitarySystemPerformance:Multispeed object fields are autosized, the proportional fraction ratios will be provided.

        // Multispeed Fan cooling flow sizing
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
            Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
            for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling; ++i) {
                if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex].coolingVolFlowRatio[i] ==
                    DataSizing::AutoSize) {
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                        double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling) * AirFlowRate;
                } else {
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                        state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                            .coolingVolFlowRatio[i] *
                        AirFlowRate;
                }
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] =
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
            }
        }


#### SetAverageAirFlow

The currect function set up average airflow is based on system on and off flow rate adjusted by part load ratio. When multiple speed fan is used, the average flow rates may be determined by speed ratio with two consecutive fan speeds. The procedure mimics the one from UnitarySystem.

If multiple speed fan is applied, the average air flow rate is given in two scenarios:

##### Speed Number = 1 #####

        AverageUnitMassFlow =
            (PartLoadRatio * state.dataHVACVarRefFlow->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataHVACVarRefFlow->CompOffMassFlow);

##### Speed Number > 1 #####

Cooling
        AverageUnitMassFlow =
            (SpeedRatio * state.dataHVACVarRefFlow->CoolMassFlowRate[CoolSpeedNum]) + ((1 - SpeedRatio) * state.dataHVACVarRefFlow->CoolMassFlowRate[CoolSpeedNum - 1]);

Heating
        AverageUnitMassFlow =
            (SpeedRatio * state.dataHVACVarRefFlow->HeatMassFlowRate[HeatSpeedNum]) + ((1 - SpeedRatio) * state.dataHVACVarRefFlow->HeatMassFlowRate[HeatSpeedNum - 1]);

#### CalcVRF ####

When multiple speed fan is applied, a for loop will be used to test which speed is used for the terminal unit. The procedure mimics one from HVACAirToAirHeatPump:MUltispeed.
