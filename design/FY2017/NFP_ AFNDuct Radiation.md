NFP: Air Duct Radiative Heat Transfer
===============

Anthony D. Fontanini, Matt Mitchell, and Jan Kosny

Fraunhofer Center for Sustainable Energy Systems (CSE)

5 Channel Center Street, Boston, MA, 02210.

## Justification for New Feature

Air ducts are often partially or fully exposed in commercial spaces and residential attics and crawl spaces.  Air ducts in EnergyPlus currently include conduction losses, but do not include radiation heat transfer.  Radiative heat transfer from supply and return ducts can be extremely important if the air ducts are uninsulated or the zone that the ducts run through is unconditioned (ex: attics or crawl spaces).  The goal of this new feature is to include a relatively simple but accurate method for calculating radiative heat transfer between air ducts and zone surfaces in conditioned and unconditioned zones.  The new feature will allow for better heat gain and loss calculation for fully exposed and partially buried air ducts.

## Overview

EnergyPlus will be modified to incorporate air duct radiation, which will provide users with enhanced air duct modeling support. The specific model being extended in EnergyPlus is the AirflowNetwork:Distribution:Component:Duct.  Based on the recommendations of the NREL development team, a new input object for user defined duct-to-surface view factors will be developed.  The project will focus on including providing duct-surface view factors and implementing a radiative flux for air ducts in both conditioned and unconditioned zones into EnergyPlus.

## Approach

### State-of-the-art

The most advanced method for calculating view factors for air ducts is in ASHRAE RP-717 (RP-717) [1]. RP was an advancement on the work of ASTM C1340 [2] and has since been integrated into the standard text, but not the software distributed with the standard.  Although RP-717 focuses on attics, most of the assumptions are applicable to conditioned zones.  In RP-717, a set of simplifying assumptions are made to estimate the view factors between surface-to-surface, duct-to-surface, and surface-to-duct in enclosed attics.  A recent letter has discussed the importance of air duct radiation heat transfer [3].  In the conclusions of this report, the authors attribute some of the discrepancies seen in their investigations as the result of treatment lack of radiation heat transfer from air ducts in EnergyPlus.  The letter also recommends that the RP-717 view factor assumptions be integrated into EnergyPlus.  These assumptions are detailed below.  Although these engineering assumptions are stated in RP-717, there is little justification or discussion of their accuracy, applicability, or their limitations.

Assumptions made in RP 717:

1. For radiation heat transfer calculations, each duct may be treated as being isothermal, gray, and diffusely emitting and reflecting.

2. The view factor between any duct run and the floor of the attic enclosure is 0.5.

3. The presence of ducts reduces the view factors from the attic floor to the other attic surfaces by a constant factor.

4. The view factor from a duct to a particular attic surface is the same for all duct segments.

5. The presence of ducts modifies only those view factors that involve the attic floor.

6. The ducts are small enough that view factors between any two duct runs may be considered negligible compared with view factors between the ducts and the attic surfaces.

Other than using the engineering assumptions in RP-717 and analytical formulas for calculation of the view factor matrix, a numerical software that computes view factors based on solving the integral equation can be used. There is a currently a software, View3D [4], used to evaluate view factors. The framework was written in C originally by Walton at NIST [5,6]. Gaussian quadrature is used to evaluate the view factor double integral. The framework can solve view factors for both convex and non-convex geometries and use adaptive refinement to solve obstructed view factors.  View3D also has algorithms with the ability to calculate diffusely emitting view factors [7].

Recently a joint effort has been undertaken by Iowa State University and Fraunhofer CSE to evaluate these view factor assumptions, and make modifications when necessary.  The work uses a set of test cases that combines different attic geometries, duct run shapes, and variable emissivity to evaluate the assumptions using View3D.  Based on this analysis, assumption 1, 2, and 3 seem to be relatively reasonable.  Assumption 4 needs some modification to account for the distance a duct is from every surface in the zone and the projected area of the duct to a given surface.  The 5th assumption seems to be accurate as long as the surface area ratio (AZ/Ad) is larger than 15, where AZ is the total surface area of the zone and Ad is the total surface area of all the duct runs in the zone.  Assumption 6 seems to be relatively accurate for ducts that are oriented at angles greater than or equal to 90 degrees and parallel duct runs that are sufficiently far apart. Assumption 6 may not be accurate for ducts at acute angles and close parallel ducts.

### Air duct geometry representation

