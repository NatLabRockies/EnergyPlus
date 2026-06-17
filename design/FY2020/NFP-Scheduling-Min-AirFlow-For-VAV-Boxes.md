#Scheduling Minimum Airflow for VAV Boxes#
==========================================================================

**Florida Solar Energy Center**

 - *B. Nigusse*

 - Original Date: December 5, 2019
 - Design Document: December 10, 2019
 - Updated NFP based on review comments: December 12, 2019

## Justification for Feature Update ##
Many inpatient and outpatient healthcare facilities in the US and around the world are governed by ASHRAE Standard 170 or similar standards that require minimum supply air flow rates during occupied mode operation by space type. In order to maintain thermal comfort, this minimum air flow rate may be exceeded and in perimeter spaces the design air flow can be twice as high the design minimum air flow. But during unoccupied mode operation, the minimum air flow based on space type can be either zero, the box minimum, or the minimum required to maintain air flow direction between spaces.

Although EnergyPlus supports minimum airflow schedule, this schedule applies to the calculated peak airflow in the zone. Users are currently unable to model the appropriate minimum airflow turndowns.

Adding a “Minimum Flow Schedule” that applies to the design minimum air flow, will allow users the flexibility to better model actual building operation.


###Conference Call Conclusions:
N/A

###Other Conference Call Topics (not in scope of current proposal):

####Github review comments and responses:

@mjwitte, Comment:
"Operating" is a bit confusing here. Even though the other inputs are the "design" minimum, they also are used for operation, and the existing "Minimum Air Flow Fraction Schedule Name" is primarily operational. It's only used for sizing if the other min flow fields are blank. How about "Minimum Air Flow Turndown Schedule Name" or "Minimum Air Flow Adjustment Schedule Name" or ???

Is this adjustment limited to zero to 1.0? Or would an adjustment >1.0 ever be applicable?

@Nigusse, Response:
I will use "Minimum Air Flow Turndown Schedule Name" as the new field name but will use "Adjustment" in the description. I am thinking to limit to 0.0 to 1.0. I think users should use the design minimum fraction or the design minimum flow if they desire scale it up.


@mjwitte, Comment:
Feel free to rename this confusing struct SysDesignParams and its associated name Sys.

@Nigusse, Response:
I will take the liberty to rename the struct when I get there. Thanks.

@mjwitte, Comment:
Same here, I've always found DamperDesignParams and Damper to be confusing.

@Nigusse, Response:
will do.

@mjwitte, Comment:
My initial thought is to apply it. The user can always set this to 1.0 for the Winter and Summer design day types. But this needs to be explained clearly that it will get used. But I'd be ok with not applying it. I guess it depends on the details of how this should be used relative to the referenced Standards.

@Nigusse, Response:
Nigusse 3 minutes ago Author Member
I agree, applying it gives users the flexibility. If they choose not to do, they have the option to specify different schedule values for Winter and Summer design days. I will add note in the I/O Ref documentation and even in the idd if necessary.


## Overview ##

All the six VAV air terminal units in EnergyPlus has input for specifying a constant minimum air flow fraction. This fraction is defined relative to the air terminal maximum air flow and used to determine the *design minimum* supply air flow. However, the "AirTerminal:SingleDuct:VAV:Reheat" and "AirTerminal:SingleDuct:VAV:NoReheat" have two more method of specifying the minimum flow input methods (1) *Fixed Minimum Air Flow Rate*, and (2) *Minimum Air Flow Fraction Schedule Name*.

The *Fixed Minimum Air Flow Rate* input method allows to enter a fixed minimum flow rate, and the *Minimum Air Flow Fraction Schedule Name* input method allows to enter a scheduled minimum air flow fractions. This existing minimum flow fraction applies the peak design flow rate only. The following two VAV air terminal objects have three different input methods for specifying the design minimum air flow:

 - **AirTerminal:SingleDuct:VAV:Reheat**
 - **AirTerminal:SingleDuct:VAV:NoReheat**

