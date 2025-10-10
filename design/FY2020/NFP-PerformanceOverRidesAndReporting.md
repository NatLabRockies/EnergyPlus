Performance Overrides and Reporting
================

**Jason Glazer, GARD Analytics**

 - January 29, 2020
 - January 30, 2020 - Try "single knob" approach for input, include minimum number of warmup days,
add warning message for PerformancePrecisionTradeoffs
 - February 10, 2020 - Use single knob approach but also have separate fields for other
 convergence inputs


## Justification for New Feature ##

Provide some new controls to speed up the performance of EnergyPlus by overriding current defaults
as well as providing reporting to make monitoring performance tuning of specific simulations easier.

## E-mail and  Conference Call Conclusions ##

None

## Overview ##

Add a single field to the PerformancePrecisionTradeoffs object to override:

 - Zone Time Step
 - Zone Air Heat Balance Algorithm
 - Minimum Number of Warmup Days
 - Harwired and user set convergence parameters

Add reporting of the PerformancePrecisionTradeoffs object into the EIO file and Tabular Initialization
Summary. Also include in the EIO report other performance oriented options that exist such as the
recent sizing speed up work from TRANE [#7567](https://github.com/NREL/EnergyPlus/pull/7567) and
the CarrollMRT method [#7534](https://github.com/NREL/EnergyPlus/pull/7567).

When using the PerformancePrecisionTradeoffs, issue a warning message [#7646](https://github.com/NREL/EnergyPlus/issues/7646)

Add new output variables for:

- oscillating zone temperature during occupancy
- oscillating zone temperature while floating
- site energy consumption during oscillating

Add a new output file related to performance that is a log file and gets appended to after each
simulation of the same file. The file would be comma delimitted and would show the EIO performance
parameters described above as well as total energy by energy type, peak electrical demand, the oscillation
variables. It would also include the total number of objects and fields to indicate if the input file
changed between simulations. The intention is that the user could review this after successive simulations
when testing out the various performance parameters. The file would be generated whenever the
PerformancePrecisionTradeoffs object is present in the file.


## Approach ##

Override the default inputs and hard wired convergence parameters with other values.

## Testing/Validation/Data Sources ##

insert text

## Input Output Reference Documentation ##

The Override Mode field would be added to the Input Output Reference including the following table

|Mode   | Description |
|-----  |-------------|
|Normal | no overrides   |
|Mode01 | Zone Time step (TimeStep object) will be set to one timestep per hour|
|Mode02 | Mode01 plus ZoneAirHeatBalanceAlgorithm will be set to Euler |
|Mode03 | Mode02 plus Minimum Number of Warmup Days will be set to 1 |
|Mode04 | Mode03 plus Begin Environment Reset Mode will be set to SuppressAllBeginEnvironmentResets|
|Mode05 | Mode04 plus Loads Convergence Tolerance Value will be set to Y|
|Mode06 | Mode05 plus Temperature Convergence Tolerance Value will be set to Y|
|Mode07 | Mode06 plus internal variable MaxZoneTempDiff will be set to Y|
|Mode08 | Mode07 plus internal variable ConvrgLim will be set to Y|
|Mode09 | Mode08 plus internal variable MaxAllowedDelTempCondFD will be set to Y|
|Mode10 | Mode09 plus internal variable MassFlowTolerance will be set to Y|
|Advanced| Allow direct input of convergence field values|

NOTE: The exact layout of this table and the various Y values and list of internal variables will be determined
based on analysis not yet completed. There may be more or fewer modes in the final table.


## Input Description ##

The current PerformancePrecisionTradeoffs object is shown below:

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
  A2; \field Zone Radiant Exchange Algorithm
      \note Determines which algorithm will be used to solve long wave radiant exchange among
      \note surfaces within a zone.
      \type choice
      \key ScriptF
      \key CarrollMRT
      \default ScriptF
```

The revised version of the PerformancePrecisionTradeoffs as proposed is shown below:

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
      \note Determines which algorithm will be used to solve long wave radiant exchange among
      \note surfaces within a zone.
      \type choice
      \key ScriptF
      \key CarrollMRT
      \default ScriptF
  A3, \field Override Mode
      \note The increasing mode number roughly correspond with increased speed. A description of each mode
      \note are shown in the documentation. When Advanced is selected the N1 to N6 field values are used.
      \type choice
      \key Normal
      \key Mode01
      \key Mode02
      \key Mode03
      \key Mode04
      \key Mode05
      \key Mode06
      \key Mode07
      \key Mode08
      \key Mode09
      \key Mode10
      \key Advanced
      \default Normal
  N1, \field Loads Convergence Tolerance Value
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
  N2, \field Temperature Convergence Tolerance Value
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
  N3, \field MaxZoneTempDiff
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
  N4, \field ConvrgLim
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
  N5, \field MaxAllowedDelTempCondFD
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
  N6; \field MassFlowTolerance
      \note Only used when Override Mode is set to Advanced
      \type real
      \minimum X
      \maximum Y
      \default Z
```

NOTE: The exact number of convergence parameters (N1 to N6 shown above) may change due to on going testing of the
impacts of these parameters.


## Outputs Description ##

The new output variables would be based on the current oscillation variable shown below:

```
Output:Variable,*,Zone Oscillating Temperatures Time,hourly; !- HVAC Sum [hr]
Output:Variable,*,Facility Any Zone Oscillating Temperatures Time,hourly; !- HVAC Sum [hr]
```

The new output variables would add:

```
Output:Variable,*,Zone Oscillating Temperatures During Occupancy Time,hourly; !- HVAC Sum [hr]
Output:Variable,*,Facility Any Zone Oscillating Temperatures During Occupancy Time,hourly; !- HVAC Sum [hr]
Output:Variable,*,Zone Oscillating Temperatures While Floating Time,hourly; !- HVAC Sum [hr]
Output:Variable,*,Facility Any Zone Oscillating Temperatures While Floating Time,hourly; !- HVAC Sum [hr]
Output:Variable,*,Site Energy When Any Zone Oscillating Temperatures,hourly; !- HVAC Sum [J]
```

The EIO file and Tabular Initialization Summary would be modified to show the values of the PerformancePrecisionTradeoffs
object. It could also include other performance oriented options that exist such as the
recent sizing speed up work from TRANE [#7567](https://github.com/NREL/EnergyPlus/pull/7567) and
the CarrollMRT method [#7534](https://github.com/NREL/EnergyPlus/pull/7567).

A new output file with the extension .perflog would be created whenever the PerformancePrecisionTradeoffs
object is present. This log file would consist of the new EIO
output and some simple overall building energy and demand results as well as oscillation outputs. The perflog
file would be appended to instead of replaced each time to facilitate evaluation of the performance objects to
the user determine which combination is best for their particular input file. It would include a timestamp
and some summary information about the input file like the number of objects and fields. A hash of all inputs
besides the PerformancePrecisionTradeoffs would be good but it might be too time consuming to generate.


## Engineering Reference ##

None

## Example File and Transition Changes ##

Since the PerformancePrecisionTradeoffs object is being appended, no transition rules are anticipating. The new
EIO report and initialization report will impact the output files.

## References ##

None



