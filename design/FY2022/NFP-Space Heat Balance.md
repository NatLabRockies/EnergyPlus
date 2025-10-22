Subdivide Heat Balance by Space
================

**Michael J. Witte, GARD Analytics, Inc.**

 - Original April 22, 2022
 - Rev1 May 3, 2022 - Expand justification, make space heat balance calcs optional
 - Rev2 June 29, 2022 - Plan Z, heat balances for zones only, add optional HVAC ZoneList to group Zones, make Zone Name optional for surfaces
 - Rev3 July 8, 2022 - Revert Plan Z, stick with mostly original plan with optional space heat balance calcs

## Table of Contents ##

[Justification for New Feature](#justification-for-new-feature)

[E-mail and Conference Call Conclusions](#e-mail-and-conference-call-conclusions)

[Overiew](#overview)

[Approach](#approach)

[Testing/Validation/Data Sources](#testingvalidationdata-sources)

[Input Description](#input-description)

[Outputs Description](#outputs-description)

[Engineering Reference](#engineering-reference)

[Example File and Transition Changes](#example-file-and-transition-changes)

[Design](#design)

## Justification for New Feature ##

Space was added as a new concept in v9.6. At the time, it was decided that the definition of Zone would not change: a thermal zone (HVAC zone) is composed of one or more rooms (spaces) controlled by a single thermostat or HVAC system control point such as a VAV terminal unit. This aligns with the most common understanding of a "zone" in HVAC design and energy modeling practice, including the OpenStudio data model.

Currently, each EnergyPlus Zone contains one or more Spaces which are used for:

  * assigning and allocating internal gains
  * specifying enclosure boundaries,
  * reporting inputs grouped by space types, and
  * reporting select output grouped by space types.

The Zone air heat balance is currently lumped across all Spaces in the Zone. During the initial Space NFP discussions, additional capabilities were requested which would require an air heat balance for each Space:

  * Room-by-room sizing is commonly done to size individual room diffusers or equipment such as PTACs. Zone-level equipment may be sized to either the coincident or non-coincident peak across the rooms (Spaces).

  * Room-by-room HVAC simulation allows modeling the impact of thermostat placement in a specific room which often results in less-than-ideal temperature control for other rooms in the same Zone. For some applications, such as a unitary system with a single thermostat, this can be achieved by modeling each room as a separate Zone and using the "control zone" options. But this is not adequate for VAV systems or unitary systems with zoning controls.

## E-mail and Conference Call Conclusions ##
Q1: What, if any, effect on runtime performance do we expect?

A1: For this phase, the goal is to have a minimal impact on speed. Most of the calculations in the predictor-corrector are already looping over every surface, every internal gain, every HVAC supply node, etc. Looping over these groupings the space level and then aggregating results to the zone level shouldn't be costly. And radiant exchange is already at the enclosure-level, so that will not change. But just to be sure, the Space heat balance will be optional (and default to Zone-only).

Q2: I still don't understand the need for this feature. If you want a finer resolution of heat balances, you can already make smaller zones, no? If you want room-by-room sizing, just make each room a separate zone. The current balance of zone and space allows you to aggregate things nicely to the higher level zone heat balance without incurring additional runtime associated with the additional heat balances for spaces that aren't expected to be thermally dissimilar.

A2: The difference comes in the HVAC system sizing and controls. For unitary systems, one can make each room a separate zone, size each zone's airflow separately, and then use the current unitary system inputs for control zone and control flow fraction to model the system. But for other types of HVAC systems, such as VAV, there is no equivalent way to control the VAV damper (and reheat coil) based on a thermostat in one Space and split the airflow to diffusers in other Spaces. In the early discussions for the original Space implementation, one of the proposed approaches was to use Zones for the room-level model and add a new HVAC-Zone concept to group rooms for HVAC control. Ultimately, it was decided to keep Zone aligned with the concept of a group of Spaces (rooms) that are an HVAC control Zone. This is the next step along that path. Regarding performance, see Q1.

**June 29, 2022 Technicalities Call**

Plan Z was presented and discussed. Under Plan Z (heat balance only at the Zone level), if a user wanted room-by-room sizing data, then every room would need to be a Zone, and a new level of HVAC ZoneList would be used to group rooms (Zones) together for HVAC simulation and control. The question arose: "Why have Space then?" Further discussion revolved around the use case of room-by-room (Space-by-Space) sizing with Spaces grouped into large Zones for the HVAC simulation.

One of the implmentation hurdles discussed was that the current heat balance method loops over `Zone.HTSurfaceFirst:HTSurfaceLast` in multiple places. Converting this to `Space.HTSurfaceFirst:HTSurfaceLast` requires sorting surfaces by Space instead of by Zone. This coding has already been done.

## Overview ##

The current implementation of Space includes the following key features:

* Surfaces belong to both a zone and a space.
* Zones contain one or more spaces.
* Spaces have floor area (but not volume).
* Enclosures contain one or more spaces.
* Internal gains are allocated by space (input by zone or by space):
    * People
    * Lights
    * Equipment (Electric, Gas, HotWater, Steam, Other, ITE:AirCooled)
* Outputs by Space include:
    * Space Summary and Space Type Summary subtables of Input Verification and Results Summary
    * Internal gain output variables (by object, space, and zone)
    * Submeters by SpaceType (not by Space)

The air heat balance is currently at the zone level only. While some components are already calculated and summed across spaces, the zone air heat balance, HVAC equipment simulation, and zone air temperature calculations are lumped for all spaces within the zone. This task will focus on refactoring the zone predictor/corrector functions to calculate air temperatures and heat balance components by Space as well as by Zone.

## Approach ##

1. The first step will be to ensure that all components of the heat balance are allocated by space. This will require changing some objects to allow specification by space or zone, similar to what is already done for people, lights, etc.

2. The next step will be to calculate the Space air heat balance. This should be fairly straightforward, modifying existing calculation loops to sum components by Space instead of by Zone.

3. Combining the Space results for the Zone will be more challenging.

  * Some Zone results are simply a sum of the Space results.

  * During sizing, all Spaces in the Zone will be controlled to the same thermostat temperature, so combining Spaces into Zones will be straightforward, and all surfaces will see the same air temperature.

  * During the HVAC simulation, for this task, the Space air masses will be lumped together for the Zone heat balance. This will force all Spaces in the Zone to the same air temperature.

  * Future work may add Space-level HVAC distribution and thermostat control options to allow calculation of independent Space air temperatures.

Key aspects of the surface and air heat balances are listed below with italics indicating required changes.

### Surface Heat Balance ###
* Surfaces are already assigned to a space and an enclosure.
* Solar and radiant internal gains to surfaces are simulated by enclosure.
* Radiant exchange between surfaces is simulated by enclosure.
* Surface convection to the air is currently to the zone air temperature (or suppply air temperature depending on the convection model).
  * *Change to space air temperature.*
  * *Surfaces which are assigned to an auto-generated space named "ZoneName-Remainder" may need special handling. If a Remainder space exists within a given zone, it may be necessary to lump the spaces in that zone together for the zone heat balance.*

### Zone/Space Air Heat Balance ##

* Surface convection to the air is currently to the zone air temperature.
  * *Change to space air temperature.*
* Internal convective gains are already computed by space.
* Zone airflows for infiltration and mixing are currently by zone (ZoneInfiltration, ZoneMixing, ZoneCrossMixing).
  * *Modify inputs to specify by zone or space (similar to internal gains, but allocated by volume or floor area depending on the input method).*
* Air volume is currently by zone.
  * *Add input and calculations for Space volume. See [Issue #9362](https://github.com/NREL/EnergyPlus/issues/9362).*
* AirflowNetwork inputs and simulation will remain at the zone level.
  * *Allocate the impact on the air heat balance by space. For this phase AFN zones may need to use a lumped air heat balance.*
* ZoneAirBalance:OutdoorAir will remain a zone-level input, and the same method will be applied to all spaces in the zone.
* ZoneRefrigerationDoorMixing is currently between two zones.
  * *Modify inputs to specify by zone or space.*
* ZoneEarthtube will remain at the zone level, because it it thermostatically controlled.
* ZoneCoolTower:Shower is currently by zone.
  * *Modify inputs to specify by zone or space.*
* ZoneThermalChimney models a zone to represent the chimney which then draws from one or more other zones.
  * *Require that the thermal chimney zone has only one space.
  * *Modify the inlet zone names to be zone or space.*


## Testing/Validation/Data Sources ##

Zone-level results should stay the same.

## Input Description ##
No new objects are proposed. Several existing objects will have changes.

### HeatBalanceAlgorithm
* *Add a new field for Air Heat Balance Detail - Zone (default), ZoneAndSpace*

### Space
* *Add new field for Volume.*
* *Add new field for Ceiling Height.*
* *Transition required.*

### ZoneInfiltration:DesignFlowRate
* *Change field "Zone or ZoneList Name" to "Zone or ZoneList or Space or SpaceList Name."*

### ZoneInfiltration:EffectiveLeakageArea and ZoneInfiltration:FlowCoefficient
* *Change field "Zone Name" to "Zone or Space Name."*

### ZoneMixing and ZoneCrossMixing
* *Change field "Zone Name" to "Zone or Space Name."*

* *Change field "Source Zone Name" to "Source Zone or Space Name."*

### ZoneRefrigerationDoorMixing
* *Change field "Zone 1 Name" to "Zone or Space Name 1."*

* *Change field "Zone 2 Name" to "Zone or Space Name 2."*

### ZoneCoolTower:Shower
* *Change field "Zone Name" to "Zone or Space Name."*

### ZoneThermalChimney
* *Change field "Zone N Name" to "Inlet Zone or Space Name N."*

### ZoneControl:Thermostat:\*
* No change. Apply the same thermostat across all Spaces in the Zone.

## Outputs Description ##

Space equivalents will be added for the following zone output variables and meters (not sure about some of these, seems excessive):

```
Zone,Average,Zone Outdoor Air Drybulb Temperature [C]
Zone,Average,Zone Outdoor Air Wetbulb Temperature [C]
Zone,Average,Zone Outdoor Air Wind Speed [m/s]
Zone,Average,Zone Outdoor Air Wind Direction [deg]

Zone,Average,Zone Mean Radiant Temperature [C]
Zone,Average,Zone Windows Total Transmitted Solar Radiation Rate [W]
Zone,Average,Zone Exterior Windows Total Transmitted Beam Solar Radiation Rate [W]
Zone,Average,Zone Interior Windows Total Transmitted Beam Solar Radiation Rate [W]
Zone,Average,Zone Exterior Windows Total Transmitted Diffuse Solar Radiation Rate [W]
Zone,Average,Zone Interior Windows Total Transmitted Diffuse Solar Radiation Rate [W]
Zone,Sum,Zone Windows Total Transmitted Solar Radiation Energy [J]
Zone,Sum,Zone Exterior Windows Total Transmitted Beam Solar Radiation Energy [J]
Zone,Sum,Zone Interior Windows Total Transmitted Beam Solar Radiation Energy [J]
Zone,Sum,Zone Exterior Windows Total Transmitted Diffuse Solar Radiation Energy [J]
Zone,Sum,Zone Interior Windows Total Transmitted Diffuse Solar Radiation Energy [J]
Zone,Average,Zone Windows Total Heat Gain Rate [W]
Zone,Average,Zone Windows Total Heat Loss Rate [W]
Zone,Sum,Zone Windows Total Heat Gain Energy [J]
Zone,Sum,Zone Windows Total Heat Loss Energy [J]

Zone,Average,Zone Mean Air Temperature [C]
Zone,Average,Zone Operative Temperature [C]
Zone,Average,Zone Mean Air Dewpoint Temperature [C]
Zone,Average,Zone Mean Air Humidity Ratio [kgWater/kgDryAir]
HVAC,Average,Zone Air Heat Balance Internal Convective Heat Gain Rate [W]
HVAC,Average,Zone Air Heat Balance Surface Convection Rate [W]
HVAC,Average,Zone Air Heat Balance Interzone Air Transfer Rate [W]
HVAC,Average,Zone Air Heat Balance Outdoor Air Transfer Rate [W]
HVAC,Average,Zone Air Heat Balance System Air Transfer Rate [W]
HVAC,Average,Zone Air Heat Balance System Convective Heat Gain Rate [W]
HVAC,Average,Zone Air Heat Balance Air Energy Storage Rate [W]

HVAC,Average,Zone Exfiltration Heat Transfer Rate [W]
HVAC,Average,Zone Exfiltration Sensible Heat Transfer Rate [W]
HVAC,Average,Zone Exfiltration Latent Heat Transfer Rate [W]
HVAC,Average,Zone Exhaust Air Heat Transfer Rate [W]
HVAC,Average,Zone Exhaust Air Sensible Heat Transfer Rate [W]
HVAC,Average,Zone Exhaust Air Latent Heat Transfer Rate [W]

HVAC,Sum,Zone Infiltration Sensible Heat Loss Energy [J]
HVAC,Sum,Zone Infiltration Sensible Heat Gain Energy [J]
HVAC,Sum,Zone Infiltration Latent Heat Loss Energy [J]
HVAC,Sum,Zone Infiltration Latent Heat Gain Energy [J]
HVAC,Sum,Zone Infiltration Total Heat Loss Energy [J]
HVAC,Sum,Zone Infiltration Total Heat Gain Energy [J]
HVAC,Average,Zone Infiltration Current Density Volume Flow Rate [m3/s]
HVAC,Average,Zone Infiltration Standard Density Volume Flow Rate [m3/s]
HVAC,Sum,Zone Infiltration Current Density Volume [m3]
HVAC,Sum,Zone Infiltration Standard Density Volume [m3]
HVAC,Sum,Zone Infiltration Mass [kg]
HVAC,Average,Zone Infiltration Mass Flow Rate [kg/s]
HVAC,Average,Zone Infiltration Air Change Rate [ach]

HVAC,Sum,Zone Air System Sensible Heating Energy [J]
HVAC,Sum,Zone Air System Sensible Cooling Energy [J]
HVAC,Average,Zone Air System Sensible Heating Rate [W]
HVAC,Average,Zone Air System Sensible Cooling Rate [W]
HVAC,Average,Zone Air Temperature [C]
HVAC,Average,Zone Thermostat Air Temperature [C]
HVAC,Average,Zone Air Humidity Ratio []
HVAC,Average,Zone Air Relative Humidity [%]
HVAC,Average,Zone Predicted Sensible Load to Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone Predicted Sensible Load to Heating Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone Predicted Sensible Load to Cooling Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone System Predicted Sensible Load to Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone System Predicted Sensible Load to Heating Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone System Predicted Sensible Load to Cooling Setpoint Heat Transfer Rate [W]
HVAC,Average,Zone Predicted Moisture Load Moisture Transfer Rate [kgWater/s]
HVAC,Average,Zone Predicted Moisture Load to Humidifying Setpoint Moisture Transfer Rate [kgWater/s]
HVAC,Average,Zone Predicted Moisture Load to Dehumidifying Setpoint Moisture Transfer Rate [kgWater/s]
HVAC,Average,Zone System Predicted Moisture Load Moisture Transfer Rate [kgWater/s]
HVAC,Average,Zone System Predicted Moisture Load to Humidifying Setpoint Moisture Transfer Rate [kgWater/s]
HVAC,Average,Zone System Predicted Moisture Load to Dehumidifying Setpoint Moisture Transfer Rate [kgWater/s]
Zone,Average,Zone Thermostat Control Type []
HVAC,Average,Zone Thermostat Heating Setpoint Temperature [C]
HVAC,Average,Zone Thermostat Cooling Setpoint Temperature [C]
Zone,Average,Zone Adaptive Comfort Operative Temperature Set Point [C]
HVAC,Average,Zone Predicted Sensible Load Room Air Correction Factor []

HVAC,Sum,Zone Oscillating Temperatures Time [hr]
HVAC,Sum,Zone Oscillating Temperatures During Occupancy Time [hr]
HVAC,Sum,Zone Oscillating Temperatures in Deadband Time [hr]
HVAC,Sum,Facility Any Zone Oscillating Temperatures Time [hr]
HVAC,Sum,Facility Any Zone Oscillating Temperatures During Occupancy Time [hr]
HVAC,Sum,Facility Any Zone Oscillating Temperatures in Deadband Time [hr]
Zone,Sum,Zone Thermal Comfort ASHRAE 55 Simple Model Summer Clothes Not Comfortable Time [hr]
Zone,Sum,Zone Thermal Comfort ASHRAE 55 Simple Model Winter Clothes Not Comfortable Time [hr]
Zone,Sum,Zone Thermal Comfort ASHRAE 55 Simple Model Summer or Winter Clothes Not Comfortable Time [hr]

Zone,Sum,Zone Heating Setpoint Not Met Time [hr]
Zone,Sum,Zone Heating Setpoint Not Met While Occupied Time [hr]
Zone,Sum,Zone Cooling Setpoint Not Met Time [hr]
Zone,Sum,Zone Cooling Setpoint Not Met While Occupied Time [hr]

Output:Meter,Electricity:Zone:ZONE 1,hourly; !- [J]
Output:Meter:Cumulative,Electricity:Zone:ZONE 1,hourly; !- [J]
Output:Meter,InteriorLights:Electricity:Zone:ZONE 1,hourly; !- [J]
Output:Meter:Cumulative,InteriorLights:Electricity:Zone:ZONE 1,hourly; !- [J]
Output:Meter,GeneralLights:InteriorLights:Electricity:Zone:ZONE 1,hourly; !- [J]
Output:Meter:Cumulative,GeneralLights:InteriorLights:Electricity:Zone:ZONE 1,hourly; !- [J]
*and all types of end-uses and sub-end-uses*

```

## Engineering Reference ##

Summary paragraphs or sentences will be added to indicate that references to "Zone" throughout the heat balance documentation are applicable to "Space or Zone".

## Example File and Transition Changes ##

* The Space object will require transition to insert the new Ceiling Height and Volume fields.

* No transition will be required for other idf inputs.

* Field name changes will be required for epJSON inputs.

* The existing example file 5ZoneAirCooledWithSpaces will demonstrate the new calculations.

* A new example file with zone air flow objects by space will also be added.

## Design ##

* The main calculations happen in ZoneTempPredictorCorrector.cc. The functions here will be refactored to calculate values by space, then aggregate them by zone.

* **OR** should the space and zone calculations be done in parallel?  i.e. keep the existing zone calcs to maintain the same results, but then add space calcs on the side?

* Any objects which are being modified to accept space or zone name will, of course, require changes to input processing, data structures and calulations.

### ZoneTempPredictorCorrector.cc
`GetZoneAirSetPoints`
* No change - Thermostats and ZoneCapacitanceMultiplier:ResearchSpecial remain at the zone level.

`InitZoneAirSetPoints`
* Many variables are allocated or dimensioned here per zone, zonelist, and zonegroup. Determine which ones need a space-level equivalent.
* Many zone output variables are set up here. Determine which ones need a space equivalent.
* Loop at line 2928 to check for matching `SurfTAirRef(SurfNum)` needs to be reworked per space(?).
* Some of the intitialization loops may need to be duplicated for spaces.

`CalcZoneAirTempSetPoints`

`CalcZoneSums` and `CalcZoneComponentLoadSums`
* Modify these functions to be by space, rename to `CalcSpaceSums` and `CalcSpaceComponentLoadSums`.
* OR add space sub-loops and data structures to store space values. This depends on how the functions which call `CalcZoneSums` and `CalcZoneComponentLoadSums` are refactored for spaces.

```
void CalcZoneSums(EnergyPlusData &state,
                  int const ZoneNum,  // Zone number
                  Real64 &SumIntGain, // Zone sum of convective internal gains
                  Real64 &SumHA,      // Zone sum of Hc*Area
                  Real64 &SumHATsurf, // Zone sum of Hc*Area*Tsurf
                  Real64 &SumHATref,  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
                  Real64 &SumMCp,     // Zone sum of MassFlowRate*Cp
                  Real64 &SumMCpT,    // Zone sum of MassFlowRate*Cp*T
                  Real64 &SumSysMCp,  // Zone sum of air system MassFlowRate*Cp
                  Real64 &SumSysMCpT, // Zone sum of air system MassFlowRate*Cp*T
                  bool const CorrectorFlag)
```

* As part of this work it would be good to precalculate and store the full zone multipliers instead of recalculating them like this in many places throughout the code:
`ZoneMult = Zone(ZoneNum).Multiplier * Zone(ZoneNum).ListMultiplier;`

`CalcZoneSensibleOutput`
* No change necessary inside the function,because all values are passed in as arguments.
* Calls to this function may need to be changed to pass in space values rather than zone values.

`PredictSystemLoads`
* Break apart into multiple functions for
  * Staged thermostat setpoint
  * Setpoint revision for onoff thermostat
  * Update zone/space temperatures
  * Calculate zone/space heat balance terms
  * Exact solution or Euler method - solve for zone/space air temp
  * Calculate predicted loads
  * Set OnOff Tstat status

`CorrectZoneAirTemp` and `CorrectZoneHumRat`
* Break apart into multiple functions for
  * Update zone/space temperatures (re-use same function from `PredictSystemLoads`?)
  * Calculate zone/space heat balance terms (re-use same function from `PredictSystemLoads`?)
  * Exact solution or Euler method - solve for zone/space air temp (re-use?)
  * Bookkeeping

`RevertZoneTimestepHistories`
* Refactor to handle zone or space

`PushZoneTimestepHistories`
* Refactor to handle zone or space

`PushSystemTimestepHistories`
* Refactor to handle zone or space
