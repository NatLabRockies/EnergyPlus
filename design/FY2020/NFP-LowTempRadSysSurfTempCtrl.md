Low Temperature Radiant System Surface Temperature Control
================

**Rick Strand, University of Illinois at Urbana-Champaign**

 - Original Date: April 29, 2020
 - Latest Revision Date: May 11, 2020 (Version 4, Design Document)
 - Minor edit/correction of type: May 25, 2020 (Version 5)


## Justification for New Feature ##

Based on research that has been done in the past, the Center for the Built Environment at the University of California at Berkeley has identified some necessary improvements to the EnergyPlus low temperature radiant system models.  These improvements would allow for more appropriate modeling of those systems in practice and would bring EnergyPlus up to date on current best practices in this area.  There are two groups of implementation requests and this document describes the first series of changes which all generally relate to how a low temperature radiant system is controlled.

## E-mail and Conference Call Conclusions ##

External Communications: To date, the author has had contact with researchers at the CBE both via email and through a 2 hour Zoom meeting.  As a result of the emails and the Zoom meeting, the author of this NFP created a “wishlist” document that summarized the request from CBE, the current status of the EnergyPlus code in relation to the CBE request, and an analysis of the potential implementation of the request in EnergyPlus (potential approach, priority, and estimated level of effort).  Some of the information in this NFP is based on that “wishlist” document.  That document was sent to the researchers from the CBE for their review, and the response was their support of the priorities and the approach as written.

## Overview ##

The first portion of this first series of changes relates to the options for controlling a radiant system.  Currently in EnergyPlus, the user has the option to control the system based on the zone MAT, zone MRT, zone Operative Temperature, the outdoor dry-bulb temperature, and the outdoor wet-bulb temperature.  Many systems in practice, particularly ones that have a significant amount of mass/concrete, tend to use surface temperature control.  This can mean that either the actual surface of the radiant slab is used to control the system or that the temperature at some depth is used as the control parameter.  Both of these surface control types are to be implemented with this work.

The second portion of this first series of changes relates to the throttling limits for controls.  At the current time in EnergyPlus, there is a minimum throttling range of 0.5C.  This was done to avoid potential instability that was assumed would happen if the system bounced back and forth between full on and full off operation.  However, there are some systems in practice which actually do operate like a two-position valve.  Thus, it would be helpful to model such a system in EnergyPlus as well.

The third and final portion of this first series of changes relates to how EnergyPlus uses the setpoint temperature.  Currently, the setpoint temperature is defined as the point at which the system is running at 50% of full flow for a variable flow.  This has led to some confusion since current industry standards use this setpoint as the point at which there is no flow.  This involves changing how this setpoint is interpreted inside EnergyPlus for the variable flow radiant system only.  The constant flow and the electric radiant systems already comply with the industry standard.

## Approach ##

For simplicity in development, these three items may be separated into three separate issues in GitHub and Pivotal Tracker.

For the changes relating to surface temperature control, the approach here will be to add surface control by using either the inside surface face temperature or a temperature at some depth rather than the other temperature options.  From there, the different radiant system models will use that to provide appropriate control for the system.  As this is done, this portion of the control algorithm will be converted to a new subroutine since all three of the radiant system models use similar code to grab the appropriate temperature for control.

The approach for dealing with the temperature internal to the radiant slab will be solved by using existing input.  In the Construction:InternalSource input, the user already has the possibility to request a temperature at a particular location.  This information will now also be used for control when the user picks the slab (interior) temperature as the controlling parameter.  In addition, when the user requests a 2-D simulation of the radiant system, the temperature inside the slab will be changed.  Currently, in the 2-D solution, the temperature is taken at the depth requested by the user but in line with the “source” point.  The temperature used for control in practice is more aligned with the mid-point between tubing.  However, given that users could desire to control on a temperature anywhere between the tubes and the mid-point, a new variable will be added to the input to give the user a chance to define the fractional distance between the tubing and the mid-point between the tubing for the temperature.  While EnergyPlus, because of the grid system in the space space method, cannot get exactly to the correct position, it will pick the closest position based on the user input.  So, the calculation of that internal temperature during a 2-D solution will be changed so that the temperature is at the depth requested and where the user requests (via a new Construction:InternalSource parameter).

