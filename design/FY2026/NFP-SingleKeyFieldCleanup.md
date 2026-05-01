Single-Key Field Cleanup
========================

## Summary ##

Several fields in `idd/Energy+.idd.in` expose exactly one allowed `\key` value. Many of these are intentional and should remain because they document supported behavior or reinforce validation. This issue covers a smaller subset that appear to be stale or redundant based on current source review.

In these cases, the field is typically one of the following:

- no longer used by runtime code,
- always overridden internally,
- redundant with an object-list or name-based lookup,
- a placeholder for functionality that was never generalized beyond one implementation.

The goal is to remove or internalize these fields through a versioned input transition, not to remove every one-key field in the IDD.

## In Scope ##

The following fields should be cleaned up:

1. `Generator:PVWatts` / `PVWatts Version`
2. `SolarCollectorPerformance:FlatPlate` / `Test Fluid`
3. `SolarCollectorPerformance:IntegralCollectorStorage` / `ICS Collector Type`
4. `Exterior:WaterEquipment` / `Fuel Use Type`
5. `HeatExchanger:Desiccant:BalancedFlow` / `Heat Exchanger Performance Object Type`
6. `AirLoopHVAC:UnitarySystem` / `Design Specification Multispeed Object Type`
7. `ZoneHVAC:WaterToAirHeatPump` / `Design Specification Multispeed Object Type`
8. `ZoneHVAC:TerminalUnit:VariableRefrigerantFlow` / `Design Specification Multispeed Object Type`
9. `WaterHeater:HeatPump:WrappedCondenser` / `Tank Object Type`
10. `WaterHeater:HeatPump:WrappedCondenser` / `DX Coil Object Type`
11. `GroundHeatExchanger:System` / `GHE:Vertical:Sizing Object Type`
12. `ChillerHeater:Absorption:DoubleEffect` / `Exhaust Source Object Type`
13. `Generator:FuelCell:ElectricalStorage` / `Choice of Model`
14. `CentralHeatPumpSystem` / `Control Method`
15. `CentralHeatPumpSystem` / `Chiller Heater Modules Performance Component Object Type N`

## Proposed Approach ##

Use a two-step cleanup:

1. Internal cleanup
   Simplify runtime code so it no longer depends on these fields where possible, while preserving current input compatibility for one release cycle.

2. Input cleanup
   In the next versioned input transition:
   - remove the fields from `idd/Energy+.idd.in`,
   - regenerate the epJSON schema,
   - add transition rules that drop the removed fields from legacy IDFs,
   - remove obsolete runtime storage, error messages, and dead code tied to those fields.

## Expected Behavior By Category ##

### Stale fields ###

These appear to be dead or dormant selectors and should be removed with a fixed internal behavior:

- `Generator:PVWatts` / `PVWatts Version`
- `SolarCollectorPerformance:FlatPlate` / `Test Fluid`
- `SolarCollectorPerformance:IntegralCollectorStorage` / `ICS Collector Type`

### Redundant fixed-value selectors ###

These are effectively single-option selectors where runtime already assumes one supported behavior:

- `Exterior:WaterEquipment` / `Fuel Use Type`
- `HeatExchanger:Desiccant:BalancedFlow` / `Heat Exchanger Performance Object Type`
- `AirLoopHVAC:UnitarySystem` / `Design Specification Multispeed Object Type`
- `ZoneHVAC:WaterToAirHeatPump` / `Design Specification Multispeed Object Type`
- `ZoneHVAC:TerminalUnit:VariableRefrigerantFlow` / `Design Specification Multispeed Object Type`
- `GroundHeatExchanger:System` / `GHE:Vertical:Sizing Object Type`
- `ChillerHeater:Absorption:DoubleEffect` / `Exhaust Source Object Type`
- `Generator:FuelCell:ElectricalStorage` / `Choice of Model`
- `CentralHeatPumpSystem` / `Control Method`

### Redundant legacy type/name pairs ###

These should be resolved by referenced object name and actual resolved type instead of a separate fixed type field:

- `WaterHeater:HeatPump:WrappedCondenser` / `Tank Object Type`
- `WaterHeater:HeatPump:WrappedCondenser` / `DX Coil Object Type`
- `CentralHeatPumpSystem` / `Chiller Heater Modules Performance Component Object Type N`

## Acceptance Criteria ##

- All in-scope fields are removed from `idd/Energy+.idd.in`.
- The epJSON schema is regenerated and no longer includes the removed fields.
- Transition rules are added so legacy IDFs drop the removed fields cleanly.
- Runtime code no longer stores or depends on removed fields.
- Name-based lookups or hard-coded supported behavior replace the removed selectors as appropriate.
- Targeted input/unit tests are updated or added for the affected objects.
- Regression coverage confirms no expected simulation result changes for these cleanup cases.

## Runtime Areas Likely Affected ##

- `src/EnergyPlus/PVWatts.cc`
- `src/EnergyPlus/SolarCollectors.cc`
- `src/EnergyPlus/ExteriorEnergyUse.cc`
- `src/EnergyPlus/HeatRecovery.cc`
- `src/EnergyPlus/UnitarySystem.cc`
- `src/EnergyPlus/HVACVariableRefrigerantFlow.cc`
- `src/EnergyPlus/WaterThermalTanks.cc`
- `src/EnergyPlus/GroundHeatExchangers/Vertical.cc`
- `src/EnergyPlus/ChillerExhaustAbsorption.cc`
- `src/EnergyPlus/FuelCellElectricGenerator.cc`
- `src/EnergyPlus/PlantCentralGSHP.cc`

## Out of Scope ##

Do not remove one-key fields that still provide useful documentation or strict validation. Examples include:

- `SetpointManager:Warmest` and `SetpointManager:Coldest` strategy fields
- `GeometryTransform` plane selection
- `Output:Surfaces:List` report specification
- template-object fields that are intentionally constrained by the expander

## Good First Batch ##

If this work is split, start with the clearest low-risk cleanup group:

1. `Generator:PVWatts` / `PVWatts Version`
2. `SolarCollectorPerformance:FlatPlate` / `Test Fluid`
3. `SolarCollectorPerformance:IntegralCollectorStorage` / `ICS Collector Type`
4. `Exterior:WaterEquipment` / `Fuel Use Type`
5. `HeatExchanger:Desiccant:BalancedFlow` / `Heat Exchanger Performance Object Type`
6. `Design Specification Multispeed Object Type` on the three affected objects
