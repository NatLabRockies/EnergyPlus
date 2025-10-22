**SUPERCEDED BY NFP-ProvideAdditionalLEEDrelatedFeatures.md**


Update LEED Summary Report
================

**Jason Glazer, GARD Analytics**

 - May 24, 2017


## Justification for New Feature ##

The current Leadership in Energy and Environmental Design (LEED) Summary Report available in EnergyPlus helps users working with LEED forms. However, the report functionality has not been updated since it was first implemented in 2006. An update of the Summary Report functionality would help LEED users and Green Building Council Certification (GBCI) reviewers.

## E-mail and  Conference Call Conclusions ##

No team call yet.

## Overview ##

While reviewing this NFP, please also look at the following file which is submitted when using LEEDv4 to fully understand desired output for LEED Summary report:

- v4_Minimum Energy Performance Calculator_v04.xlsm

the file is available at:

http://www.usgbc.org/resources/minimum-energy-performance-calculator

The scope of work for this NFP is to enhance the EnergyPlus LEED Summary Report functionality:

 - To provide more information as required by LEED submittals.
 - With details as required by the U.S. Green Building Council (USGBC), including the information needed for the Energy Performance Calculator and Table 1.4 spreadsheets.

In addition, the current outputs from the LEED Summary Report shall be revised to be consistent with the requirements of the USGBC.


## Approach ##

The main approach used was to reach out to stakeholders and to review the Minimum Energy Performance Calculator spreadsheet. This approach resulted in more feature requests than could be performed within the budget. Below are suggestions from stakeholders and an indication if they are included in this NFP.

Summary of a call with USGBC representatives:

- Update the performance results including new end uses (included).
- Old EAp2-1 style labels are not longer used. Should just use names of tabs and subtable names (included).
- Gathering results now scattered through out current tabular results file more valuable to LEED users that deriving new outputs for other tabs of Minimum Energy Performance Calculator calculator (included).

- Importing. Best option would be for utility to automatically populate Minimum Energy Performance Calculator spreadsheet based on EnergyPlus inputs and outputs (EnergyPlus or OpenStudio team would maintain the custom spreadsheet). Second best option would be make importing results as simple as possible (USGBC would maintain spreadsheet and importing formulas). A special CSV would make this more repeatable. (not included)
- Flag zones that are signficantly different maximum heating and cooling loads for Appendix G3.1.1c (not included).
- Economizer use status (not included).
- Peak operating vs. design capacity and timestamp of peak (not included).
- When is optimum start used (not included).

In addition to the USGBC call, numerous email exchanges with various users and stakeholders suggest the following changes:

 - Demand for each end use should be the peak demand for the individual end use (included).
 - New custom end uses will be attached to end use subcategory user input so that users can categorize (included).

 - Allow for import of EnergyPlus results to Minimum Energy Performance Calculator spreadsheet (not included).
 - Enhance the AppGPostProcess software to compare the proposed with the four rotated buildings and report the savings and LEED points (not included).
 - Aggregate the areas based on program type for use in the space by space lighting and equipment power density summaries. Add a space program type field into the zone object to allow for such aggregation of results (not included).
 - Add an airside equipment summary for each air handling unit instead of results across coils and fans (not included).
 - Add fenestration assembly u-value and SHGC including edge and frame effects (not included).
 - Add end use subcategory fields to objects that do not currently have them (not included).
 - Add check for simultaneous heating and cooling (not included).
 - Add check for number of hours where zone ventilation requirements not met (not included).
 - Add check for when target setpoint is not met for water use equipment (not included).
 - Add estimate for monthly and annual surface heat gains and losses (not included).

The Minimum Energy Performance Calculator spreadsheet is being updated as part of the pilot to support Appendix G from 90.1-2016. It may include other end-use subcategories and those will be included if possible.

Requested features from were evaluated. More may be included if budget allows.

Several issues related to the LEED reporting may be addressed independently prior to the release of EnergyPlus 8.8.0 including:

https://github.com/NREL/EnergyPlus/issues/5672 EAp2-4/5. Demand data taken from Design Days instead of from Annual Peak
https://github.com/NREL/EnergyPlus/issues/5559 Add LEED reporting of peak end use and EFLH
https://github.com/NREL/EnergyPlus/issues/5895 LEED Summary EAp2-4/5 Report Problems
https://github.com/NREL/EnergyPlus/issues/5613 LEED Tables service water heating doesn't appear
https://github.com/NREL/EnergyPlus/issues/5956 Report VAV minimum volume setpoint
https://github.com/NREL/EnergyPlus/issues/5298 Outdoor Air Summary report improvements


