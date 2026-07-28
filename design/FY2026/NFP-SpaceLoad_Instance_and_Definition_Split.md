SpaceLoad Instance and Definition Objects
================

**Kyle Benne, National Laboratory of the Rockies**

 - Original Date: 04/29/2026
 - Revision Date: 04/29/2026


## Justification for New Feature ##

OpenStudio has long used an Instance/Definition split for internal gains objects (e.g., `OS:ElectricEquipment` / `OS:ElectricEquipment:Definition`). EnergyPlus, however, kept all fields — physical characteristics and zone/space assignments alike — in a single object. This divergence means that every time a field is added or changed in OpenStudio's model it must be reconciled with a differently-shaped EnergyPlus object, and the translation layer between the two tools must carry the extra complexity.

By introducing the same Instance/Definition pattern in EnergyPlus, we achieve three concrete benefits:

1. **Alignment of EnergyPlus and OpenStudio** — the EnergyPlus IDD and the OpenStudio Model API now share the same conceptual shape, reducing the translation surface and the associated maintenance burden. Allowing usage of OpenStudio directly on an IDF will provide the ability to work on **any** IDF, including fully unsupported objects (objects not wrapped in OpenStudio model/ namespace) and objects which don't have ReverseTranslator rules (the ReverseTranslator IDF -> OSM is lacking in terms of HVAC especially).
2. **DRY input data** — a single `ElectricEquipment:Definition` describing, say, a standard office workstation can be referenced by any number of `ElectricEquipment:Instance` instances across the building, each with its own zone/space assignment and schedule, without repeating the physical characteristics.
3. **Cleaner IDD semantics** — the split makes explicit which properties are intrinsic to the load type (the definition) and which are extrinsic, installation-specific concerns (the instance).


## E-mail and Conference Call Conclusions ##

- 2026-06-10: As discussed at the 6/10 Technicalities meeting, the team favored keeping the existing objects as-is, and make this NFP backwards compatible.
    - The existing space loads shall remain the same and the NFP will add new instance and definition objects.
    - For example, People will remain exactly as is. We will add `People:Instance` and `People:Definition`. There will be no transition rules.


## Overview ##

Every existing internal gains object type is mapped to new objects split into two complementary objects:

| Instance                                    | Definition                                    |
|---------------------------------------------|-----------------------------------------------|
| `People:Instance`                           | `People:Definition`                           |
| `Lights:Instance`                           | `Lights:Definition`                           |
| `ElectricEquipment:Instance`                | `ElectricEquipment:Definition`                |
| `GasEquipment:Instance`                     | `GasEquipment:Definition`                     |
| `HotWaterEquipment:Instance`                | `HotWaterEquipment:Definition`                |
| `SteamEquipment:Instance`                   | `SteamEquipment:Definition`                   |
| `OtherEquipment:Instance`                   | `OtherEquipment:Definition`                   |
| `ElectricEquipment:ITE:AirCooled:Instance`  | `ElectricEquipment:ITE:AirCooled:Definition`  |

The **Definition** object holds all physical characteristics that are intrinsic to the load type: the design level calculation method and its associated values (equipment level, watts per floor area, watts per person, etc.), heat gain fractions (radiant, latent, lost, return air, ...), and any other type-specific properties such as CO₂ generation rate, fuel type, thermal comfort model selections, and ITE-specific parameters. Because a definition carries no zone or space reference, it can be shared freely across the model.

The **Instance** object holds at least three things: a reference to a `Definition` by name, a zone/space/zonelist/spacelist assignment, and an operating schedule.
It also carries the end-use subcategory and any other fields that are specific to a particular installation (e.g., `Return Air Heat Gain Node Name` and `Exhaust Air Heat Gain Node Name` for `Lights`, which are zone-topology-specific).
In addition, a `Multiplier` field (default 1) is added: this value multiplies the `Definition` value. This allows greater flexibility to define a single definition but handle different physical installations: for example a single Definition representing one object (a Workstation, or a light bulb) can be defined with an absolute Watts value, and the Instance object has the count of these objects.

## Approach ##

### IDD changes

For each existing internal gains object type, which will be left untouched, a new `<ObjectType>:Instance` and new `<ObjectType>:Definition` IDD objects are introduced.

**Instance object** retains / gains:
- `Name`
- `<ObjectType> Definition Name` (new, required reference field)
- `Zone or ZoneList or Space or SpaceList Name`
- `Schedule Name`
- `End-Use Subcategory`
- Any zone/HVAC-topology-specific fields (e.g., return/exhaust air heat gain node names on `Lights`)

**Definition object** receives all formerly inline physical fields:
- `Design Level Calculation Method` and associated numeric values
- Heat gain fraction fields
- Type-specific fields (fuel type, CO₂ generation, thermal comfort model choices, ITE parameters, etc.)

For example, `ElectricEquipment` goes from a single object with fields for zone name, schedule, design level method, fractions, and end-use subcategory, to:

```
ElectricEquipment:Definition,
  Workstation Def,        !- Name
  Watts/Area,             !- Design Level Calculation Method
  , ,                     !- (Equipment Level, Watts/Area, Watts/Person)
  10.0,                   !- Watts per Floor Area {W/m2}
  ,                       !- Watts per Person
  0.0,                    !- Fraction Latent
  0.5,                    !- Fraction Radiant
  0.0;                    !- Fraction Lost

ElectricEquipment:Instance,
  Zone1 Workstation,      !- Name
  Workstation Def,        !- Electric Equipment Definition Name
  Zone 1,                 !- Zone or ZoneList or Space or SpaceList Name
  Office Schedule,        !- Schedule Name
  1.0,                    !- Multiplier
  Workstations;           !- End-Use Subcategory
```

Another example with a Lights object:

