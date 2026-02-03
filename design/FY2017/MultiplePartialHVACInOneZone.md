Improve Control of Multiple HVAC in One Zone
====================================================

**Michael J. Witte, GARD Analytics, Inc.**

 - November 30, 2016 - Original NFP
 - January 4, 2017 - Revised NFP and Initial Design
 - September 29, 2017 - Final Design

Reviewers - Raustad, Griffith, Lee, Gu, Horowitz, Merket, Winkler, Sheier

## Table of Contents ##

[Justification for New Feature](#justification-for-new-feature)

[E-mail and Conference Call Conclusions](#e-mail-and-conference-call-conclusions)

[Approach](#approach)

[Testing/Validation/Data Sources](#testingvalidationdata-sources)

[Input Output Reference Documentation](#input-output-reference-documentation)

[Outputs Description](#outputs-description)

[Engineering Reference](#engineering-reference)

[Example File and Transition Changes](#example-file-and-transition-changes)

[Design](#design)

[Detailed Comments and Responses](#detailed-comments-and-responses)

[Original Approach - Dropped](#original-approach---dropped)

## Justification for New Feature ##

Currently, EnergyPlus allows more than one type of HVAC equipment to serve the same zone. However, there are limitations on how to share the load both for sizing and operation. Examples of applications where equipment shares the load within a zone include:

 - Single-zone residential model with a window air conditioner in one part of the house and a central system serving the rest of the house.
 - Large retail or warehouse zone with multiple rooftop units of differing type.

For sizing, there are optional inputs which allow scalable sizing of `ZoneHVAC:*` objects using `DesignSpecification:ZoneHVAC:Sizing` or fields which are directly in the ZoneHVAC object.  A companion development project, "Allow Multiple Air Loops to One Thermal Zone" is adding a similar object for AirTerminal:* objects, `DesignSpecification:AirTerminal:Sizing`.

For operation, however, the user can only specify the order of simulation. Each piece of equipment attempts to meet the entire remaining load in a sequential fashion.  For many applications, the equipment shares the load in parallel. This feature will add inputs to specify how the load is to be shared. The equipment will still be simulated in a sequential fashion, but the new inputs will allow each piece of equipment to hold back and meet only part of the load, leaving the remaining load for the next piece of equipment.

## E-mail and  Conference Call Conclusions ##

1. Add output variables and/or table report to summarize how each piece of equipment operates in a zone. E.g., what loads they meet, how many hours they operate, etc.

2. Ditch Options A and B.  New Option C - Add a single new field to ZoneHVAC:EquipmentList that is equivalent to the Load Distribution Scheme field in the PlantLoop object with options of: SequentialLoad, UniformPLR, and SequentialUniformPLR.

## Approach ##

The current `ZoneHVAC:EquipmentList` object controls the order of simulation for heating and for cooling.
```
  ZoneHVAC:EquipmentList,
    SPACE2-1 Eq,             !- Name
    ZoneHVAC:CoolingPanel:RadiantConvective:Water,  !- Zone Equipment 1 Object Type
    SPACE2 Cooling Panel,    !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type
    SPACE2-1 ATU,            !- Zone Equipment 2 Name
    2,                       !- Zone Equipment 2 Cooling Sequence
    2,                       !- Zone Equipment 2 Heating or No-Load Sequence
    ZoneHVAC:Baseboard:RadiantConvective:Water,  !- Zone Equipment 3 Object Type
    SPACE2-1 Baseboard,      !- Zone Equipment 3 Name
    3,                       !- Zone Equipment 3 Cooling Sequence
    3;                       !- Zone Equipment 3 Heating or No-Load Sequence
```

A new field will be added (after the Name field) to control the distribution of loads, similar to the same field in the PlantLoop object:
```
  A2,   \field Load Distribution Scheme
        \type choice
        \key SequentialLoad
        \key UniformLoad
        \key UniformPLR
        \key SequentialUniformPLR
        \default SequentialLoad
```

SequentialLoad is the same as the existing method, simulate the equipment in the order specified in the equipment list and let each piece of equipment serve as much of the load as possible.

UniformLoad will divide the load by the number of pieces of equipment that are available.

UniformPLR and SequentialUniformPLR will be more difficult to implement, because the zone equipment does not currently think in terms of PLR the way that boilers and chillers do.

## Testing/Validation/Data Sources ##

Build example files and check results.

## Input Output Reference Documentation ##


### Modified Object: ZoneHVAC:EquipmentList###

*New Field: Load Distribution Scheme*

This alpha field contains the Load Distribution Scheme Keyword. The Load Distribution Scheme selects the algorithm used to sequence equipment operation in order to meet the zone thermostat demand. Currently, four schemes are functional.

- **SequentialLoad** loads each piece of equipment sequentially in the order specified in the sequence fields to its maximum part load ratio and will operate the last required piece of equipment between its minimum and maximum part load ratio in order to meet the zone demand.
- **UniformLoad** evenly distributes the zone demand among all available components on the equipment list for a given load type.
- **SequentialUniformPLR** loads all operating pieces equipment to a uniform part load ratio (PLR). Components are loaded sequentially based on the order specified in the sequence fields until each component is fully loaded, at which point the next component is added and the load is distributed uniformly based on PLR between the operating  components.
- **UniformPLR** will load all operating pieces of equipment to a uniform part load ratio (PLR). No equipment will be loaded below its minimum PLR. If the total load is less than the sum of all equipment operating at their respective minimum PLRs, then the last item in the equipment list is dropped and the load is distributed based on a uniform PLR for the remaining plant equipment.


## Outputs Description ##

As proposed in the companion project, a set of general zone-level output variables will be developed report the contribution of each piece of zone equipment.  Proposed output variable names:

Zone HVAC Equipment 1 Sensible Cooling Rate [W]
Zone HVAC Equipment 1 Total Cooling Rate [W]
Zone HVAC Equipment 1 Sensible Heating Rate [W]
Zone HVAC Equipment 1 Total Heating Rate [W]
Zone HVAC Equipment 1 Sensible Cooling Energy [J]
Zone HVAC Equipment 1 Total Cooling Energy [J]
Zone HVAC Equipment 1 Sensible Heating Energy [J]
Zone HVAC Equipment 1 Total Heating Energy [J]

A table report will also be added which lists each piece of zone equipment (both air terminals and zone HVAC) with columns for Type, Name, Sensible Cooling [J], Total Cooling [J], Cooling Hours, Sensible Heating [J], Total Heating [J], Heating Hours, Outdoor Air Volume [m3]. There will be one row per piece of zone equipment and a total row.

## Engineering Reference ##

The current section "Zone Equipment Simulation" discusses simulation of air terminal units which are part of an air loop in great detail.  The only statement about other zone equipment is:

"If multiple air-conditioning components are attached to a zone, the components
are simulated in the order specified by the user assigned priority given in the ZoneHVAC:EquipmentList object."

This section will be expanded to include more details of the ZoneHVAC:EquipmentList simulation, including the accounting for initial zone loads, remaining loads, load to heating setpoint and load to cooling setpoint.

## Example File and Transition Changes ##

Proposed example files:

 - Single-zone residential model with window air conditioner and central system.
 - Retail store with two rooftop units, one high-efficiency and one low-efficiency serving the same core zone.

Transition will be required for `ZoneHVAC:EquipmentList`.

## Design ##

1. Add an enum class to `DataZoneEquipment` to hold the load distribution types.
2. Add a new field `LoadDistScheme` to `DataZoneEquipment::ZoneEquipList`.
3. Modify `DataZoneEquipment::GetZoneEquipmentData1` to take the new input.
4. Add logic for the new load distribution in a new function `ZoneEquipmentManager::DistributeSystemOutputRequired` called after `ZoneEquipmentManager::SetPrioritySimOrder`.
5. Add logic in `ZoneEquipmentManager::UpdateSystemOutputRequired` to do the right thing with remaining loads depending on the load distribution scheme.

The logical place to make the load assignments/adjustments is in
```
Array1D< Real64 > SequencedOutputRequired;
Array1D< Real64 > SequencedOutputRequiredToHeatingSP; // load required to meet heating setpoint by sequence
Array1D< Real64 > SequencedOutputRequiredToCoolingSP; // load required to meet cooling setpoint by sequence
```

Currently, only the unitary systems use these for control, so other zone equipment will need to be modified to access these, either by passing the desired loads as arguments to the equipment or by passing the equipment list sequence number.

The current functions which set/adjust the sequenced outputs required are:
```
ZoneEquipmentManager::InitZoneEquipment (allocates the sequenced output arrays)
ZoneEquipmentManager::InitSystemOutputRequired (initializes all sequenced output = full output required)
ZoneEquipmentManager::UpdateSystemOutputRequired (if EquipPriorityNum is passed in, then sets next sequenced output required to be the remaining load)

ZoneTempPredictorCorrector::InitZoneAirSetPoints (initiales all sequenced outputs to zero)
ZoneTempPredictorCorrector::CalPredictedSystemLoad (initializes all sequenced output = full output required)
```

So, the likely places to implement the load distribution options is in `ZoneEquipmentManager::InitSystemOutputRequired` and `ZoneEquipmentManager::UpdateSystemOutputRequired`.

In order to establish a PLR, some kind of query needs to be done to find out what the equipment can do and if it is active (it could be scheduled off or an airloop or plant loop could be shut down) and how much output it can give.

Also, because the air loop equipment is simulated before the zone equipment, there must be an additional iteration so that the air loop equipment can respond to the modified sequenced loads.  The minimum number of HVAC iterations will be set be the highest number of zone equipment in any zone, plus the first HVAC iteration.  The overall sequence is as follows (draft engineering ref section):

The function `DistributeSystemOutputRequired` allocates the curent zone load among the available pieces of zone equipment for the current load type (cooling, heating, or no-load). Because some air loop components such as AirLoopHVAC:UnitarySystem may be controlled based on a control zone load, the sequenced loads must be known prior to the final iteration of the HVAC simulation so that the air loop equipment will adjust its output accordingly. When the zone equipment list is initially read, the maximum number of equipment across all zones is used to set the number of air loop iterations required after the initial iteration, `MinAirLoopIterationsAfterFirst`. The control sequence is shown below.

1. Initial iteration (`FirstHVACIteration` is true)

    a. Set all sequenced loads to the full load required.

    b. Simulate air loops and zone equipment.

    c. Store the sensible output (capacity) for each piece of equipment.


2. Second iteration (`FirstHVACIteration` is false)

    a. Set all sequenced loads according to the specified load distribution scheme:

      * SequentialLoad - Initially all sequenced loads are set to the full load required.

      * UniformLoad - The sequenced loads for all active equipment are set to the full load divided by the number of active pieces of equipment. All inactive sequenced loads are set to zero.

      * UniformPLR - Using the current equipment capacities (stored during the initial iteration), distribute the load among the availalbe pieces of equipment, such that each one is operating at the same part load ratio (PLR).

      * SequentialUniformPLR - Using the current equipment capacities (stored during the initial iteration), determine how many of the available pieces of equipment are required to meet the current full load.  Then distribute the load among those pieces of equipment, such that each one is operating at the same part load ratio (PLR).

    b. Simulate air loops.

    c. Simulate zone equipment.

     * If the load distribution type is SequentialLoad, update each successive sequenced load to be the current remaining load.

      ( Otherwise, leave the sequenced loads as-is from the initial distribution.

3. Third iteration and beyond (`FirstHVACIteration` is false)

    a. Simulate air loops.

    b. Simulate zone equipment.

      * If the load distribution type is SequentialLoad, update each successive sequenced load to be the current remaining load.

      * Otherwise, leave the sequenced loads as-is from the initial distribution.

## Detailed Comments and Responses ##

11/30/2016 - Raustad

It seems to me that a multiplier addresses the total zone load (var 2-4 below), so let's be specific as to which load variable is used. The remaining load variable is adjusted as each zone equipment is simulated. Sequenced output is adjusted as zone equipment is simualated and Remaining load results are passed back to the air loop. A zone eqipment load percentage needs to be fixed to the total zone load, and limited by remainging load, and not affect the air loop on successive iterations. For this to work without oscillations, the remaining or sequenced loads should not be used since they would change each iteration. The Sequenced load vars are adjusted each iteration based on the Remaining load vars. To avoid oscillations, stick with the total zone load vars and limit to Remaining so that loads met always meet SP.

Simulation order: Airloop then zone equipment.
If air loop is controlled off of Sequenced load, than this var needs to converge quickly.

Real64 RemainingOutputRequired;
Real64 TotalOutputRequired;
Real64 OutputRequiredToHeatingSP; // Load required to meet heating setpoint (>0 is a heating load)
Real64 OutputRequiredToCoolingSP; // Load required to meet cooling setpoint (<0 is a cooling load)
Real64 RemainingOutputReqToHeatSP; // Remaining load required to meet heating setpoint (>0 is a heating load)
Real64 RemainingOutputReqToCoolSP; // Remaining load required to meet cooling setpoint (<0 is a cooling load)
Array1D< Real64 > SequencedOutputRequired;
Array1D< Real64 > SequencedOutputRequiredToHeatingSP; // load required to meet heating setpoint by sequence
Array1D< Real64 > SequencedOutputRequiredToCoolingSP; // load required to meet cooling setpoint by sequence

  *MJW - Good point about stability concerns, so that argues in favor of Option A - multiplying by the initial load from the thermostat which doesn't change.*

12/1/2016 - Glazer

It would be nice to add some tabular reporting that reports on this new feature even if it is just some Output:Table:Monthly or Output:Table:Annual objects included in the new example file. Maybe that report how much each piece of equipment is taking of the load at peak time and over time.

  *MJW - Will add.*

12/1/2016 - Raustad

I'm having difficulty with which variable to use.
In the zone list there are air loop AND zone equipment. Each use a different load variable.

The first pass through:
the air loop uses the Sequencea variable which is equal to the thermostat load
RemainingOutputReq is updated here to be the remaining total zone load
Then the zone equipment uses RemainingOutputReq as it goes to zero
Then Sequence variable is updated based on what remains so that the air loop picks up any load not met or that is added for example by DOAS equipment.

Then on the next iteration:
the air loop uses the Sequence variable which can now not be equal to the thermostat load
then the zone equipment uses an adjusted RemainingOutputReq (i.e., can be different from first pass since the air loop picked up additional load)

It feels like these should be locked in place at this point but they are not in current develop. However, zone equipment should completely? eliminate the RemaingOutputReq variable on the second pass. So from here on the Sequence variable should never change? My first thought is that only the Sequence variable should be locked at this point (not updated again?). From there, the remaining total zone load should never change.

Might it be as simple as locking the Sequence variable after the 2nd pass?
I wonder how these variables are changing in some of the SimHVAC max iteration defect files and if this method would fix those?

12/2/2016 - Gu

I think fraction schedules may not be enough. For example, if a system load is very small, then the load is divided into 2, and 2 equipment will turn on at the same time. Each equipment will handle a tiny load. It is hard to image this will happen in the real world. I would like to suggest to have an absolute minimum value for each equipment is also involved. For example, the first equipment load = max ( fraction, absolute minimum), and second equipment load = min (remaining, max (fraction, absolute minimum)).

  *MJW - That's a good point, about very small loads.  Perhaps a new object would be better than loading on more repeating fields into ZoneHVAC:EquipmentList.*

12/7/2016 - Sizing group conference call

Ditch Options A and B.  New Option C - Add a single new field to ZoneHVAC:EquipmentList that is equivalent to the Load Distribution Scheme field in the PlantLoop object with options of: SequentialLoad, UniformPLR, and SequentialUniformPLR.  PlantLoop also has an Optimal choice, but zone equipment doesn't have any information for that.

## Original Approach - Dropped ##
A new pair of fields are proposed for each piece of equipment:

```
   Zone Equipment <x> Cooling Fraction Schedule Name
   Zone Equipment <x> Heating or No-Load Fraction Schedule Name
```
There are two ways that the schedule could be applied.

### Option A - Initial Load
The zone equipment manager multiplies the initial load by the applicable schedule fraction before calling a given piece of equipment.  Here is an example where the radiant cooling panels meet 25% of the cooling load and the air system meets the remaining load.

```
  ZoneHVAC:EquipmentList,
    SPACE2-1 Eq,             !- Name
    ZoneHVAC:CoolingPanel:RadiantConvective:Water,  !- Zone Equipment 1 Object Type
    SPACE2 Cooling Panel,    !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    Always-0.25,             !- Zone Equipment 1 Cooling Fraction Schedule Name
    ,                        !- Zone Equipment 1 Heating or No-Load Fraction Schedule Name
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type
    SPACE2-1 ATU,            !- Zone Equipment 2 Name
    2,                       !- Zone Equipment 2 Cooling Sequence
    2,                       !- Zone Equipment 2 Heating or No-Load Sequence
    Always-0.75,              !- Zone Equipment 2 Cooling Fraction Schedule Name
    ;                        !- Zone Equipment 2 Heating or No-Load Fraction Schedule Name
```

If the current cooling load is 1000W, the cooling panel will be passed a load of 0.25 \* 1000=250W. The air distribution unit will be passed a load of 0.75 \* 1000W=750W.

*Pros* - The schedule fractions are intuitive, 0.25 + 0.75 = 1.0

*Cons* - If the first piece of equipment cannot provide the requested load, the next one will not try to meet to meet the unmet portion. and the zone setpoint will not be met.

### Option B - Remaining Load
The zone equipment manager multiplies the current remaining load by the applicable schedule fraction before calling a given piece of equipment.  Here is an example where the radiant cooling panels meet 25% of the cooling load and the air system meets the remaining load.

```
  ZoneHVAC:EquipmentList,
    SPACE2-1 Eq,             !- Name
    ZoneHVAC:CoolingPanel:RadiantConvective:Water,  !- Zone Equipment 1 Object Type
    SPACE2 Cooling Panel,    !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    Always-0.25,             !- Zone Equipment 1 Cooling Fraction Schedule Name
    ,                        !- Zone Equipment 1 Heating or No-Load Fraction Schedule Name
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type
    SPACE2-1 ATU,            !- Zone Equipment 2 Name
    2,                       !- Zone Equipment 2 Cooling Sequence
    2,                       !- Zone Equipment 2 Heating or No-Load Sequence
    Always-1.0,              !- Zone Equipment 2 Cooling Fraction Schedule Name
    ;                        !- Zone Equipment 2 Heating or No-Load Fraction Schedule Name
```

If the current cooling load is 1000W, the cooling panel will be passed a load of 0.25*1000=250W.  If the panel can provide this, then the remaining load of 750W is passed to the air distribution unit. If the cooling panel only provides 200W, then 1000-200=800W will be passed to the air distribution unit.

*Pros* - If the first piece of equipment cannot provide the requested load, the next one tries to meet to meet the unmet portion.

*Cons* - The schedule fractions are not intuitive. Because the schedule is applied to the remaining load, the schedule fractions will not add up to 1.0.
