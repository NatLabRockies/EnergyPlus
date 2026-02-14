Reheat Coil Sizing when there is No Central Heating Coil
================

**Fred Buhl, Rongpeng Zhang, LBNL**

 - Original Date: 10/12/2015
 - Revision Date: NA


## Justification for New Feature ##

The current reheat coil sizing approach in EnergyPlus is designed assuming there are central heating coils in the primary air systems. It does not apply for the system without central heating coils. This feature aims to improve it to handle various system configurations. Different sizing parameters will be selected depending on the existance of central heating coil, preheating coil, and outdoor air heat exchangers.


## E-mail and  Conference Call Conclusions ##

* Brent Griffith: Can't we change the value of TermUnitFinalZoneSizing().DesHeatCoilInTempTU for different system configurations, instead of CoilInTemp in terminal unit sizing?

Yes, it is a better solution to modify the value of TermUnitFinalZoneSizing().DesHeatCoilInTempTU which is at a root level. We didn't consider this solution in the original design because we thought this parameter may be used for other purposes in addition to the reheat coil sizing. But after a thorough check, we think it is fine to change its value.

## Overview ##


The reheat coil is currently sized using the design zone heating load plus the reheat load.

        CoilInTemp = TermUnitFinalZoneSizing( CurZoneEqNum ).DesHeatCoilInTempTU;
         .........
        DesZoneHeatLoad = CalcFinalZoneSizing( CurZoneEqNum
            ).DesHeatLoad * CalcFinalZoneSizing( CurZoneEqNum
            ).HeatSizingFactor;

*TermUnitFinalZoneSizing().DesHeatCoilInTempTU* is set to the user specified "System heating supply temperature" in the system sizing calculations. This means the user can change the reheat coil design reheat energy by simply changing the input field *Central Heating Design Supply Air Temperature* in *Sizing:System*. Thus the user can to some extent already take into account the lack of a central heating coil. However, most users have no way to realize this. So we will improve the program to automatically handle the issue.

The improved approach will first check the system configurations: (1) the existance of central heating coils (2) whether there are preheating coils or OA heat exchangers. Then it will make a new estimate of the reheat coil inlet temperature and humidity ratio, depending on the system configurations.


The improved approach will be implemented for the following systems with reheat coils:
-	AirTerminal:SingleDuct:ConstantVolume:Reheat
-	AirTerminal:SingleDuct:VAV:Reheat
-	AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat
-	AirTerminal:SingleDuct:SeriesPIU:Reheat
-	AirTerminal:SingleDuct:ParallelPIU:Reheat


## Approach ##

1. **Detect whether or not there is a central heating coil.**
This information is not currently detected or stored in *PrimaryAirSystems*, the main repository of central system (air loop) data. So a new flag called *CentralHeatCoilExists* will be added. At approximately line 1810 in *SimAirServingZones*, the presence of supply and return fans are detected and saved. This code can be imitated to detect central heating coils and the result stored in the *CentralHeatCoilExists* of *PrimaryAirSystems*.

2. **Detect whether or not there is a preheating coil or heat recovery.**
If the system has no central heating coils, we have to further check the existance of preheating coil and heat recovery. In the current codes, the presence of a preheat coil is already detected and saved in *PrimaryAirSystem().NumOAHeatCoils* by calling function *GetOASysNumHeatingCoils*. We can develop a similar function to check the existance of heat exchangers (heat recovery).

3. **Improve the calculation of reheat coil design load.**
After checking the system configurations, we will need to alter the reheat coil design load calculation wherever it is calculated. It is calculated and used to set the design hot water flow rate in the sizing routines in the terminal unit modules, i.e., *SingleDuct* and *PoweredInductionUnit*. It may also be calculated and used in *RequestSizing* routine. In each case the calculation will need to select different inlet air temperature/humidity ratio depending on the sytem configurations. More specifically:
- (1) If the central heating coil exists: *DesHeatCoilInTempTU* and *DesHeatCoilInHumRatTU* within *TermUnitFinalZoneSizing* array will be used.
- (2) If there is no central heating coils but there is preheating coil or OA heat-exchanger: a mixed air temperature/humidity ratio will be used. They can be calculated using the information from *FinalSysSizing*, including *DesOutAirVolFlow*, *DesHeatVolFlow*, *PreheatHumRat*, *HeatRetHumRat*.
- (3) If there is no central heating coils, preheating coil, or OA heat-exchanger: *HeatMixTemp* and *HeatMixHumRat* from *FinalSysSizing* will be used.

4. **Improve the autosizing of reheat coil inlet air temperature and humidity ratio.**
Inlet reheat coil air temperature and humidity ratio are autosized and we will need to alter these quantities as well. All cases of these calculation are now centralized in *RequestSizing*.


## Testing/Validation/Data Sources ##

Unit tests and example files will be developed to test the feature.


## Input Output Reference Documentation ##

There is no updates on the IO reference.


## Input Description ##

There is no adjustment of the IDD.


## Outputs Description ##

NA


## Engineering Reference ##

To be developed.


## Example File and Transition Changes ##

NA


## References ##

NA
