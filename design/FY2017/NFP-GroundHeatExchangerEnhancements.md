NFP - Ground Heat Exchanger Enhancements
==============

**Matt Mitchell, Jeffrey Spitler**

**Oklahoma State University**

- Original Date: Aug 10, 2017


## Justification for New Feature
Ground heat exchanger (GHE) simulation requires a substantial amount of simulation time. This is due to the long time constant of the ground.  Though the g-function method is the most computationally efficient possibility, the required superposition of the total GHE thermal history to calculate the response at the current timestep is still time-consuming. As as result, it is critical to select the best method for combining the load histories together in a manner which is computationally efficient. EnergyPlus currently uses a scheme which aggregates natural periods (day, week, month, etc.) into load blocks. However, since the model's implementation, advancements have been made over the current load aggregation algorithm which will allow shorter run times for the GHE simulation.

Simulating GHE in EnergyPlus is currently only possible by using pre-computed GHE temperature response factors known as g-functions. Each g-function is specific to a single GHE borefield configuration, i.e. a specific geometric arrangement with a single ratio of borehole spacing to depth. EnergyPlus has a small library of g-functions that only cover a handful of configurations.  For other configurations, 3rd party software is required to generate the g-functions. This can require several iterations between EnergyPlus and other software if the GHE and building design are evolving together.

As currently implemented in EnergyPlus, the simulation using g-functions makes an approximation - the mean temperature of the fluid over the borehole is the simple average of the inlet and outlet temperatures - that can lead to inaccurate exiting fluid temperature predictions at small time steps.  Beier and Spitler (2016) reported on a model that improves the accuracy at shorter time steps by computing a time-varying mean temperature that accounts for transient behavior in the borehole.

This work will improve computational speed of the GHE model by implementing a better load aggregation algorithm. EnergyPlus will also be able to calculate temperature response factors so third-party software is not required.  Finally, this work will also improve the accuracy of the model by implementing a better model of the borehole for short timestep calculations.

## E-mail and Conference Call Conclusions
N/A

## Overview

The project is broken into three main pieces: implementing a more advanced load aggregation scheme, implementing a method to generate g-function temperature response factors within EnergyPlus, and implementing a better borehole model for enhanced accuracy at short timestep.

### Load Aggregation

G-functions are temperature response factors which are used to compute the outlet temperature of a GHE borefield, The results at each time step depend on the entire history of heat transfer to/from the borefield. I.e. for each timestep, all previous GHE thermal history is used to determine the new GHE outlet temperature.  As the simulation progresses the number of hourly or sub-hourly timestep loads for all GHE history increases constantly. The temporal superposition method can therefore become quite slow to compute unless measures are taken to reduce the load histories into fewer, larger blocks.

The current method reduces the loads into hourly, daily, weekly and monthly blocks. Since this model's implementation, more advanced methods have been developed which aggregate the previous loads into blocks which are better suited for computation with minimal loss of accuracy. Claesson and Javed (2012) report on a number of these different schemes--including the method currently implemented in EnergyPlus--and propose a better method which does not involve any natural periods. Instead, the load aggregation is based solely on the temperature response function which lends itself to better computation speeds.

### G-function Generation
EnergyPlus is capable of, and often does, simulate multiple timesteps per hour. However, ground heat exchangers are often simulated for periods of one or more decades in order to determine whether the GHE is sized properly. In order to achieve accuracy over these small and large timescales, the GHE model needs to be able to calculate the response of the GHE over both the short-term and long-term.  "Short-term" can be thought of as seconds to a few hours and "long-term" from a few hours to decades.

The current version of EnergyPlus relies on the procedure originally developed by Yavuzturk and Spitler (1999) which utilized a detailed 2-dimensional finite volume method model to compute the short-term portion of the response function.  This was joined to long-term response functions computed with a detailed GHE model, capable of modeling borehole-to-borehole interference for user-specified GHE configurations.  These long-term response functions are, to this day, part of a proprietary library.

The proposed model will implement a detailed GHE model, capable of modeling borehole-to-borehole interference,  to compute the long-term temperature response factors. The long-term response will be calculated using the finite-line-source model developed by Claesson and Javed 2011b. The model relies on an analytical solution to predict the temperature field of a finite line source in an infinite medium.  The the temperature fields from multiple finite line sources are superimposed to determine the overall temperature response.

