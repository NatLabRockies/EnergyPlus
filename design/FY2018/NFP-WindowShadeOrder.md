Window Shade Order
================

**Jason Glazer, GARD Analytics**

 - December 21, 2017
 - January 10, 2018
 - January 15, 2018 - Added design section


## Justification for New Feature ##

When windows shades (including shades, blinds and switchable glazing) are deployed for glare control or daylighing, they are deployed in the order that the windows appear in the input file rather than a user specified order.

## E-mail and  Conference Call Conclusions ##

Many different approaches were considered during discussions between Jason Glazer, Tianzhen Hong, Mike Witte and Dan Macumber prior to drafting the NFP.

Raustad provided suggestion on maintaining backward compatibility when transitioning files.

## Overview ##

Zones often have more than one window. Currently, EnergyPlus applies window shades in the order of their appearance in the input file. The first shade is applied, and if it does not meet the shading control requirement the second shade is applied, and so on. Users do not have a way to specify how multiple shades can be applied in a certain order or in certain sets.

Currently, each FenestrationSurface:Detailed or Window or GlazedDoor has a field called "Shading Control Name" that references the name of WindowProperty:ShadingControl. Multiple FenestrationSurface:Detailed objects can reference the same WindowProperty:ShadingControl. The WindowProperty:ShadingControl has a field called "Shading Control Type" which has several options but control of window shades occurs with the following options:

- OnIfHighGlare

- MeetDaylightIlluminanceSetpoint (which only applies to SwitchableGlazing)

Window shades are also controled whenever the "Glare Control Is Active" field is set to Yes.

To allow the window shade order to be specified by the user, the referencing between FenestrationSurface:Detailed and WindowProperty:ShadingControl will be reversed with WindowProperty:ShadingControl references a list of FenestrationSurface:Detailed objects in the order that they should be deployed. Since the WindowProperty:ShadingControl is no longer a window property, it will be renamed "WindowShadingControl".

## Approach ##

The DayltgInteriorIllum() routine which is in DaylightingManager.cc will be updated to traverse the list of SurfaceWindows in an order established by the new object.

## IDD Changes ##

The current FenestrationSurface:Detailed and WindowProperty:ShadingControl objects:

```
FenestrationSurface:Detailed,
  A1 , \field Name
  A2 , \field Surface Type
  A3 , \field Construction Name
  A4 , \field Building Surface Name
  A5,  \field Outside Boundary Condition Object
  N1,  \field View Factor to Ground
- A6,  \field Shading Control Name
  A7,  \field Frame and Divider Name
  N2 , \field Multiplier
  N3 , \field Number of Vertices
  N4,  \field Vertex 1 X-coordinate
  N5 , \field Vertex 1 Y-coordinate
  N6 , \field Vertex 1 Z-coordinate
  N7,  \field Vertex 2 X-coordinate
  N8,  \field Vertex 2 Y-coordinate
  N9,  \field Vertex 2 Z-coordinate
  N10, \field Vertex 3 X-coordinate
  N11, \field Vertex 3 Y-coordinate
  N12, \field Vertex 3 Z-coordinate
  N13, \field Vertex 4 X-coordinate
  N14, \field Vertex 4 Y-coordinate
  N15; \field Vertex 4 Z-coordinate

WindowProperty:ShadingControl,
  A1 , \field Name
  A2 , \field Shading Type
  A3 , \field Construction with Shading Name
  A4 , \field Shading Control Type
  A5 , \field Schedule Name
  N1 , \field Setpoint
  A6 , \field Shading Control Is Scheduled
  A7 , \field Glare Control Is Active
  A8,  \field Shading Device Material Name
  A9 , \field Type of Slat Angle Control for Blinds
  A10, \field Slat Angle Schedule Name
  N2 ; \field Setpoint 2

```

The Shading Control Name is removed and is shown above with a minus sign. The revised objects are shown below with additional fields with plus signs.

