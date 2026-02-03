Allow Multiple Ground Surface Temperature and Reflectance Objects
======================

**Bereket Nigusse**

**Florida Solar Energy Center**

 - First draft: April 26, 2022
 - Modified Date: May 6, 2022
 - Added Design Document, May 16, 2022


## Justification for New Feature ##

Currently EnergyPlus only allows one single ground surface with user defined ground solar reflectance and ground temperature. Each exterior surface has a single view factor to ground. A real building usually sees multiple types of ground surfaces, e.g., bare soil, grass, sidewalks, driveways, water surface, which may have different solar reflectance and ground temperature.

**- The new feature was requeted by LBNL **

**- this feature is intended for use with exterior surfaces only **

## E-mail and  Conference Call Conclusions ##

- technicalities call: May 4, 2022

- technicalities call recap is on github.

- additional comments and responses on github PR #9409

## Overview ##

### Current Code ###

(1) Exterior surface LWR exchange with the ground is calculated using outside air dryblub temperature (default)

(2) Currently single ground surface temperature can be specified using `SurfaceProperty:SurroundingSurfaces` object

(3) Existing model uses global Site:GroundReflectance object

**- This enhancement allows each exterior surfaces to see different ground surface temperature and reflectance objects.

## Implementation Approach ##

*(1) This new feature oe enhancemnt can be implemented using an existing object and one new Object:

     Existing object: `SurfaceProperty:LocalEnvironment`

     New Object: `SurfaceProperty:GroundSurfaces`

	 The new object will have multiple ground surfaces properties of ground view factor, ground temperature and ground reflectance

	 Ground surfaces are assumed horizontal surfaces for solar reflection calculations and relected solar radiations are assumed diffuse.

*(2) Requires modifying SurfaceProperty:LocalEnvironment object:

     Add a new field `Ground Suraces Object Name` to this object. This field will be used to specify the name of the new object.

*(3) Two redundant input fields in SurfaceProperty:SurroundingSurfaces will be deprecated

*(4) Ground surfaces are assumed horizontal surfaces for solar reflection calculations

![Figure 1](GroundSurfaceProperties.PNG)


** Figure 1. Surface Property Object **


### Exsting Object SurfaceProperty:LocalEnvironment ###


```
SurfaceProperty:LocalEnvironment,
       \min-fields 3
       \memo This object defines the local environment properties of an exterior surface.
       \memo One or more environment properties have to be defined and linked to the exterior surface.
   A1, \field Name
       \required-field
       \type alpha
       \reference SurfaceLocalEnvironmentNames
   A2, \field Exterior Surface Name
       \type object-list
       \object-list SurfaceNames
       \note Enter the name of an exterior surface object
   A3, \field External Shading Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Enter the name of a Schedule object
   A4, \field Surrounding Surfaces Object Name
       \type object-list
       \object-list SurroundingSurfacesNames
       \note Enter the name of a SurfaceProperty:SurroundingSurfaces object
   A5, \field Outdoor Air Node Name
       \type object-list
       \object-list OutdoorAirNodeNames
       \note Enter the name of an OutdoorAir:Node object
   A6; \field Ground Surfaces Object Name
       \type object-list
       \object-list GroundSurfacesNames
       \note Enter the name of a SurfaceProperty:GroundSurfaces object
	   \note This field is used with SurfaceProperty:GroundSurfaces object
```


### New Object SurfaceProperty:GroundSurfaces ###


