Allow zone HVAC terminal units to draw return air from return air plenum and return duct
================

**Lixing Gu**

**Florida Solare Energy Center**

 - Original draft, 12/15/20

 - First revision- 011521
	Cover the first conference call on 1/13/21 and follow up E-mail communications

 - Second revision 01/25/21
	Add design document

 - Third revision 01/29/21
	Cover the second conference call on 1/27/21 and follow up E-mail communications


## Justification for New Feature ##

Currently EnergyPlus only allows a zone HVAC terminal unit to draw recriculation air from the zone air node - i.e. air at zone conditions.

Many applications of ceiling-mounted zone HVAC terminal units (VRF, hydronic fan coil, WSHP) actually draw air from the ceiling space or from a return air duct. In those applications the air entering the zone HVAC terminal mixing box is at a condition thermally different from the zone air node. In the case of drawing air from a return air plenum, air is at the mixed plenum temperature, having incorporated heat gains or losses occurring in the plenum. In the case of drawing air from a return duct, the air temperature may have been altered by lighting fixture heat gain in the case of returns originating at vented lighting fixtures. The altered condition of recirculating air affects the loads on cooling or heating coils in the zone HVAC terminal. It allows more accurate modeling of equipment loads and energy use.

This feature request is based on study of commercial building HVAC design drawings in which ceiling mounted HVAC terminals simply have an inlet port that draws air from the ceiling space rather than a duct running from the return grille to the zone HVAC unit mixing box. The distance between return grille and inlet port is sometimes quite large, meaning that return air from the space has sufficient time to mix with air in the plenum to absorb heat gains or losses.

Note:

The above description is mainly from GitHub Issue #8190 at https://github.com/NREL/EnergyPlus/issues/8190

## E-mail and  Conference Call Conclusions ##

Nov. 2020

Tiejun and I discuss how to implement this new feature. Revision was made after discussion.

1/13/21

A conference call was held on 1/13/21 to discuss the NFP.

Attendees: Edwin Lee, Brent Griffith, Mike Witte, Rich Raustad, Jim Spielbauer, Tienzhen Hong, Nagappan Chidambaram, Tiejun Wu, and Lixing Gu

The consensus agreement is provided below:

1. Draw air from return duct

When zone HVAC equipment draws air from the return duct, the euivelant operation is to keep the exhaust node as the equipment inlet node. Instead of using zone air properties directly, the light fraction gain will be added into both exhaust and return air nodes weighted by mass flow rate.

The last field of thr Lights object will be revised slightly:

1.14.4.1.17 Field: Return Air Heat Gain Node Name
Name of the return air node for this heat gain. If left blank, it defaults to the first return air node for the zone containing this Lights object. Leave blank when using a ZoneList name.

The node list will be used for this field. In other words, the node list is allowd for this field, so that the exhaust node of the Zone HAVC equipment can be added in the node list. The heat gain for each node will be weighted by the mass flow rates. The field name may be revised as:

1.14.4.1.17 Field: Return Air Heat Gain Node or NodeList Name

In addition, Mike provided code location how the return heat gain from Lights is calculated:

The return air heat gain is handled in ZoneEquipmentManager::CalcZoneLeavingConditions it calls some functions to collect the heat gains on the return.

It calls some functions like SumAllReturnAirConvectionGains to collect the heat gains, but I don't see anything to apportion the lights heat gains across multiple nodes as a function of flow. That may become a messy problem.

Mike

2. Draw air from return plenum


Allow the Zone HVAC equipment inlet node as one of Induced Air Outlet Node or NodeList Name defined in the AirLoopHVAC:ReturnPlenum object. The induced air outlet node as input will provide a trigger to draw air from return plenum.

In this configuration, a Return Air Node in the same zone needs to be one of the ReturnPlenum inlet nodes.

**E-mail comunication between Brent and Gu**

Brent:

Thanks for quick response. You are right, I should use Induced Air Outlet Node or NodeList Name in the AirLoopHVAC:ReturnPlenum


Therefore, the Air Inlet Node in the Zone Equipment such PTAC will allow a node listed as Induced Air Outlet Node or NodeList Name.

Thanks.

Gu
From: Brent Griffith [mailto:Brent.Griffith@EnergyArchmage.com]
Sent: Thursday, January 14, 2021 11:52 AM
To: 'Lixing Gu' <gu@fsec.ucf.edu>
Subject: RE: Feature Chat


“Since the Air Inlet Node Name must be referred in the Zone Air Exhaust Node or NodeList Name field of the ZoneHVAC:EquipmentConnections object, the NodeList should include return plenum node.
“

No, the Air Inlet Node Name should allow nodes to be plenum induced air nodes in addition to zone exhaust.  The return plenum will have its own node, but the plenum induced air node is separate.  Please study the PIU terminals.

**E-mail comments from Mike**

