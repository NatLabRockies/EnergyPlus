Airflow Network Modeling Changes for Polygonal Windows Support
================

**Lixing Gu**

**Florida Solar Energy Center**

 - Nov. 21, 2016, First revision of Design Document
 - Nov. 17, 2016, Third Edition Design Document
 - Nov. 10, 2016, Second Edition
 - Original version on Oct. 28, 2016


## Justification for New Feature ##

The AirflowNetowrk model currently implemented in EnergyPlus is used to accurately predict simulation conditions based on a pressure-network. This capability allows users to carefully define the pressure components in their building, and use this to resolve flow rates. However, the model is able to handle a rectangular windows well and unable to handle polygonal windows correctly, because the existing opening airflow calculation is based on a rectangular window. Due to more and more uses of polygonal windows from users, it is needed to enhance the model capability to allow the model to handle polygonal windows properly by converting a polygonal opening into an equivalent rectangular shape, so that existing algorithm can be applied.

## E-mail and  Conference Call Conclusions ##

E-mail communication between Mike and Gu

>On 11/10/2016 2:43 PM
>Thanks for the explanation.  Sounds good.

>Mike

>On 11/10/2016 1:41 PM, Lixing Gu wrote:
> Mike:
>
> Thanks for your comments. The formulas for effective height and width are from the ProcessSurfaceVertices function of the SurfaceGeometry module. The main purpose is used to calculate convection coefficients. Since the effective height and width are available, the AirflowNetwork model just uses them. However, the effective height and width do not provide the same area. I would like to replace the values using the proposed method. Physically, both values do not have physical sense.
>
> Therefore, the reported values will not be used in the proposed approach. The purpose is that if a rectangular is cut into two triangular through a diagonal line. The effective area from the same two triangulars based on the proposed approach is the same as a rectangular.
>
> Thanks.
>
> Gu
>
> -----Original Message-----
> From: Michael J Witte [mailto:mjwitte@gard.com]
> Sent: Thursday, November 10, 2016 2:10 PM
> To: Lixing Gu <gu@fsec.ucf.edu>
> Subject: Re: [energyplusdevteam] NFP to allow polygonal windows using AirflowNetwork
>
> Gu:
>
> Sorry - I don't think I provided any comments on this yet.
>
> Where do the formulas for effective height and width come from? In the I/O description you talk about long and short axes rather than specific vertices.  I realize this is just the NFP stage, but I'm wondering how you will do that.  And could you use the same height and width values that are already calculated which are reported in the surface details report?
>
> Mike
>

Github communication between Jason, Edwin, and Gu

1.
jasondegraw  4 days ago   National Renewable Energy Laboratory member

Is there another way to input the aspect ratio? Wouldn't this limit models to a single non-rectangular window aspect ratio?


Myoldmopar  3 days ago   National Renewable Energy Laboratory member

@lgu1234 I like the effort here to possibly reuse a previous field. But I don't like the idea of applying a plan-aspect-ratio to an elevation-aspect-ratio. If you want to leave this as a choice field, I'd like this:
•Field 1: Input Type ◦Option 1: Hardwired Aspect Ratio
◦Option 2: Height-based Autocalculated Aspect Ratio
◦Option 3: Parent surface-based adopted aspect ratio


Where the user can enter a fixed aspect ratio, or one will be created with the same height as the polygonal window, or the parent surface's aspect ratio is used. I don't know how valid option 3 is, but it certainly has to be a better assumption that just using the building's plan-view aspect ratio across the board.


lgu1234  40 minutes ago   National Renewable Energy Laboratory member

@Myoldmopar I like your suggestion to have 3 options. I am going to revise the NFP.

2.


jasondegraw  4 days ago   National Renewable Energy Laboratory member

Should this be "Height"?


lgu1234  an hour ago   National Renewable Energy Laboratory member

@jasondegraw You are right. I am going to make a change.

###More communications

More communications are available in GitHub: https://github.com/NREL/EnergyPlus/pull/5924

## Overview ##

In order to allow the AirflowNetwork model to handle polygonal windows, an equivalent rectangular shape will be used with the same area and user input choices. The choices include Aspect Ratio from user input, Height Based, and Parent Surface Aspect Ratio. The same height is preferred as a default choice, since the airflow rate calculation is heavily dependent on height.

## Approach ##

A new optional field will be added as "Equivalent Rectangular Shape Choice" in the AirflowNetwork:MultiZone:Surface object. The allowed choice will be "Aspect Ratio from Input", "Height Based" or "Parent Surface Aspect Ratio". Since one of choices is enough to define the equivalent rectangular window, these choices are interlocked. The default choice is Height Based, because the same height will generate the same equivalent flows.