Unlike the current version of EnergyPlus, the short-term response will not be modeled with response factors.  Rather, the short-term behavior will be modeled with a borehole model described in the next section.

###Short Timestep Borehole Model

Heating and cooling loads for most buildings varies continuously over any given day. Commercial and office buildings, even in cold climates, may have cooling loads during the day and heating loads at night. For these cases, heat is exchanged to and from the borehole continuously with most of the heat transfer occurring within the borehole and not interacting with the surrounding soil. Rarely, if ever, are the building loads constant. Due to this, calculating the short-term response of a GHE is critical to the model's accuracy.

Short-term response will be calculated with the model developed by Beier and Spitler (2016).  This model accounts for the non-uniform heat transfer rate and temperature variation within the borehole, short-circuiting between the different legs of the U-tube, and the transient response of the borehole due to the finite time required for fluid to circulate through the borehole.  This model has been experimentally validated by Beier et al. (2017).  Required inputs to this model include the fluid-to-borehole-wall thermal resistance and the internal thermal resistance between the U-tube legs.  These resistances will be calculated using the 1st order multipole method, which has been validated by Javed and Spitler (2017).

## Approach
When the GHE Slinky model was implemented several years ago, the GHE code was refactored to be object oriented. Code stubs were added to accommodate this future work on this project.

The load aggregation algorithm will replace the current load aggregation algorithm which is used by the current vertical GHE model and the slinky GHE model. This will enhance EnergyPlus's computational performance and will be replace the code found in ```GLHEBase::calcAggregateLoad``` .

EnergyPlus will now compute g-functions. It will also retain the ability for users to apply g-function generated by third-party software to calculate g-functions. This code will be added to ```GLHEVert::calcGFunctions``` which is currently a blank code stub out for this work.

Since the response factors calculation will take additional time to compute, the ```ResponseFactors``` object will be written to an output file which can then be added back to the input file by the user. This will eliminate the need to recompute the g-functions during each simulation. If the borefield does not change between simulations, there is no need to recompute the temperature response factors.

The model is expected to operate as a typical object on the plant loop solver. The GHE array will receive an inlet temperature and flow rate from the upstream component, and the GHE model will solve for heat rate and outlet fluid temperature using the previously described models. Given that the heat rate is not known before hand, there needs to be a method to determine heat rate and exiting fluid temperature simultaneously. This is complicated by the fact that the thermal history of the GHE affects the current timestep's results. Additionally, if the timestep is very small (i.e. smaller than the transit time of the GHE) the thermal effects at the current timestep may have not had enough time to propagate through the model. Due to these complications, the models for GHE heat rate and exiting fluid temperature will need to be solved simultaneously, and an upper limit on GHE timestep may need to be enforced within the GHE model. Successive substitution methods may be slow to compute; therefore root finding methods will be used to determine the solution at each timestep, however, this will require some investigation to determine the best solution method.

## Input Object Changes

This work changes the ```GroundHeatExchanger:Vertical``` object to an object which only describes a single GHE. The ```GroundHeatExchanger:Array``` object is added to allow users to build GHE arrays, and the ```GroundHeatExchanger:ResponseFactors``` is added for the temperature response factors. Once the first run has occurred and EnergyPlus has generated this object, the user can add the object to the input file and point the ```GroundHeatExchanger:Array``` object to it. Once that occurs, the response factors will not be regenerated.

```
GroundHeatExchanger:Array,
    Vertical GHE 1x2 Std,    !- Name
    GHE Inlet Node 1x2 Std,  !- Inlet Node Name
    GHE Outlet Node 1x2 Std, !- Outlet Node Name
    KA Ground Temps,         !- Ground Temperature Object Name
    2.493,                   !- Ground Thermal Conductivity {W/m-K}
    2.4957E+06,              !- Ground Thermal Heat Capacity {J/m3-K}
    GHE 1x2 Std Factors      !- Response Factors Object Name
    GHE 1,                   !- GHE Object 1 Name
    GHE 2;                   !- GHE Object 2 Name

GroundHeatExchanger:Vertical,
    GHE 1                    !- Name
    0,1,                     !- X,Y location
    76.2,                    !- Borehole Depth {m}
    0.05715,                 !- Borehole Diameter {m}
    0.744,                   !- Grout Thermal Conductivity {W/m-K}
    2.5E+6,                  !- Grout Thermal Heat Capacity {J/m3-K}
    0.389,                   !- Pipe Thermal Conductivity {W/m-K}
    0.0267,                  !- Pipe Outer Diameter {m}
    0.00243,                 !- Pipe Thickness {m}
    0.0254;                  !- U-Tube Distance {m}

GroundHeatExchanger:ResponseFactors,
    GHE 1x2 Std Factors,     !- Name
    0.0005,                  !- G-Function Reference Ratio {dimensionless}
    -14.583933,              !- G-Function Ln(T/Ts) Value 1
    -3.258945,               !- G-Function G Value 1
    -14.583933,              !- G-Function Ln(T/Ts) Value 2
    -3.258945,               !- G-Function G Value 2
    -14.583933,              !- G-Function Ln(T/Ts) Value N
    -3.258945;               !- G-Function G Value N
```