From: Michael J. Witte [mailto:mjwitte@gard.com]
Sent: Thursday, January 14, 2021 1:26 PM
To: Lixing Gu <gu@fsec.ucf.edu>; 'Lee, Edwin' <Edwin.Lee@nrel.gov>; thong@lbl.gov; 'Brent Griffith' <brent.griffith@energyarchmage.com>; 'Jim Spielbauer' <JSPIELBAUER@TRANE.COM>; 'Chidambaram, Nagappan' <Nagappan.Chidambaram@trane.com>; 'Rich Raustad' <rraustad@fsec.ucf.edu>
Cc: 'Wu, Tiejun UTC CCS' <tiejun.wu@carrier.com>
Subject: Re: Feature Chat


On 1/14/2021 9:38 AM, Lixing Gu wrote:
All:

Thanks for everyone to attend the conference call and provide valuable comments.

Here are summary based on the call:

1.	Draw air from return node
Revise the name of the last field in the Lights object, so that exhaust node as Zone equipment can be included. The Light heat gain will be calculated weighted by node mass flow rates.

Current: Return Air Heat Gain Node Name

Proposed: Return Air Heat Gain Node or NodeList Name

I thought the plan was to use and Exhaust node for this and allow the lights heat gain to be applied to exhaust nodes as well as return nodes. If you use a return node, then the function that allocates return flows will need to avoid stepping on the mass flow rate that has been applied by the zone equipment on that particular return node. Using exhaust nodes avoids this problem.

On the flip side, the return air heat gain function will have to look at both return nodes and exhaust nodes if you use an exhaust node. I think that's easier to tackle than figuring out which return nodes have active flow rates and which do not.


2.	Draw air from return plenum.

Allow the return plenum node as input of the Air Inlet Node Name field in Zone Equipment.

Since the Air Inlet Node Name must be referred in the Zone Air Exhaust Node or NodeList Name field of the ZoneHVAC:EquipmentConnections object, the NodeList should include return plenum node.

No, the equipment inlet node should be in the ZonePlenum Induced Air Node List, not in the zone exhaust node list - otherwise it will draw the same flow rate from two places.

In this configuration, a Return Air Node in the same zone needs to be one of the ReturnPlenum inlet nodes.

Mike

### Second Conference call on 1/27/21

A conference call was held on 1/27/21 to discuss 4 NFPs. This NFP is one of 4 NFPs discussed.

Attendees: 23 people.

Rich raised an issue to allow Return Air Node to be the Inlet Node of zone equipment for the scenario to draw air from return duct.

Mike mentioned to be careful to handle zone equipment airflow merged at the return air node for the scenario to draw air from return plenum.

### Follwou up E_mail communications after the second conference call ###

1. After E-mail communications, Mike preferred to use the exhaust node to draw air from return duct. Rich agreed to use Zone exhaust node, instead of Return Air Node.

Mike provided detailed clarifications for the scenario to draw air from return plenum:

Mike:

Thanks for detailed clarification.

Here is your concern as:

The remaining flow (PTAC flow) should get placed at the plenum return inlet but I'm not sure if the current return flow allocation will be happy with this.

I will check your concern during coding.

Gu

From: Michael J. Witte [mailto:mjwitte@gard.com]
Sent: Thursday, January 28, 2021 10:05 AM
To: Lixing Gu <gu@fsec.ucf.edu>; 'Richard Raustad' <rraustad@fsec.ucf.edu>
Cc: 'Brent Griffith' <brent.griffith@energyarchmage.com>; 'Lee, Edwin' <Edwin.Lee@nrel.gov>
Subject: Re: Use return air node as inlet node of zone equipment to draw zone air

Gu:
Here's my proposal.
Let's say you have a PTAC drawing from a plenum in a zone that also has an airloop and both return through the plenum.

PTAC
PTAC Inlet Node !- Air Inlet Node Name
PTAC Outlet Node !- Air Outlet Node Name

AirLoopHVAC:ReturnPlenum
PTAC Inlet Node, !- Induced Air Outlet Node
Plenum Return From Zone !- Inlet 1 Node Name

ZoneHVAC:AirDistributionUnit
ADU Supply Outlet, !- Air Distribution Unit Outlet Node Name

ZoneHVAC:EquipmentConnections
Zone Supply Nodes, !- Zone Air Inlet Node or NodeList Name
<blank>, !- Zone Air Exhaust Node or NodeList Name
Zone Air Node, !- Zone Air Node Name
Plenum Return From Zone , !- Zone Return Air Node or NodeList Name

NodeList,
Zone Supply Nodes, !- Name
ADU Supply Outlet
PTAC Outlet Node

Supply will enter the zone from the ADU and the PTAC.
Flow to the return node will equal the sum of the ADU and PTAC supply flows with the current zone mass flow calculations.
The PTAC will draw from the plenum induced air node.
The flow back to the airloop will equal the ADU supply flow.

I think this should work with no code changes other than allowing the PTAC to use an induced air node as an inlet.

If for some reason, there is a separate return for the air loop and the plenum just recirculates PTAC flow, then there would be to return nodes. The current logic will match the ADU supply flow with the air loop return flow. The remaining flow (PTAC flow) should get placed at the plenum return inlet but I'm not sure if the current return flow allocation will be happy with this.

Mike

#### Conlusions for the third revision ####

The proposed approach and design document remain the same. It should be careful to treat zone equipment mass flow rate to be merged at the Return Air Node during coding at different scenarios:

