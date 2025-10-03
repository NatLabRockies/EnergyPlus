An Improved Duct Model for a Single AirLoop
================

**Lixing Gu**

 - 2/26/25
 - Third revision after coding
 - Add a new field A7 in the object Duct:Loss:Conduction to have scheduled W for mositure calculation
 - Specify leakage locations for trunks and braches
 - Remove supply leak calculation, because no T and W are changed. The leakage losses will be added as a part of system loads.
 - 2/11/25
 - Second revision after discussion with Jason DeGraw
 - 1/8/25
 - First revision after Technicalities call on 1/8/25
 - 12/25/2024
 - New Feature Proposal
 

## Justification for New Feature ##

EnergyPlus is being used by many tools around the world and used as the calculation basis for hundreds of thousands of residential homes leading to billions of dollars of federal energy efficiency rebates in coming years. One aspect that potentially holds back the impact of these studies is related to duct simulation in EnergyPlus. There is skepticism about the accuracy of the model when we cannot simulate certain duct physics in a simplified manner. Although the airflow network model can handle ducts well, we need a duct model that captures duct heat transfer without having to build out a full airflow network.

The task will improve duct modeling support to enable conduction heat losses for a single air loop, and also duct leakage for a single air loop. The duct modeling roadmap was developed previously and should be used as guidance for the scope of these features.

## E-mail and Conference Call Conclusions ##

###The NFP was discussed in the Technicalities conference call on 1/8/25###

Here are comments and replies.

Rich: How do you handle mass flow rates in supply and return leaks.

Reply: The mass flow rates for supply and return leaks are obtained from Duct:Loss:Leakage. A supply leak is terminated into a zone or outdoors, while a return leak originates from a zone or outdoors. However, the duct mass flow rate is assumed to be equivalent to the fan flow rate, mirroring the calculation for the OA mixer mass flow rate. The energy conservation is ensured.

Scott: Use exiting field of Heat Transmittance Coefficient (U-Factor) (Field N6) in AirflowNetwork:Distribution:Component:Duct

Reply: I agree. Remove the change.

Jason: May have comments via E-mail communications

Reply: I am ready for further dicussion. 

### Discussion between Jason on 02/09/25###

Jason DeGraw and Lixing Gu met in the ASHRAE Winter Meeting 2025 in Orlando. No further comments from Jason. 

## Overview ##

The roadmap was developed and accepted by the team in 2024. The section provides a brief history. Since the implemetation will be divided into several tasks, the section aslo specifies the implementation for the first task. 

###Roadmap history###

NREL requested FSEC to implement an improved duct model. I submitted a new feature proposal to the team on December 15, 2023 (NFP-DuctHeatTransfer.md). The objective was to design and implement the code changes required to support a new duct model, or modify the existing AFN duct model, to enable duct heat transfer to be captured without requiring an airflow network setup.

The team disagreed with the approach because it did not align with existing AFN model inputs. Since the proposed approach was restricted to heat transfer only, it was better to have a comprehensive plan addressing all potential model coverage. Consequently, the work shifted towards developing a duct model development roadmap. The first submission of the roadmap was on January 20, 2024. After several rounds of discussion, the roadmap was accepted by the team on March 6, 2024. The implementation work will be divided into several tasks.

###implementation###

The current submission aims to provide an implementation approach to enable conduction heat losses for a single air loop, as well as address duct leakage for a single air loop. The work presented here is Task 1.

## Approach ##

All the proposed inputs and IDD changes were finalized in the roadmap. To provide a comprehensive overview of this task, these proposed changes are presented repeatedly in this NFP.

The following figure represents proposed duct configuration in a single AirLoop.

![duct model](DuctLoss.PNG)

Note: Any air movement from a zone to outdoors is not presented in the figure, becuase exfiltration is not a part of zone energy balance.

### Leakage losses without mass flow changes ###

Assumptions:

1. The leakage loss calculation is based on energy transfer. In other words, no mass flows are involved in both AirLoopHVAC and zones. For example, when a supply leak is applied, the supply fan flow rate remains the same, the changes may be outlet equivalent temperatures and humidity ratios

2. Any energy losses for supply leak occur in the source duct and target zone or outdoor air node only. No more extension to other ducts and zones.

3. Any energy losses for return leak occur in the source zone or outdoor air node and and target duct only. No more extension to other ducts and zones.

4. Any air movement between zones caused by duct leakage can be accomplished by make up air.


## Testing/Validation/Data Sources ##

The outputs will be compared to Excel calculation results to ensure numerical values are correct.

## Input Output Reference Documentation ##

This section provides input descriptions for the 3 new objects: Duct:Loss:Conduction, Duct:Loss:Leakage, and Duct:Loss:MakeupAir.

