SPECIFICATION OF INTERIOR AND EXTERIOR SIDE VALUES OF MATERIAL/CONSTRUCTION RADIANT PROPERTIES
================

Rick Strand, University of Illinois at Urbana-Champaign

 - First Draft: June 23, 2026
 - Second Draft: June 30, 2026
 - Third/Final Version: July 9, 2026

## Justification for New Feature ##

In EnergyPlus, there is a built-in assumption for opaque building materials that the radiant absorptance for long, visible, and solar wave lengths are constant throughout the material and thus the same at both sides.  This definition is problematic in situations where this is not the case.  One particular situation is a residential roof assembly that consists of shingles and plywood.  To avoid issues with the CTF calculations, users report needing to combine those two layers into a single layer.  In this situation, it is not possible to change the absorptances.  This is particularly problematic when a radiant barrier is used at the interior.  The situation is also potentially an issue when say a single layer of concrete is used for as a construction and one side has a reflective paint on one side or the other.  While a single layer of concrete is not as common, the residential roof is fairly common and there needs to be a method for allowing different absorptances on both faces of a material.  Window glass already has this capability.

## E-mail and Conference Call Conclusions ##

**EnergyPlus Technicalities Call, June 24, 2026**: Given that this was a new NFP and was posted the day before the Technicalities Call, there were not a lot of comments on this proposal.  One comment from Scott Horowitz was to make sure that the ability to specify both exterior and interior face absorptivities in EMS is addressed.  EMS was not mentioned in the original draft proposal.  To address this concern, a section on EMS has been added below in Approach 1.  The only opinions expressed regarding which approach to take were in support of Approach 1, though admittedly this was a small sample and there was not full attendance for the call.

**EnergyPlus Technicalities Call, July 8, 2026**: Three comments were brought up either during this call or after it was completed via comments on GitHub.  First, there was a question related to how this will be defined for interior walls which blur the line somewhat between inside and outside facing directions.  This will be investigated during implementation to determine the best approach to take.  Second, the proposed IO Ref sections for the new parameters in Approach 1 included some sloppy cut and paste.  The defaults for the new, optional parameters (inside absorptances) should default to the exterior values both in the code and in the documentation.  This is corrected below.  Third, the naming convention proposed does not follow the established pattern in EnergyPlus which uses "inside face" and "outside face".  This is also now corrected below.  With these corrections, there was general consensus that this can now proceed to implementation as proposed below as "Approach 1".


## Overview ##

The justification listed above is synthesized from Issue 11393 in GitHub: Move absorptance fields to Construction to better model roofs/radiant barriers/etc.  The desire is to provide a method for allowing EnergyPlus to account for a different absorptance value at each of the two facing directions (interior and exterior) of a material or construction.  There are multiple potential approaches to this problem.  This NFP documents two potential solutions as has been already discussed in the comments for Issue 11393.

## Approach ##

Approach 1: Modification of the Opaque Material Definitions

This approach looks to increase the parameters in a regular and no mass material definition.  Currently, these two input elements only require a single absorptance in the long-wavelength, solar, and visible spectrum.  This could be enhanced by adding three new input fields and differentiating between the inside and outside values of these three material properties.  For existing files where only one group (exterior values) were provided, the interior values will be assumed to default to the exterior values.  Thus, all existing files will still work as they currently do, and the user will not be required to provide two sets of values.  This will be achieved by setting the inside values equal to the outside values when the input is read into EnergyPlus.  When the user specifies different values for a particular material, then EnergyPlus will store different values.  When the absorptance value for the exterior face of a surface are needed, the exterior values of the outside most material layer will be used.  Likewise, when the absorptance value for the interior face are needed, the interior values of innermost material layer will be used.

Within the current EnergyPlus code, the values for interior and exterior absorptance are set when the constructions are read from input within the construction data structure.  This is possible because the material properties are read in before constructions.  Thus, this assignment would need to be modified to set the correct (exterior vs. interior) values.  Within the InitSurfaceHeatBalance routine, these are then set to surface level variables which are then used within the heat balances.

Beyond this, there are two other places where absorptance handling must be resolved.  In movable insulation, there are built-in assumptions about only one absorptance value and the information is taken from the material data structure.  This would need to be updated to reflect the new dual data values.