1. Zone equipment operation only
2. Zone equipment and AirLoop operate simultaneously
3. AirLoop operates alone (no changes)


## Overview ##

The new feature requests Zone HVAC objects to draw inlet air from return plenum and return duct. It should be pointed out that no return duct losses are calculated in Existing EnergyPlus, except for the AirflowNetwork model (beyond the topic for this new feature). Instead, light heat from return fraction is added to the return node after mixing. Therefore, light heat will be added into the drawn air for the return duct scenario. There is no modification for the scenario with return plenum.

This new feature is applied to the system configuration only when a zone equipment and an AirLoop serve the same zone. The proposed approach is described below.

1. Draw air from return duct

When zone HVAC equipment draws air from the return duct, the euivelant operation is to keep the exhaust node as the equipment inet node. Instead of using zone air properties directly, the light fraction gain will be added into both exhaust and return air nodes weighted by mass flow rates.

The modification will be made to replace the field of "Return Air Heat Gain Node Name" by "Return Air Heat Gain Node or NodeList Name" in the Lights object.

2. Draw air from retunr plenum

Allow the Zone HVAC equipment inlet node as one of Induced Air Outlet Node or NodeList Name defined in the AirLoopHVAC:ReturnPlenum object.

In this configuration, a Return Air Node in the same zone needs to be one of the ReturnPlenum inlet nodes.

Since this feature allows the induced air nodes as inlet air node for zone HVAC equipment, I plan to make PTAC, PTHP, and UnitarySystem work first. If time and budget allow, more equipment types will be implemented.

## Approach ##

This sectio proposes procedures to allow zone equipment to draw air from return duct and return plenum.

### Retur duct ###

Following steps are used to accomplish to allow zone equipment to draw air from retunr duct. The impact is heat gain from light return fraction. The input of exhaust node in the NodeList will be a trigger to allow the equipment to draw air from the return duct.

1. Keep equipment air inlet node as an exhaust node in a zone. No change occurs
2. Add zone exhaust node in a NodeList in the revised field of "Return Air Heat Gain Node or NodeList Name" in the Lights object.
3. Calculate air properties weighted by mass flow rates of both exhaust and return air nodes.

Nomanclature:

m = Mass flow rate

T = Air temperature

W = Air humidity ratio

Zone air properties: T<sub>zone</sub> and W<sub>zone</sub>

m<sub>total</sub> = m<sub>return</sub> + m<sub>exhaust</sub>

T<sub>return</sub> =  Q<sub>light</sub> /{m<sub>total</sub> * Cp} + T<sub>zone</sub>

W<sub>return</sub> = W<sub>zone</sub>

Therefore

Return node conditions: m<sub>return</sub>, T<sub>return</sub>, W<sub>return</sub>

Zone HVAC entry conditions: m<sub>exhaust</sub>, T<sub>return</sub>, W<sub>return</sub>

Note: The mass flow rate at the exhaust node remains as is. The only changes are air properties at exhaust node and return node.

### Return plenum ###

Folwoing steps are used to accomplish to allow zone equipment to draw air from retunr plenum.

1. Allow the Zone HVAC equipment inlet air node as one of "Induced Air Outlet Node or NodeList Name" defined in the AirLoopHVAC:ReturnPlenum object

The GetInput code will release existing restriction to accept the proposed change. The change is a trigger for the zone equipment to draw air from return plenum.

2. Modify the return node flow. The return flow rate is a sum of AirLoop return flow rate and zone equipmet flow rate. If any mass balance issue occurs, the mass conservation should be satisfied by modifying the code.

m<sub>return</sub> = m<sub>AirLoopReturn</sub> + m<sub>ZoneHVAC</sub>

Here is an example to describe what procedures will be used. Assume that there is a connection to a Zone HVAC equipment in Zone 1, there are 3 zones to return air into a return plenum as Zones 1, 2 and 3. This calculation procedure is provided below. The changes will be accordingly when there are more zones served by a return plenum.

Zone air properties:

T<sub>zone1</sub> and W<sub>zone1</sub>

T<sub>zone2</sub> and W<sub>zone3</sub>

T<sub>zone3</sub> and W<sub>zone3</sub>

Zone 1 return mass flow rate may be changed as below:

m<sub>total1</sub> = m<sub>AirLoopReturn1</sub> + m<sub>ZoneHVAC</sub>

There are no return mass flow rate changes in Zones 2 and 3

The existing return plenum calculation remain as is.

The final mass flow rate in the return plenum node and induced air node

m<sub>ReturnPlenum</sub> = m<sub>total1</sub> + m<sub>AirLoopReturn2</sub> + m<sub>AirLoopReturn3</sub>

m<sub>InducedNode</sub> = m<sub>ZoneHVAC</sub>

The air properties in both outlet nodes (induced and return plenum) are the same.



## Testing/Validation/Data Sources ##

Example file testing and unit test will be performed to ensure that the new feature works properly.

## Input Output Reference Documentation ##

This section provide the proposed changes for the last field of the Lights object.
\subsection{Lights}\label{lights-000}

The Lights statement allows you to specify information about a zone's electric lighting system, including design power level and operation schedule, and how the heat from lights is distributed thermally.