In order to approximate view factors for ducts in building zones, some assumptions about the geometric representation need to be made to determine the diffusely emitting view factors for the ducts and the surfaces in the zone.  There are a few options for implementing the geometric representation of duct models, each with a different level of complexity for developers and users.

Options for duct representation

1. Full geometric representation of duct in zone (like FATM [8]). In this option, the location of the duct in the zone is fully defined by the user. This will be required to allow automated detailed view factor calculations for radiation heat transfer calculations between the duct and the zone.
2. Simplified geometric representation.

    *  The duct will be assumed to be sitting against or next to a surface in the zone with some user defined orientation.
    *  The duct will be assumed to be sitting in the center of the space with some user defined orientation.

3. Basic geometric representation. The air duct radiates to the surfaces based on the surfaces’ surface areas and distance from the center of the zone.
4. A user defined matrix.  In this case the view factor matrix (black body or diffusely emitting) can be specified by the user through an input file.

Each option has its own advantages and disadvantages, which are outlined in the table below.  Based on the table below the "Full description" would most likely be too difficult for the scope of this project and would be too computationally intensive.  The simplified approach would be a good balance between computational speed, implementation difficulty, and accuracy.  The basic approach may lead to some accuracy issues in some applications, but would be the simplest to implement and test.  The user defined matrix option would be a good feature to make all the connections in the code and test the simplified or basic approaches to determine which approach is the best to implement later.

<table>
  <tr>
    <td>Duct Geometric Representation</td>
    <td>Advantages</td>
    <td>Disadvantages</td>
  </tr>
  <tr>
    <td>Option 1: Full description</td>
    <td>Most accurate approach
Good for research and experimental validation</td>
    <td>Computationally intensive
Most difficult to implement
Difficult for standard users to use
Take the most time to set up simulation
Potential licensing issues with View3D</td>
  </tr>
  <tr>
    <td>Option 2: Simplified </td>
    <td>Computationally cheaper than fully description
Should be accurate for most building zones</td>
    <td>Moderately difficult to implement</td>
  </tr>
  <tr>
    <td>Option 3: Basic </td>
    <td>Simple to implement
Computationally cheap</td>
    <td>May not be accurate for some buildings because approach is too simple</td>
  </tr>
  <tr>
    <td>Option 4: User defined matrix</td>
    <td>Would be a good feature for research and experimental validation
Could use any method for calculating view factors
Users could determine how accurate the values need to be
Could be used to test accuracy of other view factor algorithms
Fairly easy to implement</td>
    <td>Third-party software or analytical methods are needed by the users.</td>
  </tr>
</table>


### Recommendations for duct representation

Based on the table and discussion above the full representation would be too difficult to implement and may lead to licensing problems, so this approach is not recommended.  The simplified and basic approaches would allow for some automation in the calculation process as users would not have to specify each view factor.  But, these methods have not been properly analyzed to implement these approaches without knowing their accuracy and energy implications.  Therefore, the simplified and basic approaches are not recommended to be implemented in this phase of the project, but are good candidates for future features.  Based on the table above the user defined matrix option has the most advantages. It is recommended that this approach should be implemented to allow flexibility by the users and the testing of automated view factor calculation algorithms.

### Modification locations in EnergyPlus

Radiation exchange is between the ducts and the heat transfer and thermal mass surfaces.  Since the ducts are in "AirflowNetworkBalanceManager" namespace and the surfaces radiation heat transfer is calculated in the “HeatBalanceIntRadExhange” namespace , modifications to each section is necessary to include the radiation effects of air ducts.  The “AirflowNetworkBalanceManager” and the “HeatBalanceIntRadExhange” namespaces can operate at different timesteps.  With this in mind, careful considerations of the radiation heat transfer exchange is needed.  As described in the engineering reference, the accumulated radiative effect of the duct needs to be applied to the surfaces that ducts radiates energy to during the envelope time step.  This results in the duct surface temperatures being updated at each system timestep, while the surface is updated with the duct radiative losses at each envelop timestep.

#### Modifications to HeatBalanceIntRadExhange namespace

Surface-to-surface view factors and radiation are handled within this namespace. To account for radiation heat transfer between the duct and zone surfaces, the surface-to-surface radiation needs to be modified to maintain an energy balance. The process whereby that is accomplished is outlined below.

