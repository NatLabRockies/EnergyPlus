# EMS Actuator for CondFD Exterior Sky Longwave Radiation Override

**Brian Ball, NatLabRockies**

- March 2026 - Initial Draft

## Overview

Building energy simulation researchers coupling EnergyPlus with external radiation solvers (COMSOL, RadTherm, custom view factor codes) need to override the linearized sky longwave radiation calculation in the Conduction Finite Difference (CondFD) heat balance. Currently, EnergyPlus computes sky longwave (LW) radiation as `hsky*(Tsky - Tsurf)` using a linearized h-coefficient approach. This is adequate for standard simulations but insufficient when:

- Users have measured net sky radiation data (pyrgeometer)
- External tools compute view-factor-accurate LW exchange with complex sky models
- Research requires decoupling the sky radiation model from the EnergyPlus surface solver

## Background

### Longwave Radiation at a Building Surface

Every building surface exchanges thermal (longwave/infrared) radiation with its surroundings: the sky, the ground, and neighboring surfaces. For a roof, the dominant exchange is with the sky. Because the sky is much colder than the surface, this exchange is a net heat *loss* — the surface radiates more energy toward the sky than it receives back. This is why roofs cool well below air temperature on clear nights.

The fundamental radiation exchange between a surface and the sky is governed by the Stefan-Boltzmann law:

```
Q_sky = ε · σ · F_sky · (Tsky⁴ − Tsurf⁴)
```

where:
- `ε` = surface emissivity (0–1, typically ~0.9 for building materials)
- `σ` = Stefan-Boltzmann constant (5.67 × 10⁻⁸ W/m²·K⁴)
- `F_sky` = view factor to sky (fraction of the hemisphere "seen" by the surface that is sky vs. ground/surroundings; ≈1.0 for a flat roof, ≈0.5 for a vertical wall)
- `Tsky` = effective sky temperature [K]
- `Tsurf` = exterior surface temperature [K]

### EnergyPlus Linearization

The `T⁴` nonlinearity makes direct solution expensive. EnergyPlus linearizes this into a convection-like form:

```
Q_sky ≈ hsky · (Tsky − Tsurf)
```

where `hsky` is a *linearized radiative heat transfer coefficient* (W/m²·K) that absorbs the emissivity, Stefan-Boltzmann constant, view factor, and a temperature-dependent scaling factor. This linearization is recomputed each timestep and is accurate for small temperature differences, but it is an approximation.

### Why Override?

The linearized approach cannot easily incorporate:
- **Measured data** — pyrgeometer readings give a direct net flux, not an h-coefficient
- **External solver results** — tools like COMSOL or RadTherm compute the full nonlinear `T⁴` exchange with detailed geometry and atmospheric models, producing a net flux value
- **Custom sky models** — researchers developing improved sky radiation correlations need to inject their own flux values

This actuator lets users bypass the linearization entirely by supplying their own net sky LW flux value, called **Enet** (defined below).

### Definition of Enet

**Enet** is the user-specified net sky longwave radiation heat flux at the exterior surface, in W/m². It replaces the linearized `hsky·(Tsky − Tsurf)` term in the CondFD exterior heat balance equation.

Enet is not computed by EnergyPlus — it is an *input* supplied by the user via the EMS (Energy Management System) actuator, from any source: measured data, an external solver, or a custom model.

### Key Terminology

| Term | Meaning |
|------|---------|
| **CondFD** | Conduction Finite Difference — an EnergyPlus algorithm that solves 1-D transient heat conduction through walls/roofs by discretizing layers into nodes. Alternative to the default Conduction Transfer Function (CTF) method. |
| **EMS** | Energy Management System — EnergyPlus's runtime scripting system that lets users read sensor values and override ("actuate") simulation variables during a run, via IDF programs or the Python API. |
| **CTF** | Conduction Transfer Function — EnergyPlus's default wall conduction algorithm. Uses pre-computed transfer functions; faster but less flexible than CondFD. Not affected by this proposal. |
| **EMPD** | Effective Moisture Penetration Depth — a moisture-aware conduction algorithm. Not affected by this proposal. |
| **Linearized h-coefficient** | A technique that rewrites the nonlinear `σ·T⁴` radiation exchange as `h·ΔT`, where `h` is updated each timestep. Converts a radiation problem into a form that looks like convection, enabling a simple implicit solve. |
| **View factor (F)** | The fraction of a surface's hemispherical "field of view" occupied by another surface or the sky. A flat roof sees almost all sky (F_sky ≈ 1); a vertical wall sees half sky, half ground (F_sky ≈ 0.5). |

