EXTENSION OF MRT CALCULATION OPTIONS BEYOND THERMAL COMFORT MODELS
================

Rick Strand, University of Illinois.

 - Original Date: June 23, 2026
 - Revision Date: June 29, 2026 (first update)
 - Final Version: July 9, 2026


## Justification for New Feature ##

When a user requests that EnergyPlus report the zone operative temperature and the zone thermal comfort operative temperature, there is potentially inconsistency between the two values.  The reason for this is that the thermal comfort models (or at least some of them) allow MRT (mean radiant temperature) to be calculated using different methods that the standard MRT calculation in EnergyPlus.  Since the operative temperature is approximated as the average of MRT and MAT (mean air temperature), any difference between the standard MRT calculation and what the thermal comfort models calculate as the MRT will show up as a difference in the operative temperature.

It should be noted that the thermal comfort models give the user the option to pick from options in the Mean Radiant Temperature Calculation Type field of the People object.  The three options are: enclosure averaged (standard MRT calculation used by the heat balance), surface weighted (where the MRT is the average of a particular surface temperature and the standard MRT--allowing say the person to be located very near a particular surface), and angle factor (where the user enters a series of factors for each surface that interacts radiantly with people in the space/zone).  When the thermal comfort models were first introduced into EnergyPlus, interior radiant exchange was governed by the MRT with RBAL method that has since been replaced).  Since in MRT with RBAL used the MRT as part of the solution, MRT calculatios within the thermal comfort models could not change the MRT calculated as part of the inside surface heat balance.  This is why the different MRT calculations were isolated within the thermal comfort routines--to have an impact on comfort predictions without impacting the heat balance).

Since the replacement of the MRT with RBAL radiant exchange method within EnergyPlus, MRT became more of a reporting parameter that is calculated after the heat balances are done (or at least an iteration).  MRT and Operative Temperature can be used in the control of some systems, but it will not impact the various surface heat balances.  So, this opens up the possibility that MRT could be calculated in general with this alternate methods beyond just the thermal comfort models.  This could "fix" the issue where there is a difference between the operative temperature and thermal comfort operative temperature and at the same time provide a new feature that allows the user to calculate these alternate MRT calculation methods and have it impact the operative temperature.

This is summarized in Issue 11392:

https://github.com/NatLabRockies/EnergyPlus/issues/11392

In summary, the desire is for the ability to evaluate alternate MRT calculations for other thermal comfort models or simply for any simulation whether it requests a thermal comfort model or not.  In addition, there is the desire to have all MRT and operative temperature outputs to agree.  The solution proposed below seeks to address these two issues.

## E-mail and Conference Call Conclusions ##

**EnergyPlus Technicalities Call, June 24, 2026**: As this was the first time it was discussed and it was only posted on Slack the previous day, there were not a lot of comments on this NFP.  General support of the goal of the NFP was noted from a few team members, though admittedly the number of call participants was lower than normal.  One comment from Scott Horowitz related to the issue of which MRT specification would take precedent.  The question Scott asked was: "would it be possible and useful to allow the MRT for the zone to be a combination of different MRT values?"  It would certainly be possible to do this.  Instead of the "Zone Controlling Mean Radiant Temperature" field that is added, one could see a "Zone Mean Radiant Temperature Fraction" field that is a real variable between 0.0 and 1.0.  In this case, the sum of all fractions could be between 0.0 and 1.0.  In the case where the sum was less than 1.0, the remainder of the contribution would be based on the standard MRT (area-emissivity weighted) calculation.  This would certainly add some additional flexibility.  How useful this would be is not fully known, but it seems like potential improvement on the idea that could add siginificant flexibility and still allow the core idea of the new feature to be met.  Additional text has been added below to describe this potential path forward.

**EnergyPlus Technicalities Call, July 8, 2026**: The revised version of this proposal was revisited with the team.  Scott Horowitz proposed a suggestion that to avoid having to look at multiple people statements for a zone that a new ZoneMRT input object could be defined to define the fractions that would contribute to the actual Zone MRT from the various comfort MRTs calculated for People statements.  In effect, this would then implement "Approach 1" but also add on the new input field that would control how the actual MRT is calculated from the various comfort MRT values.  More details on this are shown below under "Approach 3" sections.  In general, the team consented to moving forward with this work with the suggestion from Scott to implement the new input field as per "Approach 3".

## Overview ##

As stated in the justification above, there is a discrepancy between report variables and a desire to have any simulation have access to the alternate MRT calculations.  This would address the concerns noted in Issue 11392.