## Testing/Validation/Data Sources ##

Compare outputs manually.

## Input Output Reference Documentation ##

Update the IORef to correspond with the IDD changes shown in the next section.

Also need to update the Using EnergyPlus for Compliance documentation.

## Input Description ##

For the following objects which already have End-Use Subcategory fields:

 - Lights
 - ElectricEquipment
 - GasEquipment
 - HotWaterEquipment
 - SteamEquipment
 - OtherEquipment
 - ElectricEquipment:ITE:AirCooled
 - ZoneBaseboard:OutdoorTemperatureControlled
 - Exterior:Lights
 - Exterior:FuelEquipment
 - Exterior:WaterEquipment
 - Fan:SystemModel
 - Fan:ConstantVolume
 - Fan:VariableVolume
 - Fan:OnOff
 - Fan:ZoneExhaust
 - Fan:ComponentModel
 - WaterHeater:Stratified
 - EnergyManagementSystem:MeteredOutputVariable
 - Refrigeration:CompressorRack
 - Refrigeration:Condenser:AirCooled
 - Refrigeration:Condenser:EvaporativeCooled
 - Refrigeration:Condenser:WaterCooled
 - Refrigeration:GasCooler:AirCooled
 - Refrigeration:Compressor
 - Refrigeration:System
 - Refrigeration:TranscriticalSystem
 - Refrigeration:SecondarySystem
 - WaterUse:Equipment

And add End-Use Subcategory fields for other selected objects that should have them:

 - Boiler:HotWater
 - Boiler:Steam
 - Chiller:Electric:EIR
 - Chiller:Electric:ReformulatedEIR
 - Chiller:Electric
 - CoolingTower:SingleSpeed
 - CoolingTower:TwoSpeed
 - WaterHeater:Mixed

Change the following or similar field:

```
  A5 , \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \note The following special tags will also place the end-use in specific rows in the LEED Summary table
       \note EAp2-4/5. Performance Rating Method Compliance:
       \note Fans-Parking Garage, Interior Lighting-Process, Cooking, Industrial Process, Elevators and Escalators.
       \type alpha
       \retaincase
       \default General
```

To:

```
  A5 , \field End-Use Subcategory
       \note Used to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \note Some of the end-use uses (prior to General) are used specifically in the LEED summary table.
       \note When using an end use not on this list select Other and define the end use in the next field.
       \type choice
       \key FansParkingGarage
       \key FansKitchenVentilation
       \key InteriorLightingProcess
       \key LightingInApartments
       \key Cooking
       \key IndustrialProcess
       \key ElevatorsEscalators
       \key ITequipment
       \key RefrigerationEquipmentUnregulated
       \key RefrigerationEquipmentRegulated
       \key BuildingTransformers
       \key General
       \key ITECPU
       \key ITEFans
       \key ITEUPS
       \key Computers
       \key OfficeEquipment
       \key GroundsLights
       \key FacadeLighting
       \key ParkingLighting
       \key AccentLighting
       \key TaskLights
       \key ClothesWashing
       \key ClothesDrying
       \key ZoneExhaustFans
       \key FumeHoods
       \key Bathrooms
       \key Dishwashing
       \key Showers
       \key BathroomHandwashing
       \key KitchenSink
       \key LightFoodPrep
       \key RainAndWellWater
       \key Other
       \default General

  A6 , \field Other End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase


```

Note that some of the \keys would only be present in appropriate objects (the water related ones for example).



## Outputs Description ##

The revised LEED Summary report is shown below:

<p>Report:<b> LEED Summary</b></p>
<p>For:<b> Entire Facility</b></p>
<p>Timestamp: <b>2017-05-17
    08:43:51</b></p>
<b>General Information - General</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Data</td>
  </tr>
  <tr>
    <td align="right">Conditioned building area [m2]</td>
    <td align="right">      927.20</td>
  </tr>
  <tr>
    <td align="right">Unconditioned building area [m2]</td>
    <td align="right">      30.00</td>
  </tr>
  <tr>
    <td align="right">Total building area [m2]</td>
    <td align="right">      957.20</td>
  </tr>
  <tr>
    <td align="right">Principal Heating Source</td>
    <td align="right">Natural Gas</td>
  </tr>
