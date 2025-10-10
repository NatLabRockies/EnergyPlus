# CondFD Heat Sink Actuators

**Matt Mitchell, NREL**

- October 26, 2021 - Initial Draft
- November 18, 2021 - Rev 01

## Overview

Last year, internal NREL users requested the addition of EMS actuators to the CondFD surface layers to control material layer conductivity and specific heat values. This project was accomplished and completed. Now, in a continuation of their work, they have requested adding EMS actuators to add electrical heat gain to the material layers. The only addition to EnergyPlus will be the addition of this actuator, which will be a one-directional way for EMS users to specify electrically-driven heat addition to material layers.

## Implementation

The new EMS actuator will leverage the capability already built into the CondFD code through the `ConstructionProperty:InternalHeatSource` object. This actuator will be tied to the heat flux input already defined by this object. The heat source will only be allowed to operate by adding heat to the node, i.e. one-directional. Negative values will be flagged with warning or cause the simulation to issue a fatal error.

When the `ConstructionProperty:InternalHeatSource` is present and adding heat to the surface material layers, this EMS actuator will *add* heat to what has already been defined by that object. Currently, there are no plans to warn the user that both the EMS actuator and the `InternalHeatSource` objects are tied to the same material layer.

New output variables will be added to track the heat source layers. They are listed here:

- "CondFD Heat Source Power After Layer N \[W\]" - This is the heating power added (flux * surf_area).
- "CondFD Heat Source Energy After Layer N \[J\]" - This is the energy of heating power added.

The heat source must be inserted between two material layers, so the actuator (like the `InternalHeatSource` object) will not be available for the final material layer.

## Testing

The "PythonPlugin1ZoneUncontrolledCondFD" test files will be modified to include usage of the new actuator. This file will be used to validate the model to ensure that energy is conserved and that the actuators are working as expected.

A series of unit tests will also be added to improve coverage on the CondFD code.

## Documentation

Documentation will be added in the EMS Reference describing the usage and limits of this actuator.

## IDD Changes and Transition

None required for the new actuator; however, I'd recommend we unify the format of these output variables. Current output variable names and proposed changes are listed below.

| Current                                             | Proposed                                  |
|-----------------------------------------------------|-------------------------------------------|
| CondFD Surface Temperature Node N                   | No change                                 |
| CondFD Surface Heat Flux Node N                     | No change                                 |
| CondFD Phase Change State N                         | CondFD Phase Change State Node N          |
| CondFD Phase Change Previous State N                | CondFD Phase Change Previous State Node N |
| CondFD Phase Change Node Temperature N              | CondFD Temperature Node N                 |
| CondFD Phase Change Node Conductivity N             | CondFD Conductivity Node N                |
| CondFD Phase Change Node Specific Heat N            | CondFD Specific Heat Node N               |
| CondFD Surface Heat Capacitance Outer   Half Node N | No change                                 |
| CondFD Surface Heat Capacitance Inner   Half Node N | No change                                 |
