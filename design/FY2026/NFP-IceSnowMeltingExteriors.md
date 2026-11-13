ICE AND SNOW MELTING ON BUILDING EXTERIORS
=========================================

**Paul Caicedo**

- Original Date: November 9, 2026

## Justification for New Feature

Buildings may use hydronic tubing embedded in exterior pavement, sidewalks, parking areas, ramps, and bridge decks to melt snow and ice. EnergyPlus can model hydronic heat exchangers and provides weather data, but its existing `GroundHeatExchanger:Surface` model does not account for the mass of accumulated snow and ice or the latent energy required to melt it. As a result, simulations cannot represent this load on the plant loop or estimate snow-melting energy use.

This proposal adds a snow- and ice-melting capability to the existing plant-connected surface heat exchanger. It addresses [EnergyPlus issue 6628](https://github.com/NatLabRockies/EnergyPlus/issues/6628), which requests exterior surfaces with horizontal or sloped geometry, shading, and plant-loop integration.

## E-mail and Conference Call Conclusions

No email exchanges or conference-call conclusions are documented in the issue or repository materials reviewed for this proposal. Issue #6628 has a comment pointing to the 2023 ASHRAE Handbook chapter on snow melting and freeze protection and to a 2007 practitioner discussion; these are cited below. This section should be updated if the proposal is discussed by email or at a technicalities call. Development-team review and acceptance are pending.

## Overview

Extend `GroundHeatExchanger:Surface` to estimate snow and ice on its exposed face, account for latent heat during melting, and use optional detailed shading geometry for panel orientation and incident solar radiation. Retain the component's existing plant-loop connection and embedded-tube construction model. The intended first increment is a weather-driven, low-input estimate that fits the existing component, not a full transient snowpack or pavement-design model.

The model estimates new snow from positive changes in weather-file snow depth and liquid precipitation as ice when rain occurs below 0 C. The proposed assumptions are a fresh-snow density of 100 kg/m3 and a latent heat of fusion of 335,000 J/kg. Snow and ice remain at the melting point while available surface energy is used for melting. Mass is depleted by calculated melt energy, rather than by decreases in weather-file snow depth.

The existing implementation does not represent snowpack sensible heating, insulation, compaction, drifting, meltwater runoff, refreezing, or explicit preheat/postheat control. It does not adjust estimated accumulation for panel tilt; detailed geometry affects solar exposure only. The model also does not determine a snow-free target or system operating schedule. These limitations matter because snow-melting energy depends on control strategy, snowfall and weather, pavement thermal mass, and the desired surface condition. This proposal asks the development team to review whether that deliberately simplified first increment is suitable for EnergyPlus, or whether additional physics or controls are required before acceptance.

## Approach

Add per-panel snow/ice mass state to the existing surface heat exchanger. On the first timestep, initialize mass from current snow depth. On later timesteps, accumulate positive changes in snow depth as fresh snow and accumulate liquid precipitation as ice when rain occurs below freezing. Do not remove mass directly when weather-file snow depth decreases. This is a practical approximation for EPW inputs, not a snow-transport model: a weather-file snow-depth change is treated as new accumulation and is not corrected for site exposure, slope, compaction, or wind.

At each timestep, calculate the exterior surface energy balance with the existing hydronic heat transfer, outdoor convection and long-wave exchange, and absorbed short-wave solar radiation. When there is snow or ice and net energy is available, hold the exposed surface at 0 C and apply the remaining energy to fusion, limited by the available mass and timestep duration. Reduce the stored mass by the melt amount, and report the remaining mass, melt rate, melt heat-transfer rate, and melt energy.

An optional `Shading:Site:Detailed` reference supplies exterior surface geometry. Use EnergyPlus's incident short-wave solar calculation for that geometry, including its orientation and shading. When no geometry is specified, preserve the existing unshaded horizontal solar calculation. The referenced geometry should have an area consistent with the panel length and width. In this proposal, the geometry is not used to adjust weather-derived snow or precipitation mass.

The primary alternative considered is a new, dedicated snow-melting component. Extending `GroundHeatExchanger:Surface` is preferred because it already models embedded hydronic tubing, interacts with the plant loop, and supports ground-coupled or exposed lower surfaces. Reusing the weather file avoids requiring duplicate user schedules or new weather objects. The tradeoff is that the simplified mass estimate and surface balance do not reproduce the detailed transient pavement/snow models in the literature.

## Testing/Validation/Data Sources

Unit tests should verify snow-depth accumulation, freezing-rain accumulation, no accumulation from decreasing snow depth, the 0 C melt behavior, melt limitation by available mass, and use of incident solar radiation from detailed shading geometry. A plant-loop simulation should verify that melt energy is coupled to the existing hydronic heat transfer and that reported melt quantities have the correct area and timestep scaling. The prototype implementation in the associated pull request has focused unit tests for those calculation paths. It has not been validated against measured pavement data or the published experimental model, and a plant-loop integration test remains a validation item for review.

Validation should begin with hand-checkable mass and energy balances: snow mass from depth change and assumed density, freezing-rain mass from liquid precipitation, and melt mass from energy divided by latent heat of fusion. Exercise both the optional-geometry and no-geometry cases. Before acceptance, the development team should decide whether a comparison against published or measured pavement data is required and define the corresponding validation case and acceptance criteria.

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

Snow-melting design loads and system operation are covered in ASHRAE Handbook—HVAC Applications, Chapter 52, “Snow Melting and Freeze Protection” (2023 edition). Published transient models provide a more detailed basis for pavement heat transfer and experimental comparison than the simplified weather-driven mass estimate proposed here. This model is not intended to replace detailed site-specific snow or pavement design calculations.

## Example File and Transition Changes

No new top-level object is proposed, and no input transition is required because the new geometry reference is optional. The Input Output Reference example shows a `GroundHeatExchanger:Surface` connected to a plant loop and a matching `Shading:Site:Detailed` surface for panel geometry.

## References

- [EnergyPlus issue 6628: Add capability to model Ice and Snow Melting on Building Exteriors](https://github.com/NatLabRockies/EnergyPlus/issues/6628)
- ASHRAE. 2023. *ASHRAE Handbook—HVAC Applications*, Chapter 52, “Snow Melting and Freeze Protection.” The chapter was cited in the issue discussion; consult the Handbook for its detailed design guidance.
- Liu, X., S. J. Rees, and J. D. Spitler. 2007. “Modeling Snow Melting on Heated Pavement Surfaces. Part I: Model Development.” *Applied Thermal Engineering* 27(5–6):1115–1124. [https://doi.org/10.1016/j.applthermaleng.2006.06.017](https://doi.org/10.1016/j.applthermaleng.2006.06.017).
- Liu, X., S. J. Rees, and J. D. Spitler. 2007. “Modeling Snow Melting on Heated Pavement Surfaces. Part II: Experimental Validation.” *Applied Thermal Engineering* 27(5–6):1125–1131. [https://doi.org/10.1016/j.applthermaleng.2006.07.029](https://doi.org/10.1016/j.applthermaleng.2006.07.029).
- Ramsey, J. W., et al. 1999. *Updated Design Guidelines for Snow Melting Systems*. ASHRAE Research Project 926-RP, as cited in the archived BLDG-SIM discussion and Liu et al. (2007).
- Rees, S. J., et al. 2001. *Development of a Two-Dimensional Transient Model of Snow-Melting System, and Use of the Model for Analysis of Design Alternatives*. ASHRAE Research Project 1090-RP, as cited in the archived BLDG-SIM discussion.
- Liu, X. 2007. “[BLDG-SIM] How to model Snowmelt System in eQUEST?” Archived practitioner discussion, October 26. [https://onebuilding.org/archive/bldg-sim-onebuilding.org/2007-October/034952.html](https://onebuilding.org/archive/bldg-sim-onebuilding.org/2007-October/034952.html).
- EnergyPlus Input Output Reference, `GroundHeatExchanger:Surface` and `Shading:Site:Detailed`.