</table>

<b>General Information - Energy Model Information</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Data</td>
  </tr>
  <tr>
    <td align="right">Cooling Degree Days</td>
    <td align="right">1743</td>
  </tr>
  <tr>
    <td align="right">Heating Degree Days</td>
    <td align="right">3430</td>
  </tr>
  <tr>
    <td align="right">HDD and CDD data source</td>
    <td align="right">Weather File Stat</td>
  </tr>
  <tr>
    <td align="right">Climate Zone</td>
    <td align="right">5A</td>
  </tr>
  <tr>
    <td align="right">Weather File</td>
    <td align="right">Chicago Ohare Intl Ap IL USA TMY3 WMO#=725300</td>
  </tr>
</table>

<b>Schedules-EFLH (Schedule Type=Fraction)</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">First Object Used</td>
    <td align="right">Equivalent Full Load Hours of Operation Per Year</td>
  </tr>
  <tr>
    <td align="right">LIGHTS-1</td>
    <td align="right">SPACE1-1 Lights 1</td>
    <td align="right">3066</td>
  </tr>
  <tr>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE2-1 ElecEq 1</td>
    <td align="right">3430</td>
  </tr>
  <tr>
    <td align="right">Always On Sch</td>
    <td align="right">Kitchen Exhaust Fan</td>
    <td align="right">8760</td>
  </tr>
  <tr>
    <td align="right">OCCUPY-1</td>
    <td align="right">SPACE1-1 People 1</td>
    <td align="right">2758</td>
  </tr>
</table>

<b>Schedules-SetPoints (Schedule Type=Temperature)</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">First Object Used</td>
    <td align="right">11am Monday [C]</td>
    <td align="right">11pm Monday [C]</td>
  </tr>
  <tr>
    <td align="right">Htg-SetP-Sch</td>
    <td align="right">HeatingSetpoint</td>
    <td align="right">21.1</td>
    <td align="right">12.8</td>
  </tr>
  <tr>
    <td align="right">Clg-SetP-Sch</td>
    <td align="right">CoolingSetpoint</td>
    <td align="right">23.9</td>
    <td align="right">40.0</td>
  </tr>
</table>

<b>Opaque Assemblies</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">First Object Used</td>
    <td align="right">Assembly U-Factor with Film [W/m2-K]</td>
    <td align="right">Reflectance</td>
  </tr>
  <tr>
    <td align="right">WALL-1</td>
    <td align="right">North WALL</td>
    <td align="right">0.384</td>
    <td align="right">0.22</td>
  </tr>
  <tr>
    <td align="right">ROOF-1</td>
    <td align="right">Top</td>
    <td align="right">0.271</td>
    <td align="right">0.35</td>
  </tr>
</table>

<b>Shading and Fenestration - Above Grade Wall and Vertical Glazing Area</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Above Grade Wall Area [m2]</td>
    <td align="right">Vertical Glazing Area [m2]</td>
    <td align="right">Above Ground Window-Wall Ratio [%]</td>
  </tr>
  <tr>
    <td align="right">North (315 to 45 deg)</td>
    <td align="right">       91.50</td>
    <td align="right">       20.85</td>
    <td align="right">       22.79</td>
  </tr>
  <tr>
    <td align="right">East (45 to 135 deg)</td>
    <td align="right">       45.60</td>
    <td align="right">        9.12</td>
    <td align="right">       20.00</td>
  </tr>
  <tr>
    <td align="right">South (135 to 225 deg)</td>
    <td align="right">       91.50</td>
    <td align="right">       21.81</td>
    <td align="right">       23.84</td>
  </tr>
  <tr>
    <td align="right">West (225 to 315 deg)</td>
    <td align="right">       45.60</td>
    <td align="right">        9.12</td>
    <td align="right">       20.00</td>
  </tr>
  <tr>
    <td align="right">Total</td>
    <td align="right">      274.20</td>
    <td align="right">       60.90</td>
    <td align="right">       22.21</td>
  </tr>
</table>

<b>Shading and Fenestration - Roof and Skylight Area</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Roof Area [m2]</td>
    <td align="right">Skylight Area [m2]</td>
    <td align="right">Skylight-Roof Ratio [%]</td>
  </tr>
  <tr>
    <td align="right">       Total</td>
    <td align="right">      463.60</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
