NFP: Liquid-cooled Data Centers
================

**LBNL Scope: ElectricEquipment:ITE:LiquidCooled**

**Yujie Xu, Kaiyu Sun, Tianzhen Hong, LBNL**

 - Original Date: June 2026
 - Revision Date: Sep 2026
 - Status: Draft for Review


## Justification for Feature Update ##

The data center industry is rapidly shifting toward higher adoption of liquid cooling technologies, often involving supply water temperatures between 80F and 120F. Currently, EnergyPlus lacks native support for water-cooled or liquid-cooled IT equipment. Users must rely on complex and inefficient workarounds combining the existing air-cooled IT equipment object with plant load profiles and Energy Management System (EMS) scripting. Previous modeling efforts, such as the MOSTCOOL project, had to rely on PlantComponent:UserDefined to link with external piping modules or use HeatExchanger:FluidToFluid to approximate a Coolant Distribution Unit (CDU). These workarounds are not robust, and relying on external modules defeats the goal of having a native, self-contained EnergyPlus solution.

Furthermore, using simple load profiles like LoadProfile:Plant on the fluid side fails to generate an accurate electrical load for proper meter reporting and cannot dynamically monitor actual chip performance constraints. There is also no native capability to accurately model hybrid data centers that utilize a combination of liquid cooling for high-density chips and air cooling for the remaining components not on the liquid loop. A dedicated native liquid-cooled IT equipment object, paired with specific data center cooling coils (such as Coil:Cooling:ITE:ColdPlate), is required to accurately capture these distinct thermal dynamics—calculating the real-time heat split between the fluid and the zone air, and integrating that load directly and seamlessly into standard EnergyPlus plant loops.

This proposal was reviewed with two industry experts (see E-mail and Conference Call Conclusions). Their feedback confirmed that the intended user of this feature is a building energy modeler, not a chip- or component-level thermal engineer. Accordingly, the new objects are designed to accept the kind of aggregate performance data a cooling vendor already publishes—flow rate, pressure drop, supply/return temperature, and thermal resistance versus flow/temperature curves—rather than requiring the user to model internal cold plate geometry or fluid chemistry. This mirrors how EnergyPlus already treats chillers: a modeler specifies performance curves and connections, not refrigerant selection or internal coil design.

## E-mail and Conference Call Conclusions ##

Two consultation calls were held with the data center industry experts (Dale Sartor and Eric Yang) to review this proposal. Key conclusions that shaped this revision:

- The parent/child architecture (ElectricEquipment:ITE:LiquidCooled plugging into Coil:Cooling:ITE:ColdPlate or Coil:Cooling:ITE:UserDefined) and the Liquid Heat Capture Fraction air/liquid split were both confirmed as the right level of abstraction; no CFD-level detail is needed to represent hybrid racks (e.g., rear-door heat exchanger plus cold plate).
- Cold plates have a vendor-specified maximum flow rate independent of the thermally-derived flow requirement, which becomes more binding as loop temperatures rise and available ΔT shrinks. The downstream cooling coil object needs a hard flow-rate cap.
- Reporting total ITE electricity (CPU + fan), not just CPU, is important for downstream metrics such as PUE and ITUE, and the end-use categorization of that electricity needs to be clean enough to avoid CDU/fan energy being miscounted against IT compute load.
- The liquid-cooled object should not be over-parameterized the way the existing air-cooled object is; detailed hot/cold aisle containment modeling for the residual air-cooled fraction was explicitly deprioritized.
- EnergyPlus's existing plant loop machinery already adequately represents the primary loop; the highest-value native modeling target is the secondary Technology Cooling System (TCS) loop and the IT-to-coolant interface.
- Two adjacent needs were raised that sit outside the core ITE object but are addressed by this revision: short-circuit prevention modeling for air-cooled heat rejection equipment, and thermal energy storage (TES) normal-mode bypass behavior. A built-in default two-phase/immersion cooling model was explicitly not requested; the EMS hook in Coil:Cooling:ITE:UserDefined is considered sufficient for now.
- Existing FluidCooler and EvaporativeFluidCooler objects already cover dry-cooler and adiabatic-cooler primary-loop equipment; the gap is example files and performance curves at data-center-relevant temperatures, not new IDD objects.

## Overview ##