There is also the possibility that the absorptance values can be variable as outlined in the MaterialProperty:VariableAbsorptance input syntax.  This overrides the values defined in the standard material definition and also assumes the same value for both interior and exterior.  This would need to be upgraded as well so that it was possible to define different approaches for both the interior and exterior side.

In addition, EMS currently supports the alteration of the three absorptances.  Thus, it would be useful to make modifications to the EMS code to allow the control of both the interior and exterior face absorptances as well.  The existing EMS variables will need to be renamed and three new variables will need to be added to the list of controllable quantities in EnergyPlus.  In addition, there will likely be some minor documentation enhancements to note the ability to specify absorptances at both faces.

Both of the phases of this approach would require careful testing as well as a look at the documentation to make sure it was clear that there is no longer an assumption that the absorptances are the same on both sides of a material.

This approach would be handled as three phases:

1. Expansion of Material and Material:NoMass Definitions
2. Expansion of Variable Absorptance to Account for Interior and Exterior Differences
3. Expansion of Interior and Exterior Differences to EMS

These two phases could potentially be handled concurrently since they deal with different sections of the code.

Approach 2: Addition of a New Construction Type(s)

This approach advocates that a new construction type Construction:WithAbsorptances be added.  A new construction with this name (or something similar) would be identical to the standard construction element except with additional values for exterior and interior absorptances (thermal, solar, visible).  These values would override the material layer values.  In this way, a different absorptance could be added to a construction.

This change would require an addition to the InitSurfaceHeatBalance routine or a routine called from it.  It would check a construction flag and then reset the absorptances based on what is on the construction.  One question here is what happens when variable absorptance is used.  In this approach, one potential strategy is to first set the absorptances based on the material, overwrite them if the new construction is used, and then allow the variable absorptance to override that if it is present.  This would fit best with the current code as the current order of code can be maintained with the additional level of adjustments with the new construction type.

The expansion of variable absorptances as described above for Approach 1, Phase 2 could either be done in addition to this or the addition of a new construction could simply be used to cover most cases that led to this NFP being developed.

Also, for both Approach 1 and Approach 2, it might be helpful/necessary to add a new output variable for surface absorptances at the interior and exterior if this does not already exist.

## Testing/Validation/Data Sources ##

Testing will consist of running various input files using the existing model with different absorptances and one with modified values and compare this to the new code using the modified values.  This would verify that the changes are doing what they are supposed to do.

## Input Output Reference Documentation ##

If Approach 1 is chosen, one would need to add to the current material and no-mass material definitions already existing in the IO Reference.  The names of the current absorptances would have an "outside" added to the field name and description and additional fields would be named with "inside" in the field name (see below in Input Description)

## Input Description ##

### Approach 1, Phase 1: This would require modification of the existing Material and Material:NoMass objects.  Currently, there is the following information in the IO Reference:

1.9.3.1.7 Field: Thermal Absorptance

The thermal absorptance field in the Material input syntax represents the fraction of incident long wavelength (>2.5 microns) radiation that is absorbed by the material. This parameter is used when calculating the long wavelength radiant exchange between various surfaces and affects the surface heat balances (both inside and outside as appropriate). For long wavelength radiant exchange, thermal emissivity and thermal emittance are equal to thermal absorptance. Values for this field must be between 0.0 and 1.0 (with 1.0 representing "black body" conditions). The default value for this field is 0.9.

1.9.3.1.8 Field: Solar Absorptance

The solar absorptance field in the Material input syntax represents the fraction of incident solar radiation that is absorbed by the material. Solar radiation (0.3 to 2.537 microns) includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident solar radiation absorbed by various surfaces and affects the surface heat balances (both inside and outside as appropriate). If solar reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. The default value for this field is 0.7.

1.9.3.1.9 Field: Visible Absorptance

The visible absorptance field in the Material input syntax represents the fraction of incident visible wavelength radiation that is absorbed by the material. Visible wavelength radiation (0.37 to 0.78 microns weighted by photopic response) is slightly different than solar radiation in that the visible band of wavelengths is much more narrow while solar radiation includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident visible radiation absorbed by various surfaces and affects the surface heat balances (both inside and outside as appropriate) as well as the daylighting calculations. If visible reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. The default value for this field is 0.7.

