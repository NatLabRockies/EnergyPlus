Construction Assignment Set
===========================

**Kyle Benne, National Laboratory of the Rockies**

 - Original Date: 05/19/2026
 - Revision Date: 07/26/2026

## Justification for New Feature ##

EnergyPlus currently requires a `Construction Name` to be explicitly specified on every heat transfer surface and subsurface. In many models, these fields repeat a much smaller set of construction assignments based on surface type and outside boundary condition.

This proposal provides an optional representation for reducing that repetition. Construction assignments may be defined by semantic category at Building or Space scope, while individual surfaces may retain explicit assignments as exceptions. For a scope containing *N* surfaces whose assignments reduce to *K* applicable categories, the shared assignments can be represented by approximately *K* construction references plus a scope reference instead of *N* repeated construction references.

Use of this representation is optional for each individual surface:

- An explicitly specified `Construction Name` is authoritative and retains the existing behavior.
- A blank `Construction Name` requests resolution through the surface's Space and then the Building.
- Defining or referencing a `ConstructionAssignmentSet` does not change any surface that has an explicit `Construction Name`.
- A model may mix explicit and inherited construction assignments.
- If a blank `Construction Name` cannot be resolved, the input is invalid and EnergyPlus terminates with an error.

This representation is not intended to replace explicit construction assignments or to be universally preferable. It trades some local explicitness for reduced repetition and centralized, scoped assignment. A surface with a blank field is not self contained. Users who prefer each surface assignment to remain directly visible can continue to use explicit `Construction Name` fields throughout the model.

## E-mail and Conference Call Conclusions ##

Pull request review identified improved schema alignment between OpenStudio and EnergyPlus and preservation of construction assignment intent during OpenStudio → EnergyPlus → OpenStudio round trips as project context that influenced the proposed object structure. A DOE request for improved schema alignment was also noted during review. These considerations are recorded here as context rather than presented as the native EnergyPlus justification for the feature; the justification above is the optional reduction of repeated construction assignments.

Review also raised whether a single unique construction set at the Building scope would be simpler and whether the word "default" implies a single global object. The object is named `ConstructionAssignmentSet` to describe its role without implying global uniqueness. This is an EnergyPlus input name; the existing OpenStudio API names remain unchanged and translators will map between them. The proposed use of multiple named sets and assignment at the Space scope is addressed in the scope rationale below. The handling of contextual defaults and inherited values should be carefully considered during implementation, particularly with respect to established input processing conventions and input processing order.

## Overview ##

Three new IDD objects are added:

- **`ConstructionAssignmentSet`** — top-level object referenced by `Building` and/or `Space`. Groups references to `SurfaceConstructionAssignments`, `SubSurfaceConstructionAssignments`, and individual constructions for special cases (interior partitions, adiabatic surfaces).
- **`SurfaceConstructionAssignments`** — holds Floor, Wall, and Roof/Ceiling construction references for one category (exterior, interior, or ground-contact).
- **`SubSurfaceConstructionAssignments`** — holds construction references for each sub-surface type (fixed window, operable window, door, glass door, overhead door, skylight, tubular daylighting) for one category (exterior or interior).

Two existing objects are extended with an optional field:

- **`Building`** — gains `Construction Assignment Set Name`.
- **`Space`** — gains `Construction Assignment Set Name`.

The `Construction Name` field on `BuildingSurface:Detailed`, `FenestrationSurface:Detailed`, and `InternalMass` is made optional. If blank, construction is resolved at input processing time by searching Space → Building; if still unresolved, a fatal error is issued.

### Scope Rationale: Multiple Named Sets

A single unique set at the Building scope would reduce repeated construction references for a model with one uniform assignment policy. It would not represent scoped specialization without returning to explicit assignments on every affected surface. For example, a building may use one broadly applicable set while basement spaces use different exterior wall and ground contact constructions but inherit all remaining categories from the Building.

