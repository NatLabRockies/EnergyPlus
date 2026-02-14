Constant Flow Radiant System Control Temperature Setpoint Reset on Running Mean Outside Air Temperature
================

**Rick Strand, UIUC**

 - Original Date: June 8, 2020
 - Revision Date: none yet


## Justification for New Feature ##

The request for this new feature came from the Center for the Built Environment (CBE) at the University of California Berkeley based on their experience with radiant system research and how systems are implemented in practice.  Their request was initially summarized with the following text:

“Add an option…that resets [the] control temperature setpoint and/or supply water temperature setpoint (for ConstantFlow) [and]  is calculated as a function of running mean outside air temperature.”

In further conversations and email exchanges with CBE, the point of this control is that the running mean outside air temperature is the control temperature and then this is used to set the temperature of the water being sent to the radiant system.  In practice, this is done for constant flow systems but an equivalent strategy does not exist for variable flow systems or for electric systems.  So, the request here is to specifically add this capability for constant flow low temperature radiant systems.  CBE reports that this control is very popular particularly in Germany and Switzerland and has been shown in research to be a potentially more effective and stable method than controlling on conditions within the space.

## E-mail and  Conference Call Conclusions ##

There has been several exchanges of email between CBE and the author of this NFP regarding specifically what is being requested.  The following is a short summary of the conclusion of that email discussion:

The proposal here is to only modify the constant flow system which allows for the variation of the water supply temperature.  This type of control would not apply to the variable flow and electric radiant system models in EnergyPlus.

Your recommendation for the average outdoor temperature is to use what was Equation (2) [see below] in the snippet that you sent which points to the average outside temperature from the previous day and the running mean temperature from the previous day.  The alpha term in that equation should be an input value that the user sets.  Using this equation, a value that is used for the entire day will be calculated using this equation at the beginning of each day and held constant for the entire day.

The water temperature and control temperature schedules (four in all) are fine because they will allow the user to enter a linear profile between water temperature and the control temperature.

## Overview ##

The work associated with this NFP involves the implementatiom of a new control for the constant flow low temperature radiant system that looks at an average of the outdoor air temperature over a period of time based on an equation that will be shown below in the Approach section.  This time-average of the outdoor air temperature will then be used as the “control” temperature for the constant flow system as would any other type of control.  In practice, this temperature is used along with a linear equation to define the inlet water supply temperature to a radiant system.  This is actually exactly what is currently used for the constant flow system which through a group of schedule definitions that are part of the current input define a linear relationship between the control temperature and the water supply inlet temperature.  So, the work here is to implement this new control type and also keep track of the new control parameter: the running mean outdoor air temperature.

## Approach ##

The new control type will be implemented the same as all the other existing control types (mean air temperature, mean radiant temperature, operative temperature, outdoor dry-bulb temperature, outdoor wet-bulb temperature, surface face temperature, and surface internal temperature).  The new control will be called the running mean outdoor air temperature control.

The new control will require code modifications to the recently revised control algorithm.  At the moment, all three of the low temperature radiant system types have the same control options and thus use the same control routine.  With this work, the new running mean outdoor air temperature control is only being implemented in the constant flow radiant system.  As a result, something will need to be done in the routine that assigned the internal integer for the control type so that variable flow and electric systems cannot use this new control type.  While it could be implemented for both of these systems, in discussions with CBE, the use of a control that varies the inlet supply temperature doesn’t actually make sense for a system that is variable flow (with flow varied because this is the method for controlling system output) or a system that does not involve a fluid loop.  So, the only system that will use the new control is the constant flow system.  In the existing data structure for low temperature radiant systems, there is a variable that notes what the system type is (variable flow, constant flow, or electric).  Thus, this can be used with straightforward logic to allow only the constant flow system and not the other system types.  This new control type will set Control Temperature to the new variable that tracks the running mean outdoor air temperature.

A new variable that tracks the running mean outdoor air temperature will be added to the code for the low temperature radiant systems.  Because this variable will be the same for all zones since it is dependent on the outdoor air temperature only, there is no need to track this on a system by system basis.  In other words, every low temperature radiant system that uses this control will be referencing the same temperature and this new variable will not need to be tracked separately for each system.

The equation that will be used to track the running mean outdoor air temperature will be:

Theta(RM) = (1-alpha)*Theta(ED-1)+(alpha)Theta(RM-1)

where:

Theta(RM) is the running mean outdoor air temperature for today

Theta(RM-1) is the running mean outdoor air temperature from yesterday

Theta(ED-1) is the average outdoor air temperature from yesterday

alpha is a weighting coefficient that is typically set to 0.8 but will be a new user input that is added to the constant flow radiant system existing input (must be between 0.0 and 1.0).

Since Theta(RM-1) includes the effect of previous days’ outdoor air temperature, the impact of not just yesterday’s weather is taken into account but also previous days.

Within the code, new variables and a new subroutine(s) will be added to track the value of the running mean outside air temperature.  This will be launched from the initialization routine that already exists at the beginning of the day (to update the running mean outdoor air temperature and the average outdoor temperature from the previous day) and at the beginning of each hour (to update the hourly temperature that is used to calculate the average outdoor air temperature for the current day).