```
SurfaceProperty:GroundSurfaces,
       \min-fields 3
       \memo This object defines a list of ground surfaces for use with an exterior surface.
       \extensible:4 -- duplicate last set of ground surface properties (the last four fields), remembering to remove ; from "inner" fields.
   A1, \field Name
       \required-field
       \type alpha
       \reference GroundSurfacesNames
   A2, \field Ground Surface 1 Name
       \begin-extensible
       \required-field
       \type alpha
   N1, \field Ground Surface 1 View Factor
       \required-field
	   \type real
	   \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.5
   A3, \field Ground Surface 1 Temperature Schedule Name
	   \type alpha
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
   A4, \field Ground Surface 1 Reflectance Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are fraction, 0.0 to 1.0, units dimensionless
   A5, \field Ground Surface 2 Name
       \type alpha
       \note optional
   N2, \field Ground Surface 2 View Factor
       \type real
	   \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.0
       \note optional
   A6, \field Ground Surface 2 Temperature Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
       \note optional
   A7, \field Ground Surface 2 Reflectance Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are fraction, 0.0 to 1.0, units dimensionless
       \note optional
   A8, \field Ground Surface 3 Name
       \type alpha
       \note optional
   N3, \field Ground Surface 3 View Factor
       \type real
	   \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.0
       \note optional
   A9, \field Ground Surface 3 Temperature Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
       \note optional
  A10, \field Ground Surface 3 Reflectance Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are fraction, 0.0 to 1.0, units dimensionless
       \note optional
  A11, \field Ground Surface 4 Name
       \type alpha
       \note optional
   N4, \field Ground Surface 4 View Factor
       \type real
	   \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.0
       \note optional
  A12, \field Ground Surface 4 Temperature Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
       \note optional
  A13, \field Ground Surface 4 Reflectance Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are fraction, 0.0 to 1.0, units dimensionless
       \note optional
  A14, \field Ground Surface 5 Name
       \type alpha
       \note optional
   N5, \field Ground Surface 5 View Factor
       \type real
	   \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.0
       \note optional
  A15, \field Ground Surface 5 Temperature Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
       \note optional
  A16; \field Ground Surface 5 Reflectance Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are fraction, 0.0 to 1.0, units dimensionless
       \note optional
```


### Existing Object SurfaceProperty:SurroundingSurfaces ###

Requires removing or deprecating the following two redundant input fields from SurfaceProperty:SurroundingSurfaces object since ground view factors and ground surface temperatures are now specified in the new object SurfaceProperty:GroundSurfaces.


```
SurfaceProperty:SurroundingSurfaces,
       \min-fields 10
       \memo This object defines a list of surrounding surfaces for an exterior surface.
       \extensible:4 -- duplicate last set of surrounding surface properties (the last four fields), remembering to remove ; from "inner" fields.

		...

   N2, \field Ground View Factor
       \minimum 0.0
       \maximum 1.0
       \default 0.5
       \note optional
   A3, \field Ground Temperature Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule values are real numbers, -100.0 to 100.0, units C
       \note optional
```



## Testing/Validation/Data Source(s): ##

Demonstrate that the new approach duplicates the current results using exact set of inputs. Unit tests will be added to demonstrate the new feature.

## Input Output Reference Documentation ##

\subsection{SurfaceProperty:GroundSurfaces}\label{surfacePropertyGroundSurfaces}

This object is used for calculating building exterior surfaces long-wave radation exchange with the ground, and calculation of solar radiation reflection from ground surfaces to a building exterior surface. A given exterior surface can view multiple ground surfaces with different surface properties and view factors. Thus, this object allows to specify multiple ground view factors, ground surfaces temperature, and ground surfaces refelectance properties viewed by a given exterior surface. View factors are assumed to be constant values. The ground surface temperature and ground surface reflectance are specified using schedule object. At least one ground surface should be defined in this object.

The sum of all defined view factors that includes sky virew factor, ground surfaces view factors and surrounding surfaces view factors seen by a given building exterior surface should be 1.0, or the sum of the ground view factors for a given exterior surface should not exceed 1 minus the sky view plus the sum of view factors of surrounding surfaces defined in SurfaceProperty:SurroundingSurfaces object.

\subsubsection{Field: Name}\label{field-name}

This is a unique name of the surface property ground surfaces object.

\subsubsection{Field: Ground Surface 1 Name}\label{field-ground-surface-1-name}

This is a unique name for ground surface 1.

