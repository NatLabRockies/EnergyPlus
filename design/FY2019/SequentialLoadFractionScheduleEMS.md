Zone Equipment List Sequential Load Fraction Scheduling or Actuators
====================================================================

**Noel Merket, NREL**

 - 16 May 2019, Initial Draft

## Justification for New Feature ##

> I can't think of a situation where someone would want to schedule the load fractions. Let's not complicate it by making them schedules.

-- Noel Merket, during the discussions about adding load fractions to `ZoneHVAC:EquipmentList`

Well, it turns out there are some cases where it would be convenient to have the load fraction be available to be actuated via EMS. Specifically, in retrofit scenarios a mini split can be added to a building with a furnace or boiler is already sized to meet the whole load. The capacity of the mini split varies with outdoor temperature and it would be useful to be able to actuate the fraction to reflect that.

## E-mail and  Conference Call Conclusions ##

Based on the feedback received on the proposal, I am inclined to go with Option A, changing the fraction to a schedule. It allows for the most user flexibility.

### Feedback on Proposal

from Chris Balbach, PSD:

> Of the (3) Options presented, PSD prefers **Option A**

> Specifically **Option A** provides a method for 'recording' any changes that a EMS program might make to the schedule values. Using the OpenStudio API, I can imagine an EMS measure that first clones the existing schedule for archival purposes (leaving a transparent record of the original schedule values), and then applies logic to potentially replace the active schedule value at each model timestep. The subsequent  schedule also represents a transparent and easily inspect-able object.

from Brent Griffith:

> I like option C. It will likely work better for HVAC control because it could update faster to vary within system iterations.

from Mike Witte:

> I'm ok with A or C. The transiton for A isn't all that bad - just add one-liner Schedule:Constant objects for each unique fraction (not likely to be many) and leave it blank for incoming values of 1.0

> Brent suggested the EMS control would be tighter but I think it would be the same either way because getcurrentschedulevalue checks for ems override at the time it's called.

## Overview ##

I added sequential load fractions in the 9.1 release to capture situations where multiple HVAC equipment each serves a user-specifiable fraction the same zone. This feature proposal adds the capability to actuate those fractions via EMS.

## Approach ##

There are a couple ways to do this, each with some benefits and drawbacks.

### Option A: Change the single fractional input to a schedule

This would change the following

```
  N4 , \field Zone Equipment 1 Sequential Heating Fraction
       \note The fraction of the remaining heating load this equipment will attempt to serve
       \note if the load distribution scheme is SequentialLoad, otherwise ignored.
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 1.0
```

to something like

```
  N4 , \field Zone Equipment 1 Sequential Heating Fraction Schedule Name
       \note The fraction of the remaining heating load this equipment will attempt to serve
       \note if the load distribution scheme is SequentialLoad, otherwise ignored.
       \type object-list
       \object-list ScheduleNames
```

and similarly change the cooling fractions.

If this is left blank, assume the fraction is 1.0 all the time.

#### Pros

- Probably easiest to implement.
- Schedules can already be actuated by EMS.
- The fractions can be supplied as a schedule (for example pre-calculated based on weather data) and EMS can be avoided in some cases.

#### Cons

- More complicated object definition for the vast majority of users who don't need to schedule or actuate these fractions.
- Transition will be gross.

### Option B: Add a schedule _in addition to_ the single fractional input

In this case we would have both the numerical fraction and a schedule input for each heating and cooling fraction. Throw an error if both were provided. Assume 1.0 if neither are provided.

#### Pros

- Transition is less gross.
- the majority of users can set a single number without the hassle of defining a constant schedule.
- Schedules can already be actuated by EMS.
- The fractions can be supplied as a schedule (for example pre-calculated based on weather data) and EMS can be avoided in some cases.

#### Cons

- It's kind of an icky pattern to start using. I don't think this is being doing anywhere else in EnergyPlus.
- It could be confusing which inputs need to be provided for an already confusing input.

### Option C: Add an EMS actuator for sequential load fraction

#### Pros

- Nothing changes for the vast majority of users who don't need this edge case feature.
- No transition necessary.

#### Cons

- Can you put an EMS actuator on something inside an extensible list?
- Can't just define a schedule, must use EMS.

## Testing/Validation/Data Sources ##

TBD

## Input Output Reference Documentation ##

TBD

## Input Description ##

TBD

## Outputs Description ##

TBD

## Engineering Reference ##

TBD

## Example File and Transition Changes ##

TBD