\subsection{Duct:Loss:Conduction}\label{ductlossconduction}

This object allows users to input ducts with conduction loss only for simplified duct model. The object is used without using Airflow Network model exclusively. 

\subsubsection{Field: Name}\label{field-name-duct-000}

The name identifies the duct for later reference and in the output listing. Each duct loss should have a unique name.

\subsubsection{Field: AirLoopHAVC Name}\label{airloophavcname}

The name of an AirLoopHVAC object, which is also defined in a \hyperref[airloophvac]{AirLoopHVAC} object.

\subsubsection{Field: Zone Name}\label{field-zone-name-1-000}

The name of a zone object, which is also defined in a \hyperref[zone]{Zone} object.

\subsubsection{Field: AirflowNetwork:Distribution:Linkage Name}\label{airflownetworkdistributionlinkagename}

The name of an AirflowNetwork:Distribution:Linkage object, which is also defined in a \hyperref[airflownetworkdistributionlinkage]{AirflowNetwork:Distribution:Linkage} object.

\subsubsection{Field: Environment Type}\label{environmenttype}

Environment type is the environment in which the duct is placed. It can be either Zone or Schedule.
If specified as Zone, a zone name must be specified in the next field. If specified as Schedule, the Ambient
Temperature Zone can be left blank, while a schedule must be specified for the temperature.

\subsubsection{Field: Ambient Zone Name}\label{ambienttemperaturezonename}

If Zone is specified as the environment type, this field is used to specify the name of the zone in which
the duct is located. The zone temperature is used to calculate the heat transfer rate from the duct.

\subsubsection{Field: Ambient Temperature Schedule Name}\label{ambienttemperatureschedulename}

If Schedule is specified as the environment type, this field is used to specify the name of the
temperature schedule that gives the ambient air temperature surrounding the duct. This temperature is
used as the outside boundary condition to calculate heat transfer from the duct.

\subsubsection{Field: Ambient Humidity Ratio Schedule Name}\label{ambienttemperatureschedulename}

If Schedule is specified as the environment type, this field is used to specify the name of the
humidity ratio schedule that gives the ambient air humidity ratio surrounding the duct. This humidity ratio is
used as the outside boundary condition to calculate moisture transfer from the duct.

An IDF example is provided below:

\begin{lstlisting}

	Duct:Loss:Conduction,
      Main duct,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Mail Duct Linkage,   !- AirflowNetwork:Distribution:Linkage Name
      Zone,         !- Environment Type
   	  Zone 1,       !- Ambient Zone Name
      ,             !- Ambient Temperature Schedule Name
      ;             !- Ambient Humidity Ratio Schedule Name

\end{lstlisting}

\subsection{Duct:Loss:Leakage}\label{ductlossleakage}

This object allows users to input duct leaks only for simplified duct model. The object is used without using Airflow Network model exclusively. 

\subsubsection{Field: Name}\label{field-name-duct-000}

The name identifies the duct leak for later reference and in the output listing. Each duct leak should have a unique name.

\subsubsection{Field: AirLoopHAVC Name}\label{airloophavcname}

user assigned name for a single instance of an AirLoopHVAC object. Any other object referencing this AirLoopHVAC will use this name

The name of a AirLoopHVAC object, which is also defined in a \hyperref[airloophvac]{AirLoopHVAC} object.

\subsubsection{Field: AirflowNetwork:Distribution:Linkage Name}\label{airflownetworkdistributionlinkagename}

The name of an AirflowNetwork:Distribution:Linkage object, which is also defined in a \hyperref[airflownetworkdistributionlinkage]{AirflowNetwork:Distribution:Linkage} object.

An IDF example is provided below:

\begin{lstlisting}

	Duct:Loss:Leakage,
      Main duct leak,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Mail Duct Leak;  \field AirflowNetwork:Distribution:Linkage Name

\end{lstlisting}

\subsection{Duct:Loss:MakeupAir}\label{ductlossmakeupair}

This object allows users to input makeup air caused by supply leaks only for simplified duct model. The object is used without using Airflow Network model exclusively. 

\subsubsection{Field: Name}\label{field-name-duct-000}

The name identifies the makeup air for later reference and in the output listing. Each duct leak should have a unique name.

\subsubsection{Field: AirLoopHAVC Name}\label{airloophavcname}

user assigned name for a single instance of an AirLoopHVAC object. Any other object referencing this AirLoopHVAC will use this name

The name of a AirLoopHVAC object, which is also defined in a \hyperref[airloophvac]{AirLoopHVAC} object.

\subsubsection{Field: AirflowNetwork:Distribution:Linkage Name}\label{airflownetworkdistributionlinkagename}

