Add Fuel Use to `OtherEquipment`
================================

**Noel Merket, NREL**

 - 14 June 2016

## Justification for New Feature ##

See [#5656](https://github.com/NREL/EnergyPlus/issues/5656). EnergyPlus currently has `ElectricEquipment` and `GasEquipment` objects that use electricity or natural gas and contribute to internal gains. There is also a `OtherEquipment` which uses no fuel and contributes to internal gains. Additionally, there is a `Exterior:FuelEquipment` that uses a fuel specified in a field (Electricity, NaturalGas, PropaneGas, FuelOil, etc) but does not contribute to internal gains.

For residential use there exists equipment that uses fuels other than electricity or natural gas and contributes to internal gains (i.e. a propane oven/range or a propane clothes dryer). Therefore there is a need for a generic object that allows the user to specify a fuel and contributes to internal gains.


## E-mail and  Conference Call Conclusions ##

We have not had any emails or conference calls, but there was a lot of discussion on the [GitHub issue](https://github.com/NREL/EnergyPlus/issues/5656). Several options were proposed. Including:

1. Create a `FuelEquipment` to replace (or deprecate) `GasEquipment` and `ElectricEquipment`.
2. Create a `FuelEquipment` that has all of the fuel types except electricity or natural gas.
3. Create separate `PropaneEquipment`, `FuelOilEquipment`, etc objects.
4. Equipment that replaces all interior/exterior, gas/electric/other. From [below](https://github.com/NREL/EnergyPlus/issues/5656#issuecomment-220482638)
5. Create a `FuelEquipment` to replace `GasEquipment`. From [below](https://github.com/NREL/EnergyPlus/issues/5656#issuecomment-219066932)
6. Add "Fuel Type" field to `OtherEquipment`. From [below](https://github.com/NREL/EnergyPlus/issues/5656#issuecomment-221892458)

| Object                 | Generates Heat in Zone | Exterior (no zone heat gain) | Generates CO2 | Consumes Electricity | Consumes Natural Gas | Uses District Heat | Consumes any Fuel Type | Consumes no Energy Resource | Net Number of Objects |
|------------------------|:----------------------:|:----------------------------:|:-------------:|:--------------------:|:--------------------:|:------------------:|:----------------------:|:---------------------------:|:---------------------:|
| ElectricEquipment      |            x           |                              |               |           x          |                      |                    |                        |                             |           NA          |
| GasEquipment           |            x           |                              |       x       |                      |           x          |                    |                        |                             |           NA          |
| HotWaterEquipment      |            x           |                              |               |                      |                      |          x         |                        |                             |           NA          |
| SteamEquipment         |            x           |                              |               |                      |                      |          x         |                        |                             |           NA          |
| OtherEquipment         |            x           |                              |               |                      |                      |                    |                        |              x              |           NA          |
| Exterior:FuelEquipment |                        |               x              |               |           x          |           x          |          x         |            x           |                             |           NA          |
| Option 1               |            x           |                              |       x       |           x          |           x          |          x         |            x           |                             |           -1          |
| Option 2               |            x           |                              |       x       |                      |                      |          x         |            x           |                             |           +1          |
| Option 3               |            x           |                              |       x       |                      |                      |                    |                        |                             |          +11          |
| Option 4               |            x           |               x              |       x       |           x          |           x          |          x         |            x           |              x              |           -5          |
| Option 5               |            x           |                              |       x       |                      |           x          |          x         |            x           |                             |           0           |
| Option 6               |            x           |                              |               |           x          |           x          |          x         |            x           |              x              |           0           |

After much discussion, the least intrusive was Option 6, which is adding a fuel type to `OtherEquipment`.

## Overview ##

In order to meet this need, `OtherEquipment` will be augmented with the following additional fields:

- Fuel Type
- End-use subcategory
- Carbon Dioxide Generation Rate

When Fuel Type is omitted, `OtherEquipment` will behave as it does now. When a Fuel Type is specified it will behave as the `ElectricEquipment` or `GasEquipment` objects do where energy will be used and the gains will go to the zone accordingly.

## Approach ##

The approach will be to add inputs to `OtherEquipment` to make it work more like `GasEquipment` but with a user specified fuel as described above. Then add the calculations into `OtherEquipment` to make it use fuel and possibly generate emissions when a fuel type is specified.

## Testing/Validation/Data Sources ##

Testing will include:

- Unit tests of specific functionality.
- Full simulation test of a simple building with an `OtherEquipment` with and without a fuel type to determine it behaves as expected.
- Regression tests comparing an `ElectricEquipment` and `GasEquipment` to equivalent `OtherEquipment` objects.

## Input Output Reference Documentation ##

### OtherEquipment

Other Equipment object is provided as an additional source for heat gains or losses directly to the zone.  That is to say, a loss can be entered by putting a negative value into the Design Level field(s). Note, too, that this object does not have an end-use component – gains or losses do not show up in the bottom energy lines (except as influencing overall zone gains or losses).

#### Inputs

##### Field: Name

The name of the OtherEquipment object.

##### Field: Fuel Use Type

This field designates the appropriate meter for the equipment. Valid fuel types are: Electricity, NaturalGas, PropaneGas, FuelOil\#1, FuelOil\#2, Diesel, Gasoline, Coal, Steam, DistrictHeating, DistrictCooling, OtherFuel1 and OtherFuel2. The fuel type triggers the application of consumption amounts to the appropriate energy meters.

##### Field: Zone or ZoneList Name

This field is the name of the thermal zone (ref: Zone) and attaches a particular other equipment statement to a thermal zone or set of thermal zones in the building. When the ZoneList option is used then this other equipment definition is applied to each of the zones in the zone list effecting a global definition for the amount of other in the zone. This option can be used effectively with the watts/area and watts/person options of the Design Level Calculation Method.

##### Field: Schedule Name

This field is the name of the schedule that modifies the design level parameter for other equipment (see Design Level Calculation Method field and related subsequent fields). The schedule values can be any positive number. The actual energy input for other equipment in a zone as defined by this statement is the product of the design level field and the value of the schedule specified by name in this field.

##### Field: Design Level Calculation Method

This field is a key/choice field that tells which of the next three fields are filled and is descriptive of the method for calculating the nominal other equipment level in the Zone. The key/choices are:

-   EquipmentLevel

With this choice, the method used will be a straight insertion of the other equipment level (Watts) for the Zone.  (The Design Level field should be filled.)

-   Watts/Area or Power/Area

With this choice, the method used will be a factor per floor area of the zone. (The Power per Zone Floor Area field should be filled).

-   Watts/Person or Power/Person

With this choice, the method used will be a factor of equipment level (watts) per person. (The Power per Person field should be filled).

##### Field: Design Level

This field (in Watts) is typically used to represent the maximum energy input to other equipment in a zone that is then multiplied by a schedule fraction (see previous field). In EnergyPlus, this is slightly more flexible in that the other equipment design level could be a “diversity factor” applied to a schedule of real numbers. This value can be negative to denote a loss. Note that while the schedule value can vary from hour to hour, the design level field is constant for all simulation environments.

##### Field: Power per Zone Floor Area

This factor (watts/m<sup>2</sup>) is used, along with the Zone Area to determine the maximum equipment level as described in the Design Level field. This value can be negative to denote a loss. The choice from the method field should be “**Watts/Area**” or “**Power/Area**”.

##### Field: Power per Person

This factor (watts/person) is used, along with the number of occupants (people) to determine the maximum equipment level as described in the Design Level field. This value can be negative to denote a loss. The choice from the method field should be “**Watts/Person**” or “**Power/Person**”.

##### Heat Gains/Losses from Other Equipment:

The fuel input to the equipment ultimately appears as heat that contributes to zone loads. In EnergyPlus this heat is divided into four different fractions. Three of these are given by the input fields Fraction Latent, Fraction Radiant and Fraction Lost. A fourth, defined as the fraction of the heat from other equipment convected to the zone air, is calculated by the program as:

$$f\_{\\rm{convected}} = 1.0 - (\\rm{Fraction Latent} + \\rm{Fraction Radiant} + \\rm{Fraction Lost})$$

You will get an error message if $\\rm{Fraction Latent} + \\rm{Fraction Radiant} + \\rm{Fraction Lost}$ exceeds 1.0.

##### Field: Fraction Latent

This field is a decimal number between 0.0 and 1.0 and is used to characterize the amount of latent heat given off by other equipment in a zone. The number specified in this field will be multiplied by the total energy consumed by other equipment to give the amount of latent energy produced by the other equipment. This energy affects the moisture balance within the zone.

##### Field: Fraction Radiant

This field is a decimal number between 0.0 and 1.0 and is used to characterize the amount of long-wave radiant heat being given off by other equipment in a zone. The number specified in this field will be multiplied by the total energy consumed by other equipment to give the amount of long wavelength radiation gain from other equipment in a zone.

##### Field: Fraction Lost

This field is a decimal number between 0.0 and 1.0 and is used to characterize the amount of “lost” heat being given off by other equipment in a zone. The number specified in this field will be multiplied by the total energy consumed by other equipment to give the amount of heat which is “lost” and does not impact the zone energy balances. This might correspond to input energy converted to mechanical work or heat that is vented to the atmosphere.

##### Field: Carbon Dioxide Generation Rate

This numeric input field specifies carbon dioxide generation rate with units of m3/s-W. The default value of 0.0 assumes the equipment is fully vented to outdoors. The maximum value for this input field is 3.45E-7 m3/s-W.

##### Field: End-Use Subcategory

Allows you to specify a user-defined end-use subcategory, e.g., “Cooking”, “Clothes Drying”, etc. A new meter for reporting is created for each unique subcategory  (ref: Output:Meter objects). Subcategories are also reported in the ABUPS table. If this field is omitted or blank, the equipment will be assigned to the “General” end-use subcategory. Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table. The following special tags will also place the end-use in specific rows in the LEED Summary table EAp2-4/5. Performance Rating Method Compliance: Fans-Parking Garage, Interior Lighting-Process, Cooking, Industrial Process, Elevators and Escalators.


## Input Description ##

Inputs would look much the same as `GasEquipment` but with a Fuel Type right after the Object name.

## Outputs Description ##

Outputs would generally mirror what's in `GasEquipment` as well.

## Engineering Reference ##

It will be updated to reflect the changes accordingly.

## Example File and Transition Changes ##

### Old Object

```
OtherEquipment,	BASE-1 OthEq 1, !- Name	BASE-1, !- Zone Name ALWAYSON, !- SCHEDULE Name	EquipmentLevel, !- Design Level calculation method 6766., !- Design Level {W}	, !- Power per Zone Floor Area {watts/m2}	, !- Power per Person {watts/person}	0, !- Fraction Latent 0.3, !- Fraction Radiant	0; !- Fraction Lost
```

### New Object

```
OtherEquipment,	BASE-1 OthEq 1, !- Name	PropaneGas,  !- Fuel Use Type	BASE-1, !- Zone Name
	ALWAYSON, !- SCHEDULE Name
	EquipmentLevel, !- Design Level calculation method
	6766., !- Design Level {W}	, !- Power per Zone Floor Area {watts/m2}	, !- Power per Person {watts/person}	0, !- Fraction Latent
	0.3, !- Fraction Radiant	0, !- Fraction Lost
	1.2E-7, !- Carbon Dioxide Generation Rate
	SubCategory1; !- End-Use Subcategory
```

### Transition

Transition will insert the Fuel Use Type field and leave it blank, which will cause `OtherEquipment` to behave as it currently does. Carbon Dioxide Generation Rate and End-Use Subcategory will be omitted from the end of the list as they will be optional.

## References ##

n/a