</table>

<b>Lighting - Interior</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Zone</td>
    <td align="right">Lighting Power Density [W/m2]</td>
    <td align="right">Zone Area [m2]</td>
    <td align="right">Total Power [W]</td>
    <td align="right">End Use Subcategory</td>
    <td align="right">Schedule Name</td>
    <td align="right">Scheduled Hours/Week [hr]</td>
    <td align="right">Hours/Week > 1% [hr]</td>
    <td align="right">Full Load Hours/Week [hr]</td>
    <td align="right">Return Air Fraction</td>
    <td align="right">Conditioned (Y/N)</td>
    <td align="right">Consumption [kWh]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1 LIGHTS 1</td>
    <td align="right">SPACE1-1</td>
    <td align="right">     15.9742</td>
    <td align="right">       99.16</td>
    <td align="right">     1584.00</td>
    <td align="right">GeneralLights</td>
    <td align="right">LIGHTS-1</td>
    <td align="right">       57.70</td>
    <td align="right">      168.00</td>
    <td align="right">       57.70</td>
    <td align="right">      0.2000</td>
    <td align="right">Y</td>
    <td align="right">     4766.00</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1 LIGHTS 1</td>
    <td align="right">SPACE2-1</td>
    <td align="right">     16.0056</td>
    <td align="right">       42.73</td>
    <td align="right">      684.00</td>
    <td align="right">GeneralLights</td>
    <td align="right">LIGHTS-1</td>
    <td align="right">       57.70</td>
    <td align="right">      168.00</td>
    <td align="right">       57.70</td>
    <td align="right">      0.2000</td>
    <td align="right">Y</td>
    <td align="right">     2058.05</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1 LIGHTS 1</td>
    <td align="right">SPACE3-1</td>
    <td align="right">     16.4179</td>
    <td align="right">       96.48</td>
    <td align="right">     1584.00</td>
    <td align="right">GeneralLights</td>
    <td align="right">LIGHTS-1</td>
    <td align="right">       57.70</td>
    <td align="right">      168.00</td>
    <td align="right">       57.70</td>
    <td align="right">      0.2000</td>
    <td align="right">Y</td>
    <td align="right">     4766.00</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1 LIGHTS 1</td>
    <td align="right">SPACE4-1</td>
    <td align="right">     16.0056</td>
    <td align="right">       42.73</td>
    <td align="right">      684.00</td>
    <td align="right">GeneralLights</td>
    <td align="right">LIGHTS-1</td>
    <td align="right">       57.70</td>
    <td align="right">      168.00</td>
    <td align="right">       57.70</td>
    <td align="right">      0.2000</td>
    <td align="right">Y</td>
    <td align="right">     2058.05</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1 LIGHTS 1</td>
    <td align="right">SPACE5-1</td>
    <td align="right">     16.2420</td>
    <td align="right">      182.49</td>
    <td align="right">     2964.00</td>
    <td align="right">GeneralLights</td>
    <td align="right">LIGHTS-1</td>
    <td align="right">       57.70</td>
    <td align="right">      168.00</td>
    <td align="right">       57.70</td>
    <td align="right">      0.2000</td>
    <td align="right">Y</td>
    <td align="right">     8918.20</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting Total</td>
    <td align="right">&nbsp;</td>
    <td align="right">     16.1777</td>
    <td align="right">      463.60</td>
    <td align="right">     7500.00</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">    22566.30</td>
  </tr>
</table>

<b>Lighting - Exterior</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Total Watts</td>
    <td align="right">Astronomical Clock/Schedule</td>
    <td align="right">Schedule Name</td>
    <td align="right">Scheduled Hours/Week [hr]</td>
    <td align="right">Hours/Week > 1% [hr]</td>
    <td align="right">Full Load Hours/Week [hr]</td>
    <td align="right">Consumption [kWh]</td>
  </tr>
  <tr>
    <td align="right">Exterior Lighting Total</td>
    <td align="right">        0.00</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">        0.00</td>
  </tr>
</table>