In order to test this new algorithm and make sure that it is working properly, new output variables will also be added so that the system response can be compared to the running mean outdoor air temperature.

## Testing/Validation/Data Sources ##

Testing of the new subroutines will happen via unit test(s).  In addition, a new IDF will be added to the test suite to demonstrate that the new control algorithm is functioning properly.

## Input Output Reference Documentation ##

Because a new control type is being added, the existing documentation for the Field: Temperature Control Type will need to be enhanced to show the new control type.  This means that the current list of control types will need to be increased for the new control type.  So, the term “RunningMeanOutdoorAirTemperature” will be added to the existing list.  In addition, some text will need to be added at the end of the parameter immediately after the list of control types.  The proposed text to be added is:

“The \emph{RunningMeanOutdoorAirTemperature} option will allow the user to control the inlet water temperature to the system as a function of the running mean outdoor air temperature.  The next field (\hyperref[field-running-mean-outdoor-air-temperature-weighting-factor]{Running Mean Outdoor Air Temperature Weighting Factor}) will define how this temperature is actually calculated within EnergyPlus.”

This will also require a new field for the constant flow low temperature radiant system.  This field description will be incorporated into the existing documentation and inserted after the existing field called Temperature Control Type. A sample of the next documentation is shown here (everything through the end of this subsection):

\paragraph{Field: Running Mean Outdoor Air Temperature Weighting Factor}\label{field-running-mean-outdoor-air-temperature-weighting-factor}

This field specifies the weighting factor that is used to calculate the running mean outdoor air temperature.  The running mean outdoor air temperature is determined using the following equation:

\begin{equation}
(\Theta)_rm = (1 - \alpha)(\Theta)_(ed-1) + (\alpha)(\Theta)_(rm-1)
\label{eq:RunningMeanOutdoorAirTemperatureEquation}
\end{equation}

where:

(\Theta)_rm is the current running mean outdoor air temperature that is used to control the radiant system

\((\Theta)_(ed-1)) is the average outdoor air temperature from the previous day

\((\Theta)_(rm-1)) is the running mean outdoor air temperature from the previous day

\(\alpha\) is the user defined weighting factor that is defined by this field.  It controls the weighting of the running mean outdoor air temperature from the previous day and the average outdoor air temperature from the previous day for the purposes of calculating the running mean outdoor air temperature for the current day.  The value for this weighting factor must be between zero and 1.


## Input Description ##

The addition to the input output reference are shown above.  Within the IDD, this will look as follows (again added after the Temperature Control Type field that already exists within the IDD for constant flow low temperature radiant systems):

N3, \field Running Mean Outdoor Air Temperature Weighting Factor
    \note this is the weighting factor in the equation that calculate the running mean outdoor air temperature as a weighted average of the previous day’s running mean outdoor air temperature and the previous day’s average outdoor air temperature
    \note this value is only used by EnergyPlus when the user elects to use the RunningMeanOutdoorAirTemperature control type
    \minimum 0.0
    \maximum 1.0
    \default 0.8

## Outputs Description ##

Three output variables will be added so that the calculation of the running mean temperature can be checked and also so that the system response can be verified.  These three output variables will be added at the end of the existing outputs for the constant flow radiant system and will be:

\item
  HVAC,Average,Constant Flow Running Mean Outdoor Air Temperature {[}C{]}
\item
  HVAC,Average,Constant Flow Previous Day Running Mean Outdoor Air Temperature {[}C{]}
\item
  HVAC,Average,Constant Flow Previous Day Average Outdoor Air Temperature {[}C{]}

Then in the portion of this subsection that describes these additions, the following lines will be added (through the end of this subsection):

\paragraph{Constant Flow Running Mean Outdoor Air Temperature {[}C{]}}\label{constant-flow-running-mean-outdoor-air-temperature-1}

This field reports the current running mean outdoor air temperature in Celsius. This value is used to control the constant flow low temperature radiant system when the user opts to use the \emph{RunningMeanOutdoorAirTemperature} control type.

\paragraph{Constant Flow Previous Day Running Mean Outdoor Air Temperature {[}C{]}}\label{constant-flow-previous-day-running-mean-outdoor-air-temperature}

This field reports the running mean outdoor air temperature of the previous day in Celsius. This value is used to calculate the running mean outdoor air temperature when the user opts to use the \emph{RunningMeanOutdoorAirTemperature} control type.

\paragraph{Constant Flow Previous Day Average Outdoor Air Temperature {[}C{]}}\label{constant-flow-previous-day-average-outdoor-air-temperature}

This field reports the average of the outdoor air temperature for the previous day in Celsius. This value is used to calculate the running mean outdoor air temperature when the user opts to use the \emph{RunningMeanOutdoorAirTemperature} control type.


## Engineering Reference ##

No changes needed as the equation used to calculate the running mean outdoor air temperature will be discussed in the Input Output Reference.

## Example File and Transition Changes ##

Various example files will have to be updated to reflect that there is a new input field in the middle of the syntax of the constant flow low temperature radiant system input.  A transition change will also be added for this as well.

## References ##

insert text
