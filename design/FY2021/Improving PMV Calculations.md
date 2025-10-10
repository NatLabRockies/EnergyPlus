Improving PMV Calculations
================

**Xuan Luo, Tianzhen Hong**

**Lawrence Berkeley National Laboratory**

 - Original Date: Nove 28, 2020

## Justification for New Feature ##

ASHRAE Thermal Comfort Standard 55 has been evolving in recent years to encourage more sustainable building designs and operational practices. Current thermal comfort models in EnergyPlus do not reflect the changes added to latest ASHRAE Standard 55-2017, including providing cooling credit for elevated air velocity, and evaluating local thermal discomfort such as the ankle draft risk. Updating the calculations of existing thermal comfort parameters is crucial to support passive cooling technologies such as natural ventilation, ceiling fan and portable fan. It is noted that such PMV/PPD calculations are already implemented in the CBE Thermal Comfort tool for ASHRAE Standard 55, https://comfort.cbe.berkeley.edu/ and pythermalcomfort, https://pythermalcomfort.readthedocs.io/.

In this new feature, we propose to implement the following adjustments according to ASHRAE Standard 55-2017 in EnergyPlus to facilitate more accurate comfort modeling:

1) Add the cooling effect (CE) calculations, according to ASHRAE Standard 55-2017 NORMATIVE APPENDIX D.

2) Add a modified set of PMV/PPD calculations that consider the cooling effect of elevated air speed. Keep the original PMV/PPD calculations.

3) Calculate the percentage of thermally dissatisfied people with the ankle draft at 0.1 m above floor level.

This proposal does not cover the calculations of effective radiant field which measures the comfort impact of solar heat gain on occupants, according to ASHRAE Standard 55-2017 NORMATIVE APPENDIX C. One of the considerations is such calculations require detailed time-varying occupant locations and explicit definition of individual windows (location, size) of a space, which involve lots of inputs from users that are hard to determine or highly uncertain. This type of detailed calculations is better done through Python EMS or co-simulation with EnergyPlus.


## Overview ##

1. Cooling effect (CE) calculations

ASHRAE Standard 55-2017 Section 5.3 requires that the Elevated Air Speed Comfort Zone Method be used when average air speed V<sub>a</sub> is greater than 0.20 m/s.

The Standard Effective Temperature (SET) model shall be used to account for the cooling effect of air speeds greater than the 0.20 m/s. Specifically, for a given set of environmental and personal variables, including an elevated average air speed, an average air temperature t<sub>a</sub>, and a mean radiant temperature t<sub>r</sub>, the SET is first calculated. Then the average air speed V<sub>a</sub> is replaced by still air (0.1 m/s), and the average air temperature and radiant temperature are adjusted according to the cooling effect (CE). The CE of the elevated air speed is the value that, when subtracted equally from both the average air temperature and the mean radiant temperature, yields the same SET under still air as in the first SET calculation under elevated air speed.

The following is a formal description of this process. To define the CE, we assert that it satisfies the following:

SET(t<sub>a</sub>, t<sub>r</sub>, v<sub>elev</sub>, \*) = SET(t<sub>a</sub> - CE, t<sub>r</sub> - CE, v<sub>still</sub>, \*) &nbsp;&nbsp; Eq. (1)

where t<sub>a</sub> is the average air temperature, t<sub>r</sub> is the mean radiant temperature, v<sub>elev</sub> is the elevated average air speed, such that v<sub>elev</sub> > 0.1 m/s, still is the elevated average air speed (v<sub>still</sub> = 0.1 m/s). The other parameters (indoor air humidity, clothing, metabolic rate) that stay the same are denoted by “\*”.

We will modify the current PierceSET Two-Node method to calculate SET implemented in EnergyPlus to reflect the changes made in the ASHRAE 55- 2020 standard Apendix D4. We’ll calculate the CE using the modified PierceSET model.

2. Adjusted ASHRAE 55 PMV/PPD calculations

The current PMV/PPD calculations implemented in EnergyPlus are consistent with the algorithms described in ASHRAE 55 NORMATIVE APPENDIX B for still air speed. The Cooling Effect adjusted PMV/PPD for an environment with elevated average air speed is calculated using the adjusted average air temperature, the adjusted radiant temperature, and still air (0.1 m/s). The following are equations to calculate the adjusted PMV and PPD.

PMV<sub>adj</sub> = PMV(t<sub>a</sub> - CE, t<sub>r</sub> - CE, v<sub>still</sub>, \*) &nbsp;&nbsp; Eq. (2)

PPD<sub>adj</sub> = 100 - 95 \* exp(-0.03353 \* PMV<sub>adj</sub><sup>4</sup> - 0.2179 \* PMV<sub>adj</sub><sup>2</sup>) &nbsp;&nbsp; Eq. (3)

3. ASHRAE 55 PPD on draft at ankle level calculations

Draft is unwanted local cooling of the body caused by air movement. It is most prevalent when the whole-body thermal sensation is cool (below neutral). Draft sensation depends on air speed, air temperature, activity, and clothing. Sensitivity to draft is greatest where the skin is not covered by clothing, especially the head region comprising the head, neck, and shoulders and the leg region comprising the ankles, feet, and legs. Draft at the lower leg region may occur in the buildings conditioned by thermally stratified systems, such as displacement ventilation and underfloor air distribution, or with cold-dropping airflow along external walls and/or windows.