\subsubsection{Field: Ground Surface 1 View Factor}\label{field-ground-surface-1-view-factor}

This field defines the constant ground view factor of a building exterior surface to ground surface 1. The ground surface view factor is used in long-wave radiation exchange and ground surface solar reflectance calculations.

\subsubsection{Field: Ground Surface 1 Temperature Schedule Name}\label{field-ground-surface-1-temp-schedule-name}

This field is used to provide a schedule name of the ground surface 1 temperature. The ground surface temperature used in long-wave radiation exchange calculation of a building exterior surface with the ground surface. If the field is left blank, the default method will be used.

\subsubsection{Field: Ground Surface 1 Reflectance Schedule Name}\label{field-ground-surface-1-reflectance-schedule-name}

This field is used to provide a schedule name of the of a ground surface reflectance. The ground surface reflectance is used to calculate ground surface reflected solar radition to a building exterior surface. If the field is left blank, the global reflectance object defined else where or default values will be used.

This object is extensible, so the last four fields can be repeated to define ground surface name, ground surface view factor, ground surface temperature, and ground surface reflectace sets.

Ground surfaces solar reflectance values derived from satelite data for common surfaces obtained from Ground Albedo Measurements and Modeling (Bill Marion, 2018) are summerized in table below.

\begin{longtable}[c]{p{1.5in}p{3.5in}p{1.5in}}
\toprule
Item & Ground Surface Type & Ground Surface Reflectance [] \tabularnewline
\midrule
\endfirsthead

\toprule
Item & Ground Surface Type & Ground Surface Reflectance [] \tabularnewline
\midrule
\endhead

1 & Grass & 0.15 - 0.26 \tabularnewline
2 & Black Earth & 0.08 - 0.13 \tabularnewline
3 & White Sand, New Mexico & 0.60 \tabularnewline
4 & Snow & 0.55 - 0.98 \tabularnewline
5 & Asphalt Pavement & 0.09 - 0.18 \tabularnewline
6 & Concrete Pavement & 0.20 - 0.40 \tabularnewline

\bottomrule
\end{longtable}

An example IDF objects.

\begin{lstlisting}
SurfaceProperty:GroundSurfaces,
  GndSurfs:South,              !- Name
  GndSurfSouth_Parking,        !- Ground Surface 1 Name
  0.2,                         !- Ground Surface 1 View Factor
  GndSurfsSouth_Parking_Sch,   !- Ground Surface 1 Temperature Schedule Name
  GndSurfsSouth_Parking_Sch,   !- Ground Surface 1 Reflectance Schedule Name
  GndSurfSouth_Grass,          !- Ground Surface 2 Name
  0.2,                         !- Ground Surface 2 View Factor
  GndSurfsSouth_Grass_Sch,     !- Ground Surface 2 Temperature Schedule Name
  GndSurfsSouth_Grass_Sch,     !- Ground Surface 2 Reflectance Schedule Name
  GndSurfSouth_Water,          !- Ground Surface 3 Name
  0.1,                         !- Ground Surface 3 View Factor
  GndSurfsSouth_Water_Sch,     !- Ground Surface 3 Temperature Schedule Name
  GndSurfsSouth_Water_Sch;     !- Ground Surface 3 Reflectance Schedule Name
\end{lstlisting}


\begin{lstlisting}
SurfaceProperty:LocalEnvironment,
    LocEnv:Zn001:Wall001,      !- Name
    Zn001:Wall001,             !- Exterior Surface Name
    ,                          !- External Shading Fraction Schedule Name
    ,                          !- Surrounding Surfaces Object Name
    OutdoorAirNode:0001,       !- Outdoor Air Node Name
	GndSurfs:South;            !- Ground Surfaces Object Name
\end{lstlisting}

## Engineering Reference ##

### External Longwave Radiation ###

The total longwave radiative heat flux is the sum of components due to radiation exchange with the ground, sky, air, and surrounding surfaces.

