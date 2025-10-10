Control of Multi-Zone Radiant Systems
================

**Rick Strand, UIUC**

 - Original Date: June 9, 2020
 - Revision Date: none (original version)


## Justification for New Feature ##

The request for this new feature came from the Center for the Built Environment (CBE) at the University of California Berkeley based on their experience with radiant system research and how systems are implemented in practice.  Their request was initially summarized with the following text:

“In practice, it is common to configure radiant systems such that ‘controlled zones’ are much larger than the ‘controlled zones’ for a typical air system. These ‘controlled zones’ are often larger than the ‘thermal zones’ defined in building energy models, and may not align with the boundaries used for ‘thermal zones’.  Water flow to each ‘controlled zone’ is controlled based on temperature measurement (of some type) in one or more ‘thermal zone’, or in one or more ‘surface’.”
[So, the request is to] “allow a radiant ‘control zone’ to span multiple ‘thermal zones’; but they still require that each LowTempRadiant object be associated with a single ‘thermal zone’.  In practical terms, they can represent all of the parallel tubing loops downstream of a single control valve, but they do not allow supply-to-return tubing loops to cross between ‘thermal zones’.”

Currently, in EnergyPlus, there is not a way to have radiant system surfaces in different thermal zones.  In fact, the algorithm looks at the input for a particular system and then insists that all of the surfaces actually reside in the same zone as the one that is served by the radiant system.  This is done primarily due to the fact that when the model was written is was assumed that radiant systems would act based on being in the same thermal zone.  So, the checks that are made when the input is read are specifically done to make sure that a user has not made an error.  Since, however, there are some systems that operate in this fashion, having this ability in EnergyPlus would be helpful for some of the larger radiant systems.

It should be noted that along with this request by the CBE was an additional request to allow flow between different surfaces that are part of a single radiant system to be either parallel (as in currently the assumption) or in series.  As is noted in the Input Output Reference for the ZoneHVAC:LowTemperatureRadiant:SurfaceGroup object, it is possible to do some series flow in EnergyPlus provided that the user enters multiple radiant systems and carefully lays out the systems and the plant loop to have flow move from one surface to another.  There will need to be close coordination amongst these different radiant systems to get this to work properly.  As a result, this work will only deal with getting a multi-zone system to work in parallel.  However, in doing this, it should also open up series systems to much greater flexibility.

## E-mail and  Conference Call Conclusions ##

There have been exchanges of emails as well as a conference call discussion with researchers from the CBE to discuss the request and potential outcome of having surfaces for a single radiant system be present in more than a single thermal zone.

## Overview ##

This work will implement code changes that allow surfaces for a single radiant system to be present in more than one zone.  When the list of radiant surfaces includes surfaces from more than a single zone, a warning message will still be generated so that the user is aware of this situation and can then confirm that this is what was desired.

## Approach ##

In the subroutine GetLowTempRadiantSystem, each of the three low temperature radiant systems is read in and the data is transferred to the appropriate data type for each radiant system type.  Once the surface or surface list defined, EnergyPlus checks to make sure that the surface or all of the surfaces in the list belong to the same zone as the radiant system.  If any surface does not belong to the zone for the radiant system, this is currently denoted as a severe error and the program doesn’t run.

The proposal for this work is to change this severe error to a WarningMessage and to not set ErrorsFound to true.  Because of similarity of the process of finding whether the zone matches or not and some of the other checks, as much code as possible will be generalized to handle different tasks as a standalone subroutine.  In addition, there will need to be a unit test for any new subroutine(s) and at least one additional test suite file that has a group of surfaces that are not all from the same zone to test this new feature (testing will need to show that it can be done and also that the results produced make sense and are reasonable).

It should be noted that the zone for the radiant system will not have to match the zone for any of the surfaces that make up the radiant system.  However, if the user elects to use some sort of zone based control (MAT, MRT, or Operative Temperature), it will use the information from the zone specified as the zone in the radiant system input object.  If the user elects to use some sort of surface based control (like Surface Face Temperature or Surface Interior Temperature) that will still be based on the FIRST surface in the surface list.

## Testing/Validation/Data Sources ##

Any new subroutine(s) will have a unit test that will validated that the subroutine is functioning properly.  The relaxation of the zone criteria will be tested with at least one new input file and the results will verify that the surfaces are being controlled the same way despite being in different zones.

## Input Output Reference Documentation ##

In the ZoneHVAC:LowTemperatureRadiant:SurfaceGroup object, there is an existing field (Surface <x> Name) that has the following description:

This field is the name of a surface in the zone being conditioned by the radiant system. Base surfaces (e.g., \hyperref[buildingsurfacedetailed]{BuildingSurface:Detailed}), door surfaces and internal mass are valid. Window surfaces are not valid surface types for embedded radiant systems.

This will be changed to:

This field is the name of a surface being conditioned by the radiant system. It should be noted that the surfaces listed in this field do \emph{not} need to be present in the zone being served by the radiant system to which this surface is related. This allows radiant systems to span more than one zone and have surfaces controlled by conditions in another zone. When the user does select surfaces that are in different zones than the zone being served by the radiant system, a warning message is generated to alert the user that the surface is not in the same zone as the zone being served by the radiant system. If this is the intent of the user, this warning message can be ignored. Base surfaces (e.g., \hyperref[buildingsurfacedetailed]{BuildingSurface:Detailed}), door surfaces and internal mass are valid. Window surfaces are not valid surface types for embedded radiant systems.

## Input Description ##

See previous section.

## Outputs Description ##

No new output variables will be implemented with this work as existing variables are sufficient to gather information about the operation of such a radiant system.

## Engineering Reference ##

There is nothing special about the modeling of a radiant system where the surfaces are in different zones. Each surface receives flow as determined by the controls and the flow fraction defined in the surface list. So, in reality there is not any change to the methodology and thus there is no need for any changes in the Engineering Reference.

## Example File and Transition Changes ##

At least one new example file will be added to the test suite as mentioned above to demonstrate that this is functioning properly in EnergyPlus. No transition changes are needed since the IDD is not changing.

## References ##

Information received from CBE, the author’s own notes on the discussions with CBE, or any of the email exchanges that took place regarding this work is available upon request.