The name of an AirflowNetwork:Distribution:Linkage object, which is also defined in a \hyperref[airflownetworkdistributionlinkage]{AirflowNetwork:Distribution:Linkage} object.

An IDF example is provided below:

\begin{lstlisting}

	Duct:Loss:MakeupAir,
      Makeup air,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Duct Leak Makeup;  \field AirflowNetwork:Distribution:Linkage Name

\end{lstlisting}

## Input Description ##

This section presnts inputs of new objects and modified existing objects.

### General inputs ###

This section presents inputs used in losses of both conduction and leakage, extracted from the roadmap.

#### 3 new objects ####

Three new objects has been proposed. Each object represents each loss type explicitly. 

	Duct:Loss:Conduction,
   	A1,  \field Name
        \required-field
   	A2,  \field AirLoopHAVC Name
        \required-field
        \type object-list
        \object-list AirPrimaryLoops
   	A3;  \field AirflowNetwork:Distribution:Linkage Name
        \required-field
        \type object-list
        \object-list AirflowNetworkDistributionLinkageNames
    A4,  \field Environment Type
        \type choice
        \key Zone
        \key Schedule
        \default Zone
   	A5,  \field Ambient Zone Name
        \type object-list
        \object-list ZoneNames
   	A6,  \field Ambient Temperature Schedule Name
        \type object-list
        \object-list ScheduleNames
   	A7;  \field Ambient Humidity Ratio Schedule Name
        \type object-list
        \object-list ScheduleNames

	Duct:Loss:Leakage,
   	A1,  \field Name
        \required-field
   	A2,  \field AirLoopHAVC Name
        \required-field
        \type object-list
        \object-list AirPrimaryLoops
   	A3;  \field AirflowNetwork:Distribution:Linkage Name
        \required-field
        \type object-list
        \object-list AirflowNetworkDistributionLinkageNames

	Duct:Loss:MakeupAir,
   	A1,  \field Name
        \required-field
   	A2,  \field AirLoopHAVC Name
        \required-field
        \type object-list
        \object-list AirPrimaryLoops
   	A3;  \field AirflowNetwork:Distribution:Linkage Name
        \required-field
        \type object-list
        \object-list AirflowNetworkDistributionLinkageNames
 
#### Existing object AirflowNetwork:Distribution:Node ####

One more choice is added as "Zone" in the Component Object Type or Node Type field. The added choice is highlighted in red.

	AirflowNetwork:Distribution:Node,
      \min-fields 4
      \memo This object represents an air distribution node in the AirflowNetwork model.
 	A1 , \field Name
      \required-field
      \type alpha
      \reference AirflowNetworkNodeAndZoneNames
      \note Enter a unique name for this object.
 	A2 , \field Component Name or Node Name
      \type alpha
      \note Designates node names defined in another object. The node name may occur in air branches.
      \note Enter a node name to represent a node already defined in an air loop.
      \note Leave this field blank if the Node or Object Type field below is entered as
      \note AirLoopHVAC:ZoneMixer, AirLoopHVAC:ZoneSplitter, AirLoopHVAC:OutdoorAirSystem, or Other.
 	A3 , \field Component Object Type or Node Type
      \type choice
      \key AirLoopHVAC:ZoneMixer
      \key AirLoopHVAC:ZoneSplitter
      \key AirLoopHVAC:OutdoorAirSystem
      \key OAMixerOutdoorAirStreamNode
      \key OutdoorAir:NodeList
      \key OutdoorAir:Node
      \key Other
<span style="color:red">

      \key Zone
</span>

      \default Other
      \note Designates Node type for the Node or Component Name defined in the field above.
      \note AirLoopHVAC:ZoneMixer -- Represents a AirLoopHVAC:ZoneMixer object.
      \note AirLoopHVAC:ZoneSplitter -- Represents a AirLoopHVAC:ZoneSplitter object.
      \note AirLoopHVAC:OutdoorAirSystem -- Represents an AirLoopHVAC:OutdoorAirSystem object.
      \note OAMixerOutdoorAirStreamNode -- Represents an external node used in the OutdoorAir:Mixer
      \note OutdoorAir:NodeList -- Represents an external node when a heat exchanger is used before
      \note the OutdoorAir:Mixer
      \note OutdoorAir:Node -- Represents an external node when a heat exchanger is used before
      \note the OutdoorAir:Mixer
      \note Other -- none of the above, the Node name already defined in the previous field is part
      \note of an air loop.
      \note Zone -- Enter a zone name for duct simple model to calculate duct leakage loss.
 	N1 ; \field Node Height
      \type real
      \units m
      \default 0.0
      \note Enter the reference height used to calculate the relative pressure.