A zone may have multiple Lights statements. For example, one statement may describe the general lighting in the zone and another the task lighting. Or you can use multiple Lights statements for a zone that has two or more general lighting systems that differ in design level, schedule, etc.

\subsubsection{Inputs}\label{inputs-2-021}

\paragraph{Field: Name}\label{field-name-2-020}

The name of the Lights object.

\paragraph{Field: Zone or ZoneList Name}\label{field-zone-or-zonelist-name-1-000}

The field is the name of the thermal zone (ref: Zone) or \hyperref[zonelist]{ZoneList} (ref: ZoneLIst) and links this Lights statement to a thermal zone or set of thermal zones in the buidling. When the \hyperref[zonelist]{ZoneList} option is used then this lights definition is applied to each of the zones in the zone list effecting a global definition for the amount of light wattage in the zone. The Zonelist option can be used effectively with the watts/area and watts/person options of the Design Level Calculation Method.

The name of the actual lights object becomes \textless{}Zone Name\textgreater{} \textless{}Lights Object Name\textgreater{} and should be less than the standard length (100 characters) for a name field. If it is greater than this standard length, it may be difficult to specify in output reporting as it will be truncated. A warning will be shown if the generated name is greater than 100 characters. If it duplicates another such concatenated name, there will be a severe error and terminate the run.

\paragraph{Field: Schedule Name}\label{field-schedule-name-002}

The name of the schedule that modifies the lighting power design level (see Design Level Calculation Method field and related subsequent fields). The schedule values can be any positive number. The electrical input for lighting in a particular timestep is the product of the design level and the value of this schedule in that timestep. If the design level is the maximum lighting power input the schedule should contain values between 0.0 and 1.0.

\paragraph{Field: Design Level Calculation Method}\label{field-design-level-calculation-method}

This field is a key/choice field that tells which of the next three fields are filled and is descriptive of the method for calculating the nominal lighting level in the Zone. The key/choices are:

\begin{itemize}
\tightlist
\item
  LightingLevel
\end{itemize}

With this choice, the method used will be a straight insertion of the lighting level (Watts) for the Zone.~ (The Lighting Level field should be filled.)

\begin{itemize}
\tightlist
\item
  Watts/Area
\end{itemize}

With this choice, the method used will be a factor per floor area of the zone. (The Watts per Zone Floor Area field should be filled).

\begin{itemize}
\tightlist
\item
  Watts/Person
\end{itemize}

With this choice, the method used will be a factor of lighting level (watts) per person. (The Watts per person field should be filled).

\paragraph{Field: Lighting Level}\label{field-lighting-level}

This is typically the maximum electrical power input (in Watts) to lighting in a zone, including ballasts, if present. This value is multiplied by a schedule fraction (see previous field) to get the lighting power in a particular timestep. In EnergyPlus, this is slightly more flexible in that the lighting design level could be a ``diversity factor'' applied to a schedule of real numbers.

\paragraph{Field: Watts per Zone Floor Area}\label{field-watts-per-zone-floor-area}

This factor (watts/m\(^{2}\)) is used, along with the Zone Floor Area to determine the maximum lighting level as described in the Lighting Level field. The choice from the method field should be ``watts/area''.

\paragraph{Field: Watts per Person}\label{field-watts-per-person}

This factor (watts/person) is used, along with the number of occupants (people) to determine the maximum lighting level as described in the Lighting Level field. The choice from the method field should be ``watts/person''.

\paragraph{Heat Gains from Lights:}\label{heat-gains-from-lights}

The electrical input to lighting ultimately appears as heat that contributes to zone loads or to return air heat gains. In EnergyPlus this heat is divided into four different fractions. Three of these are given by the input fields Return Air Fraction, Fraction Radiant and Fraction Visible. A fourth, defined as the fraction of the heat from lights convected to the zone air, is calculated by the program as:

f\(_{convected}\) = 1.0 -- (Return Air Fraction + Fraction Radiant + Fraction Visible)

You will get an error message if Return Air Fraction + Fraction Radiant + Fraction Visible exceeds 1.0.

These fractions depend on the type of lamp and luminaire, whether the luminaire is vented to the return air, etc.

\paragraph{Field: Return Air Fraction}\label{field-return-air-fraction}

The fraction of the heat from lights that goes into the zone return air (i.e., into the zone outlet node). If the return air flow is zero or the zone has no return air system, the program will put this fraction into the zone air. Return Air Fraction should be non-zero only for luminaires that are return-air ducted~ (see Table~\ref{table:approximate-values-of-return-air-fraction} and Figure 51). (However, see the field ``Return Air Fraction Is Calculated from Plenum Temperature,'' below, for an approach to modeling the case where Return Air Fraction is caused by \emph{conduction} between a luminaire that is in contact with a return-air plenum.)

\paragraph{Field: Fraction Radiant}\label{field-fraction-radiant-1}

The fraction of heat from lights that goes into the zone as long-wave (thermal) radiation. The program calculates how much of this radiation is absorbed by the inside surfaces of the zone according the area times thermal absorptance product of these surfaces.

\paragraph{Field: Fraction Visible}\label{field-fraction-visible}