<b>Process Loads - Space by Space Equipment Power Densities</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Name</td>
    <td align="right">Schedule Name</td>
    <td align="right">Zone Name</td>
    <td align="right">Zone Floor Area {m2}</td>
    <td align="right"># Zone Occupants</td>
    <td align="right">Equipment Level {W}</td>
    <td align="right">Equipment/Floor Area {W/m2}</td>
    <td align="right">Equipment per person {W/person}</td>
    <td align="right">Fraction Latent</td>
    <td align="right">Fraction Radiant</td>
    <td align="right">Fraction Lost</td>
    <td align="right">Fraction Convected</td>
    <td align="right">End-Use SubCategory</td>
    <td align="right">Nominal Minimum Equipment Level {W}</td>
    <td align="right">Nominal Maximum Equipment Level {W}</td>
  </tr>
  <tr>
    <td align="right">           1</td>
    <td align="right">SPACE1-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE1-1</td>
    <td align="right">99.16</td>
    <td align="right">11.0</td>
    <td align="right">1056.000</td>
    <td align="right">10.649</td>
    <td align="right">96.000</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">General</td>
    <td align="right">21.120</td>
    <td align="right">950.400</td>
  </tr>
  <tr>
    <td align="right">           2</td>
    <td align="right">SPACE2-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE2-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">456.000</td>
    <td align="right">10.670</td>
    <td align="right">91.200</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">General</td>
    <td align="right">9.120</td>
    <td align="right">410.400</td>
  </tr>
  <tr>
    <td align="right">           3</td>
    <td align="right">SPACE3-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE3-1</td>
    <td align="right">96.48</td>
    <td align="right">11.0</td>
    <td align="right">1056.000</td>
    <td align="right">10.945</td>
    <td align="right">96.000</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">General</td>
    <td align="right">21.120</td>
    <td align="right">950.400</td>
  </tr>
  <tr>
    <td align="right">           4</td>
    <td align="right">SPACE4-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE4-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">456.000</td>
    <td align="right">10.670</td>
    <td align="right">91.200</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">General</td>
    <td align="right">9.120</td>
    <td align="right">410.400</td>
  </tr>
  <tr>
    <td align="right">           5</td>
    <td align="right">SPACE5-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE5-1</td>
    <td align="right">182.49</td>
    <td align="right">20.0</td>
    <td align="right">1976.000</td>
    <td align="right">10.828</td>
    <td align="right">98.800</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">General</td>
    <td align="right">39.520</td>
    <td align="right">1778.400</td>
  </tr>
</table>


<b>Process Loads - Non-Receptcle Process Loads</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Energy Source</td>
    <td align="right">Name</td>
    <td align="right">Schedule Name</td>
    <td align="right">Zone Name</td>
    <td align="right">Zone Floor Area {m2}</td>
    <td align="right"># Zone Occupants</td>
    <td align="right">Equipment Level {W}</td>
    <td align="right">Equipment/Floor Area {W/m2}</td>
    <td align="right">Equipment per person {W/person}</td>
    <td align="right">Fraction Latent</td>
    <td align="right">Fraction Radiant</td>
    <td align="right">Fraction Lost</td>
    <td align="right">Fraction Convected</td>
    <td align="right">End-Use SubCategory</td>
    <td align="right">Nominal Minimum Equipment Level {W}</td>
    <td align="right">Nominal Maximum Equipment Level {W}</td>
  </tr>
  <tr>
    <td align="right">           1</td>
    <td align="right">Gas</td>
    <td align="right">SPACE2-1 GASEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE2-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">1000.000</td>
    <td align="right">23.400</td>
    <td align="right">200.000</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">Cooking</td>
    <td align="right">20.000</td>
    <td align="right">900.000</td>
  </tr>
  <tr>
    <td align="right">           2</td>
    <td align="right">Gas</td>
    <td align="right">SPACE4-1 GASEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE4-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">808.000</td>
    <td align="right">18.907</td>
    <td align="right">161.600</td>
    <td align="right">0.000</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.700</td>
    <td align="right">Laundry</td>
    <td align="right">16.160</td>
    <td align="right">727.200</td>
  </tr>
  <tr>
    <td align="right">           3</td>
    <td align="right">HotWater</td>
    <td align="right">SPACE2-1 HWEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE2-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">300.000</td>
    <td align="right">7.020</td>
    <td align="right">60.000</td>
    <td align="right">0.200</td>
    <td align="right">0.100</td>
    <td align="right">0.500</td>
    <td align="right">0.200</td>
    <td align="right">Dishwashing</td>
    <td align="right">6.000</td>
    <td align="right">270.000</td>
  </tr>
  <tr>
    <td align="right">           4</td>
    <td align="right">Steam</td>
    <td align="right">SPACE4-1 ELECEQ 1</td>
    <td align="right">EQUIP-1</td>
    <td align="right">SPACE4-1</td>
    <td align="right">42.74</td>
    <td align="right">5.0</td>
    <td align="right">1050.000</td>
    <td align="right">24.570</td>
    <td align="right">210.000</td>
    <td align="right">0.500</td>
    <td align="right">0.300</td>
    <td align="right">0.000</td>
    <td align="right">0.200</td>
    <td align="right">Laundry</td>
    <td align="right">21.000</td>
    <td align="right">945.000</td>
  </tr>