Note for AirflowNetwork:Distribution:Node: 

1. Component Name or Node Name

The field of Component Name or Node Name is either Air Node name or Zone name. If a Zone name or an outdoor air node is entered, this node is used as a return leakage source or a supply leak target.

2. N1 field are ignored

Since the proposed feature is used for duct energy losses from conduction and leakage, N1 field is not used in energy loss calculation. The outdoor air node is allowed as either leakage source or target.

3. A new choice of A3 field is added as zone name. When AFN is fully implemented, the zone name is defined in AirflowNetwork:MultiZone:Zone. For the simplified duct model without using AFN, the added new choice can be used to define a zone for duct leakage calculation. Therefore, there is no need to use AirflowNetwork:MultiZone:Zone.  

#### Existing object AirflowNetwork:Distribution:Linkage ####

	AirflowNetwork:Distribution:Linkage,
      \min-fields 4
      \memo This object defines the connection between two nodes and a component.
 	A1 , \field Name
      \required-field
      \type alpha
      \note Enter a unique name for this object.
      \reference AirflowNetworkDistributionLinkageNames
 	A2 , \field Node 1 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node or Air Node.
 	A3 , \field Node 2 Name
      \required-field
      \type object-list
      \object-list AirflowNetworkNodeAndZoneNames
      \object-list AirNodeAndZoneNames
      \note Enter the name of zone or AirflowNetwork Node or Air Node.
 	A4 , \field Component Name
      \required-field
      \type object-list
      \object-list AirflowNetworkComponentNames
      \object-list FansCVandOnOffandVAV
      \object-list AFNCoilNames
      \object-list AFNHeatExchangerNames
      \object-list AFNTerminalUnitNames
      \note Enter the name of an AirflowNetwork component. A component is one of the
      \note following AirflowNetwork:Distribution:Component objects: Leak, LeakageRatio,
      \note Duct, ConstantVolumeFan, Coil, TerminalUnit, ConstantPressureDrop, or HeatExchanger.
 	A5 ; \field Thermal Zone Name
      \type object-list
      \object-list ZoneNames
      \note Only used if component = AirflowNetwork:Distribution:Component:Duct
      \note The zone name is where AirflowNetwork:Distribution:Component:Duct is exposed. Leave this field blank if the duct
      \note conduction loss is ignored.

Note for AirflowNetwork:Distribution:Linkage: 

1. Conduction

If duct conduction loss is simulated, the selected Component Name (A4) should be AirflowNetwork:Distribution:Component:Duct. Node 1 and Node 2 names should be Air Node names, provided in the list of AirflowNetwork:Distribution:Node.

2. Leakage

If duct leakage loss is simulated, the selected Component Name should be AirflowNetwork:Distribution:Component:LeakageRatio. The input of Effective Leakage Ratio field should be a fraction of mass flow rate of the AirLoopHVAC. The other field inputs are not used. One of nodes should be either zone name or outdoor air node name.

If a supply leak is defined, Node 1 name should be either an air node of zone inlet node for supply branch leak or a splitter for supply trunk leak, Node 2 name should be either a zone name or an outdoor air node name.

If a return leak is defined, Node 1 name should be either a zone name or an outdoor air node name, Node 2 name should be either an air node of zone outlet for return branch leak or a zone mixer for return trunk leak.

The above specification to define supply and retunr leaks is used to catch mass flow rate in either trunks or branches.

3. Restriction

Although there is no restriction of the number of ducts and locations, it is proposed for Task 1 to have limits as follows. 

3.1 There is a single duct used for SupplyTrunk and ReturnTrunk.

The connection of SupplyTrunk is between the inlet node (AirloopHVAC Demand Side Inlet Node) of AirLoopHVAC and the AirLoopHVAC:ZoneSplitter inlet node. If AirLoopHVAC:ZoneMixer is available, The connection of ReturnTrunk is between AirLoopHVAC:ZoneMixer outlet node and the outlet node (AirloopHVAC Demand Side Outlet Node) of AirLoopHVAC.

3.2 Each branch has a single duct.

The connection of SupplyBranch is between AirLoopHVAC:ZoneSplitter outlet node and one of the Air terminal inlet node. If AirLoopHVAC:ZoneMixer is available, The connection of ReturnBranch is between the zone outlet node and the AirLoopHVAC:ZoneMixer inlet node.

### Conduction loss ###

The AirflowNetwork:Distribution:Component:Duct object is used to calculate duct conduction loss.