Multiple named `ConstructionAssignmentSet` objects allow a set to be reused by multiple Spaces and allow a Space to reference a partially populated set. Lookup is performed for the requested construction category. If the set at the Space scope does not supply that category, lookup continues to the set at the Building scope. The objects are assignment sets at explicitly referenced scopes rather than competing global defaults. Each participates with a defined priority.

This proposal intentionally limits the available scopes to Building and Space. It does not introduce additional inheritance scopes such as SpaceType or BuildingStory. Surfaces that require distinctions not represented by the selected categories or scopes, such as assignments based on orientation, may continue to use an explicit `Construction Name`.

## Approach ##

### IDD Changes

Add `ConstructionAssignmentSet`, `SurfaceConstructionAssignments`, and `SubSurfaceConstructionAssignments` objects (see **Input Description** for full IDD text).

Add an optional `Construction Assignment Set Name` field to `Building` (after the existing `Minimum Number of Warmup Days` field) and to `Space` (before the extensible `Tag` fields).

Remove `\required-field` from `Construction Name` (field A3) in `BuildingSurface:Detailed`, `FenestrationSurface:Detailed`, and `InternalMass`.

Extend `FenestrationSurface:Detailed`'s `Surface Type` field with four new `\key` values: `FixedWindow`, `OperableWindow`, `Skylight`, `OverheadDoor`. These allow users (and the OpenStudio ForwardTranslator) to express sub-surface intent more precisely, eliminating the current lossy collapse of FixedWindow/OperableWindow/Skylight → `Window` and OverheadDoor → `Door`.

These values are construction selection categories only. They do not change heat transfer, airflow, or operability behavior. Existing `Window` and `Door` values remain valid.

### C++ Changes

**New `SurfaceClass` enum values** (`DataSurfaces.hh`):

Add `FixedWindow`, `OperableWindow`, `Skylight`, and `OverheadDoor` to the `SurfaceClass` enum before `Num`. The existing comment at line 151 already warns that downstream data in `ComputeNominalUwithConvCoeffs` (`DataHeatBalance.cc`) must be kept in sync.

**`OriginalClass` / `Class` remapping** (`SurfaceGeometry.cc` ~line 2469):

EnergyPlus already uses this pattern for `GlassDoor` and `TDD_Diffuser`: `OriginalClass` stores the IDD-declared type while `Class` is remapped for thermal calculations. Extend the existing loop with:

```cpp
if (surf.Class == SurfaceClass::FixedWindow || surf.Class == SurfaceClass::OperableWindow ||
    surf.Class == SurfaceClass::Skylight) {
    surf.Class = SurfaceClass::Window;
}
if (surf.Class == SurfaceClass::OverheadDoor) {
    surf.Class = SurfaceClass::Door;
}
```

**Downstream touch-points** — several places test `OriginalClass` against an explicit set of classes and must be extended to include the new types:

- `AirflowNetwork/Solver.cpp` — multiple checks for `OriginalClass == Window | Door | GlassDoor` that determine whether a sub-surface participates in airflow; the new types must be included.
- `HeatBalanceSurfaceManager.cc` line 1255 — counting logic that special-cases `GlassDoor` and `TDD_Diffuser` by `OriginalClass`; extend to handle new types.
- `OutputReports.cc` lines 651–660 and 950–959 — reporting blocks that enumerate `Window | GlassDoor | TDD_Dome | TDD_Diffuser` by `OriginalClass`; extend for new types.
- `DataHeatBalance.cc` — `ComputeNominalUwithConvCoeffs` per the existing enum comment.

All of these are additive `||` extensions to existing enum checks — no structural changes required.

**Reading new objects** (new function, likely in `SurfaceGeometry.cc` or a dedicated helper):

- Parse all `SurfaceConstructionAssignments` and `SubSurfaceConstructionAssignments` objects into structs.
- Parse all `ConstructionAssignmentSet` objects, resolving the nested object-list references.
- Read the new `Construction Assignment Set Name` field from `Building` and each `Space`.

**Construction resolution** (`GetHTSurfaceData` in `SurfaceGeometry.cc`):