The following four VAV air terminal object types have only one input method for specifying the minimum air flow and that is *Zone Minimum Air Flow Fraction* input method and this fraction is defined relative to the air terminal maximum (peak) air flow.

 - **AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan**
 - **AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat**
 - **AirTerminal:SingleDuct:VAV:HeatAndCool:NoReheat**
 - **AirTerminal:DualDuct:VAV**

The **AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan** object minimum air flow is used when there is no load (or when the air terminal VAV fan is off). The use of the minimum air flow for this air terminal object is slightly different.

## Approach ##

The approach this enhancement implementation is to add a new input field that operates on the design minimum air flow. The value of this schedule multiplies the design minimum air flow. This enhancement gives users the flexibility to adjust the box design minimum air flow for the different building application needs.

### New Input Field ###
"Operating Minimum Air Flow Fraction Schedule Name"



### Pseudocode ###


#### Calculating of Operating (actual) Minimum Air Flow at the Air Terminal Box ####

Currently the design minimum air flow is determined from user specified minimum air flow fraction and the maximum (peak) air flow as follows:

***DesignMinimumFlowRate*** is the design minimum air flow determined based one of the three methods currently supported.

***DesignMinimumFlowRate = Sys.AirMassFlowRateMax * Sys.ZoneMinAirFrac***

This enhancement will allow adjusting the design minimum air flow to determine minimum air flow turndown using the fraction values specified in the new input field (Minimum Air Flow Turndown Schedule Name) as follows:

***MinimumTurnDownFlowRate = DesignMinimumFlowRate * TurnDownMinimumFraction***

or

***OperatingMinimumFlowRate = Sys.AirMassFlowRateMax * Sys.ZoneMinAirFrac * TurnDownMinimumFraction***

***TurnDownMinimumFraction*** is defined as the ratio of *Minimum TurnDown AirFlow* to the *Design Minimum AirFlow*. The *Minimum Air Flow Turndown Fraction*, which is obtained from the new input field and multiplies the design minimum air flow to calculate minimum turndown air flow.

If there is outdoor air flow required for ventilation, then the actual minimum will be the maximum of the minimum turndown air flow calculated above and the current outdoor air flow requirement as shown below:

***MinimumTurnDownFlowRate = max(MinimumTurnDownFlowRate, CurrentOAMassFlowRate)***


And this operating minimum air flow is checked against the minimum and maximum air flow of the air terminal boxes set during initialization at every iteration as shown below:

***MinimumTurnDownFlowRate = max(MinimumTurnDownFlowRate, SysInlet.AirMassFlowRateMinAvail)***

***MinimumTurnDownFlowRate = min(MinimumTurnDownFlowRate, SysInlet.AirMassFlowRateMaxAvail)***


The system inlet minimum available air flow rate will be the operating minimum and is calculated as follows:

***SysInlet.AirMassFlowRateMinAvail = Sys.AirMassFlowRateMax * Sys.ZoneMinAirFrac * TurnDownMinimumFraction***


The maximum air flow during reheat will not be adjusted by the new field, will remain the design minimum and is given by:

***MaxAirVolFlowRateDuringReheat = Sys.MaxAirVolFlowRate * Sys.ZoneMinAirFrac***

If the new input field is blank, then the **TurnDownMinimumFraction** variable will be set to 1.0.


##Testing/Validation/Data Source(s):
The new feature will be compared against exiting model. Unit test will be performed to ensure report variables values are correct. Also a new example file will be provided as needed.


##IO Ref (draft):

The new input field will be appended to the end of the existing air terminal VAV objects. Modified *AirTerminal:SingleDuct:VAV:Reheat* object is shown below as a sample.

