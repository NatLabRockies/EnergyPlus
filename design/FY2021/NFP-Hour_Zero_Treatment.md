Adding input option for Hour Zero weather data treatment
================

**J. Yuan & M.J. Witte, GARD Analytics**

 - Original Date - 2/8/21
 - Revision Date


## Justification for New Feature ##

This topic deals with how the first few sub-hourly weather data should be used. Currently in the code, Hr 1 and Hr 24 data was used as the base for the interpolation for the first hour time steps. However, intrinsically, there is no phsyical connection between Hr 1 and Hr 24 weather data, which can be observed in many other weather files (e.g. Chicago ORD airport 725300, a similar large Hr 1 and Hr 24 difference can be observed). To a user’s point view, this is might also a little bit difficult to interprete the first few time step output weather data , without knowing the reported values actually show the interpolated results with the data from Hour 1 and Hour 24.

There were also a little bit of history that this setting had changed back and forth a few time in the past. Here we proposed to add options to allow both these two options. They shoudl differ minimally for most simulations. However, it would allow the user to choose to use a "natural" way of using Hr 1 data for Hr 0 in the interpolation.

## E-mail and  Conference Call Conclusions ##



## Overview ##

The proposal seeks to add an input to allow for the choice of using Hr 1 or Hr 24 for the first sub-hour timesteps' interpolation of weather data.

## Approach ##

After considering a few places such as "Simulation Control", "TimeStep", and "Run Period", it is proposed to the additional input field at the end of "Run Period."

Original:

RunPeriod,
       \memo Specify a range of dates and other parameters for a simulation.
       \memo Multiple run periods may be input, but they may not overlap.
	...
  A8;  \field Treat Weather as Actual
       \type choice
       \key Yes
       \key No
       \default No

Proposed:

RunPeriod,
       \memo Specify a range of dates and other parameters for a simulation.
       \memo Multiple run periods may be input, but they may not overlap.
	...
  A8,  \field Treat Weather as Actual
       \type choice
       \key Yes
       \key No
       \default No
  A9, \field First Hour Interpolation Starting Values
       \note When the weather data timestep is longer than the simulation timestep, weather data is interpolated. For the first hour of the
       \note simulation, this field specifies which values from the first day of the run period to use as the interpolation starting point.
       \note This same interpolation will be used for repeated warmup days.
       \key Hour1
       \key Hour24
       \default Hour24

This approach will allow the user both ways of treating the first step. It would allow the users to select and try different methods based on their simulation needs.

## Testing/Validation/Data Sources ##

Test units will be developed to check the correct implementation of 1). the new input processing; and 2) the corresponding methods.

## Input Output Reference Documentation ##

An additional item describing the added field will be created to explain how the options can be used.

## Input Description ##

Input descriptions will be added for the new fields.

## Outputs Description ##

No change.

## Engineering Reference ##

TBD: Likely no change would be required.

## Example File and Transition Changes ##

New example file is not needed.

## References ##
