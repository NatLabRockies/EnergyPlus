# NFP: Deprecation Marking #

## Justification for New Feature ##

According to the description of "Level 2 - Deprecation" in Deprecation.html:

> If a release cycle is completed, and a capability is still planned for deprecation, then the relevant input objects will be marked with a deprecation tag, and the object will be marked as Level 2 in this document.

Unfortunately, while there are several related tags that can be used in the IDD, there isn't a clear way to mark objects as deprecated so that usage of the object is flagged. A process with no means to work the process is not a good process.

## Overview ##

The IDD has two related tags: **deprecated** and **obsolete**:

- **deprecated**: field-level tag that indicates a field is no longer used  
- **obsolete**: object-level tag that indicates the object is no longer used and points the user to a new object  

```
\obsolete NewObject
```

Neither of those are a great fit for marking an object that is to be removed and not replaced.

The **obsolete** tag is closer to the needed tag, with the transition programming emitting a warning if it encounters an obsolete object.


## Approach ##

This NFP proposes to modify the handling of the obsolete tag so that it retains the current behavior when the "new object" is different from the marked object and behaves slightly differently when the new and marked objects are the same.

### Current behavior
- **Transition**: warn that the object is obsolete and give the new object  
  - `Obsolete object=GoingAway encountered. Should be replaced with new object=NewObject`
- **Engine**: NA

The proposed new behavior depends on the marked object and the new object. In the case where the marked object is different from the new object, the same general behavior is retained. So, for the input:

```
ObsoleteObject,
  \obsolete NewObject
  ...
```

the transition program will warn about the new object as it does now and the engine will emit a warning. If the two objects are the same, the warning contents will not refer to the new object but will instead warn that the object is to be removed and not replaced. This input:

```
GoingAway,
  \obsolete GoingAway
  ...
```

will trigger the new behavior. 

### New behavior
- **Transition**: warns about objects that have the obsolete tag, but in two flavors:
  - If the new object is the same as the old object, warn that the object is to be removed and will not be replaced
  - Otherwise, warn that the object is obsolete and give the new object
- **Engine**: warn that the object is obsolete

As an example, an object associated with the DElight feature is marked in the IDD as follows:

```text
Daylighting:DELight:ComplexFenestration,
\min-fields 5
\memo Used for DElight Complex Fenestration of all types
\obsolete Daylighting:DELight:ComplexFenestration
A1, \field Name
\required-field
\note Only used for user reference
\type alpha
...
```

The resulting epJSON schema is:

```json
"Daylighting:DELight:ComplexFenestration": {
  "patternProperties": {
    "^.*\\S.*$": {
      "type": "object",
      "properties": {
        "complex_fenestration_type": {…},
        "building_surface_name": {…},
        "window_name": {…},
        "fenestration_rotation": {…}
      },
      "obsolete": "Daylighting:DELight:ComplexFenestration",…
```

and the (current) `ERR` message is:

```text
Program Version,EnergyPlus, Version 26.2.0-76ea7da922, YMD=2026.04.15 12:26,
** Warning ** warnObsoleteObjects: Object Daylighting:DELight:ComplexFenestration is obsolete and will be removed in the future.
************* Testing Individual Branch Integrity
************* All Branches passed integrity testing
************* Testing Individual Supply Air Path Integrity
************* All Supply Air Paths passed integrity testing
************* Testing Individual Return Air Path Integrity
************* All Return Air Paths passed integrity testing
************* No node connection errors were found.
************* Beginning Simulation
************* Simulation Error Summary *************
************* EnergyPlus Warmup Error Summary. During Warmup: 0 Warning; 0 Severe Errors.
************* EnergyPlus Sizing Error Summary. During Sizing: 0 Warning; 0 Severe Errors.
************* EnergyPlus Completed Successfully-- 1 Warning; 0 Severe Errors; Elapsed Time=00hr 00min 13.53sec
```

## Open Questions ##

The are still some open questions:

- Should both transition and the engine report the same thing (based on the "new object")?
- Should both messages (or just transition) point to **Deprecation.html** and/or the docs?
- This does mean there will be transition breakage in the future. How should that be handled?
- ...

## Input Output Reference Documentation ##
Documentation of this behavior will need to be added.

## Engineering Reference ##
NA

## Appendix: Development Team Comments ##
