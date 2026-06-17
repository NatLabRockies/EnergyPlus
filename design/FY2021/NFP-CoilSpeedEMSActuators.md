# Adding Multispeed Coil Speed Level Control Actuators

**Xuan Luo, Yujie Xu, Tianzhen Hong, Lawrence Berkeley National Laboratory**

- July 2, 2021 - Initial Draft

## Justification for Feature Update

GEBs (Grid-interactive efficient buildings) can respond to grid or pricing signals by adjusting the operation of lighting, plug-in equipment, and HVAC systems. Adjusting HVAC operation strategies include changing thermostat setpoints, turning equipment on/off, and adjusting the speed or capacity of variable-speed or multi-speed compressors and fans.

The typical EnergyPlus approach to modeling two-speed and variable-speed coils is to have a constant setpoint without a deadband and find the appropriate speed ratio to perfectly meet the load. Deadband thermostats are already included in EnergyPlus, but new control logic is needed to model realistic coil behavior to take advantage of this feature. For two-speed residential equipment, when the coil first turns on, it starts at speed 1 for several minutes, checks if the space temperature is moving in the right direction, and then either stays at speed 1 or moves to speed 2.

To resolve this, we propose to make the coil speed level actuatable in EMS. Allowing the coil speed to be actuatable would give a much wider flexibility to model HVAC speed switch strategies that are not limited to the control logic. For example, it would enable variable speed coils (using the DX multispeed object) to exhibit the behavior specified in response to certain load shedding events where the coil can still run, but not at more than a certain percent of the maximum capacity (70% during shed events in AHRI standard 1380).

## Overview

We propose to make the speed level of the cooling and heating multispeed coils, namely `Coil:Cooling:DX:MultiSpeed` and `Coil:Heating:DX:Multispeed`, actuable. Both the multispeed cooling and heating coil can only be referenced by an `AirLoopHVAC:UnitarySystem` or `AirLoopHVAC:UnitaryHeatPump:AirToAir:Multispeed` object.

Currently, both `AirLoopHVAC:UnitarySystem` or `AirLoopHVAC:UnitaryHeatPump` have a reportable variable “Unitary System DX Coil Speed Level” indicating the index of the speed level, and the variable is originally controlled either by load or temperature. We will add EMS actuators to both speed level indices of the unitary system and unitary heat pump, and overwrite the values of both variables at the end of the control logic. We will also add and appropriate EnergyPlus API functionality to allow users to actuate these controls from the EnergyPlus Python Plugin System.

## Approach

The actuators will be implemented to override the unitary system DX coil speed level.

Actuator-specific unique key values are determined from the unitary system (or unitary heat pump) name.
The following example Python codes show a possible implementation of the unique handler.

```python
comp_type = "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed"
ctrl_type = "Unitary System DX Coil Speed Level"

unitary_system_name =  "TwoSpeed Heat Pump 1"
actuator_key = f"{unitary_system_name}"

k_handle = self.api.exchange.get_actuator_handle(state, comp_type, ctrl_type, actuator_key)
```

## Testing/Validation/Data Source(s)

A new example file built upon the existing file, `SingleFamilyHouse_TwoSpeed_ZoneAirBalance.idf` with a unitary heat pump, will be created to demonstrate the use of the new feature EMS actuators and appropriate E+ API. Simulation results will be manually checked.

## Email Discussion

The feature and implementation details were discussed at one of the EnergyPlus Technicality calls. A separate call was also organized with LBNL, NREL, and ORNL teams.

## References

Another branch `HVACFlexMeasures`, owned by ORNL, is working on developing HVAC flexibility measures in EnergyPlus to assess impact of grid-responsive building equipment technologies and energy storage. The coil side focuses on variable-speed coils.