This would need to change to the following:

1.9.3.1.7 Field: Thermal Absorptance Outside Face

The thermal absorptance field in the Material input syntax represents the fraction of incident long wavelength (>2.5 microns) radiation that is absorbed by the material. This particular field is for the exterior side or face only. This parameter is used when calculating the long wavelength radiant exchange between various surfaces and affects the outside surface heat balances. For long wavelength radiant exchange, thermal emissivity and thermal emittance are equal to thermal absorptance. Values for this field must be between 0.0 and 1.0 (with 1.0 representing "black body" conditions). The default value for this field is 0.9.

1.9.3.1.8 Field: Solar Absorptance Outside Face

The solar absorptance field in the Material input syntax represents the fraction of incident solar radiation that is absorbed by the material. This particular field is for the exterior side or face only. Solar radiation (0.3 to 2.537 microns) includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident solar radiation absorbed by various surfaces and affects the outside surface heat balances. If solar reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. The default value for this field is 0.7.

1.9.3.1.9 Field: Visible Absorptance Outside Face

The visible absorptance field in the Material input syntax represents the fraction of incident visible wavelength radiation that is absorbed by the material. This particular field is for the exterior side or face only. Visible wavelength radiation (0.37 to 0.78 microns weighted by photopic response) is slightly different than solar radiation in that the visible band of wavelengths is much more narrow while solar radiation includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident visible radiation absorbed by various surfaces and affects the outside surface heat balances as well as the daylighting calculations. If visible reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. The default value for this field is 0.7.

1.9.3.1.10 Field: Thermal Absorptance Inside Face

The thermal absorptance field in the Material input syntax represents the fraction of incident long wavelength (>2.5 microns) radiation that is absorbed by the material. This particular field is for the interior side or face only. This parameter is used when calculating the long wavelength radiant exchange between various surfaces and affects the inside surface heat balances. For long wavelength radiant exchange, thermal emissivity and thermal emittance are equal to thermal absorptance. Values for this field must be between 0.0 and 1.0 (with 1.0 representing "black body" conditions). This is an optional field, and the default value for this field is the value entered for the Thermal Absorptance Outside Face entered in the field shown above.

1.9.3.1.11 Field: Solar Absorptance Inside Face

The solar absorptance field in the Material input syntax represents the fraction of incident solar radiation that is absorbed by the material. This particular field is for the interior side or face only. Solar radiation (0.3 to 2.537 microns) includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident solar radiation absorbed by various surfaces and affects the inside surface heat balances. If solar reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. This is an optional field, and the default value for this field is the value entered for the Solar Absorptance Outside Face entered in the field shown above.

1.9.3.1.12 Field: Visible Absorptance Inside Face

The visible absorptance field in the Material input syntax represents the fraction of incident visible wavelength radiation that is absorbed by the material. This particular field is for the interior side or face only. Visible wavelength radiation (0.37 to 0.78 microns weighted by photopic response) is slightly different than solar radiation in that the visible band of wavelengths is much more narrow while solar radiation includes the visible spectrum as well as infrared and ultraviolet wavelengths. This parameter is used when calculating the amount of incident visible radiation absorbed by various surfaces and affects the inside surface heat balances as well as the daylighting calculations. If visible reflectance (or reflectivity) data is available, then absorptance is equal to 1.0 minus reflectance (for opaque materials). Values for this field must be between 0.0 and 1.0. This is an optional field, and the default value for this field is the value entered for the Visible Absorptance Outside Face entered in the field shown above.


Similar changes would need to be made to the Material:NoMass object.

### Approach 1, Phase 2: This would require modification of the existing MaterialProperty:VariableAbsorptance object.  Currently, there is the following information in the IO Reference:

1.9.11.1.3 Field: Control Signal

It can be one of the following: surface temperature, surface received solar radiation, zone heating/cooling mode, or a schedule. If the control signal is "Scheduled", then a schedule needs to be specified in "Thermal Absorptance Schedule Name" or "Solar Absorptance Schedule Name". The schedule value will override the material absorptance value. If the control signal is not "Scheduled", then the control signal value at the target surface or zone will decide the absorptance, based on the function referenced in "Thermal Absorptance Function Name" or "Solar Absorptance Function Name". If not specified, the control signal will be assumed to be surface temperature.