```
FenestrationSurface:Detailed,
  A1 , \field Name
  A2 , \field Surface Type
  A3 , \field Construction Name
  A4 , \field Building Surface Name
  A5,  \field Outside Boundary Condition Object
  N1,  \field View Factor to Ground
  A7,  \field Frame and Divider Name
  N2 , \field Multiplier
  N3 , \field Number of Vertices
  N4,  \field Vertex 1 X-coordinate
  N5 , \field Vertex 1 Y-coordinate
  N6 , \field Vertex 1 Z-coordinate
  N7,  \field Vertex 2 X-coordinate
  N8,  \field Vertex 2 Y-coordinate
  N9,  \field Vertex 2 Z-coordinate
  N10, \field Vertex 3 X-coordinate
  N11, \field Vertex 3 Y-coordinate
  N12, \field Vertex 3 Z-coordinate
  N13, \field Vertex 4 X-coordinate
  N14, \field Vertex 4 Y-coordinate
  N15; \field Vertex 4 Z-coordinate

WindowShadingControl,
  A1 , \field Name
+ N1 , \field Shading Control Sequence Number
  A2 , \field Shading Type
  A3 , \field Construction with Shading Name
  A4 , \field Shading Control Type
  A5 , \field Schedule Name
  N1 , \field Setpoint
  A6 , \field Shading Control Is Scheduled
  A7 , \field Glare Control Is Active
  A8,  \field Shading Device Material Name
  A9 , \field Type of Slat Angle Control for Blinds
  A10, \field Slat Angle Schedule Name
  N2 , \field Setpoint 2
+ A11, \field Daylighting Control Object Name
+ A12, \field Multiple Surface Control Type
+      \type choice
+      \key Sequential
+      \key Group
+      \default Sequential
+ A13, \field Fenestration Surface 1 Name
+ A14, \field Fenestration Surface 2 Name
+ A15, \field Fenestration Surface 3 Name
+ A16, \field Fenestration Surface 4 Name
+ A17; \field Fenestration Surface 5 Name
```

Notes:

- WindowProperty:ShadingControl renamed to WindowShadingControl
- Window and GlazedDoor objects would also have Shading Control Name field removed.


## Testing/Validation/Data Sources ##

Test cases that use multiple windows where the order of the window shading deployment will be constructed to test if the new algorithm works correctly.

## Input Output Reference Documentation ##

Remove "Shading Control Type" field from FenestrationSurface:Detailed, Window and GlazedDoor objects.

Rename the WindowProperty:ShadingControl object to WindowShadingControl and replace the reference to this object wherever it occurs.

### WindowShadingControl

Add the following new paragraphs:

To specify the order of when shades are deployed, the two different approaches can be used:

- If shades for each window are independently controlled, than a single WindowShadingControl object should be used and the Multiple Surface Control Type field should be set to Sequential. The windows should be specified in the Fenestration Surface N Name fields in the order that they should be deployed. The Shading Control Sequence Number field is not used being used since only one WindowShadingControl is used so it can be set to any value or left blank.

- If shades for a group of windows are deployed together and shades for another group of windows are deployed after that, than multiple WindowShadingControl objects should be used. Each WindowShadingControl should have Multiple Surface Control Type field should be set to Group and the names of each window in the group should be specified in the Fenestration Surface N Name fields. The Shading Control Sequence Number in the  WindowShadingControl object that controls the first group of shades should be set to 1. The Shading Control Sequence Number in the  WindowShadingControl object that controls the second group of shades should be set to 2. Any number of additional groups of shades can be added and Shading Control Sequence Number can be incremented by one for each group.

It is possible to mix these two approaches. For example, if first a group of shades is controlled together followed by a sequence of individual windows than these approaches can be combined with the Multiple Surface Control Type field should be set to Sequential in the second WindowShadingControl object.

### Inputs

Only showing new fields in the following:

**Field: Shading Control Sequence Number**

If multiple WindowShadingControl objects are used than the order that they deploy the window shades can be set with this field. The first WindowShadingControl should be 1 and subsequent WindowShadingControl should 2, 3, etc. This is usually used when the Multiple Surface Control Type field is set to Group and groups of windows are being controlled in a certain order.

**Field: Daylighting Controls Object Name**

