SpaceLoad Instance and Definition Objects
================

**Kyle Benne, National Laboratory of the Rockies**

 - Original Date: 04/29/2026
 - Revision Date: 04/29/2026


## Justification for New Feature ##

OpenStudio has long used an Instance/Definition split for internal gains objects (e.g., `OS:ElectricEquipment` / `OS:ElectricEquipment:Definition`). EnergyPlus, however, kept all fields — physical characteristics and zone/space assignments alike — in a single object. This divergence means that every time a field is added or changed in OpenStudio's model it must be reconciled with a differently-shaped EnergyPlus object, and the translation layer between the two tools must carry the extra complexity.

By introducing the same Instance/Definition pattern in EnergyPlus, we achieve three concrete benefits:

1. **Alignment of EnergyPlus and OpenStudio** — the EnergyPlus IDD and the OpenStudio Model API now share the same conceptual shape, reducing the translation surface and the associated maintenance burden. Allowing usage of OpenStudio directly on an IDF will provide ability to work on **any** IDF, including fully unsupported objects (objects not wrapped in OpenStudio model/ namespace) and objects which don't have ReverseTranslator rules (the ReverseTranslator IDF -> OSM is lacking in terms of HVAC especially).
2. **DRY input data** — a single `ElectricEquipment:Definition` describing, say, a standard office workstation can be referenced by any number of `ElectricEquipment` instances across the building, each with its own zone/space assignment and schedule, without repeating the physical characteristics.
3. **Cleaner IDD semantics** — the split makes explicit which properties are intrinsic to the load type (the definition) and which are extrinsic, installation-specific concerns (the instance).


## E-mail and Conference Call Conclusions ##

N/A


## Overview ##

Every internal gains object type is split into two complementary objects:

| Instance                           | Definition                                    |
|------------------------------------|-----------------------------------------------|
| `People`                           | `People:Definition`                           |
| `Lights`                           | `Lights:Definition`                           |
| `ElectricEquipment`                | `ElectricEquipment:Definition`                |
| `GasEquipment`                     | `GasEquipment:Definition`                     |
| `HotWaterEquipment`                | `HotWaterEquipment:Definition`                |
| `SteamEquipment`                   | `SteamEquipment:Definition`                   |
| `OtherEquipment`                   | `OtherEquipment:Definition`                   |
| `ElectricEquipment:ITE:AirCooled`  | `ElectricEquipment:ITE:AirCooled:Definition`  |

The **Definition** object holds all physical characteristics that are intrinsic to the load type: the design level calculation method and its associated values (equipment level, watts per floor area, watts per person, etc.), heat gain fractions (radiant, latent, lost, return air, ...), and any other type-specific properties such as CO₂ generation rate, fuel type, thermal comfort model selections, and ITE-specific parameters. Because a definition carries no zone or space reference, it can be shared freely across the model.

The **Instance** object holds at least three things: a reference to a Definition by name, a zone/space/zonelist/spacelist assignment, and an operating schedule. It also carries the end-use subcategory and any other fields that are specific to a particular installation (e.g., `Return Air Heat Gain Node Name` and `Exhaust Air Heat Gain Node Name` for `Lights`, which are zone-topology-specific).


## Approach ##

### IDD changes

For each existing internal gains object type, a new `<ObjectType>:Definition` IDD object is introduced, and the corresponding `<ObjectType>` instance object is slimmed down to reference it.

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

ElectricEquipment,
  Zone1 Workstation,      !- Name
  Workstation Def,        !- Electric Equipment Definition Name
  Zone 1,                 !- Zone or ZoneList or Space or SpaceList Name
  Office Schedule,        !- Schedule Name
  Workstations;           !- End-Use Subcategory
```

### C++ changes (`InternalHeatGains.cc`)

A helper `GetSpaceLoadDefinition` function is introduced. It reads all objects of a given Definition type from the IDD and returns a vector of `ZoneEquipDefinitionData` structs containing the pre-parsed physical fields.
The existing per-type `GetInternalHeatGains*` logic then looks up the matching definition by name for each instance object and merges the two sets of data before populating the internal data structures that unique per space/zone (e.g., `state.dataHeatBal->ZoneElectric`).

No changes are needed to the downstream simulation code — the internal `ZoneEquip*` structs remain unchanged; only input parsing is affected.

The `ZoneEquipmentDefinitionData` and similar structs are Plain Old Data (POD) structs, which are populated directly from epJSON without allocating any arrays, and it scoped only to the relevant portion of GetInternalHeatGains, therefore not increasing the memory burden of the `EnergyPlusData state`.


### Backward compatibility / Transition

A Transition rule (V26.2.0) is provided for each object type. The rule:
1. Creates a new `<ObjectType>:Definition` object, naming it after the original object, and copies the physical fields into it.
2. Rewrites the original `<ObjectType>` object to remove the physical fields and add the `Definition Name` reference pointing to the newly created definition.

Existing IDF files therefore continue to work after transition without any manual edits. Each original object becomes a 1-to-1 instance/definition pair; users can then consolidate definitions manually when multiple instances share the same physical characteristics.


## Testing/Validation/Data Sources ##

- All existing EnergyPlus test files (`.idf`, `.epJSON`, `.imf`) are updated via the Transition rules and re-validated.
- Unit tests in `InternalHeatGains` are updated to use the new two-object form.
- Python pytest tests cover the Transition CLIs for each object type, verifying that pre-transition files produce outputs identical to post-transition files.
- The existing regression test suite is used as the primary validation criterion; no regression is expected since the simulation data structures are unchanged.


## Input Output Reference Documentation ##

A new subsection, **SpaceLoad Instance and Definition Objects**, is added to the *Group -- Internal Gains* section of the Input Output Reference, placed immediately after *Specifying Applicable Zone(s) or Space(s)*. It describes the pairing pattern, distinguishes what belongs in each object, and gives a short example showing a shared definition referenced by multiple instances.

Each individual object section (`People`, `Lights`, `ElectricEquipment`, ...) gains a new field description for the `<ObjectType> Definition Name` field, and the physical fields are moved to the corresponding `<ObjectType>:Definition` subsection.


## Input Description ##

The changes to the IDD are described under **Approach** above. No new fields are added to any existing field; fields are only moved between the instance and the new definition object. The `End-Use Subcategory` field, previously the last field of the instance objects, is retained on the instance.

The `<ObjectType>:Definition` objects are:
- **required** — if an instance references a definition that does not exist, a severe error is raised.
- **shareable** — multiple instance objects may reference the same definition name.
- **independent of zone/space** — a definition object carries no zone or space reference and can be placed anywhere in the IDF.


## Outputs Description ##

No new output variables or meters are introduced. Existing output variables remain unchanged since the internal simulation data structures are unmodified.


## Engineering Reference ##

No changes to the Engineering Reference are required. The split is purely an input/parsing concern; the underlying heat balance and thermal comfort calculations are unaffected.


## Example File and Transition Changes ##

- All existing test files are updated by the Transition rules.
- Transition: `CreateNewIDFUsingRulesV26_2_0` implements the rules for all eight object types.

- A new example file `XXX.imf` demonstrates the shared-definition pattern. <-- IS THIS NEEDED?

## References ##

- OpenStudio Model API — `OS:ElectricEquipment` / `OS:ElectricEquipment:Definition` and analogous types.
