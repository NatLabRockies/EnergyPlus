# NFP: Improve Year-Awareness of EnergyPlus #

## Justification for New Feature ##

Users and interface developers are requesting that EnergyPlus handle dates in a more standard way to facilitate actual-weather simulations (both single year and multiple years) and the post-processing of the simulation results. Many of the packages available for results processing use “real” dates that require valid, internally consistent data (e.g. the year 2016 is a leap year and must include February 29). EnergyPlus already properly simulates most situations if correct input is supplied, but does not always produce readily processed output and there are many ways to produce simulations that cannot be represented with valid dates and times.

## Overview ##

EnergyPlus will be modified to always have a real year associated with the simulation and output appropriate dates and times. Simulations that would violate date and time rules (e.g. simulate 2016 without February 29) will no longer be allowed unless requested. Information from the weather file information will no longer take precedence over objects in the input simulation file. Some weather file inputs (e.g. the leap year flag) will always be ignored.

The **RunPeriod** and **RunPeriod:CustomRange** objects will be combined into a single **RunPeriod** object that specifies start and end dates as valid calendar dates. Schedule:File will be extended to allow multiple years in a single schedule file. The simulation year will be added SQLite output.

## Approach ##

EnergyPlus already handles dates correctly when asked to by the user, this work will eliminate the paths that allow users to ask EnergyPlus to handle dates incorrectly. The steps to this result are given below.

1. Annual Outputs: (not in original NFP) Add an annual output frequency to output at the end of each calendar year.

2. RunPeriod Consolidation: **RunPeriod** and **RunPeriod:CustomRange** will be combined into a single object that fully specifies dates. The table below gives the details on what will be added or removed from RunPeriod. See Appendix A for the new RunPeriod IDD.

3. Time Consolidation: Date and time tracking and updating is now currently spread across at least three namespaces (DataEnvironment, WeatherManager, and SimulationManager). While this does present some maintenance issues, it will also be an impediment to OO conversion. Where possible, time tracking should be consolidated.

4. Schedule Additions (Delayed): Currently, multiyear simulations repeat the same single year schedule, which is unlikely to a problem for many multiyear simulations. However, there are situations where schedules might change from year to year (e.g. school occupancy schedules) and some calibration studies might require more flexibility (e.g. efficiency measures applied partway through a calibration data set). In these situations, it is necessary to provide some way for users to have multiyear schedules. Currently, EnergyPlus translates all schedules into a single "Year Schedule" container. This makes it difficult to directly support multiyear schedules. For true multiyear simulations, this will need to be addressed.

  |     | RunPeriod Field                            | Notes                  |
  | :-: | :----------------------------------------- | ---------------------- |
  | A1  | Name                                       | Required               |
  | N1  | Begin Month                                | Required               |
  | N2  | Begin Day of Month                         | Required               |
  | New | Begin Year                                 | Computed if not input  |
  | A2  | Day of Week for Start Day                  | Computed if not input  |
  | N3  | End Month                                  | Required               |
  | N4  | End Day of Month                           | Required               |
  | New | End Year                                   | Computed if not input  |
  | A3  | Use Weather File Holidays and Special Days |                        |
  | A4  | Use Weather File Daylight Saving Period    |                        |
  | A5  | Apply Weekend Holiday Rule                 |                        |
  | A6  | Use Weather File Rain Indicators           |                        |
  | A7  | Use Weather File Snow Indicators           |                        |
  | N5  | Number of Times Runperiod to be Repeated   | *Remove*               |
  | A8  | Increment Day of Week on repeat            | *Remove*               |
  | N6  | Start Year                                 | *Remove* (redundant)   |
  | New | Treat Weather as Actual                    | Be strict about dates  |

See Appendix C for details of the "Computed if not input" fields.

## Testing/Validation/Data Sources ##
Existing EnergyPlus test files will be modified to produce several models that are multiyear simulations.

## Input Output Reference Documentation ##
The documentation of the **RunPeriod** object will be modified to reflect the new behavior. Schedule documentation will be will be developed if needed.

## Engineering Reference ##
The Engineering Reference will be checked to make sure that it documents the new behavior.

