NFP – Dedicated OA to supply and inlet side of zone HVAC units

FSEC, B. Nigusse, December 6, 2016, initial NFP
FSEC, B. Nigusse, December 22, 2016, Final NFP


#Justification for Feature Update:
In its current form, EnergyPlus 8.6 allow a central dedicated outdoor air system (DOAS) to provide conditioned outdoor air directly to the supply or the inlet side of *ZoneHVAC:FourPipeFanCoil*, *ZoneHVAC:WaterToAirHeatPump*, *ZoneHVAC:PackagedTerminalAirConditioner*, *ZoneHVAC:PackagedTerminalHeatPump*, *ZoneHVAC:TerminalUnit:VariableRefrigerantFlow*, or *AirLoopHVAC:UnitarySystemp* zoneHVAC equipment. Such an arrangement is commonly used in current building design practices and should be extended to other zoneHVAC equipment in EnergyPlus. While there is a credible work-around in EnergyPlus version 8.6 that is thermodynamically similar to the supply side delivery arrangement described above, the OA from the central DOAS is delivered to the zone directly instead of being mixed with the supply air at supply node of the zoneHVAC equipment. This effort will extend such capability to ***ZoneHVAC:UnitVentilator*** zoneHVAC equipment. The total airflow delivered to the zone is important to be established as it helps in sizing of the duct work and supply registers for the zone, and is also the more commonly used arrangement in practice. In addition to the total airflow, supply air and outdoor air conditions may also be set taking into account the mixing taking place upstream or downstream.

##Conference Call Conclusions:

###Other Conference Call Topics (not in scope of current proposal):

#Overview:
The arrangements shown in Figure 1 is supported in EnergyPlus version 8.6 for the following ZoneHVAC equipment only:

-**ZoneHVAC:FourPipeFanCoil**

-**ZoneHVAC:WaterToAirHeatPump**

-**ZoneHVAC:PackagedTerminalAirConditioner**

-**ZoneHVAC:PackagedTerminalHeatPump**

-**ZoneHVAC:TerminalUnit:VariableRefrigerantFlow**

-**AirLoopHVAC:UnitarySystemp**


A central DOAS provides fresh air to the zone mixed either at the inlet or outlet of a local zoneHVAC equipment that conditions the recirculation air.

![Figure 1 Air Terminal OA Mixer at inlet and supply side of each zoneHVAC equipment](DOAToZoneHVACEquipment.png)

To allow for this configuration, the terminal unit mixer existing objects, *AirTerminal:SingleDuct:Mixer* is used. The proposed task will add such capability to **ZoneHVAC:UnitVentilator** zoneHVAC equipment.

#Approach:
The *AirTerminal:SingleDuct:Mixer* object provides the zoneHVAC equipment object type and object name, primary air inlet node name, secondary air inlet node name, an air outlet node name, and terminal units connection type. Each mixer object will sum the two air streams and average the air properties of the two incoming streams and deliver mixed air either to the zone supply node or the inlet of the local zoneHVAC equipment. The total supply air delivered to a zone is equal to the sum of the primary air and the recirculating air. The primary air flow is balanced with the zone return air flow to meet the DOAS airloop mass balance requirement. This is done automatically in the subroutine *CalcZoneMassBalance*, module *ZoneEqupmentManager*. These terminal units resemble the induction type terminal units: they mix supply air with recirculated zone air and deliver the mixed air into a zone. The mixer objects will contain a reference to the upstream or downstream allowed zoneHVAC equipment. If present, the mixers will be simulated and will be able to pass the zone *load met* taking into account the outdoor air conditions to the local zoneHVAC equipment calculation routines.

**Connecting Unit Ventilator to DOA**

- Currently supported outdoor air control types for unit ventilator are: FixedAmount, VariablePercent and FixedTemperature. These Outdoor Air Control Types require the minimum and maximum outdoor air flow rates and the corresponding schedule input fields to work. The three outdoor air flow control types will be available when the unit ventilator is connected to DOA.

- Input fields: *Outdoor Air Node Name*, *Exhaust Air Node Name*, and *Mixed Air Node Name* in **ZoneHVAC:UnitVentilator** object are the node names that are required for *built-in* outdoor air mixer only. These three node names are not required if the UnitVentilator object is connected to DOA. Therefore, the required field restriction will be removed. The IDD will be modified to remove the restriction and appropriate note will be added for each of the three input fields. Eventually it is recommended to replace the "built-in" outdoor air mixer with a OutdoorAir:Mixer child object as it is done for other ZoneHVAC equipment.


###Testing/Validation/Data Source(s):
The new feature will be compared against exiting model.

##IO Ref (draft):


The IDD entry for **ZoneHVAC:UnitVentilator** object follows.

