DEFECT: Fix for Chiller Heater Always Assuming Evaporator is at Full Load
================

**Rick Strand, University of Illinois at Urbana-Champaign**

 - Original Date: July 22, 2024
 - Revision Date: July 30, 2024


## Justification for New Feature ##

The current heater mode portion of the chiller heater model in PlantCentralGSHP.cc is written with the built-in assumption that the evaporator is running at full load.  When the condenser load comes back at less than full load, the evaporator load is never adjusted and reports load and electric power at full load.  This is not correct and resulted in a defect being logged (Defect 10065).  This document is a plan for a potential solution to this problem.

## E-mail and Conference Call Conclusions ##

July 24, 2024: Discussed this on the technicalities call.  Decision was made to not implement an iteration strategy but to simply make an approximation of the PLR from the condenser load and then multiple full load evaporator load, compressor power, and false loading by that PLR.  Not ideal, but given all of the suspected problems in this model, it was decided to not invest too heavily in this now and turn this into a potential development topic in the future.

## Overview ##

The heat pump model for the ChillerHeater model is contained in the PlantCentralGSHP.cc file.  The model allows for various modes of operation: off (0), cooling only (1), heating only (2), heat recovery (3), cooling dominant (4), and heating dominant (5).  Off mode is obvious--no loads, nothing happening.  When in cooling or heating only mode, the heat rejection is lost/sent to the other thermal environment.  When in heat recovery mode, heat rejection at the condenser is used for heating purposes in the HVAC system.  Cooling and heating dominant modes have heat recovery but there is more heat recovery than is needed so the excess is rejected to whatever the outside environment is.  The cooling controlled modes (1, 3, and 4) are simulated using the CalcChillerModel in CentralPlantGSHP.cc.  The heating controlled modes (2 and 5) are simulated using the CalcChillerHeaterModel in CentralPlantGSHP.cc.  The cooling controlled modes seem to be working without any known issues.  The heating modes run the condenser to the correct load for heating.  However, a user noticed that there was an issue with the reported evaporator load and power consumption which always seemed to be relatively constant at a high level whenever the condenser was needed for a heating load.  This was traced back to the assumptions in the heater portion of the chiller heater model.

The simlation flow in the heater portion of the model is summarized as follows.  Once it is identified that the chiller is in one of the heating modes, the chiller is assumed to run at full capacity to get an evaporator load, a compressor power consumption, and any false load for when the chiller is below the minimum part load ratio.  A condenser load at full power is calculated from this information.  This condenser load is then adjusted to fit the actual heating need.  From this condenser load, flows or temperatures are adjusted for this load as needed on the condenser side.  The simulation then moves on from there.

The problem here is that the evaporator load and the compressor power are still at full load and are never adjusted when the condenser load gets reduced because the heating load does not require full load.  This is the source of the error--evaporator load and compressor power never change in heating mode regardless of the actual part load ratio based on the condenser load.  PLR simply stays at near 100%.  This is not correct and leads to over-estimation of both the evaporator load and the compressor power consumption.

## Original Approach ##

Note: before the actual fix takes place, it was decided to make a code improvement pass through the current chiller heater model.  This has already taken place and has been merged into develop.  The point was to make the code re-usable within the chiller heater model but it also realized some improvements in the cooling mode subroutine as well.  The changes took several different code sections and turned them into smaller subroutines.  The heating mode code is now much easier to follow, reducing the size of the routine by a factor of more than 3 (based on printouts of the routine before and after restructuring).  The real benefit will be seen when the problem is fixed as the algorithm should stay fairly compact and easy to follow (hopefully).

The approach to this problem is to for the most part leave the initial pass at full load in tact.  The code still needs to know what the condenser is capable of from a heating standpoint.  If the required heating load (HeatingLoadToMeet) is larger than the condenser load at full power, then there is no change to be made.  However, when the condenser load is reduced because the HeatingLoadToMeet is less than the condenser load at full power, there needs to be additional code to handle this case.

When the QCondenser is initially calculated based on full load evaporator conditions, a new variable (qCondOrig or something like that) tracking the original value will be set equal to this value.  After QCondenser is compared to HeatingLoadToMeet and then potentially modified by the new restructured subroutine adjustChillerHeaterFlowTemp, QCondenser will be compared to qCondOrig.  If QCondenser is lower than qCondOrig, then this means that we are no longer at full load and need to go into an adjustment procedure.

