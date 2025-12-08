# AirflowNetwork Specified Flow Element #

Jason DeGraw, Prateek Shrestha

Oak Ridge National Laboratory

Original Date: January 31, 2021

## Justification for New Feature ##
AirflowNetwork does not currently have a constant flow element linkage, which limits the degree to which
certain elements (including supply/exhaust fans and infiltration) can be connected to AirflowNetwork.

## Overview ##
AirflowNetwork has a number of ways to represent unintentional flows between zones, but no good way to
represent flows of known magnitude. This limitation means that AirflowNetwork

  * is not compatible with the various available interzonal flow objects,
  * is not compatible with the various available infiltration objects, and
  * cannot represent very simple fans very well.

This new feature will begin to address these deficiencies. The Jacobian elements needed by
AirflowNetwork’s solver is computed by the flow element “calculate” function:

```
NF = element.calculate(..., F, DF);
```

The constant flow with specified flow will return 1, and the Jacobian values are computed as follows:

  * F is computed from the specified flow
  * DF is zero

## Approach ##
This feature will be added through the execution of these subtasks:

  * Create two new airflow elements SpecifiedVolumeFlow and SpecifiedMassFlow
  * Create a new input object AirflowNetwork:MultiZone:SpecifiedFlowRate
  * Create necessary inputs to allow users to incorporate this element into models
  * Create unit tests and documentation

Adding two elements will avoid the need for explicit logic to differentiate the specified mass flow and specified volume flow objects.
Resolution of which `calculate` to use will still take place at runtime, so this choice is more about maintainabilty than it is about
performance. Lots of logic in the calculation function makes them much harder to maintain, this will make the `calculuate` functions
somewhat cleaner and easy to understand.

## Testing/Validation/Data Sources ##
Unit test will be added to ensure numerical values are calculated properly.

## Input Output Reference Documentation ##
Additions will be made to the Input Output Reference for the SpecifiedFlowRate airflow element.

## Input Description ##
```
AirflowNetwork:MultiZone:SpecifiedFlowRate,
      \min-fields 2
      \memo This object specifies the flow through a link.
 A1 , \field Name
      \required-field
      \type alpha
      \reference SurfaceAirflowLeakageNames
      \note Enter a unique name for this object.
 N1 , \field Air Flow Value
      \type real
      \required-field
      \note Enter the air flow value for this element.
 A2 ; \field Air Flow Units
      \type choice
      \key kg/s
      \key scfm
      \key slpm
      \default kg/s
      \note Enter the air flow units for this element.
```

## Outputs Description ##
No new output variables will be implemented with this work as existing variables are sufficient to gather relevant airflow information.

## Engineering Reference ##
There will be no change to the Engineering Reference as the constant airflow rate will be user-defined in this case.

## Example File and Transition Changes ##
Suitable example file will be generated. Since the SpecifiedFlowRate object is being added, no transition rules are anticipated.

## References ##
NA