The fraction of heat from lights that goes into the zone as visible (short-wave) radiation. The program calculates how much of this radiation is absorbed by the inside surfaces of the zone according the area times solar absorptance product of these surfaces.

Approximate values of Return Air Fraction, Fraction Radiant and Fraction Visible are given in Table~\ref{table:approximate-values-of-return-air-fraction} for overhead fluorescent lighting for a variety of luminaire configurations. The data is based on ASHRAE 1282-RP ``Lighting Heat Gain Distribution in Buildings'' by Daniel E. Fisher and Chanvit Chantrasrisalai.

% table 14
\begin{longtable}[c]{p{0.8in}>{\raggedright}p{1.6in}>{\raggedright}p{0.9in}>{\raggedright}p{0.9in}>{\raggedright}p{0.9in}>{\raggedright}p{0.9in}}
\caption{Approximate values of Return Air Fraction, Fraction Radiant and Fraction Visible for overhead fluorescent lighting for different luminaire configurations. \label{table:approximate-values-of-return-air-fraction}} \tabularnewline
\toprule
Fixture No. & Luminaire Feature & Return Air Fraction & Fraction Radiant & Fraction Visible & fconvected \tabularnewline
\midrule
\endfirsthead

\caption[]{Approximate values of Return Air Fraction, Fraction Radiant and Fraction Visible for overhead fluorescent lighting for different luminaire configurations.} \tabularnewline
\toprule
Fixture No. & Luminaire Feature & Return Air Fraction & Fraction Radiant & Fraction Visible & \(f_{\rm{convected}}\) \tabularnewline
\midrule
\endhead

1 & Recessed, Parabolic Louver, Non-Vented, T8 & 0.31 & 0.22 & 0.20 & 0.27 \tabularnewline
2 & Recessed, Acrylic Lens, Non-Vented, T8 & 0.56 & 0.12 & 0.20 & 0.12 \tabularnewline
3 & Recessed, Parabolic Louver, Vented, T8 & 0.28 & 0.19 & 0.20 & 0.33 \tabularnewline
4 & Recessed, Acrylic Lens, Vented, T8 & 0.54 & 0.10 & 0.18 & 0.18 \tabularnewline
5 & Recessed, Direct/Indirect, T8 & 0.34 & 0.17 & 0.16 & 0.33 \tabularnewline
6 & Recessed, Volumetric, T5 & 0.54 & 0.13 & 0.20 & 0.13 \tabularnewline
7 & Downlights, Compact Fluorescent, DTT & 0.86 & 0.04 & 0.10 & 0.00 \tabularnewline
8 & Downlights, Compact Fluorescent, TRT & 0.78 & 0.09 & 0.13 & 0.00 \tabularnewline
9a & Downlights, Incandescent, A21 & 0.29 & 0.10 & 0.6 & 0.01 \tabularnewline
9b & Downlights, Incandescent, BR40 & 0.21 & 0.08 & 0.71 & 0.00 \tabularnewline
10 & Surface Mounted, T5HO & 0.00 & 0.27 & 0.23 & 0.50 \tabularnewline
11 & Pendant, Direct/Indirect, T8 & 0.00 & 0.32 & 0.23 & 0.45 \tabularnewline
12 & Pendant, Indirect, T5HO & 0.00 & 0.32 & 0.25 & 0.43 \tabularnewline
 &  &  &  &  &  \tabularnewline \midrule
1 & Recessed, Parabolic Louver, Non-Vented, T8 - Ducted & 0.27 & 0.27 & 0.21 & 0.25 \tabularnewline
5 & Recessed, Direct/Indirect, T8 - Ducted & 0.27 & 0.22 & 0.17 & 0.34 \tabularnewline
 &  &  &  &  &  \tabularnewline \midrule
1 & Recessed, Parabolic Louver, Non-Vented, T8 - Half Typical Supply Airflow Rate & 0.45 & 0.30 & 0.22 & 0.03 \tabularnewline
3 & Recessed, Parabolic Louver, Vented, T8 - Half Typical Supply Airflow Rate & 0.43 & 0.25 & 0.21 & 0.11 \tabularnewline
5 & Recessed, Direct/Indirect, T8 - Half Typical Supply Airflow Rate & 0.43 & 0.27 & 0.18 & 0.12 \tabularnewline
 &  &  &  &  &  \tabularnewline \midrule
1 & Recessed, Parabolic Louver, Non-Vented, T8 - Half Typical Supply Airflow Rate & 0.10 & 0.16 & 0.20 & 0.54 \tabularnewline
3 & Recessed, Parabolic Louver, Vented, T8 - Half Typical Supply Airflow Rate & 0.11 & 0.15 & 0.19 & 0.55 \tabularnewline
5 & Recessed, Direct/Indirect, T8 - Half Typical Supply Airflow Rate & 0.04 & 0.13 & 0.16 & 0.67 \tabularnewline
\bottomrule
\end{longtable}

\paragraph{Field: Fraction Replaceable}\label{field-fraction-replaceable}

This field defines the daylighting control for the LIGHTS object.