\begin{equation}
{q''_{LWR}} = {q''_{gnd}} + {q''_{sky}} + {q''_{air} + {q''_{sur_surf)}}
\end{equation}


Long-wave radiation exchange of a building exterior surface with multiple ground surfaces is given by:

\begin{equation}
{q''_{gnd}} = \varepsilon \sigma \sum\limits_{j = 1}^{{N_{gnd_surfaces}}} {F_{gnd,j}} \left(T_{gnd,j}^4 - T_{surf}^4 \right)
\end{equation}

where

$\varepsilon$ = long-wave emittance of a building exterior surface

$\sigma$ = Stefan-Boltzmann constant

F\(_{gnd, j}\) = view factor of a building exterior surface to jth ground surface

T\(_{gnd, j}\) = jth ground surface temperature

T\(_{surf}\) = building outside surface temperature

N\(_{gnd_surfaces}\) = Number ground surfaces seen by a building exterior surface


The above equation can be recast using multiple ground surfaces average temperature seen by an exterior surface as follows:

\begin{equation}
{q''_{gnd}} = \varepsilon \sigma \{F_{gnd, sum}} \left(T_{gnd,avg}^4 - T_{surf}^4 \right)
\end{equation}

\begin{equation}
{F_{gnd,sum}} = \sum\limits_{j = 1}^{{N_{gnd_surfaces}}} {F_{gnd,j}}
\end{equation}

\begin{equation}
{T_{gnd,avg}} = ((\sum\limits_{j = 1}^{{N_{gnd_surfaces}}} {F_{gnd,j}} \left(T_{gnd,j}^4)) / \{F_{gnd,sum}})^{{1/4}}
\end{equation}

where

T\(_{gnd,avg}\) = view factor weighted average surface temperature of multiple ground surfaces seen by an exterior surface

F\(_{gnd,sum}\) = sum of the view factors of an exterior surfaces to multiple ground surfaces


### Ground Surfaces Solar Reflectance ###

Ground reflected solar radiation is calculated for beam and diffuse components separately.

\subsection{Sky Solar Radiation Diffusely Reflected from the Ground}\label{sky-solar-radiation-diffusely-reflected-from-the-ground}

The diffuse irradiance received on a building exterior surface due to sky solar radiation reflected from multiple ground surfaces at each time step is given by:

\begin{equation}
\begin{array}{l}
{QRadSWOutIncSkyDiffReflGnd(RecSurfNum)} = \\{DifSolarRad * {\rho_{gnd,avg}} * SurfReflFacSkySolGnd(RecSurfNum)~ (W/m^{2})}
\end{array}
\end{equation}

where
rho\(_{gnd,avg}\) = view factor weighed ground solar reflectance of multiple ground surfaces seen by an exterior surface

The view factor weighted average reflectance of multiple ground surfaces is calculated from the ground refelctances and view factors of the ground surfaces seen by the exterior surface and is given by:

\begin{equation}
{\rho_{gnd,avg}} = \sum\limits_{j = 1}^{{N_{gnd_surfaces}}} {\rho_{gnd,j}} * {F_{gnd,j}} / \sum\limits_{j = 1}^{{N_{gnd_surfaces}}} {F_{gnd,j}}
\end{equation}

where

rho\(_{gnd, j}\) = reflectance of the jth ground surface seen by an exterior surface


\subsection{Beam Solar Radiation Diffusely Reflected from the Ground}\label{beam-solar-radiation-diffusely-reflected-from-the-ground}

The beam radiation is assumed to be reflected uniformaly to all directions. The diffuse irradiance received by a building exterior surface due to beam solar diffusely reflected from multiple ground surfaces is given by:

\begin{equation}
\begin{split}
QRadSWOutIncBmToDiffReflGnd(RecSurfNum) =
\\BeamSolarRad * {\rho_{gnd, avg}} * (WeightNow * SurfReflFacBmToDiffSolGnd(RecSurfNum,HourOfDay)
\\+ WeightPreviousHour * SurfReflFacBmToDiffSolGnd(RecSurfNum,PreviousHour))
\end{split}
\end{equation}