## Testing/Validation/Data Sources ##

The equivalent shape will be reported and checked to ensure that the shape conversion is performed correctly.

## Input Output Reference Documentation ##

The AirflowNetwork:MultiZone:Surface object will be modified by adding an optional field to allow user to enter a choice how the equivalent shape is created. This field is required when a polygonal shape is input.

Additions to the documentation are noted as **<span style="color:red;">bold red</span>** non-blocked insertions at the appropriate location throughout the Input Output Reference.

Removal sections are noted as **<span style="color:blue;">bold blue</span>**.

It should be pointed out that since this section is extracted from Input Output Reference directly with the format of Latex, it may not be shown correctly in the Markdown format. In order to keep the same format in the E+ documentation, the LaText format is kept.


\subsection{AirflowNetwork:MultiZone:Surface}\label{airflownetworkmultizonesurface}

The AirflowNetwork:MultiZone:Surface object specifies the properties of a surface ``linkage'' through which air flows. This linkage is always associated with a heat transfer surface (wall, roof, floor, or a ceiling) or subsurface (door, glass door, or window) with both faces exposed to air. The linkage specifies two connected nodes: two zone nodes defined in AirflowNetwork:MultiZone:Zone objects based on inside and outside face environment for an interior surface, or a zone node defined in an AirflowNetwork:MultiZone:Zone object based on inside face environment and an external node defined in an AirflowNetwork:MultiZone:ExternalNode object for an exterior surface. The associated leakage component for this surface can be a crack (or surface effective leakage area) in an exterior or interior heat transfer surface or subsurface, or an exterior or interior window, door or glass door (heat transfer subsurface) that can be opened to allow air flow. The allowed surface air leakage components are:

\begin{itemize}
\item
  AirflowNetwork:MultiZone:Surface:Crack
\item
  AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea
\item
  AirflowNetwork:MultiZone:Component:DetailedOpening
\item
  AirflowNetwork:MultiZone:Component:HorizontalOpening
\item
  AirflowNetwork:MultiZone:Component:SimpleOpening
\item
  AirflowNetwork:MultiZone:Component:ZoneExhaustFan
\end{itemize}

The two ``opening'' components are used to modulate openness based on required conditions.

The AirflowNetwork:MultiZone:Surface object allows a heat transfer surface or subsurface to have one crack (or one surface effective leakage area object), or a subsurface (i.e., window, door or glass door) to have one opening (detailed or simple).

An interior heat transfer surface (BuildingSurface:Detailed) whose surface name is used as the input for the Outside Boundary Condition Object field represents a floor without ground contact and is not allowed as an AirflowNetwork:MultiZone:Surface. A heat transfer surface defined in the BuildingSurface:Detailed:ExteriorNaturalVentedCavity is also not allowed.

**<span style="color:blue;">When a triangular subsurface is used, the model provides a warning and treats this subsurface as rectangular. The effective width and height calculated in the ProcessSurfaceVertices function of the SurfaceGeometry module are used to represent a rectangular subsurface.</span>**

**<span style="color:blue;">Effective width = 0.75 x Length between Vertex 1 and Vertex 2</span>**

**<span style="color:blue;">Effective height = 4 x Area / ( 3 x Length between Vertex 2 and Vertex 3 )</span>**

\subsubsection{Field: Surface Name}\label{field-surface-name-000}

This is the name of the corresponding surface (wall, roof, ceiling, floor, window, door or glass door).

Information on this surface is used by the program as follows:

(1)~~For a linkage associated with an exterior heat transfer surface: air flow through this linkage is between the outside environment and the thermal zone to which the surface belongs.

(2)~~For a linkage associated with an interior (i.e., interzone) heat transfer surface: air flow through this linkage is between the thermal zones separated by the surface (i.e., the thermal zone associated with the inside face environment and the thermal zone associated with the outside face environment).

(3)~~This heat transfer surface determines the height of the linkage, which is used in calculating buoyancy-related flow through the linkage.

\textbf{Note:} It is possible to define an interzone surface twice in EnergyPlus, once in each of the zones that the surface separates. Previously this was a requirement of EnergyPlus (prior to version 2.0), but now it is optional and the user also has the option of only defining the surface once (EnergyPlus defines the second surface automatically within the program). For each interzone surface, use only one (of possible two) interzone surface names in the AirflowNetwork:MultiZone:Surface object for ``Surface Name.'' \textbf{Do not} enter two AirflowNetwork:MultiZone:Surface objects corresponding to the two possible interzone names. This would cause the air flow through the surface to be counted twice.