## Approach ##

### Approach 1: New Character Field Defining One People Statement MRT Calculation Method as the Determinant and New Input Object for Zone MRT Calculation

The original proposed solution to this is to leave the existing input alone to maintain the functionality of calculating different MRT values for different people statements.  However, to define which calculation is used for calculating the zone MRT, a new field will be added to the people statement that will allow the user to specify which people statement will determine how the zone MRT is calculated.  This would resolve both of the goals of consistency between the zone MRT and the thermal comfort MRT and providing a way to change MRT and thus operative temperature so that it impacts controls.  Routines like CalcRadTemp, CalcSurfaceWeightedMRT, CalcAngleFactorMRT, etc. would remain where they are currently located within ThermalComfort.cc and CalcRadTemp would be access both from within the thermal comfort routines as well as from the CalculateZoneMRT routine.  These steps would allow MRT at the zone level to be calculated in the same way as the thermal comfort routines.  While the zone MRT and the comfort MRT will not always be the same, it will be the same for the People statement that sets the method for calculating MRT.  In addition, it will allow this MRT to impact systems that are controlled via MRT or operative temperature.

### Approach 2: New Real Number Field Allowing a Combination of MRT Calculations

In this approach, the zone level MRT would be calculated using a combination of the different MRT values from all of the people statements defined for the zone. So, if say a zone had three different people statements, the user could define the zone MRT with weighting factors of 0.4 for the first people statement, 0.3 for the second people statement, and 0.2 from the third people statement. In this case, since the fractions are less than 1.0, the remainder of the average would come from the standard zone MRT calculation that is an area-emissivity weighted average of the surfaces within the zone. The new field would be limited between 0.0 and 1.0. So, if this field is 0.4, 0.3, and 0.2, respectively for the first, second, and third People statements of a zone and no other people statements have a non-zero value and since these three fractions only add up to 0.9, the remaining 0.1 fraction would come from the standard MRT calculation.  When the fractions to add up to 1.0, these fractions will be used directly without modification.  When the People statements have values that do not comply with the 0.0 to 1.0 range or the fractions add up to greater than 1.0, a warning message will be provided and everything will default back to the standard MRT calculation.  One disadvantage to this approach is that across People input syntax, there has to be some agreement--meaning that one People statement is slightly "dependent" on another one.  This would be a new "precedent", though technically one could also argue that Approach 1 also has this issue.

### Approach 3: Create a New Input Object to Define How Zone MRT is Calculated

In this approach, a new input object called ZoneMRTCalculation will be added to EnergyPlus.  The ZoneMRTCalculation object would consist of a zone name and then an expandable list of field pairs that consist of a People statement and a decimal fraction.  This would effectively implement Approach 2 but avoid the need for a new parameter inside the existing People statement which could stay the same.

The new field would have to verify that the People objects contained with the list are in fact connected to the Zone noted in the first field of the object.  In addition, the list would also have to verify that the sum of the fractions was between 0.0 and 1.0.  As with Approach 2, if the user does not input fractions that sum up to that range, a severe error would be produced to note that this was invalid and the MRT calculation would go back to the standard calculation.  If the sum was greater than or equal to 0.0 but less than 1.0, a warning would be produced noting that the remaining fraction would come from the standard MRT calculation.  The calculation itself would be as follows:

*ZoneMRT = (fracPeople1)(People1MRT) + (fracPeople2)(People2MRT) + ... + (1.0-sumFracAllPeople)stdMRT*

The existing routine that is called from the heat balance would then reference the calculation of the MRT that has been done by the thermal comfort routines and calculate the zone MRT based on the user input fractions for a particular zone.  Documentation would need to be updated so that the user is aware of the connection between the reported Zone MRT and the Thermal Comfort MRT values based on the user input for this statement.

The ZoneMRTCalculation object would itself be completely optional.  When it is not present, the calculation will revert back to the current, standard method.  With this approach, there will not need to be any transition and existing input files will still work as they currently do with no change in results.

## Testing/Validation/Data Sources ##

Testing will involve running existing input files with variations on People statements to trigger different MRT evaluations and comparing them to the comfort MRT output.  In addition, this will also be used to control at least one system or object in the input file to note the differences in the results.

## Input Output Reference Documentation ##

### Approach 1: New Character Field Defining One People Statement MRT Calculation Method as the Determinant