## Justification

The CondFD algorithm already supports EMS actuators for material conductivity, specific heat, and internal heat flux (added FY2021-2022). This proposal extends that pattern to the exterior sky radiation boundary condition — the last remaining piece needed for full external coupling of the CondFD exterior heat balance.

## Implementation

Add an EMS actuator `"CondFD Surface" / "Sky Longwave Radiation Override"` (W/m2) that replaces the `hsky*(Tsky - Tsurf)` term in the CondFD exterior boundary equation (`ExteriorBCEqns` in `HeatBalFiniteDiffManager.cc`). When not actuated, existing behavior is unchanged.

### Sign Convention

Positive = heat INTO the surface (consistent with all existing EnergyPlus reporting):
- `Enet > 0` — net sky LW heats the surface (uncommon but physically real; occurs when heavy cloud cover or a warm air mass causes the atmosphere to radiate more LW toward the surface than the surface emits upward — see references below)
- `Enet < 0` — net sky LW cools the surface (typical for buildings; surfaces radiate more energy toward the cold sky than they receive back, especially on clear nights)
- `Enet = 0` — no sky LW exchange (surface and sky in thermal equilibrium, or sky LW intentionally disabled for research)

#### Physical Basis for Sign Convention

Net longwave radiation at a surface equals downwelling LW (from sky/clouds) minus upwelling LW (emitted by the surface). Whether the net value is positive or negative depends on the temperature of the atmosphere relative to the surface. For most building surfaces in temperate climates, the sky is colder than the surface, so the net is negative (cooling). However, under heavy overcast with warm, low cloud bases — or when a warm air mass advects over a radiatively cooled surface — downwelling LW can exceed upwelling, producing a positive (heating) net flux. This is well-documented in Arctic and snow-surface research and has been directly measured with pyrgeometers.