\subsubsection{Field: Leakage Component Name}\label{field-leakage-component-name}

The name of the AirflowNetwork:MultiZone:Surface:Crack, AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea, AirflowNetwork:MultiZone:Component:SimpleOpening, AirflowNetwork:MultiZone:Component:HorizontalOpening, AirflowNetwork:MultiZone:Component:DetailedOpening or AirflowNetwork:MultiZone:Component:ZoneExhaustFan object associated with this air flow linkage.

If the name of an opening component (i.e.~AirflowNetwork:MultiZone:Component:DetailedOpening. AirflowNetwork:MultiZone:Component:HorizontalOpening,~ or AirflowNetwork:MultiZone:Component:SimpleOpening is given here, then the Surface Name in the previous field must be that of a window, door or glass door heat transfer subsurface. Otherwise an error message will be reported.

If the name of an AirflowNetwork:MultiZone:Surface:Crack object or AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea object is given here, the program will position the crack at the average height of the associated heat transfer surface or subsurface. The user can define multiple heat transfer surfaces (e.g., split a wall into several surfaces) to be more precise in establishing the crack location. Similarly, the user can define multiple heat transfer surfaces if a wall, for example, has multiple cracks or openings that need to be defined individually.

If the name of an AirflowNetwork:MultiZone:Component:ZoneExhaustFan is given here, then the Surface Name in the previous field must be that of an exterior heat transfer surface. The zone name defined in the Zone Name field for this heat transfer surface must be the same zone name defined in the ZoneHVAC:EquipmentConnections object (which references a ZoneHVAC:EquipmentList containing the name of the corresponding zone exhaust fan).Otherwise an error message will be reported. When this zone exhaust fan is operating for a simulation timestep, all surface-level controls described below are ignored for that timestep.

\subsubsection{Field: External Node Name}\label{field-external-node-name}

The name of the associated AirflowNetwork:MultiZone:ExternalNode object, which determines the wind pressure coefficients for the heat transfer surface. Used only if Surface Name is for an exterior surface.

If Wind Pressure Coefficient Type = SurfaceAverageCalculation in the AirflowNetwork:SimulationControl object, this field is not used and a blank may be entered. If the surface is an interior (i.e., interzone) surface, leave this field blank.

\subsubsection{Field: Window/Door Opening Factor, or Crack Factor}\label{field-windowdoor-opening-factor-or-crack-factor}

If this linkage is associated with an AirflowNetwork:MultiZone:Component:DetailedOpening or AirflowNetwork:MultiZone:Component:SimpleOpening object (which means it is an openable window or door), then this field is called ``Window/Door Opening Factor'' and represents the value of the Opening Factor that is in effect when the Vent Temperature Schedule (defined in the AirflowNetwork:MultiZone:Zone object) indicates that this window or door is open.

The AirflowNetwork model uses a combination of factors to determine the actual opening area for a window or door when it is venting. For example, consider a window that is 1.5m high and 2.0m wide (excluding frame). Assume that the AirflowNetwork:MultiZone:Component:DetailedOpening for this window has Type of Large Vertical Opening = 1 (non-pivoting window), Height Factor = 0.5 and Width Factor = 0.8. Then when the window is fully open, the opening area = height of opening (0.5x1.5) times width of opening (0.8x2.0) = 0.75x1.6 = 1.2 m\(^{2}\). If the Window/Door Opening Factor is 0.75, then the opening area = 0.75x1.2 = 0.9 m\(^{2}\).

If, in addition, the window is in a thermal zone for which opening modulation has been specified (ref: AirflowNetwork:MultiZone:Zone) and the multiplication factor due to modulation is 0.3 in a particular timestep, then the actual opening factor that timestep = 0.3x0.75 = 0.225 and the actual opening area that timestep = 0.3x0.9 = 0.27 m\(^{2}\).

If this linkage is associated with an AirflowNetwork:MultiZone:Surface:Crack object, the following crack air flow equation is used.

\begin{equation}
Q = \left( Crack\;Factor \right) * C_T * C_Q \left( \Delta P \right)^{n}
\end{equation}

Where

\emph{Q}~~~ = air mass flow (kg/s)

\emph{C\(_{Q}\)}~ = air mass flow coefficient (kg/s @ 1 Pa)

\emph{C\(_{T}\)}~ = reference condition temperature correction factor (dimensionless). See AirflowNetwork:MultiZone:Surface:Crack object.

\(\Delta P\) = pressure difference across crack (Pa)

\emph{n}~~~ = air flow exponent (dimensionless)

\emph{The following fields control venting. They are used only when Name of Associated Heat Transfer Surface is that of an openable exterior or interior window, door or glass door. They only apply to openings, and do not apply to surface cracks, effective leakage area or zone exhaust fans. If none of these fields is specified, or if Ventilation Control Mode = ZoneLevel, venting is controlled by the AirflowNetwork:MultiZone:Zone object for the thermal zone containing the window or door (ref: AirflowNetwork:MultiZone:Zone Data).}

\subsubsection{Field: Ventilation Control Mode}\label{field-ventilation-control-mode-1}

Specifies the type of surface-level natural ventilation control.

Let T\(_{out}\) equal the outdoor air temperature, T\(_{zone}\) equal the previous timestep's zone air temperature, T\(_{set}\) equal the Vent Temperature Schedule value, H\(_{zone}\) equal the specific enthalpy of zone air from the previous timestep, and H\(_{out}\) equal the specific enthalpy of outdoor air. Then the four allowed choices for Ventilation Control Mode are:

\textbf{NoVent}: The openable window or door associated with this surface is closed at all times independent of indoor or outdoor conditions. The Venting Availability Schedule is ignored in this case.

\textbf{Temperature}: The openable window or door associated with this surface is opened if T\(_{zone}\) \textgreater{} T\(_{out}\) \textbf{and} T\(_{zone}\) \textgreater{} T\(_{set}\) \textbf{and} Venting Availability Schedule (see below) allows venting.

\textbf{Enthalpy:} The openable window or door associated with this surface is opened if H\(_{zone}\) \textgreater{} H\(_{out}\) \textbf{and} T\(_{zone}\) \textgreater{} T\(_{set}\) \textbf{and} Venting Availability Schedule allows venting.

\textbf{Constant}: Whenever this object's Venting Availability Schedule allows venting, the openable window or door associated with this surface is open, independent of indoor or outdoor conditions. Note that ``Constant'' here means that the size of this opening is fixed while venting; the air flow through this opening can, of course, vary from timestep to timestep.

\textbf{ASHRAE55Adaptive}: The openable window or door associated with this surface is opened if the operative temperature is greater than the comfort temperature (central line) calculated from the ASHRAE Standard 55-2010 adaptive comfort model \textbf{and} Venting Availability Schedule allows venting.

\textbf{CEN15251Adaptive:} The openable window or door associated with this surface is opened if the operative temperature is greater than the comfort temperature (central line) calculated from the CEN15251 adaptive comfort model \textbf{and} Venting Availability Schedule allows venting.

\textbf{ZoneLevel}: Venting of the window or door is not controlled individually, but is controlled instead at the zone level. This means that the venting is determined by the AirflowNetwork:MultiZone:Zone object for the thermal zone containing the window or door (ref: AirflowNetwork:MultiZone:Zone object). This is the default value for this field.

\textbf{AdjacentTemperature}: This choice is used for an interior surface only. The openable interior window or door associated with this surface is opened if T\(_{zone}\) \textgreater{} T\(_{adjacent\\ zone}\) \textbf{and} T\(_{zone}\) \textgreater{} T\(_{set}\) \textbf{and} Venting Availability Schedule (see below) allows venting, where T\(_{adjacent\\ zone}\)is the adjacent zone temperature.

\textbf{AdjacentEnthalpy:} This choice is also used for an interior surface only. The interior openable window or door associated with this surface is opened if H\(_{zone}\) \textgreater{} H\(_{adjacent\\ zone}\) \textbf{and} T\(_{zone}\) \textgreater{} T\(_{set}\) \textbf{and} Venting Availability Schedule allows venting, where H\(_{adjacent\\ zone}\)is the adjacent zone specific enthalpy.

\subsubsection{Field: Ventilation Control Zone Temperature Setpoint Schedule Name}\label{field-ventilation-control-zone-temperature-setpoint-schedule-name-1}

The name of a schedule of zone air temperature set points that controls the opening of a window or door associated with this surface to provide natural ventilation. This setpoint is the temperature above which this openable window or door will be opened if the conditions described in the previous field Ventilation Control Mode are met.

The Ventilation Control Zone Temperature Setpoint Schedule applies only to a window or door attached to this surface that is specified using AirflowNetwork:MultiZone:Component:DetailedOpening or AirflowNetwork:MultiZone:Component:SimpleOpening.

(The discussion under the field Window/Door Opening Factor in this object describes how the actual opening area of a window or door in a particular timestep is determined.)

\emph{Modulation of Openings}

The following five fields can be used to modulate this window/door opening when Ventilation Control Mode = Temperature or Enthalpy. These fields determine a factor between 0 and 1 that multiplies the opening factor of this window or door according to the control action shown in Figure~\ref{fig:modulation-of-venting-area-according-to} for Ventilation Control Mode = Temperature and in Figure~\ref{fig:modulation-of-venting-area-according-to-001} for Ventilation Control Mode = Enthalpy. Modulation of this opening can reduce the large temperature swings that can occur if the window/door is open too far when it is venting, especially when there is a large inside-outside temperature difference.

The modulation takes the following form when Ventilation Control Mode = Temperature:

\textbf{if} T\(_{zone}\) - T\(_{out}\)~\textless{} = {[}Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor{]}~~\textbf{then}~~Multiplication factor = 1.0

\textbf{if} {[}Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor{]} \textless{} T\(_{zone}\) - T\(_{out}\) \textless{} {[}Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor{]}~ \textbf{then} Multiplication factor varies linearly from 1.0 to {[}Limit Value on Multiplier for Modulating Venting Open Factor{]}

\textbf{if} T\(_{zone}\) - T\(_{out}\)~\textgreater{} = ~ {[}Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor{]}~ \textbf{then} Multiplication factor = {[}Limit Value on Multiplier for Modulating Venting Open Factor{]}

One way of ``tuning'' the following modulation control parameters is to perform a sensitivity analysis for winter and/or summer design days to determine what combination of values causes the biggest reduction in zone air temperature fluctuations due to venting.

Note that the default values for the following fields are such that, if none of the fields are specified, modulation will not occur.

\subsubsection{Field: Minimum Venting Open Factor}\label{field-minimum-venting-open-factor-1}

See Figure~\ref{fig:modulation-of-venting-area-according-to} or Figure~\ref{fig:modulation-of-venting-area-according-to-001}. This field applies only if Ventilation Control Mode = Temperature or Enthalpy. This value may be from zero to 1.0, with the default being 0.0.

\subsubsection{Field: Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor}\label{field-indoor-and-outdoor-temperature-difference-lower-limit-for-maximum-venting-open-factor-1}

See Figure~\ref{fig:modulation-of-venting-area-according-to}. This field applies only if Ventilation Control Mode = Temperature. This value may be from zero to less than 100°C, with the default being 0°C. The value for this field must be less than the value specified for the following field.

\subsubsection{Field: Indoor and Outdoor Temperature Difference Upper Limit for Minimun Venting Open Factor}\label{field-indoor-and-outdoor-temperature-difference-upper-limit-for-minimun-venting-open-factor-1}

See Figure~\ref{fig:modulation-of-venting-area-according-to}. This field applies only if Ventilation Control Mode = Temperature. This value must be greater than 0°C, with the default being 100°C. The value for this field must be greater than the value specified for the previous field.

\subsubsection{Field: Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor}\label{field-indoor-and-outdoor-enthalpy-difference-lower-limit-for-maximum-venting-open-factor-1}

See Figure~\ref{fig:modulation-of-venting-area-according-to-001}. This field applies only if Ventilation Control Mode = Enthalpy. This value may be from zero to less than 300,000 J/kg, with the default being 0 J/kg. The value for this field must be less than the value specified for the following field.

\subsubsection{Field: Indoor and Outdoor Enthalpy Difference Upper Limit for Minimun Venting Open Factor}\label{field-indoor-and-outdoor-enthalpy-difference-upper-limit-for-minimun-venting-open-factor-1}

See Figure~\ref{fig:modulation-of-venting-area-according-to-001}. This field applies only if Ventilation Control Mode = Enthalpy. This value must be greater than zero, with the default being 300,000 J/kg. The value for this field must be greater than the value specified for the previous field.

\subsubsection{Field: Venting Availability Schedule Name}\label{field-venting-availability-schedule-name-1}

The name of a schedule that specifies when venting is available. A zero or negative schedule value means venting is not allowed. A value greater than zero means venting can occur if other venting control conditions (specified by Ventilation Control Mode and Vent Temperature Schedule Name) are satisfied. This schedule name should not be confused with Vent Temperature Schedule Name.

If a Venting Availability Schedule Name is not specified, it is assumed that venting is always available.

Using Venting Availability Schedule allows you to turn off venting at certain times of the day (at night, for example), week (on weekends, for example), or year (during the winter, for example).

If used with Ventilation Control Mode = Constant, the ventilation rate is constant only when this schedule allows venting; otherwise the ventilation rate is set to zero.

If Ventilation Control Mode = NoVent, this schedule has no effect.

\textbf{Note:} In order to establish an airflow network, each AirflowNetwork:MultiZone:Zone object must have at least two surfaces defined with AirflowNetwork:MultiZone:Surface objects, so that air can flow from one zone into other zones (or to outdoors) through the network (air mass flow conserved). In addition, for all AirflowNetwork:MultiZone:Surface objects facing the same Zone Name (ref. BuildingSurface:Detailed), at least two different environments must be defined for the other side of these surfaces (e.g., an external node and an adjacent zone, two adjacent zones, or two external nodes).

\subsubsection{Field: Occupant Ventilation Control Name}\label{field-occupant-ventilation-control-name-1}

The name of an AirlowNetwork:OccupantVentilationControl object. The object is used to perform advanced window opening control based on occupant conditions. When an object name is given, advanced window opening control is performed and, the Ventilation Control defined in the Ventilation Control Mode field will be overridden.

**<span style="color:red;">
\subsubsection{Field: Equivalent Rectangular Shape Choice}\label{equivalent-rectangular-shape- choice}bold red</span>**

**<span style="color:red;">
This field is applied to a non-rectangular window or door. The equivalent surface has the same area as the non-rectangular one. When PolygonHeight is entered, the equivalent width is equal to the area divided by the PolygonHeight.  When BaseSurfaceAspectRatio is entered, the equivalent height is equal to Square Root of Area divided by the base surface aspect ratio. The equivalent width is equal to the equivalent height * base surface aspect ratio. When UserDefinedAspectRatio is entered, the equivalent height is equal to Square Root of Area divided by UserDefinedAspectRatio. The equivalent width is equal to the equivalent height * user defined aspect ratio.</span>**

**<span style="color:red;">
\subsubsection{Field: Equivalent Rectangle Aspect Ratio}\label{equivalent-rectangular-aspect-ratio}bold red</span>**

**<span style="color:red;">
This field applies only if Equivalent Rectangular Shape Choice = UserDefinedAspectRatio. This value must be greater than zero, with the default being 1.0. </span>**

IDF examples are provided below:

\begin{lstlisting}

AirflowNetwork:MultiZone:Surface,
      Zn001:Wall001,           !- Name of Associated Heat Transfer Surface
      CR-1,                    !- Leakage Component Name
      SFacade,                 !- External Node Name
      1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}

  AirflowNetwork:MultiZone:Surface,
      Zn001:Wall001:Win001,    !- Name of Associated Heat Transfer Surface
      WiOpen1,                 !- Leakage Component Name
      SFacade,                 !- External Node Name
      0.5;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}

  AirflowNetwork:MultiZone:Surface,
      Zn003:Wall003,           !- Name of Associated Heat Transfer Surface
      Zone3 Exhaust Fan,       !- Leakage Component Name
      EFacade,                 !- External Node Name
      1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}

  AirflowNetwork:MultiZone:Surface,
      Zn001:Wall001:Win002,    !- Name of Associated Heat Transfer Surface
      WiOpen2,                 !- Leakage Component Name
      WFacade,                 !- External Node Name
      0.5;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}
      Temperature,             !- Ventilation Control Mode
      WindowVentSched,         !- Vent Temperature Schedule Name
      0.3,                     !- Limit Value on Multiplier for Modulating Venting Open Factor
                               !- {dimensionless}
      5.0,                     !- Lower Value on Inside/Outside Temperature Difference for
                               !- Modulating the Venting Open Factor {deltaC}
      10.0,                    !- Upper Value on Inside/Outside Temperature Difference for
                               !- Modulating the Venting Open Factor {deltaC}
      0.0,                     !- Lower Value on Inside/Outside Enthalpy Difference for Modulating
                               !- the Venting Open Factor {J/kg}
      300000.0,                !- Upper Value on Inside/Outside Enthalpy Difference for Modulating
                               !- the Venting Open Factor {J/kg}
      VentingSched;            !- Venting Availability Schedule Name