We will add another metric, namely the predicted percentage dissatisfied on draft at ankle level (PPD AD) to evaluate the ankle draft risk.This proposed model can evaluate PPD_AD as a function of PMV and air speed at ankle. The following is the equation to calculate PPD_AD.

PMV<sub>vr</sub> = PMV(t<sub>a</sub>, t<sub>r</sub>, v<sub>r</sub>, \*) &nbsp;&nbsp; Eq. (4)

PMV_AD = round(exp⁡(-2.58 + 3.05 * v<sub>ankle</sub> - 1.06 * PMV<sub>vr</sub>) / (1 + exp⁡(-2.58 + 3.05 * v<sub>ankle</sub> - 1.06 * PMV<sub>vr</sub>)), 1) &nbsp;&nbsp; Eq. (5)

Where v<sub>r</sub> is the relative air velocity caused by body movement, and v<sub>ankle</sub> is the air speed at 0.1 m above the floor.

This equation is only applicable for v<sub>r</sub> < 0.2 m/s. And the subject’s metabolic rate and clothing level should be kept below 1.3 met and 0.7 clo, respectively. Warnings would be thrown if these conditons are not met.


## Approach ##

We will add new keys, `CoolingEffectASH55` and `AnkleDraftASH55`, to the current `People` object, `Thermal Comfort Model N Type` field to reflect the adjusted thermal comfort calculations with considerations of the elevated air velocity cooling effect. We would also add a optional field `Ankle Level Air Velocity Schedule Name` to allow calculations of PPD_AD. When choosing the ASHRAE55 as the thermal comfort model, the still-air PMV/PPD and the original Pierce SET will be calculated, along with the adjusted cooling effect adjusted PMV/PPD. Both the original still-air and the adjusted thermal comfort metrics will be reported.

```
People,
   \memo Sets internal gains and contaminant rates for occupants in the zone.
   \memo If you use a ZoneList in the Zone or ZoneList name field then this definition applies
   \memo to all the zones in the ZoneList.
   \min-fields 10

   A14, \field Thermal Comfort Model 1 Type
       \type choice
       \key Fanger
       \key Pierce
       \key KSU
       \key AdaptiveASH55
       \key AdaptiveCEN15251
       \key CoolingEffectASH55
       \key AnkleDraftASH55

   A21, \field Ankle Level Air Velocity Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note units in the schedule are m/s
       \note this is the schedule of the air speed at the 0.1 m above the floor
       \note optional (only required for runs of thermal comfort models ASHRAE55 and PPD_AD variable is declared)
```

The following new variables will be added in the thermal comfort data structures and be set as reportable variables to be included in the output.

- ASHRAE55CoolingEffect
- CoolingEffectAdjustedPMV
- CoolingEffectAdjustedPPD
- ASHRAE55AnkleDraftPPD

## Testing/Validation/Data Source(s) ##

The PMV/PPD and SET results will be validated with the validation table in the ASHRAE Standard. Two example files (the DOE reference small office and the one zone uncontrolled model) will be modified to demonstrate the use of the new feature. Simulation results will be manually checked/benchmarked using excel spreadsheet with input and output from EnergyPlus runs.

## Input Output Reference Documentation ##

To be developed.

## Input Description ##

The `Thermal Comfort Model N Type` field in the `People` object will be modified to take two extra keys, `CoolingEffectASH55` and `AnkleDraftASH55`, as choices.

## Outputs Description ##
The following new report variables will be added:

- Zone, Average, Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect [°C]
- Zone, Average, Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect Adjusted PMV []
- Zone, Average, Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect Adjusted PPD []
- Zone, Average, Zone Thermal Comfort ASHRAE 55 Ankle Draft PPD []

## Engineering Reference ##

To be developed.

## Example Files and Transition Changes ##

The existing 1ZoneUncontrolled model will be modified to report the ASH55 thermal comfort calculations.

No transition change required.

## E-mail and  Conference Call Conclusions ##

From Stefano Schiavon <schiavon@berkeley.edu> on Dec 7, 2020, 12:03 PM:

1. Dr Edward A. ARENS is working on an addendum to reduce the still airspeeds from 0.2 to 0.1 m/s and that this was already implemented in the CBE thermal comfort tool. The addendum on elevated air speed that (I believe) has been voted through SSPC 55 in 2020 autumn, going now to public review. It will be a pity to have EnergyPlus using the old airspeed value. It may take many years before this change could be updated in E+. It makes sense to fix it now.

2. The code in ASHRAE 55 has been modified due to issues that were discovered in the SET calculation. Ed Arens lead those changes. I doubt that they have been documented somewhere. If I recall correctly, the changes were related to the autogenerated air speed by physical movement and some unit transformation that was incorrect.

Reply:

We will using the new airspeed value in E+ implementation. We'll modify the current SET code implementations in EnergyPlus to reflect the modifications in ASHRAE 55.


## Acknowledgments ##

N/A