Data center liquid cooling efficiently removes massive amounts of heat from high-density electronics by circulating a liquid capable of absorbing and transporting thermal energy much faster than traditional air systems. The continuous process begins at the chip level, where cold plates attached directly to processors capture heat and transfer it to the circulating coolant. This warmed liquid then flows through flexible tubes and safe, quick-disconnect valves into rack-level manifolds, which aggregate the fluid from multiple servers and route it to a Coolant Distribution Unit (CDU). The CDU acts as a vital bridge, utilizing a liquid-to-liquid heat exchanger to safely transfer the thermal energy from the isolated IT cooling loop into the building's primary water loop without the two fluids ever mixing. Finally, at the building level, large pumps send this heated facility water outside to heavy equipment like chillers, dry coolers, or cooling towers, where the collected heat is rejected into the atmosphere before the newly chilled water cycles back inside to repeat the process.

<img src="figure_1_datacenter_lbl.png" alt="figure-1-datacenter-lbl" width="800px">
</img>
Figure 1. Schematic of data center liquid cooling at chip, rack, and building level.

This feature enhancement will add native liquid cooling modules to EnergyPlus to accurately model liquid-cooled and hybrid data centers. Liquid-cooled IT equipment is defined as systems cooled by a fluid other than air, such as water, glycol, or refrigerants. To manage these systems natively, the project will introduce a parent IT equipment object (ElectricEquipment:ITE:LiquidCooled) that calculates transient power consumption and scales rack-level loads using a multiplier. Rather than creating isolated internal piping networks, this parent object will pass its thermal load to new dedicated cooling coil objects, such as Coil:Cooling:ITE:ColdPlate or Coil:Cooling:ITE:UserDefined. These coils will connect directly to standard EnergyPlus plant loops. This allows standard HeatExchanger:FluidToFluid and pump objects to accurately represent the CDU, seamlessly integrating the IT equipment into the existing plant architecture.

These new IT and coil components will replace the use of basic load profiles. They will utilize actual component-side heat transfer physics (such as overall thermal resistance and maximum allowable chip temperatures) to accurately calculate the required flow rate and return temperature of the liquid loop. Crucially, this enables native hybrid load splitting: the system dynamically calculates the fraction of server heat captured by the cold plates while the remainder is correctly rejected as an air stream load to the zone. Connecting these components directly to the plant loop architecture enables robust evaluation of heat recovery, cogeneration, and water consumption while supporting flexible controls for high-temperature cooling.

Modeling a cold plate—which conducts heat away from the CPU and rejects it into a circulating liquid—can be approached using either detailed computational fluid dynamics (CFD) or reduced-order system models. Table 1 shows the two main calculation pathways and their key inputs. The proposed EnergyPlus implementation will utilize the System-Level lumped parameter approach.

Table 1. Cold plate calculation pathways and key inputs

| Category | Goal | Typical Software Tools | Key Inputs Required |
| --- | --- | --- | --- |
| Component-Level (CFD) | Optimize internal fins, channels, and pressure drop. | Ansys Fluent, Icepak, Star-CCM+, OpenFOAM, SimScale | Precise 3D geometry (CAD), material thermal properties, coolant fluid properties, boundary conditions (inlet flow, chip heat flux). |
| System-Level (Lumped) | Simulate total cooling plant response, energy use, and safety. | Modelica (Buildings Library), Datacor Fathom/Impulse, EnergyPlus | Performance curves (thermal resistance vs mass flow rate), pressure drop coefficients (K), thermal mass/capacitance, total heat load. |

## Approach ##

The development approach focuses on adding a suite of native data center IT and cooling coil objects that integrate directly into the existing EnergyPlus plant loop architecture. By formatting the liquid cooling hardware as standard EnergyPlus coil objects, this approach leverages the robust, existing plant network solver, allowing users to model Coolant Distribution Units (CDUs) using standard HeatExchanger:FluidToFluid and pump objects. Figure 2 shows a schematic diagram of the added components and their relationships.

<img src="figure_2_datacenter_lbl.png" alt="figure-2-datacenter-lbl" width="800px">
</img>
Figure 2. Schematic diagram of the added components.

At the rack level, a new ElectricEquipment:ITE:LiquidCooled parent object natively represents liquid-cooled data center IT equipment racks. This object calculates the total transient IT power load, scales the system using a multiplier for rapid block-modeling of identical racks, and references a specific cooling coil component (e.g., Coil:Cooling:ITE:ColdPlate). By acting as the parent, the IT object calculates the raw thermal load and passes it down to the coil to determine the physical heat transfer split between the liquid loop and the zone air.