If \textbf{\hyperref[daylightingcontrols-000]{Daylighting:Controls}} is specified for the zone, this field is used as an on/off flag for dimming controls. If set to 0.0, the lights are not dimmed by the daylighting controls. If set to 1.0, the lights are allowed to be dimmed by the daylighting controls.

\paragraph{Field: End-Use Subcategory}\label{field-end-use-subcategory-002}

Allows you to specify a user-defined end-use subcategory, e.g., ``Task Lights'', ``Hall Lights'', etc. A new meter for reporting is created for each unique subcategory~ (ref: \hyperref[outputmeter-and-outputmetermeterfileonly]{Output:Meter} objects). Subcategories are also reported in the ABUPS table. If this field is omitted or blank, the lights will be assigned to the ``General'' end-use subcategory. Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table and the LEED EAp2-4/5. Performance Rating Method Compliance table.

\paragraph{Field: Return Air Fraction Calculated from Plenum Temperature}\label{field-return-air-fraction-calculated-from-plenum-temperature}

Accepts values Yes or No (the default). Yes is for advanced used only. In this case the program will calculate the return air fraction by assuming that it is due to conduction of some of the light heat into the zone's return air plenum and that the amount of the conduction depends on the plenum air temperature. A Yes value should only be used for luminaires that are recessed and non-vented, as shown in Figure~\ref{fig:vertical-section-through-a-zone-and-its}.

The value you enter for the Return Air Fraction field will be ignored and you can enter, for fluorescent lighting, Fraction Radiant = 0.37 and Fraction Visible = 0.18, as indicated in Table~\ref{table:approximate-values-of-return-air-fraction}.

This feature requires that the coefficients described below be determined from measurements or detailed calculations since they are very sensitive to the luminaire type, lamp type, thermal resistance between fixture and plenum, etc.

If ``Return Air Fraction Is Calculated from Plenum Temperature'' = Yes, the return air fraction is calculated \emph{each timestep} from the following empirical correlation:

\begin{equation}
  (\rm{Return Air Fraction})_{\rm{calculated}} = C_{1} - C_{2} \times T_{\rm{plenum}}
\end{equation}

where T\(_{\rm{plenum}}\) is the previous-time-step value of the return plenum air temperature (C),

and C\(_{1}\) and C\(_{2}\) are the values of the coefficients entered in the next two fields.

To compensate for the change in the return air fraction relative to its input value, the program modifies Fraction Radiant and \(f_{\rm{convected}}\) by a scale factor such that

\begin{equation}
  (\rm{Return Air Fraction})_{\rm{calculated}} + (\rm{Fraction Radiant})_{\rm{modified}} + (f_{\rm{convected}})_{\rm{modified}} + (\rm{Fraction Visible})_{\rm{input}} = 1.0
\end{equation}

It is assumed that Fraction Visible is a constant equal to its input value.

\paragraph{Field: Return Air Fraction Function of Plenum Temperature Coefficient 1}\label{field-return-air-fraction-function-of-plenum-temperature-coefficient-1}

The coefficient C\(_{1}\) in the equation for (Return Air Fraction)\(_{calculated}\).

\paragraph{Field: Return Air Fraction Function of Plenum Temperature Coefficient 2}\label{field-return-air-fraction-function-of-plenum-temperature-coefficient-2}

The coefficient C\(_{2}\) in the equation for (Return Air Fraction)\(_{calculated}\). Its units are 1/\(^{O}\)C.

<span style="color:red">

\paragraph{Field: Return Air Heat Gain Node or NodeList Name}\label{field-return-air-heat-gain-node-name}

Name of the return air node for this heat gain. If left blank, it defaults to the first return air node for the zone containing this Lights object. Leave blank when using a \hyperref[zonelist]{ZoneList} name. When a NodeList Name is entered, the NodeList can include a zone return node name and zone exhaust node name. The zone exhaust node name will be used for an inlet air node name of a zone HVAC equipment, so that the equipment can draw air with properties by mixing mass flow rates of both nodes and added lights heat gain.

</span>

An example using ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER for the case to draw air from return duct

  	ZoneHVAC:PackagedTerminalAirConditioner,
    Zone1PTAC,               !- Name
    FanAndCoilAvailSched,    !- Availability Schedule Name
<span style="color:red">

    Zone1PTACAirInletNode,   !- Air Inlet Node Name