```
Lights:Instance,
  Space 1 Lights,                         !- Name
  60 W Incandescent Bulb,                 !- Lights Definition Name
  Space 1,                                !- Zone or ZoneList or Space or SpaceList Name
  Lights Schedule,                        !- Schedule Name
  1.0,                                    !- Fraction Replaceable
  12,                                     !- Multiplier
  General,                                !- End-Use Subcategory
  ,                                       !- Return Air Heat Gain Node Name
  ;                                       !- Exhaust Air Heat Gain Node Name

Lights:Definition,
  60W Incandescent Bulb,                  !- Name
  LightingLevel,                          !- Design Level Calculation Method
  60.0,                                   !- Lighting Level {W}
  ,                                       !- Watts per Floor Area {W/m2}
  ,                                       !- Watts per Person {W/person}
  0,                                      !- Return Air Fraction
  0,                                      !- Fraction Radiant
  0,                                      !- Fraction Visible
  No,                                     !- Return Air Fraction Calculated from Plenum Temperature
  0.0,                                    !- Return Air Fraction Function of Plenum Temperature Coefficient 1
  0.0;                                    !- Return Air Fraction Function of Plenum Temperature Coefficient 2 {1/K}
```

Below is the proposed IDD for each object

#### People

| Field Name                                           | New Location      |
|------------------------------------------------------|-------------------|
| Name                                                 | People:Instance   |
| Zone or ZoneList or Space or SpaceList Name          | People:Instance   |
| Number of People Schedule Name                       | People:Instance   |
| Multiplier                                           | People:Instance   |
| Number of People Calculation Method                  | People:Definition |
| Number of People                                     | People:Definition |
| People per Floor Area                                | People:Definition |
| Floor Area per Person                                | People:Definition |
| Fraction Radiant                                     | People:Definition |
| Sensible Heat Fraction                               | People:Definition |
| Activity Level Schedule Name                         | People:Instance   |
| Carbon Dioxide Generation Rate                       | People:Definition |
| Enable ASHRAE 55 Comfort Warnings                    | People:Definition |
| Mean Radiant Temperature Calculation Type            | People:Definition |
| Surface Name/Angle Factor List Name                  | People:Instance   |
| Work Efficiency Schedule Name                        | People:Instance   |
| Clothing Insulation Calculation Method               | People:Instance   |
| Clothing Insulation Calculation Method Schedule Name | People:Instance   |
| Clothing Insulation Schedule Name                    | People:Instance   |
| Air Velocity Schedule Name                           | People:Instance   |
| Thermal Comfort Model 1 Type                         | People:Definition |
| Thermal Comfort Model 2 Type                         | People:Definition |
| Thermal Comfort Model 3 Type                         | People:Definition |
| Thermal Comfort Model 4 Type                         | People:Definition |
| Thermal Comfort Model 5 Type                         | People:Definition |
| Thermal Comfort Model 6 Type                         | People:Definition |
| Thermal Comfort Model 7 Type                         | People:Definition |
| Ankle Level Air Velocity Schedule Name               | People:Instance   |
| Cold Stress Temperature Threshold                    | People:Instance   |
| Heat Stress Temperature Threshold                    | People:Instance   |

```
People:Instance,
       \memo Sets internal gains and contaminant rates for occupants in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 5
  A1, \field Name
       \type alpha
       \required-field
       \reference PeopleNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field People Definition Name
       \type object-list
       \required-field
       \object-list PeopleDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Number of People Schedule Name
       \note units in schedule should be fraction applied to number of people (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  A5, \field Activity Level Schedule Name
       \note Note that W has to be converted to mets in TC routine
       \note units in schedule are W/person
       \type object-list
       \required-field
       \object-list ScheduleNames
  A6, \field Surface Name/Angle Factor List Name
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)
       \type object-list
       \object-list AllHeatTranAngFacNames
  A7, \field Work Efficiency Schedule Name
       \note units in schedule are 0.0 to 1.0
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)
       \type object-list
       \object-list ScheduleNames
  A8, \field Clothing Insulation Calculation Method
       \type choice
       \default ClothingInsulationSchedule
       \key ClothingInsulationSchedule
       \key DynamicClothingModelASHRAE55
       \key CalculationMethodSchedule
  A9, \field Clothing Insulation Calculation Method Schedule Name
       \note a schedule value of 1 for the Scheduled method, and 2 for the DynamicClothingModelASHRAE55 method
       \type object-list
       \object-list ScheduleNames
  A10, \field Clothing Insulation Schedule Name
       \note use "Clo" from ASHRAE or Thermal Comfort guides
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)
       \type object-list
       \object-list ScheduleNames
  A11, \field Air Velocity Schedule Name
       \note units in the schedule are m/s
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)
       \type object-list
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A12, \field Ankle Level Air Velocity Schedule Name
       \note units in the schedule are m/s
       \note this is the schedule of the air speed at the 0.1 m above the floor
       \note optional (only required for runs of thermal comfort models AnkleDraftASH55)
       \type object-list
       \object-list ScheduleNames
  N2, \field Cold Stress Temperature Threshold
       \note this is the indoor safe temperature threshold for cold stress
       \type real
       \units C
       \default 15.56
  N3; \field Heat Stress Temperature Threshold
       \note this is the indoor safe temperature threshold for heat stress
       \type real
       \units C
       \default 30

People:Definition,
       \min-fields 5
  A1, \field Name
       \type alpha
       \required-field
       \reference PeopleDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Number of People Calculation Method
       \note The entered calculation method is used to create the maximum number of people
       \note for this set of attributes (i.e. sensible fraction, schedule, etc)
       \note Choices: People -- simply enter number of occupants.
       \note People per Floor Area -- enter the number to apply. Value * Floor Area = Number of people
       \note Floor Area per Person -- enter the number to apply. Floor Area / Value = Number of people
       \type choice
       \default People
       \key People
       \key People/Area
       \key Area/Person
  N1, \field Number of People
       \type real
       \minimum 0
  N2, \field People per Floor Area
       \type real
       \units person/m2
       \minimum 0
  N3, \field Floor Area per Person
       \type real
       \units m2/person
       \minimum 0
  N4, \field Fraction Radiant
       \note This is radiant fraction of the sensible heat released by people in a zone. This value will be
       \note multiplied by the total sensible heat released by people yields the amount of long wavelength
       \note radiation gain from people in a zone. Default value is 0.30.
       \type real
       \minimum 0
       \maximum 1
       \default 0.3
  N5, \field Sensible Heat Fraction
       \note if input, overrides program calculated sensible/latent split
       \type real
       \autocalculatable
       \minimum 0
       \maximum 1
       \default autocalculate
  N6, \field Carbon Dioxide Generation Rate
       \note CO2 generation rate per unit of activity level.
       \note The default value is obtained from ASHRAE Std 62.1 at 0.0084 cfm/met/person over
       \note the general adult population.
       \type real
       \units m3/s-W
       \minimum 0
       \maximum 3.82e-07
       \default 3.82E-8
  A3, \field Enable ASHRAE 55 Comfort Warnings
       \type choice
       \default No
       \key Yes
       \key No
  A4, \field Mean Radiant Temperature Calculation Type
       \note optional (only required for thermal comfort runs)
       \type choice
       \default EnclosureAveraged
       \key EnclosureAveraged
       \key SurfaceWeighted
       \key AngleFactor
  A5, \field Thermal Comfort Model 1 Type
       \note optional (only needed for people thermal comfort results reporting)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A6, \field Thermal Comfort Model 2 Type
       \note optional (second type of thermal comfort model and results reporting)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A7, \field Thermal Comfort Model 3 Type
       \note optional (third thermal comfort model and report type)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A8, \field Thermal Comfort Model 4 Type
       \note optional (fourth thermal comfort model and report type)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A9, \field Thermal Comfort Model 5 Type
       \note optional (fifth thermal comfort model and report type)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A10, \field Thermal Comfort Model 6 Type
       \note optional (sixth thermal comfort model and report type)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55
  A11; \field Thermal Comfort Model 7 Type
       \note optional (seventh thermal comfort model and report type)
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55

```