## Example File and Transition Changes ##

An example file will be modified to demonstrate the use of multiple ground surface objects. Simulation results will be examined and sample results will be provided.

Transition is required to remove two redundant fields in SurfaceProperty:SurroundingSurfaces object. These redundant fields are "Ground View Factor" and "Ground Temperature Schedule Name".

## Proposed Report Variables: ##

Output variables applicable to SurfaceProperty:GroundSurfaces object are:

Zone,Average,Surfaces Property Ground Surfaces Average Temperature [C]
Zone,Average,Surfaces Property Ground Surfaces Average Reflectance []


Surfaces Property Ground Surfaces Average Temperature [C]
This is average surface temperature of multiple ground surfaces in deg C viewed by an exterior surface for each time step. If there is only one ground surface specified in a given SurfaceProperty:GroundSurfaces object, then the average surface temperature will be the same as the specified ground surface temperature.


Surfaces Property Ground Surfaces Average Reflectance []
This is average surface reflectance of multiple ground surfaces viewed by an exterior surface for each time step. If there is only one ground surface specified in a given SurfaceProperty:GroundSurfaces object, then the average surface reflectance will be the same as the specified ground surface reflectance.

## References ##

Bill Marion. 2018. Ground Albedo Measurements and Modeling. Bifacial PV Workshop Lakewood, Colorado. September 11, 2018
https://www.nlr.gov/docs/fy20osti/72589.pdf




## Design Documentation ##

The new feature will modify modules:

   DataSurfaces,

   SurfaceGeomtery,

   HeatBalanceSurfaceManager,

   HeatBalFinieDiffMamanger; and

   ConvectionCoefficients.

   Some existing functions will be modified and new functions will be added for getinput, average ground temperature and ground reflectance calculations.


### Design GroundSurfaces Temperature ###

Adds new function that calculates ground surfaces average temperature for each SurfaceProperty:GroundSurfaces object.

#### DataSurfaces.hh ####

	*// adds module level two new member vectors*
    Array1D<bool> IsSurfPropertyGndSurfacesDefined; // true if ground surfaces properties are listed for an external surface
    Array1D<int> GroundSurfsPropertyNum;            // index to a ground surfaces list (defined in SurfaceProperties::GroundSurfaces)

    *Adds new struct for ground surfaces data:*

    // ground surfaces data
    struct GroundSurfacesData
    {
        // Members
        std::string Name;  // name of a ground surface
        Real64 ViewFactor; // view factor to a ground surface
        int TempSchPtr;    // pointer to a ground surface temperature schedule object
        int ReflSchPtr;    // pointer to a ground Surface reflectance schedule object

        // Default Constructor
        GroundSurfacesData() : ViewFactor(0.0), TempSchPtr(0), ReflSchPtr(0)
        {
        }
    };

	*Adds new struct for ground surfaces properties:*

	*// ground surfaces object*
    struct GroundSurfacesProperty
    {
        // Members
        std::string Name;                     // name of multiple ground surfaces object
        int NumGndSurfs;                      // number of groundSurfaces
        Array1D<GroundSurfacesData> GndSurfs; // ground surfaces data
        Real64 SurfsTempAvg;                  // ground Surfaces average temperature at each time step
        Real64 SurfsReflAvg;                  // ground Surfaces average reflectance at each time step
        Real64 SurfsViewFactorSum;            // sum of view factors of ground surfaces seen by an exterior surface

        // Default Constructor
        GroundSurfacesProperty() : NumGndSurfs(0), SurfsTempAvg(0.0), SurfsReflAvg(0.0), SurfsViewFactorSum(0.0)
        {
        }
    };

    *Add a new member variables to SurfaceLocalEnvironment *

    struct SurfaceLocalEnvironment
    {
        // Members
        std::string Name;
        int SurfPtr;                // surface pointer
        int ExtShadingSchedPtr;     // schedule pointer
        int SurroundingSurfsPtr;    // schedule pointer
        int OutdoorAirNodePtr;      // schedule pointer

		*// add new member pointer to SurfaceProperty:GroundSurfaces object*
		int GndSurfsPtr;            // pointer to multiple ground surfaces object

        // Default Constructor
        SurfaceLocalEnvironment() : SurfPtr(0), ExtShadingSchedPtr(0), SurroundingSurfsPtr(0), OutdoorAirNodePtr(0), GndSurfsPtr(0)
        {
        }
    };