#### Existing object for duct conduction loss as AirflowNetwork:Distribution:Component:Duct ####

	AirflowNetwork:Distribution:Component:Duct,
      \min-fields 8
      \memo This object defines the relationship between pressure and air flow through the duct.
 	A1 , \field Name
      \required-field
      \type alpha
      \reference AirflowNetworkComponentNames
      \note Enter a unique name for this object.
 	N1 , \field Duct Length
      \required-field
      \type real
      \units m
      \minimum> 0.0
      \note Enter the length of the duct.
 	N2 , \field Hydraulic Diameter
      \required-field
      \type real
      \units m
      \minimum> 0.0
      \note Enter the hydraulic diameter of the duct.
      \note Hydraulic diameter is defined as 4 multiplied by cross section area divided by perimeter
 	N3 , \field Cross Section Area
      \required-field
      \type real
      \units m2
      \minimum> 0.0
      \note Enter the cross section area of the duct.
 	N4 , \field Surface Roughness
      \type real
      \units m
      \default 0.0009
      \minimum> 0.0
      \note Enter the inside surface roughness of the duct.
 	N5 , \field Coefficient for Local Dynamic Loss Due to Fitting
      \type real
      \units dimensionless
      \default 0.0
      \minimum 0.0
      \note Enter the coefficient used to calculate dynamic losses of fittings (e.g. elbows).
 	N6 , \field Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction
      \note conduction only
      \type real
      \units W/m2-K
      \minimum> 0.0
      \default 0.943
      \note Default value of 0.943 is equivalent to 1.06 m2-K/W (R6) duct insulation.
 	N7 , \field Overall Moisture Transmittance Coefficient from Air to Air
      \type real
      \units kg/m2
      \minimum> 0.0
      \default 0.001
      \note Enter the overall moisture transmittance coefficient
      \note including moisture film coefficients at both surfaces.
 	N8 , \field Outside Convection Coefficient
      \note optional. convection coefficient calculated automatically, unless specified
      \type real
      \units W/m2-K
      \minimum> 0.0
 	N9 ; \field Inside Convection Coefficient
      \note optional. convection coefficient calculated automatically, unless specified
      \type real
      \units W/m2-K
      \minimum> 0.0

Note:

1. Keep N6

Although the change was proposed in the roadmap, Field N7 will be kept based on suggestion by Scott.

2. Keep N6 Overall Moisture Transmittance Coefficient from Air to Air

Although I can require to add one more field for moisture diffusivity in the Material object, this property is driven by humidity ratio. As we know, heat transfer is driven by the temperature difference, while the mositure transfer is driven by partial vapor pressure difference. The humidity ratio, as an independent variable, may not be proper.

Let's keep the field for the time being. We may need to think to use the partial vapor pressure as driving force to simulate moisture performance across walls. 

3. The Component:Duct is the only component listed in the Component Name defined in the Linkage object for conduction loss calculation.

4. The Component:LeakageRatio is the only component listed in the Component Name defined in the Linkage object for conduction leakage calculation.

An example of objects used to calculate duct condiuction loss in an IDF is:

\begin{lstlisting}

	Duct:Loss:Conduction,
      Main duct,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Mail Duct Linkage,   !- AirflowNetwork:Distribution:Linkage Name
      Zone,         !- Environment Type
   	  Zone 1,       !- Ambient Temperature Zone Name
      ;             !- Ambient Temperature Schedule Name

  	AirflowNetwork:Distribution:Node,
      EquipmentOutletNode,      !- Name
      Equipment outlet node,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Node,
      SplitterInletNode,      !- Name
      ZoneSplitter Inlet Node,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Component:Duct,
      AirLoopSupply,           !- Name
      0.1,                     !- Duct Length {m}
      1.0,                     !- Hydraulic Diameter {m}
      0.7854,                  !- Cross Section Area {m2}
      ,                        !- Surface Roughness {m}
      ,                        !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}
      0.1,                     !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction
      0.0001,                  !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}
      0.006500,                !- Outside Convection Coefficient {W/m2-K}
      0.032500;                !- Inside Convection Coefficient {W/m2-K}

  	AirflowNetwork:Distribution:Linkage,
      Mail Duct Linkage,             !- Name
      EquipmentOutletNode,      !- Node 1 Name
      SplitterInletNode,     !- Node 2 Name
      AirLoopSupply,              !- Component Name
      Attic Zone;              !- Thermal Zone Name

\end{lstlisting}

### Leakage losses without mass flow changes ###

Assumptions:

1. The leakage loss calculation is based on energy transfer only. In other words, no mass flows are involved in both AirLoopHVAC and zones. For example, when a supply leak is applied, the supply fan flow rate remains the same, the changes may be outlet equivalent temperatures and humidity ratios

2. Any energy losses for supply leak occur in the source duct and target zone or outdoor air node only. No more extension to other ducts and zones.