#### Lights

| Field Name                                                       | New Location      |
|------------------------------------------------------------------|-------------------|
| Name                                                             | Lights:Instance   |
| Zone or ZoneList or Space or SpaceList Name                      | Lights:Instance   |
| Schedule Name                                                    | Lights:Instance   |
| Design Level Calculation Method                                  | Lights:Definition |
| Lighting Level                                                   | Lights:Definition |
| Watts per Floor Area                                             | Lights:Definition |
| Watts per Person                                                 | Lights:Definition |
| Return Air Fraction                                              | Lights:Definition |
| Fraction Radiant                                                 | Lights:Definition |
| Fraction Visible                                                 | Lights:Definition |
| Fraction Replaceable                                             | Lights:Instance   |
| Multiplier                                                       | Lights:Instance   |
| End-Use Subcategory                                              | Lights:Instance   |
| Return Air Fraction Calculated from Plenum Temperature           | Lights:Definition |
| Return Air Fraction Function of Plenum Temperature Coefficient 1 | Lights:Definition |
| Return Air Fraction Function of Plenum Temperature Coefficient 2 | Lights:Definition |
| Return Air Heat Gain Node Name                                   | Lights:Instance   |
| Exhaust Air Heat Gain Node Name                                  | Lights:Instance   |

```
Lights:Instance,
       \memo Sets internal gains for lights in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 5
  A1, \field Name
       \type alpha
       \required-field
       \reference LightsNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Lights Definition Name
       \type object-list
       \required-field
       \object-list LightsDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Schedule Name
       \note units in schedule should be fraction applied to design level of lights, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Fraction Replaceable
       \note For Daylighting:Controls must be 0 or 1:  0 = no dimming control, 1 = full dimming control
       \type real
       \minimum 0
       \maximum 1
       \default 1.0
  N2, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A5, \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General
  A6, \field Return Air Heat Gain Node Name
       \note Name of the return air node for this heat gain.
       \note If left blank, defaults to the first return air node for the zone.
       \note Leave this field blank when using a ZoneList name.
       \type node
  A7; \field Exhaust Air Heat Gain Node Name
       \note Name of the exhaust air node for this heat gain.
       \note If the node name is entered, return heat gain will be shared by both return and exhaust air nodes.
       \note The air properties of both nodes are weighted by both node mass flow rates.
       \type node

Lights:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference LightsDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of lights
       \note for this set of attributes
       \note Choices: LightingLevel => Lighting Level -- simply enter watts of lights
       \note Watts/Area => Watts per Floor Area -- enter the number to apply. Value * Floor Area = Lights
       \note Watts/Person => Watts per Person -- enter the number to apply. Value * Occupants = Lights
       \type choice
       \default LightingLevel
       \key LightingLevel
       \key Watts/Area
       \key Watts/Person
  N1, \field Lighting Level
       \type real
       \units W
       \ip-units W
       \minimum 0
  N2, \field Watts per Floor Area
       \type real
       \units W/m2
       \ip-units W/ft2
       \minimum 0
  N3, \field Watts per Person
       \type real
       \units W/person
       \ip-units W/person
       \minimum 0
  N4, \field Return Air Fraction
       \note Used only for sizing calculation if return-air-fraction
       \note coefficients are specified.
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6, \field Fraction Visible
       \type real
       \minimum 0
       \maximum 1
       \default 0
  A3, \field Return Air Fraction Calculated from Plenum Temperature
       \type choice
       \default No
       \key Yes
       \key No
  N7, \field Return Air Fraction Function of Plenum Temperature Coefficient 1
       \note Used only if Return Air Fraction Is Calculated from Plenum Temperature = Yes
       \note Equation is Return Air Fraction = Coefficient#1 - Coefficient#2 X PlenumTemp(degC)
       \type real
       \minimum 0
       \default 0.0
  N8; \field Return Air Fraction Function of Plenum Temperature Coefficient 2
       \note Used only if Return Air Fraction Is Calculated from Plenum Temperature = Yes
       \note Equation is Return Air Fraction = Coefficient#1 - Coefficient#2 X PlenumTemp(degC)
       \type real
       \units 1/K
       \minimum 0
       \default 0.0

```


#### ElectricEquipment