**References:**
1. Viúdez-Mora, A., Costa-Surós, M., Calbó, J., & González, J.A. (2015). "Modeling atmospheric longwave radiation at the surface during overcast skies: The role of cloud base height." *J. Geophys. Res. Atmospheres*, DOI: [10.1002/2014JD022310](https://agupubs.onlinelibrary.wiley.com/doi/full/10.1002/2014JD022310) — Shows cloud base height governs downwelling LW magnitude under overcast skies.
2. Park, M.-S., et al. (2018). "The observed relationship of cloud to surface longwave radiation and air temperature at Ny-Ålesund, Svalbard." *Tellus B*, DOI: [10.1080/16000889.2018.1450589](https://www.tandfonline.com/doi/full/10.1080/16000889.2018.1450589) — Documents warm air mass advection driving increased downwelling LW and surface warming at an Arctic station.
3. Bertrand, L., Kay, J.E., & de Boer, G. (2025). "Increasing wintertime cloud opacity increases surface longwave radiation at a long-term Arctic observatory." *Nature Communications*, 16. [Link](https://www.nature.com/articles/s41467-025-64441-8) — Two decades of measurements showing increasing cloud opacity drives increasing net surface LW radiation.
4. Ritter, M.E. "The Radiation Balance." *The Physical Environment*, Geosciences LibreTexts. [Link](https://geo.libretexts.org/Bookshelves/Geography_(Physical)/The_Physical_Environment_(Ritter)/04:_Energy_and_Radiation/4.03:_Radiation_and_Energy_Balance_of_the_Earth_System/4.3.01:_The_Radiation_Balance) — States: "If the air is warmer than the ground a positive value exists. This could occur with heavy cloud cover...or if a warm air mass travels over a colder surface."
5. U.S. Army Corps of Engineers. "Longwave Radiation." *HEC-HMS Technical Reference Manual*. [Link](https://www.hec.usace.army.mil/confluence/hmsdocs/hmsum/4.8/meteorology-description/longwave-radiation) — Confirms: "Whether the atmosphere and clouds are a net source of longwave radiation to the land surface depends on their temperature relative to the land surface temperature."

### Scope

- CondFD algorithm only (CTF/EMPD not affected)
- Replaces only the sky component; ground, surrounding surface, and air LW terms unchanged
- All 4 solver paths patched: R-layer, Crank-Nicolson, fully implicit, movable insulation
- Reporting (QNetSurfFromOutside, SurfQdotRadOutRepPerArea) updated consistently

### Actuator Details

| Field | Value |
|-------|-------|
| Component Type | `CondFD Surface` |
| Unique ID | Surface name (e.g., `Zn001:Roof001`) |
| Control Type | `Sky Longwave Radiation Override` |
| Units | `[W/m2]` |

One actuator per exterior CondFD surface, controllable via IDF EMS programs or Python API.

### New Output Variable

`CondFD EMS Sky Longwave Radiation Override Heat Flux [W/m2]` — reports the actuated value (0.0 when not actuated).

### Files Modified

| File | Change |
|------|--------|
| `HeatBalFiniteDiffManager.hh` | Add `enetActuator` field to `SurfaceDataFD` |
| `HeatBalFiniteDiffManager.cc` | Actuator registration + 6 solver location overrides |

### Physics

The CondFD exterior heat balance solves for the outside surface node temperature `T_o` implicitly. The standard linearized form (simplified, showing only the key terms) is:

```
         QSolarAbs + hconv·T_air + hsky·Tsky + hgnd·Tgnd + hcond·T_i
T_o  =  ─────────────────────────────────────────────────────────────
                    hconv + hsky + hgnd + hcond
```

where:
- `hconv` = convective heat transfer coefficient, `T_air` = outdoor air temperature
- `hsky` = linearized sky radiative coefficient, `Tsky` = effective sky temperature
- `hgnd` = linearized ground radiative coefficient, `Tgnd` = ground temperature
- `hcond` = conduction coupling to the next interior node, `T_i` = next node temperature
- `QSolarAbs` = absorbed solar radiation (a fixed flux, not temperature-dependent)

The `hsky·Tsky` term in the numerator and `hsky` in the denominator together represent the sky LW contribution in this implicit formulation.

**When Enet is actuated**, the sky terms are replaced:

```
         QSolarAbs + hconv·T_air + Enet + hgnd·Tgnd + hcond·T_i
T_o  =  ─────────────────────────────────────────────────────────
                    hconv + hgnd + hcond
```

- **Numerator:** `hsky · Tsky` is replaced by `Enet` (a fixed flux, like solar)
- **Denominator:** `hsky` is removed entirely

This works because Enet is a *fixed flux* — it does not depend on the surface temperature being solved for. Just like absorbed solar radiation, it contributes a known quantity of heat and does not need to appear in the denominator. The linearized `hsky` term, by contrast, is temperature-dependent (it multiplies a temperature difference), which is why it appears in both numerator and denominator.

This substitution is applied consistently across all 6 code locations in `ExteriorBCEqns`: the R-layer solver, Crank-Nicolson solver, fully implicit solver, movable insulation path, net flux calculation, and LW reporting.

## Testing

- Unit test: `HeatBalFiniteDiffManager_EnetActuatorOverride` — exercises ExteriorBCEqns with actuator OFF, Enet=0, Enet=-200, Enet=+200; verifies correct temperature ordering
- Integration tests: `1ZoneCondFD_Enet_Test.idf` (baseline) and `1ZoneCondFD_Enet_EMS.idf` (EMS with Enet=-200)
- Validated against Phase 2 hardcoded results (roof temps match within 0.1C)

### Phase 2 Validation Results

| Run | Enet (W/m2) | Roof Temp (C) | LW Report (W/m2) |
|-----|-------------|---------------|-------------------|
| Baseline | n/a | -23.8 | -32.0 |
| Enet=0 | 0 | -16.0 | 0.0 |
| Enet=-200 | -200 | -55.8 | -200.0 |
| Enet=+200 | +200 | +12.6 | +200.0 |

## Documentation

EMS Application Guide will be updated to describe the new actuator, including:
- Usage with IDF EMS programs
- Usage with Python plugin API
- Usage with standalone Python API (Jupyter notebook example provided)

## IDD Changes and Transition

None required. The actuator is registered programmatically via `SetupEMSActuator`.

## Example Files

- `testfiles/1ZoneCondFD_Enet_Test.idf` — baseline CondFD test
- `testfiles/1ZoneCondFD_Enet_EMS.idf` — EMS actuator with Enet=-200
- `scripts/CondFD_Enet_Override.ipynb` — Jupyter notebook for interactive use with any IDF/EPW

## Possible Enhancements

### Branch Hoist / Equation Unification in `ExteriorBCEqns`

Pure refactor — no behavior change.

**Problem.** `ExteriorBCEqns` in `src/EnergyPlus/HeatBalFiniteDiffManager.cc` contains 6 `if (enetAct.isActuated)` sites (R-layer, Crank-Nicolson 2nd-order, Fully-Implicit 1st-order, MovInsul `TInsulOut`, `QNetSurfFromOutside`, `SurfQdotRadOutRepPerArea`). Each duplicates the surrounding equation. Introduced in commit 2487d34a39.

**Performance.** Not a real concern.
- `enetAct.isActuated` is a single bool, hoisted once per call via `auto const &enetAct = ...`.
- Called per exterior CondFD surface × outer-face node × CondFD sub-iter × HVAC timestep. High, not innermost.
- Branch predictor ~100% accurate: value is constant across all sub-iterations of a timestep. EMS can flip at most once per zone timestep, so at worst one mispredict (~10-20 cycles) amortized over thousands of calls.
- Both arms are straight-line arithmetic, in I-cache. Cost is in the noise vs. divides / `pow_2` / property lookups / reporting around them.
- Mid-run EMS toggling does not change this analysis — the bool is re-read every call.

**Real cost: maintenance.** 4 duplicated equations + 2 duplicated report lines. Risk of silent drift if someone fixes the sky term in one arm but not the other.

**Recommended refactor.** Hoist the branch once at the top of the `CondFD` block in `ExteriorBCEqns`, compute effective locals, then use unified equations:

```cpp
Real64 const hsky_eff    = enetAct.isActuated ? 0.0                    : hsky;
Real64 const QskyTerm    = enetAct.isActuated ? enetAct.actuatedValue  : hsky * Tsky;
// Form used by QNet / report lines, which need (Tsky - TDT_i):
Real64 const QskyNetTerm = enetAct.isActuated ? enetAct.actuatedValue  : hsky * (Tsky - TDT_i);
```

Then every site collapses. R-layer example:

```cpp
TDT_i = (TDT_p + (QRadSWOutFD + hgnd*Tgnd + (hconvo+hrad)*Toa + QskyTerm + hsurr*Tsurr) * Rlayer)
      / (1.0 + (hconvo + hgnd + hrad + hsky_eff + hsurr) * Rlayer);
```

Works identically for CN 2nd-order, FI 1st-order, `TInsulOut`. For `QNetSurfFromOutside` / `SurfQdotRadOutRepPerArea`, replace `hsky*(Tsky-TDT_i)` (resp. `hsky*(TDT_i-Tsky)`) with `QskyNetTerm` (resp. `-QskyNetTerm`).

**Properties.**
- Bit-for-bit identical when not actuated (same operand order, same linearization).
- Correct under arbitrary mid-run EMS toggling — locals recomputed every call from current bool.
- Removes all 6 duplicated sites; 4 equations shrink to 1 form each.
- Same or slightly faster: compiler gets straight-line code; ternaries typically lower to `cmov`.

**Rejected alternatives.**
- *Function-pointer / template dispatch per surface* — indirect call overhead, I-cache pressure, zero upside.
- *Partition surfaces into actuated / not-actuated lists at init* — over-engineered, and wrong if EMS can flip mid-run.
- *Always treat sky as a flux (drop `hsky` from denominator universally)* — changes the implicit linearization in the non-actuated path; likely worse convergence. Do not do.
- *Hoist the branch outside `ExteriorBCEqns` scope* — breaks mid-run toggling.

**Open questions.**
- Extend same pattern to CTF / HAMT / Kiva, or CondFD-only?
- Land as part of the SkyLW actuator PR, or separate follow-up?