</table>

<b>Service Water Heating</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Type</td>
    <td align="right">Storage Volume [m3]</td>
    <td align="right">Input [W]</td>
    <td align="right">Thermal Efficiency [W/W]</td>
    <td align="right">Recovery Efficiency [W/W]</td>
    <td align="right">Energy Factor</td>
  </tr>
  <tr>
    <td align="right">None</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>

<b>Air-Side HVAC - Unitary Cooling</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">DX Cooling Coil Type</td>
    <td align="right">Standard Rated Net Cooling Capacity [W]</td>
    <td align="right">Standard Rated Net COP [W/W]</td>
    <td align="right">EER [Btu/W-h]</td>
    <td align="right">SEER [Btu/W-h]</td>
    <td align="right">IEER [Btu/W-h]</td>
  </tr>
  <tr>
    <td align="right">None</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>

<b>Air-Side HVAC - DX Heating Coils</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">DX Heating Coil Type</td>
    <td align="right">High Temperature Heating (net) Rating Capacity [W]</td>
    <td align="right">Low Temperature Heating (net) Rating Capacity [W]</td>
    <td align="right">HSPF [Btu/W-h]</td>
    <td align="right">Region Number</td>
  </tr>
  <tr>
    <td align="right">None</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>

<b>Air-Side HVAC - Heating Coils</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Type</td>
    <td align="right">Design Coil Load [W]</td>
    <td align="right">Nominal Total Capacity [W]</td>
    <td align="right">Nominal Efficiency [W/W]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1 ZONE COIL</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">     8121.80</td>
    <td align="right">     7211.11</td>
    <td align="right">-</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1 ZONE COIL</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">     4054.77</td>
    <td align="right">     3595.55</td>
    <td align="right">-</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1 ZONE COIL</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">     7902.28</td>
    <td align="right">     7013.24</td>
    <td align="right">-</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1 ZONE COIL</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">     3763.84</td>
    <td align="right">     3476.53</td>
    <td align="right">-</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1 ZONE COIL</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">    11585.19</td>
    <td align="right">     7250.47</td>
    <td align="right">-</td>
  </tr>
  <tr>
    <td align="right">MAIN HEATING COIL 1</td>
    <td align="right">Coil:Heating:Water</td>
    <td align="right">    16448.49</td>
    <td align="right">    10885.25</td>
    <td align="right">-</td>
  </tr>
</table>

<b>Air-Side HVAC - Fans</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Type</td>
    <td align="right">Total Efficiency [W/W]</td>
    <td align="right">Delta Pressure [pa]</td>
    <td align="right">Max Air Flow Rate [m3/s]</td>
    <td align="right">Rated Electric Power [W]</td>
    <td align="right">Rated Power Per Max Air Flow Rate [W-s/m3]</td>
    <td align="right">Motor Heat In Air Fraction</td>
    <td align="right">End Use</td>
  </tr>
  <tr>
    <td align="right">SUPPLY FAN 1</td>
    <td align="right">Fan:VariableVolume</td>
    <td align="right">        0.70</td>
    <td align="right">      600.00</td>
    <td align="right">        1.36</td>
    <td align="right">     1169.79</td>
    <td align="right">      857.14</td>
    <td align="right">        1.00</td>
    <td align="right">General</td>
  </tr>
</table>

