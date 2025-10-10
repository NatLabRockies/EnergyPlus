Design Document - More Daylighting Sensors/Integrate DElight
================

**Jason Glazer, GARD Analytics, Inc.**

 - June 13, 2016


## New Feature Proposal ##

See the file "More Daylighting Control Sensors with Comments-Rev05.docx" for details on the justification,  overview, and IDD changes.

## Overview ##

The current source files that are related to following objects:

- Daylighting:Controls
- Daylighting:DELight:Controls
- Daylighting:DELight:ReferencePoint
- Daylighting:DELight:ComplexFenestration
- Output:IlluminanceMap

are primarily:

- DataDaylighting.cc
- DaylightingManager.cc
- DElightManagerF.cc

with small references in:

- HeatBalanceManager.cc
- InternalHeatGains.cc
- CostEstimateManager.cc
- SurfaceGeometry.cc
- DataErrorTracking.cc

While the DElight inputs are handled in the DElightManagerF.cc file, the calculation routines are located in the "third_party\DElight\" directory and include 82 separate files that are .c .h .in and .txt files. No modification to these DElight calculation files is anticipated in this work.

The main data structures are defined in DataDaylighting.cc and are:

- ZoneDaylight using the structure ZoneDaylightCalc
- IllumMap using the structure IllumMapData
- IllumMapCalc using the structure IllumMapCalc

For this effort if time is available, the ZoneDaylight array will be the focus of refactoring to make it an object oriented class. The other two data structures will be not be touched.

## Approach for More Reference Points

In many places in the current code, calculations are performed for the first reference point and then repeated (within an if block) for the second reference point. These would be replaced with code that loops over all associated reference points. The ZoneDaylightCalc structure already includes subarrays that hold the inputs for each reference point such as the illuminance set point and fraction of the zone controlled. The sub arrays are allocated to MaxRefPoints which is set to 2. This will be changed to be the number of reference points associated with the Daylighting:Controls object.

## Approach for Combining Daylighting and DElight objects ##

The general approach will be to create a single ZoneDaylight structure that can handle the inputs for current versions of both the Daylighting:Controls and Daylighting:DELight:Controls and change the DElightInputGenerator to using that data structure instead of performing GetInput related calls. The DElightInputGenerator creates the input format needed for the DElight routine which will still be done. The GetDaylightingParametersDetaild which currently does the GetInput routines for Daylighting:Controls, Output:IlluminanceMap, OutputControl:IlluminanceMap:Style may be broken into separate functions to do the GetInput on each object separately.

## Object Oriented Approach

The first step will be to implement the code changes described above without performing any refactoring to make the code more object oriented. Unit tests will be created and the code will be tested in that configuration and committed to the branch. Only after we are certain that the code is working will we attempt to refactor portions of the daylighting code to be object oriented.

The general approach for refactoring plant components:

[https://github.com/NREL/EnergyPlus/wiki/RefactoringPlantComponentApproach](https://github.com/NREL/EnergyPlus/wiki/RefactoringPlantComponentApproach)

will be adapted for the portions of the daylighting code that is touched. No class hierarchy is expected at this point.

The ZoneDaylightCalc structure will be made into class and the main routines that modify the ZoneDaylight array will be the focus of making the code object oriented. That may include from the DaylightingManager.cc:

- DayltgAveInteriorReflectance()
- CalcDayltgCoeffsRefPoints()
- FigureRefPointDayltgFactorsToAddIllums()
- GetDaylightingParametersDetaild()
- DayltgInteriorIllum()
- DayltgSetupAdjZoneListsAndPointers()
- DayltgInterReflIllFrIntWins()

and from DElightManagerF.cc:

- DElightInputGenerator()

These functions frequently assign to the ZoneDaylight array. When refactored, we assume that these functions  will be made into methods for the new ZoneDaylightCalc class. It is possible that only portions of these functions that are directly related to the ZoneDaylightCalc class. Many other functions access the ZoneDaylight vector but at this point they are not planned on being refactored.

The DaylightingManager.cc file is over 10,000 lines of code that is not object oriented and originally developed in Fortran. An attempt to make it object oriented will be made but if the effort exceeds the time available, it may not be completed.

## Old Short Variable Names ##

The daylighting code includes many old variable names that are 4 to 6 characters long, all uppercase, and are difficult to read and understand. If time is available we will replace these variables names with ones that are easier to read and understand.