The approach to dealing with the on-off control is to relax the check on throttling range so that a throttling range can be less than 0.5 but no smaller than 0.0.  If the throttling range is less than 0.5, a warning message will be produced and the simulation allowed to continue.  There will have to be some additional code in the electric radiant system that avoids a divide by zero since in one place it divides by throttling range, but when the user picks a 0.0 throttling range, it can simply do a slightly modified sequence where the panel is either on or off which should be fairly straight forward.

The change for the variable flow radiant system regarding what point in the system the schedule defines will implement a new parameter that will offer two possibilities.  For this new setpoint interpretation parameter, the two options will be “ZeroFlow” and “HalfFlow”.  For the “ZeroFlow” option, the setpoint temperature established by the user will set the temperature at which there is no flow through the radiant system and the full throttling range is used to establish when the flow is at 100%.  For the “HalfFlow” option, the setpoint temperature schedule defines when the flow to the system is 50% of the maximum flow and the setpoint is the mid-point of the throttling range.  The ”HalfFlow” option is what EnergyPlus currently does and this will be used to transition older files up to the current version.  The “ZeroFlow” option is what is typically used in industry when controlling low temperature radiant systems.


## Testing/Validation/Data Sources ##

Testing will be done using unit tests as well as new/modified input files in the existing EnergyPlus series of test files.  The changes/additions will include at least one example for each of the new control types and also looking at using a 0.0 throttling range.  The setpoint temperature for variable flow radiant systems will be checked comparison EnergyPlus output before and after the change.  So, this will rely on existing EnergyPlus test files and modified versions of those files.

## Input Output Reference Documentation ##

For the new control options (surface face and surface internal), the text for variable flow radiant systems currently is:

1.34.7.1.7 Field: Temperature Control Type.  This field specifies along with the throttling range and setpoint schedules how the user wishes to control the hydronic radiant system. The temperature denoted in the setpoint schedule can refer to one of five different temperatures: the zone mean air temperature, the zone mean radiant temperature, the zone operative temperature, the outdoor dry-bulb temperature, or the outdoor wet-bulb temperature. The choice of temperature is controlled by the current field—temperature control type. The user must selectfrom the following options:

* MeanAirTemperature
* MeanRadiantTemperature
* OperativeTemperature
* OutdoorDryBulbTemperature
* OutdoorWetBulbTemperature

Operative temperature for radiant system controls is the average of Mean Air Temperature and Mean Radiant Temperature. If the user does not select a control type, MeanAirTemperature control is assumed by EnergyPlus. See the throttling range and control temperature schedule fields below for more information.

The list will be expanded to include the options SurfaceFaceTemperature and SurfaceInteriorTemperature.  In the last paragraph after the description of the Operative temperature, new text will be added to the effect:

The SurfaceFaceTemperature option allows the user to control the radiant system using the inside face surface temperature of the radiant system (the inside surface temperature).  The SurfaceInteriorTemperature option will allow the user to control the radiant system using a surface temperature that is calculated inside the radiant system.  This point will be defined by the Construction:InternalSource description for the system.  In that input, the user has the option to calculate a temperature at a particular point in the construction.  The radiant system will then use this information for controlling the slab not just producing temperatures for outputting at that point.  Users should consult the input for Field: Temperature Calculation Requested After Layer Number for more information.

Similar changes need to be made in the constant flow and electric radiant systems.

Note that the text for the Field: Temperature Calculation Requested After Layer Number in Construction:InternalSource will also be modified to reflect this additional interpretation of this input field and the change to its interpretation for 2-D solutions.  Here is the proposed additional text:

