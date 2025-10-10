

Change EquationFit coils to reference curve objects (Issue #8352)
================

**Yueyue Zhou, NREL**

 - 11 Jan 2021


## Background ##

Excerpts lifted from [issue #8352](https://github.com/NREL/EnergyPlus/issues/8352)

 - The EquationFit coil objects (like `Coil:Cooling:WaterToAirHeatPump:EquationFit` and `Coil:Heating:WaterToAirHeatPump:EquationFit`) have curve coefficients embedded in them.

 - But other coil objects refer to standalone curve objects instead.

 - Embedded coefficients limit the flexibility of using EMS to model coil performance impact when needed.

 - Changing EquationFit coils to reference curve objects also allows the user to use different equation forms and to use lookup tables. (From [Mike Witte](https://github.com/NREL/EnergyPlus/issues/8352#issuecomment-724140062))

## E-mail and  Conference Call Conclusions ##

*None yet.*

## Coil Objects Need to change##

- Coil:Cooling:WaterToAirHeatPump:EquationFit

- Coil:Heating:WaterToAirHeatPump:EquationFit

- HeatPump:WaterToWater:EquationFit:Cooling

- HeatPump:WaterToWater:EquationFit:Heating

## Curve Objects Needed ##
- For `Coil:Cooling:WaterToAirHeatPump:EquationFit`, there're **quad-linear** curves and **quint-linear** curves used to describe coil performances. However, currently only **quad-linear** curves are existing.

## Decision to Make ##

### Add quint-linear curve object? ###
As described above, both quad-linear and quint-linear curves are needed for  Coil:Cooling:WaterToAirHeatPump:EquationFit, while other coils only require quad-linear curves. In order to satisfy Coil:Cooling:WaterToAirHeatPump:EquationFit curve specifications, we can either:
 - Add quint-linear curves so that the curve type of every curve referred can be explicitly specified. This is a more straightforward path to move forward to.
 - Change quad-linear curves to be more general as "multi-linear" or simply replace it by quint-linear curves, regarding to the similarity between quad-linear and quint linear curves. Users can simply add 0 coefficients to whatever variables that are not important or E+ can only read specific number of coefficients for specific input. Notice that there's no tri-linear or bi-linear curve types in Energyplus which might imply some preference to this approach in its first place (Is that true? Any example of using quad-linear curve type to read in tri-linear curves?).


## Approach ##

Once the above decision are made:

 - A new curve type will be added or current curve type will be modified in Curve and other curve related codes/data structures. The curve will be very similar to current quad-linear curve, eg.
 ```
CURVE:QUINTLINEAR,
  SensCapCurve, ! Curve Name
  4.27615968,           ! CoefficientC1
  13.90195633,          ! CoefficientC2
  -17.28090511,         ! CoefficientC3
  -0.70050924,          ! CoefficientC4
  0.51366014,           ! CoefficientC5
  0.017194205,          ! CoefficientC6
  -30.,              ! Minimum Value of v
  80.,               ! Maximum Value of v
  -30.,              ! Minimum Value of w
  60.,               ! Maximum Value of w
  0.,                ! Minimum Value of x
  80.,               ! Maximum Value of x
  0.,                ! Minimum Value of y
  5.,                ! Maximum Value of y
  0,                 ! Minimum Value of z
  1,                 ! Maximum Value of z
  0.,                ! Minimum Curve Output
  5.;                ! Maximum Curve Output
```

 - EquationFit coil/heatpump models will be changed to refer to curves instead of reading in embedded coefficients.

```
 Coil:Cooling:WaterToAirHeatPump:EquationFit,
  ground source heat pump clg coil,       !- Name
  Node 11,                                !- Water Inlet Node Name
  Node 12,                                !- Water Outlet Node Name
  ground source heat pump unitary system Fan - Cooling Coil Node, !- Air Inlet Node Name
  ground source heat pump unitary system Cooling Coil - Heating Coil Node, !- Air Outlet Node Name
  0.695196533092189,                      !- Rated Air Flow Rate {m3/s}
  0.00075708235679397,                    !- Rated Water Flow Rate {m3/s}
  14067.4113682667,                       !- Gross Rated Total Cooling Capacity {W}
  10269.2102988347,                       !- Gross Rated Sensible Cooling Capacity {W}
  5.3555091458258,                        !- Gross Rated Cooling COP {W/W}
  TotCapCurve,                            !- Total Cooling Capacity Curve Name
  SensCapCurve,                           !- Sensible Cooling Capacity Curve Name
  PowConsCurve,                           !- Cooling Power Consumption Curve Name
  1000,                                   !- Nominal Time for Condensate Removal to Begin {s}
  1.5;                                    !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}
  ```

## Testing/Validation/Data Sources ##

There's potentially going to be many string diffs, but numbers should all remain the same.

## Input Output Reference Documentation ##

There's potentially many doc changes required.

## Example File and Transition Changes ##

And transition will be required.

 - For all of the test files that use above coil/heatpump objects.
