Allow Multiple Scheduled Shades to Reference a Single Window
================

**Jason Glazer, GARD Analytics**

 - May 12, 2020
 - May 29, 2020 - added Design Document portion and remove unused sections.


## Justification for New Feature ##

Two related requests to add flexiblity to window and shading have been made. The first is to allow modeling a window with
different shading coefficient during the summer and window. This is based on a requirement in "ANSI/RESNET/ICC 301-2014
Standard for the Calculation and Labeling of the Energy Performance of Low-Rise Residential Buildings" available here:

http://www.resnet.us/wp-content/uploads/archive/resblog/2016/01/ANSI-RESNET-ICC_301-2014-Second-Edition-Publish-Version.pdf

which requires (see page 14) that glazing should have an interior shade coefficient of 0.70 in the summer and 0.85 in the
winter.

When trying to implement this using multiple WindowShadingControl objects, each using a scheduled shading
control, to reference a single window now produces an error ("Fenestration surface named "FOOBAR" appears on
more than one WindowShadingControl list").

The second request is related to allowing multiple WindowMaterial:* objects to be used for a window. This would allow
EnergyPlus to model a window with, for example, both insect screens and interior shades. This requirement is prompted by
CBECC-Res and EnergyPlus Comparison: Report to the California Energy Commission which stated "Windows cannot be modeled
with multiple coverings (e.g., insect screens and interior drapes)."

The first request will be the focus of the effort, if effort remains after resolving the first request, the second request
will be also addressed.


## E-mail and  Conference Call Conclusions ##

No changes were based on the feedback from the reviewers of the intial NFP.

## Overview ##

Address the first request to allow multiple WindowShadingControl object to be used for a single window and not produce
an error. Error message that is currently produced "Fenestration surface named "FOOBAR" appears on
more than one WindowShadingControl list" will be removed to accommodate residential buildings where
interior shading is defined by summer and winter shading factors.

Also, where multiple WindowShadingControls each deploy the same shade on the same window(s) in a zone, no error
message will be generated and the shade will be deployed if any of the WindowShadingControls are activated.

## Approach ##

The error message is currently produced in SufaceGeometry::InitialAssociateWindowShadingControlFenestration and is due to
a single windows shading control pointer allowed for each Surface() in EnergyPlus. This will be changed to allow for a vector
of references and all code related to this will be modified to allow multiple window shading controls.

If the schedules enable two different WindowShadingControls to be active at the same time (scheduled or not), and
they deploy the same shading material or shading construction, then any control activation will prompt the
deployment of either the shading material or shaded construction. In other words, if multiple WindowShadingControls activate the
same shade, they will behave in an "OR" fashion where any active WindowShadingControl will deploy the shade.

The implementation will provide appropriate error checking for, e.g., warnings when two or more shading control schedules that
overlap, errors if they overlap with different materials, warnings if two shading controls that are not scheduled, etc.
Testing will also be conducted to see the difficulty if allowing WindowShadingControls to control shades in other zones that are
part of the same solar enclosure.

## Input Output Reference Documentation ##

No new fields or objects will be added but paragraphs will be added to WindowShadingControl describing this new capability.

## Example File and Transition Changes ##

No transition is needed. A new example file that demonstrates this capability will be added.

## References ##

"ANSI/RESNET/ICC 301-2014
Standard for the Calculation and Labeling of the Energy Performance of Low-Rise Residential Buildings"
http://www.resnet.us/wp-content/uploads/archive/resblog/2016/01/ANSI-RESNET-ICC_301-2014-Second-Edition-Publish-Version.pdf

## Design Document ##

To implement the features described above, the following will be done:

- Update SufaceGeometry::InitialAssociateWindowShadingControlFenestration() to remove the error message
- Update unit test InitialAssociateWindowShadingControlFenestration_test() or add new unit test for overlapping controls
- Modify the SurfaceData struct in DataSurfaces.hh to replace int WindowShadingControlPtr with std::vector \<int\> called
WindowShadingControlList and add ActiveWindowShadingControlPtr.

The following list shows functions that currently reference WindowShadingControlPtr. For most functions, the only
change will be to substitute ActiveWindowShadingControlPtr for places that WindowShadingControlPtr had been used previously.

- DaylightingManager::FigureDayltgCoeffsAtPointsSetupForWindow()
- DaylightingManager::GetDaylightingParametersInput()
- DaylightingManager::DayltgInteriorIllum()
- DaylightingManager::DayltgInterReflectedIllum()
- DaylightingManager::DayltgInteriorMapIllum()
- EMSManager::SetupWindowShadingControlActuators()
- SolarShading::AllocateModuleArrays()
- SolarShading::WindowShadingManager()
- SolarShading::ComputeWinShadeAbsorpFactors()
- SurfaceGeometry::GetSurfaceData()
- SurfaceGeometry::GetHTSubSurfaceData()
- SurfaceGeometry::GetRectSubSurfaces()
- SurfaceGeometry::CheckWindowShadingControlFrameDivider()
- SurfaceGeometry::MakeMirrorSurface()
- SurfaceGeometry::InitialAssociateWindowShadingControlFenestration()
- SurfaceGeometry::FinalAssociateWindowShadingControlFenestration()
- SurfaceGeometry::GetWindowGapAirflowControlData()
- SurfaceGeometry::AddWindow()
- WindowManager::W5InitGlassParameters()
- WindowManager::CalcWindowScreenProperties()

Plus unit tests in SurfaceGeometry.unit.cc and EMSManager.unit.cc.

For each timestep, the first active WindowShadingControl will be used and set as the ActiveWindowShadingControlPtr. A new
function will be added to make this decision and will probably be called from the first function listed above that is
called each timestep.

Additional changes may also be required to implement the feature.
