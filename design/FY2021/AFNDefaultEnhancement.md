Airflow Network Default Behavior Enhancements
================

**Lixing Gu, Florida Solar Energy Center**

 - 1/21/21, Add Design Document
 - 01/04/21, Original Date
 - Revision Date


## Justification for New Feature ##

Currently in EnergyPlus when no Airflow Network (AFN) simulation control object is present all AFN objects are semi-silently ignored. The Subcontractor shall modify the model to better handle this situation. Some options that shall be considered and discussed with NREL are, for example, defaulting all fields of the simulation control object, or generate higher level warnings regarding the unused objects.

The above justification is copied from GitHub Issue #8010 as AirflowNetwork objects not used without AFN:SimControl object and no error/warning message #8010

## E-mail and  Conference Call Conclusions ##

1/17/21

I received a single comment from Jason DeGraw:

This makes sense to me.

Action by Gu: Go to the next step to write Design Document

## Overview ##

Although the issue provides two possible options, I would like to propose the logic as show below:

If (NoExist AirflowNetwork:SimulationControl) && (Exist AirflowNetwork:MultiZone:Zone & AirflowNetwork:MultiZone:Surface) Then

	Default all fields of AirflowNetwork::SimulationControl and issue a warning to make the default
	Let the program catches other possible errors

Else

	Bypass, because eio provides information on AirflowNetwork Model:Control,NoMultizoneOrDistribution

End If

The justification is that the objects of AirflowNetwork:MultiZone:Zone and AirflowNetwork:MultiZone:Surface are two main objects for AFN model to work. If both exist, the default of AirflowNetwork:SimulationControl makes sense. Otherwise, it assumes no AFN simulations. Therefore, there is no need to generate any warnings. In addition, the eio provide information below:

	! <AirflowNetwork Model:Control>, No Multizone or Distribution/Multizone with Distribution/Multizone without Distribution/Multizone with Distribution only during Fan Operation
	AirflowNetwork Model:Control,NoMultizoneOrDistribution


## Approach ##

When default conditions are satisfied, the follwoing inputs are provided below:

  	AirflowNetwork:SimulationControl,
    AFNDefaultControl,          !- Name
    MultizoneWithoutDistribution,  !- AirflowNetwork Control
    SurfaceAverageCalculation,  !- Wind Pressure Coefficient Type
    OpeningHeight,           !- Height Selection for Local Wind Pressure Calculation
    LOWRISE,                 !- Building Type
    500,                     !- Maximum Number of Iterations {dimensionless}
    ZeroNodePressures,       !- Initialization Type
    1.0E-05,                 !- Relative Airflow Convergence Tolerance {dimensionless}
    1.0E-06,                 !- Absolute Airflow Convergence Tolerance {kg/s}
    -0.5,                    !- Convergence Acceleration Limit {dimensionless}
    0.0,                     !- Azimuth Angle of Long Axis of Building {deg}
    1.0;                     !- Ratio of Building Width Along Short Axis to Width Along Long Axis


## Testing/Validation/Data Sources ##

An example will be provided for testing to ensure defualt inputs are used if the default conditions are satisfied.

## Input Output Reference Documentation ##

The default inputs are provide in the IO Ref.

## Input Description ##

No changes.

## Outputs Description ##

N/A

## Engineering Reference ##
N/A

## Example File and Transition Changes ##

No transition

## References ##

insert text

## Design Document ##

The new feature will revise a single module: AirflowNetworkBalanceManager.


### AirflowNetworkBalanceManager ###

The module revision includes the GetAirflowNetworkInput fuction and a new variable as bool AFNDefaultControlFlag

####AFNDefaultControlFlag####

The new variable at the module level is added to indicate if the default control is assigned or not. The defaul value of the flag is false. If this is true, the section to read input values is bypassed.

This variable will be reset in the clear_state function.

####GetAirflowNetworkInput####

The section, shown below, in the GetAirflowNetworkInput fuction will be revised.

#####Current section#####

        // *** Read AirflowNetwork simulation parameters
        CurrentModuleObject = "AirflowNetwork:SimulationControl";
        state.dataAirflowNetworkBalanceManager->NumAirflowNetwork = inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.dataAirflowNetworkBalanceManager->NumAirflowNetwork == 0) {
            SimulateAirflowNetwork = AirflowNetworkControlSimple;
            print(state.files.eio, Format_110);
            print(state.files.eio, Format_120, "NoMultizoneOrDistribution");
            return;
        }

#####Proposed revision#####

        // *** Read AirflowNetwork simulation parameters
        CurrentModuleObject = "AirflowNetwork:SimulationControl";
        state.dataAirflowNetworkBalanceManager->NumAirflowNetwork = inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.dataAirflowNetworkBalanceManager->NumAirflowNetwork == 0) {
<span style="color:red">

			if (AirflowNetworkNumOfZones >= 1 && AirflowNetworkNumOfSurfaces >= 2) {
				AFNDefaultControlFlag = true
				Set default values for the AirflowNetwork:SimulationControl object
</span>

			} else {
            	SimulateAirflowNetwork = AirflowNetworkControlSimple;
            	print(state.files.eio, Format_110);
            	print(state.files.eio, Format_120, "NoMultizoneOrDistribution");
            	return;
			}
        }
<span style="color:red">

		if (AFNDefaultControlFlag) {
			Bypass the section to read the object field values
		}

</span>

In order to get a network, the minimum value of AirflowNetworkNumOfZones is 1, the minumum value of AirflowNetworkNumOfSurfaces should be 2.