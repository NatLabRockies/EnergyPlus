An EIR Formulated Water-to-Water Heat Pump Model
================================================

Edwin Lee, NREL

 - 2018-May-30
 - Revision Date


## Justification for New Feature ##

 - TRANE requested as high priority feature
 - Existing models for central plant water to water heat pump are not formulated to use industry standard curves, limiting the suitability of the model in representing equipment performance.

## E-mail and  Conference Call Conclusions ##

insert text

## Overview ##

Water-to-water heat pumps are used to connect two water loops using a refrigerant cycle to move heat.
EnergyPlus models the performance of these units using the `HeatPump:WaterToWater` objects.
There are two models in EnergyPlus:

 - HeatPump:WaterToWater:EquationFit:(Heating, Cooling)
 - HeatPump:WaterToWater:ParameterEstimation:(Heating, Cooling)

Users have noted that the parameter estimation model is not robust enough in practice, so a majority of users attempt to utilize the equation fit model.
The formulation of the equation fit model, however, does not match the formulation of curves that are available in industry.
The current model uses this type of formulation:

```
Q/Q_ref = a + b*(T_load/T_load_ref) + c*(T_source/T_source_ref) + d*(V_load/T_load_ref)  + e*(V_source/V_source_ref)
```

An EIR formulation, in the context of a water to water heat pump, would look like this:

```
CapacityMultiplier(temperature) = a + b*(T_load) + c*(T_load^2) + d*(T_source) + e*(T_source^2) + f*(T_load*T_source)
```

This formulation is more common to manufacturers and should provide an increase in the usability of the plant simulation.

## Approach ##

The high level approach for this task will be:

 - Create a proposal document, laying out justification and high level approach
 - Create a design document, laying out specific code patterns and approaches
 - Define the input structure in IDD
 - Create unit tests around a non-functioning implementation, showing test failures, and locking in the requirements of the model
 - Implement the changes in the code to get the tests to pass
 - Finalize all documentation changes
 - Make sure any other required changes are completed (ExpandObjects-ish)
 - Wrap up all testing, polishing, and submit for final review and merging

From a code standpoint, the approach will involve the following steps and more:

 - A test driven design, where unit tests are written prior to implementation
 - This will utilize the previously created plant object structure
 - This code will be as modular as possible, making is extremely easy to test and maintain
 - As I work on this, I may consider abstracting out the heat pump models so that any of the three performance representations can be dropped onto a single topology object
   - This is similar to what we are doing in the coil refactor
   - This would take a lot of work, and a lot of transition, so I'm not sure I'll do this...but I will make sure the new model is set up to be adapted to this in the future

## Testing/Validation/Data Sources ##

 - Unit tests will ensure that worker functions provide correct results under a variety of conditions
 - Integration tests will show proper operation in the context of a whole building model (actually plant only)
 - Regression tests are not valid here since this is a new implementation, but we can compare results against existing other models for comparative purposes

## Input Output Reference Documentation ##

To be completed...

## Input Description ##

To be completed...

## Outputs Description ##

To be completed...

## Engineering Reference ##

To be completed...

## Example File and Transition Changes ##

To be completed...

## References ##

To be completed...



