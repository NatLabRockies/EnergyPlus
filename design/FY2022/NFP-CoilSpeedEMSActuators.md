# Enhancing Multispeed Coil Speed Level Control Actuators

**Xuan Luo, Yujie Xu, Tianzhen Hong, Lawrence Berkeley National Laboratory**

- Nov 12, 2021 - Initial Draft

## Justification for Feature Update

GEBs (Grid-interactive efficient buildings) can respond to grid or pricing signals by adjusting the operation of lighting, plug-in equipment, and HVAC systems. Adjusting HVAC operation strategies include changing thermostat setpoints, turning equipment on/off, and adjusting the speed or capacity of variable-speed or multi-speed compressors and fans.

The typical EnergyPlus approach to modeling two-speed and variable-speed coils is to have a constant setpoint without a deadband and find the appropriate speed ratio to perfectly meet the load. Deadband thermostats are already included in EnergyPlus, but new control logic is needed to model realistic coil behavior to take advantage of this feature.

Previously in the 9.6.0 release, we added the EMS actuators to control the speed level of the cooling and heating multispeed coils, namely `Coil:Cooling:DX:MultiSpeed` and `Coil:Heating:DX:Multispeed`. As an enhancement, we propose to extend the EMS functionality to a more general coil type in EnergyPlus, `Coil:Cooling:DX`, which will replace the single speed, two speed, variable speed, multi-speed, and multi-stage coils going forward.

## Approach

We propose to make cooling coil speed level and cycling/speed ratio for `Coil:Cooling:DX` actuatable by EMS at run-time. When EMS is used to overwrite the `Unitary System DX Coil Speed Value`, the speed levels will be set based on EMS values rather than speed levels determined by the non-EMS EnergyPlus control logic. The EMS override coil speed value is a continuous number below the maximum coil speed level allowed. With any EMS override coil speed value, if the input value is an integer, the speed level is set as the exact speed value input, with the cycling or speed ratio = 1.0. Otherwise, if the floating-point part is greater than zero, the speed level is calculated as the closest integer greater than the EMS speed value, and the cycling/speed ratio is set as the floating-point part of the EMS speed value. For example, if EMS overrides coil speed value = 1.2, the speed level number is set as 2 with a speed ratio of 0.2.

The actuated speed level, cycling ratio, and speed ratio will be reported as `Unitary System DX Coil Cycling Ratio`, `Unitary System DX Coil Speed Ratio` and `Unitary System DX Coil Speed Level`. These variables already exist in EnergyPlus.
We will also add appropriate EnergyPlus API functionality to allow users to actuate these controls from the EnergyPlus Python Plugin System. The actuators will be implemented to override the unitary system DX coil speed value. Actuator-specific unique key values are determined from the unitary system name. The following example Python codes show a possible implementation of the unique handler.

```python
comp_type = "AirLoopHVAC:UnitarySystem"
ctrl_type = "Unitary System DX Coil Speed Level"

unitary_system_name =  "DXAC Gas Furnace"
actuator_key = f"{unitary_system_name}"

k_handle = self.api.exchange.get_actuator_handle(state, comp_type, ctrl_type, actuator_key)
```
When `Coil:Heating:DX` model is implemented for general heating coils in EnergyPlus, similar logic will be added to control the heating coil speed externally.

## Testing/Validation/Data Source(s)

A new example file built upon the existing file, `UnitarySystem_MultiSpeedDX.idf`, using the new general coil model, will be created to demonstrate the use of the new feature EMS actuators and appropriate E+ API. Simulation results will be manually checked.

## Email Discussion

The feature and implementation details were based on previous PR comments and discussions during EnergyPlus Technicality calls.