\end{lstlisting}


## Input Description ##

A new optional field of "Equivalent Rectangular Shape Choice" is added as the last field to give a user a choice to select which method is used to define the equivalent rectangular shape.

Revisions to the IDD are noted as **<span style="color:red;">bold red</span>** non-blocked insertions at the appropriate location throughout the input data dictionary description.

	AirflowNetwork:MultiZone:Surface,
      \min-fields 4
      \memo This object specifies the properties of a surface linkage through which air flows.
      \memo Airflow Report: Node 1 as an inside face zone;
      \memo Node 2 as an outside face zone or external node.
  	A1, \field Surface Name
      \required-field
      \type object-list
      \object-list SurfAndSubSurfNames
      \note Enter the name of a heat transfer surface.
  	A2, \field Leakage Component Name
      \required-field
      \type object-list
      \object-list SurfaceAirflowLeakageNames
      \note Enter the name of an Airflow Network leakage component. A leakage component is
      \note one of the following AirflowNetwork:Multizone objects:
      \note AirflowNetwork:MultiZone:Component:DetailedOpening,
      \note AirflowNetwork:MultiZone:Component:SimpleOpening,
      \note AirflowNetwork:MultiZone:Surface:Crack,
      \note AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea,
      \note AirflowNetwork:MultiZone:Component:HorizontalOpening, or
      \note AirflowNetwork:MultiZone:Component:ZoneExhaustFan.
      \note When the zone exhaust fan name is entered, any surface control fields below A3 are
      \note ignored when the zone exhaust fan turns on.
  	A3, \field External Node Name
      \type object-list
      \object-list ExternalNodeNames
      \note Used if Wind Pressure Coefficient Type = Input in the AirflowNetwork:SimulationControl object,
      \note otherwise this field may be left blank.
  	N1, \field Window/Door Opening Factor, or Crack Factor
      \type real
      \units dimensionless
      \minimum> 0.0
      \maximum 1.0
      \default 1.0
      \note This field specifies a multiplier for a crack, window, or door.
  	A4, \field Ventilation Control Mode
      \type choice
      \key Temperature
      \key Enthalpy
      \key Constant
      \key ASHRAE55Adaptive
      \key CEN15251Adaptive
      \key NoVent
      \key ZoneLevel
      \key AdjacentTemperature
      \key AdjacentEnthalpy
      \default ZoneLevel
      \note When Ventilation Control Mode = Temperature or Enthalpy, the following
      \note fields are used to modulate the Ventilation Open Factor for a
      \note window or door opening according to the parent zone's
      \note indoor-outdoor temperature or enthalpy difference.
      \note When Ventilation Control Mode = AdjacentTemperature or AdjacentEnthalpy, the following
      \note fields are used to modulate the Ventilation Open Factor for an interior
      \note window or door opening according to temperature or enthalpy difference
      \note between the parent zone and the adjacent zone.
      \note Constant: controlled by field Venting Schedule Name.
      \note NoVent: control will not open window or door during simulation (Ventilation Open Factor = 0).
      \note ZoneLevel: control will be controlled by AirflowNetwork:MultiZone:Zone
      \note Mode.
 	A5, \field Ventilation Control Zone Temperature Setpoint Schedule Name
      \type object-list
      \object-list ScheduleNames
      \note Used only if Ventilation Control Mode = Temperature or Enthalpy.
  	N2, \field Minimum Venting Open Factor
      \type real
      \units dimensionless
      \minimum 0.0
      \maximum 1.0
      \default 0.0
      \note Used only if Ventilation Control Mode = Temperature or Enthalpy.
  	N3, \field Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor
      \note Applicable only if Ventilation Control Mode = Temperature
      \type real
      \units deltaC
      \minimum 0.0
      \maximum< 100
      \default 0.0
  	N4, \field Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor
      \type real
      \units deltaC
      \minimum> 0.0
      \default 100.0
      \note Applicable only if Ventilation Control Mode = Temperature.
      \note This value must be greater than the corresponding lower value (previous field).
  	N5, \field Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor
      \type real
      \units deltaJ/kg
      \minimum 0.0
      \maximum< 300000.0
      \default 0.0
      \note Applicable only if Ventilation Control Mode = Enthalpy.
      \note This value must be less than the corresponding upper value (next field).
  	N6, \field Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor
      \type real
      \units deltaJ/kg
      \minimum> 0.0
      \default 300000.0
      \note Applicable only if Ventilation Control Mode = Enthalpy.
      \note This value must be greater than the corresponding lower value (previous field).
  	A6, \field Venting Availability Schedule Name
      \type object-list
      \object-list ScheduleNames
      \note Non-zero schedule value means venting is allowed if other venting control conditions are
      \note satisfied. A zero (or negative) schedule value means venting is not allowed under any
      \note circumstances. The schedule values should be greater than or equal to 0 and less than or
      \note equal to 1. If this schedule is not specified then venting is allowed if
      \note other venting control conditions are satisfied.
      \note Not used if Ventilation Control Mode = NoVent or ZoneLevel.
  	A7, \field Occupant Ventilation Control Name
      \type object-list
      \object-list AirflowNetworkOccupantVentilationControlNames
      \note Enter the name where Occupancy Ventilation Control is required.
