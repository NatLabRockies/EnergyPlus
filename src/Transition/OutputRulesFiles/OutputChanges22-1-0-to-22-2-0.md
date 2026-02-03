Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.


### Template Title

Change description goes here, use links to the PR such as [#9117](https://github.com/NatLabRockies/EnergyPlus/pull/9117/files).

### SurfaceProperty:GroundSurfaces New Object Output Variables

Two new output variables related to the `SurfaceProperty:GroundSurfaces` have been added along with the new feature development. These new variables are:

- Surfaces Property Ground Surfaces Average Temperature [C]
- Surfaces Property Ground Surfaces Average Reflectance []

See pull request [#9409](https://github.com/NatLabRockies/EnergyPlus/pull/9409) for more details.

### Added a few new tables in the annual thermal/visual/CO2 resilience reporting in the tabular output. Added thermal/visual/CO2 resilience reporting for user-specified reporting period.

For the annual thermal resilience reports, added the following tables
- Heat Index OccupiedHours
- Humidex OccupiedHours
- Hours of Safety for Cold Events
- Hours of Safety for Heat Events
- Unmet Degree-Hours
- Discomfort-weighted Exceedance OccupantHours
- Discomfort-weighted Exceedance OccupiedHours

For the annual CO2 resilience report, the following table is added
- CO2 Level OccupiedHours

For the annual Visual resilience report, the following table is added
- Illuminance Level OccupiedHours

For the period-specific reports, when users choose ThermalResilienceSummary, CO2ResilienceSummary, or VisualResilienceSummary, the same set of tables as the annual thermal, CO2, or visual resilience table will be generated for the specified reporting period. When the "AllResilienceSummaries" is chosen for the report name field, all of the three sets of reports will be generated for the specified reporting period.

A "Reporting Period Time and Consumption" table is created to summarize the type, time range, and total electricity consumption for all reporting periods.

Please see PR [#9156](https://github.com/NatLabRockies/EnergyPlus/pull/9156).

### SurfaceProperty:GroundSurfaces New Object Output Variables

Two new output variables related to the `SurfaceProperty:GroundSurfaces` have been added along with the new feature development. These new variables are:

- Surfaces Property Ground Surfaces Average Temperature [C]
- Surfaces Property Ground Surfaces Average Reflectance []

See pull request [#9409](https://github.com/NatLabRockies/EnergyPlus/pull/9409) for more details.

### Tabular report changes in Output:SQLite and Output:JSON

#### Renames in SQLite

The report `Initialization Summary` which was the only one with a space in its name was renamed to `InitializationSummary`.

#### New reports in JSON output

`AnnualHeatEmissionsReport` and `InitializationSummary` reports were **added** to the JSON output.

#### Renames in JSON

Six reports have been renamed to remove spaces:

```diff
-Annual Building Utility Performance Summary
+AnnualBuildingUtilityPerformanceSummary

-Component Sizing Summary
+ComponentSizingSummary

-Demand End Use Components Summary
+DemandEndUseComponentsSummary

-Input Verification and Results Summary
+InputVerificationandResultsSummary

-Source Energy End Use Components Summary
+SourceEnergyEndUseComponentsSummary

-Surface Shadowing Summary
+SurfaceShadowingSummary
```

#### Renames in both SQLite and JSON

Under the report `SourceEnergyEndUseComponentsSummary`, the table `Source Energy End Use Component Per Conditioned Floor Area` was renamed to `Source Energy End Use Components Per Conditioned Floor Area` to match the HTML output and the other Source Energy End Use Component**s** tables.

The Adaptive Comfort Summary/Report was renamed to match the HTML:

```diff
-AdaptiveComfortReport,People Summary
+AdaptiveComfortSummary,Time Not Meeting the Adaptive Comfort Models during Occupied Hours
```

See pull request [#9461](https://github.com/NatLabRockies/EnergyPlus/pull/9461) for more details.

### Output:Table:Monthly and Output:Table:Annual

In `Output:Table:Monthly` and `Output:Table:Annual`, variables with an aggregation type such as `Maximum`, `Minimum`, `MaximumDuringHoursShown`, and `MinimumDuringHoursShown` produce an extra `<Variable of Meter Name> {TIMESTAMP}` column that had an extra trailing space in the SQL. That extra trailing space was removed. cf [#9647](https://github.com/NatLabRockies/EnergyPlus/pull/9647)

### Latent Sizing

New report variables were added to identify results associated with latent heat energy transfer. These reports include the zone air system sensible heat ratio (i.e., sensible heat transfer divided by total heat transfer) and zone air vapor pressure difference (i.e., difference of zone air vapor pressure from saturated air vapor pressure at the current temperature).

    Zone Air System Latent Heating Energy
    Zone Air System Latent Cooling Energy
    Zone Air System Latent Heating Rate
    Zone Air System Latent Cooling Rate
    Zone Air System Sensible Heat Ratio
    Zone Air Vapor Pressure Difference

New results were added to the zone sizing results file eplusout.zsz

    Des Latent Heat Load [W]
    Des Latent Cool Load [W]
    Des Latent Heat Mass Flow [kg/s]
    Des Latent Cool Mass Flow [kg/s]
    Des Heat Load No DOAS [W]
    Des Cool Load No DOAS [W]
    Des Latent Heat Load No DOAS [W]
    Des Latent Cool Load No DOAS [W]
    Heating Zone Temperature [C]
    Heating Zone Relative Humidity [%]
    Cooling Zone Temperature [C]
    Cooling Zone Relative Humidity [%]

See pull request [#9406](https://github.com/NatLabRockies/EnergyPlus/pull/9406) for more details.

### Envelope Summary Report - Interior Fenestration Subtable

Missing Parent Surface column was added.

See pull request [#9582](https://github.com/NatLabRockies/EnergyPlus/pull/9582) for more details.