The input field *Minimum Air Flow Turndown Schedule Name* fraction values (0.0 to 1.0) will be used to adjust the design minimum air flow turndowns (for example during unoccupied hours) by multiplying the design minimum air flow. These fractions also adjust the design minim air flow used for sizing purpose but users may choose not to so by specifying a fraction value of 1.0 for Summer and Winter Design Days. This field can be used with any of the three *zone minimum air flow input methods*.

  AirTerminal:SingleDuct:VAV:Reheat,

    VAV with Reheat System,  !- Name
    FanAndCoilAvailSched,    !- Availability Schedule Name
    Reheat Air Inlet Node,   !- Damper Air Outlet Node Name
    Damper Inlet Node,       !- Air Inlet Node Name
    0.47,                    !- Maximum Air Flow Rate {m3/s}
    Constant,                !- Zone Minimum Air Flow Input Method
    0.3,                     !- Constant Minimum Air Flow Fraction
    ,                        !- Fixed Minimum Air Flow Rate {m3/s}
    ,                        !- Minimum Air Flow Fraction Schedule Name
    Coil:Heating:Water,      !- Reheat Coil Object Type
    Reheat Coil 1,           !- Reheat Coil Name
    0.0013,                  !- Maximum Hot Water or Steam Flow Rate {m3/s}
    0.0,                     !- Minimum Hot Water or Steam Flow Rate {m3/s}
    Reheat Air Outlet Node,  !- Air Outlet Node Name
    0.001,                   !- Convergence Tolerance
    Normal,                  !- Damper Heating Action
    AutoCalculate,           !- Maximum Flow per Zone Floor Area During Reheat {m3/s-m2}
    AutoCalculate,           !- Maximum Flow Fraction During Reheat
    ,                        !- Maximum Reheat Air Temperature
    ,                        !- Design Specification Outdoor Air Object Name
    TurndownMinAirFlowSch;   !- Minimum Air Flow Turndown Schedule Name

  Schedule:Compact,

    TurndownMinAirFlowSch,   !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: Weekdays,           !- Field 2
    Until: 7:00,0.00,        !- Field 3
    Until: 17:00,0.50,       !- Field 4
    Until: 24:00,0.00,       !- Field 5
    For: SummerDesignDay WinterDesignDay, !- Field 6
    Until: 24:00,1.00,       !- Field 7
    For: Weekends Holidays CustomDay1 CustomDay2, !- Field 8
    Until: 24:00,0.00;       !- Field 9