At the cooling coil level, the simulation establishes the absolute maximum physical cooling capacity based on the user-defined maximum allowable chip temperature, the real-time fluid inlet conditions provided by the plant loop, and the cold plate's overall thermal resistance (or heat transfer coefficient). During the simulation, the engine continuously scales the nominal thermal resistance using a bivariate modifier curve to account for changing system conditions like varying flow fractions and fluid temperatures. For standard single-phase systems, this physical heat transfer ceiling is calculated using the sensible heat capacity of the liquid and the combined solid and convective thermal resistances.

The final heat transferred into the fluid is set as the smaller value between the targeted rack heat load and this dynamically calculated physical limit. Because the coil is plant equipment and is simulated during the HVAC/plant solution—after the zone's internal gains, including ElectricEquipment:ITE:LiquidCooled, have already been evaluated for that system timestep—the coil cannot report a capacity-checked value back to the parent object in time for that same timestep's zone load prediction. The coil therefore owns the full load-splitting decision and registers any uncaptured heat directly against its own zone or space via `SetupZoneInternalGain`, exactly as DXCoils.cc already does for a DX coil's optional condenser heat rejection to a zone (its `Zone Name for Condenser Placement` field) and as RefrigeratedCase.cc does for compressor rack condensers. This reuses an accepted, precedented pattern rather than introducing a new one: the zone's predictor step sees the coil's prior-timestep result, and the corrector step picks up the freshly computed value once the coil has run—the same one-phase lag those existing objects already carry.

While single-phase cooling is the standard for most applications, the proposed Coil:Cooling:ITE:UserDefined object provides an extensible framework for complex or emerging technologies. Rather than hardcoding two-phase fluid properties into EnergyPlus, this object provides Energy Management System (EMS) hooks. Users can deploy custom scripts to calculate two-phase latent heat limits, Rear Door Heat Exchanger (RDHx) air-side interactions, or immersion cooling dynamics, and use EMS actuators to directly override the coil's plant node conditions. Table 2 shows the key fields of the added components.

Table 2. Core fields of the new objects

| Object or interface | Purpose | Key inputs/outputs | Notes |
| --- | --- | --- | --- |
| ElectricEquipment:ITE:LiquidCooled | Calculates transient server power consumption and passes the resulting thermal load to a specified cooling coil. | Zone name, operating schedule, design power input, multiplier, cooling coil object type, cooling coil name. | Uses a multiplier field for rapid scaling of identical racks. Acts as the parent object defining total heat generation; passes the full target liquid load to the coil and reports—rather than itself registering—the resulting liquid/air split via its output variables. |
| Coil:Cooling:ITE:ColdPlate | Defines the physical heat transfer between the IT equipment and the standard E+ plant loop for single-phase coolants; owns the final liquid/air load split. | Heat transfer input method (UA or thermal resistance), maximum allowable chip temperature, fluid inlet/outlet nodes, design pressure drop, design maximum flow rate, **Zone or Space Name** (air-spillover destination). | Establishes the maximum thermal ceiling, dictating when cooling capacity is maxed out and residual heat must spill over into the zone air. The coil registers that spillover itself via `SetupZoneInternalGain` against its own Zone or Space Name field, mirroring the `Zone Name for Condenser Placement` field already on DX coils (DXCoils.cc) rather than passing a value back through the parent. Note: this object is implemented under a separate NFP led by PNNL; this table flags the `Design Maximum Flow Rate` and `Zone or Space Name` fields as requirements for that implementation, representing the vendor-specified physical flow limit independent of the thermally-derived flow. No IDD for this object is authored here. |
| Coil:Cooling:ITE:UserDefined | Provides a hook for EMS/Python-driven workflows to override default heat transfer physics at the IT-to-coolant interface. | Fluid inlet/outlet nodes, EMS Program Calling Manager Name, Design Fluid Flow Rate. | Allows researchers to model proprietary two-phase boiling behaviors, RDHx, or immersion cooling while retaining standard E+ plant loop connectivity. This is the only supported path for two-phase/immersion modeling in this phase; no default built-in two-phase model is provided. NLR will lead the implementation of this object/feature. |