Reference to the Daylighting:Controls object that provides the glare and illuminance control to the zone.

**Field: Multiple Surface Control Type**

The field can have one of two options:

- Sequential - this means that the following list of fenestration surfaces are controlled individually in the order specified

- Group - this means that the entire list is controlled simultaneously and if glare control is needed the entire group of window shades are deployed together a the same time

**Field: Fenestration Surface 1 Name**

The name of the FenestrationSurface:Detailed, Window, or GlazedDoor objects. WHen Multiple Surface Control Type is set to Sequential, the order of the  FenestrationSurface:Detailed, Window, or GlazedDoor objects is the order that the shades will be deployed. This field can be repeated for each fenestration surface with shading. The object is extensible so that additional fields of fenestration surface names can be added to the object.


## Outputs Description ##

No output changes are expected.

## Engineering Reference ##

The section on Glare Control Logic in the Daylighting and Window Calculations section of the Engineering Reference will be updated. The current text reads:


**Glare Control Logic**

If glare control has been specified and the glare index at either reference point exceeds a userspecified
maximum value, G_Imax, then the windows in the zone are shaded one by one in attempt
to bring the glare at both points below GI;max. (Each time a window is shaded the glare and
illuminance at each reference point is recalculated.) The following logic is used:

1) If there is only one reference point, shade a window if it is unshaded and shading it decreases
the glare, even if it does not decrease the glare below GI;max. Note that if a window has already
been shaded, say to control solar gain, it will be left in the shaded state.

2) If there are two reference points, then:

- If glare is too high at both points, shade the window if it decreases glare at both points.
If glare is too high only at the first point, shade the window if the glare at the first point decreases,
and the glare at the second point stays below G_Imax.

- If glare is too high only at the second point, shade the window if the glare at the second point
decreases, and the glare at the first point stays below G_Imax.

3) Shades are closed in the order of window input until glare at both points is below G_Imax,
or until there are no more windows left to shade.


Bullet (3) will be rewritten to read:


3) Shades are closed in the order ~~of window input~~ __specified by order of fenestration objects listed in  the WindowShadingControl object__ until glare at both points is below G_Imax, or until there are no more windows left to shade.



## Example File and Transition Changes ##

Signaticant transition changes will be necessary since the referencing between objects is changing as well as the addition of new fields.

To help provide backward compatibility the transition program will set WindowShadingControl Shading Control Sequence Number = n, where n is the order of occurrence of WindowProperty:ShadingControl in the idf.

## References ##

None.

## Design ##

In order to read the objects that have changed, the following will be modified:

- GetHTSubSurfaceData() and GetRectSubSurfaces() and GetWindowShadingControlData() in SurfaceGeometry.cc
- SurfData struct will be modified to remove  the member WindowShadingControlPtr
- WindowShadingControlData struct will be modified to add the new input fields

WindowShadingControlPtr is used in:

- DaylightingDevices.cc
- DaylightingManager.cc
- DElightManagerF.cc
- EMSManager.cc
- HeatBalanceSurfaceManager.cc
- SolarShading.cc
- SurfaceGeometry.cc
- WindowManager.cc

In most cases, the code is just checking if the suface has shading, so those should be easy to fix. In other cases, more extensive changes will be required. Upon a more detailed code review, it might be advantageous to leave the struct alone and just populate it from the new WindowShadingControl object.

While the object is changing name from WindowProperty:ShadingControl to WindowShadingControl, the same WindowShadingControl() array will be used with just the underlying struct modified. The addition of members for the Shading Control Sequence Number, the Daylighting Control Object Name, Multiple Suface Control Type and an array for each Fenestration Suface will be made.

The DayltgInteriorIllum() function is in DaylightingManager.cc is the main function to be updated to support the new ordered window shading controls. The function will be modified to traverse the list of SurfaceWindows in an order established by the revised objects. A sort of the WindowShadingControl objects will be made once based on the Shading Control Sequence Number during GetInput and not repeated during timestep simulation.

Testing will be done by comparing the results on a timestep basis with develop for buildings with multiple window shades.



