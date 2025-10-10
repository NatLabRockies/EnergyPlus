Implement Optional Time Delay Between Mode Changeover for Low Temperature Radiant Systems (#8057)
================

**Rick Strand, UIUC**

 - Original Date: July 2, 2020
 - Revision Date: Version 2—July 2, 2020


## Justification for New Feature ##

Based on research that has been done in the past, the Center for the Built Environment at the University of California at Berkeley has identified some necessary improvements to the EnergyPlus low temperature radiant system models.  These improvements would allow for more appropriate modeling of those systems in practice and would bring EnergyPlus up to date on current best practices in this area.  One feature that is not currently possible is a changeover lock out that prevents the radiant system bouncing back and forth between heating and cooling in short periods of time.  This generally does not happen in actual systems and there are systems which prevent this by establishing switching back and forth for a certain period of time.  This work will bring the EnergyPlus model in line with what is typical in practice and users will be allowed to chose the length of the changeover in hours via a new input field in the objects of the variable and constant flow low temperature radiant systems.  Because the electric low temperature radiant system is heating only and thus does not cooling, it will not need to be modified.

The following is text from the CBE describing the request: “One of the most common problems with high thermal mass radiant systems is excessively frequently changeovers between heating and cooling operation due to poor controls. One [of] the common solutions is to implement a minimum time delay between switching from one mode to another (e.g. 24 hours). This could be a user-input value included with the controls for either model.”

## E-mail and  Conference Call Conclusions ##

External Communications: To date, the author has had contact with researchers at the CBE both via email and through a 2 hour Zoom meeting.  As a result of the emails and the Zoom meeting, the author of this NFP created a “wishlist” document that summarized the request from CBE, the current status of the EnergyPlus code in relation to the CBE request, and an analysis of the potential implementation of the request in EnergyPlus (potential approach, priority, and estimated level of effort).  Some of the information in this NFP is based on that “wishlist” document.  That document was sent to the researchers from the CBE for their review, and the response was their support of the priorities and the approach as written.


## Overview ##

Current controls in EnergyPlus look at what is happening at the present only and do not look back in time to see what has happened previously.  This would require additional checks and variables to track when the system was last in heating or cooling mode.  It would also require at least one additional input in the IDD to implement the length of time for locking out the opposite mode of operation.

## Approach ##

This work will implement a changeover “delay” that prevents the radiant system from bouncing from heating to cooling or vice versa for a certain period of time.  This changeover period or lockout will prevent this for a user specified period of time (in hours).  So, as the system operates, it will track what was the last operating mode and how long it has been since it was last in that mode.  If the length of time since it was last in whatever conditioning mode equals or exceeds the need to go into the opposite conditioning mode, then the algorithm will allow the changeover to the other conditioning mode.  However, if the length of time is less than the changeover delay, then it will simply keep the system off.

A new input parameter for both the variable and constant flow low temperature radiant systems will allow the user to set the delay between changeover in hours (integer) via a schedule of values, allowing the user to change this value as desired over time if necessary.  The code will read that value in and store it in the local data type for hydronic systems.  When the radiant system is simulated and checks as to whether the unit is scheduled on, it will now also check to see when the system last operated, which mode it was in, and compare that time difference to the delay parameter.  The new variables in the data structure are expected to include the delay parameter input by the user as well as time markers for last on time and the mode that the system was in.  Additional variables may be needed as the code work progresses.

In addition to the code to check whether the system is allowed to operate in a certain mode, there will need to be code that accurately reflects the mode that the radiant system ends up being in during a particular time step.  This will require some code in the “initialize” routine and some code in the “update” routine as well.

## Testing/Validation/Data Sources ##

While there may be validation sources, it seems best that the main goal here is to test that the new capability is actually working properly.  The best way to do this is to provide a new input file in the test suite that reflects the original statement from CBE—a thermally massive radiant system.  This system will then be given a design day that varies greatly in temperature so that the system might need both heating and cooling during a 24 hour period.  This would allow the changeover lock out to be validated as working properly.

## Input Output Reference Documentation ##

Descriptions for the new input parameter will need to show up in two locations: the variable flow low temperature radiant system and the constant flow low temperature radiant system.  In both cases, the new parameter will be added to the existing input object and will be interpreted the same way.  So, the Input Description shown below will be used for both system types.  In addition, a new output field will be added to chronicle what mode the system is in to make it easier to verify that the system is working properly with the delay parameter.

## Input Description ##

The following is the plan for making an addition to the input description for both system types in tex format.  This new parameter will be added to the end of the existing description.

\paragraph{Field: Changeover Delay Time Period Schedule Name}\label{changover-delay-time-period-schedule-name}

This parameter defines a schedule which should be populated with values for the amount of time in hours that is required before this low temperature radiant system can switch between either heating or cooling and the opposite conditioning mode.  If this schedule value is set to 1 (hour) at a particular time and the system was previously in heating mode, then it will be locked out from switching into cooling mode until at least one hour of time has passed in the simulation.  This helps avoid systems bouncing back and forth between the two conditioning modes in successive time steps.  This parameter can be set to any positive value, and a negative value will be assumed to have no delay (equivalent to a delay of zero).  If the user leaves this field blank, EnergyPlus will assume that no changeover delay is requested.

## Outputs Description ##

The following is the plan for making an addition to the output description for both system types in tex format.  This new output parameter will also be added to the list that starts every output description section in the Input Output Reference.  In tex format, this means the following will need to be added:

\item
    HVAC,Average,Zone Radiant HVAC Operation Mode {[}{]}

Also, the following will be added:

\paragraph{Zone Radiant HVAC Operation Mode {[}{]}}\label{zone-radiant-hvac-operation-mode}

This field reports the operating mode for a particular system over whatever time period is requested.  The value that is reported in the output can be interpreted as follows.  A zero value will indicate that the system was not running.  A value of +1 will indicate that the system was in heating mode.  A value of -1 will indicate that the system was in cooling mode.  At the system time step level, the value of this parameter should be one of these values.  However, at larger time steps due to the fact that this is an averaged output, this parameter could have a value of anything between -1 and +1.

## Engineering Reference ##

No changes to the Engineering Reference are proposed.

## Example File and Transition Changes ##

A new example file that shows the use of the new parameter as well as verifies that it is working properly will be added to the test suite.  The new parameter will be added at the end of the description.  While no transition changes are technically needed, the transition files (Rules and F90 code) will be updated to add the new parameter to the end of existing input files.

## References ##

Communications from CBE and the summary written by the author of this NFP are available upon request.



