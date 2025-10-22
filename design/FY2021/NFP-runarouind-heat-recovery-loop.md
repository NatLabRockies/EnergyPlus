Run-around Heat Recovery Loop
================

**R. Raustad, FSEC**

 - Original Date - 2/5/21 DRAFT NFP
 - Revision Date - 8/2/21 FINAL NFP and Design Doc


## Justification for New Feature ##

Heat recovery is common in HVAC systems. A heat exchanger can easily be installed in an air system where two air steams are side-by-side. However, when the air steams are not located near each other, application of heat recovery is more difficult. Designers have overcome this issue by using water coils connected with piping and a pump to create a closed loop system where energy is transfered between these two coil. Third parties are not able to model this type of system in EnergyPlus.

## E-mail and  Conference Call Conclusions ##

Several conference calls have guided this new feature over the last few months. The model evolved as:
1) allow coil-on-branch configuration with existing plant loop (HVACController did not work)
2) allow coil-on-branch configuration with RootSolver style controller (water temps did not converge and SimHVAC max itertion warnings appeared)
3) create new class for CoilSystem:WaterCoil:HeatRecovery and initialize plant loop water temp on FirstHVACIteration (good control)
4) use new object CoilSystem:Cooling:Water and integrate into UnitarySystem with similar plant loop water temperature initialization (same good result, much less code overhead)

## Overview ##

The following is an except of the abstract from <http://what-when-how.com/energy-engineering/run-around-heat-recovery-systems-energy-engineering/>.

Run-around heat-recovery systems are often used to recover heat from the exhaust air in building ventilation systems, particularly in cold climates. In a typical heat-recovery system, an ethylene glycol and water solution is used as a “coupling fluid” to prevent the system from freezing. The design of a run-around heat-recovery system involves consideration of the heat-transfer rates between the fluids. A challenging problem is how to significantly increase those rates between the fluids, while using less energy for heating and cooling loads in buildings. Due to the heat-transfer characteristics of this coupling fluid and the operating and capital cost factors, the typical overall effectiveness of such systems is only about 50%. Studies show that two-phase, gas-liquid coupling fluids have much higher convective heat-transfer rates than single-phase flows at the same mass flow rates.

The following figures and text were provided in the Trane TRACE 700 User’s Manual • CDS-PRM001-EN at <https://software.trane.com/RightNow/0103-CoilLoopEnergyRecovery/Coil_loop_exhaust-air_energy_recovery.pdf>.

