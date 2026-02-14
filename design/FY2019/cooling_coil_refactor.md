# Cooling Coil Refactor

Through the years of EnergyPlus, the ability to simulate DX cooling coils has been improved in various ways.
One key way is the ability to simulate multiple compressor speeds or operating modes.
What originally started as the `COIL:DX:Doe2` object, ultimately became a long list of DX cooling coil types:

- Coil:Cooling:DX:SingleSpeed
- Coil:Cooling:DX:TwoSpeed
- Coil:Cooling:DX:MultiSpeed
- Coil:Cooling:DX:VariableSpeed
- Coil:Cooling:DX:TwoStageWithHumidityControl

This group of coils have varying interoperability with parent objects, and share varying amounts of underlying code.
This has caused significant uncertainty for users who attempt to use these coils.

## Justification

It is clear that the list of coils, interoperability confusion, and maintenance burden of non-dry code, makes this coil refactor a highly sought after task.
Bringing the list down to a single coil type that has widespread interoperability will improve the user experience, increasing adoption of EnergyPlus as a whole.
This task has support from BTO and all major stakeholders have shown interest in this task.

## High Level Design

A new DX cooling coil object will be created: `Coil:Cooling:DX`.
This object is the singular physical piece of equipment to be placed on an air system, it captures only topology (nodes) and very high level information.
Child objects will be created that will capture the simulation and operation characteristics.
Separating out the topology from the modeling will allow easier swapping of models within a simulation.

Initially, only a single performance modeling option will be available, and it will be defined in the object `Coil:Cooling:DX:Performance:CurveFit`.
This performance model is based on the current curve-fitted coil performance model that is used in the existing coil.
The purpose of this object is to define rated parameters and other information that apply to all of the potential operating modes and compressor speeds.

For the curve fit performance model, the performance object will allow entry of up to two operating modes.
In almost all simulations, the user will only enter one mode, but in the case of the TwoStageWithHumidityControl, a second, enhanced mode, can be entered.
The purpose of the mode object is to define the high level conditions for a single operating mode.

For the curve fit performance model, the mode object will allow entry of any number of speeds.
The speed object will define the curves and other performance parameters for a single speed operation.

For a single speed DX coil, the user will then have one topology object, one performance object, one mode object, and one speed object.
The number of inputs is much not higher than the original single speed object, but the inputs are spread out into different objects, each with a specific role.
For a two speed coil, the user will simply need a second speed object.
For a multi-speed coil, the user will enter the appropriate number of speeds, and the coil should be set to discrete speed mode (no interpolation).
For a variable speed coil, the user will enter the appropriate number of speeds, and the coil will interpolate between them to capture the variable operation effect.

## Questions and Answers

There are some big picture questions that can be answered here:

### What about transition/deprecation?

Transition is in place, we could talk about all the inputs, testing, etc.
Also, we will be getting rid of the other Coil:Cooling:DX:* objects.

### What parent object support will there be?

What parent objects?

### What about airflow network?

Discuss what AFN currently supports and what we will support now.

### Example files?

We have a large assortment of example files

### Will there be diffs?

There could be diffs, but they are expected to be extremely small.
Any significant diffs must be accompanied with a full justification.

However, as the coil work is progressing, sometimes diffs are encountered that are not because of a flaw in the new coil, but because a bug was uncovered in the original code.
A branch `CoilRefactorBugFixes` was added to the upstream repository as a place to drop bug fixes in the upstream codebase.
When the coil branch is ready for review, it can use this branch as a baseline (if it isn't already merged into develop) to avoid unrelated diffs from showing up.

## Detailed Design

Discuss the code changes, classes, testing strategy, anything interesting that will be done

## Status

Here we can talk about the status of the work in terms of specific targeted tasks:

### Matching Single Speed

Show plots and relevant stuff

### Matching Two Speed

Show plots and relevant stuff

### Matching Multi-Speed

Show plots and relevant stuff

### Matching Variable Speed

Show plots and relevant stuff

### Matching Two Mode

Show plots and relevant stuff

### Condenser Types

Diffs for Dry vs Evaporative condenser

### Appendage Pieces

Condensate collection tank, Evaporative condenser supply water tank, basin heater, crankcase heater

### Unit Tests

Status, coverage

### Documentation

IO, Engineering Ref

### Auxiliary Tools

ExpandObjects, other?

## Other Notes

- During implementation of the condensate collection tank, it was revealed that even the current develop branch does not seem to add condensate to the tank.
  This was found to be due to the timing of the initialization of the tank flow rate in the water manager.
  The flow is getting reset to zero before the coil's value can be used in the calculation.
  This was not fixed in either develop or this branch as it could interfere with diffs and the progress of the coil refactor.