</span>

    Zone1PTACAirOutletNode,  !- Air Outlet Node Name
    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type
    Zone1PTACOAMixer,        !- Outdoor Air Mixer Name
    0.4,                     !- Cooling Supply Air Flow Rate {m3/s}
    0.4,                     !- Heating Supply Air Flow Rate {m3/s}
    ,                        !- No Load Supply Air Flow Rate {m3/s}
    0.028,                   !- Cooling Outdoor Air Flow Rate {m3/s}
    0.028,                   !- Heating Outdoor Air Flow Rate {m3/s}
    0.0186,                  !- No Load Outdoor Air Flow Rate {m3/s}
    Fan:SystemModel,         !- Supply Air Fan Object Type
    Zone1PTACFan,            !- Supply Air Fan Name
    Coil:Heating:Fuel,       !- Heating Coil Object Type
    Zone1PTACHeater,         !- Heating Coil Name
    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type
    Zone1PTACDXCoolCoil,     !- Cooling Coil Name
    BlowThrough,             !- Fan Placement
    ConstantFanSch;          !- Supply Air Fan Operating Mode Schedule Name

  	NodeList,
    Zone1Exhausts,           !- Name
    Zone1PTACAirInletNode;   !- Node 1 Name

  	ZoneHVAC:EquipmentConnections,
    WEST ZONE,               !- Zone Name
    Zone1Equipment,          !- Zone Conditioning Equipment List Name
    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name
    Zone1Exhausts,           !- Zone Air Exhaust Node or NodeList Name
    Zone 1 Node,             !- Zone Air Node Name
    Zone 1 Outlet Node;      !- Zone Return Air Node or NodeList Name

  	Lights,
    WEST ZONE Lights 1,      !- Name
    WEST ZONE,               !- Zone or ZoneList Name
    OFFICE LIGHTING,         !- Schedule Name
    LightingLevel,           !- Design Level Calculation Method
    1878.6252,               !- Lighting Level {W}
    ,                        !- Watts per Zone Floor Area {W/m2}
    ,                        !- Watts per Person {W/person}
    0,                       !- Return Air Fraction
    0.2000000,               !- Fraction Radiant
    0.2000000,               !- Fraction Visible
    0,                       !- Fraction Replaceable
    GeneralLights,           !- End-Use Subcategory
    ,                        !- Return Air Fraction Calculated from Plenum Temperature
    ,                        !- Return Air Fraction Function of Plenum Temperature Coefficient 1
    ,                        !- Return Air Fraction Function of Plenum Temperature Coefficient 2
    Zone1ReturnNodeList;          !- Return Air Heat Gain Node or NodeList Name

  	NodeList,
    Zone1ReturnNodeList,           !- Name
<span style="color:red">

    Zone1PTACAirInletNode,   !- Node 1 Name
</span>

    Zone 1 Outlet Node;   !- Node 2 Name

An example using ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER for the case to draw air from return plenum

  	ZoneHVAC:PackagedTerminalAirConditioner,
    Zone1PTAC,               !- Name
    FanAndCoilAvailSched,    !- Availability Schedule Name
<span style="color:red">

    Zone1PTACAirInletNode,   !- Air Inlet Node Name
</span>

    Zone1PTACAirOutletNode,  !- Air Outlet Node Name
    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type
    Zone1PTACOAMixer,        !- Outdoor Air Mixer Name
    0.4,                     !- Cooling Supply Air Flow Rate {m3/s}
    0.4,                     !- Heating Supply Air Flow Rate {m3/s}
    ,                        !- No Load Supply Air Flow Rate {m3/s}
    0.028,                   !- Cooling Outdoor Air Flow Rate {m3/s}
    0.028,                   !- Heating Outdoor Air Flow Rate {m3/s}
    0.0186,                  !- No Load Outdoor Air Flow Rate {m3/s}
    Fan:SystemModel,         !- Supply Air Fan Object Type
    Zone1PTACFan,            !- Supply Air Fan Name
    Coil:Heating:Fuel,       !- Heating Coil Object Type
    Zone1PTACHeater,         !- Heating Coil Name
    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type
    Zone1PTACDXCoolCoil,     !- Cooling Coil Name
    BlowThrough,             !- Fan Placement
    ConstantFanSch;          !- Supply Air Fan Operating Mode Schedule Name

 	AirLoopHVAC:ReturnPlenum,
    Return-Plenum-1,         !- Name
    PLENUM-1,                !- Zone Name
    PLENUM-1 Node,           !- Zone Node Name
    PLENUM-1 Out Node,       !- Outlet Node Name
<span style="color:red">

    Zone1PTACAirInletNode,  !- Induced Air Outlet Node or NodeList Name
</span>

    SPACE1-1 Out Node,       !- Inlet 1 Node Name
    SPACE2-1 Out Node,       !- Inlet 2 Node Name
    SPACE3-1 Out Node,       !- Inlet 3 Node Name
    SPACE4-1 Out Node,       !- Inlet 4 Node Name
    SPACE5-1 Out Node;       !- Inlet 5 Node Name

  	ZoneHVAC:EquipmentConnections,
    WEST ZONE,               !- Zone Name
    Zone1Equipment,          !- Zone Conditioning Equipment List Name
    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name
    Zone1Exhausts,           !- Zone Air Exhaust Node or NodeList Name
    Zone 1 Node,             !- Zone Air Node Name
    Zone 1 Outlet Node;      !- Zone Return Air Node or NodeList Name

## Input Description ##