## IDD

```
GroundHeatExchanger:Array,
		  \extensible:1
	A1,   \field Name
          \required-field
	      \reference-class-name validPlantEquipmentTypes
	      \reference validPlantEquipmentNames
	      \reference-class-name validCondenserEquipmentTypes
          \reference validCondenserEquipmentNames
          \reference-class-name validBranchEquipmentTypes
	      \reference validBranchEquipmentNames
	A2,   \field Inlet Node Name
          \required-field
          \type node
	A3,   \field Outlet Node Name
          \required-field
          \type node
	A4,   \field Undisturbed Ground Temperature Model Name
          \required-field
          \type object-list
          \object-list UndisturbedGroundTempModels
	N1,   \field Ground Thermal Conductivity
          \required-field
          \type real
          \minimum> 0.0
          \units W/m-K
    N2,   \field Ground Thermal Heat Capacity
          \required-field
          \type real
          \minimum> 0.0
          \units J/m3-K
	A5,   \field Response Factors Object Name
		  \type object-list
		  \object-list GroundHeatExchangerResponseFactors
	A6,   \field GHE Object Name
	      \begin-extensible
	      \type alpha
	      \object-list GroundHeatExchangerVeritical
	      \required-field
	A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19,
	A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
	A32, A33, A34, A35, A36, A37, A38, A39, A40, A41, A42, A43, A44,
	A45, A46, A47, A48, A49, A50, A51, A52, A53, A54, A55, A56, A57,
	A58, A59, A60, A61, A62, A63, A64, A65, A66, A67, A68, A69, A70,
	A71, A72, A73, A74, A75, A76, A77, A78, A79, A80, A81, A82, A83,
	A84, A85, A86, A87, A88, A89, A90, A91, A92, A93, A94, A95, A96,
	A97, A98, A99, A100;

GroundHeatExchanger:Vertical,
	A1,   \field Name
          \required-field
          \reference GroundHeatExchangerVertical
    N1,   \field X-coordinate
          \required-field
          \units m
          \type real
	N2 ,  \field Y-coordinate
          \required-field
          \units m
          \type real
	N3,   \field Borehole depth
	      \required-field
	      \type real
          \minimum> 0.0
          \units m
    N4,   \field Borehole diameter
          \required-field
          \type real
          \minimum> 0.0
          \units m
	N5,   \field Grout Thermal Conductivity
          \required-field
          \type real
          \minimum> 0.0
          \units W/m-K
	N6,   \field Grout Thermal Heat Capacity
          \required-field
          \type real
          \minimum> 0.0
          \units J/m3-K
    N7,   \field Pipe Thermal Conductivity
          \required-field
          \type real
          \minimum> 0.0
          \units W/m-K
    N8,   \field Pipe Outer Diameter
          \required-field
          \type real
          \minimum> 0.0
          \units m
    N9,   \field Pipe Thickness
          \required-field
          \type real
          \minimum> 0.0
          \units m
    N10;  \field U-Tube Distance
          \required-field
          \type real
          \minimum> 0.0
          \units m

GroundHeatExchanger:ResponseFactors,
		\extensible:2
    A1,  \field Name
         \required-field
    N1,  \field G-Function Reference Ratio
         \type real
         \minimum> 0.0
         \units dimensionless
         \default 0.0005
    N2,  \field G-Function Ln(T/Ts) Value 1
		 \begin-extensible
         \required-field
         \type real
    N3,  \field G-Function G Value 1
         \required-field
         \type real
    N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, N16, N17, N18, N19,
    N20, N21, N22, N23, N24, N25, N26, N27, N28, N29, N30, N31, N32, N33, N34,
    N35, N36, N37, N38, N39, N40, N41, N42, N43, N44, N45, N46, N47, N48, N49,
    N50, N51, N52, N53, N54, N55, N56, N57, N58, N59, N60, N61, N62, N63, N64,
    N65, N66, N67, N68, N69, N70, N71, N72, N73, N74, N75, N76, N77, N78, N79,
    N80, N81, N82, N83, N84, N85, N86, N87, N88, N89, N90, N91, N92, N93, N94,
    N95, N96, N97, N98, N99, N100, N101, N102, N103, N104, N105, N106, N107, N108,
    N109, N110, N111, N112, N113, N114, N115, N116, N117, N118, N119, N120, N121,
    N122, N123, N124, N125, N126, N127, N128, N129, N130, N131, N132, N133, N134,
    N135, N136, N137, N138, N139, N140, N141, N142, N143, N144, N145, N146, N147,
    N148, N149, N150, N151, N152, N153, N154, N155, N156, N157, N158, N159, N160,
    N161, N162, N163, N164, N165, N166, N167, N168, N169, N170, N171, N172, N173,
    N174, N175, N176, N177, N178, N179, N180, N181, N182, N183, N184, N185, N186,
    N187, N188, N189, N190, N191, N192, N193, N194, N195, N196, N197, N198, N199,
    N200, N201, N202;
```