<b>Water-Side HVAC - Central Plant</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Type</td>
    <td align="right">Nominal Capacity [W]</td>
    <td align="right">Nominal Efficiency [W/W]</td>
    <td align="right">IPLV in SI Units [W/W]</td>
    <td align="right">IPLV in IP Units [Btu/W-h]</td>
  </tr>
  <tr>
    <td align="right">CENTRAL BOILER</td>
    <td align="right">Boiler:HotWater</td>
    <td align="right">    39035.23</td>
    <td align="right">        0.80</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
  </tr>
  <tr>
    <td align="right">CENTRAL CHILLER</td>
    <td align="right">Chiller:Electric</td>
    <td align="right">    33478.64</td>
    <td align="right">        3.20</td>
    <td align="right">&nbsp;</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>

<b>Water-Side HVAC - Pumps</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Type</td>
    <td align="right">Control</td>
    <td align="right">Head [pa]</td>
    <td align="right">Water Flow [m3/s]</td>
    <td align="right">Electric Power [W]</td>
    <td align="right">Power Per Water Flow Rate [W-s/m3]</td>
    <td align="right">Motor Efficiency [W/W]</td>
  </tr>
  <tr>
    <td align="right">HW CIRC PUMP</td>
    <td align="right">Pump:VariableSpeed</td>
    <td align="right">Intermittent</td>
    <td align="right">   179352.00</td>
    <td align="right">    0.000845</td>
    <td align="right">      215.86</td>
    <td align="right">   255487.18</td>
    <td align="right">        0.90</td>
  </tr>
  <tr>
    <td align="right">CW CIRC PUMP</td>
    <td align="right">Pump:VariableSpeed</td>
    <td align="right">Intermittent</td>
    <td align="right">   179352.00</td>
    <td align="right">    0.001196</td>
    <td align="right">      305.51</td>
    <td align="right">   255487.18</td>
    <td align="right">        0.90</td>
  </tr>
</table>

<b>Performance Outputs - Energy Sources</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Utility Rate</td>
    <td align="right">Virtual Rate [$/unit energy]</td>
    <td align="right">Units of Energy</td>
    <td align="right">Units of Demand</td>
  </tr>
  <tr>
    <td align="right">Electricity</td>
    <td align="right"> EXAMPLEA EXAMPLEI-SELL</td>
    <td align="right">       0.055</td>
    <td align="right">kWh</td>
    <td align="right">kW</td>
  </tr>
  <tr>
    <td align="right">Natural Gas</td>
    <td align="right"> EXAMPLEA-GAS</td>
    <td align="right">       0.570</td>
    <td align="right">Therm</td>
    <td align="right">Therm/Hr</td>
  </tr>
  <tr>
    <td align="right">Other</td>
    <td align="right"> WATER TARIFF</td>
    <td align="right">       7.000</td>
    <td align="right">&nbsp;</td>
    <td align="right">/Hr</td>
  </tr>
</table>


<b>Performance Outputs - On-Site Renewable Energy Production</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Rated Capacity [kW]</td>
    <td align="right">Annual Energy Generated [kWh]</td>
  </tr>
  <tr>
    <td align="right">Photovoltaic</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Wind</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>