1.9.11.1.4 Field: Thermal Absorptance Function Name

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the thermal absorptance.

1.9.11.1.5 Field: Thermal Absorptance Schedule Name

The name of a Schedule object that overwrites the material thermal absorptance. If neither this field or the previous field are defined, then the thermal absorptance is assumed to be constant.

1.9.11.1.6 Field: Solar Absorptance Function Name

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the solar absorptance.

1.9.11.1.7 Field: Solar Absorptance Schedule Name

The name of a Schedule object that overwrites the material solar absorptance. If neither this field or the previous field are defined, then the solar absorptance is assumed to be constant.

To accommodate information at both sides or faces of the material, the above documentation would need to change to:

1.9.11.1.3 Field: Control Signal Outside Face

For the outside or exterior face of the material, the control signal can be one of the following: surface temperature, surface received solar radiation, zone heating/cooling mode, or a schedule. If the control signal is "Scheduled", then a schedule needs to be specified in "Thermal Absorptance Schedule Name" or "Solar Absorptance Schedule Name". The schedule value will override the material absorptance value. If the control signal is not "Scheduled", then the control signal value at the target surface or zone will decide the absorptance, based on the function referenced in "Thermal Absorptance Function Name" or "Solar Absorptance Function Name". If not specified, the control signal will be assumed to be surface temperature.

1.9.11.1.4 Field: Thermal Absorptance Function Name Outside Face

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the thermal absorptance at the outside or exterior side or face.

1.9.11.1.5 Field: Thermal Absorptance Schedule Name Outside Face

The name of a Schedule object that overwrites the material thermal absorptance at the outside or exterior side or face. If neither this field or the previous field are defined, then the thermal absorptance is assumed to be constant.

1.9.11.1.6 Field: Solar Absorptance Function Name Outside Face

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the solar absorptance at the outside or exterior side or face.

1.9.11.1.7 Field: Solar Absorptance Schedule Name Outside Face

The name of a Schedule object that overwrites the material solar absorptance at the outside or exterior side or face. If neither this field or the previous field are defined, then the solar absorptance is assumed to be constant.

1.9.11.1.8 Field: Control Signal Inside Face

For the inside or interior face of the material, the control signal can be one of the following: surface temperature, surface received solar radiation, zone heating/cooling mode, or a schedule. If the control signal is "Scheduled", then a schedule needs to be specified in "Thermal Absorptance Schedule Name" or "Solar Absorptance Schedule Name". The schedule value will override the material absorptance value. If the control signal is not "Scheduled", then the control signal value at the target surface or zone will decide the absorptance, based on the function referenced in "Thermal Absorptance Function Name" or "Solar Absorptance Function Name". If not specified, the control signal will be assumed to be surface temperature.

1.9.11.1.9 Field: Thermal Absorptance Function Name Inside Face

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the thermal absorptance at the inside or interior side or face.

1.9.11.1.10 Field: Thermal Absorptance Schedule Name Inside Face

The name of a Schedule object that overwrites the material thermal absorptance at the inside or interior side or face. If neither this field or the previous field are defined, then the thermal absorptance is assumed to be constant.

1.9.11.1.11 Field: Solar Absorptance Function Name Inside Face

The name of a Curve or a Table:Lookup object describing the relationship between the control signal and the solar absorptance at the inside or interior side or face.

1.9.11.1.12 Field: Solar Absorptance Schedule Name Inside Face

The name of a Schedule object that overwrites the material solar absorptance at the inside or interior side or face. If neither this field or the previous field are defined, then the solar absorptance is assumed to be constant.

### Approach 1, Phase 3: Modification of Text in EMS Application Guide

Currently, there is a section entitled "Material Surface Properties".  This paragraph will be replaced with the following:

Six actuators are available for controlling the surface properties material related to absorptance. Material layers used in a Construction object can have different thermal, solar, and visible absorptances at the exterior and interior side of the layer. The material at the outside of the construction defines the absorptances at the exterior using the exterior values of these parameters for this material. The material at the inside of the construction defines the absorptances at the interior using the interior values of these parameters. The absorptances determine how much radiation in the thermal, solar, and visible spectrum are absorbed at the surface and thus impact the heat balance of the surface. Actuators called "Material" are available with the control types: "Surface Property Solar Absorptance Outside Face," "Surface Property Thermal Absorptance Outside Face," "Surface Property Visible Absorptance Outside Face,""Surface Property Solar Absorptance Inside Face," "Surface Property Thermal Absorptance Inside Face," and "Surface Property Visible Absorptance Inside Face,"   These are dimensionless parameters between 0.0 and 1.0.  These actuators are useful for modeling switchable coatings such as thermochromic paints. Note that for a single-layer construction, both the inside and outside properties will be overwritten. Properties at both faces can also be modified using the "MaterialProperty:VariableAbsorptance" input object (see InputOutputReference).

### Approach 2: This would require the addition of a Construction:WithAbsorptances.  The syntax for this object would change from the current:

1.9.41.1 Inputs

1.9.41.1.1 Field: Name

This field is a user specified name that will be used as a reference by other input syntax. For example, a heat transfer surface (ref: Building Surfaces) requires a construction name to define what the make-up of the wall is. This name must be identical to one of the Construction definitions in the input data file.

1.9.41.1.2 Field: Outside Layer

Each construction must have at least one layer. This field defines the material name associated with the layer on the outside of the construction - outside referring to the side that is not exposed to the zone but rather the opposite side environment, whether this is the outdoor environment or another zone. Material layers are defined based on their thermal properties elsewhere in the input file (ref: Material and Material Properties and Materials for Glass Windows and Doors). As noted above, the outside layer should NOT be a film coefficient since EnergyPlus will calculate outside convection and radiation heat transfer more precisely.

1.9.41.1.3 Field(s) 2-10: Layers {...}

To the new:

1.9.41.1 Inputs

1.9.41.1.1 Field: Name

This field is a user specified name that will be used as a reference by other input syntax. For example, a heat transfer surface (ref: Building Surfaces) requires a construction name to define what the makeup of the wall is. This name must be identical to one of the Construction definitions in the input data file.

1.9.41.1.2 Field: Exterior Thermal Absorptance

This field is used to specify the thermal absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.3 Field: Exterior Solar Absorptance

This field is used to specify the solar absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.4 Field: Exterior Visible Absorptance

This field is used to specify the visible absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.5 Field: Interior Thermal Absorptance

This field is used to specify the thermal absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.6 Field: Interior Solar Absorptance

This field is used to specify the solar absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.7 Field: Interior Visible Absorptance

This field is used to specify the visible absorptance of the construction at the exterior. It overrides the properties of the material layer on the outside.

1.9.41.1.8 Field: Outside Layer

Each construction must have at least one layer. This field defines the material name associated with the layer on the outside of the construction - outside referring to the side that is not exposed to the zone but rather the opposite side environment, whether this is the outdoor environment or another zone. Material layers are defined based on their thermal properties elsewhere in the input file (ref: Material and Material Properties and Materials for Glass Windows and Doors). As noted above, the outside layer should NOT be a film coefficient since EnergyPlus will calculate outside convection and radiation heat transfer more precisely.

1.9.41.1.5 Field(s) 2-10: Layers {...}

Obviously, the section numbers would change. Similar changes would likely need to be made to other construction objects so that all of them would have access to this new functionality

## Outputs Description ##

As noted above, it might be helpful to have the surface absorptances added as report variables to verify that the various options are functioning properly.

## Engineering Reference ##

There may not need to be any changes to the engineering reference, but this will need to be investigated further.  However, there is some brief discussion of solar absorptance at the beginning page of Chapter 6.  This should be reviewed and clarified if necessary depending on the approach used to resolve this issue/new feature.

## Example File and Transition Changes ##

It is not anticipated that transition changes will be needed since both approaches will allow current input files to continue to be interpreted as they currently are.  An additional example file demonstrating the new capabilities will be added to the test suite as will appropriate unit tests.

## References ##

Current EnergyPlus code and the description for Issue 11393 found at:

https://github.com/NatLabRockies/EnergyPlus/issues/11393
