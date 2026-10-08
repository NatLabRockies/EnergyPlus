Modify `obsolete` Tag Handling for Deprecation Warnings
=======================================================

**Jason W. DeGraw, ORNL**

 - Original Date: 06/18/2026
 - Revision Date: 07/17/2026


## Justification for New Feature ##

EnergyPlus currently documents a Level 2 deprecation process in `Deprecation.html`, but the existing set of IDD tags does not map cleanly onto that process. The relevant statement is:

> If a release cycle is completed, and a capability is still planned for deprecation, then the relevant input objects will be marked with a deprecation tag, and the object will be marked as Level 2 in this document.

In practice, the IDD has two related mechanisms:

- `\deprecated`, a field-level tag indicating that a field is no longer used
- `\obsolete`, an object-level tag indicating that an object is no longer used and names a replacement object

The `\deprecated` tag is too narrow for object-level deprecation, and the current documented `\obsolete` behavior assumes there is always a replacement object. That makes it difficult/impossible to mark an object as planned for removal when there is no direct successor. Looking through historical versions of the IDD, an object in version 1.3.0 was tagged with the `obsolete` tag with a value of `deleted`. The object was then removed from subsequent versions and skipped by transition.

This proposal updates behavior so that the existing `\obsolete` tag explicitly supports the "to be removed, no replacement" case, enabling EnergyPlus to better follow its own documented deprecation process without introducing new schema constructs.

## E-mail and Conference Call Conclusions ##

The feature was discussed on two technicalities calls and suggestions were incorporated into the document.

## Overview ##

This proposal changes the behavior associated with the object-level `obsolete` tag in the IDD and generated JSON schema.

Today, the `obsolete` tag is interpreted as a replacement mapping:

- Transition warns that the object is obsolete and identifies the replacement object.
- The EnergyPlus engine does not issue a corresponding warning.

This proposal changes the handling of the tag based upon the value:

- If the value of the `obsolete` tag is `deleted`, interpret the tag to mean that "this object is obsolete and will be removed in the future with no replacement object."
- In all other cases, keep the current meaning of the tag: the object is obsolete and should be replaced with the named object.

This allows the existing tag to express both "replace this object with another object" and "this object is going away and there is no automatic path forward". No schema format changes are proposed. The change is strictly behavioral and formalizes what was previously done on a case-by-case basis.

## Approach ##

The proposed implementation is to preserve the existing `obsolete` field but change how it is interpreted and handled by both the engine and transition:

1. Modify transition warning behavior so the warning depends on the `obsolete` value:
   - If `obsolete != deleted`, warn that the object is obsolete and should be replaced with the named object.
   - If `obsolete == deleted`, warn that the object is obsolete, will be removed in the future, and has no replacement.
2. Add an EnergyPlus runtime warning for obsolete objects encountered by the engine:
   - If `obsolete != deleted`, warn that the object is obsolete and should be replaced with the named object.
   - If `obsolete == deleted`, warn that the object is obsolete, will be removed in the future, and has no replacement.

One motivating example is `Daylighting:DELight:ComplexFenestration`, which has been identified for possible deprecation in `Deprecation.html`. Under this proposal, it can be marked as:

```text
Daylighting:DELight:ComplexFenestration,
       \min-fields 5
       \memo Used for DElight Complex Fenestration of all types
       \obsolete New=>deleted
       ...
```
This keeps the schema unchanged while signaling that the object is on a removal path without naming a successor object.

Note that the IDD syntax calls for a `New=>` prefix, but then transition splits on `=>`, so the following are equivalent:

```
\obsolete New=>deleted
\obsolete =>deleted
```

This is arguably a transition bug, and fixing it would expand the scope here too much.

## Testing/Validation/Data Sources ##

Testing in the near term will be relatively easy, particularly for the runtime warnings. Both unit testing and regression testing for the runtime warnings is easy while an object is marked for deprecation, but once the object is removed, the unit test is no longer legitimate. As such, the recommended approach to testing is to rely on regression testing:

- the addition of the tag will lead to warnings and diffs that will need to be justified, and
- the removal of the object will similarly lead to fewer warnings and thus diffs that will need to be justified.

This approach will have the least impact the development process.

## Input Output Reference Documentation ##

No Input Output Reference changes are expected for object syntax itself because no new input fields, no new objects, and no new outputs are introduced.

However, related user-facing documentation should be updated:

- `Deprecation.html` should clarify that a self-referential `obsolete` tag means the object is marked for future removal with no designated replacement object.
- If desired, warning messages can direct users to `Deprecation.html` and/or the relevant object documentation for more context.
- The IDD documentation of the `obsolete` tag will be updated.

## Input Description ##

No new input syntax is proposed. This feature changes only the interpretation of an existing object-level tag.

Proposed meanings:

```text
OldObjectName
    \obsolete New=>NewObjectName
```

Means:

- This object is obsolete.
- Transition and runtime warnings identify `NewObjectName` as the replacement.

```text
SameObjectName
    \obsolete New=>deleted
```

Means:

- This object is obsolete.
- The object will be removed in the future.
- There is no replacement object.

## Output Description ##

No new report variables or tabular outputs are proposed.

The user-visible output change is new or revised warning behavior in:

- Transition output
- EnergyPlus runtime warning output

An example runtime warning for the no-replacement case could be:

```text
** Warning ** warnObsoleteObjects: Object Daylighting:DELight:ComplexFenestration is obsolete and will be removed in the future.
```

Possible future refinement:

- Modify warning text in both transition and runtime paths to direct users to `Deprecation.html` and/or the Input Output Reference.

## Engineering Reference ##

No Engineering Reference changes.

## Example File and Transition Changes ##

No example file transition changes are required at this time. However, changes will be required once an object has been removed, and this proposal does not resolve how future transition should behave once an object is actually removed.

## References ##

N/A