At a high level, the ElectricEquipment:ITE:LiquidCooled object evaluates the total IT load, applies the Liquid Heat Capture Fraction to get a target liquid load, and passes that entire target down to the referenced Coil:Cooling:ITE:ColdPlate object—not a pre-split, already-capped value. The coil checks the current fluid supply conditions from the facility plant loop and calculates the maximum physical heat transfer possible given its thermal properties, its Design Maximum Flow Rate, and the maximum allowable chip temperature. It absorbs min(target, physical limit) into the liquid loop and returns that achieved value back to the parent, which uses it only to populate the ITE object's own Liquid Heat Gain and Air Heat Gain to Zone output variables—the parent no longer registers any part of the air spillover as its own zone gain.

Any remaining heat, calculated as the target liquid load minus the achieved liquid load, is diverted directly into the data center space as a sensible air load—but the coil pushes this heat itself, via its own `SetupZoneInternalGain` registration against the Zone or Space Name field on the coil object (see Table 2), rather than the parent object doing so. Finally, the E+ plant loop automatically aggregates the heat and fluid flow from all connected cold plate coils. This aggregated hot fluid travels through the standard E+ plant pipe network to the secondary side of a HeatExchanger:FluidToFluid (representing the CDU), which then rejects the data center heat to the primary facility cooling plant.

### Zone Heat Gain Registration for Air Spillover ###

