# Output Changes

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it is not clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It is not expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### Refrigeration:AirChiller report variable names

PR #11677 resolves duplicate `Refrigeration:AirChiller` report variable names by adding `Coil` to the coil-level reports.

The following old names are still valid for zone-level Air Chiller reports in 26.2.0, so they cannot be safely transitioned to the new coil-level names without changing valid zone-level output requests:

- `Refrigeration Zone Air Chiller Total Cooling Rate` -> `Refrigeration Zone Air Chiller Coil Total Cooling Rate`
- `Refrigeration Zone Air Chiller Total Cooling Energy` -> `Refrigeration Zone Air Chiller Coil Total Cooling Energy`
- `Refrigeration Zone Air Chiller Sensible Cooling Rate` -> `Refrigeration Zone Air Chiller Coil Sensible Cooling Rate`
- `Refrigeration Zone Air Chiller Sensible Cooling Energy` -> `Refrigeration Zone Air Chiller Coil Sensible Cooling Energy`
- `Refrigeration Zone Air Chiller Latent Cooling Rate` -> `Refrigeration Zone Air Chiller Coil Latent Cooling Rate`
- `Refrigeration Zone Air Chiller Latent Cooling Energy` -> `Refrigeration Zone Air Chiller Coil Latent Cooling Energy`
- `Refrigeration Zone Air Chiller Water Removed Mass Flow Rate` -> `Refrigeration Zone Air Chiller Coil Water Removed Mass Flow Rate`

The following old names are fully renamed to coil-level reports and are transitioned by the report-variable CSV:

- `Refrigeration Zone Air Chiller Sensible Heat Ratio` -> `Refrigeration Zone Air Chiller Coil Sensible Heat Ratio`
- `Refrigeration Zone Air Chiller Frost Accumulation Mass` -> `Refrigeration Zone Air Chiller Coil Frost Accumulation Mass`
- `Refrigeration Zone Air Chiller Defrost Electricity Rate` -> `Refrigeration Zone Air Chiller Coil Defrost Electricity Rate`
- `Refrigeration Zone Air Chiller Defrost Electricity Energy` -> `Refrigeration Zone Air Chiller Coil Defrost Electricity Energy`

### EIO and Initialization Summary Report Changes for Material Conductivity / Thermal Resistance in InchPound

When using `OutputControl:Table:Style` with Unit Conversion = `InchPound`, the tables "Material CTF Summary" and "WindowMaterial:Glazing" were incorrectly reporting some quantities in SI Units. They now report correctly in IP units.

- Conductivity {w/m-K} -> {Btu-in/hr-ft2-F}
- ThermalResistance {m2-K/w} -> {ft2-F-hr/Btu}

Additionally, the EIO now will use the standard `W` (uppercase) notation for these units, instead of `w` (lowercase).

See pull request [#11736](https://github.com/NatLabRockies/EnergyPlus/pull/11736/files) for more details.