Since a new field in an existing object is being proposed, the following information would be added before the current section "1.14.2.1.13 Field: Mean Radiant Temperature Calculation Type" in the People object description of the IO Reference:

1.14.2.1.13 Field: Zone Controlling Mean Radiant Temperature

This field determines whether the MRT value calculated for this zone is determined using the information shown in the next parameter(s).  Since there could be multiple people objects for a single space and/or zone, there can only be one people statement that makes this selection.  The options for this field are: ZoneMRTCalculation or ComfortMRTCalculationOnly.  If this field is set to ZoneMRTCalculation, then this People statement information also defines how the zone MRT is calculated based on the information in the next field(s).  So, in effect, setting this field to ZoneMRTCalculation controls how MRT is calculated for the zone in question.  If this field is set to ComfortMRTCalculationOnly, then it will not calculate the zone MRT using the information that follows in the next field(s).  At most, one People statement per zone can set this field to ZoneMRTCalculation.  If no People statements use this option, then the zone MRT will be the standard MRT calculation or EnclosureAveraged as described in the next field.  If more than one People statement sets this field to ZoneMRTCalculation, a warning message will be produced and only the first People statement encountered with this setting will determine the calculation of zone MRT.

Following field section numbers will be adjusted to reflect the new field.

### Approach 2: New Real Number Field Allowing a Combination of MRT Calculations

Since a new field in an existing object is being proposed, the following information would be added before the current section "1.14.2.1.13 Field: Zone Mean Radiant Temperature Fraction" in the People object description of the IO Reference:

1.14.2.1.13 Field: Zone Mean Radiant Temperature Fraction

This field determines what fraction of the zone MRT is calculated from this people statement.  This value must be between 0.0 and 1.0.  When the value of this field is 1.0, then the zone MRT is based on the MRT calculation of this People statement only.  Values of this field between people statements for the same zone must add up to between 0.0 and 1.0.  When the values of this field for different people statements do not add up to 1.0, the remainder of the MRT average will come from the standard area-emissivity weighted zone MRT calculation. When this field does not comply with the 0.0 to 1.0 range or when the People statements for a particular zone have fractions that add up to greater than 1.0, a warning message will be provided and everything will default back to the standard MRT calculation.

Following field section numbers will be adjusted to reflect the new field.

### Approach 3: Create a New Input Object to Define How Zone MRT is Calculated

This involves a completely new input object.  So, a new section would be added and would probably fit best at the end of the "1.11 Group – Advanced Surface Concepts" section in the Input Output Reference.

#### 1.11.33 ZoneMRTCalculation

EnergyPlus has the ability to analyze various MRT calculations via the People {hyperlink here} statement. In the People input object, the user can select to have the standard MRT calculation impact the thermal comfort model or either a surface weighted or angle factor calculation {another hyperlink or two here} define the MRT for the thermal comfort model(s) chosen. Since it is possible to have multiple People statements per zone and each one can have a different MRT calculation, it might be desirable to have the zone MRT be based on one or more of these People statement defined MRT values. This would impact both the MRT of the zone as well as the operative temperature of the zone and thus would affect how systems that are controlled on either one of these temperatures to function.

This input object defines the relative weighting or contribution each people-based MRT contributes to the zone MRT. It contains a zone name that refers to the zone in question and then continues with pairs of parameters that reference a specific people statement and the relative weighting factor that each of these statemnts contributes to the zone MRT. This list is extensible and can be as short as a single pair.

The sum of all weighting factors entered through a single ZoneMRTCalculation instance must be between 0.0 and 1.0. Error messages will be produced when this principle is violated. If the sum of all of the weighting factors is less than 1.0, the remaining portion will be based on the standard zone MRT that is an air-emissivity weighted average of all of the surface temperatures in the zone. The equation for this calculation is as follows:

[insert weighting equations here}

This input fields for this object are defined below.

##### 1.11.33.1 Field: Zone Name

This field specifies the name of the zone for which the MRT calculation details are being defined.  This must be a valid zone name that is defined elsewhere in this input file.

##### 1.11.33.2 Field: Pair <#> People Statement Name

This field specifies the name of the People statement that will contribute to the zone MRT calculation.  In conjunction with the next parameter, it defines a fraction of the MRT calculation value for the zone named in the previous input.

##### 1.11.33.3 Field: Pair <#> MRT Weighting Factor

