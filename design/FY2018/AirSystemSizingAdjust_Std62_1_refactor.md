Air System Sizing Adjustments, Standard 62.1 VRP Refactor, and Autosizing Central Heating Flow Ratio
================

**Brent Griffith, Energy Archmage Company/SEI Associates/Trane**

 - 8/16/2017
 -

## Justification for New Feature ##

This project introduces a new method for adjusting air system design air flows that is more robust, improves the accuracy of ASHARE Standard 62.1 ventilation rate procedure (VRP) calculations, and allows autosizing the central heating maximum air flow ratio.  The new method accounts for details of the air terminal units attached to the system by making adjustments to the system sizing results after the zone equipment has been setup and sized.  Currently the system sizing calculations are completed before the zone equipment or air system equipment is read in and setup.  By adding routines that first setup the zone equipment and then make adjustments to the system sizing, information from the air terminal models for minimum flow fractions, operating modes during heating, and any user-defined hard sizes can be mined and used to adjust system sizing results to improve accuracy and robustness.

Air system design flow rates for autosized systems are currently based on the larger of the cooling and heating design air flows based on the ideal loads zone sizing calculations.  It is often the case that cooling design air flow is larger than the heating design air flow.  Once the air terminals are setup however, the actual operating flow rates during heating are often quite different from the design heating flow rate derived from ideal loads sizing.  The new method scans all the air terminals and develops accurate values for the minimum and maximum flow rates that will occur during heating operation.  Then the design heating flow rate is adjusted to reflect this new result.  For example in a VAV system with Normal action dampers that is cooling dominate, the heating design flow rate is no longer really related to the zone loads, but is the cooling design flow rate times the minimum flow fraction at the dampers. Similarly when heating loads dominate, the cooling design flow rate needs to be adjusted to account for how the terminals will really operate during cooling.

ASHRAE Standard 62.1 VRP calculations for sizing outdoor air intake (Vot) are challenging and critically important.  Users and interface developers need to be able to rely on EnergyPlus's VRP calculations for system Vot at being robust and accurate.  The system sizing adjustment routines, and other changes that are part of this project, enable fundamental improvements in the Standard 62.1 VRP calculations which have the following existing problems:
- disconnect between design air flow rates from sizing and final operation of the air terminals,
- inaccurate calculation of occupant diversity factor
- inability to handle hard-sized air terminal units in combination with autosized system level. Or a mixture of hard-sized terminals and autosized terminals.
- disconnect in timing of summary table reporting
- data held in subroutine variables and not retained

The input field called Central Heating Maximum System Air Flow Ratio in the Sizing:System object is difficult for users to obtain accurate values for input.  This project enables autosizing this field.  Mining data from the details of air terminal models, after they have been set up, allows the program to accurately calculate the ratio of system air flow during heating operation to main system design airflow.


## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

Code changes that adjust the system design flow rates, improve the Standard 62.1 VRP calculations, and allow autosizing Central Heating Maximum System Air Flow Ratio have been developed and are being offered as a contribution to the DOE version of EnergyPlus. A new approach adjusts system sizing results and stores data for VRP calculations by mining information from the air terminal units after they have been fully setup.   The existing VRP sizing calculations execute during zone and system sizing calculations and use the design cooling and heating air flow rates from ideal loads sizing.  These existing calculations are largely retained as a sort of first pass at the VRP calculations (at the zone level) and then the system sizing adjustment routines are used to improve the 62.1 VRP calculations as discussed in the next section. The existing Standard 62.1 tables are unchanged except for adding reporting of the environment and time of peak for both the cooling and heating peaks.


## Approach ##


The code changes consist of changes to existing VRP code in SimAirServingZones.cc, adds three new routines added to SizingManager.cc, and modifies air terminal models to store their air loop index.  Changes are primarily under the hood, except for one new field becoming autosizable.

SimAirServingZones.cc, modify routine InitAirLoops() -  move code from MixedAir.cc routine GetOAControllerInputs() into this routine to fill out AirLoopZoneInfo data structure earlier in the program flow.  This allows for using the data structure earlier in the system sizing calculations, before the air system equipment is setup.  This allows for cleaner code in Standard 62.1 calculations which more simply loop over all zones on an air loop and not use the confusing code constructs based on separate "Cooled Zones" and "Heating Zones."