To take the user-defined view factor approach in implementing duct-surface radiation heat transfer, two different approaches need to be explored. First, either the duct needs to have a physical, geometrical representation within the EnergyPlus simulation so the surface-surface view factors can be modified, or second, the effects of the duct-surface heat transfer needs to be applied to the participating surfaces with appropriate methods. In the first option, the duct radiation object needs to be given a geometry and location within a zone by the user. Without that information, the view factors that EnergyPlus creates between zone surface cannot be appropriately modified to account for the ducts. After that, EnergyPlus will need to automatically create a surface(s) which will participate in the interior surface radiation exchange. In the second option, the user-defined view factors from the zone surfaces to the duct surfaces are provided as input. Given that the view factor, duct surface emittance, the surface temperatures of the duct and communicating zone surface, the duct cross-sectional area, and the length are known, radiation heat transfer between the zone surface and the duct can be directly calculated using Eq. (1) through Eq. (6) above. This heat transfer can then be applied to the duct and to the zone surface as a heat source. Some scaling of the net longwave radiative loads may be necessary to maintain a heat balance in the interior radiant exchange.

Due to the complexity associated with accurately modifying the view factors as outlined in the first approach, the duct geometrical representation within EnergyPlus is not recommended. The second approach simplifies the problem and is representative of the detail required for an initial validation of the approach and its effects on duct performance.

#### Modifications to AirflowNetwork:Distribution:Component:Duct

In order to include radiation into the duct, minor modifications to the duct object will be required. If the duct view factor object is present, duct-surface radiation will occur automatically; if it is omitted, there will be no effect on duct performance.

## Input Output Reference

#### New Object AirflowNetwork:Distribution:DuctViewFactors

```
AirflowNetwork:Distribution:DuctViewFactors,
   Main Link 1,		!- Name of linkage
   0.5,			!- Surface exposure fraction
   0.9,			!- Duct surface emittance
   Lshaped Zone:Wall 1,	!- To Surface 1
   0.2,			!- View Factor 1
   Lshaped Zone:Wall 2,	!- To Surface 2
   0.05,		!- View Factor 2
   Lshaped Zone:Wall 3,	!- To Surface 3
   0.5,			!- View Factor 3
   Lshaped Zone:Wall 4,	!- To Surface 4
   0.25;		!- View Factor 4
```

##### **Inputs**

_Field: Name of linkage_
Name of the linkage in which the view factors are applied.

_Field: Surface exposure fraction_
The fraction, range 0 to 1, of the duct surface that is exposed to the zone for a partially buried duct (dimensionless).

_Field: Surface emittance_
The air duct surface emittance factor in the range 0 to 1 based on the material properties (dimensionless) .

_Field: To Surface 1_
This field specifies the name of the surface is seen by the duct. Field is extensible to accommodate as many surfaces as required.

_Field: View Factor 1_
This field specifies the view factor from the duct to surface 1. Field is extensible to accommodate as many surfaces as required.

## Engineering Reference

The duct-surface user-defined view factors object defines the view factor from the duct to the zone surfaces. Because the view factors, duct surface area, and temperatures are known, the heat gain to the duct can be directly calculated and applied to the duct air heat balance to determine the outlet temperature. The duct heat gain is then treated as a direct heat loss from each participating surface. Once the heat loss to the duct for each surface is calculated, the surface temperatures are adjusted to maintain the surface energy balance.

#### Theory on the radiative loads for air ducts

Radiation will be treated as point loads heat source/sink on the zone surfaces and the duct.  These point loads will be added or subtracted using the same implementation strategy as the convective load for the ducts to the zone air.  The radiative rate of energy transfer between the duct (d) and a given surface (s) for two grey surfaces can be written as Eq. (1) or Eq. (2).

$$\dot{Q}_{d\rightarrow s} = \frac{\sigma (T^4_d - T^4_s)}{\frac{1 - \epsilon_d}{f_e A_d \epsilon_d} + \frac{1}{f_e A_d F_{d\rightarrow s}}+\frac{1 - \epsilon_s}{A_s \epsilon_s}}$$	(1)

$$\dot{Q}_{s\rightarrow d} = \frac{\sigma (T^4_s - T^4_d)}{\frac{1 - \epsilon_d}{f_e A_d \epsilon_d} + \frac{1}{f_e A_d F_{s\rightarrow d}}+\frac{1 - \epsilon_s}{A_s \epsilon_s}}$$	(2)

Given that there is potentially a time step mismatch between the airflow network model and the envelope calculations.  The total radiative energy has to be integrated over the timestep mismatch.  Let’s assume that the system timestep is smaller or equal to the envelope time step and that there are a set of system time steps in a single envelope timestep Eq. (3).

