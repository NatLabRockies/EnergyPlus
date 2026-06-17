Outdoor Air Controller Cleanup - Issue #4663
================

**M.J. Witte, GARD Analytics, Inc.**

 - April 22, 2016
 - Revised April 28, 2016
 - Revised July 28-30, 2016 (see dated notes below)
 - Revised August 30, 2016 (back away from autosizing change for Controller:OutdoorAir Minimum Outdoor Air Flow Rate)

## Conference Calls and Other Discussion ##

e-mail comments
Questions about why `DesignSpecification:OutdoorAir` has a schedule in the first place?
It was part of the original object that was added to provide OA control for VAV terminal units (v6.0).

There was general support for modifying the objects (original proposal to move some fields from `Controller:MechanicalVentilation` into `Controller:OutdoorAir`.



April 27, 2016 - Sizing group conference call
Group consensus was to *not* reconfigure the objects right now, but just focus on fixing the items that are not working properly, fixing documentation, and some code refactoring.

There was a question about how many test suite diffs would be caused by activating the `DesignSpecification:OutdoorAir` OA schedule in `Controller:MechanicalVentilation`.  Only a few files would be impacted:  5ZoneAutoDXVAV.idf, AirflowNetwork_MultiZone_SmallOffice_VAV.idf, DOASDualDuctSchool.idf, HVACTemplate-5ZonePurchAir.idf


## Background ##

The current combination of Controller:OutdoorAir plus Controller:MechanicalVentilation can be confusing. Common frustrations that users encounter with these objects include:


1. Trying to set the OA flow to zero at night when using Controller:MechanicalVentilation
[Issue #4633](https://github.com/NatLabRockies/EnergyPlus/issues/4633) requires that the Controller:MechanicalVentilation schedule be set to zero as well as the Controller:OutdoorAir Minimum Outdoor Air Schedule (*if* the Controller:OutdoorAir Minimum Outdoor Air Flow Rate is > 0).

2. Specifying a schedule in DesignSpecification:OutdoorAir only to find that it is ignored by Controller:MechanicalVentilation.

3. Controller:MechanicalVentilation requires a redundant list of DesignSpecification:OutdoorAir and DesignSpecification:ZoneAirDistribution names which typically are already listed in the corresponding Sizing:Zone objects.  (The I/O Ref and an error message for omitted zones says that the information will be found from the Sizing:Zone objects but that code was removed some years back - not sure why.)

4.  Specifying a 100% outdoor air system is not obvious. The Minimum Outdoor Air Flow Rate and Maximum Outdoor Air Flow rate both need to match the system flow rate.  If the correct inputs are set in Sizing:System then these will autosize to the same value, but for some applications this does not work.

5.  Using VRP or DCV with Controller:MechanicalVentilation and autosizing the Controller:MechanicalVentilation Minimum Outdoor Air Flow Rate can result in a Minimum Outdoor Air Flow Rate always greater than the DCV or VRP OA flow, which overrides the DCV or VRP operation.

6. Uncertainties about what limits take precedence over others, especially the Controller:OutdoorAir Maximum Fraction of Outdoor Air Schedule which is described as a hardward limit which should take precedence over all other OA flow calcs, but it does not.

7. Some users want to disable economizer by time of day of year, but there is no option for that now.  They try using the "Time of Day Economizer Control Schedule" but it doesn't turn off the main economizer controls.

## Current Example Objects ##


```

 Controller:OutdoorAir,
    VAV WITH REHEAT_OA_Controller,  !- Name
    VAV WITH REHEAT_OARelief Node,  !- Relief Air Outlet Node Name
    VAV WITH REHEAT Supply Equipment Inlet Node,  !- Return Air Node Name
    VAV WITH REHEAT_OA-VAV WITH REHEAT_CoolCNode,  !- Mixed Air Node Name
    VAV WITH REHEAT_OAInlet Node,  !- Actuator Node Name
    AUTOSIZE,                !- Minimum Outdoor Air Flow Rate {m3/s}
    AUTOSIZE,                !- Maximum Outdoor Air Flow Rate {m3/s}
    NoEconomizer,            !- Economizer Control Type
    ModulateFlow,            !- Economizer Control Action Type
    19.0,                    !- Economizer Maximum Limit Dry-Bulb Temperature {C}
    32000.0,                 !- Economizer Maximum Limit Enthalpy {J/kg}
    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}
    ,                        !- Electronic Enthalpy Limit Curve Name
    0.0,                     !- Economizer Minimum Limit Dry-Bulb Temperature {C}
    NoLockout,               !- Lockout Type
    FixedMinimum,            !- Minimum Limit Type
    MinOA_MotorizedDamper_Sched,  !- Minimum Outdoor Air Schedule Name
    ,                        !- Minimum Fraction of Outdoor Air Schedule Name
    ,                        !- Maximum Fraction of Outdoor Air Schedule Name
    VAV WITH REHEAT_OAMechanical Ventilation,  !- Mechanical Ventilation Controller Name
    ,                        !- Time of Day Economizer Control Schedule Name
    ,                        !- High Humidity Control
    ,                        !- Humidistat Control Zone Name
    ,                        !- High Humidity Outdoor Air Flow Ratio
    ,                        !- Control High Indoor Humidity Based on Outdoor Humidity Ratio
    BypassWhenWithinEconomizerLimits;  !- Heat Recovery Bypass Control Type

 Controller:MechanicalVentilation,
    VAV WITH REHEAT_OAMechanical Ventilation,  !- Name
    ALWAYS_ON,               !- Availability Schedule Name
    Yes,                     !- Demand Controlled Ventilation
    VentilationRateProcedure,!- System Outdoor Air Method
    0.3,                     !- Zone Maximum Outdoor Air Fraction {dimensionless}
    ZN_1_FLR_1_SEC_1,        !- Zone 1 Name
    CM DSOA ZN_1_FLR_1_SEC_1,!- Design Specification Outdoor Air Object Name 1
    ZoneAirDistribution,     !- Design Specification Zone Air Distribution Object Name 1
    ZN_1_FLR_1_SEC_2,        !- Zone 2 Name
    CM DSOA ZN_1_FLR_1_SEC_2,!- Design Specification Outdoor Air Object Name 2
    ZoneAirDistribution,     !- Design Specification Zone Air Distribution Object Name 2
    ZN_1_FLR_1_SEC_3,        !- Zone 3 Name
    CM DSOA ZN_1_FLR_1_SEC_3,!- Design Specification Outdoor Air Object Name 3
    ZoneAirDistribution,     !- Design Specification Zone Air Distribution Object Name 3
    ZN_1_FLR_1_SEC_4,        !- Zone 4 Name
    CM DSOA ZN_1_FLR_1_SEC_4,!- Design Specification Outdoor Air Object Name 4
    ZoneAirDistribution,     !- Design Specification Zone Air Distribution Object Name 4
    ZN_1_FLR_1_SEC_5,        !- Zone 5 Name
    CM DSOA ZN_1_FLR_1_SEC_5,!- Design Specification Outdoor Air Object Name 5
    ZoneAirDistribution;     !- Design Specification Zone Air Distribution Object Name 5

 DesignSpecification:ZoneAirDistribution,
    ZoneAirDistribution,     !- Name
    1.2,                     !- Zone Air Distribution Effectiveness in Cooling Mode {dimensionless}
    0.8,                     !- Zone Air Distribution Effectiveness in Heating Mode {dimensionless}
    ,                        !- Zone Air Distribution Effectiveness Schedule Name
    0.0;                     !- Zone Secondary Recirculation Fraction {dimensionless}

 DesignSpecification:OutdoorAir,
    CM DSOA ZN_1_FLR_1_SEC_1,!- Name
    Sum,                     !- Outdoor Air Method
    0.0100,                  !- Outdoor Air Flow per Person {m3/s-person}
    0.0000,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}
    ,                        !- Outdoor Air Flow per Zone {m3/s}
    ,                        !- Outdoor Air Flow Air Changes per Hour {1/hr}
    MinOA_MotorizedDamper_Sched;  !- Outdoor Air Flow Rate Fraction Schedule Name

```

## Proposed/Actual Changes ##

x 1. No changes to `Controller:MechanicalVentilation` and `Controller:OutdoorAir` inputs.

x 2. In `Controller:MechanicalVentilation` make the zone names and design object names optional.  If all of the zones have a Sizing:Zone object, then the designspec info will be taken from there. *This would be consistent with current docs and warning messages.*

3.  *July 28, 2016* Change the autosizing for `Controller:OutdoorAir` Minimum Outdoor Air Flow Rate to be zero **if a Controller:MechanicalVentilation object is specified.**
~~if one of the advanced methods (VRP, DCV, IAQ) of control is selected.  *Or maybe autosize to the non-per-person flow?*~~
*August 30, 2016* This change was removed, postponed for further review.


x 4. Make the `Controller:OutdoorAir` Maximum Fraction of Outdoor Air Schedule Name be king - OA fraction can never be greater than the current schedule value.
    *July 29, 2016* **Order of Precedence for OA Flow Rate Limit Checks**
    - Minimum Outdoor Air Flow Rate * Minimum Outdoor Air Schedule
    - Apply economizer controls
    - OA flow rate >= Controller:MechanicalVentilation OA flow rate
    - OA flow rate >= System exhaust flow rate
    - (OA flow rate)/(Current Mixed air flow rate) >= Minimum Fraction of Outdoor Air Schedule
    - (OA flow rate)/(Current Mixed air flow rate) <= Maximum Fraction of Outdoor Air Schedule (even if this is smaller than Min Fraction  schedule)
    - OA flow rate <= Maximum Outdoor Air Flow Rate
    - Apply OA mass flow rate specified by demand limiting
    - Apply OA mass flow rate specified by EMS
    - OA flow rate <= Current mixed air flow rate (system flow rate) **previously this check was before demand limiting and EMS, but my understanding is that OA flow > Mixed air flow can cause problems further into the system simulation.**

x 5. Rename `DesignSpecification:OutdoorAir` "Outdoor Air Flow Rate Fraction Schedule Name" to "Outdoor Air Schedule Name" and use this schedule in `Controller:MechanicalVentilation`.  Currently it is ignored. *The primary goal here is to address the original issue that setting this schedule to zero should shut off OA.  There is a question of how this would be applied for the CO2 and IAQP methods.*
    - *July 30, 2016 Implementation notes*
    - OARequirements (which holds DesignSpecification:OutdoorAir)
    - OARequirements includes this variable: Real64 MaxOAFractionSchValue; // - Maximum value from OAFlow fraction schedule (used for sizing) which is set in SizingManager::ProcessInputOARequirements but it's never used. **Deleted**
    - DataZoneEquipment::CalcDesignSpecificationOutdoorAir calculates an OA flow rate but has arguments to control whether the DSOA Schedule is to be applied or not.
    - ZoneEquipmentManager:SetUpZoneSizingArrays calls CalcDesignSpecificationOutdoorAir with UseMinOASchFlag=false
    - AirTerminal:SingleDuct:VAV:NoReheat, AirTerminal:SingleDuct:VAV:Reheat, AirTerminal:DualDuct:VAV:OutdoorAir, ZoneHVAC:FourPipeFanCoil, and ZoneHVAC:IdealLoadsAirSystem call CalcDesignSpecificationOutdoorAir and always sets UseMinOASchFlag=true
    - When DesignSpecification:OutdoorAir input is processed, the choice of method will silently zero out all other OA rate inputs if one of the exclusive OA methods, such as Flow/Person, is used.
    - Controller:MechanicalVentilation does not use CalcDesignSpecificationOutdoorAir
    - Controller:MecahnicalVentilation does not store grab either the outdoor air method, but it checks it during init and calc
    - **New July 30, 2016** Add more columns to HVAC Sizing Summary table Demand Controlled Ventilation using Controller:MechanicalVentilation
    - **FIXED** Schedule name was not correct in above report for air distribution effectiveness when not specified, now is blank.


6. Change "Time of Day Economizer Control Schedule" to apply to any type of economizer control, and add a new Economizer Control Type = TimeOfDay.
*Not done, postponed for further review.*

x 7. Refactor MixedAir::CalcOAController to make separate functions for each control type.  Current CalcOAController is over 1000 lines long.

x 8. *August 30, 2016* Added new output variable "Air System Outdoor Air Mechanical Ventilation Requested Mass Flow Rate".

## Modified Objects ##


```
 DesignSpecification:OutdoorAir,
    CM DSOA ZN_1_FLR_1_SEC_1,!- Name
    Sum,                     !- Outdoor Air Method
    0.0100,                  !- Outdoor Air Flow per Person {m3/s-person}
    0.0000,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}
    ,                        !- Outdoor Air Flow per Zone {m3/s}
    ,                        !- Outdoor Air Flow Air Changes per Hour {1/hr}
    MinOA_MotorizedDamper_Sched;  !- **Outdoor Air Schedule Name**

```
