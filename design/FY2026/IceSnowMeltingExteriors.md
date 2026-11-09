ICE AND SNOW MELTING ON BUILDING EXTERIORS
=========================================

**Paul Caicedo**

- Original Date: November 9, 2026

## Justification for New Feature

Buildings may use hydronic tubing embedded in exterior pavement, sidewalks, parking areas, ramps, and bridge decks to melt snow and ice. EnergyPlus can model hydronic heat exchangers and provides weather data, but its existing `GroundHeatExchanger:Surface` model does not account for the mass of accumulated snow and ice or the latent energy required to melt it. As a result, simulations cannot represent this load on the plant loop or estimate snow-melting energy use.

This proposal adds a snow- and ice-melting capability to the existing plant-connected surface heat exchanger. It addresses [EnergyPlus issue 6628](https://github.com/NatLabRockies/EnergyPlus/issues/6628), which requests exterior surfaces with horizontal or sloped geometry, shading, and plant-loop integration.

## E-mail and Conference Call Conclusions

No email or conference-call conclusions have been recorded for this proposal. It is submitted for development-team review; acceptance of the proposal is pending.

## Overview

Extend `GroundHeatExchanger:Surface` to track snow and ice on its exposed face, account for latent heat during melting, and use optional detailed shading geometry to represent panel orientation and incident solar radiation. Retain the component's existing plant-loop connection and embedded-tube construction model.

The model estimates new snow from increases in weather-file snow depth and liquid precipitation as ice when it is raining below 0 C. The proposed assumptions are a fresh-snow density of 100 kg/m3 and a latent heat of fusion of 335,000 J/kg. Snow and ice remain at the melting point while available surface energy is used for melting. Mass is depleted by calculated melt energy, rather than by decreases in weather-file snow depth.

## Approach

Add per-panel snow/ice mass state to the existing surface heat exchanger. On the first timestep, initialize mass from current snow depth. On later timesteps, accumulate positive changes in snow depth as fresh snow and accumulate liquid precipitation as ice when rain occurs below freezing. Do not remove mass directly when weather-file snow depth decreases.

At each timestep, calculate the exterior surface energy balance with the existing hydronic heat transfer, outdoor convection and long-wave exchange, and absorbed short-wave solar radiation. When there is snow or ice and net energy is available, hold the exposed surface at 0 C and apply the remaining energy to fusion, limited by the available mass and timestep duration. Reduce the stored mass by the melt amount, and report the remaining mass, melt rate, melt heat-transfer rate, and melt energy.

An optional `Shading:Site:Detailed` reference supplies exterior surface geometry. Use EnergyPlus's incident short-wave solar calculation for that geometry, including its orientation and shading. When no geometry is specified, preserve the existing unshaded horizontal solar calculation. The referenced geometry should have an area consistent with the panel length and width.

This proposal does not add a separate plant component or a separate snowfall/weather input object. It reuses the established `GroundHeatExchanger:Surface` plant connection and available weather-file data. Snow density and the accumulation-from-snow-depth method are simplified estimates; they do not model compaction, drifting, runoff, or spatially varying snow cover.

## Testing/Validation/Data Sources

Unit tests should verify snow-depth accumulation, freezing-rain accumulation, no accumulation from decreasing snow depth, the 0 C melt behavior, melt limitation by available mass, and use of incident solar radiation from detailed shading geometry. A plant-loop simulation should verify that melt energy is coupled to the existing hydronic heat transfer and that reported melt quantities have the correct area and timestep scaling.

Validation should compare simple energy and mass balances against hand calculations: snow mass from depth and assumed density, freezing-rain mass from liquid precipitation, and melt mass from energy divided by latent heat of fusion. Exercise both the optional-geometry and no-geometry cases.

## Input Output Reference Documentation

Update the `GroundHeatExchanger:Surface` description to document the weather-derived accumulation assumptions, melt energy balance, and optional geometry. Describe the new `Shading Surface Name` input, provide an IDF example, and document the new snow/ice mass and melt output variables.

## Input Description

The existing `GroundHeatExchanger:Surface` fields remain unchanged. Add one optional field:

- **Shading Surface Name**: name of a `Shading:Site:Detailed` surface whose orientation and incident solar radiation represent the active panel. If omitted, use the existing unshaded horizontal solar calculation.

Snow depth and liquid precipitation are taken from the weather data. No additional accumulation schedule or user-specified snow-density field is proposed.

## Outputs Description

Add the following outputs for each surface heat exchanger:

- **Ground Heat Exchanger Snow and Ice Mass** (kg): estimated remaining accumulated mass on the panel.
- **Ground Heat Exchanger Snow and Ice Melt Rate** (kg/s): melt rate during the timestep.
- **Ground Heat Exchanger Snow and Ice Melt Heat Transfer Rate** (W): latent heat rate used for melting.
- **Ground Heat Exchanger Snow and Ice Melt Energy** (J): latent heat used for melting during the timestep.

Existing plant-side and surface heat-transfer outputs continue to report the component's hydronic operation.

## Engineering Reference

Snow-melting and freeze-protection applications are discussed in ASHRAE Handbook—HVAC Applications, Chapter 52, “Snow Melting and Freeze Protection” (2023 edition). The model uses the latent heat of fusion of water and a simplified weather-driven mass estimate; it is not intended to replace detailed site-specific snow or pavement design calculations.

## Example File and Transition Changes

No new top-level object is proposed. Existing input files remain valid because the new geometry reference is optional. Add an example using a `GroundHeatExchanger:Surface` connected to a plant loop and a matching `Shading:Site:Detailed` surface for a sloped or shaded panel.

## References

- [EnergyPlus issue 6628: Add capability to model Ice and Snow Melting on Building Exteriors](https://github.com/NatLabRockies/EnergyPlus/issues/6628)
- ASHRAE Handbook—HVAC Applications, Chapter 52, “Snow Melting and Freeze Protection,” 2023.
- EnergyPlus Input Output Reference, `GroundHeatExchanger:Surface` and `Shading:Site:Detailed`.