###AirTerminal:SingleDuct:VAV:Reheat,

       \memo Central air system terminal unit, single duct, variable volume, with reheat coil (hot
       \memo water, electric, gas, or steam).
       \min-fields 18

  A1 , \field Name

       \required-field
       \reference AirTerminalUnitNames
       \reference AFNTerminalUnitNames

  A2 , \field Availability Schedule Name

       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames

  A3 , \field Damper Air Outlet Node Name

       \note the outlet node of the damper and the inlet node of the reheat coil
       \note this is an internal node to the terminal unit and connects the damper and reheat coil
       \required-field
       \type node

  A4 , \field Air Inlet Node Name

       \note the inlet node to the terminal unit and the damper
       \required-field
       \type node

  N1 , \field Maximum Air Flow Rate

       \required-field
       \units m3/s
       \minimum 0.0
       \autosizable

  A5 , \field Zone Minimum Air Flow Input Method

       \type choice
       \key Constant
       \key FixedFlowRate
       \key Scheduled
       \default Constant
       \note Constant = Constant Minimum Air Flow Fraction (a fraction of Maximum Air Flow Rate)
       \note FixedFlowRate = Fixed Minimum Air Flow Rate (a fixed minimum air volume flow rate)
       \note Scheduled = Scheduled Minimum Air Flow Fraction (a fraction of Maximum Air Flow

  N2 , \field Constant Minimum Air Flow Fraction

       \type real
       \autosizable
       \default autosize
       \note This field is used if the field Zone Minimum Air Flow Input Method is Constant
       \note If the field Zone Minimum Air Flow Input Method is Scheduled, then this field
       \note is optional; if a value is entered, then it is used for sizing normal-action reheat coils.
       \note If both this field and the following field are entered, the larger result is used.
       \note The values for autosizing are picked up from the Sizing:Zone input fields
       \note "Cooling Minimum Air Flow per Zone Floor Area", "Cooling Minimum Air Flow", and
       \note "Cooling Minimum Air Flow Fraction". If there is no sizing calculation a default of
       \note 0.000762 m3/s-m2 (0.15 cfm/ft2) is used.
       \note This constant minimum air flow fraction will be multiplied with the fractional values from input
       \note field "Minimum Air Flow Turndown Schedule Name".

  N3 , \field Fixed Minimum Air Flow Rate

       \type real
       \units m3/s
       \autosizable
       \default autosize
       \note This field is used if the field Zone Minimum Air Flow Input Method is FixedFlowRate.
       \note If the field Zone Minimum Air Flow Input Method is Scheduled, then this field
       \note is optional; if a value is entered, then it is used for sizing normal-action reheat coils.
       \note If both this field and the previous field are entered, the larger result is used.
       \note The values for autosizing are picked up from the Sizing:Zone input fields
       \note "Cooling Minimum Air Flow per Zone Floor Area", "Cooling Minimum Air Flow", and
       \note "Cooling Minimum Air Flow Fraction". If there is no sizing calculation a default of
       \note 0.000762 m3/s-m2 (0.15 cfm/ft2) is used.
       \note This fixed minimum air flow  will be multiplied with the fractional values from input
       \note field "Minimum Air Flow Turndown Schedule Name".

  A6 , \field Minimum Air Flow Fraction Schedule Name

       \type object-list
       \object-list ScheduleNames
       \note This field is used if the field Zone Minimum Air Flow Input Method is Scheduled
       \note Schedule values are fractions, 0.0 to 1.0.
       \note If the field Constant Minimum Air Flow Fraction is blank, then the average of the
       \note minimum and maximum schedule values is used for sizing normal-action reheat coils.
       \note This fraction will be multiplied with the fractional values from input field
       \note "Minimum Air Flow Turndown Schedule Name".

  A7 , \field Reheat Coil Object Type

       \required-field
       \type choice
       \key Coil:Heating:Water
       \key Coil:Heating:Electric
       \key Coil:Heating:Fuel
       \key Coil:Heating:Steam

  A8 , \field Reheat Coil Name

       \required-field
       \type object-list
       \object-list HeatingCoilName

  N4 , \field Maximum Hot Water or Steam Flow Rate

       \note Not used when reheat coil type is gas or electric
       \units m3/s
       \minimum 0.0
       \autosizable
       \ip-units gal/min

  N5 , \field Minimum Hot Water or Steam Flow Rate

       \note Not used when reheat coil type is gas or electric
       \units m3/s
       \minimum 0.0
       \default 0.0
       \ip-units gal/min

  A9 , \field Air Outlet Node Name

       \required-field
       \type node
       \note The outlet node of the terminal unit and the reheat coil.
       \note This is also the zone inlet node.

  N6 , \field Convergence Tolerance

       \type real
       \minimum> 0.0
       \default 0.001

  A10, \field Damper Heating Action

       \type choice
       \key Normal
       \key Reverse
       \key ReverseWithLimits
       \default ReverseWithLimits
       \note Normal means the damper is fixed at the minimum position in heating mode
       \note Reverse means the damper can open fully during reheat
       \note ReverseWithLimits means the damper will open partially during reheat
       \note as specified in the following 2 fields

  N7 , \field Maximum Flow per Zone Floor Area During Reheat

       \type real
       \units m3/s-m2
       \autosizable
       \default autosize
       \note Used only when Reheat Coil Object Type = Coil:Heating:Water and Damper Heating Action = ReverseWithLimits
       \note When autocalculating, the maximum flow per zone is set to 0.002032 m3/s-m2 (0.4 cfm/sqft)
       \note This optional field limits the maximum flow allowed in reheat mode.
       \note At no time will the maximum flow rate calculated here exceed the value of
       \note Maximum Air Flow Rate.

  N8 , \field Maximum Flow Fraction During Reheat

       \type real
       \autosizable
       \default autosize
       \note Used only when Reheat Coil Object Type = Coil:Heating:Water and Damper Heating Action = ReverseWithLimits
       \note When autocalculating, the maximum flow fraction is set to the ratio of
       \note 0.002032 m3/s-m2 (0.4 cfm/sqft) multiplied by the zone floor area and the
       \note Maximum Air Flow Rate.
       \note This optional field limits the maximum flow allowed in reheat mode.
       \note At no time will the maximum flow rate calculated here exceed the value of
       \note Maximum Air Flow Rate.

  N9 , \field Maximum Reheat Air Temperature

       \type real
       \units C
       \minimum> 0.0
       \note Specifies the maximum allowable supply air temperature leaving the reheat coil.
       \note If left blank, there is no limit and no default. If unknown, 35C (95F) is recommended.

  A11, \field Design Specification Outdoor Air Object Name

       \type object-list
       \object-list DesignSpecificationOutdoorAirNames
       \note When the name of a DesignSpecification:OutdoorAir object is entered, the terminal
       \note unit will increase flow as needed to meet this outdoor air requirement.
       \note If Outdoor Air Flow per Person is non-zero, then the outdoor air requirement will
       \note be computed based on the current number of occupants in the zone.
       \note At no time will the supply air flow rate exceed the value for Maximum Air Flow Rate.
       \note If this field is blank, then the terminal unit will not be controlled for outdoor air flow.

  A12; \field Minimum Air Flow Turndown Schedule Name

       \type object-list
       \object-list ScheduleNames
       \note This field adjusts the design minimum flow rate by multiplying it using this scheduled fraction
       \note values. This field can be used with any of the three "Zone Minimum Air Flow Input Method"
       \note Schedule values are fractions, 0.0 to 1.0. This field adjusts the minimum airflow turndown
       \note below the design minimum air flow and is intended for use with ASHRAE Standard 170.
       \note If this field is left blank, then the operating minimum air flow fraction value is set to 1.0


## Design Documentation ##

### Initial Changes
The following functions of SingleDuct module may be moved as member function as an incremental contribution to re-factor.

    void InitSys(int const SysNum, bool const FirstHVACIteration);
    void SizeSys(int const SysNum);
    void SimVAV(int const SysNum, bool const FirstHVACIteration, int const ZoneNum, int const ZoneNodeNum);
    void CalcOAMassFlow(int const SysNum, Real64 &SAMassFlow, Real64 &AirLoopOAFrac);
    void SimCBVAV(int const SysNum, bool const FirstHVACIteration, int const ZoneNum, int const ZoneNodeNum);
    void SimVAVVS(int const SysNum, bool const FirstHVACIteration, int const ZoneNum, int const ZoneNodeNum);
    void SimConstVol(int const SysNum, bool const FirstHVACIteration, int const ZoneNum, int const ZoneNodeNum);
    void CalcVAVVS(int const SysNum, bool const FirstHVACIteration, int const ZoneNode, int const HCoilType, Real64 const HWFlow,
                   Real64 const HCoilReq, int const FanType, Real64 const AirFlow, int const FanOn, Real64 &LoadMet);
    Real64 VAVVSCoolingResidual(Real64 const SupplyAirMassFlow, Array1<Real64> const &Par);
    Real64 VAVVSHWNoFanResidual(Real64 const HWMassFlow, Array1<Real64> const &Par);
    Real64 VAVVSHWFanOnResidual(Real64 const SupplyAirMassFlow,Array1<Real64> const &Par);
    Real64 VAVVSHCFanOnResidual(Real64 const HeatingFrac,Array1<Real64> const &Par);
    void UpdateSys(int const SysNum);
    void ReportSys(int const SysNum);

The following functions of DualDuct module may be moved as member function as an incremental contribution to re-factor.

    void InitDualDuct(int const DamperNum, bool const FirstHVACIteration);
    void SizeDualDuct(int const DamperNum);
    void SimDualDuctConstVol(int const DamperNum, int const ZoneNum, int const ZoneNodeNum);
    void SimDualDuctVarVol(int const DamperNum, int const ZoneNum, int const ZoneNodeNum);
    void SimDualDuctVAVOutdoorAir(int const DamperNum, int const ZoneNum, int const ZoneNodeNum);
    void CalcOAMassFlow(int const DamperNum, Real64 &SAMassFlow, Real64 &AirLoopOAFrac);
    void CalcOAOnlyMassFlow(int const DamperNum, Real64 &OAMassFlow, Optional<Real64> MaxOAVolFlow = _);
    void UpdateDualDuct(int const DamperNum);
    void ReportDualDuct(int const DamperNum); // unused1208


### SingleDuct

  SingleDuct.hh
  Modify struct "SysDesignParams"  to add two new member variables.

    struct SysDesignParams
    {
        // Members
        std::string SysName;      // Name of the Sys
        std::string SysType;      // Type of Sys ie. VAV, Mixing, Inducing, etc.
        int SysType_Num;          // Numeric Equivalent for System type
        std::string Schedule;     // Sys Operation Schedule
        int SchedPtr;             // Pointer to the correct schedule

        ...

        // add two new member variables
        int ZoneOperatingMinAirFracSchPtr;    // pointer to the schedule for operating minimum airflow fraction
        Real64 ZoneOperatingMinAirFrac;       // operating minimum airflow fraction value, multiplier of zone design minimum air flow

        // Default Constructor
        SysDesignParams()
            : SysType_Num(0), SchedPtr(0), ReheatComp_Num(0), ReheatComp_Index(0), ReheatComp_PlantType(0), Fan_Num(0), Fan_Index(0),
              ControlCompTypeNum(0), CompErrIndex(0), MaxAirVolFlowRate(0.0), AirMassFlowRateMax(0.0), MaxHeatAirVolFlowRate(0.0),
              HeatAirMassFlowRateMax(0.0), ZoneMinAirFracMethod(ConstantMinFrac), ZoneMinAirFrac(0.0), ZoneMinAirFracReport(0.0),
              ZoneFixedMinAir(0.0), ZoneMinAirFracSchPtr(0), ConstantMinAirFracSetByUser(false), FixedMinAirSetByUser(false), DesignMinAirFrac(0.0),
              DesignFixedMinAir(0.0), InletNodeNum(0), OutletNodeNum(0), ReheatControlNode(0), ReheatCoilOutletNode(0), ReheatCoilMaxCapacity(0.0),
              ReheatAirOutletNode(0), MaxReheatWaterVolFlow(0.0), MaxReheatSteamVolFlow(0.0), MaxReheatWaterFlow(0.0), MaxReheatSteamFlow(0.0),
              MinReheatWaterVolFlow(0.0), MinReheatSteamVolFlow(0.0), MinReheatWaterFlow(0.0), MinReheatSteamFlow(0.0), ControllerOffset(0.0),
              MaxReheatTemp(0.0), MaxReheatTempSetByUser(false), DamperHeatingAction(0), DamperPosition(0.0), ADUNum(0), FluidIndex(0), ErrCount1(0),
              ErrCount1c(0), ErrCount2(0), ZoneFloorArea(0.0), CtrlZoneNum(0), CtrlZoneInNodeIndex(0), ActualZoneNum(0),
              MaxAirVolFlowRateDuringReheat(0.0), MaxAirVolFractionDuringReheat(0.0), AirMassFlowDuringReheatMax(0.0), ZoneOutdoorAirMethod(0),
              OutdoorAirFlowRate(0.0), NoOAFlowInputFromUser(true), OARequirementsPtr(0), AirLoopNum(0), HWLoopNum(0), HWLoopSide(0),
              HWBranchIndex(0), HWCompIndex(0), SecInNode(0), IterationLimit(0), IterationFailed(0), OAPerPersonMode(0), EMSOverrideAirFlow(false),
              EMSMassFlowRateValue(0.0), HeatRate(0.0), CoolRate(0.0), HeatEnergy(0.0), CoolEnergy(0.0),

              ZoneOperatingMinAirFracSchPtr(0), ZoneOperatingMinAirFrac(0.0)

        {
        }

        void SimConstVolNoReheat(int const SysNum, bool const EP_UNUSED(FirstHVACIteration), int const EP_UNUSED(ZoneNum), int const ZoneNodeNum);
    };

Existing function GetSysInput() will be modified to get the new input field value for single duct VAV system.  A new member variable will be read and set user specified value.

Existing function InitSys() will be modified to calculate and set the operating minimum air flow rate using the new schedule fraction value.

The minimum operating air flow available will be determined as follows:

**Node(InletNode).MassFlowRateMinAvail = Sys.AirMassFlowRateMax * Sys.ZoneMinAirFrac * Sys.ZoneOperatingMinAirFrac**

Existing functions for simulating the various VAV air terminal units (such as SimVAV, SimVAVVS, SimCBVAV, SizeSys....) will be modified to apply the operating minimum air flow fraction as needed.


### DualDuct

  DualDuct.hh

  Modify struct "DamperDesignParams"  to add new new member variable

    struct DamperDesignParams
    {
        // Members
        std::string DamperName; // Name of the Damper
        //  CHARACTER(len=MaxNameLength) :: DamperType  = ' ' ! Type of Damper ie. VAV, Mixing, Inducing, etc.
        int DamperType;            // Type of Damper ie. VAV, Mixing, Inducing, etc.
        std::string Schedule;      // Damper Operation Schedule
        int SchedPtr;              // Pointer to the correct schedule

        ...

        // add two new member variables
        int ZoneOperatingMinAirFracSchPtr;    // pointer to the schedule for operating minimum airflow fraction
        Real64 ZoneOperatingMinAirFrac;       // operating minimum airflow fraction value, multiplier of zone design minimum airflow

        // Default Constructor
        DamperDesignParams()
            : DamperType(0), SchedPtr(0), MaxAirVolFlowRate(0.0), MaxAirMassFlowRate(0.0), HotAirInletNodeNum(0), ColdAirInletNodeNum(0),
              OutletNodeNum(0), ZoneMinAirFrac(0.0), ColdAirDamperPosition(0.0), HotAirDamperPosition(0.0), OAInletNodeNum(0),
              RecircAirInletNodeNum(0), RecircIsUsed(true), DesignOAFlowRate(0.0), DesignRecircFlowRate(0.0), OAControlMode(0),
              RecircAirDamperPosition(0.0), OADamperPosition(0.0), OAFraction(0.0), ADUNum(0), CtrlZoneNum(0), CtrlZoneInNodeIndex(0),
              ActualZoneNum(0), OutdoorAirFlowRate(0.0), NoOAFlowInputFromUser(true), OARequirementsPtr(0), OAPerPersonMode(PerPersonModeNotSet),
              OAPerPersonByDesignLevel(0.0), AirLoopNum(0),

              ZoneOperatingMinAirFracSchPtr(0), ZoneOperatingMinAirFrac(0.0)

          {
        }
    }

Existing function GetDualDuctInput() will be modified to get the new input field value for dual duct VAV system.  A new member variable will be read and set user specified value.

Existing function InitDualDuct() will be modified to calculate and set the damper operating minimum air flow rate using the new schedule fraction value.

The hot and cold deck minimum operating air flow available will be determined as follows:

**Node(OutletNode).MassFlowRateMin = Node(OutletNode).MassFlowRateMax * Damper.ZoneMinAirFrac * Damper.ZoneOperatingMinAirFrac**

**Node(HotInNode).MassFlowRateMinAvail = DamperHotAirInlet.AirMassFlowRateMax * Damper.ZoneMinAirFrac * Damper.ZoneOperatingMinAirFrac**

**Node(ColdInNode).MassFlowRateMinAvail = DamperColdAirInlet.AirMassFlowRateMax * Damper.ZoneMinAirFrac * Damper.ZoneOperatingMinAirFrac**

Existing function SimDualDuctVarVol() will be modified to calculate and set the damper operating minimum air flow rate.


### Sizing Issue

One obvious technical issue that needs to be discussed is that do we want to change the minimum air flow used for sizing the child objects in the air terminal objects. For example, should we size the reheat coil using the design minim air flow, or should we use the adjusted minimum air flow?

Based on github comments from @mjwitte, the fraction values from the new input field "Minimum Air Flow Turndown Schedule Name" will multiply the design minimum air flow used for sizing purposes and users will have the option to not to do so by appropriately scheduling a value of 1.0 for Winter and Summer Design Days.