<b>Performance Outputs - Energy Summary by End Use</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Energy Type</td>
    <td align="right">Units of Annual Energy and Peak Demand</td>
    <td align="right">Value</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     22566.39</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">     7.51</td>
  </tr>

  <tr>
    <td align="right">Exterior Lighting</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     2412.</td>
  </tr>
  <tr>
    <td align="right">Exterior Lighting</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">     20.00</td>
  </tr>

  <tr>
    <td align="right">Space Heating</td>
    <td align="right">Natural Gas</td>
    <td align="right">Consumption (therms)</td>
    <td align="right">     28632.</td>
  </tr>
  <tr>
    <td align="right">Space Heating</td>
    <td align="right">Natural Gas</td>
    <td align="right">Demand (MBtu/h)</td>
    <td align="right">     4.20</td>
  </tr>

  <tr>
    <td align="right">Space Cooling</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     5002.94</td>
  </tr>
  <tr>
    <td align="right">Space Cooling</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">     9.68</td>
  </tr>

  <tr>
    <td align="right">Pumps</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     409.27</td>
  </tr>
  <tr>
    <td align="right">Pumps</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.31</td>
  </tr>

  <tr>
    <td align="right">Heat Rejection</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Heat Rejection</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Fans-Interior ventilation</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     1951.56</td>
  </tr>
  <tr>
    <td align="right">Fans-Interior ventilation</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.66</td>
  </tr>

  <tr>
    <td align="right">Fans-Parking Garage</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Fans-Parking Garage</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Service Water Heating</td>
    <td align="right">Natural Gas</td>
    <td align="right">Consumption (therm)</td>
    <td align="right">     9473.</td>
  </tr>
  <tr>
    <td align="right">Service Water Heating</td>
    <td align="right">Natural Gas</td>
    <td align="right">Demand (MBtu/h)</td>
    <td align="right">     0.35</td>
  </tr>

  <tr>
    <td align="right">Receptacle Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     13249.21</td>
  </tr>
  <tr>
    <td align="right">Receptacle Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.45</td>
  </tr>

  <tr>
    <td align="right">IT Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">IT Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Interior Lighting-Process</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting-Process</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Refrigeration Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Refrigeration Equipment</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Fans-Kitchen Ventilation</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Fans-Kitchen Ventilation</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Cooking</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Cooking</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Industrial Process</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Industrial Process</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Elevators and Escalators</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Elevators and Escalators</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Heat Pump Supplementary</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Heat Pump Supplementary</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Space Heating (Electricity)</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Space Heating (Electricity)</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Misc. Equipment (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Consumption (therm)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Misc. Equipment (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Demand (MBtu/h)</td>
    <td align="right">     0.00</td>
  </tr>

  <tr>
    <td align="right">Auxiliary (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Consumption (therms)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Auxiliary (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Demand (MBtu/h)</td>
    <td align="right">     0.00</td>
  </tr>

  <tr>
    <td align="right">Cooking (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Consumption (therm)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Cooking (Natural Gas)</td>
    <td align="right">Natural Gas</td>
    <td align="right">Demand (MBtu/h)</td>
    <td align="right">     0.00</td>
  </tr>

  <tr>
    <td align="right">Service Water Heating (Electricity)</td>
    <td align="right">Electricity</td>
    <td align="right">Consumption (kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Service Water Heating (Electricity)</td>
    <td align="right">Electricity</td>
    <td align="right">Demand (kW)</td>
    <td align="right">    0.00</td>
  </tr>

  <tr>
    <td align="right">Total Consumption by Energy Type</td>
    <td align="right">Electricity</td>
    <td align="right">(kWh)</td>
    <td align="right">     0.00</td>
  </tr>
  <tr>
    <td align="right">Total Consumption by Energy Type</td>
    <td align="right">Natural Gas</td>
    <td align="right">(therm)</td>
    <td align="right">     0.00</td>
  </tr>
</table>


<b>Performance Outputs - Annual Energy Cost by Energy Type</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Process Subtotal [$]</td>
    <td align="right">Total Energy Cost [$]</td>
  </tr>
  <tr>
    <td align="right">Electricity</td>
    <td align="right">      553.05</td>
    <td align="right">     1802.39</td>
  </tr>
  <tr>
    <td align="right">Natural Gas</td>
    <td align="right">        0.00</td>
    <td align="right">     1474.56</td>
  </tr>
  <tr>
    <td align="right">Other</td>
    <td align="right">&nbsp;</td>
    <td align="right">     1671.53</td>
  </tr>
  <tr>
    <td align="right">Total</td>
    <td align="right">      553.05</td>
    <td align="right">     4948.49</td>
  </tr>
  <tr>
    <td align="right">Additional</td>
    <td align="right">        0.00</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>
<i>Process energy cost based on ratio of process to total energy.</i>

<b>Performance Outputs - Virtual rate</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Virtual Rate [$/unit energy]</td>
    <td align="right">Units of Energy</td>
  </tr>
  <tr>
    <td align="right">Electricity</td>
    <td align="right">       0.055</td>
    <td align="right">kWh</td>
  </tr>
  <tr>
    <td align="right">Natural Gas</td>
    <td align="right">       0.570</td>
    <td align="right">Therm</td>
  </tr>
  <tr>
    <td align="right">Other</td>
    <td align="right">       7.000</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>

<b>Performance Outputs - Unmet Loads</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Data</td>
  </tr>
  <tr>
    <td align="right">Number of hours heating loads not met</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Number of hours cooling loads not met</td>
    <td align="right">       10.75</td>
  </tr>
  <tr>
    <td align="right">Number of hours not met</td>
    <td align="right">       10.75</td>
  </tr>
</table>


## Engineering Reference ##

No changes

## Example File and Transition Changes ##

The end use subcategory fields will be updated.

## References ##

v4_Minimum Energy Performance Calculator_v04.xlsm