A simple schematic of a run-around coil heat recovery loop is shown in Figure 1. Insert A and B show heat recovery between two air stream while insert C shows energy transfer from before to after a cooling coil (similar to `CoilSystem:Cooling:DX:HeatExchangerAssisted`.

![RunaroundCoilLoop](RunaroundCoilHXLoop.png)
## *Figure 1. Run around coil loop schematic.* ##

Figure 2 shows an example run-around coil design for winter and summer operation. During the heating season (Inset A), heat extracted from the exhaust air stream (EA)
warms the air brought into the building. Operation of the coil loop is limited to prevent the supply-air temperature from exceeding the cooling set point. (This condition is most likely to occur on mild days during the spring and fall.) Preconditioning the outdoor
air (OA) in this manner reduces the heating load, which in turn reduces the energy consumption of the HVAC system.

![RunaroundCoilDesignExample](RunaroundCoilDesignExample.png)
## *Figure 2. Run around coil design example.* ##

## Approach ##

There are 3 water coil models in EnergyPlus.

1. Coil:Cooling:Water
2. Coil:Cooling:Water:DetailedGeometry
3. Coil:Heating:Water

As shown in figure 2, one coil heats in winter and cools in summer while the other does the reverse as is typical in heat recovery components. For this reason, a water coil model that can exchange heat based on the entering air and water temperature is required. It is believed that the cooling coil model behaves this way (i.e., can heat the air under certain conditions). I have also seen this a few times and MJWitte also believes the water cooling coil model has this capability. Through testing it has been confirmed Coil:Cooling:Water has the ability to cool or heat the air/water.

There are two possible approaches I can think of.

1. Connect 2 water cooling coils in a typical plant configuration. With one coil on the demand side of the plant loop while the other is a supply side component. This would allow alternate configurations if desired. For example to allow a boiler, chiller, evap cooler, etc. on the supply side which maintains a certain temperature to the supply side coil. Although I can't think of a reason to do this, it seems like a very flexible way to achieve the goal for a run-around coil application. A user could create a plant loop and then add two coils and a pump.

**Pro:** This approach may require minimal changes to the IDD (the PlantLoop field for Plant Equipment Operation Scheme Name is required). Would provide flexibility in configuration. Pipe losses could be modeled.
**Con:** The coil model would use ControlcompOutput as the solution algorithm. The air-side and water-side calculations would be disassociated the same as water coils on a branch are currently modeled.

I'm not yet sure if objects other than the PlantLoop are even needed for a loop with 2 coils and a pump. Without a plant component (e.g., boiler or chiller) would the other typical plant objects shown here even be needed?

    PlantLoop,
      Chilled Water Loop,      !- Name
      Water,                   !- Fluid Type
      ,                        !- User Defined Fluid Type
      CW Loop Operation,       !- Plant Equipment Operation Scheme Name
      CW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name
      98,                      !- Maximum Loop Temperature {C}
      1,                       !- Minimum Loop Temperature {C}
      autosize,                !- Maximum Loop Flow Rate {m3/s}
      0.0,                     !- Minimum Loop Flow Rate {m3/s}
      ,                        !- Plant Loop Volume {m3}
      CW Supply Inlet Node,    !- Plant Side Inlet Node Name
      CW Supply Outlet Node,   !- Plant Side Outlet Node Name
      Cooling Supply Side Branches,  !- Plant Side Branch List Name
      Cooling Supply Side Connectors,  !- Plant Side Connector List Name
      CW Demand Inlet Node,    !- Demand Side Inlet Node Name
      CW Demand Outlet Node,   !- Demand Side Outlet Node Name
      Cooling Demand Side Branches,  !- Demand Side Branch List Name
      Cooling Demand Side Connectors,  !- Demand Side Connector List Name
      SequentialLoad,          !- Load Distribution Scheme
      CW Avail List;           !- Availability Manager List Name

    PlantEquipmentOperationSchemes,
      CW Loop Operation,       !- Name
      PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type
      Central Chiller Only,    !- Control Scheme 1 Name
      PlantOnSched;            !- Control Scheme 1 Schedule Name

    PlantEquipmentOperation:CoolingLoad,
      Central Chiller Only,    !- Name
      0,                       !- Load Range 1 Lower Limit {W}
      900000,                  !- Load Range 1 Upper Limit {W}
      Cooling Plant;           !- Range 1 Equipment List Name

    PlantEquipmentList,
      Cooling Plant,           !- Name
      Chiller:Electric,        !- Equipment 1 Object Type
      Central Chiller;         !- Equipment 1 Name


2. Connect two water cooling coils together water outlet node of one coil is connected directly to the water inlet node of the other coil. A pump could be connected in series with these coils. Using this approach a new object could be created that specifies the type and name of the water coils, and also the pump. Since these components would be in an air stream, air inlet and outlet node name would be needed. Two of these objects would be required to complete the configuration and these objects could be placed in any air loop directly on a branch. A tentative approach would be to model both coils when any object is simulated.

**Pro:** The ControlCompOutput solution algorithm would not be required. RootSolver could be used instead and the solution of the air and water sides of this loop could be solved at one time.
**Con:** This approach would require an IDD change. There is also no real pipe objects so pipe losses could not be modeled. Pipe objects should be considered as part of this effort (i.e., use of plant and demand side branch and connector lists).

    CoilSystem:WaterCoil:HeatExchanger,
    A1,  Name
    A2,  Companion CoilSystem:WaterCoil:HeatExchanger object name
    A2,  Cooling Coil Type
    A3,  Cooling Coil Name
    A6,  Pump Type
    A7,  Pump Name
    A8,  Coil system inlet air node name
    A9;  Coil system outlet air node name

**Update:** the new CoilSystem:Cooling:Water object will be used to replace this suggestion. This new object was recently added to UnitarySystem and is in the process of being merged into E+ develop. Three new inputs were added to allow control of the new run-around coil loop.

     CoilSystem:Cooling:Water,
     ...
    A10, \field Economizer Lockout
         \type choice
         \key Yes
         \key No
         \default Yes
         \note Yes means that the heat recovery will be locked out (off)
     N2, \field Minimum Water Loop Temperature For Heat Recovery
         \note Only used for heat recovery loops.
         \note Loop will turn off below this temperature.
         \type real
         \units C
         \default 0.0
    A11; \field Companion Coil Used For Heat Recovery
         \note Only used for heat recovery loops.
         \note Entering a coil name indicates a heat recovery loop is specified.
         \note Coil listed is connected in series with this objects coil on demand side
         \note branch of a plant loop. A dedicated plant loop with no supply side
         \note equipment, other than a pump, is currently required.
         \note Only Coil:Cooling:Water coil type is currently allowed for heat recovery loops.
         \type object-list
         \object-list CoolingCoilsWater


## Design Document ##
- Implement new feature using existing CoilSystem:Cool:Water object
- Add new IDD inputs for model control
- Incorporate new inputs in function getCoilWaterSystemInputData
- Add controls to Init
- Execute model in function controlCoolingSystemToSP

## Testing/Validation/Data Sources ##

A run around coil system will be tested in the OA system. Other configurations will be investigated, such as return to supply of an air loop.

## Input Output Reference Documentation ##

Add to new documentation for CoilSystem:Cooling:Water

## Input Description ##

TBD

## Outputs Description ##

TBD

## Engineering Reference ##

TBD

## Example File and Transition Changes ##

An example file will be created highlighting the use and connections for this new feature.

## References ##

<http://www.colmaccoil.com/media/42064/run-around-coil-loop-heat-recovery.pdf>

<https://software.trane.com/RightNow/0103-CoilLoopEnergyRecovery/Coil_loop_exhaust-air_energy_recovery.pdf>

<http://what-when-how.com/energy-engineering/run-around-heat-recovery-systems-energy-engineering/>