$$\Delta t_e = \Delta T_s^{(0,1)} + \Delta t_s^{(1,2)} + \cdots + \Delta t_s^{(n-1, n)}$$	(3)

Within the envelope timestep there is a set of discrete time intervals for the system components to be calculated.

$$t_s = \{t_s^{(0)}, t_s^{(1)}, \cdots , t_s^{(n-1)}\}$$	(4)

Then the total radiative energy can be integrated between the current and next time instances of the envelope timestep, Eq. (5).

$$Q_{d\rightarrow s} = \int_t^{t+\Delta t_e} \dot{Q}_{d\rightarrow s}(t) dt$$	(5)

Since the system operates at discrete timesteps, the integral can be discretely approximated using the trapezoidal rule with non-uniform timesteps, Eq. (6) (NOTE:  If required, higher order approximations for the integral can be used.
).

$$Q_{d\rightarrow s} = \frac{1}{2} \sum_{k=0}^{N-1}(\Delta t_s^{(k, k+1)}[\dot{Q}_{d \rightarrow s}(t_s^{(k+1)}) + \dot{Q}_{d \rightarrow s}(t_s^{(k)})])$$	(6)

Over the envelope timestep, the temperature of the surface remains constant while the duct temperature is updated based on the impact of the convective and radiative loads.

## Code Design

AirflowNetworkBalanceManager::InitAirflowNetwork -- Modifications are made to read the duct surface view factors input object into memory.

AirflowNetworkBalanceManager::UpdateAirflowNetwork -- A new function is added to calculate duct-surface radiation.  This function calculates the heat sources/sinks based on the equations in the engineering reference. Once the duct-surface radiation has been calculated for each surface, the surface heat loss is then stored in the surface data to be applied when the next interior surface heat balance calculation occurs.

HeatBalanceSurfaceManager::CalcInteriorRadExchange -- Modifications are made to apply the heat gain/loss to the duct from each zone surface. The heat gain/loss is treated as a simple heat gain loss to the surface. Temperatures are scaled appropriately to maintain surface heat balance.

## Testing/Validation/Data Sources

For attic specific energy models there are 4 distinct numerical frameworks.

1. ASTM C1340 - The original consensus method for simulating attics containing radiant technologies
2. ASHRAE RP-717 - ASTM C1340 with air ducts and code is distributed in an unsearchable PDF
3. AtticSIM - A continuously developing in house framework by ORNL based on ASTM C1340
4. FATM - A continuously developing in house framework by Fraunhofer CSE based on ASTM C1340

Verification data is available through FATM, ASTM C1340, AtticSIM, and View3D.  The accuracy of the view factors that are calculated with the new algorithm can be tested with a more accurate framework View3D.  The thermal accuracy can be tested with the FATM, AtticSIM, and ASTM C1340 frameworks, although ASTM C130 does not have any air ducts. Experimental validation can be tested with the steady state summer and winter data available from experiments at ORNL.

## Transition

N/A

## References

[1]  D.G. Ober, K.E. Wiles, "An Energy Calculation Model for Attics, Including Radiant Barriers," ASHRAE Research Project RP 717, 1997.

[2]  ASTM Standard C1340, 2015, "Standard Practice for Estimation of Heat gain or Loss Through Ceilings Under Attics Containing Radiant Barriers by Use of a Computer Program," ASTM International, West Conshohocken, PA, 2015.

[3]  W.A. Miller, S. Shrestha, L. Gu, K. Childs, J. New, "A comparison of Simulation Capabilities for Ducts," ORNL/LTR-2014/283, (2014).

[4]  G.N. Walton, "View3D User manual," 2009.

[5]  G.N. Walton, "Calculation of Obstructed View Factors by Adaptive Integration," National Bureau of Standards NISTIR 6925, Gaithersburg MD (2002).

[6]  G.N. Walton, "Algorithms for Calculating Radiation View Factors Between Plane Convex Polygons with Obstructions", National Bureau of Standards NBSIR 86 3463, Gaithersburg MD (1986).

[7]  H.C. Hottel, A.F. Sarofim, Radiative Transfer, McGraw Hill, New York NY (1967).

[8]  A.D. Fontanini, J. Kosny, N. Shukla, A. Fallahi, B. Ganapathysubramanian, "Development and verification of the Fraunhofer attic thermal model, *Journal of Building Performance Simulation* (2016): 1-19."