For each `BuildingSurface:Detailed` or `FenestrationSurface:Detailed` with a blank `Construction Name`, apply the following resolution in priority order:

1. Hardcoded construction on the surface (existing behavior — unchanged).
2. `ConstructionAssignmentSet` on the surface's `Space` (if the Space has one).
3. `ConstructionAssignmentSet` on the `Building`.
4. If still unresolved → fatal error.

For paired surfaces, when both surfaces have an explicit `Construction Name`, existing EnergyPlus validation remains unchanged. When at least one construction is inherited, an explicit assignment takes precedence over a Space assignment, and a Space assignment takes precedence over a Building assignment. The construction with higher precedence governs the pair, and EnergyPlus finds or creates its reversed construction for the paired surface.

The lookup within a `ConstructionAssignmentSet` uses `surf.OriginalClass` (the IDD-declared type, set before the thermal remapping) and the outside boundary condition:

| `surf.OriginalClass` | Outside BC | Slot in `ConstructionAssignmentSet` |
|---|---|---|
| Wall / Floor / Roof | Outdoors | Exterior Surface Construction Assignments → Wall/Floor/Roof slot |
| Wall / Floor / Roof | Surface / Zone / Space | Interior Surface Construction Assignments → Wall/Floor/Roof slot |
| Wall / Floor / Roof | Ground* / Foundation | Ground Contact Surface Construction Assignments → Wall/Floor/Roof slot |
| Wall / Floor / Roof | Adiabatic | Adiabatic Surface Construction (direct) |
| FixedWindow / OperableWindow / Skylight / GlassDoor / TDD_Dome / TDD_Diffuser | Exterior parent | Exterior SubSurface Construction Assignments → matching slot |
| FixedWindow / OperableWindow / Skylight / GlassDoor / TDD_Dome / TDD_Diffuser | Interior parent | Interior SubSurface Construction Assignments → matching slot |
| Door / OverheadDoor | Exterior parent | Exterior SubSurface Construction Assignments → matching slot |
| Door / OverheadDoor | Interior parent | Interior SubSurface Construction Assignments → matching slot |
| IntMass | — | Interior Partition Construction (direct) |

Note: `Ceiling` maps to `Roof` in EnergyPlus (`SurfaceGeometry.cc` line 3895–3897); `SurfaceConstructionAssignments` uses a `Roof Ceiling Construction Name` slot for both.

### Scope Limitations

Shading-related fields present in the OS IDD (`Space Shading Construction Name`, `Building Shading Construction Name`, `Site Shading Construction Name`) are **not** included in this initial implementation. These map to `ShadingProperty:Reflectance` objects in E+ and involve a more complex translation path.

`SpaceType` and `BuildingStory` inheritance levels from OS are not included (no E+ equivalent).

### Backward Compatibility

No existing behavior changes for models that hardcode `Construction Name` on every surface.

## Testing/Validation/Data Sources ##

New unit tests in `tst/EnergyPlus/unit/SurfaceGeometry.unit.cc` covering:

- Resolution via Building-level `ConstructionAssignmentSet` only.
- Resolution via Space-level `ConstructionAssignmentSet`, with Building-level fallback for unset slots.
- Space-level set overrides Building-level set for a specific surface type.
- Adiabatic and interior partition construction resolution.
- Paired surfaces retain existing validation when both constructions are explicit.
- Paired surface resolution when an explicit assignment takes precedence over an inherited assignment.
- Paired surface resolution when a Space assignment takes precedence over a Building assignment.
- Creation or reuse of a reversed construction for the paired surface when inherited assignments participate in resolution.
- Fatal error when construction cannot be resolved at any level.
- Surfaces with hardcoded `Construction Name` are unaffected by any `ConstructionAssignmentSet`.
- `FenestrationSurface:Detailed` with `Surface Type = FixedWindow / OperableWindow / Skylight / OverheadDoor` is parsed correctly, `OriginalClass` is set, and `Class` is remapped to `Window` / `Door` as appropriate.

A new shoebox example file (see below) will serve as an integration test via the regression suite.

