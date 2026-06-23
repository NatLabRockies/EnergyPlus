Liquid-Cooled Plant Equipment
===============================

**Jeremy Lerond, Pacific Northwest National Laboratory**

 - Original Date: 06/2026
 - Revision Date: 06/2026

## Justification for New Feature ##

The rapid expansion of datacenter projects across the US has driven industry stakeholders to request native modeling capabilities for liquid-cooled ITE in EnergyPlus, as documented in [[0]][Issue #11408]. Currently, EnergyPlus lacks plant equipment capable of modeling loads introduced by ElectricEquipment:ITE objects. This NFP proposes a new plant equipment object designed to capture these loads and interact with supply-side plant equipment to satisfy them.

## E-mail and Conference Call Conclusions ##

## Overview ##

[[0]][Issue #11408] provides an overview of current liquid-cooled datacenter technologies. Expanded information is available in [[1]][Datacenter Liquid Cooling Market Characterization, Final Report, 12/12/2025, CalNEXT]. Based on these resources, rear-door heat exchanger (RDHx) and direct-to-chip (D2C) technologies emerge as the dominant solutions. Immersion cooling remains limited to experimental applications and is not yet widely deployed. Consequently, this proposal focuses on developing a new plant equipment object capable of simulating both RDHx and D2C technologies.

## Approach ##

One new plant object will be implemented as part of the new feature proposal. This new object will be used to model either D2C or RDHx cooling.

### D2C
The new object will simulate D2C devices as a cold plate (liquid-to-solid) heat exchanger. The heat transfer through the cold plate can be characterized by the interface's case-to-fluid thermal resistance. As documented in [[2]][Wang et. al], [[3]][Cheng et. al], [[4]][Shaeri et. al], and [[6]][Martinez et. al], the case-to-fluid thermal resistance of a cold plate, $R_{th}$, is expressed as follows: 

$R_{th} = \frac{T_{case} - T_f}{\dot{q}}$

Where:
- $T_{case}$ is the case temperature (see $T_{c}$ in Figure 1 below where TIM is the thermal interface material)
- $T_{f}$ is the liquid coolant temperature
- $\dot{q}$ is the heat transfer rate

As mentioned in [[2]][Wang et. al], $T_{f}$ corresponds to the inlet fluid temperature for single-phase coolant, while for two-phase coolant $T_{f}$ is taken as the saturation temperature inside the cold plate.

The enthalpy of the fluid leaving the cold plate will be calculated as follows:

$h_{out} = h_{in} + \frac{\dot{q}}{\dot{m}}$

Temperature and other properties will subsequently be calculated based on the fluid properties data.

<img src="./cold_plate.jpg" alt="Project Screenshot" width="400">

_Figure 1 - Cross Section of a Cold Plate_ (source: [[6]][Martinez et. al])


[[5]][Eaton] shows that cold plate designs are selected based on a maximum allowable cold plate temperature. Since $R_{th}$ is a function of $T_{case}$, a user-defined maximum case temperature will be used to estimate the maximum heat transfer rate at nominal conditions. As mentioned in [[0]][Issue #11408], D2C systems are not always able to address all IT-generated loads, and air systems are used to meet the remainder. When the maximum $T_{case}$ is reached, the rest of the heat generated will be added to the zone heat balance.

[[2]][Wang et. al], [[3]][Cheng et. al], and [[4]][Shaeri et. al] show that the thermal resistance varies as a function of coolant flow rate and heat flux (for two-phase flow), so the nominal thermal resistance will be adjusted using a user-defined empirical curve.

The proposed object will be added by users to the demand side of a `PlantLoop` and, together with a `HeatExchanger:FluidToFluid` object and a pump (on the supply side of the same `PlantLoop`), will represent a coolant distribution unit (CDU). As requested by [[0]][Issue #11408], this approach allows D2C to "_be connected to existing EnergyPlus PlantLoop architecture_" and "_support for liquids with other heat transfer properties would be handled at the loop level with the fluid_type and user_defined_fluid_type fields_". Coolant properties can be specified using the `FluidProperties` family of objects. Note that PG25, which according to the literature (see [[2]][Wang et. al]) is a widely used single-phase coolant (a mixture of 25% propylene glycol and water), is natively supported by EnergyPlus when using the `FluidProperties:GlycolConcentration` object. Two-phase heat transfer is not currently handled in EnergyPlus, so we propose to focus on single-phase D2C this FY and add capabilities for two-phase D2C in the next FY.

Moreover, [[6]][Martinez et. al] proposes an LMTD-based thermal resistance definition for single-phase liquid-cooled cold plates, which could be considered an alternative approach to the traditional thermal resistance. We propose to focus on the $R_{th}$ approach for now, since it appears to be the current standard practice.

### RDHx

RDHxs are air-to-liquid heat exchangers placed on the back of server cabinets. RDHx can be either active or passive. The former uses dedicated fans to push hot air from the server to the coils, while the latter relies on natural airflow and server rack fans only.

The new object will simulate RDHx by assuming that the system operates as a sensible-only air-to-liquid heat exchanger. Heat transfer and air-/liquid-stream conditions will be analyzed using the effectiveness-NTU method.

The connection to the ITE equipment will be used to determine the conditions of air entering the rack and how much heat is added by the equipment. A user-defined temperature schedule will be used to define the targeted leaving air temperature. Knowing the entering and leaving air temperatures, the airflow rate can be determined.

[[8]][Legrand] shows that active RDHx systems are equipped with electronically commutated fans capable of variable-speed control, so the new object will assume a variable-speed airflow to meet the load. Fan heat from the dedicated fan system will be added to the load. For passive systems, airflow through the heat exchanger depends on server rack fans and natural ventilation and is therefore difficult to predict. Instead of using the dedicated fan input, users can specify an airflow schedule — for instance, a constant flow rate schedule if server rack fans operate at a constant nominal flow rate during ITE equipment operation. Server rack fan power will be accounted for by the ITE equipment, but could also be accounted for by the auxiliary power input. Auxiliary power inputs were requested in [[0]][Issue #11408]: "_Allow specification of auxiliary electric power associated with the liquid cooling system_".

## Testing/Validation/Data Sources ##

TBD

## Input Output Reference Documentation ##

TBD

## Input Description ##

```
Coil:Cooling:ITE
    \memo This object can be used to simulate a cold-plate or rear-door heat exchanger for liquid-cooled datacenter equipment
    A1, \field Name
        \required-field
        \type alpha
    A2, \field ITE Equipment
        \reference ITEequipments
        \note Name of an ElectricEquipment:ITE:LiquidCooled object
    A3, \field Inlet Node
        \note Inlet node name
        \required-field
        \type node
    A4, \field Outlet Node
        \note Outlet node name
        \required-field
        \type node
    A5, \field Cooling Type
        \note Type of liquid-cooled technology to be simulated
        \type choice
        \key ColdPlate
        \key RearDoorHeatExchangerActive
        \key RearDoorHeatExchangerPassive
        \required-field
    N1, \field Cold Plate Nominal Thermal Resistance
        \note This is the cold plate thermal resistance at the nominal flow rate
        \default 0.025
        \minimum> 0.0
        \units K/W
    N2, \field Cold Plate Maximum Case Temperature
        \note Maximum allowed case temperature
        \minimum> 0.0
    A6, \field Cold Plate Thermal Resistance Degradation Curve Name
        \type object-list
        \object-list UnivariateFunctions
        \object-list BivariateFunctions
        \note Flow rate ratio (Flow rate / Nominal Flow Rate) is used as the first independent variable
        \note Heat transfer rate ratio (Heat transfer rate / Nominal Transfer Rate) is used as the second independent variable
    A7, \field Cold Plate Fluid Temperature for Thermal Resistance Calculation
        \note Reference temperature used to calculate the cold plate's thermal resistance
        \note Liquid Inlet Temperature is typical for single-phase coolant
        \note Saturation Temperature is typical for two-phase coolant
        \type choice
        \key Liquid Inlet Temperature
        \key Saturation Temperature
    N3, \field Rear Door Heat Exchanger UA
        \units W/K
    N4, \field Rear Door Heat Exchanger Fan Power
        \note Nominal power of the dedicated fan (only when Cooling Type is RearDoorHeatExchangerActive)
        \units W
    A8, \field Rear Door Heat Exchanger Fan Power Modifier Function of Flow Fraction
        \note The output of this curve adjusts the nominal fan power based on varying flow fraction (actual fan air flow rate / nominal fan flow rate)
        \type object-list
        \object-list UnivariateFunctions
    A9, \field Rear Door Heat Exchanger Fan Flow Rate Schedule Name
        \note Specifies the flow rate through the rear door heat exchanger (only used when Cooling Type is RearDoorHeatExchangerPassive)
        \note Schedule values should be in m3/s
        \type object-list
        \object-list ScheduleNames
    A10, \field Rear Door Heat Exchanger Leaving Air Temperature Setpoint Schedule Name
        \note Temperature schedule that represents the targeted leaving air setpoint temperature
        \note Schedule values should be in degrees C
        \type object-list
        \object-list ScheduleNames
    N5, \field Nominal Liquid Flow Rate
        \note Nominal liquid flow rate of the device
        \autosizable
        \minimum> 0.0
    N6, \field Nominal Air Flow Rate
        \note Nominal air flow rate of the device (only for rear door heat exchangers)        
        \autosizable
        \minimum> 0.0
    N7, \field Auxiliary Electric Power
        \type real
        \units W
        \minimum 0.0
        \ip-units W
        \default 0.0
        \note The auxiliary electric power input in watts when the unit is running
    A11, \field ZoneName
        \note Zone where excess heat from the heat exchanger will be discharged
        \type object-list
        \object-list ZoneNames
    A12; \field End-Use Subcategory
        \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
        \type alpha
        \retaincase
        \default General
```

## Outputs Description ##

TBD

## Engineering Reference ##

TBD

## Example File and Transition Changes ##

This new object, along with the other liquid-cooled data center objects, will be integrated into a single example file. This effort will be coordinated with other entities involved in the development of these liquid-cooled data center objects.

No transition will be required.

## References ##

- [0] Issue #11408

[Issue #11408]: https://github.com/NatLabRockies/EnergyPlus/issues/11408

- [1] Datacenter Liquid Cooling Market Characterization, Final Report, 12/12/2025, CalNEXT 

[Datacenter Liquid Cooling Market Characterization, Final Report, 12/12/2025, CalNEXT]: https://calnext.com/wp-content/uploads/2025/12/ET24SWE0065_Datacenter-Liquid-Cooling-Market-Characterization_Final-Report.pdf

- [2] Universal Direct-to-Chip Cold Plates for Single- and Two-Phase Cooling, Wang et. al

[Wang et. al]: https://accelsius.com/wp-content/uploads/Universal-Direct-to-Chip-Cold-Plate-2.pdf

- [3] OCP OAI System Liquid Cooling Guidelines, Cheng et. al

[Cheng et. al]: https://www.opencompute.org/documents/oai-system-liquid-cooling-guidelines-in-ocp-template-mar-3-2023-update-pdf

- [4] Demonstration of CTE-Matched Two-Phase Minichannel Heat Sink, Shaeri et. al

[Shaeri et. al]: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10177605&utm_source=sciencedirect_contenthosting&getft_integrator=sciencedirect_contenthosting

- [5] Selecting a Liquid Cold Plate Technology

[Eaton]: https://www.eaton.com/us/en-us/products/thermal-management-solutions/cold-plate-heat-exchanger/selecting-a-cold-plate-technology-and-performance-comparison.html

- [6] Experimental validation of the effectiveness-NTU approach for single-phase liquid cold plates and a consistent definition of thermal resistance

[Martinez et. al]: https://www.sciencedirect.com/science/article/pii/S0017931025004673

- [7] Performance Comparison of R1233zd(E) and R515B for Two-Phase Direct-to-Chip Cooling

[Wang et. al 2]: https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=11235787

- [8] Rear-Door Heat Exchangers Smart Cooling at the Rack Level

[Legrand]: https://www.legrand.com/datacenter/at-en/news/rear-door-heat-exchangers-rdhx-smart-cooling-at-the-rack-level

- [9] A Practical Metric for Cold Plate Thermal Performance in Two-Phase Direct-to-Chip Cooling

[Wang et. al 3]: https://accelsius.com/wp-content/uploads/A-Practical-Metric-for-Cold-Plate-Thermal-Performance-in-Two-Phase-Direct-to-Chip-Cooling.pdf

- [10] Experimental evaluation of direct-to-chip cold plate liquid cooling for high-heat-density data centers

[Heydari et. al]: https://www.sciencedirect.com/science/article/pii/S1359431123021518?casa_token=yMz4c59NaAoAAAAA:njacRyk_Tos7PSyigDx1yWpRixkV3-Vbiqv1IR7vkgFqLkSTbfue6P6pvFMKMt6bWbwFj4f9aQ