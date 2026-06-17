# Enhancing Electric Multi Stage Coil Stage Level Control Actuators

**Yueyue Zhou, Jeff Maguire, National Renewable Energy Laboratory**

- April 13, 2022 - Initial Draft

## Justification for New Feature

GEBs (Grid-interactive efficient buildings) can respond to grid or pricing signals by adjusting the operation of HVAC, water heaters, and some other systems. Adjusting HVAC operation strategies include changing thermostat setpoints, turning equipment on/off, and adjusting the speed or capacity of variable-speed or multi-speed compressors and fans. The typical EnergyPlus HVAC approach is to have a setpoint without a deadband and find the appropriate speed ratio to perfectly meet the load. If a heating load for an ASHP is unable to be met by DX coils, then the remaining loads will be passed to the supplemental electric coil. With growing demands to model a more realistic on-off cycles and hvac operations, the current supplemental coil capability is limited in two reasons:
1. It currently only allows a single stage electric coil to be attached to *AirLoopHVAC:UnitarySystem* as the supplemental coil. In reality, it's very likely that a multistage backup heating is installed with multispeed heat pumps. *Coil:Heating:Electric:MultiStage* already exists and is able to serve as a supplemental coil for some parent objects, but not *AirLoopHVAC:UnitarySystem*. Enabling this coil in UnitarySystem is a bugfix.
2. The commonly used logic of simply passing remaining loads to backup coils doesn't work well when using an on/off thermostat, where most of time HVAC systems are operating when zone temperature is within deadband instead of being maintained exactly at setpoint. In current E+, supplemental coils will always run fully/at high stage along with DX coil to try to meet the upper end of the deadband as quickly as possible. This will cause higher energy use and shorter cycle than reality, which is not desired.

Therefore, enhancements to the current *Coil:Heating:Electric:MultiStage* object are needed to take advantage of existing GEB capabilities to model realistic HVAC systems in E+.


## Approach

We propose to make Coil:Heating:Electric:MultiStage stage level actuatable by EMS at run-time. The approach is similar to the recently added the Multispeed coil speed level actuators, which are also used to more realistically model the cycling of HVAC. When EMS is used to overwrite the `Unitary System Supplemental Coil Stage Level`, the stage levels will be set based on EMS values rather than the default E+ approach, which with an on/off thermostat will almost always be running at the maximum stage. With any EMS override coil stage value, if the input value is an integer, the stage level is set as the exact stage value input. If a decimal value is entered, it will be rounded to the nearest integer so that the backup coil is always running at one of the user specified stages rather than interpolating. For example, if EMS overrides coil stage value = 1.2, the stage level number is set as 1.

The actuated stage level will be reported as `Unitary System Supplemental Coil Stage Level`. This variable need to be added to EnergyPlus.
We will also add appropriate EnergyPlus API functionality to allow users to actuate these controls from the EnergyPlus Python Plugin System. The actuators will be implemented to override the unitary system supplemental coil stage value. Actuator-specific unique key values are determined from the unitary system name. The following example Python codes show a possible implementation of the unique handler.

```python
comp_type = "AirLoopHVAC:UnitarySystem"
ctrl_type = "Unitary System Supplemental Coil Stage Level"

unitary_system_name =  "Two-Speed Heat Pump"
actuator_key = f"{unitary_system_name}"

k_handle = self.api.exchange.get_actuator_handle(state, comp_type, ctrl_type, actuator_key)
```

## Testing/Validation/Data Source(s)

A new example file built upon the existing file, `UnitarySystem_MultiStageSupplementalCoil.idf`, using the new general coil model, will be created to demonstrate the use of the new feature EMS actuators and appropriate E+ API. Simulation results will be manually checked.

## Email Discussion

The feature and implementation details were based on [issue 9357](https://github.com/NatLabRockies/EnergyPlus/issues/9357)