## Input Output Reference Documentation ##

Add new sections to `doc/input-output-reference/src/overview/group-surface-construction-elements.tex` for `ConstructionAssignmentSet`, `SurfaceConstructionAssignments`, and `SubSurfaceConstructionAssignments`, placed after the existing `Construction` section.

Document the resolution priority (hardcoded → Space → Building), the mapping from surface type + boundary condition to slot, and the fatal error behavior.

Document the new optional `Construction Assignment Set Name` fields on `Building` and `Space`.

Update `BuildingSurface:Detailed`, `FenestrationSurface:Detailed`, and `InternalMass` field descriptions to note that `Construction Name` may now be blank when a `ConstructionAssignmentSet` is in effect.*


## Input Description ##

```
ConstructionAssignmentSet,
       \memo Defines a set of construction assignments for surfaces and sub-surfaces
       \memo based on their type and outside boundary condition. Can be referenced by Building
       \memo or Space objects. Space-level assignments take precedence over Building-level.
  A1 , \field Name
       \type alpha
       \required-field
       \reference ConstructionAssignmentSetNames
  A2 , \field Exterior Surface Construction Assignments Name
       \note Constructions for exterior walls, floors, and roofs (Outside Boundary Condition = Outdoors).
       \type object-list
       \object-list SurfaceConstructionAssignmentNames
  A3 , \field Interior Surface Construction Assignments Name
       \note Constructions for interior surfaces (Outside Boundary Condition = Surface, Zone, or Space).
       \type object-list
       \object-list SurfaceConstructionAssignmentNames
  A4 , \field Ground Contact Surface Construction Assignments Name
       \note Constructions for ground-contact surfaces (Outside Boundary Condition = Ground*, Foundation).
       \type object-list
       \object-list SurfaceConstructionAssignmentNames
  A5 , \field Exterior SubSurface Construction Assignments Name
       \note Constructions for sub-surfaces on exterior base surfaces.
       \type object-list
       \object-list SubSurfaceConstructionAssignmentNames
  A6 , \field Interior SubSurface Construction Assignments Name
       \note Constructions for sub-surfaces on interior base surfaces.
       \type object-list
       \object-list SubSurfaceConstructionAssignmentNames
  A7 , \field Interior Partition Construction Name
       \note Construction for InternalMass objects in spaces referencing this set.
       \type object-list
       \object-list ConstructionNames
  A8 ; \field Adiabatic Surface Construction Name
       \note Construction for surfaces with Outside Boundary Condition = Adiabatic.
       \type object-list
       \object-list ConstructionNames

SurfaceConstructionAssignments,
       \memo Defines construction assignments for the three opaque surface types (Floor, Wall, Roof/Ceiling)
       \memo for a single boundary-condition category (exterior, interior, or ground-contact).
  A1 , \field Name
       \type alpha
       \required-field
       \reference SurfaceConstructionAssignmentNames
  A2 , \field Floor Construction Name
       \type object-list
       \object-list ConstructionNames
  A3 , \field Wall Construction Name
       \type object-list
       \object-list ConstructionNames
  A4 ; \field Roof Ceiling Construction Name
       \type object-list
       \object-list ConstructionNames

SubSurfaceConstructionAssignments,
       \memo Defines construction assignments for each sub-surface type for a single boundary-condition
       \memo category (exterior or interior).
  A1 , \field Name
       \type alpha
       \required-field
       \reference SubSurfaceConstructionAssignmentNames
  A2 , \field Fixed Window Construction Name
       \type object-list
       \object-list ConstructionNames
  A3 , \field Operable Window Construction Name
       \type object-list
       \object-list ConstructionNames
  A4 , \field Door Construction Name
       \type object-list
       \object-list ConstructionNames
  A5 , \field Glass Door Construction Name
       \type object-list
       \object-list ConstructionNames
  A6 , \field Overhead Door Construction Name
       \type object-list
       \object-list ConstructionNames
  A7 , \field Skylight Construction Name
       \type object-list
       \object-list ConstructionNames
  A8 , \field Tubular Daylight Dome Construction Name
       \type object-list
       \object-list ConstructionNames
  A9 ; \field Tubular Daylight Diffuser Construction Name
       \type object-list
       \object-list ConstructionNames
```