3. Any energy losses for return leak occur in the source zone or outdoor air node and and target duct only. No more extension to other ducts and zones.

4. Any air movement between zones caused by duct leakage can be accomplished by make up air.

#### Existing objects ####

AirflowNetwork:Distribution:Node and AirflowNetwork:Distribution:Linkage are the same as above

No change of AirflowNetwork:Distribution:Component:LeakageRatio.

Note for AirflowNetwork:Distribution:Component:LeakageRatio

Field Effective Leakage Ratio is used as leakage, a fraction of AirLoopHVAC flow. The inputs of the rest of fields are not used.

#### Makeup air ####

An important factor for duct leakage is to introduce make up flow due to supply and return leaks. Since we don't use pressure to calculate make up airflow impact, we will allow users to specify makeup air flows and direction using existing AFN object, so that make up airflows can flow from outdoor to a zone, and from a zone to another zone. The requirements are as follows:

1. The Node 1 name and Node 2 name in the AirflowNetwork:Distribution:Linkage object have to be either zone names for both fields or a zone name and an outdoor node name. The Node 1 name represents flow starting point, and the Node 2 name represents flow ending points. The flow direction for a linkage with a zone name and an outdoor node name should be from outdoor to a zone, equivalent to air infiltration. When both zone names are specified, the equivalent object should Zobe Mixing. 

2. Exfiltration is not used in energy calculation. In other words, an outdoor air node can not be specified as Node 2 name.

3. The current makeup air is limited in a single AirLoopHVAC. When multiple AirLoopHVACs are applied, makeup air movement between two zones can be very complicated. More deep discussion may be needed after implementation with a single AirLoopHVAC.  

An example of objects used to calculate duct leak loss in an IDF is:

\begin{lstlisting}

	Duct:Loss:Leakage,
      Main duct leak,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Mail Duct Leak;  \field AirflowNetwork:Distribution:Linkage Name

  	AirflowNetwork:Distribution:Node,
      SplitterInletNode,      !- Name
      ZoneSplitter Inlet Node,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Node,
      Attic Zone,      !- Name
      Attic zone Name,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Component:LeakageRatio,
      ZoneSupplyELR,          !- Name
      0.05,                    !- Effective Leakage Ratio {dimensionless}
      1.9,                     !- Maximum Flow Rate {m3/s}
      59.0,                    !- Reference Pressure Difference {Pa}
      0.65;                    !- Air Mass Flow Exponent {dimensionless}

  	AirflowNetwork:Distribution:Linkage,
      Main Duct Leak,             !- Name
      SplitterInletNode,      !- Node 1 Name
      Attic Zone,     !- Node 2 Name
      ZoneSupplyELR,              !- Component Name
      ;              !- Thermal Zone Name

! Make up air

	Duct:Loss:MakeupAir,
      Makeup air,    !- Name
      Main AirLoopHVAC,    !- AirLoopHAVC Name
   	  Duct Leak Makeup;  \field AirflowNetwork:Distribution:Linkage Name

  	AirflowNetwork:Distribution:Node,
      OutdoorAirNode,      !- Name
      Outdoor Air Node,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Node,
      Living Zone,      !- Name
      Living zone Name,  !- Component Name or Node Name
      Other,                   !- Component Object Type or Node Type
      3.0;                     !- Node Height {m}

  	AirflowNetwork:Distribution:Component:LeakageRatio,
      MakeupAirELR,          !- Name
      0.05,                    !- Effective Leakage Ratio {dimensionless}
      1.9,                     !- Maximum Flow Rate {m3/s}
      59.0,                    !- Reference Pressure Difference {Pa}
      0.65;                    !- Air Mass Flow Exponent {dimensionless}

  	AirflowNetwork:Distribution:Linkage,
      Duct Leak Makeup,             !- Name
      OutdoorAirNode,      !- Node 1 Name
      Living Zone,     !- Node 2 Name
      MakeupAirELR,              !- Component Name
      ;              !- Thermal Zone Name

\end{lstlisting}


## Outputs Description ##

insert text

## Engineering Reference ##

This section provides algorithms used in code implementation. The detailed eneineering documents will be submitted later.

### Algorithms ###

The algorithms cover temperature and humidity ratio at the outlet for supply and return leaks.

#### Conduction ####

Section of 13.1.4 Node Temperature Calculations in Engineering Reference presents temperature at duct outlet node due to conduction losses:

The outlet air temperature at the end of the duct (x = L) is:

T<sub>o</sub> = T<sub>∞</sub> + (T<sub>i</sub> − T<sub>∞</sub>)*exp[-UA/(mC<sub>p</sub>)]

where:

Ti = Inlet air temperature [°C]

To = Outlet air temperature [°C]

