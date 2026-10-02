# EnergyPlus Output CSV Formatting (RVI/MVI) #

**Brett Bass, Mark Adams**

**Oak Ridge National Laboratory**

- Date: April 7, 2022

## Justification for new feature ##

---
EnergyPlus output CSV format does not currently allow for ordering of reporting or meter variables or specification of variable units.

## Overview ##

---

The EnergyPlus native CSV output is a useful new feature but it eliminates the need for “ReadVarsESO”. “ReadVarsESO” previously had the capability to use “.rvi” or “.mvi” file types. The “.rvi” file was used to order and filter the reporting variables while the “.mvi” was used to order and filter meter variables. A need for unit conversions in the native CSV output has also been raised. This was previously possible using the convertESOMTR executable.
These new features will address these deficiencies in the native CSV output by adding “rvi”, “mvi”, “CSV:Style” objects which can be used to:

- Filter and order the output reporting (rvi) and meter variables (mvi)
- Convert output units (CSV:Style)

## Approach ##

---

This new feature will be added through and execution of these subtasks:

1. Create three new EnergyPlus input objects (rvi, mvi, ouput units)
2. Properly read and integrate new EnergyPlus input objects
3. Add filtering and sorting mechanism based on rvi and mvi input object
4. Add ability to convert units between SI and imperial units to “CSVWriter::parseTSOutputs” based on output unit input object
5. Create unit tests to ensure that column order and unit conversions are correct
6. Document changes

## Testing/Validation/Data Sources ##

---
Additions will be made to the Input Output Reference for the mvi, rvi, and output unit objects.

## Input Description ##

---

```
OutputControl:RVI,
    \extensible:1
       \memo Determine order of CSV output reporting variables
    \unique-object
  A1,  \field Key Name 1
       \begin-extensible
  A2,  \field Key Name 2
  A3,  \field Key Name 3
  A4,  \field Key Name 4
  A5,  \field Key Name 5
  A6,  \field Key Name 6
  A7,  \field Key Name 7
  A8,  \field Key Name 8
  A9,  \field Key Name 9
  A10;  \field Key Name 10

OutputControl:MVI,
    \extensible:1
       \memo Determine order of CSV output meter variables
    \unique-object
  A1,  \field Key Name 1
       \begin-extensible
  A2,  \field Key Name 2
  A3,  \field Key Name 3
  A4,  \field Key Name 4
  A5,  \field Key Name 5
  A6,  \field Key Name 6
  A7,  \field Key Name 7
  A8,  \field Key Name 8
  A9,  \field Key Name 9
  A10;  \field Key Name 10

OutputControl:CSV:Style,
       \memo Unit conversions on reporting and meter variables
       \unique-object
   A1; \field Unit Conversion
       \type choice
       \key None
       \key JtoKWH
       \key JtoMJ
       \key JtoGJ
       \key InchPound
       \default None
```

## Outputs Description ##

---
The order of output reporting and meter variables in the CSV output and the units of the CSV output may change based on the rvi, mvi, and output unit input objects.

## Engineering Reference ##

---
There will be no change to the Engineering Reference as only column order and units will be changed with this new feature.

## Example File and Transition Changes ##

---
Suitable example file will be generated to demonstrate the ability to change output CSV column order and units

## References ##

---

NA