New fields on existing objects:

```
Building,
  [existing fields...]
  A4 ; \field Construction Assignment Set Name
       \note Optional. Provides inherited construction assignments for surfaces and sub-surfaces in the building.
       \note Assignments referenced by Space take precedence over assignments referenced by Building.
       \type object-list
       \object-list ConstructionAssignmentSetNames

Space,
  [existing fields, before extensible Tag fields...]
  A4 , \field Construction Assignment Set Name
       \note Optional. Overrides the Building-level Construction Assignment Set for surfaces in this Space.
       \type object-list
       \object-list ConstructionAssignmentSetNames
  A5,  \field Tag 1   ! (renumbered from A4)
       \begin-extensible
  [...]
```

The `Construction Name` field on `BuildingSurface:Detailed`, `FenestrationSurface:Detailed`, and `InternalMass` is made optional (remove `\required-field`).

`FenestrationSurface:Detailed` `Surface Type` gains four new keys:

```
  A2 , \field Surface Type
       \type choice
       \key Window
       \key Door
       \key GlassDoor
       \key TubularDaylightDome
       \key TubularDaylightDiffuser
       \key FixedWindow          ! new
       \key OperableWindow       ! new
       \key Skylight             ! new
       \key OverheadDoor         ! new
```


## Outputs Description ##

No new time series output variables are added.

The existing Envelope Summary already reports the resolved Construction for each reported surface. To make inherited assignments auditable, it will be augmented with a Construction Assignment Source column whose values distinguish Explicit, Space, and Building assignments. No new report is introduced.

## Engineering Reference ##

No changes to the Engineering Reference are required. Construction resolution is a bookkeeping operation with no new physics.

## Example File and Transition Changes ##

A new shoebox example file will be added demonstrating:

- A `Building` with a `ConstructionAssignmentSet` covering all surface/sub-surface types.
- One `Space` with a partial `ConstructionAssignmentSet` that overrides only the Exterior and Ground-contact constructions for that space.
- The remaining spaces relying on the building-level set.
- At least one surface per category (exterior wall, interior wall, ground floor, adiabatic, exterior window, interior door).
- A few surfaces with hardcoded `Construction Name` to verify they are unaffected.

**Transition rules required:**

- **`Space`** — a new field (`Construction Assignment Set Name`) is inserted as A4, before the existing extensible `Tag` fields (which shift from A4/A5/A6 to A5/A6/A7). A transition rule must insert an empty string at position A4 to preserve existing tag values.
- **`Building`** — a new optional field (`Construction Assignment Set Name`) is appended as the last field (after `Minimum Number of Warmup Days`). No data change needed; existing IDFs simply omit it.
- **`BuildingSurface:Detailed`**, **`FenestrationSurface:Detailed`**, **`InternalMass`** — `Construction Name` changes from required to optional. No data change needed; existing IDFs with explicit constructions are unaffected.
- **`FenestrationSurface:Detailed` `Surface Type`** — four new keys added (`FixedWindow`, `OperableWindow`, `Skylight`, `OverheadDoor`). No data change needed; existing `Window` values remain valid and are treated as `FixedWindow` for `ConstructionAssignmentSet` resolution.

## References ##

- OpenStudio Model API — `OS:DefaultConstructionSet` / `OS:DefaultSurfaceConstructions`, corresponding to the proposed EnergyPlus `ConstructionAssignmentSet` / `SurfaceConstructionAssignments`
- OpenStudio `Space_Impl::getDefaultConstructionWithSearchDistance` (`src/model/Space.cpp`)
- OpenStudio `DefaultConstructionSet_Impl::getDefaultConstruction` (`src/model/DefaultConstructionSet.cpp`)
- OpenStudio ForwardTranslator — `ForwardTranslateInteriorPartitionSurface.cpp`