| Field Name                                  | New Location                 |
|---------------------------------------------|------------------------------|
| Name                                        | ElectricEquipment:Instance   |
| Zone or ZoneList or Space or SpaceList Name | ElectricEquipment:Instance   |
| Schedule Name                               | ElectricEquipment:Instance   |
| Multiplier                                  | ElectricEquipment:Instance   |
| Design Level Calculation Method             | ElectricEquipment:Definition |
| Design Level                                | ElectricEquipment:Definition |
| Watts per Floor Area                        | ElectricEquipment:Definition |
| Watts per Person                            | ElectricEquipment:Definition |
| Fraction Latent                             | ElectricEquipment:Definition |
| Fraction Radiant                            | ElectricEquipment:Definition |
| Fraction Lost                               | ElectricEquipment:Definition |
| End-Use Subcategory                         | ElectricEquipment:Instance   |

```
ElectricEquipment:Instance,
       \memo Sets internal gains for electric equipment in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 4
  A1, \field Name
       \type alpha
       \required-field
       \reference ElectricEquipmentNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Electric Equipment Definition Name
       \type object-list
       \required-field
       \object-list ElectricEquipmentDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Schedule Name
       \note units in schedule should be fraction applied to design level of electric equipment, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A5; \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General

ElectricEquipment:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference ElectricEquipmentDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of electric equipment
       \note for this set of attributes
       \note Choices: EquipmentLevel => Equipment Level -- simply enter watts of equipment
       \note Watts/Area => Watts per Floor Area -- enter the number to apply. Value * Floor Area = Equipment Level
       \note Watts/Person => Watts per Person -- enter the number to apply. Value * Occupants = Equipment Level
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
       \key Watts/Person
  N1, \field Design Level
       \type real
       \units W
       \ip-units W
       \minimum 0
  N2, \field Watts per Floor Area
       \type real
       \units W/m2
       \ip-units W/ft2
       \minimum 0
  N3, \field Watts per Person
       \type real
       \units W/person
       \ip-units W/person
       \minimum 0
  N4, \field Fraction Latent
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6; \field Fraction Lost
       \type real
       \minimum 0
       \maximum 1
       \default 0

```


#### GasEquipment

| Field Name                                  | New Location            |
|---------------------------------------------|-------------------------|
| Name                                        | GasEquipment:Instance   |
| Zone or ZoneList or Space or SpaceList Name | GasEquipment:Instance   |
| Schedule Name                               | GasEquipment:Instance   |
| Multiplier                                  | GasEquipment:Instance   |
| Design Level Calculation Method             | GasEquipment:Definition |
| Design Level                                | GasEquipment:Definition |
| Power per Floor Area                        | GasEquipment:Definition |
| Power per Person                            | GasEquipment:Definition |
| Fraction Latent                             | GasEquipment:Definition |
| Fraction Radiant                            | GasEquipment:Definition |
| Fraction Lost                               | GasEquipment:Definition |
| Carbon Dioxide Generation Rate              | GasEquipment:Definition |
| End-Use Subcategory                         | GasEquipment:Instance   |

```
GasEquipment:Instance,
       \memo Sets internal gains and contaminant rates for gas equipment in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 4
  A1, \field Name
       \type alpha
       \required-field
       \reference GasEquipmentNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Gas Equipment Definition Name
       \type object-list
       \required-field
       \object-list GasEquipmentDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Schedule Name
       \note units in Schedule should be fraction applied to design level of gas equipment, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A5; \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General

GasEquipment:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference GasEquipmentDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of gas equipment
       \note for this set of attributes
       \note Choices: EquipmentLevel => Design Level -- simply enter power input of equipment
       \note Watts/Area or Power/Area => Power per Floor Area -- enter the number to apply. Value * Floor Area = Equipment Level
       \note Watts/Person or Power/Person => Power per Person -- enter the number to apply. Value * Occupants = Equipment Level
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
       \key Watts/Person
       \key Power/Area
       \key Power/Person
  N1, \field Design Level
       \type real
       \units W
       \ip-units Btu/h
       \minimum 0
  N2, \field Power per Floor Area
       \type real
       \units W/m2
       \ip-units Btu/h-ft2
       \minimum 0
  N3, \field Power per Person
       \type real
       \units W/person
       \ip-units Btu/h-person
       \minimum 0
  N4, \field Fraction Latent
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6, \field Fraction Lost
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N7; \field Carbon Dioxide Generation Rate
       \note CO2 generation rate per unit of power input
       \note The default value assumes the equipment is fully vented.
       \note For unvented equipment, a suggested value is 3.45E-8 m3/s-W. This value is
       \note converted from a natural gas CO2 emission rate of 117 lbs CO2 per million Btu.
       \note The maximum value assumes to be 10 times of the recommended value.
       \type real
       \units m3/s-W
       \ip-units (ft3/min)/(Btu/h)
       \minimum 0
       \maximum 4e-07
       \default 0.0

```


#### HotWaterEquipment

| Field Name                                  | New Location                 |
|---------------------------------------------|------------------------------|
| Name                                        | HotWaterEquipment:Instance   |
| Zone or ZoneList or Space or SpaceList Name | HotWaterEquipment:Instance   |
| Schedule Name                               | HotWaterEquipment:Instance   |
| Multiplier                                  | HotWaterEquipment:Instance   |
| Design Level Calculation Method             | HotWaterEquipment:Definition |
| Design Level                                | HotWaterEquipment:Definition |
| Power per Floor Area                        | HotWaterEquipment:Definition |
| Power per Person                            | HotWaterEquipment:Definition |
| Fraction Latent                             | HotWaterEquipment:Definition |
| Fraction Radiant                            | HotWaterEquipment:Definition |
| Fraction Lost                               | HotWaterEquipment:Definition |
| End-Use Subcategory                         | HotWaterEquipment:Instance   |