## Example File and Transition Changes ##
To objects will need to be transitioned **RunPeriod** and **RunPeriod:CustomRange**. All example files will be transitioned to the new **RunPeriod** object. There are no **RunPeriod:CustomRange** tests.

### RunPeriod Transition ###
Not all old **RunPeriod** objects can be perfectly transitioned to the new **RunPeriod**. The relevant scenarios are given below:

#### Minimal RunPeriod (mm/dd to mm/dd)
Set the begin and end dates from the input, leave the rest as default.

#### Start Year Specified (mm/dd/yyyy to mm/dd) ####
Set the begin and end dates from the input, set "Begin Year" to input start year, then leave the rest as default.

#### Start Year and Weekday Specified (W, mm/dd/yyyy to mm/dd) ####
Set the begin and end dates from the input, set "Begin Year" to input start year, then leave the rest as default. Compute the correct starting day for the input begin date and year. Set this as the value for "Day of Week for Start Day". If the input and computed weekday do not match, issue a warning.

#### Minimal RunPeriod with N>1 Repeats ((mm/dd to mm/dd) x N)
Set the begin and end dates from the input, leave the rest as default. Set the "Begin Year" to 2017 (since 2017 starts on a Sunday) and compute the "End Year" by adding N to 2017.

#### Start Year Specified with N>1 Repeats ((mm/dd/yyyy to mm/dd) x N) ####
Set the begin and end dates from the input, set "Begin Year" to input start year, then leave the rest as default. Compute the "End Year" by adding N to the "Begin Year".

#### Start Year and Weekday Specified with N>1 Repeats ((W, mm/dd/yyyy to mm/dd) x N) ####
Set the begin and end dates from the input, set "Begin Year" to input start year, then leave the rest as default. Compute the correct starting day for the input begin date and year. Set this as the value for "Day of Week for Start Day". If the input and computed weekday do not match, issue a warning. Compute the "End Year" by adding N to the "Begin Year".

#### Any RunPeriod with "Increment Day of Week on repeat" Enabled ####
This option will no longer be supported. Warn and complete transition as above.

## Appendix A: New RunPeriod IDD Object ##

```
RunPeriod,
       \memo Specify a range of dates and other parameters for a simulation.
       \memo Multiple run periods may be input, but they may not overlap.
       \min-fields 7
  A1 , \field Name
       \reference RunPeriodsAndDesignDays
       \note descriptive name (used in reporting mainly)
       \note if blank, weather file title is used.  if not blank, must be unique
  N1 , \field Begin Month
       \required-field
       \minimum 1
       \maximum 12
       \type integer
  N2 , \field Begin Day of Month
       \required-field
       \minimum 1
       \maximum 31
       \type integer
  N3,  \field Begin Year
       \note start year of the simulation, if this field is specified it must agree with the Day of Week for Start Day
  N4 , \field End Month
       \required-field
       \minimum 1
       \maximum 12
       \type integer
  N5 , \field End Day of Month
       \required-field
       \minimum 1
       \maximum 31
       \type integer
  N6,  \field End Year
       \note end year of simulation, if specified
  A2 , \field Day of Week for Start Day
       \note =[Sunday|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday];
       \default Sunday
       \type choice
       \key Sunday
       \key Monday
       \key Tuesday
       \key Wednesday
       \key Thursday
       \key Friday
       \key Saturday
  A3,  \field Use Weather File Holidays and Special Days
       \note If yes or blank, use holidays as specified on Weatherfile.
       \note If no, do not use the holidays specified on the Weatherfile.
       \note Note: You can still specify holidays/special days using the RunPeriodControl:SpecialDays object(s).
       \type choice
       \default Yes
       \key Yes
       \key No
  A4,  \field Use Weather File Daylight Saving Period
       \note If yes or blank, use daylight saving period as specified on Weatherfile.
       \note If no, do not use the daylight saving period as specified on the Weatherfile.
       \type choice
       \default Yes
       \key Yes
       \key No
  A5,  \field Apply Weekend Holiday Rule
       \note if yes and single day holiday falls on weekend, "holiday" occurs on following Monday
       \type choice
       \key Yes
       \key No
       \default No
  A6,  \field Use Weather File Rain Indicators
       \type choice
       \key Yes
       \key No
       \default Yes
  A7,  \field Use Weather File Snow Indicators
       \type choice
       \key Yes
       \key No
       \default Yes
  A8;  \field Treat Weather as Actual
       \type choice
       \key Yes
       \key No
       \default No
```

