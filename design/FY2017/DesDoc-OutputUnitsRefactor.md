Design Document - Output Units Refactoring
================

**Jason Glazer, GARD Analytics, Inc.**

 - June 1, 2017
 - June 20, 2017 - Answers to questions at end.


## New Feature Proposal ##

No new feature proposal was produced since this will not impact input or output and is purely a refactoring effort.

## Overview ##

Refactor the SetUpOutputVariable call and all related functions, so that units are stored in a separate data field from the variable name, and passed as an integer or enum value instead of a string. The corresponding string in International System of Units (SI units), string in Imperial Units (IP units), and conversion shall all be predefined.

The unit type designator may be implemented to have multiple "flavors" for the same unit (for example, Watts), such as electricW, refrigerationW, etc. for cases where there is more than one IP unit for a given SI unit.


## Approach

Currently, SetupOutputVariable() has three functions with the following signatures:

```
void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable (with units)
	Real64 & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	std::string const & KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
	Optional_string_const EndUseKey = _, // Meter End Use Key (Lights, Heating, Cooling, etc)
	Optional_string_const EndUseSubKey = _, // Meter End Use Sub Key (General Lights, Task Lights, etc)
	Optional_string_const GroupKey = _, // Meter Super Group Key (Building, System, Plant)
	Optional_string_const ZoneKey = _, // Meter Zone Key (zone name)
	Optional_int_const ZoneMult = _, // Zone Multiplier, defaults to 1
	Optional_int_const ZoneListMult = _, // Zone List Multiplier, defaults to 1
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);

void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable
	int & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	std::string const & KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);

void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable
	Real64 & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	int const KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
	Optional_string_const EndUseKey = _, // Meter End Use Key (Lights, Heating, Cooling, etc)
	Optional_string_const EndUseSubKey = _, // Meter End Use Sub Key (General Lights, Task Lights, etc)
	Optional_string_const GroupKey = _, // Meter Super Group Key (Building, System, Plant)
	Optional_string_const ZoneKey = _, // Meter Zone Key (zone name)
	Optional_int_const ZoneMult = _, // Zone Multiplier, defaults to 1
	Optional_int_const ZoneListMult = _, // Zone List Multiplier, defaults to 1
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);
```

In each case the VariableName string contains the units being used in brackets such as:

- SetupOutputVariable( "Zone Total Internal Radiant Heating Energy [J]", ...
- SetupOutputVariable( "ITE Air Mass Flow Rate [kg/s]", ...
- SetupOutputVariable( "Contaminant Source or Sink CO2 Gain Volume Flow Rate [m3/s]", ...

These will be changed to

- SetupOutputVariable( "Zone Total Internal Radiant Heating Energy", unitJ, ...
- SetupOutputVariable( "ITE Air Mass Flow Rate", unitkg_s, ...
- SetupOutputVariable( "Contaminant Source or Sink CO2 Gain Volume Flow Rate", unitm3_s", ...

The revised function signatures will be:

```
void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable (with units)
	outputUnits const & VariableUnit, // enum of output units
	Real64 & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	std::string const & KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
	Optional_string_const EndUseKey = _, // Meter End Use Key (Lights, Heating, Cooling, etc)
	Optional_string_const EndUseSubKey = _, // Meter End Use Sub Key (General Lights, Task Lights, etc)
	Optional_string_const GroupKey = _, // Meter Super Group Key (Building, System, Plant)
	Optional_string_const ZoneKey = _, // Meter Zone Key (zone name)
	Optional_int_const ZoneMult = _, // Zone Multiplier, defaults to 1
	Optional_int_const ZoneListMult = _, // Zone List Multiplier, defaults to 1
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);

void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable
	outputUnits const & VariableUnit, // enum of output units
	int & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	std::string const & KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);

void
SetupOutputVariable(
	std::string const & VariableName, // String Name of variable
	outputUnits const & VariableUnit, // enum of output units
	Real64 & ActualVariable, // Actual Variable, used to set up pointer
	std::string const & IndexTypeKey, // Zone, HeatBalance=1, HVAC, System, Plant=2
	std::string const & VariableTypeKey, // State, Average=1, NonState, Sum=2
	int const KeyedValue, // Associated Key for this variable
	Optional_string_const ReportFreq = _, // Internal use -- causes reporting at this freqency
	Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
	Optional_string_const EndUseKey = _, // Meter End Use Key (Lights, Heating, Cooling, etc)
	Optional_string_const EndUseSubKey = _, // Meter End Use Sub Key (General Lights, Task Lights, etc)
	Optional_string_const GroupKey = _, // Meter Super Group Key (Building, System, Plant)
	Optional_string_const ZoneKey = _, // Meter Zone Key (zone name)
	Optional_int_const ZoneMult = _, // Zone Multiplier, defaults to 1
	Optional_int_const ZoneListMult = _, // Zone List Multiplier, defaults to 1
	Optional_int_const indexGroupKey = _ // Group identifier for SQL output
);
```

The exact location of the inserted VariableUnit is still to be detemined.

Overall in the EnergyPlus code, SetupOutputVariable is called over 3900 times so this conversion process will be performed using either search and replace using regular expressions in a text editor or using a custom Python script.

Other functions that will change are:

- GetVariableUnitsString()
- AddToOutputVariableList()
- AttachMeters()
- WriteReportVariableDictionaryItem()
- createSQLiteReportDictionaryRecord()
- WriteMeterDictionaryItem()
- GetMeteredVariables()
- ReportMeterDetails()
- GetVariableKeyCountandType()
- ProduceRDDMDD()

Which process or use the unit portion of the VariableName. These functions will need to be refactored also.

Several data structures also use a string that represents the units including:

- VariableTypeForDDOutput used in DDVariableTypes
- RealVariableType used in RVariableTypes
- IntegerVariableType used in IVariableTypes

These will change to have an enum instead of a string representing the units.

Besides OutputProcessor.cc and the calls to SetupOutputVariables, these other source files will be specifically reviewed and refactored due to the use of UnitString, often calling calling GetMeteredVariables() and GetVariableKeyCountandType():

- EconomicTariff.cc
- EMSManager.cc
- ExternalInterface.cc
- OutputReportData.cc
- OutputTabular.cc
- OutputTabularAnnual.cc
- SetPointManager.cc
- SimulationManager.cc
- SystemReports.cc

For the predefined reports defined in OutputReportPredefined.cc, similar strings are used like this to define outputs:

```
	pdchOaoZoneVol1 = newPreDefColumn( pdstOAavgOcc, "Zone Volume [m3]" );
```

the current function signature is:

```
	int
	newPreDefColumn(
		int const subTableIndex,
		std::string const & columnHeading
	);
```

Will be replaced with:

```
	int
	newPreDefColumn(
		int const subTableIndex,
		std::string const & columnHeading
		outputUnits const & VariableUnit, // enum of output units
	);
```


The list of different units used with SetupOutputVariable() is shown below:

- []
- [%]
- [A]
- [ach]
- [Ah]
- [Btu/h/W]
- [C]
- [cd/m2]
- [clo]
- [deg]
- [deltaC]
- [hr]
- [J/kg]
- [J/kg-K]
- [J/kgWater]
- [J/m2]
- [J]
- [K/m]
- [kg/kg]
- [kg/m2-s]
- [kg/m3]
- [kg/s]
- [kg]
- [kgWater/kgDryAir]
- [kgWater/s]
- [kmol/s]
- [L]
- [lum/W]
- [lux]
- [m/s]
- [m]
- [m2]
- [m3/s]
- [m3]
- [min]
- [Pa]
- [ppm]
- [rad]
- [rev/min]
- [s]
- [V]
- [W/K]
- [W/m2]
- [W/m2-C]
- [W/m2-K]
- [W/W]
- [W]

And the enums:

- [] =>  unitNone
- [%] =>  unitPerc
- [A] =>  unitA
- [ach] =>  unitach
- [Ah] =>  unitAh
- [Btu/h/W] =>  unitBtu_hW
- [C] =>  unitC
- [cd/m2] =>  unitcd_m2
- [clo] =>  unitclo
- [deg] =>  unitdeg
- [deltaC] =>  unitdeltaC
- [hr] =>  unithr
- [J/kg] =>  unitJ_kg
- [J/kg-K] =>  unitJ_kgK
- [J/kgWater] =>  unitJ_kgWater
- [J/m2] =>  unitJ_m2
- [J] =>  unitJ
- [K/m] =>  unitK_m
- [kg/kg] =>  unitkg_kg
- [kg/m2-s] =>  unitkg_m2-s
- [kg/m3] =>  unitkg_m3
- [kg/s] =>  unitkg_s
- [kg] =>  unitkg
- [kgWater/kgDryAir] =>  unitkgWater_kgDryAir
- [kgWater/s] =>  unitkgWater_s
- [kmol/s] =>  unitkmol_s
- [L] =>  unitL
- [lum/W] =>  unitlum_W
- [lux] =>  unitlux
- [m/s] =>  unitm_s
- [m] =>  unitm
- [m2] =>  unitm2
- [m3/s] =>  unitm3_s
- [m3] =>  unitm3
- [min] =>  unitmin
- [Pa] =>  unitPa
- [ppm] =>  unitppm
- [rad] =>  unitrad
- [rev/min] =>  unitrev_min
- [s] =>  units
- [V] =>  unitV
- [W/K] =>  unitW_K
- [W/m2] =>  unitW_m2
- [W/m2-C] =>  unitW_m2K (remove using C in denominator)
- [W/m2-K] =>  unitW_m2K
- [W/W] =>  unitW_W
- [W] =>  unitW

The exact format for the enum's is a question to be resolved.

We may need differentiate between "flavors" of units that would get converted to different units depending on what their IP unit conversion would be:

- normalW converts to IP as Btu/h
- electricW converts to IP as W
- refrigerationW converts to IP as Tons

In addition, the tabular reports already have a data structure to do unit conversions (see SetupUnitConversions() on line 14511 of OutputReportTabular.cc) that include 115 different unit conversions that are used with tabular reporting. The beginning of that list is shown below:

```
		UnitConv( 1 ).siName = "%";
		UnitConv( 2 ).siName = "°C";
		UnitConv( 3 ).siName = "0=OFF 1=ON";
		UnitConv( 4 ).siName = "0-NO  1-YES";
		UnitConv( 5 ).siName = "1-YES 0-NO";
		UnitConv( 6 ).siName = "A";
		UnitConv( 7 ).siName = "ACH";
		UnitConv( 8 ).siName = "ACH";
		UnitConv( 9 ).siName = "BASE 10C";
		UnitConv( 10 ).siName = "BASE 18C";
		UnitConv( 11 ).siName = "C";
		UnitConv( 12 ).siName = "CD/M2";
		UnitConv( 13 ).siName = "DEG";
		UnitConv( 14 ).siName = "FRAC";
		UnitConv( 15 ).siName = "HOUR";
		UnitConv( 16 ).siName = "HOURS";
		UnitConv( 17 ).siName = "HR";
		UnitConv( 18 ).siName = "HRS";
		UnitConv( 19 ).siName = "J";
		UnitConv( 20 ).siName = "J";
		UnitConv( 21 ).siName = "J";
		UnitConv( 22 ).siName = "J";
		UnitConv( 23 ).siName = "J";
		UnitConv( 24 ).siName = "J";
		UnitConv( 25 ).siName = "J/KG";
		UnitConv( 26 ).siName = "J/KG H2O";
		UnitConv( 27 ).siName = "J/M2";
		UnitConv( 28 ).siName = "K/M";
		UnitConv( 29 ).siName = "KG";
		UnitConv( 30 ).siName = "KG/KG";
		UnitConv( 31 ).siName = "KG/M3";
		UnitConv( 32 ).siName = "KG/S";

		UnitConv( 1 ).ipName = "%";
		UnitConv( 2 ).ipName = "F";
		UnitConv( 3 ).ipName = "0=Off 1=On";
		UnitConv( 4 ).ipName = "0-No  1-Yes";
		UnitConv( 5 ).ipName = "1-Yes 0-No";
		UnitConv( 6 ).ipName = "A";
		UnitConv( 7 ).ipName = "ACH";
		UnitConv( 8 ).ipName = "ach";
		UnitConv( 9 ).ipName = "base 50F";
		UnitConv( 10 ).ipName = "base 65F";
		UnitConv( 11 ).ipName = "F";
		UnitConv( 12 ).ipName = "cd/in2";
		UnitConv( 13 ).ipName = "deg";
		UnitConv( 14 ).ipName = "Frac";
		UnitConv( 15 ).ipName = "Hour";
		UnitConv( 16 ).ipName = "Hours";
		UnitConv( 17 ).ipName = "hr";
		UnitConv( 18 ).ipName = "hrs";
		UnitConv( 19 ).ipName = "kBtu";
		UnitConv( 20 ).ipName = "kWh";
		UnitConv( 21 ).ipName = "therm";
		UnitConv( 22 ).ipName = "MMBtu";
		UnitConv( 23 ).ipName = "Wh";
		UnitConv( 24 ).ipName = "ton-hrs";
		UnitConv( 25 ).ipName = "Btu/lb";
		UnitConv( 26 ).ipName = "Btu/lbWater";
		UnitConv( 27 ).ipName = "kBtu/sqft";
		UnitConv( 28 ).ipName = "F/ft";
		UnitConv( 29 ).ipName = "lb";
		UnitConv( 30 ).ipName = "lb/lb";
		UnitConv( 31 ).ipName = "lb/ft3";
		UnitConv( 32 ).ipName = "lb/s";
```

At minimum the enum values should correspond with the indices used in the UnitConv array so that they enum's can be used as indices in the UnitConv array.

An alternative enum list would be to use unit types rather than units.  From the IDD for curves

```
  A2,  \field Input Unit Type for X
       \type choice
       \key Dimensionless
       \key Temperature
       \key VolumetricFlow
       \key MassFlow
       \key Power
       \key Distance
       \default Dimensionless
  A3;  \field Output Unit Type
       \type choice
       \key Dimensionless
       \key Capacity
       \key Power
```

This list would have to be grealy expanded but would work towards consistency between input and output.

For the sake of completeness, the IDF Editor also has a long list of 155 unit conversions that are used to allow users to use IP units in IDF Editor and for the IDF produced be in SI. While it is separate code, perhaps the units themselves should be expressed consistently.


##Implementation Process

Given the large number of source code changes across so many files, the refactoring process will take three steps:

- Make all SetupOutputVariable() calls consistent on a single line and with spaces before the bracket and make sure no changes are found with the units tests and regression tests.
- Change the calls to SetupOutputVariable() but make the changes within that function as minimal as possible to see if any changes are with the unit tests and regression tests.
- After no changes are detected, the SetupOutputVariable() functions and other functions will be refactored.

The goal is to make small changes that will not show any diffs throughout the refactoring process.


## Questions

Should enum or enum class, a C+11 feature, be used?

- The enum class should be used, better scoping.

Where should the VariableUnit be inserted in the SetupOutputVariable calls?

- The VariableUnit will be the second argument, right after the VariableName.

Should each enum start with "unit", for example kg_m3 or unitkg_m3?

- Since the enum class will include the class name such as "units" it is enough to have the unit names as kg_m3. When used normally they would be units::kg_m3.

Is using "_" to separate the numerator and denominator reasonable or should we use "per"?

- The "_" will be used to separate the numerator and the denominator in the units enum class. If after implmentation this decision is revisted, it can be easily changed to "per" using search and replace.

Should units be expressed in unit types instead of units (Temperature, VolumetricFlow, MassFlow, Power, etc.)?

- No, this would not be feasible for the wide variety of units used and might get more confusing. This could also be revisited in the future.

Should units used in tabular reports be included in this change so there is a single set of units for all outputs?

- Yes.

Should the individual enum values correspond to the UnitConv array used in the tabular reports?

- Yes.

Is the three step implementation process reasonable?

- Yes, proceeding in small steps and avoiding diffs is a good idea.

Should we provide an option to provide IP converted values directly from EnergyPlus to the  ESO and MTR (and thus the normal CSV files)?

- We probably won't have enough time to take this on and we did it would need some discussion and an NFP.