SimAirServingZones.cc, modify routine SetUpSysSizingArrays() - Move Standard 62.1 variables out to DataSizing.hh.  Remove existing system population diversity calculations (which are not accurate), add call to new DetermineSystemPopulationDiversity() routine.  Add code to support new SysAirMinFlowRatWasAutoSized variable in system sizing data structure that stores input for if the central heating air flow ratio was set to autosize on input. Revise handling of VozClgByZone and VozHtgByZone terms so that they do not already include system population diversity factor, to properly follow definition of Voz in Standard 62.1. Remove summary table reporting, to be replaced by new cleaner code in new routine ManageSystemVentilationAdjustments in SizingManager.cc

SimAirServingZones.cc, modify routine UpdateSysSizing() - Move Standard 62.1 variables out to DataSizing.hh. Store state of new booleans sysSizeCoolingDominant and sysSizeHeatingDominant in FinalSysSizing data structure to know how the air system main design flow was determined. Remove summary table reporting, to be replaced by new cleaner code in new routine ManageSystemVentilationAdjustments in SizingManager.cc

DataSizing.cc/.hh add data variables for Standard 62.1 calculations, move variables buried in SimAirServingZones.cc subroutines and put into DataSizing.cc where the variables persist longer and can be accessed.

SizingManager.cc, new routine ManageSystemSizingAdjustments() - This routine adjusts system sizing outcomes based on how the zone air terminals finish out their sizing. The zone models are executed to trigger their sizing routines. Then the air terminal units data structures are scanned to sum design flow rates. Every air terminal connected to a particular air loop is summed for: 1) minimum heating flow rate, 2) maximum heating flow rate, 3) maximum flow rate, and 4) store zone level flow information for Standard 62.1 calculations, Vpz, Vpz_min, Vdz, and Vdz_min for both cooling and heating. The summed values are used to "Adjust" the system sizing results.  The corrected values are used to autosize the central heating flow ratio, if set to autosize by the user.

SizingManager.cc, new routine ManageSystemVentilationAdjustments() Redo Standard 62.1 VRP calculations using latest information on zone flows and report to tables.  Collect all summary table reporting one place with organized code.

SizingManager.cc, new routine DetermineSystemPopulationDiversity(). New routine to determine Pz sum, Ps, and D for each air system for Standard 62.1 calculations. Temporarily stop with system DoingSizing and do a side calculation that marches through all timesteps for each runperiod and sum up the concurrent max occupants in all the zones by air system.  If the input file has no RunPeriod objects, then use the design days.  This factors in all the scheduled detail for an accurate calculation of system population diversity, D.  Current code uses the schedule max for each people object but that cannot handle the coincident air system total unless all the schedules in the building have maximums at the same time of day.

Every one of the air terminal models is modified to store the air loop index in their local data structures.  The scanning done to mine data from the air terminals matches air loop index to find all the terminals attached to each air handler.  This generally involves storing the control zone index (CtrlZoneNum) during get input, if not already done, so that the air loop index (AirLoopNum) can be filled later on once the air loop has been setup (typically in a begin environment block in init routine).  This involves minor changes to SingleDuct.cc\.hh, PoweredInductionUnits.cc\.hh, HVACSingleDuctInduc.cc\.hh, HVACFoutPipeBeam.cc/.hh, AirTerminalUnit.hh, HVACCooledBeam.cc/.hh, DualDuct.cc/.hh, and DirectAirManager.cc/hh.

SimulationManager.cc is modified to execute calls to SizingManager::ManageSystemSizingAdjustments() and then SizingManager::ManageSystemVentilationAdjustments();  These calls are located just after when the current sizing routines are completed and before the main simulation is initialized (SetupSimulation())

OutputReportPredefined.cc is modified to include the name of the environment and time of peak for population peaks found during the new method for determining system population diversity into the the tables for standard 62.1 cooling and heating ventilation requirements.

## Testing/Validation/Data Sources ##

The new code has been incorporated in a derivative product which has been extensively tested by comparing to hand calculations.

## Input Output Reference Documentation ##

insert text

## Input Description ##

The N2 field in Sizing:System is made autosizable, and autosize used as default.

Sizing:System,

  N2, \field Central Heating Maximum System Air Flow Ratio
      \type real
      \minimum 0.0
      \maximum 1.0
      \default autosize
      \autosizable

## Outputs Description ##

No changes to data, but Standard 62.1 values will be corrected and changed in many places.

## Engineering Reference ##

Discussion of system sizing calculations in Engineering Reference will be modified to describe new system sizing adjustment method based on mining data from air terminal units after they are setup.

## Example File and Transition Changes ##

insert text

## References ##

insert text