The Lights object is modified by adding an optional field as Return Air Heat Gain Node or NodeList Name. The revised idd is provided below. The revision is highlighted in red.


	Lights,
   	\memo Sets internal gains for lights in the zone.
   	\memo If you use a ZoneList in the Zone or ZoneList name field then this definition applies
   	\memo to all the zones in the ZoneList.
       \min-fields 11
  	A1 , \field Name
       \required-field
       \type alpha
       \reference LightsNames
  	A2 , \field Zone or ZoneList Name
       \required-field
       \type object-list
       \object-list ZoneAndZoneListNames
  	A3 , \field Schedule Name
       \required-field
       \type object-list
       \object-list ScheduleNames
       \note units in schedule should be fraction applied to design level of lights, generally (0.0 - 1.0)
 	 A4 , \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of lights
       \note for this set of attributes
       \note Choices: LightingLevel => Lighting Level -- simply enter watts of lights
       \note Watts/Area => Watts per Zone Floor Area -- enter the number to apply.  Value * Floor Area = Lights
       \note Watts/Person => Watts per Person -- enter the number to apply.  Value * Occupants = Lights
       \type choice
       \key LightingLevel
       \key Watts/Area
       \key Watts/Person
       \default LightingLevel
  	N1 , \field Lighting Level
       \units W
       \type real
       \minimum 0
       \ip-units W
  	N2 , \field Watts per Zone Floor Area
       \type real
       \minimum 0
       \units W/m2
       \ip-units W/ft2
  	N3 , \field Watts per Person
       \type real
       \minimum 0
       \units W/person
       \ip-units W/person
  	N4 , \field Return Air Fraction
       \note Used only for sizing calculation if return-air-fraction
       \note coefficients are specified.
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0
  	N5 , \field Fraction Radiant
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0
  	N6 , \field Fraction Visible
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0
  	N7 , \field Fraction Replaceable
       \note For Daylighting:Controls must be 0 or 1:  0 = no dimming control, 1 = full dimming control
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 1.0
  	A5 , \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General
  	A6 , \field Return Air Fraction Calculated from Plenum Temperature
       \type choice
       \key Yes
       \key No
       \default No
  	N8 , \field Return Air Fraction Function of Plenum Temperature Coefficient 1
       \note Used only if Return Air Fraction Is Calculated from Plenum Temperature = Yes
       \note Equation is Return Air Fraction = Coefficient#1 - Coefficient#2 X PlenumTemp(degC)
       \type real
       \minimum 0.0
       \default 0.0
  	N9 , \field Return Air Fraction Function of Plenum Temperature Coefficient 2
       \note Used only if Return Air Fraction Is Calculated from Plenum Temperature = Yes
       \note Equation is Return Air Fraction = Coefficient#1 - Coefficient#2 X PlenumTemp(degC)
       \type real
       \units 1/K
       \minimum 0.0
       \default 0.0
<span style="color:red">

  	A7 ; \field Return Air Heat Gain Node or NodeList Name
       \note Name of the return air node for this heat gain.
       \note If left blank, defaults to the first return air node for the zone.
       \note Leave this field blank when using a ZoneList name.
       \note If a zone exhaust node is listed in the NodeList, the air properties are calculated
       \note using both node mass flow rates and light return air heat gain
       \type node

</span>


## Outputs Description ##

insert text

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

### Example file ###

Expect to revise an existig example file with this new feature.

### Transition changes ###

No transition is expected, because an optional field is proposed to be added.


## References ##

insert text

## Design Document ##

The code modification will be performed in modules as follows:

InternalHeatGains

DataHeatBalance

ZoneEquipmentManager

PackagedTerminalHeatPump

UnitarySystem

ZonePlenum

### InternalHeatGains ###

Since IDD change is required by modifying the last field name in Lights object, the corresponding change in GetInternalHeatGainsInput will be performed to allow NodeList, in addition node.

#### GetInternalHeatGainsInput ####

The changes is to allow the last field to include NodeList. One of the node in the NodeList should be the zone exhaust node name.

### DataHeatBalance ###

The Struct LightsData will be modified

#### Add two new variables ####

1. bool NodeListActive

The varaible is set to false, when NodeList is not an input, Otherwise, the value is set to true.

2. int NumofReturnHeatNodes

If NodeListActive, then assign the number of nodes in the NodeLIst into this variables.

#### Replace int ZoneReturnNum by Array1D<int> ZoneReturnNodeNum()

Since a single node will be replaced by NodeList, 1-D array is needed to carry NodeList information.

### ZoneEquipmentManager ###

The return air heat gain is handled in ZoneEquipmentManager::CalcZoneLeavingConditions it calls some functions to collect the heat gains on the return.

A new section will be added to perform followig calculations

1. A sum of total light return heat gains from multiple lights objects in the same zone
2. Collect mass flow rates from return node and exhaust node
3. Calculate outlet air properties weighted by mass flow rates and total light heat gains.

### PackagedTerminalHeatPump ###

The GetPTUnit function will be modified to allow Induced Air Outlet Node or NodeList Name as Inlet Air Node for both PTAC and PTHP.

### UnitarySystem ###

The getUnitarySystemInput function will be modified to allow Induced Air Outlet Node or NodeList Name as Inlet Air Node for UnitarySystem as a zone equipment.

### ZonePlenum ###

Both functions of InitAirZoneReturnPlenum and CalcAirZoneReturnPlenum may be modified.

1. Add Equipment mass flow rate in the return mass flow rate
2. Calculate zone plenumn outlet conditions using existing procedure
3. Set air properties for all induced air nodes and return plenum node
4. Split mass flow rate of plenum outlets for retur plenum node and induced nodes.