Adjustment Procedure: The adjustment process to obtain a revised/appropriate evaporator load and compressor power will take place in the code after the initial pass to adjust the chiller heater flow rate and temperature.  The code in CalcChillerHeaterModel, after restructuring, looks like this:

                if (CurrentMode == 2 || this->SimulHtgDominant) {
                    if (CondMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance && CondDeltaTemp > 0.0) {
                        this->adjustChillerHeaterFlowTemp(state, QCondenser, CondMassFlowRate, CondOutletTemp, CondInletTemp, CondDeltaTemp);
                    } else {
                        QCondenser = 0.0;
                        CondOutletTemp = CondInletTemp;
                    }
                }

New code will be added right after the call to adjustChillerHeaterFlowTemp.  There will need to be an iteration loop as there is no guarantee that one pass will obtain the correct evaporator conditions.  The iteration loop will end after 20 attempts or when the latest condenser load is sufficiently similar to the condenser load from the previous iteration.

Before the iteration loop, set iteration number to zero and old condenser load to full load (qCondOrig) and new condenser load to existing condenser load.

During the iteration loop (while iteration is less than 20 and difference between old and new QCondenser is outside of tolerance), the following functions will be done:

Step 0: Update last iteration condenser load and iteration number

Step 1: Calculate PLR based on comparison of last iteration condenser load to full condenser load, limit to MaxPartLoadRatio.

Step 2: Calculate QEvaporator and EvapOutletTemp based on new PartLoadRatio.  Could potentially be turned into a new subroutine.

Step 3: Call checkEvapOutletTemp. This in all likelihood won't do anything because it just makes sure that the minimum evaporator temperature limits are not violated, but it still needs to be run just in case.

Step 4: Call calcPLRAndCyclingRatio.  This shouldn't change PLR because it was just used to calculate a new QEvaporator.  It could, however, result in some false loading if PLR drops below the minimum.

Step 5: Recalculate CHPower, the compressor power using the same equation used from previously in this routine.  Could potentially be turned into a new function (one line).

Step 6: Recalculate ActualCOP.  Could Could potentially be turned into a new function.

Step 7: Calculate the new QCondenser for this iteration.  Reuse existing code that limits this based on minPLR, HeatingLoadToMeet.

Step 8: Call adjustChillerHeaterFlowTemp to adjust flow rate and temperature if necessary.

At this point, a new QCondenser has been calculated so the iteration cycle is done.  No additional code is needed after the iteration cycle as it should just be able to pick up where it left off as it currently does.

## Modified Approach ##

During the technicalities call, it was suggested that rather than iterating, we should just approximate the PLR from the condenser load and then multiply evaporator load and compressor power by this PLR.  The false load was also factored in this way though in this case it was probably zero at full load anyway.  Other problems in the algorithm were also fixed along the way.  No guarantees that this model is now 100% bug free but it should be improved.

## Testing/Validation/Data Sources ##

Testing will be done using the existing user input file that shows the problem.  Comparisons will be made between develop and the new version to establish that the results have changed after the fix has been implemented and that the new output makes sense.

Unit testing: as this is being handled as part of a bug fix, at least two unit tests will be generated.  As there are several new subroutines, the unit tests will likely center on these new subroutines.

## Input Output Reference Documentation ##

No changes needed--this is an algorithm fix that does not require input changes.

## Input Description ##

No changes needed--this is an algorithm fix that does not require input changes.

## Outputs Description ##

No changes needed--this is an algorithm fix that does not require input changes or new output.

## Engineering Reference ##

Currently, the Engineering Reference has a section for the chiller heater model (ChillerHeaterPerformance\:Electric:EIR).  The subsection entitled "Heating-only mode and Simultaneous cooling-heating mode" essentially outlines the calculation process in heating only (2) and heating dominant (5) modes.  Additional text will be added to the end of this section to describe the work implemented as part of this fix that will outline the steps in a similar fashion to what is shown above in the Approach section of this document.

## Example File and Transition Changes ##

No transition changes are needed since there is no change to the input.  A change to an existing input file that has a chiller heater equipment may be needed to show differences in output, but it likely will without any changes to the .idf.  So, there are no changes anticipated to existing example files either.

## References ##

Current code in PlantCentralGSHP.cc