```
HotWaterEquipment:Instance,
       \memo Sets internal gains for hot water equipment in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 4
  A1, \field Name
       \type alpha
       \required-field
       \reference HotWaterEquipmentNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Hot Water Equipment Definition Name
       \type object-list
       \required-field
       \object-list HotWaterEquipmentDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Schedule Name
       \note units in Schedule should be fraction applied to design level of hot water equipment, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A5; \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General

HotWaterEquipment:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference HotWaterEquipmentDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of hot water equipment
       \note for this set of attributes
       \note Choices: EquipmentLevel => Design Level -- simply enter power input of equipment
       \note Watts/Area or Power/Area => Power per Floor Area -- enter the number to apply. Value * Floor Area = Equipment Level
       \note Watts/Person or Power/Person => Power per Person -- enter the number to apply. Value * Occupants = Equipment Level
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
       \key Watts/Person
       \key Power/Area
       \key Power/Person
  N1, \field Design Level
       \type real
       \units W
       \ip-units Btu/h
       \minimum 0
  N2, \field Power per Floor Area
       \type real
       \units W/m2
       \ip-units Btu/h-ft2
       \minimum 0
  N3, \field Power per Person
       \type real
       \units W/person
       \ip-units Btu/h-person
       \minimum 0
  N4, \field Fraction Latent
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6; \field Fraction Lost
       \type real
       \minimum 0
       \maximum 1
       \default 0

```


#### SteamEquipment

| Field Name                                  | New Location              |
|---------------------------------------------|---------------------------|
| Name                                        | SteamEquipment:Instance   |
| Zone or ZoneList or Space or SpaceList Name | SteamEquipment:Instance   |
| Schedule Name                               | SteamEquipment:Instance   |
| Multiplier                                  | SteamEquipment:Instance   |
| Design Level Calculation Method             | SteamEquipment:Definition |
| Design Level                                | SteamEquipment:Definition |
| Power per Floor Area                        | SteamEquipment:Definition |
| Power per Person                            | SteamEquipment:Definition |
| Fraction Latent                             | SteamEquipment:Definition |
| Fraction Radiant                            | SteamEquipment:Definition |
| Fraction Lost                               | SteamEquipment:Definition |
| End-Use Subcategory                         | SteamEquipment:Instance   |

```
SteamEquipment:Instance,
       \memo Sets internal gains for steam equipment in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 4
  A1, \field Name
       \type alpha
       \required-field
       \reference SteamEquipmentNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Steam Equipment Definition Name
       \type object-list
       \required-field
       \object-list SteamEquipmentDefinitionNames
  A3, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A4, \field Schedule Name
       \note units in Schedule should be fraction applied to design level of steam equipment, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A5; \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General

SteamEquipment:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference SteamEquipmentDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of steam equipment
       \note for this set of attributes
       \note Choices: EquipmentLevel => Design Level -- simply enter power input of equipment
       \note Watts/Area or Power/Area => Power per Floor Area -- enter the number to apply. Value * Floor Area = Equipment Level
       \note Watts/Person or Power/Person => Power per Person -- enter the number to apply. Value * Occupants = Equipment Level
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
       \key Watts/Person
       \key Power/Area
       \key Power/Person
  N1, \field Design Level
       \type real
       \units W
       \ip-units Btu/h
       \minimum 0
  N2, \field Power per Floor Area
       \type real
       \units W/m2
       \ip-units Btu/h-ft2
       \minimum 0
  N3, \field Power per Person
       \type real
       \units W/person
       \ip-units Btu/h-person
       \minimum 0
  N4, \field Fraction Latent
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6; \field Fraction Lost
       \type real
       \minimum 0
       \maximum 1
       \default 0

```


#### OtherEquipment

| Field Name                                  | New Location              |
|---------------------------------------------|---------------------------|
| Name                                        | OtherEquipment:Instance   |
| Fuel Type                                   | OtherEquipment:Instance   |
| Zone or ZoneList or Space or SpaceList Name | OtherEquipment:Instance   |
| Schedule Name                               | OtherEquipment:Instance   |
| Multiplier                                  | OtherEquipment:Instance   |
| Design Level Calculation Method             | OtherEquipment:Definition |
| Design Level                                | OtherEquipment:Definition |
| Power per Floor Area                        | OtherEquipment:Definition |
| Power per Person                            | OtherEquipment:Definition |
| Fraction Latent                             | OtherEquipment:Definition |
| Fraction Radiant                            | OtherEquipment:Definition |
| Fraction Lost                               | OtherEquipment:Definition |
| Carbon Dioxide Generation Rate              | OtherEquipment:Definition |
| End-Use Subcategory                         | OtherEquipment:Instance   |

```
OtherEquipment:Instance,
       \memo Sets internal gains or losses for "other" equipment in the zone.
       \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 5
  A1, \field Name
       \type alpha
       \required-field
       \reference OtherEquipmentNames
       \reference SpaceItemNames
       \reference SpaceLoadNames
       \reference SpaceComponentInstanceNames
  A2, \field Other Equipment Definition Name
       \type object-list
       \required-field
       \object-list OtherEquipmentDefinitionNames
  A3, \field Fuel Type
       \type choice
       \default None
       \key None
       \key Electricity
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Diesel
       \key Gasoline
       \key Coal
       \key OtherFuel1
       \key OtherFuel2
       \key DistrictHeatingWater
       \key DistrictHeatingSteam
       \key DistrictCooling
  A4, \field Zone or ZoneList or Space or SpaceList Name
       \type object-list
       \required-field
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A5, \field Schedule Name
       \note units in Schedule should be fraction applied to design level of other equipment, generally (0.0 - 1.0)
       \type object-list
       \required-field
       \object-list ScheduleNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A6; \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General

OtherEquipment:Definition,
       \min-fields 8
  A1, \field Name
       \type alpha
       \required-field
       \reference OtherEquipmentDefinitionNames
       \reference SpaceComponentDefinitionNames
  A2, \field Design Level Calculation Method
       \note The entered calculation method is used to create the maximum amount of other equipment.
       \note to set a loss, use a negative value in the following fields.
       \note for this set of attributes
       \note Choices: EquipmentLevel => Design Level -- simply enter power input of equipment
       \note Watts/Area or Power/Area => Power per Floor Area -- enter the number to apply. Value * Floor Area = Equipment Level
       \note Watts/Person or Power/Person => Power per Person -- enter the number to apply. Value * Occupants = Equipment Level
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
       \key Watts/Person
       \key Power/Area
       \key Power/Person
  N1, \field Design Level
       \type real
       \units W
       \ip-units Btu/h
  N2, \field Power per Floor Area
       \type real
       \units W/m2
       \ip-units Btu/h-ft2
  N3, \field Power per Person
       \type real
       \units W/person
       \ip-units Btu/h-person
  N4, \field Fraction Latent
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N5, \field Fraction Radiant
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N6, \field Fraction Lost
       \type real
       \minimum 0
       \maximum 1
       \default 0
  N7; \field Carbon Dioxide Generation Rate
       \note CO2 generation rate per unit of power input
       \note The default value assumes the equipment is fully vented.
       \type real
       \units m3/s-W
       \ip-units (ft3/min)/(Btu/h)
       \minimum 0
       \maximum 4e-07
       \default 0.0

```