## Documentation

### I/O Ref
\subsection{GroundHeatExchanger:Array}
\label{groundheatexchangerarray}
​
The ground heat exchanger (GHE) array object is used to define an array of GHE which operate together as as group on the condenser loop. All GHE object which makeup the array are defined on the referenced by this object.
​
	\subsubsection{Inputs}

		\paragraph{Field: Name}
		This field specifies the unique name of the GHE array object.

		\paragraph{Field: Inlet Node Name}
		This field specifies the inlet node name of the GHE array object.

		\paragraph{Field: Outlet Node Name}
		This field specifies the outlet node name of the GHE array object.

		\paragraph{Field: Ground Temperature Object Name}
		This field specifies the name of the ground temperature object.

		\paragraph{Field: Ground Thermal Conductivity}
		This numeric field specifies the thermal conductivity of the soil, in W/m-K.

		\paragraph{Field: Ground Thermal Heat Capacity}
		This numeric field specifies the thermal heat capacity of the soil, in J/m\(^{3}\)-K.

		\paragraph{Field: Response Factor Object Name}
		This field specifies the name of the response factor object which contains the g-functions used. If this object is not referenced or present, the response factors will be calculated automatically during each EnergyPlus run. Because this may cause additional simulation time, the GroundHeatExchanger:ResponseFactors object is written to the eplusout.ghe output file. Once this object has been created, it may be added to the input file and its name referenced here. When a valid response factors object is referenced here, the response factors contained within that object will be used and not generated at the beginning of the EnergyPlus simulation.

		\paragraph{Field: GHE Object Name}
		This field specifies the name of the GHE objects used to create the GHE array. This field is extensible.

		An example input is provided below.

		\begin{lstlisting}
GroundHeatExchanger:Array,
Vertical Ground Heat Exchanger, !- Name
GHE Inlet Node,                 !- Inlet Node Name
GHE Outlet Node,                !- Outlet Node Name
KA Ground Temps,		!- Ground Temperature Object Name
2.493,                   	!- Ground Thermal Conductivity {W/m-K}
2.4957E+06,              	!- Ground Thermal Heat Capacity {J/m3-K}
GHE 1x2 Std Factors      	!- Response Factors Object Name
GHE 1,                   	!- GHE Object 1 Name
GHE 2;                   	!- GHE Object 2 Name
​
		\end{lstlisting}

	\subsubsection{Outputs}

		\begin{itemize}
			\item HVAC,Average,Ground Heat Exchanger Heat Transfer Rate {[}W{]}
			\item HVAC,Average,Ground Heat Exchanger Inlet Temperature {[}C{]}
			\item HVAC,Average,Ground Heat Exchanger Outlet Temperature {[}C{]}
			\item HVAC,Average,Ground Heat Exchanger Mass Flow Rate {[}kg/s{]}
		\end{itemize}

		\paragraph{Ground Heat Exchanger Heat Transfer Rate {[}W{]}}
		Rate of heat transfer between the working fluid and the ground heat exchanger array, in Watts.

		\paragraph{Ground Heat Exchanger Inlet Temperature {[}C{]}}
		Temperature of the working fluid entering the ground heat exchanger array.

		\paragraph{Ground Heat Exchanger Outlet Temperature {[}C{]}}
		Temperature of the working fluid leaving the ground heat exchanger array.

		\paragraph{Ground Heat Exchanger Mass Flow Rate {[}kg/s{]}}
		Mass flow rate of the working fluid through the heat exchanger array.