**ZoneHVAC:UnitVentilator,**

        \memo Unit ventilator. Forced-convection ventilation unit with supply fan (constant-volume
        \memo or variable-volume), optional chilled water cooling coil, optional heating coil
        \memo (gas, electric, hot water, or steam) and controllable outdoor air mixer.
        \min-fields 16

   A1 , \field Name

        \required-field

   A2 , \field Availability Schedule Name

        \note Availability schedule name for this system. Schedule value > 0 means the system is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames

   N1 , \field Maximum Supply Air Flow Rate
        \required-field
        \units m3/s
        \minimum> 0
        \autosizable

   A3 , \field Outdoor Air Control Type

        \required-field
        \type choice
        \key FixedAmount
        \key VariablePercent
        \key FixedTemperature
   N2 , \field Minimum Outdoor Air Flow Rate

        \required-field
        \units m3/s
        \autosizable
        \minimum 0.0
   A4 , \field Minimum Outdoor Air Schedule Name*

        \required-field
        \type object-list
        \object-list ScheduleNames
        \note schedule values multiply the minimum outdoor air flow rate
   N3 , \field Maximum Outdoor Air Flow Rate

        \required-field
        \units m3/s
        \minimum 0
        \autosizable
   A5 , \field Maximum Outdoor Air Fraction or Temperature Schedule Name

        \note that this depends on the control type as to whether it is a fraction or temperature
        \required-field
        \type object-list
        \object-list ScheduleNames
   A6 , \field Air Inlet Node Name

        \required-field
        \type node

   A7 , \field Air Outlet Node Name

        \required-field
        \type node

   *A8,  \field Outdoor Air Node Name*

        \required-field
        \type node
        \note Not used when the unit ventilator is connected to DOA
   *A9,  \field Exhaust Air Node Name*

        \required-field
        \type node
        \note Not used when the unit ventilator is connected to DOA
   *A10, \field Mixed Air Node Name*

        \required-field
        \type node
        \note inlet to coils
        \note Not used when the unit ventilator is connected to DOA
   A11, \field Supply Air Fan Object Type

        \required-field
        \type choice
        \key Fan:OnOff
        \key Fan:ConstantVolume
        \key Fan:VariableVolume
        \note Allowable fan types are Fan:ConstantVolume, Fan:OnOff and
        \note Fan:VariableVolume

   A12, \field Supply Air Fan Name

        \required-field
        \type object-list
        \object-list FansCVandVAV

...

   A20; \field Design Specification ZoneHVAC Sizing Object Name

        \note Enter the name of a DesignSpecificationZoneHVACSizing object.
        \type object-list
        \object-list DesignSpecificationZoneHVACSizingName


Example of Unit Ventilator served with dedicated outdoor air (DOA). The Outdoor Air Mixer object type and name input fields are left blank.

**ZoneHVAC:UnitVentilator,**

    UV1,                     !- Name
    UVAvailability,          !- Availability Schedule Name
    autosize,                !- Maximum Supply Air Flow Rate {m3/s}
    VariablePercent,         !- Outdoor Air Control Type
    autosize,                !- Minimum Outdoor Air Flow Rate {m3/s}
    UVMinOA,                 !- Minimum Outdoor Air Schedule Name
    autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}
    UnitVentMaxOA,           !- Maximum Outdoor Air Fraction or Temperature Schedule Name
    Zone1UVAirInletNode,     !- Air Inlet Node Name
    Zone1UVAirOutletNode,    !- Air Outlet Node Name
    ,         !- Outdoor Air Node Name (Not Required)
    ,         !- Exhaust Air Node Name (Not required, should be renamed to Relief Air Node)
    ,         !- Mixed Air Node Name (Not Required)
    Fan:ConstantVolume,      !- Supply Air Fan Object Type
    Zone1UnitVentFan,        !- Supply Air Fan Name
    HeatingAndCooling,       !- Coil Option
    ,                        !- Supply Air Fan Operating Mode Schedule Name
    Coil:Heating:Water,      !- Heating Coil Object Type
    Zone1UVHeatingCoil,      !- Heating Coil Name
    0.001,                   !- Heating Convergence Tolerance
    Coil:Cooling:Water,      !- Cooling Coil Object Type
    Zone1UVCoolingCoil,      !- Cooling Coil Name
    0.001;                   !- Cooling Convergence Tolerance


###AirTerminal:SingleDuct:Mixer
This terminal unit mixer object is used to mix conditioned outdoor air (primary air) from DOAS air loop and recirculating (secondary air) and deliver it either to inlet and supply side of a local zoneHVAC equipment. The terminal unit mixer can be connected either to the inlet or supply side of the local zoneHVAC equipment and the connection type is specified by a user in the input field ***Terminal Unit Connection Type.*** If the *AirTerminal:SingleDuct:Mixer* object is connected to the supply side, a mix of conditioned outdoor air from a central dedicated outdoor air system (DOAS) with conditioned recirculation air from the local zoneHVAC equipment is supplied as a single stream to the conditioned zone at its inlet node. If the *AirTerminal:SingleDuct:Mixer* object is connected to the inlet side, a mix of outdoor air from the a central dedicated outdoor air system (DOAS) with un-conditioned recirculation air from a zone exhaust node is supplied to the zoneHVAC equipment inlet node. The mixer will sum the two air streams and average the air properties of the two incoming streams.

**Field: Name**

Unique name for this air terminal mixer.

**Field: ZoneHVAC Unit Object Type**