#### SurfaceGeomtery.cc ####

    *// new getinput function for SurfaceProperty:GroundSurfaces object*
    void GetSurfaceGroundSurfsData(EnergyPlusData &state, bool &ErrorsFound)
    {
      *// read input data for ground surfaces properties used in building exterior surface*
	}

    *// add a calling point for new getinput function*
	void GetSurfaceData(EnergyPlusData &state, bool &ErrorsFound)
    {
	  *// calling the new getinput function*
      GetSurfaceGroundSurfsData(state, ErrorsFound);
    }

	*// modify the getinput function for SurfaceProperty:LocalEnvironment*
    void GetSurfaceLocalEnvData(EnergyPlusData &state, bool &ErrorsFound)
    {
	  *// read in new input field Ground Surfaces Object Name*
	}


#### HeatBalanceSurfaceManager.cc ####

    *// adds a new function that calculates average ground surface temperature*
    void GetGroundSurfacesTemperatureAverage(EnergyPlusData &state)
    {
      *// returns ground surfaces average temperature in degree C*
      *// ground temperature seen by a building exterior surface*
      *// view factor weighted multiple ground surfaces average temperature*
    }


    void CalcOutsideSurfTemp()
    {

    ...
    *// set average ground surfaces temperature for use with exterior surfaces*
    if (IsSurfPropertyGndSurfacesDefined(SurfNum)) {
        *TGround = set to ground surfaces average temperature*
    }

    ...
   }

#### HeatBalFinieDiffMamanger.cc ####

    void ExteriorBCEqns()
	{

    *// Set ground surfaces temperature for use with outside surface heat balance (Finite Diff Method)*
    if (IsSurfPropertyGndSurfacesDefined(SurfNum)) {
        *TGround = set to ground surfaces average temperature*
    }

	}
#### ConvectionCoefficients.cc ####


    void InitExteriorConvectionCoeff()
    {

    ...
    // set average ground surfaces temperature for use with exterior surfaces
    if (IsSurfPropertyGndSurfacesDefined(SurfNum)) {
       *TGround = set to ground surfaces average temperature*
    }

    ...
   }



### Design Ground Reflectance ###

Ads new function that calculates ground surfaces average reflectance for each SurfaceProperty:GroundSurfaces object.

#### HeatBalanceSurfaceManager.cc ###

    *// add a functions that updates ground reflectance each time step*
    void GetGroundSurfacesReflectanceAverage(EnergyPlusData &state)
    {
       *// returns average ground surfaces reflectance for each timestep*
       *// ground reflectance seen by a building exterior surface*
    }

    void InitSolarHeatGains(EnergyPlusData &state)
    {
      *// Adds a calling point for the new function that updates ground surfaces average reflectance*

      *// updates ground surfaces average reflectance*
      GetGroundSurfacesReflectanceAverage(state);

	  *// set ground surfaces average reflectance value in several places for reflected solar*
	  *// radiation calculations for exteriour opaque surfaces and windows at each timestep*
	  *// in InitSolarHeatGains() function.*

    }


### Note: ###

The ground surfaces temperature and reflectance are used for exterior surfaces only. Ground surfaces average temperature and average reflectance are used when the multiple ground surfaces are specified. Averaging the ground the temperature and reflectance values does not impcat results.