The air-spillover heat is registered directly by the cooling coil rather than by the parent ElectricEquipment:ITE:LiquidCooled object. This follows the existing DXCoils.cc pattern (the `Zone Name for Condenser Placement` field, which calls `SetupZoneInternalGain` from within the coil's own `GetInput`) and the equivalent compressor-rack condenser handling in RefrigeratedCase.cc, rather than introducing a new mechanism.

This choice is not just a code-organization preference: ElectricEquipment:ITE:LiquidCooled is a zone internal gain, evaluated once per system timestep before HVAC/plant is simulated, while Coil:Cooling:ITE:ColdPlate is plant equipment, evaluated later in the same timestep's HVAC/plant solution. The parent therefore cannot obtain a capacity-checked split from the coil in time to register it as its own gain for that timestep's zone load prediction. Letting the coil self-register through the standard `SetupZoneInternalGain` pointer mechanism means the zone's predictor step sees the coil's prior-timestep result and the corrector step sees the freshly computed one—the same one-phase lag already accepted for DX coil secondary/condenser heat rejection and refrigeration rack condensers.

Practically, this means the coil object must carry its own `Zone or Space Name` field (see Table 2) rather than inheriting one from the parent, and the placeholder code in InternalHeatGains.cc that currently computes `Air Heat Gain = Total Heat Generation − Liquid Heat Gain` and registers it as the ITE object's own zone gain (necessary only because the coil objects do not yet exist) will need to be replaced once the coil lands: the parent will keep using the achieved liquid load solely to populate its own output variables, and the coil's `SetupZoneInternalGain` registration becomes the sole path for the uncaptured heat reaching the zone air balance.

### Scope: Primary Loop vs. Secondary (TCS) Loop ###

EnergyPlus's existing plant loop architecture (pumps, pipes, HeatExchanger:FluidToFluid, chillers, cooling towers, fluid coolers) already adequately represents the primary facility cooling loop, including high-temperature chilled water loops, provided the loop components are given performance curves valid at data-center-relevant supply temperatures. The highest-value target for native modeling is therefore the secondary Technology Cooling System (TCS) loop and the IT-to-coolant interface, which is what ElectricEquipment:ITE:LiquidCooled and its cooling coils are designed to capture. Hybrid racks that combine a rear-door heat exchanger (liquid-to-air) with direct-to-chip cold plates (liquid-to-chip) are both represented through the same Liquid Heat Capture Fraction split; a separate rear-door-specific object is not needed.

### Cold Plate Flow Rate Limit ###

Real cold plates cannot supply arbitrarily high flow rates: the flow rate a manufacturer can physically deliver is capped independently of what the thermal calculation would otherwise require, and this becomes more binding as loop supply temperatures rise and the available ΔT across the cold plate shrinks. Whichever cooling coil object ultimately implements Coil:Cooling:ITE:ColdPlate (a separate NFP) needs a `Design Maximum Flow Rate` field representing this vendor-specified physical limit. When the thermally-derived flow requirement would exceed this cap, the coil should cap the flow at the maximum and divert the resulting shortfall in captured heat to the air spillover path, pushing it into the zone through the same coil-side `SetupZoneInternalGain` registration described above for when thermal capacity is the binding constraint.

Consistent with this vendor-data-driven approach, no internal pump or fan hardware within the cold plate or CDU is separately modeled inside the ITE or coil objects—standard Pump:VariableSpeed or Pump:ConstantSpeed objects on the secondary plant loop represent this circulation, exactly as they would for any other plant coil.

### Explicitly Out of Scope ###

Two features raised in the review are outside the scope of this first implementation:

- **Detailed hot/cold aisle containment modeling for the liquid-cooled object's air spillover.** The uncaptured heat fraction remains a simple sensible convective gain to the zone, consistent with keeping ElectricEquipment:ITE:LiquidCooled from accumulating the same parameter count as ElectricEquipment:ITE:AirCooled. Users who need detailed approach-temperature or containment-leakage modeling for an air-cooled portion of their data center should continue to use ElectricEquipment:ITE:AirCooled, which already supports FlowControlWithApproachTemperatures and a recirculation fraction for this purpose.
- **A built-in default two-phase or immersion cooling model.** Coil:Cooling:ITE:UserDefined provides EMS hooks so users with their own two-phase or immersion heat transfer calculations (e.g., custom Python-derived curves) can actuate the coil's plant node conditions directly. No default physics-based two-phase model is provided in this phase.

### Companion Additions: Short-Circuit Prevention and TES Bypass ###

Two adjacent needs, outside the core ITE object but related to the liquid cooling scope, were identified:

- **Short-circuit prevention on air-cooled heat rejection equipment.** Dry coolers and adiabatic ("enclosed swamp cooler"-style) coolers are commonly used as the primary heat rejection equipment for data center TCS loops (represented by the existing FluidCooler:SingleSpeed, FluidCooler:TwoSpeed, EvaporativeFluidCooler:SingleSpeed, and EvaporativeFluidCooler:TwoSpeed objects). In the field, recirculation of hot discharge air back into the intake degrades performance, and physical barriers (baffles, pads) are a common retrofit to prevent it. Each of these four objects will gain an optional `Condenser Air Inlet Temperature Adder` field (plus an optional companion schedule) so users can represent this degradation—or model the benefit of adding a short-circuit prevention barrier—without a new object, mirroring the existing Design Recirculation Fraction / Supply Temperature Difference pattern already used in ElectricEquipment:ITE:AirCooled.
- **TES normal-mode bypass.** ThermalStorage:ChilledWater:Mixed and ThermalStorage:ChilledWater:Stratified already support charge and discharge operation. However, the sequence described in review keeps a small fixed bypass flow (e.g., ~10%) through the tank branch during normal (non-charge/discharge) operation to prevent stagnant fluid in the loop, which is not clearly achievable with current plant equipment operation controls. This NFP proposes a `Design Bypass Flow Fraction` field for normal-mode operation on these TES objects, to be finalized once a detailed sequence-of-operations document is reviewed.

## Testing/Validation/Data Source(s) ##

The feature will be tested and demonstrated with a test file derived from a baseline liquid-cooling data center model using a dry cooler as the cooling source, `1ZoneDataCenterCRAHandplant-liquidcooling-NoEMS-drycooler-2CDUs.idf`. Manual checks of the time-step EnergyPlus simulation results will be conducted to ensure the new components accurately calculate secondary loop performance compared to previous HeatExchanger:FluidToFluid approximations.

Additional example files are planned to cover configurations raised in review:

- A hybrid rack combining a rear-door heat exchanger (modeled using the Coil:Cooling:ITE:UserDefined object) and direct-to-chip cold plates (modeled using the Coil:Cooling:ITE:ColdPlate) on the same liquid loop, exercising the Liquid Heat Capture Fraction split.
- A dry-cooler-only primary loop operating at high secondary loop supply temperature, to validate that existing FluidCooler and chiller performance curves can be parameterized for data-center-relevant conditions without new IDD objects.
- A TES example modeling the normal/charging/discharging sequence once a detailed sequence-of-operations document is available from industry review.

Performance curves for high-temperature chillers and dry/adiabatic coolers will be sourced from industry partners (e.g., Trane) where available, supplemented by ASHRAE 205-based data as it becomes available for this equipment class. If a data center operator or vendor is able to share metered data (a possibility raised with Trane and NVIDIA customer contacts), the example files will additionally be validated against real operating data rather than only against the prior HeatExchanger:FluidToFluid approximation.

## Input Output Reference Documentation ##

N/A

## Input Description ##

The following new IDD objects will be added.

```
ElectricEquipment:ITE:LiquidCooled,
  \memo Represents liquid-cooled data center IT equipment racks.
  \memo Calculates power consumption and rejects heat to ITE cooling coils.
  A1 , \field Name
       \required-field
       \type alpha
       \reference ITEAndITEListNames
  A2 , \field Zone or Space Name
       \required-field
       \type object-list
       \object-list ZoneAndSpaceNames
       \note Zone or Space the IT equipment is located in.
       \note Spillover air heat will be rejected to this space's heat balance.
  A3 , \field Availability Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Availability schedule name for this equipment. Schedule value > 0 means the equipment is on.
       \note If this field is blank, the equipment is always available.
  A4 , \field Compute Load Schedule Name
       \required-field
       \type object-list
       \object-list ScheduleNames
       \note Defines the transient CPU loading schedule.
       \note This 0-1 factor multiplied by the design power input is the current CPU power.
  N1 , \field Design Power Input
       \required-field
       \type real
       \units W
       \note Max power consumption of a single IT equipment rack.
  N2 , \field Multiplier
       \type real
       \default 1.0
       \minimum 1.0
       \note Scales power and heat to represent multiple identical racks.
  N3 , \field Design Fan Power Input Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
       \note Retained for auxiliary server fans contributing to air load.
  A5 , \field IT Equipment Power Modifier Curve Name
       \type object-list
       \object-list BivariateFunctions
       \note Modifies power based on loading and inlet temperature.
  N4 , \field Liquid Heat Capture Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
       \default 0.8
       \note The fraction of the total ITE heat generation (CPU + Fan) that is removed by the liquid cooling loop at design conditions. The remaining fraction is transferred to the zone air.
  A6 , \field Liquid Heat Capture Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note If provided, this schedule multiplies the Liquid Heat Capture Fraction field. This allows the capture effectiveness to vary dynamically during the simulation.
  A7 , \field Cooling Coil 1 Object Type
       \required-field
       \type choice
       \key Coil:Cooling:ITE:ColdPlate
       \key Coil:Cooling:ITE:UserDefined
       \note The type of the first cooling component handling physical heat transfer.
  A8 , \field Cooling Coil 1 Name
       \required-field
       \type object-list
       \object-list CoilCoolingITENames
       \note The specific name of the first cooling component handling physical heat transfer.
  N5 , \field Cooling Coil 1 Load Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
       \note A static fraction (0.0 to 1.0) of the total IT liquid load directed to this coil.
       \note If this field is used, the Schedule Name field below should be left blank.
  A9 , \field Cooling Coil 1 Load Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Schedule (0.0 to 1.0) defining the fraction of the total IT liquid load directed to this coil.
       \note If this field is used, the static Load Fraction field above should be left blank.
  A10, \field Cooling Coil 2 Object Type
       \type choice
       \key Coil:Cooling:ITE:ColdPlate
       \key Coil:Cooling:ITE:UserDefined
       \note The type of the second cooling component, if applicable (e.g., a secondary RDHx).
  A11, \field Cooling Coil 2 Name
       \type object-list
       \object-list CoilCoolingITENames
  N6 , \field Cooling Coil 2 Load Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
  A12, \field Cooling Coil 2 Load Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
  A13, \field Cooling Coil 3 Object Type
       \type choice
       \key Coil:Cooling:ITE:ColdPlate
       \key Coil:Cooling:ITE:UserDefined
  A14, \field Cooling Coil 3 Name
       \type object-list
       \object-list CoilCoolingITENames
  N7 , \field Cooling Coil 3 Load Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
  A15, \field Cooling Coil 3 Load Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
  A16, \field Cooling Coil 4 Object Type
       \type choice
       \key Coil:Cooling:ITE:ColdPlate
       \key Coil:Cooling:ITE:UserDefined
  A17, \field Cooling Coil 4 Name
       \type object-list
       \object-list CoilCoolingITENames
  N8 , \field Cooling Coil 4 Load Fraction
       \type real
       \minimum 0.0
       \maximum 1.0
  A18, \field Cooling Coil 4 Load Fraction Schedule Name
       \type object-list
       \object-list ScheduleName
  A19, \field CPU End-Use Subcategory
       \type alpha
       \retaincase
       \default ITE-CPU
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
  A20; \field Fan End-Use Subcategory
       \type alpha
       \retaincase
       \default ITE-Fans
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
```

The `CPU End-Use Subcategory` and `Fan End-Use Subcategory` fields mirror the equivalent fields already present on `ElectricEquipment:ITE:AirCooled`, so that CPU and fan electricity can be separated in ABUPS reporting. This directly addresses the concern that CDU pump and fan energy is often bundled into IT compute load when calculating PUE- and ITUE-adjacent metrics.

The following additional fields, outside the ElectricEquipment:ITE:LiquidCooled object itself, are also proposed based on the review:

- `FluidCooler:SingleSpeed`, `FluidCooler:TwoSpeed`, `EvaporativeFluidCooler:SingleSpeed`, `EvaporativeFluidCooler:TwoSpeed`: an optional `Condenser Air Inlet Temperature Adder` field (real, units deltaC, default 0.0) and an optional companion `Condenser Air Inlet Temperature Adder Schedule Name` field, to represent hot-air short-circuiting/recirculation at the condenser intake, or the benefit of adding a physical short-circuit prevention barrier.
- `ThermalStorage:ChilledWater:Mixed`, `ThermalStorage:ChilledWater:Stratified`: a proposed `Design Bypass Flow Fraction` field for normal (non-charge/discharge) mode operation, pending a detailed sequence-of-operations document from industry review. The exact field definition is not yet finalized.

## Outputs Description ##

The outputs for `ElectricEquipment:ITE:LiquidCooled` are as follows
```
   Zone,Average,ITE CPU Electricity Rate [W]
   Zone,Sum,ITE CPU Electricity Energy [J]
   Zone,Average,ITE Fan Electricity Rate [W]
   Zone,Sum,ITE Fan Electricity Energy [J]
   Zone,Average,ITE Total Electricity Rate [W]
   Zone,Sum,ITE Total Electricity Energy [J]
   Zone,Average,ITE Total Heat Generation Rate [W]
   Zone,Sum,ITE Total Heat Generation Energy [J]
   Zone,Average,ITE Liquid Heat Capture Fraction []
   Zone,Average,ITE Liquid Heat Gain Rate [W]
   Zone,Sum,ITE Liquid Heat Gain Energy [J]
   Zone,Average,ITE Air Heat Gain to Zone Rate [W]
   Zone,Sum,ITE Air Heat Gain to Zone Energy [J]
```

The ITE CPU electricity rate and energy outputs represent the power and energy consumed specifically by the compute components of the IT equipment. The ITE fan electricity rate and energy outputs represent the power and energy consumed by the internal fans used to assist with thermal management within the equipment. The ITE total electricity rate and energy outputs represent the overall power and energy consumption of the liquid-cooled IT equipment, which is the sum of the CPU and fan electricity usage.

The ITE total heat generation rate and energy outputs represent the entire thermal load produced by the equipment, which is inherently equal to the total electricity consumed. The ITE liquid heat capture fraction reports the final fraction of heat being routed to the liquid loop at the current timestep, accounting for the design input field and any modifying schedules. The ITE liquid heat gain rate and energy outputs report the amount of this total heat generation that is captured and removed directly by the attached liquid cooling loop. The ITE air heat gain to zone rate and energy outputs represent the remaining thermal fraction that is not captured by the liquid loop and is instead dissipated into the surrounding zone air as a sensible heat gain. In addition to these core object-level outputs, EnergyPlus will automatically generate corresponding Space and Zone level aggregations for every variable listed above.

The CPU and Fan End-Use Subcategory fields (see Input Description) let the CPU and Fan electricity outputs above roll up separately in the ABUPS End Uses by Subcategory table, so downstream PUE- and ITUE-adjacent analysis can distinguish IT compute load from auxiliary fan load rather than relying on manual post-processing.

## Engineering Reference ##

N/A

## Example File and Transition Changes ##

N/A

## References ##

- Consultation calls with industry reviewers Dale Sartor and Eric Yang, September 2026 (see E-mail and Conference Call Conclusions).