The type of zoneHVAC equipment to which this terminal unit will be connected. This is a choice field. The choices are *ZoneHVAC:FourPipeFanCoil*, *ZoneHVAC:WaterToAirHeatPump*, ***ZoneHVAC:PackagedTerminalAirConditioner***, ***ZoneHVAC:PackagedTerminalHeatPump***, ***ZoneHVAC:TerminalUnit:VariableRefrigerantFlow***, ***AirLoopHVAC:UnitarySystem***, or ***ZoneHVAC:UnitVentilator***.

**Field: ZoneHVAC Unit Object Name**

The name of the zoneHVAC equipment to which this air terminal unit will be connected.

**Field: Mixer Outlet Node Name**

The name of the air outlet node of the mixer. This will be an inlet air node name of the conditioned zone if the connection type specified in the input field *Terminal Unit Connection Type* below is ***SupplySide.***  This will be an inlet air node of the zoneHVAC equipment if the connection type in the input field *Terminal Unit Connection Type* below is ***InletSide.***

**Field: Mixer Primary Air Inlet Node Name**

The name of the primary air (outdoor air) inlet node of the mixer. This will be an outlet node of an AirLoopHVAC:ZoneSplitter, providing the connection to the DOAS system.

**Field: Mixer Secondary Air Inlet Node Name**

The name of the secondary air (recirculating air) inlet node of the mixer. This will be the outlet air node name of the zoneHVAC equipment if the connection type in the input field *Terminal Unit Connection Type* below is ***SupplySide,*** or else this will be exhaust air node name of the zone that is being conditioned if the connection type in the input field *Terminal Unit Connection Type* below is ***InletSide.***

**Field: Mixer Connection Type**

This mixer connection type. Valid choices are *InletSide* or *SupplySide*. This is a required input field. If the mixer connection type selected is InletSide, then the mixer is connected on the inlet side of the ZoneHVAC equipment, or else if the mixer connection type selected is SupplySide, then the mixer is connected at the outlet side of the ZoneHVAC equipment.

The IDD entry for this object follows.

**AirTerminal:SingleDuct:Mixer,**

       \memo The mixer air terminal unit provides a means of supplying central system
       \memo air to the air inlet or outlet side of a zoneHAC equipment such as a four
       \memo pipe fan coil. Normally the central air would be ventilation air from a
       \memo dedicated outdoor air system (DOAS).

   A1, \field Name

       \required-field

   A2, \field ZoneHVAC Unit Object Type

       \required-field
       \type choice
       \key ZoneHVAC:FourPipeFanCoil
       \key ZoneHVAC:WaterToAirHeatPump
       \key ZoneHVAC:PackagedTerminalAirConditioner
       \key ZoneHVAC:PackagedTerminalHeatPump
       \key ZoneHVAC:TerminalUnit:VariableRefrigerantFlow
       \key ZoneHVAC:UnitVentilator
       \key AirLoopHVAC:UnitarySystem

   A3, \field ZoneHVAC Unit Object Name

       \required-field
       \type object-list
       \object-list DOAToZonalUnit

   A4, \field Mixer Outlet Node Name

       \required-field
       \type node

   A5, \field Mixer Primary Air Inlet Node Name

       \required-field
       \type node

   A6, \field Mixer Secondary Air Inlet Node Name

       \required-field
       \type node

   A7; \field Mixer Connection Type

       \required-field
       \type choice
       \key InletSide
       \key SupplySide

An example is shown below.

**AirTerminal:SingleDuct:Mixer,**

    SPACE4-1 DOAS Air Terminal,         !- Name
    ZoneHVAC:UnitVentilator,            !- ZoneHVAC Unit Object Type
    SPACE4-1 PTAC,                      !- ZoneHVAC Unit Object Name
    SPACE4-1 Supply Inlet,              !- Mixer Outlet Node Name
    SPACE4-1 AT Mixer Primary Inlet,    !- Mixer Primary Air Inlet Node Name
    SPACE4-1 Heat Pump Outlet,          !- Mixer Secondary Air Inlet Node Name
    SupplySide;                         !- Mixer Connection Type

**AirTerminal:SingleDuct:Mixer,**

    SPACE1-1 DOAS Air Terminal,         !- Name
    ZoneHVAC:UnitVentilator,            !- ZoneHVAC Unit Object Type
    SPACE1-1 PTHP,                      !- ZoneHVAC Unit Object Name
    SPACE1-1 Heat Pump Inlet,           !- Mixer Outlet Node Name
    SPACE1-1 ATMixer Primary Inlet,     !- Mixer Primary Air Inlet Node Name
    SPACE1-1 ATMixer Secondary Inlet,   !- Mixer Secondary Air Inlet Node Name
    InletSide;                          !- Mixer Connection Type


###Proposed Report Variables:
N/A

###Proposed additions to Meters:
N/A

##EngRef (draft):
<required>

###Example File and Transition changes:
New example file using both inlet and supply side OA mixing configurations will be provided for Unit Ventilator ZoneHVAC equipment. Transition is not required.

###Other documents:
None.


#Email Communication:
None.