“In addition, this field is also used by the radiant system model when the user selects the SurfaceInterior control method.  In this control type, the user is controlling the system based on a temperature on the interior of the slab.  This field then also sets this point inside the slab for this control.  Note also that when this field is used in conjunction with a 2-D solution (see next input field) that the interior temperature being calculated (and then used for control) will be calculated at the depth specified and at the mid-point between the tubing (not at the horizontal point in-line with the tubing).”

After the section in Construction:InternalSource entitled “Field: Tube Spacing”, a new field will be added: “Field: Horizontal Location of Control Temperature”.  Here is proposed text for this new section:

“This field defines the location of a control temperature point within the slab in the direction that is horizontal to the main direction of heat transfer.  This is a dimensionless parameter between 0.0 and 1.0.  It is interpreted as the distance perpendicular to the main direction of heat transfer and as a fraction of the total distance between the tubing where water is circulated and the mid-point between the neighboring piping.  So, if the value here is 0.0, then the control point is at the tubing.  If the value for this parameter is 1.0, then the control point is located at the mid-point between the tubing.  For any fraction between 0.0 and 1.0, EnergyPlus will locate the control point as close as possible to the user requested position.  For more information on this parameter and the 2-D solution for radiant systems in EnergyPlus, please refer to the Engineering Reference.”

For the new parameter in the variable flow radiant systems, a new field will be added at the end of the input description.  This new field will be called “Field: Setpoint Flow Interpretation”.  Here is proposed additional text for this new field:

“This input field defines how the setpoint schedules relate to the flow rate of the system.  The two options are ‘ZeroFlow’ and ‘HalfFlow’.  When the user selects ‘ZeroFlow’, the temperature setpoint schedule is used to define when the system is a zero flow.  So, for example, when this parameter is ‘ZeroFlow’ and the current setpoint schedule for cooling is 24C, then the cooling flow rate when the control parameter is at or below 24C is zero.  Once the controlled temperature goes above 24C, the flow rate will throttling from 0% to 100% when the temperature reaches the throttling range temperature above 24C.  If this parameter is ‘HalfFlow’, the temperature setpoint schedule is used to define when the system is at half flow.  So, for example when the parameter is ‘HalfFlow’, the current setpoint schedule for cooling is 24C, and the throttling range is 2C, when the control parameter is at 24C, the flow rate for the system will be set to 50% of the maximum flow.  The flow will vary from 0% at half the throttling range below the setpoint value (or 23C in this example) to 100% at half the throttling range above the setpoint value (or 25C in this example).  A similar interpretation is used for heating.”

## Input Description ##

Changes and additions need to be made to existing descriptions (see previous section).

## Outputs Description ##

No new input is being proposed so no changes need to be made to the output descriptions except clarification of the existing output (Surface Internal User Specified Location Temperature) similar to what is mentioned above for the input.

## Engineering Reference ##

A variety of changes will need to be made to the Engineering Reference in the section on Low Temperature Radiant System Controls.  The new control options will need to be defined and the changes to the throttling range and its interpretation will also need to be made.  In addition, documentation for the 2-D modeling capabilities will be added to the Engineering Reference as currently there isn’t anything here describing this.  When this is added, a discussion of how the new user position for internal temperature is decided.

## Example File and Transition Changes ##

New and modified example files will be created to test the new controls that are implemented in this work.  A transition will need to be made for files that use the Construction:InternalSource input since a new parameter is being added and this parameter will NOT be at the end of the existing input.  A default value for this new parameter will be 0.0 as that is what is currently done in EnergyPlus.  A transition will also be needed for the new parameter used to define the controls for the variable flow radiant system.  The transition will take existing variable flow radiant system definitions and add the new parameter with “HalfFlow” as the input.

## References ##

Google document authored by CBE (their own internal document)
Current EnergyPlus code and documentation
RadiantControlsWorkWishlistWithEvaluation.docx (author’s internal document, submitted to CBE and NREL and available upon request)