**<span style="color:red;">
  	A8, \field Equivalent Rectangle Method </span>**

      \type choice
      \key PolygonHeight
      \Key BaseSurfaceAspectRatio
      \Key UserDefinedAspectRatio
      \default PolygonHeight
      \note This field is applied to a non-rectangular window or door. The equivalent shape has
      \note the same area as a polygonal window or door.

**<span style="color:red;">
  	N7; \field Equivalent Rectangle Aspect Ratio </span>**
       \note This field is used when UserDefinedAspectRatio is entered in the Equivalent
       \note Rectangular Method field.
       \units dimensionless
       \type real
       \minimum> 0.0
       \default 1.0


## Outputs Description ##

This section presents outputs of equivalent height and width for polygonal windows in eplusout.eio.

! <AirflowNetwork Model:Equivalent Rectangle Surface>, Name, Equivalent Height {m}, Equivalent Width {m}
AirflowNetwork Model:Equivalent Rectangle Surface, Surface 1, .60,0.47
AirflowNetwork Model:Equivalent Rectangle Surface, Surface 2, .50,0.40


## Engineering Reference ##

NA

## Example File and Transition Changes ##

No transition.

An existing example file will be modified to show such capability. A potential candidate file is AirflowNetwork3zVent.idf.

## References ##