This decimal fraction defines the contribution of the MRT calculated for the People statement referenced by its input pair in the previous field to the overall zone MRT.  The value of this parameter must be between 0.0 and 1.0.  Note that the sum of all weighting factors for all People statements referenced in these input field pairs must be between 0.0 and 1.0.  When the sum of the weighting factors is less than 1.0, the remaining portion of the MRT calculation comes from the standard area-emissivity weighted calculation of MRT for the zone.

This object is extensible, so additional pairs of these last two fields of this type can be added to the end of this object. An example of this object is:

ZoneMRTCalculation,
  West Wing Zone,                   !- Zone Name
  West Wing Office People,          !- People 1 Name
  0.4,
  West Wing Service Center People,  !- People 2 Name
  0.3,
  West Wing Reception People,       !- People 3 Name

## Input Description ##

### Approach 1: New Character Field Defining One People Statement MRT Calculation Method as the Determinant

People,
   \memo Sets internal gains and contaminant rates for occupants in the zone.
   \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
   \memo then this definition applies to all applicable spaces, and each instance will
   \memo be named with the Space Name plus this Object Name.
   \min-fields 10
  A1 , \field Name
       \required-field
       \type alpha
       \reference PeopleNames
  A2 , \field Zone or ZoneList or Space or SpaceList Name
       \required-field
       \type object-list
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A3 , \field Number of People Schedule Name
       \required-field
       \type object-list
       \note units in schedule should be fraction applied to number of people (0.0 - 1.0)
       \object-list ScheduleNames
  A4 , \field Number of People Calculation Method
       \note The entered calculation method is used to create the maximum number of people
       \note for this set of attributes (i.e. sensible fraction, schedule, etc)
       \note Choices: People -- simply enter number of occupants.
       \note People per Floor Area -- enter the number to apply. Value * Floor Area = Number of people
       \note Floor Area per Person -- enter the number to apply. Floor Area / Value = Number of people
       \type choice
       \key People
       \key People/Area
       \key Area/Person
       \default People
  N1 , \field Number of People
       \type real
       \minimum 0
  N2 , \field People per Floor Area
       \type real
       \minimum 0
       \units person/m2
  N3 , \field Floor Area per Person
       \type real
       \minimum 0
       \units m2/person
  N4 , \field Fraction Radiant
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0.3
       \note This is radiant fraction of the sensible heat released by people in a zone. This value will be
       \note multiplied by the total sensible heat released by people yields the amount of long wavelength
       \note radiation gain from people in a zone. Default value is 0.30.
  N5,  \field Sensible Heat Fraction
       \note if input, overrides program calculated sensible/latent split
       \autocalculatable
       \default autocalculate
       \minimum 0.0
       \maximum 1.0
  A5 , \field Activity Level Schedule Name
       \required-field
       \note Note that W has to be converted to mets in TC routine
       \type object-list
       \note units in schedule are W/person
       \object-list ScheduleNames
  N6 , \field Carbon Dioxide Generation Rate
       \note CO2 generation rate per unit of activity level.
       \type real
       \units m3/s-W
       \default 3.82E-8
       \minimum 0.0
       \maximum 3.82E-7
       \note The default value is obtained from ASHRAE Std 62.1 at 0.0084 cfm/met/person over
       \note the general adult population.
  A6 , \field Enable ASHRAE 55 Comfort Warnings
       \type choice
       \key Yes
       \key No
       \default No
  A7 , \field Zone Controlling Mean Radiant Temperature
       \note optional
       \type choice
       \key ZoneMRTCalculation
       \key ComfortMRTCalculationOnly
       \default ComfortMRTCalculationOnly
  A8 , \field Mean Radiant Temperature Calculation Type
       \note optional
       \type choice
       \key EnclosureAveraged
       \key SurfaceWeighted
       \key AngleFactor
       \default EnclosureAveraged
  A9 , \field Surface Name/Angle Factor List Name
       \type object-list
       \object-list AllHeatTranAngFacNames
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)

{...following character fields will be renumbered...}

### Approach 2: New Real Number Field Allowing a Combination of MRT Calculations

