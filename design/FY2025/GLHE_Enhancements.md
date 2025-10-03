# Some Groundwork for EnergyPlus

A couple enhancements are coming for EnergyPlus related to ground heat exchangers:
 - An update to the GLHE physics model that cleans up an instability/unrealistic response
 - The inclusion of GHEDesigner as a part of the EnergyPlus package to perform GLHE borehole size and design

# Updated GLHE Model

The updated model is from Matt Mitchell.
It was originally written in Python by him as part of his PhD, and included input processing, output management, a wide array of simulation options, etc.
We then took some time to trim this down to a minimal version to eventually be put into EnergyPlus.
We then took this minimal Python version and converted it over to C++ in preparation for getting it into EnergyPlus.
After checking the results matched the Python version, it was considered ready for implementation.

The next step is preparing the EnergyPlus code to accept the component model.
One major clean up I'm wanting to do is break the large ground heat exchanger source, header, and test files into a set of smaller files, each with a dedicated model.
Right now, the file contains a base class, a vertical model class, and a slinky heat exchanger class.  
I will be making a new directory in the src/ folder called GroundHeatExchangers, where I will put Base.hh, Vertical.hh, and Slinky.hh files, along with accompanying source files.
I will also update the unit tests to match this structure.
Eventually it would be cool to bring in any other specific ground heat exchanger objects, but it's not necessary here.
Making this change alone will make it much easier to modify the physics of the vertical borehole model.

Usual EnergyPlus Aspects
- **Inputs** I don't believe I'll have any input changes, but if I do, I will of course fix up any impacted IDFs, unit tests, and add transition where necessary
- **Testing** I'll verify result changes are within a small, reasonable tolerance, and update any files accordingly.
- **Documentation** I'll update the documentation to include Matt's changes, and add a citation to his work

# GHEDesigner Implementation

[GHEDesigner](https://github.com/BETSRG/GHEDesigner) is a Python package that is self-described as "A Flexible and Automatic Ground Heat Exchanger Design Tool".
It can design and size borehole fields given ground or building loads.
A recent refactor cleaned up the codebase and input structure to more easily call it purely for g-function generation.
For the sake of EnergyPlus implementation, a subset of the capabilities will be exposed through new input fields, and a call will be made to execute GHEDesigner's command line interface.
GHEDesigner inputs and outputs are in json, so EnergyPlus will use nlohmann::json to create the input file and get results from the output file.
An example GHEDesigner input that calculates a g-function from a pre-defined borehole field is provided here:

```json
{
  "version": 2,
  "topology": [
    {
      "type": "ground-heat-exchanger",
      "name": "ghe1"
    }
  ],
  "fluid": {
    "fluid_name": "WATER",
    "concentration_percent": 0,
    "temperature": 20
  },
  "ground-heat-exchanger": {
    "ghe1": {
      "flow_rate": 0.5,
      "flow_type": "BOREHOLE",
      "grout": {
        "conductivity": 1,
        "rho_cp": 3901000
      },
      "soil": {
        "conductivity": 2,
        "rho_cp": 2343493,
        "undisturbed_temp": 18.3
      },
      "pipe": {
        "inner_diameter": 0.03404,
        "outer_diameter": 0.04216,
        "shank_spacing": 0.01856,
        "roughness": 0.000001,
        "conductivity": 0.4,
        "rho_cp": 1542000,
        "arrangement": "SINGLEUTUBE"
      },
      "borehole": {
        "buried_depth": 2,
        "diameter": 0.14
      },
      "pre_designed": {
        "arrangement": "RECTANGLE",
        "H": 150,
        "spacing_in_x_dimension": 4.5,
        "spacing_in_y_dimension": 5.5,
        "boreholes_in_x_dimension": 4,
        "boreholes_in_y_dimension": 8
      }
    }
  }
}
```

It is a relatively small amount of inputs, just defining fluid, grout, soil, and pipe thermal properties, and then geometric arrangement of the borehole field and pipe layout in the borehole.
EnergyPlus already has the ability to do this with the fairly recent implementation of `cpgfunction`.
In order to add in borehole field design, the user must provide additional inputs for geometric constraints of the field, and design parameters, such as:

```json
{
  "geometric_constraints": {
    "length": 100,
    "width": 100,
    "b_min": 3,
    "b_max": 10,
    "method": "RECTANGLE"
  },
  "design": {
    "max_eft": 35,
    "min_eft": 5,
    "max_height": 135,
    "min_height": 60,
    "max_boreholes": 200
  }
}
```

The implementation will achieve two major goals:

- EnergyPlus will be able to drop the `cpgfunction` third party package and instead call GHEDesigner to generate the g-function for a specific borehole field.
- EnergyPlus will be able to size a borehole field for a specific load.
  This will eliminate the need for the user to exactly specify the borehole field layout, and instead let it be auto-sized based on load.
  There will be more inputs required around the design conditions and geometric constraints.
  There is also a complication for situations where the ground loop heat exchanger is not the only supply component, as it will not know how much of the load the heat exchanger is expected to support.
  For now, I suspect we will just assume the ground heat exchanger is meeting the entire loop demand.

# Implementation

This section describes the implementation for both the new GLHE model and the GHEDesigner work.
Defining a GLHE simulation now involves the following inputs:

- A `GroundHeatExchanger:System` which specifies
  - the physical placement of the heat exchanger on the plant loop
  - some thermal properties
  - references other objects for ground temperature, response factors, and optionally vertical borehole definitions.
- There are multiple `Site:GroundTemperature:Undisturbed:` objects for defining the undisturbed ground temperature boundary condition
- The response factors are defined in the `GroundHeatExchanger:ResponseFactors` object, which specifies
  - physical borehole field properties
  - response factor terms
  - references a properties object for borehole specific properties
- The `GroundHeatExchanger:Vertical:Properties` object allows specifying borehole geometry, and grout and pipe properties 
- The borehole specifications may be done in two ways:
  - A `GroundHeatExchanger:Vertical:Array` object allows quickly specifying a rectangular borehole field of N x M boreholes with spacing B.
  - The user may provide individual `GroundHeatExchanger:Vertical:Single` objects that each define their own X, Y positions to allow flexible borehole field arrangement.

In order to complete these GLHE enhancements, the following modifications are anticipated:

- Add `ghedesigner` to the Python ecosystem in the EnergyPlus package 
- Convert the call to `cpgfunction` into a call to GHEDesigner, validating the results along the way.
- Eliminate the `cpgfunction` package from the repository
- Add IDD elements

# Future Notes

The GLHE model supports a wide array of borehole geometry and piping options.
For now we will support the single u-tube geometry that is already supported by current inputs.
In the future, adding more inputs would allow calling the GLHE model to simulate more diverse ground heat exchanger simulations.
