# ASHRAE 205 Coils

**Tanaya Mankad, Neal Kruis; Big Ladder Software**

## Overview

ASHRAE Standard 205 defines common data models and serialization formats for facility equipment performance data needed for engineering applications such as energy simulation.  The formats allow automated exchange among data sources (manufacturers), simulation models, and other engineering applications. The formats and procedures specified in the standard are developed by SPC (Standard Project Committee) 205 under ASHRAE and ANSI consensus processes. SSPC-205 membership includes equipment manufacturers, application software developers, and engineering practitioners.

The initial edition of the standard was published near the beginning of 2023; an overview can be found at http://data.ashrae.org/standard205/.

205 chillers (RS0001) were added to EnergyPlus in 22.2. In this NFP, we propose to add support for RS0004 (Air-to-Air Direct Expansion Refrigerant System). The current scope of this RS is limited to cooling DX coils, but future adenda will accommodate heating DX coils as well.

## Implementation

ASHRAE 205 coil performance will be indicated through a new object, `Coil:DX:ASHRAE205:Performance`, which will be referenceable from `Coil:Cooling:DX` object (and the future `Coil:Heating:DX` object). The current `Coil:[Cooling]:DX:CurveFit:Performance` object and the new performance object will both derive from a new base class that encapsulates common functionality and isolates the performance calculation. (Making this structural change will ease the transition to a future Coil:Heating:DX:Curvefit:Performance object, should a non-205 performance imlementation ever be desired.)

Standard 205 representations are stored in either human-readable JSON or serialized CBOR (Concise Binary Object Representation) format. libtk205 (Toolkit 205) library will populate the internal EnergyPlus class data.

### Calculations

One of the advantages of ASHRAE Standard 205 is that available equipment operating conditions must be provided as raw performance maps in a representation. These performance maps are straightforward to arrange in a tabular format, which the libtk205 library passes to the same multi-dimensional interpolation engine ([btwxt](https://github.com/bigladder/btwxt)) which is already used in EnergyPlus to calculate performance at different operating conditions. Rather than being extracted from a regression curve, which can produce physically unrealistic results near their boundaries and can fail to capture internal inflections, equipment performance is interpolated within the closest available data to the operating point. The performance calculation will be embedded in the `simulate()` function of the DX coil class.

## Testing

In order to test the new `Coil:DX:ASHRAE205:Performance` object, we will create a new model based on an existing one with `Coil:Cooling:DX:Curvefit:Performance`. A new ASHRAE 205 representation file will be created using performance characteristics from the known model, using its performance curves to back-calculate the tabular parameter and lookup information required by the new representation. We will ensure results between the two files are similar, within reason.

## IDD Changes and Transition

We will add a new object `Coil:DX:ASHRAE205:Performance`.

```
Coil:DX:ASHRAE205:Performance,
       \memo DX coil performance specification referencing an ASHRAE Standard 205 compliant representation
       \memo for air-to-air direct expansion refrigerant system (Representation Specification RS0004). As
       \memo RS0004 files are intended to support both heating and cooling performance, this object may
       \memo referenced by the Coil:Cooling:DX and the corresponding Coil:Heating:DX object (planned for
       \memo future addition).
       \min-fields 2
  A1,  \field Name
       \required-field
       \type alpha
       \reference DXCoolingPerformanceNames
       !\reference DXHeatingPerformanceNames
  A2,  \field Representation File Name
       \note The name of the ASHRAE 205 RS0004 (air-to-air direct expansion refrigerant system) representation file
       \type alpha
       \retaincase
       \required-field
  A3,  \field Performance Interpolation Method
       \type choice
       \key Linear
       \key Cubic
       \default Linear
  N1,  \field Rated Total Cooling Capacity
       \note Not yet implemented / reserved for future use. Full load cooling capacity at AHRI 210/240 "A" test conditions.
       \note Used to scale representation data.
       \type real
       \units W
       \minimum> 0.0
       \autosizable
       \default autosize
  N2;  \field Rated Steady-State Heating Capacity
       \note Not yet implemented / reserved for future use. Full load heating capacity at AHRI 210/240 "H1" test conditions.
       \note Used to scale representation data.
       \type real
       \units W
       \minimum> 0.0
       \autosizable
       \default autosize
```

## Documentation

Much of the content above will be modified to describe the feature in "Input Output Reference", and details of the aggregation calculations in "Engineering Reference".