#### ElectricEquipment:ITE:AirCooled

| Field Name                                                              | New Location                               |
|-------------------------------------------------------------------------|--------------------------------------------|
| Name                                                                    | ElectricEquipment:ITE:AirCooled:Instance   |
| Zone or Space Name                                                      | ElectricEquipment:ITE:AirCooled:Instance   |
| Air Flow Calculation Method                                             | ElectricEquipment:ITE:AirCooled:Definition |
| Design Power Input Calculation Method                                   | ElectricEquipment:ITE:AirCooled:Definition |
| Watts per Unit                                                          | ElectricEquipment:ITE:AirCooled:Definition |
| Multiplier                                                              | ElectricEquipment:ITE:AirCooled:Instance   |
| Watts per Floor Area                                                    | ElectricEquipment:ITE:AirCooled:Definition |
| Design Power Input Schedule Name                                        | ElectricEquipment:ITE:AirCooled:Instance   |
| CPU Loading Schedule Name                                               | ElectricEquipment:ITE:AirCooled:Instance   |
| CPU Power Input Function of Loading and Air Temperature Curve Name      | ElectricEquipment:ITE:AirCooled:Definition |
| Design Fan Power Input Fraction                                         | ElectricEquipment:ITE:AirCooled:Definition |
| Design Fan Air Flow Rate per Power Input                                | ElectricEquipment:ITE:AirCooled:Definition |
| Air Flow Function of Loading and Air Temperature Curve Name             | ElectricEquipment:ITE:AirCooled:Definition |
| Fan Power Input Function of Flow Curve Name                             | ElectricEquipment:ITE:AirCooled:Definition |
| Design Entering Air Temperature                                         | ElectricEquipment:ITE:AirCooled:Definition |
| Environmental Class                                                     | ElectricEquipment:ITE:AirCooled:Definition |
| Air Inlet Connection Type                                               | ElectricEquipment:ITE:AirCooled:Definition |
| Air Inlet Room Air Model Node Name                                      | ElectricEquipment:ITE:AirCooled:Instance   |
| Air Outlet Room Air Model Node Name                                     | ElectricEquipment:ITE:AirCooled:Instance   |
| Supply Air Node Name                                                    | ElectricEquipment:ITE:AirCooled:Instance   |
| Design Recirculation Fraction                                           | ElectricEquipment:ITE:AirCooled:Definition |
| Recirculation Function of Loading and Supply Temperature Curve Name     | ElectricEquipment:ITE:AirCooled:Definition |
| Design Electric Power Supply Efficiency                                 | ElectricEquipment:ITE:AirCooled:Definition |
| Electric Power Supply Efficiency Function of Part Load Ratio Curve Name | ElectricEquipment:ITE:AirCooled:Definition |
| Fraction of Electric Power Supply Losses to Zone                        | ElectricEquipment:ITE:AirCooled:Definition |
| CPU End-Use Subcategory                                                 | ElectricEquipment:ITE:AirCooled:Instance   |
| Fan End-Use Subcategory                                                 | ElectricEquipment:ITE:AirCooled:Instance   |
| Electric Power Supply End-Use Subcategory                               | ElectricEquipment:ITE:AirCooled:Instance   |
| Supply Temperature Difference                                           | ElectricEquipment:ITE:AirCooled:Definition |
| Supply Temperature Difference Schedule                                  | ElectricEquipment:ITE:AirCooled:Definition |
| Return Temperature Difference                                           | ElectricEquipment:ITE:AirCooled:Definition |
| Return Temperature Difference Schedule                                  | ElectricEquipment:ITE:AirCooled:Definition |


Note: this object already has a sort of "Multiplier" field in the sense that it has a "Number of Units" field, but that field only applies if Design Power Input Calculation Method is "Watts/Unit". This is changed to be a proper Multiplier field to match the other objects, and it will also apply if Design Power Input Calculation Method is "Watts/Area".
Similarly, Design Power Input Calculation Method choices are changed: key "Watts/Unit" is renamed to "EquipmentLevel" to match the other objects terminology.