All date and time tracking, which is presently split among a number of the managers, will be consolidated in SimulationManager. A Schedule:MultiYear object will be added in order to allow users to schedule years separately.

## Appendix B: Incomplete Survey of Current Behavior ##
There are presently two paths to multiyear simulations, one using repeated run periods and the second using a custom range object:

1. Repeated years: Use RunPeriod, set ‘Number of Times Runperiod to be Repeated’ to 2 or more.
2. Multiple years: Use RunPeriod:CustomRange. This requires that the the EPW DataPeriods contain the date range given in the RunPeriod:CustomRange object and that the beginning date and first hour exist in the EPW. The minimal steps to duplicate a 2-year simulation run through path #1 are:
  * Create a RunPeriod:CustomRange object with the appropriate dates and delete the RunPeriod object
  * Duplicate the weather by copy-pasting all 8760 entries of the EPW file at the end of the EPW file
  * Edit the EPW data periods to include the start year and end years
  * Edit the first data line of the EPW to have the start year (all of the other years get ignored)

Another important case to consider are multiyear simulations with leap years. Following the two paths:

1. Repeated years: Add a specific starting year to the RunPeriod object and add February 29 to the EPW. For a simulation of two years (one non-leap year followed by a leap year), E+ warns about a skipped February 29 data for the first year. If the EPW file does not have Febuary 29, that date is silently skipped.
2. Multiple years: Set the year interval, EnergyPlus works as expected.

## Appendix C: Development Team Comments ##

**Jason Glazer, Re: February 29:** Assuming that the TMY3 or IWEC or what ever does not have a February 29 day of data, what are you going to do for February 29? If it is repeating the February 28 data that could be introduce errors since no smoothing would be done from February 28 midnight to February 29 1AM.

*Response:* The ability to skip the leap day is retained in the current implementation, there are too many EPWs in the wild that are missing that day.

**Jason Glazer, Re: Outputs:** The year of the simulation should also appear in the tabular output file in the input verification and output summary table and perhaps other outputs.

*Response:* The current implementation includes annual outputs in the ESO.

**Dan Macumber, Re: Begin/End Year:** What happens if End Year is specified but Begin Year is not or End Year < Begin Year, etc.

*Response:* These would be invalid and are severe errors.

**Lixing Gu, Re: Removal of repetition field:** When this field is removed, how do you keep the same existing capability to allow multiyear simulations using the same single year weather?

*Response:* The number of years in the old repetition field will need to be reflected in the difference between the start year and the end year in the input object, and the "Treat as Actual Weather" field should be set to "No" (which is the default) to get nearly the current behavior. The one difference will be that the weekdays are incremented and not reset; leap years (or the lack thereof) are handled the same way as they are currently in the repeated run period scenario.

**Dan Macumber, Re: Output Changes:** Could this be a new field in an output request object? The default could be to not include years in output for one release, then the default could change to include years, then the option could be deprecated.

**Jason Glazer, Re: Output Changes:** I think that it is important that timestamped output be an option for the normal ESO/MTR output files even if not supported by ReadVarsESO immediately. Although it would be good if ReadVarsESO was modified to support this new format.

*Response:* An additional annual output frequency was added to provide output at a yearly frequency. As part of that work, a year field was added to the SQL output and a yearly timestamp was added to the ESO/MTR.

## Appendix D: Design and Implementation ##

Very little is changed structurally in the code. The crucial point of implementation is the determination of a start year. The general approach is to use the start month and day with other inputs to get the year.

* If only the begin month and day are specified, then the default weekday (Sunday) is used to determine a year.
* If the begin year is specified and the weekday is not, the corresponding weekday is looked up.
* If the weekday is specified, then then the corresponding year is determined.
* If both begin year and weekday are specified, the inputs are verified.
