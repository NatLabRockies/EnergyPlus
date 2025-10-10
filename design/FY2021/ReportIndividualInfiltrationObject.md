# Add ZoneInfiltration Output Variables for Individual objects(Issue #8209)
================

**Yueyue Zhou, NREL**

 - 25 June 2021


## Background ##

Excerpts lifted from [issue #8209](https://github.com/NREL/EnergyPlus/issues/8209)

 - There can be one infiltration object serving multiple zones, and there can be multiple infiltration objects serving the same zone. Currently the output variables only report at zone level, which is good for the first case. However, for the second case, there's no way to obtain, e.g., Sensible Heat Gain Energy specific to each object.
 - Some related discussion in [Add Space PR](https://github.com/NREL/EnergyPlus/pull/8394#discussion_r530585696):
	 - Dan Macumber: Is it possible to make infiltration or ventilation at the space level? That would simplify input for cases where ventilation requirements are a function of space type.
	 - Jason Degraw: I think it is possible, I think we probably just want to avoid having both (e.g. SpaceInfiltration _and_ ZoneInfiltration). Moving everything to the space level will probably be more work, but if that's the right solution then we should do that.
	 - Scott Horowitz: Whether it's SpaceInfiltration or ZoneInfiltration, it'd be nice to be able to get individual outputs for each object. With current EnergyPlus, you can have multiple ZoneInfiltration objects in a zone, but can only get aggregate outputs (i.e., the key for Zone Infiltration Sensible Heat Gain Energy is a zone object, not an infiltration object).
	 - Richard Raustad: Should probably have outputs for both space and zone to show individual as well as aggregated.
	 - Edwin Lee: I agree with this: limit to one input approach but output at both the space and zone level.


## E-mail and  Conference Call Conclusions ##

*None yet.*

## Overview ##

This work will implement code changes that calculate infiltration output variables at individual levels. Proposed list of variables to report is:

 - Infiltration Sensible Heat Loss Energy
 - Infiltration Sensible Heat Gain Energy
 - Infiltration Latent Heat Loss Energy
 - Infiltration Latent Heat Gain Energy
 - Infiltration Total Heat Loss Energy
 - Infiltration Total Heat Gain Energy
 - Infiltration Current Density Volume Flow Rate
 - Infiltration Standard Density Volume Flow Rate
 - Infiltration Current Density Volume
 - Infiltration Standard Density Volume
 - Infiltration Mass
 - Infiltration Mass Flow Rate
 - Infiltration Air Change Rate

Considering the mapping of multiple infiltration and multiple zone, it might require two-level reporting, eg:
1. **Infiltration1** is serving ThermalZoneList where **ThermalZone1** and **ThermalZone2** are included
2. **Infiltration2** is serving **ThermalZone1** only

Then we will want Sensible Heat Loss to be reported in the csv file with the format like (similar to how PEOPLE is reported):

 - ThermalZone1 Infiltration1: Infiltiration Sensible Heat Loss Energy
 - ThermalZone1 Infiltration2: Infiltiration Sensible Heat Loss Energy
 - ThermalZone2 Infiltration1: Infiltiration Sensible Heat Loss Energy .

The impacted infiltration objects are:
-   [ZoneInfiltration:DesignFlowRate](https://bigladdersoftware.com/epx/docs/9-4/input-output-reference/group-airflow.html#zoneinfiltrationdesignflowrate) (Primary)
-   [ZoneInfiltration:EffectiveLeakageArea](https://bigladdersoftware.com/epx/docs/9-4/input-output-reference/group-airflow.html#zoneinfiltrationeffectiveleakagearea) (Optional depends on implementation)
-   [ZoneInfiltration:FlowCoefficient](https://bigladdersoftware.com/epx/docs/9-4/input-output-reference/group-airflow.html#zoneinfiltrationflowcoefficient) (Optional depends on implementation)

## Approach ##

### Existing approach description ###

Below description takes Infiltiration Sensible Heat Loss as an example:
Current E+ reported variable is **ZnAirRpt(ZoneLoop).InfilHeatLoss**, it is calcualted at *HVACManager.cc* function *ReportAirHeatBalance* with a zone loop, the equation is:

    ZnAirRpt(ZoneLoop).InfilHeatLoss = 0.001 * state.dataHeatBalFanSys->MCPI(ZoneLoop) * (state.dataHeatBalFanSys->MAT(ZoneLoop) - Zone(ZoneLoop).OutDryBulbTemp) * TimeStepSys * DataGlobalConstants::SecInHour * 1000.0 * ADSCorrectionFactor;

where the **state.dataHeatBalFanSys->MCPI(ZoneLoop)** is the summed (infiltration mass flow * air specific heat) at zone level, it is calculated at *ZoneEquipmentManager.cc* in function *CalcAirFlowSimple* , see:


    In infiltration object loop (example of ZoneInfiltration:DesignFlowRate):
	    NZ = state.dataHeatBal->Infiltration(j).ZonePtr; // zone index
	    MCpI_temp = state.dataHeatBal->Infiltration(j).VolumeFlowRate * AirDensity * CpAir;
	    state.dataHeatBalFanSys->MCPI(NZ) += MCpI_temp;

And data initialization of state.dataHeatBal->Infiltration is in HeatBalanceAirManager.cc

### Proposed approach ###

The idea is to follow how`People` object is implemented to report both individual and zone level outputs.

The requested feature will be implemented by allocating new variable `state.dataHeatBal->Infiltration(NumInfil).HeatLoss`, etc. Since `state.dataHeatBal->Infiltration` already arranges and contains infiltration objects that only point to single zone (instead of zone or zone list), the total number of `state.dataHeatBal->Infiltration` should be equal to `state.dataHeatBal->TotInfiltration`. This structure will store calculated infiltration data at object level. It will be calculated at HVACManager.cc or ZoneEquipmentManager.cc or HeatBalanceAirManager.cc depending on detailed implementation.

New variables will be set up for Infiltration objects.

## Testing

No new example file needed. Assume it isn't supposed to be any diffs expected on ci testing. Local tests should verify that the new variables are able to be requested and reported correctly.

## Input Output Reference Documentation ##

Need to update the output descriptions for infiltration objects as reporting both at zone and individual object level.

## IDD Changes and Transition

None