```
ElectricEquipment:ITE:AirCooled:Instance,
       \memo This object describes air-cooled electric information technology equipment (ITE) which has
       \memo variable power consumption as a function of loading and temperature.
       \memo If a Zone comprised of more than one Space is specified
       \memo then this definition applies to all applicable spaces, and each instance will
       \memo be named with the Space Name plus this Object Name.
       \min-fields 12
  A1, \field Name
       \type alpha
       \required-field
  A2, \field ElectricEquipment ITE AirCooled Definition Name
       \type object-list
       \required-field
       \object-list ElectricEquipmentITEAirCooledDefinitionNames
  A3, \field Zone or Space Name
       \note ZoneList and SpaceList names are not allowed.
       \type object-list
       \required-field
       \object-list ZoneNames
       \object-list SpaceNames
  N1, \field Multiplier
       \type real
       \minimum 0
       \default 1
  A4, \field Design Power Input Schedule Name
       \note Operating schedule for this equipment, fraction applied to the design power input,
       \note generally (0.0 - 1.0)
       \note If this field is blank, the schedule is assumed to always be 1.0.
       \type object-list
       \object-list ScheduleNames
  A5, \field CPU Loading Schedule Name
       \note CPU loading schedule for this equipment as a fraction from 0.0 (idle) to 1.0 (full load).
       \note If this field is blank, the schedule is assumed to always be 1.0.
       \type object-list
       \object-list ScheduleNames
  A6, \field Air Inlet Room Air Model Node Name
       \note Name of a RoomAir:Node object which is connected to the ITE air inlet.
       \type object-list
       \object-list RoomAirNodes
  A7, \field Air Outlet Room Air Model Node Name
       \note Name of a RoomAir:Node object which is connected to the ITE air outlet.
       \type object-list
       \object-list RoomAirNodes
  A8, \field Supply Air Node Name
       \note Name of the supply air inlet node serving this ITE. Required if the
       \note Air Node Connection Type = AdjustedSupply (set in the Definition). Also required if
       \note Air Flow Calculation Method = FlowControlWithApproachTemperatures (set in the Definition).
       \note Also required if reporting of Supply Heat Index is desired.
       \type node
  A9, \field CPU End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default ITE-CPU
  A10, \field Fan End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default ITE-Fans
  A11; \field Electric Power Supply End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default ITE-UPS

ElectricEquipment:ITE:AirCooled:Definition,
       \memo Performance characteristics for ElectricEquipment:ITE:AirCooled.
       \memo Defines power, airflow, and efficiency as functions of loading and temperature.
       \min-fields 18
  A1, \field Name
       \type alpha
       \required-field
       \reference ElectricEquipmentITEAirCooledDefinitionNames
  A2, \field Air Flow Calculation Method
       \note The specified method is used to calculate the IT inlet temperature and zone return
       \note air temperature. If FlowFromSystem is chosen, the zone is assumed to be well-mixed.
       \note If FlowControlWithApproachTemperatures is chosen, Supply and Return approach temperature
       \note should be defined to indicate the temperature difference due to the air distribution. When
       \note FlowControlWithApproachTemperatures is chosen, the inputs of Air Inlet Connection Type, Design Recirculation Fraction
       \note and Recirculation Function of Loading and Supply Temperature Curve Name are ignored. For multiple
       \note ITE objects defined for one zone, the same calculation method should apply.
       \note The FlowControlWithApproachTemperatures only applies to ITE zones with single duct VAV terminal unit.
       \note Other return air heat gains from window or lights are not allowed when FlowControlWithApproachTemperatures is chosen.
       \type choice
       \default FlowFromSystem
       \key FlowFromSystem
       \key FlowControlWithApproachTemperatures
  A3, \field Design Power Input Calculation Method
       \note The entered calculation method is used to specify the design power input
       \note Choices: EquipmentLevel => Equipment Level -- simply enter watts of equipment
       \note Watts/Area => Watts per Floor Area -- Design Power = Watts per Floor Area * Floor Area
       \type choice
       \default EquipmentLevel
       \key EquipmentLevel
       \key Watts/Area
  N1, \field Watts per Unit
       \type real
       \units W
       \ip-units W
       \minimum 0
  N2, \field Watts per Floor Area
       \type real
       \units W/m2
       \ip-units W/ft2
       \minimum 0
  A4, \field CPU Power Input Function of Loading and Air Temperature Curve Name
       \note The name of a two-variable curve or table lookup object which modifies the CPU power
       \note input as a function of CPU loading (x) and air inlet node temperature (y).
       \note This curve (table) should equal 1.0 at design conditions (CPU loading = 1.0 and
       \note Design Entering Air Temperature).
       \type object-list
       \required-field
       \object-list BivariateFunctions
  N3, \field Design Fan Power Input Fraction
       \note The fraction of the total power input at design conditions which is for the cooling fan(s)
       \type real
       \minimum 0
       \maximum 1
       \default 0.0
  N4, \field Design Fan Air Flow Rate per Power Input
       \note The cooling fan air flow rate per total electric power input at design conditions
       \type real
       \required-field
       \units m3/s-W
       \minimum 0
  A5, \field Air Flow Function of Loading and Air Temperature Curve Name
       \note The name of a two-variable curve or table lookup object which modifies the cooling
       \note air flow rate as a function of CPU loading (x) and air inlet node temperature (y).
       \note This curve (table) should equal 1.0 at design conditions (CPU loading = 1.0 and
       \note Design Entering Air Temperature).
       \type object-list
       \required-field
       \object-list BivariateFunctions
  A6, \field Fan Power Input Function of Flow Curve Name
       \note The name of a single-variable curve or table lookup object which modifies the cooling
       \note fan power as a function of flow fraction (x).
       \note This curve (table) should equal 1.0 at a flow fraction of 1.0.
       \type object-list
       \required-field
       \object-list UnivariateFunctions
  N5, \field Design Entering Air Temperature
       \note The entering air temperature at design conditions.
       \type real
       \units C
       \default 15.0
  A7, \field Environmental Class
       \note Specifies the allowable operating conditions for the air inlet conditions.
       \note Used for reporting time outside allowable conditions.
       \type choice
       \default None
       \key None
       \key A1
       \key A2
       \key A3
       \key A4
       \key B
       \key C
       \key H1
  A8, \field Air Inlet Connection Type
       \note Specifies the type of connection between the zone and the ITE air inlet node.
       \note AdjustedSupply = ITE inlet temperature will be the current Supply Air Node temperature
       \note adjusted by the current recirculation fraction.
       \note All heat output is added to the zone air heat balance as a convective gain.
       \note ZoneAirNode = ITE air inlet condition is the average zone condition.
       \note All heat output is added to the zone air heat balance as a convective gain.
       \note RoomAirModel = ITE air inlet and outlet are connected to room air model nodes.
       \note This field is only used when Air Flow Calculation Method is FlowFromSystem.
       \type choice
       \default AdjustedSupply
       \key AdjustedSupply
       \key ZoneAirNode
       \key RoomAirModel
  N6, \field Design Recirculation Fraction
       \note The recirculation fraction for this equipment at design conditions. This field is used only
       \note if the Air Node Connection Type = AdjustedSupply. The default is 0.0 (no recirculation).
       \note This field is only used when Air Flow Calculation Method is FlowFromSystem.
       \type real
       \minimum 0
       \maximum 0.5
       \default 0.0
  A9, \field Recirculation Function of Loading and Supply Temperature Curve Name
       \note The name of a two-variable curve or table lookup object which modifies the recirculation
       \note fraction as a function of CPU loading (x) and supply air node temperature (y).
       \note This curve (table) should equal 1.0 at design conditions (CPU loading = 1.0 and
       \note Design Entering Air Temperature). This field is used only if the
       \note Air Node Connection Type = AdjustedSupply. If this curve is left blank, then the curve
       \note is assumed to always equal 1.0.
       \note This field is only used when Air Flow Calculation Method is FlowFromSystem.
       \type object-list
       \object-list BivariateFunctions
  N7, \field Design Electric Power Supply Efficiency
       \note The efficiency of the power supply system serving this ITE
       \type real
       \minimum> 0
       \maximum 1
       \default 1.0
  A10, \field Electric Power Supply Efficiency Function of Part Load Ratio Curve Name
       \note The name of a single-variable curve or table lookup object which modifies the electric
       \note power supply efficiency as a function of part-load ratio (x).
       \note This curve (table) should equal 1.0 at full load (PLR = 1.0).
       \note If this curve is left blank, then the curve is assumed to always equal 1.0.
       \type object-list
       \object-list UnivariateFunctions
  N8, \field Fraction of Electric Power Supply Losses to Zone
       \note Fraction of the electric power supply losses which are a heat gain to the zone
       \note If this field is <1.0, the remainder of the losses are assumed to be lost to the outdoors.
       \type real
       \minimum 0
       \maximum 1
       \default 1.0
  N9, \field Supply Temperature Difference
       \note The difference of the IT inlet temperature from the AHU supply air temperature.
       \note Either Supply Temperature Difference or Supply Temperature Difference Schedule is required if Air Flow Calculation Method is set to FlowControlWithApproachTemperatures.
       \note This field is ignored when Air Flow Calculation Method is FlowFromSystem.
       \type real
       \units deltaC
  A11, \field Supply Temperature Difference Schedule
       \note The difference schedule of the IT inlet temperature from the AHU supply air temperature.
       \note Either Supply Temperature Difference or Supply Temperature Difference Schedule is required if Air Flow Calculation Method is set to FlowControlWithApproachTemperatures.
       \note This field is ignored when Air Flow Calculation Method is FlowFromSystem.
       \type object-list
       \object-list ScheduleNames
  N10, \field Return Temperature Difference
       \note The difference of the actual AHU return air temperature to the IT equipment outlet temperature.
       \note Either Return Temperature Difference or Return Temperature Difference Schedule is required if Air Flow Calculation Method is set to FlowControlWithApproachTemperatures.
       \note This field is ignored when Air Flow Calculation Method is FlowFromSystem.
       \type real
       \units deltaC
  A12; \field Return Temperature Difference Schedule
       \note The difference schedule of the actual AHU return air temperature to the IT equipment outlet temperature.
       \note Either Return Temperature Difference or Return Temperature Difference Schedule is required if Air Flow Calculation Method is set to FlowControlWithApproachTemperatures.
       \note This field is ignored when Air Flow Calculation Method is FlowFromSystem.
       \type object-list
       \object-list ScheduleNames

```


