# NFP: HAMT Model Airflow Enhancements Part I #

## Justification for New Feature ##
The current HAMT model does not include air leakage in its calculation of the moisture deposition. Air movement as a moisture transport mechanism is typically far more important than vapor diffusion. Air leaks through walls, roofs, and floors can have the most damaging effect on the durability of a house. Uncontrolled airflow through the shell not only carries moisture into framing cavities, causing mold and rot, but it also can account for a huge portion of a home's energy use and can cause indoor air quality problems.

## Overview ##
The modification adds an internal moisture source in HAMT components due to air flow through the component. The moisture source is attached to a user-selected layer inside the component. Input options to determine the air flow are added, including a user-defined air flow, air flow determined from stack height, air permeance of the component and ventilation overpressure as well as are flow computed with the AirflowNetwork model.

These modifications to EnergyPlus are part of a larger effort to enhance the moisture transport capabilities of EnergyPlus. This NFP describes the first two steps of this process:

Step 1 - Enable air leakage to be applied as a source/sink at any arbitrary point inside the simulation model. Manual inputs of air flow rates is required.

Step 2 - Allow air leakage rates to be controlled by air pressure gradients induced by wind forces, temperature differences and ventilation system.

The necessary inputs will be developed to allow both user specified airflows and airflows calculated using the AirflowNetwork feature. The HAMT model calculations will be modified to account for this additional mode of transport.

## Approach ##
The HAMT model subdivides a wall into set of cells and computes the movement of moisture through the wall. Equation (3.36) in the Engineering Reference describes the calculation that is done. A source term that adds moisture to a specific cell is added to this equation of the form

```
S_i A_ij
```

where

```
S_i = m_dot(c_src - c_i_sat)
```

and

`m_dot` - air flow through the envelope per unit area (m^3/(m^2 h))

`c_src` - water vapor concentration in the source volume (kg/m^3)

`c_i_sat` - water vapor saturation concentration for cell `i` (kg/m^3)

## Engineering Reference ##
The HAMT section of the Engineering Reference will be modified to include and describe the new source term.

## I/O Reference ##
The I/O Reference will be modified to describe the additional moisture source inputs. A moisture source object
is attached to a particular layer in a manner similar to the way that heat sources are specified. The current proposal
suggests transitioning all `Construction:InternalSources` objects into regular construction objects. This does raise some
issues: (a) this approach isn't strictly a component model and (b) multiplicity of sources. However, this approach is
cleaner from a data model perspective in that it no longer requires a special construction to use these models.

`Construction:InternalSources` is removed and source information (both thermal and moisture) is moved out into separate objects.

```
ConstructionProperty:InternalHeatSource,
       \memo Internal heat source to be attached to a construction layer
       \memo Enter the name for the heat source
  A1,  \field Name
       \required-field
       \type alpha
       \reference InternalHeatSourceNames
  A2,  \field Construction Name
       \type object-list
       \object-list ConstructionNames
  N1,  \field Thermal Source Present After Layer Number
       \required-field
       \type integer
       \minimum 1
       \note refers to the list of materials which follows
  N2,  \field Temperature Calculation Requested After Layer Number
       \required-field
       \type integer
       \note refers to the list of materials which follows
  N3,  \field Dimensions for the CTF Calculation
       \required-field
       \type integer
       \minimum 1
       \maximum 2
       \note 1 = 1-dimensional calculation, 2 = 2-dimensional calculation
  N4;  \field Tube Spacing
       \required-field
       \type real
       \units m
       \note uniform spacing between tubes or resistance wires in direction
       \note perpendicular to main intended direction of heat transfer

ConstructionProperty:InternalMoistureSource,
      \memo HeatBalanceAlgorithm = CombinedHeatAndMoistureFiniteElement solution algorithm only.
      \memo Adds moisture source to selected layer inside the component.
      \memo Has no effect with other HeatBalanceAlgorithm solution algorithms
  A1, \field Name
      \required-field
      \type alpha
      \reference InternalMoistureSourceNames
  A2,  \field Construction Name
       \type object-list
       \object-list ConstructionNames
  N1,  \field Moisture Source Present In Layer Number
       \required-field
       \type integer
       \minimum 1
       \maximum 10
       \note Layer number to which the moisture source should be applied (from outside to inside)
       \note refers to the list of layers specified for the construction applied to the above surface
       \note If a source occurs on a layer surface, the layer should be split into two layers (thin where source occurs) and the source added to the thin layer
  A3, \field Source Type
      \required-field
      \type choice
      \key UserDefined
      \key StackAndOverPressure
      \key AirflowNetwork
      \note Type of Moisture Source Calculation
      \note UserDefined: User defined input of air flow through component
      \note StackAndOverPressure: Calculation of dynamic air flow through component due to stack effect and ventilation overpressure (according to Kuenzel, Zirkelbach and Schafazcek 2012)
      \note AirflowNetwork: Calculation of dynamic air flow through component with AirflowNetwork model
  N2, \field Air Flow Rate
      \units m/s
      \type real
      \minimum -0.001
      \maximum 0.001
      \note Air flux density in m3 per m2 and second.
      \note Only required for Source Type UserDefined.
  N3, \field Stack height
      \units m
      \type real
      \minimum 0
      \maximum 100
      \note Height of the connected airspace in the building envelope element
      \note Only required for Source Type StackAndOverPressure.
  N4, \field Component Air Permeance
      \units m/s
      \type real
      \minimum 0
      \maximum 1
      \note Moisture specific air permeance of the component in m3 per m2, hour and Pascal.
      \note Only required for Source Type StackAndOverPressure.
  N5; \field Mechanical Ventilation Overpressure
      \units Pa
      \type real
      \minimum 0
      \maximum 100
      \note Constant mechanical ventilation overpressure in Pascal.
      \note Only required for Source Type StackAndOverPressure.
```

## Output Details ##
No additional outputs will be added during this phase of the work.

## Example File and Transition Changes ##
An example file will be created or modified to demonstrate the new feature.

## Discussion and Comments
TBD

## References ##
NA
