Addition of Pipe Wall and Pipe to Surface Contact Resistance to the Low Temperature Radiant
================

**Rick Strand, UIUC**

 - Original Date: July 2, 2020
 - Revision Date


## Justification for New Feature ##

Based on research that has been done in the past, the Center for the Built Environment at the University of California at Berkeley has identified some necessary improvements to the EnergyPlus low temperature radiant system models. These improvements would allow for more appropriate modeling of those systems in practice and would bring EnergyPlus up to date on current best practices in this area.  One of those areas of improvement in the EnergyPlus low temperature radiant system model is the characterization of heat transfer between the fluid being circulated through the system and the slab itself.  Currently, only the convection between fluid and the slab itself is taken into account.  When the model was created, it was assumed that the convection was the most important part and because the slab itself is very significant that other factors were not critical.  However, ISO 11855-2 does provide a method to for calculating resistance through the pipe wall and also contact resistance between the pipe and the slab.  It is uncertain as to how much of an impact this has on overall heat transfer.  However, in general, experience has shown that the convection itself provides very little resistance to heat transfer and the slab temperature is often nearly identical to the fluid outlet temperature.  So, implementing an improvement here would likely improve the accuracy of the model within EnergyPlus and bring the model in line with ISO.

## E-mail and  Conference Call Conclusions ##

External Communications: To date, the author has had contact with researchers at the CBE both via email and through a 2 hour Zoom meeting. As a result of the emails and the Zoom meeting, the author of this NFP created a “wishlist” document that summarized the request from CBE, the current status of the EnergyPlus code in relation to the CBE request, and an analysis of the potential implementation of the request in EnergyPlus (potential approach, priority, and estimated level of effort). Some of the information in this NFP is based on that “wishlist” document. That document was sent to the researchers from the CBE for their review, and the response was their support of the priorities and the approach as written.

## Overview ##

Overall, the task here is to implement aspects of ISO 11855-2 in the variable and constant flow low temperature radiant system models.  Using the information from the standard, the modeling of heat transfer between the fluid and the slab will be improved.  Specifically, the resistance in the pipe wall and the contact resistance between the pipe and the slab will be added to the current model.

## Approach ##

The current method for modeling heat transfer between the fluid in the piping and the radiant system is handled in a single subroutine and uses the effectiveness/NTU method for heat exchangers with the assumption that the slab is at a single temperature at the location of the piping.  In the existing code, the convection between the fluid in the piping and the slab is modeled using the Colburn equation, which is a relationship for convective heat transfer when a fluid flows inside a pipe.  This will be compared to the recommendations in ISO 118855-2.  If the ISO standard recommends a different equation, then that will be implemented if it is determined that it is better and meets the other assumptions of the existing radiant model.  In addition, the recommendations from ISO 11855-2 for heat transfer through the pipe material and the contact resistance will also be added to the resistance from the convective heat transfer to provide a combined resistance/heat transfer coefficient that includes all three of these effects.  For the piping, the user will be asked to specify the conductivity of the piping material and the inner and outer diameter of the piping.  Defaults for these parameters will be based on ISO 11855-2 for PEX piping, the most common material used in the radiant industry currently, and the pipe diameter parameters will be based on standard 1/2” PEX piping.  Note that the inner piping diameter is already part of the object definition in EnergyPlus so only two new parameters are needed.

## Testing/Validation/Data Sources ##

Standard unit test(s) will be used to verify that the components of the improved model are being evaluated properly.  Data for defaults will be based on ISO-11855-2.  Existing input files will be modified to use the default parameters.

## Input Output Reference Documentation ##

Additions will be made to the Input Output Reference for the three new model parameters.  Examples of how this will be integrated into the existing documentation as shown below.  Only new input will be needed.  No new output reporting will be necessary.  Note that the inside diameter already exists in EnergyPlus—the information below is simply a modification of the existing information.

## Input Description ##

\paragraph{Field: Hydronic Tubing Inside Diameter}\label{field-hydronic-tubing-inside-diameter-000}

This field is the inside diameter of the tubes through which water is circulated for the system being defined by this statement. The inside diameter should be recorded in meters and is used to determine the convective heat transfer from the water to the inside surface of the hydronic tubing as well as the thickness of the pipe for conduction through the pipe wall.  The default value for this parameter is the value for common 1/2” piping or 0.012m.

\paragraph{Field: Hydronic Tubing Outside Diameter}\label{field-hydronic-tubing-outside-diameter-000}

This field is the outside diameter of the tubes through which water is circulated for the system being defined by this statement. The outside diameter should be recorded in meters and is used to determine the contact resistance between the pipe and the slab as well as the thickness of the pipe for conduction through the pipe wall.  The default value for this parameter is the value for common 1/2” piping or 0.016m.

\paragraph{Field: Hydronic Tubing Conductivity}\label{field-hydronic-tubing-conductivity-000}

This field is the conductivity of the piping material used in the radiant system in W/(m-K).  The default value for this parameter is 0.35 which is the approximation for most commonly used piping material (PEX tubing).


## Outputs Description ##

No new output descriptions are necessary for this work.

## Engineering Reference ##

A section will be added to the Engineering Reference to further explain the equations used in the updated model.  Currently in the ER, there is a section that describes the convection between the fluid and the pipe.  This will be expanded to include the new modeling equations that are taken from ISO 11855-2.

## Example File and Transition Changes ##

All existing input files that use either the variable or constant flow radiant systems will be enhanced to include the new input parameters.  Transition changes for the addition of these three new parameters will be added to the rule set for V9.4 and code will be added to the transition F90 code.

## References ##

Communications between the CBE and the author of this NFP are available upon request as is the summary of the features requested by the CBE and the status within EnergyPlus (wishlist).
ISO Standard 11855-2.