NA

## Design Document ##

This new feature will revise several modules: DataAirflowNetwork, and AirflowNetworkBalanceManager.

The revision of DataAirflowNetwork is to add 3 members in the struct MultizoneSurfaceProp to handle new fields for polygonal surfaces. The revision of AirflowNetworkBalanceManager will involve GetInput to read new fields of the AirflowNetwork:MultiZone:Surface object, and calculate equivalent width and height for polygonal surfaces.

### AirflowNetworkBalanceManager

#### New parameters

Add 3 parameter to define choices of Equivalent Rectangle Method in the module

	int const EquivRec_Height( 1 ); // Effective rectangle polygonal height selection
	int const EquivRec_BaseAspectRatio( 2 ); // Effective rectangle base surface aspect ratio selection
	int const EquivRec_UserAspectRatio( 3 ); // Effective rectangle user input aspect ratio selection

#### Revise a GetInout section of AirflowNetwork:MultiZone:Surface

The section of reading AirflowNetwork:MultiZone:Surface object will be revised due to addition of new fields. Here is pseudo code

If ( Rectangular ) Then

	Existing operation

Else

	Reading new fileds
	If blank, then
		use default value
	else
		Read choice
		If (UserDefined), Read Aspect Ratio
		Calculate equivalent width and height
	End if