T∞ = Temperature of air surrounding the duct element [°C]

U = Overall heat transfer coefficient [W/m2-K]

A = Surface area (Perimeter * Length) [m2]

m˙ = Airflow rate [kg/s]

The heat transfer by convection to ambient, Q, is:

Q<sub>sen</sub> = m* C<sub>p</sub>(T<sub>∞</sub> − T<sub>i</sub>){1-exp[-UA/(mC<sub>p</sub>)]}

13.1.5 Node Humidity Ratio Calculations presents temperature and humidity ratio at duct outlet node due to diffusion losses:

The outlet air humidity ratio at the end of the duct (x = L) is:

W<sub>o</sub> = W<sub>∞</sub> + (W<sub>i</sub> − W<sub>∞</sub>)*exp[-U<sub>m</sub>A/m]

where:

Wi = Inlet air temperature [°C]

Wo = Outlet air temperature [°C]

W∞ = Temperature of air surrounding the duct element [°C]

U<sub>m</sub> = Overall moisture transfer coefficient [W/m2-K]

A = Surface area (Perimeter * Length) [m2]

m˙ = Airflow rate [kg/s]

The mositure transfer by convection to ambient, Q, is:

Q<sub>lat</sub> = m* (W<sub>∞</sub> − W<sub>i</sub>){1-exp[-U<sub>m</sub>A/m]}


#### Supply leaks ####

The schmetic of supply leak is shown below.

![Supply Leaks](SupplyLeaks.PNG)

The proposed equivelant temperature and humidity ratio are not necessary.

#### Return leaks ####

The schmetic of return leak is shown below.

![Return Leaks](ReturnLeaks.PNG)


The algorithm and code to calculate outlet condition of OAMixer is apply to return leaks.

OAMixer calculation:

    Real64 RecircMassFlowRate = state.dataMixedAir->OAMixer(OAMixerNum).RetMassFlowRate - state.dataMixedAir->OAMixer(OAMixerNum).RelMassFlowRate;

	state.dataMixedAir->OAMixer(OAMixerNum).MixMassFlowRate = state.dataMixedAir->OAMixer(OAMixerNum).OAMassFlowRate + RecircMassFlowRate;

    state.dataMixedAir->OAMixer(OAMixerNum).MixEnthalpy =
        (RecircMassFlowRate * RecircEnthalpy +
         state.dataMixedAir->OAMixer(OAMixerNum).OAMassFlowRate * state.dataMixedAir->OAMixer(OAMixerNum).OAEnthalpy) /
        state.dataMixedAir->OAMixer(OAMixerNum).MixMassFlowRate;
    state.dataMixedAir->OAMixer(OAMixerNum).MixHumRat = (RecircMassFlowRate * RecircHumRat + state.dataMixedAir->OAMixer(OAMixerNum).OAMassFlowRate *                                                                                                 state.dataMixedAir->OAMixer(OAMixerNum).OAHumRat) /                                                         state.dataMixedAir->OAMixer(OAMixerNum).MixMassFlowRate;

    state.dataMixedAir->OAMixer(OAMixerNum).MixTemp =
        Psychrometrics::PsyTdbFnHW(state.dataMixedAir->OAMixer(OAMixerNum).MixEnthalpy, state.dataMixedAir->OAMixer(OAMixerNum).MixHumRat);

Return leak calculation:

1. Calculate recirculate flow

m<sub>Recirculate</sub> = m<sub>Airloop</sub> - m<sub>Returnleak</sub>

2. Calculate mixed flow

m<sub>Mixed</sub> = m<sub>Recirculate</sub> + m<sub>returnleak</sub>

3. Calculate mixed air properties

Mass flow rate weighted air properties

T<sub>Mixed</sub> = (T<sub>Recirculte</sub> * m<sub>Recirculte</sub> +  T<sub>Returnleak</sub> * m<sub>Returnleak</sub> ) /m<sub>Mixed</sub>

W<sub>Mixed</sub> = (W<sub>Recirculte</sub> * m<sub>Recirculte</sub> +  W<sub>Returnleak</sub> * m<sub>Returnleak</sub> ) /m<sub>Mixed</sub>

#### Makeup air ####

The additional energy losses from makeup air is treated as infiltration with airflow from outdoor to a zone, and mixing with airflow from a zone to another.

##### Infiltration #####

Q<sub>sen</sub> = m<sub>inf</sub>* C<sub>p</sub>(T<sub>∞</sub> − T<sub>i</sub>)

Q<sub>lat</sub> = m<sub>inf</sub>h<sub>g</sub>(W<sub>∞</sub> − W<sub>i</sub>)

##### Mixing #####