\subsection{GroundHeatExchanger:Vertical}
\label{groundheatexchangervertical}

	This object represents a single vertical GHE. The GHE is referenced by the GroundHeatExchanger:Array object.

	\subsubsection{Inputs}

		\paragraph{Field: Name}
		This field specifies the unique name of the object.

		\paragraph{Field: X-Location}
		This field specifies the x-location of the GHE, in meters.

		\paragraph{Field: Y-Location}
		This field specifies the y-location of the GHE, in meters.

		\paragraph{Field: Borehole Depth}
		This numeric field specifies the depth of the borehole, in meters.

		\paragraph{Field: Borehole Diameter}
		This numeric field specifies the diameter of the borehole, in meters.

		\paragraph{Field: Grout Thermal Conductivity}
		This numeric field specifies the thermal conductivity of the grout material, in W/m-K.

		\paragraph{Field: Grout Thermal Heat Capacity}
		This numeric field specifies the thermal heat capacity of the grout material, in J/m\(^{3}\)-K.

		\paragraph{Field: Pipe Thermal Conductivity}
		This numeric field specifies the pipe thermal conductivity, in W/m-K.

		\paragraph{Field: Pipe Outer Diameter}
		This numeric field specifies the pipe outer diameter, in meters.

		\paragraph{Field: Pipe Thickness}
		This numeric field specifies the pipe wall thickness, in meters.

		\paragraph{Field: U-tube Distance}
		This numeric field specifies the pipe-to-pipe spacing in the borehole, in meters. This is also referred to as the ``shank spacing."

		An example input is provided below.

		\begin{lstlisting}
GroundHeatExchanger:Vertical,
Vertical Ground Heat Exchanger, !- Name
0,1,				!- X,Y location
76.2,                           !- Borehole Depth {m}
.635080E-01,                    !- Borehole Diameter {m}
.692626E+00,                    !- Grout Thermal Conductivity {W/m-K}
.234700E+07,                    !- Grout Thermal Heat Capacity {J/m3-K}
.391312E+00,                    !- Pipe Thermal Conductivity {W/m-K}
2.66667E-02,                    !- Pipe Outer Diameter {m}
2.41285E-03,                    !- Pipe Thickness {m}
2.53977E-02;                    !- U-Tube Distance {m}
		\end{lstlisting}

\subsection{GroundHeatExchanger:ResponseFactors}
\label{groundheatexchangerresponsefactors}
​
The GroundHeatExchanger:Vertical and :Array objects currently use the g-function approach to calculate GHE exiting fluid temperature and heat rate during the simulation. The heat exchanger response is defined by a G-function which is a non-dimensional function that is used to calculate the response to square heat pulses of different duration. (This function is not the same as ‘G-factors’ referred to in the ASHRAE Applications Handbook). This continuous function is specified by a series of data pairs (LNTTSi, GFNCi) where,

	\begin{itemize}
		\item LNTTSi is the non-dimensional time: ln(T/Ts)
		\item GFNCi is the G-function value
	\end{itemize}

The G-function is different for each borehole field configuration (i.e. a 4x4 field has a different response than a 80x80 field) and the borehole thermal resistance. It is also dependent on the ratio of borehole spacing to depth. G-function values, for accurate simulation, have to be calculated for each specific heat exchanger design. This can be done using some commercial ground loop heat exchanger design tool and the like. A reference data set, containing examples input data for 1x2, 4x4 and 8x8 configurations and for both standard and thermally enhanced grout, have also been provided.

