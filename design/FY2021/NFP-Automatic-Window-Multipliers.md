# Automatic Window Multipliers

**Neal Kruis, Big Ladder**

## Overview

One time saving technique used in EnergyPlus models is the use of Window Multipliers to reduce redundant heat balance calculations on multiple window surfaces. However, this practice requires users to modify their building geometry to be substantially different from the actual architectural features they are trying to represent. This work will explore, propose, and implement an approach where these simplifications can be inferred automatically from user input and performed behind-the-scenes so that models with many windows can benefit from time-saving techniques without sacrificing geometric fidelity.

This work will focus largely on the one-dimensional heat balance calculations in `CalcWindowHeatBalanceInternalRoutines` that require solutions to non-linear systems of equations with potential off-diagonal matrix components. This work will not impact solar shading or daylighting calculations.

## Implementation

EnergyPlus will evaluate user input to group window surfaces with similar inputs. Each group will be assigned a representative window where the heat balance calculations will be performed and subsequently referenced by other window surfaces in the same group.

### Grouping

Fenestration surfaces will be grouped where they have identical:

- Construction
- Shading / storm window schedules
- Orientation
- Boundary conditions (e.g., same interior zone)
  - View factor to ground
  - Centroid height (with some tolerance to allow for the minor impacts on surface wind speed and surface temperature)
- Window height (with some tolerance to allow for minor impacts on interior convection coefficients)
- Frame and divider properties
  - Conductance
  - Projections
  - Edge-of-glass ratio
  - Solar absorptance
  - IR emissivity
  - Reveal surfaces

### Calculations

One-dimensional heat balance calculations will be performed for a representative window surface for each grouping of window surfaces. EnergyPlus will internally set data members of window surfaces in the same group to reference the values calculated by the representative window. When looping through windows for calculations, EnergyPlus will skip over the non-representative windows.

The one-dimensional calculation results will then be scaled by the respective areas of each surface to calculate zonal heat balances and output reporting variables.

Windows surfaces within a group will also be aggregated into a single surface area for participating in interior longwave radiation exchange.

***Note:*** Solar shading, interior solar distribution and daylighting will still be calculated on a window-by-window basis using the existing routines.
Absorbed solar radiation used in the one-dimensional window heat balance will be calculated based on an area weighted average of all solar fractions within the group. Optical properties for the current solar incidence angle will also be calculated once within a group, but the total transmitted solar for a given window will be calculated based in the solar incidence calculated by the shading routines.

### Limitations

In theory, this concept could apply to any group of surfaces with similar properties (including opaque surfaces), this work will be limited to a subset of fenestration surfaces. This implementation will not initially apply to:

- Equivalent Layer windows
- Complex Fenestration
- Thermochromics
- Tubular Daylight Devices
- Windows with airflow control

Some of these options will be further evaluated throughout the course of implementation.

## Testing

We will start this work by developing a scalable test suite that compares:

1. Aggregating windows into a single surface
2. Use of manual window multipliers
3. Explicit definition of separate windows without new feature
4. (after implementation) Explicit definition of separate windows with new feature

We will test a single zone with fixed total window areas on four facades, but separate cases with increasing numbers of windows.

The initial tests will give some idea of what level of performance gains to expect, and what the potential impact on results will be. (We expect the results will be similar to aggregating windows into a single surface.)

## IDD Changes and Transition

Add new "A4" field to `PerformancePrecisionTradeoffs`:

```
PerformancePrecisionTradeoffs,
      \unique-object
      \memo This object enables users to choose certain options that speed up EnergyPlus simulation,
      \memo but may lead to small decreases in accuracy of results.
  A1, \field Use Coil Direct Solutions
      \note If Yes, an analytical or empirical solution will be used to replace iterations in
      \note the coil performance calculations.
      \type choice
      \key Yes
      \key No
      \default No
  A2, \field Zone Radiant Exchange Algorithm
      \note Determines which algorithm will be used to solve long wave radiant exchange among surfaces within a zone.
      \type choice
      \key ScriptF
      \key CarrollMRT
      \default ScriptF
  A3, \field Override Mode
      \note The increasing mode number roughly correspond with increased speed. A description of each mode
      \note are shown in the documentation. When Advanced is selected the N1 field value is used.
      \type choice
      \key Normal
      \key Mode01
      \key Mode02
      \key Mode03
      \key Mode04
      \key Mode05
      \key Mode06
      \key Mode07
      \key Advanced
      \default Normal
  N1, \field MaxZoneTempDiff
      \note Maximum zone temperature change before HVAC timestep is shortened.
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum 0.1
      \maximum 3.0
      \default 0.3
  N2, \field MaxAllowedDelTemp
      \note Maximum surface temperature change before HVAC timestep is shortened.
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum 0.002
      \maximum 0.1
      \default 0.002
  A4; \field Use Automatic Window Multipliers
      \note Automatically group window surfaces with similar characteristics and perform relevant calculations only once for each group.
      \type choice
      \key Yes
      \key No
      \default No
```

## Documentation

Much of the content above will be modified to describe the feature in "Input Output Reference", and details of the aggregation calculations in "Engineering Reference".