Q<sub>sen</sub> = m<sub>mix</sub>* C<sub>p</sub>(T<sub>j</sub> − T<sub>i</sub>)

Q<sub>lat</sub> = m<sub>mix</sub>h<sub>g</sub>(W<sub>j</sub> − W<sub>i</sub>)

#### Add loss to zone load and system load ####

This section provides treament pathway of losses to either a system or a zone. 

1. Duct conduction and leakage loss

When the system outlet node is replaced by the zone supply inlet zone, duct losses can be included for system capacity request without any changes of system load prediction. This happens every iteration.

2. Makeup losses

I have two choices for the makeup losses. The first choice is to add losses as a part of system load, so that all losses will be added every system iteration. The second choice is to treat makeup losses as zone load used in the next time step. 

Here is a reason: 

When makeup losses are added as zone gain, they are excluded in the predictor calculation, so that requested system load will not have makeup losses at beginning of every time step. Thereofre, the makeup losses needs to be caught in the next zone time step. 

I will try the first choice first. If not working well, the second choice will be implemented.


## Example File and Transition Changes ##

insert text

## Design Document ##

This section provides code implementation approach.


##### Conduction and leakage #####

All losses from conduction and leakage will be added to a system as Duct loss. The implementation code is similar to the DuctLoss code in UntarySystem, Furnace and Multispeed AirToAir Heat pump mdules. The addon is summed by all conduction and leakage losses served to a system by the same Airloop.  

![DuctlossAddon](DuctlossAddon.png)

It should be pointed out that the added loss is calculated at every iteration of the entire Airloop. Since the loss add-on is system-based, each individual system modification will be performed.

<span style="color:red">

###### Possible approach ######

If the system outlet node is changed to a zone inlet node in the call of CalcZoneSensibleLatentOutput, the added losses may be removed. I will test this approach to see if it works or not.

</span>


##### Makeup air #####

The energy losses from makeup air will be added in a zone used for the next time step.

### New module ###

A new module of DuctLoss will be created for inputs process and calculations. Here is a list of possible functions and associated functionality.

1. SimDuctLoss
2. GetDuctLossInput
3. InitDuctLoss
4. CalcDuctLoss
5. ReportDuctLoss

#### SimDuctLoss ####

A possible function is:

void DuctLoss::SimDuctLoss(EnergyPlusData &state, bool const FirstHVACIteration)

The function may contain calls shown below:

        if (state.dataDuctLoss->GetInputOnceFlag) {
            // Get the AirLoopHVACDOAS input
            getDuctLossSInput(state);
            state.dataDuctLoss->GetInputOnceFlag = false;
        }

        this->initDuctLoss(state, FirstHVACIteration);

        this->CalcDuctLoss(state, FirstHVACIteration);

        this->ReportDuctLoss(state);

The function will be called in the SimZoneEquipment of the ZoneEquipmentManager. The location will be similar with the call of SplitterComponent::SimAirLoopSplitter or the call of ReturnAirPathManager::SimReturnAirPath.

#### GetDuctLossInput ####

The 3 new objects (Duct:Loss:XXX) will be processed in the new module. The other related AFN objects will be handled in the AFN module to read Node, Linkage, Duct and Leakage object. I will check to see possiblity if separated functions are needed or not in the AFN model, because AFN model and Non AFN model need to process the same objects of Node, Linkage, Duct and Leakage.

The function may contain functionality shown below:

Read all inputs for Duct:Loss:XXX
Check possible errors
Setup node connections
Call AFN model functions to get all required AFN objects

##### Local variables #####

All new objects and AFN obejcts will have local array variables defined in the header file.   

#### InitDuctLoss ####

This function has two functionalities. The first one is to assign component values and check error when some components data are not available in the process of GetDuctLossInput.

The second functionality is to assign component values at each AirLoop iteration.

#### CalcDuctLoss ####

All duct losses will be calculated in this function, including conduction, leaks and makeup air. The outputs will be temperature and humidity ratio at outlet nodes.

#### ReportDuctLoss ####

All sensible and latent losses from each Duct:Loss:XXX object will be reported.

### Other module revisions ###

The major revisions will be performed in 3 modules, UnitarySystem, MultispeedHeatPump, and Furnace. The change will replace system outlet node by supply inlet node to call CalcZoneSensibleLatentOutput, so that duct losses will be included automatically when system capacity is requested to reach zone setpoint.


## References ##

NFP-DuctHeatTransfer.md: https://github.com/NREL/EnergyPlus/blob/develop/design/FY2024/NFP-DuctHeatTransfer.md

DuctModelRoadMap.md: https://github.com/NREL/EnergyPlus/blob/develop/design/FY2024/DuctModelRoadMap.md