EnergyPlus will create these g-function values and write this object (GroundHeatExchanger:ResponseFactors) to the eplusout.ghe file. Custom G-function values may also be generated using an external program such as GLHEPro.

	\subsubsection{Inputs}

		\paragraph{Field: Name}
		This field specifies the unique name of the object.

		\paragraph{Field: Reference Ratio (dimensionless)}
		The G-Functions may be formulated slightly differently based on the program which generated them. The ``raw'' G-Functions are based on an borehole radius to active length ratio of 0.0005. If the physical ratio is different from this, a correction must be applied. EnergyPlus will apply the correction, based on the reference ratio entered in this field. Therefore, therefore two possible input configurations.

		\begin{itemize}
		\item If the G-Functions have not had a correction applied, then the G-Functions are still based on a reference of 0.0005, so use a value of 0.0005 in this field. EnergyPlus will adjust the G-Functions internally to create the properly referenced G-Function.
		\item If the correction has already been applied, then the input G-Functions are based on a reference to the actual (physical) radius/length ratio, so enter the physical radius/length in this field. Entering the actual value will nullify any internal corrections, which will avoid re-basing the set.
		\end{itemize}

		The software GLHEPro has been making this ``pre-correction'' to the data sets since version 3.1 of that software, so this input field should match the actual (physical) radius/length ratio.

		\paragraph{Field: Ln(T/Ts) Value}
		This numeric field contains the natural log of time/steady state time: \emph{ln(T/T\(_{s}\))}. This field and the following field are extensible.

		\paragraph{Field: G-Function `G' Value}
		This numeric field contains the G-function value of the corresponding LNTTS.

		An example input is provided below.

		\begin{lstlisting}
GroundHeatExchanger:ResponseFactors,
GHE Array g-functions,          !- Name
0.0005,                         !- Reference Ratio
-15.2996, -0.348322,            ! Ln(T/Ts) Value 1, G Value 1
-14.201,  0.022208,             ! Ln(T/Ts) Value 2, G Value 2
-13.2202, 0.412345,             ! Ln(T/Ts) Value 3, G Value 3
-12.2086, 0.867498,             ! Ln(T/Ts) Value 4, G Value 4
-11.1888, 1.357839,             ! Ln(T/Ts) Value 5, G Value 5
-10.1816, 1.852024,             ! Ln(T/Ts) Value 6, G Value 6
-9.1815,  2.345656,             ! Ln(T/Ts) Value 7, G Value 7
-8.6809,  2.593958,             ! Ln(T/Ts) Value 8, G Value 8
-8.5,     2.679,                ! etc, etc.
-7.8,     3.023,
-7.2,     3.32,
-6.5,     3.681,
-5.9,     4.071,
-5.2,     4.828,
-4.5,     6.253,
-3.963,   7.894,
-3.27,    11.82,
-2.864,   15.117,
-2.577,   18.006,
-2.171,   22.887,
-1.884,   26.924,
-1.191,   38.004,
-0.497,   49.919,
-0.274,   53.407,
-0.051,   56.632,
0.196,    59.825,
0.419,    62.349,
0.642,    64.524,
0.873,    66.412,
1.112,    67.993,
1.335,    69.162,
1.679,    70.476,
2.028,    71.361,
2.275,    71.79,
3.003,    72.511;               !- 35 PAIRS
		\end{lstlisting}


### Eng. Ref
TBD

## Testing/Validation/Data Sources

TBD

## References

Beier, R.A. and J.D. Spitler, 2016. 'Weighted average of inlet and outlet temperatures in borehole heat exchangers.' Applied Energy. 174(2016): 118-129.

Beier, R.A., M.S. Mitchell, J.D. Spitler, and S. Javed, 2017. 'Validation of borehole heat exchanger models against multi-flow rate thermal response tests.' Geothermics. Manuscript submitted.

Claesson, J. and S. Javed, 2011a. 'New analytical and numerical solutions for the short-term analysis of vertical ground heat exchangers.' ASHRAE Transactions. 117(1): 3-12.

Claesson, J. and S. Javed, 2011b. 'An analytical method to calculate borehole fluid temperatures for time-scales from minutes to decades.' ASHRAE Transactions. 117(2): 279-288.

Claesson, J. and S. Javed, 2012. 'A load-aggregation method to calculate extraction temperatures of borehole heat exchangers.' ASHRAE Transactions. 118(1): 530-539.

Javed, S. and J.D. Spitler, 2017. 'Accuracy of borehole thermal resistance calculation methods for grouted single u-tube ground heat exchangers.' Applied Energy. 187(2017): 790-806.

Yavuzturk, C., J.D. Spitler. 1999. A Short Time Step Response Factor Model for Vertical Ground Loop Heat Exchangers. ASHRAE Transactions. 105(2):475-485.