### C++ changes (`InternalHeatGains.cc`)

A helper `GetSpaceLoadDefinition` function is introduced. It reads all objects of a given Definition type from the IDD and returns a vector of `ZoneEquipDefinitionData` structs containing the pre-parsed physical fields.
The existing per-type `GetInternalHeatGains*` logic then looks up the matching definition by name for each instance object and merges the two sets of data before populating the internal data structures that are unique per space/zone (e.g., `state.dataHeatBal->ZoneElectric`).

No changes are needed to the downstream simulation code — the internal `ZoneEquip*` structs remain unchanged; only input parsing is affected.

The `ZoneEquipDefinitionData` and similar structs are Plain Old Data (POD) structs, which are populated directly from epJSON without allocating any arrays, and are scoped only to the relevant portion of GetInternalHeatGains, therefore not increasing the memory burden of the `EnergyPlusData state`.


### Backward compatibility / Transition

No transition rules are needed. The existing internal load objects are left untouched.
No existing testfiles IDFs nor unit tests need to be updated.

## Testing/Validation/Data Sources ##

C++ unit tests are added to ensure that the pairing process works as intended.
A new IDF file is added as a regression case.


## Input Output Reference Documentation ##

A new subsection, **SpaceLoad Instance and Definition Objects**, is added to the *Group -- Internal Gains* section of the Input Output Reference, placed immediately after *Specifying Applicable Zone(s) or Space(s)*. It describes the pairing pattern, distinguishes what belongs in each object, and gives a short example showing a shared definition referenced by multiple instances.

A new entry is added to document each new object pair added (`People:Instance`, `People:Definition`, `Lights:Instance`, `Lights:Definition`, etc)


## Input Description ##

The changes to the IDD are described under **Approach** above. The existing objects are left untouched.

The `<ObjectType>:Definition` objects are:
- **required** — if an instance references a definition that does not exist, a severe error is raised.
- **shareable** — multiple instance objects may reference the same definition name.
- **independent of zone/space** — a definition object carries no zone or space reference and can be placed anywhere in the IDF.


## Outputs Description ##

No new output variables or meters are introduced. Existing output variables remain unchanged since the internal simulation data structures are unmodified.


## Engineering Reference ##

No changes to the Engineering Reference are required. The split is purely an input/parsing concern; the underlying heat balance and thermal comfort calculations are unaffected.


## Example File and Transition Changes ##

No existing testfiles IDFs nor unit tests need to be updated.

A new IDF test file is added to demonstrate the usage of the new Instance and Definition objects.


## References ##

- OpenStudio Model API — `OS:ElectricEquipment` / `OS:ElectricEquipment:Definition` and analogous types.