People,
   \memo Sets internal gains and contaminant rates for occupants in the zone.
   \memo If a ZoneList, SpaceList, or a Zone comprised of more than one Space is specified
   \memo then this definition applies to all applicable spaces, and each instance will
   \memo be named with the Space Name plus this Object Name.
   \min-fields 10
  A1 , \field Name
       \required-field
       \type alpha
       \reference PeopleNames
  A2 , \field Zone or ZoneList or Space or SpaceList Name
       \required-field
       \type object-list
       \object-list ZoneAndZoneListNames
       \object-list SpaceAndSpaceListNames
  A3 , \field Number of People Schedule Name
       \required-field
       \type object-list
       \note units in schedule should be fraction applied to number of people (0.0 - 1.0)
       \object-list ScheduleNames
  A4 , \field Number of People Calculation Method
       \note The entered calculation method is used to create the maximum number of people
       \note for this set of attributes (i.e. sensible fraction, schedule, etc)
       \note Choices: People -- simply enter number of occupants.
       \note People per Floor Area -- enter the number to apply. Value * Floor Area = Number of people
       \note Floor Area per Person -- enter the number to apply. Floor Area / Value = Number of people
       \type choice
       \key People
       \key People/Area
       \key Area/Person
       \default People
  N1 , \field Number of People
       \type real
       \minimum 0
  N2 , \field People per Floor Area
       \type real
       \minimum 0
       \units person/m2
  N3 , \field Floor Area per Person
       \type real
       \minimum 0
       \units m2/person
  N4 , \field Fraction Radiant
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0.3
       \note This is radiant fraction of the sensible heat released by people in a zone. This value will be
       \note multiplied by the total sensible heat released by people yields the amount of long wavelength
       \note radiation gain from people in a zone. Default value is 0.30.
  N5,  \field Sensible Heat Fraction
       \note if input, overrides program calculated sensible/latent split
       \autocalculatable
       \default autocalculate
       \minimum 0.0
       \maximum 1.0
  A5 , \field Activity Level Schedule Name
       \required-field
       \note Note that W has to be converted to mets in TC routine
       \type object-list
       \note units in schedule are W/person
       \object-list ScheduleNames
  N6 , \field Carbon Dioxide Generation Rate
       \note CO2 generation rate per unit of activity level.
       \type real
       \units m3/s-W
       \default 3.82E-8
       \minimum 0.0
       \maximum 3.82E-7
       \note The default value is obtained from ASHRAE Std 62.1 at 0.0084 cfm/met/person over
       \note the general adult population.
  A6 , \field Enable ASHRAE 55 Comfort Warnings
       \type choice
       \key Yes
       \key No
       \default No
  A7 , \field Zone Controlling Mean Radiant Temperature
       \note optional
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0.0
  A8 , \field Zone Mean Radiant Temperature Fraction
       \note optional
       \type choice
       \key EnclosureAveraged
       \key SurfaceWeighted
       \key AngleFactor
       \default EnclosureAveraged
  A9 , \field Surface Name/Angle Factor List Name
       \type object-list
       \object-list AllHeatTranAngFacNames
       \note optional (only required for runs of thermal comfort models: Fanger, Pierce, KSU, CoolingEffectASH55 and AnkleDraftASH55)

{...following character fields will be renumbered...}

### Approach 3: Create a New Input Object to Define How Zone MRT is Calculated

This approach involves a new input object as shown below:

ZoneMRTCalculation,
       \memo Used to specify portion of zone MRT calculated from MRT in People statements
       \memo Note that the following angle factor fractions must sum up to between 0.0 and 1.0
       \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
       \memo All People statements listed must be in the same Zone.
  A1 , \field Zone Name
       \type alpha
       \reference ZoneNames
  A2 , \field People 1 Name
       \type object-list
       \object-list PeopleNames
       \begin-extensible
  N1 , \field MRT Weighting Factor 1
       \type real
       \minimum 0.0
       \maximum 1.0
  A3 , \field People 2 Name
       \type object-list
       \object-list PeopleNames
  N2 , \field MRT Weighting Factor 2
       \type real
       \minimum 0.0
       \maximum 1.0
{...and repeated for additional people statements...}


## Outputs Description ##

No new outputs will be added.  However, text will be added to the existing zone MRT and comfort MRT fields to clarify the impact of this new field and when to expect the two output values to be equal.

## Engineering Reference ##

No changes anticipated.  For Approach 3, the equation used to calculate the zone MRT will be included in the IO Reference.  So again, no Engineering Reference changes are anticipated.

## Example File and Transition Changes ##

If Approach 1 or 2 were followed, all existing files with People statements will be updated for the new field (all have a blank new field which will maintain the existing output).  A new output file will have at least one ZoneMRTCalculation field to show the MRT output is the same.  A transition will need to take place adding a new blank field into all People statements.

With Approach 3, existing files can stay the same and no transition changes will be needed.  At least one new input file will be created to show the implementation of this new feature.

## References ##

https://github.com/NatLabRockies/EnergyPlus/issues/11392