End If

#### Calculate equivalent width and height

##### PolygonHeight

Height = PolygonHeight
Width = Area / PolygonHeight

##### BaseSurfaceAspectRatio

Note: The aspect ratio of a rectangle is the ratio of its longer side to its shorter side - the ratio of width to height (https://en.wikipedia.org/wiki/Aspect_ratio)

If base surface is rectangular

Width = sqrt ( Area * BaseAspectRatio )
Height = Area /  Equivalent Width

Else

Use PolygonHeight and provide a warning with DisplayExtraWarnings = true

End if

##### UserDefinedAspectRatio

Width = sqrt ( Area * UserAspectRatio )
Height = Area /  Equivalent Width

Here are actions for special cases:

1. Horizontal surface

When PolygonHeight is entered, it is obvious that this choice is not valid. The BaseSurfaceAspectRatio is automatically selected as a first replacement. If the base surface is not rectangular, the default value of UserDefinedAspectRatio will be used to calculate equivalent width and height. A warning will be issued with DisplayExtraWarnings = true.

2. None-rectangular base surface

When BaseSurfaceAspectRatio is entered and base surface is not rectangular, PolygonHeight will be automatically selected first. If PolygonHeight is not applied, the default value of UserDefinedAspectRatio will be used to calculate equivalent width and height. A warning will be issued with DisplayExtraWarnings = true.


### DataAirflowNetwork

#### struct MultizoneSurfaceProp

Add 3 members with default values to handle new fields

		bool NonRectangular( false ); // True if this surface is not rectangular
		int EquivRecMethod( 1 ); // Equivalent Rectangle Method input: 1 Height; 2 Base surface aspect ratio; 3 User input aspect ratio
		Real64 EquivRecUserAspectRatio( 1.0 ); // user input value when EquivRecMethod = 3

