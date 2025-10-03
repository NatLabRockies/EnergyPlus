Heating and Cooling Fuel Efficiency for Ideal Loads Air System
==============================================================

**Bereket Nigusse**

**Florida Solar Energy Center**

 - First draft: March 5, 2025
 - Modified Date: April 30, 2025
 - Modified Date: June 10, 2025

## Justification for New Feature ##

Ideal loads air systems are increasingly used for early-stage design analysis and in a building energy code compliance method. 
However, currently this object has no built-in method for converting the loads to energy consumption. 
As a result, users have to post-process simulation results to convert the loads to energy. 

**- This enhancement is intended to support loads conversion into energy consumption for ideal loads air system object **
**- Also reduces the burden of simulation results post processing and increase the usability of the object **

## E-mail and  Conference Call Conclusions ##

NA

## Overview ##

(1) ZoneHVAC:IdealLoadsAirSystem reports zone loads but not the equivalent energy consumption.
(2) This object needs a method for converting loads into energy consumptions. 
(3) It doesn't have input fields for specifying fuel efficiency values.


## Approach ##

The following two alternative approaches have been proposed for consideration to achieve the intended goal:

These two options will be discussed and the full NFP will be drafted after the initial discussion.

** Option I: Adding EMS Actuators **
Adds EMS actuator for heating and cooling fuel efficiencies to the ZoneHVAC:IdealLoadsAirSystem object. 

 * - Requires adding two actuators: *
 * - (1) EMS actuator “Heating Fuel Efficiency” for heating *
 * - (2) EMS actuator “Cooling Fuel Efficiency” for cooling *

These actuators pass the efficiency values and give access to the conversion method to calculate the energy consumption during runtime.

** Option II: Adding New INput Fields **
Add optional heating and cooling fuel efficiency input fields to the ZoneHVAC:IdealLoadsAirSystem object. 
The cooling and heating efficiency values will be used to calculate the energy consumed from the zone's ideal loads.

 * - Requires adding two new input fields: *
 * - (1) New Input Field: “Heating Fuel Efficiency Schedule Name” for heating *
 * - (2) New Input Field: “Cooling Fuel Efficiency Schedule Name” for cooling *

For both options, the heating and cooling energy rate and energy consumption will be calculated from the corresponding loads and fuel efficiencies as follows:

	 {E_dot_{heat}} = {Q_dot_{load,heat}} / {Fuel_Eff_{heat}}
	 {E_dot_{cool}} = {Q_dot_{load,cool}} / {Fuel_Eff_{cool}}
	 
	 where,
	 {Q_{dot,cool}}     = Zone Ideal Loads Zone Total Cooling Rate, [W]
	 {Q_{dot,heat}}     = Zone Ideal Loads Zone Total Heating Rate, [W]
	 {E_dot_{cool}}     = Zone Ideal Loads Zone Cooling Fuel Energy Rate, [W]
	 {E_dot_{heat}}     = Zone Ideal Loads Zone Heating Fuel Energy Rate, [W]
	 {E_{cool}}         = Zone Ideal Loads Zone Cooling Fuel Energy, [J]
	 {E_{heat}}         = Zone Ideal Loads Zone Heating Fuel Energy, [J]	 
	 {Fuel_Eff_{cool}}  = Cooling Fuel Efficiency, [-]
	 {Fuel_Eff_{heat}}  = Heating Fuel Efficiency, [-]

** Option II was implemented **
	 
### Questions for Discussion:

* (1) Which alternative approach is preferred? *

*     - Option II requires transition while option I does not *
*     - Option II does not require transition if the new fields are appended as an optional fields * 

* (2) The efficiencies apply to the total (sensible and latent components) heating and cooling loads *

* (3) Existing reporting variables will not change *

* (4) Adds the following new report variables: *

      * - Zone Ideal Loads Zone Cooling Fuel Energy Rate [W] *
      * - Zone Ideal Loads Zone Cooling Fuel Energy [J] *
      * - Zone Ideal Loads Zone Heating Fuel Energy Rate [W] *
      * - Zone Ideal Loads Zone Heating Fuel Energy [J] *

* (5) Consider adding the following new report variables: *

      * - Zone Ideal Loads Supply Air Heating Fuel Energy Rate [W] *
      * - Zone Ideal Loads Supply Air Heating Fuel Energy [J] *
      * - Zone Ideal Loads Supply Air Cooling Fuel Energy Rate [W] *
      * - Zone Ideal Loads Supply Air Cooling Fuel Energy [J] *

      * - these four new variables will also be derived from the corresponding the following report variables by applying the fuel efficiencies *

      * - Zone Ideal Loads Supply Air Total Heating Rate [W] *
      * - Zone Ideal Loads Supply Air Total Cooling Rate [W] *

* (6) What should be the maximum limit for the fuel efficiency value, or no maximum limit?
	  
## Testing/Validation/Data Source(s): ##

Demonstrate that the fuel efficiency input fields set to 1.0 duplicates the current results. Unit tests will be added to demonstrate the enhancement.

## Input Output Reference Documentation ##

Update the documentations for the changed sections.

```
ZoneHVAC:IdealLoadsAirSystem,
       \memo Ideal system used to calculate loads without modeling a full HVAC system. All that is
       \memo required for the ideal system are zone controls, zone equipment configurations, and
       \memo the ideal loads system component. This component can be thought of as an ideal unit
       \memo that mixes zone air with the specified amount of outdoor air and then adds or removes
       \memo heat and moisture at 100% efficiency in order to meet the specified controls. Energy
       \memo use is reported as DistrictHeatingWater and DistrictCooling.
       \min-fields 27
  A1 , \field Name
       \required-field
       \reference ZoneEquipmentNames
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
  A3 , \field Zone Supply Air Node Name
       \note Must match a zone air inlet node name.
       \required-field
       \type node
  A4 , \field Zone Exhaust Air Node Name
       \note Should match a zone air exhaust node name.
       \note This field is optional, but is required if this
       \note this object is used with other forced air equipment.
       \type node
  A5 , \field System Inlet Air Node Name
       \note This field is only required when the Ideal Loads Air System is connected to an
       \note AirloopHVAC:ZoneReturnPlenum, otherwise leave this field blank. When connected to a plenum
       \note the return plenum Outlet Node Name (or Induced Air Outlet Node Name when connecting multiple
       \note ideal loads air systems) is entered here. The two ideal loads air system node name fields described above,
       \note the Zone Supply Air Node Name and the Zone Exhaust Air Node Name must also be entered.
       \note The Zone Supply Air Node Name must match a zone inlet air node name for the zone where this
       \note Ideal Loads Air System is connected. The Zone Exhaust Air Node Name must match an inlet air
       \note node name of an AirloopHVAC:ReturnAirPlenum object.
       \type node
  N1 , \field Maximum Heating Supply Air Temperature
       \units C
       \minimum> 0
       \maximum< 100
       \default 50
  N2 , \field Minimum Cooling Supply Air Temperature
       \units C
       \minimum> -100
       \maximum< 50
       \default 13
  N3 , \field Maximum Heating Supply Air Humidity Ratio
       \units kgWater/kgDryAir
       \minimum> 0
       \default 0.0156
  N4 , \field Minimum Cooling Supply Air Humidity Ratio
       \units kgWater/kgDryAir
       \minimum> 0
       \default 0.0077

        ....

  N10, \field Sensible Heat Recovery Effectiveness
       \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.70
  N11, \field Latent Heat Recovery Effectiveness
       \note Applicable only if Heat Recovery Type is Enthalpy.
       \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.65
  A17, \field Design Specification ZoneHVAC Sizing Object Name
       \note Enter the name of a DesignSpecificationZoneHVACSizing object.
       \type object-list
       \object-list DesignSpecificationZoneHVACSizingName
```

  **  The following four optional NEW input fields were added  **

```
  A18, \field Heating Fuel Efficiency Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Reference heating fuel efficiency value for converting heating 
       \note ideal air loads into fuel energy consumption.
       \note The minimum schedule value must be greater than 0.0. The maximum value
       \note depends on the technology, and can exceed 1.0.
       \note If blank, heating fuel efficiency value is always 1.0.
  A19, \field Heating Fuel Type
       \type choice
       \key Electricity
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Coal
       \key Diesel
       \key Gasoline
       \key OtherFuel1
       \key OtherFuel2
       \key DistrictCooling
       \key DistrictHeatingWater
       \key DistrictHeatingSteam
	   \Default DistrictHeatingWater
  A20, \field Cooling Fuel Efficiency Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note Reference cooling fuel efficiency value for converting cooling 
       \note ideal air loads into fuel energy consumption.
       \note The minimum schedule value must be greater than 0.0. The maximum value
       \note depends on the technology, and can exceed 1.0.
       \note If blank, cooling fuel efficiency value is always 1.0.
  A21; \field Cooling Fuel Type
       \type choice
       \key Electricity
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Coal
       \key Diesel
       \key Gasoline
       \key OtherFuel1
       \key OtherFuel2
       \key DistrictCooling
       \key DistrictHeatingWater
       \key DistrictHeatingSteam
	   \Default DistrictCooling

```   

## Engineering Reference ##

As needed.

## Example File and Transition Changes ##

An example file will be modified to demonstrate the use of availability schedule. Simulation results will be examined and sample results will be provided.

Transition is not required since the new input fields are optional, have defaults, and are appended at the end.

## Proposed Report Variables: ##

As proposed.


## References ##

N/A

## Design ##

### Get Input ###

(1) * Modify the get input function *
	
```
   GetPurchasedAir()
```
    
	(1.1) Convert getinput to JSON format IDD 
	(1.2) Use Enum for all key choice fields
	(1.3) Update the error report functions
	

(2) * Add new member and report variables

    (2.1) Add four new member variables

```
        Sched::Schedule *heatFuelEffSched = nullptr; // heating feul efficiency schedule
        Sched::Schedule *coolFuelEffSched = nullptr; // cooling feul efficiency schedule
		
        Constant::eFuel heatingFuelType = Constant::eFuel::DistrictHeatingWater; // fuel resource type assignment
        Constant::eFuel coolingFuelType = Constant::eFuel::DistrictCooling;      // fuel resource type assignment		
```

    (2.2) Add eight new report variables

```
        Real64 ZoneTotHeatFuelRate;   // Zone total heating fuel energy consumption rate [W]
        Real64 ZoneTotCoolFuelRate;   // zone total cooling fuel energy consumption rate [W]
        Real64 ZoneTotHeatFuelEnergy; // Zone total heating fuel energy consumption [J]
        Real64 ZoneTotCoolFuelEnergy; // Zone total cooling fuel energy consumption [J]
        Real64 TotHeatFuelRate;       // Total heating fuel consumption rate [W]
        Real64 TotCoolFuelRate;       // Total cooling fuel consumption rate [W]
        Real64 TotHeatFuelEnergy;     // Total heating fuel consumption [J]
        Real64 TotCoolFuelEnergy;     // Total cooling fuel consumption [J]
``` 

### IDD Change ###

(1) * Modified the IDD by adding four new input fields *

    *    - (1.1) Add heating_fuel_efficiency_schedule_name new input field *
    *    - (1.2) Add heating_fuel_type new input field *
	*    - (1.3) Add cooling_fuel_efficiency_schedule_name new input field *
	*    - (1.4) Add cooling_fuel_type new input field *

    
### Modify Reporting Function ###

(1) * Modify the ReportPurchasedAir() to support the energy consumption calculation

    *    - (1.1) Fuel Energy Consumption Rate Calculations

```	
    Real64 heatFuelEffValue = PurchAir.heatFuelEffSched->getCurrentVal();
    PurchAir.ZoneTotHeatFuelRate = PurchAir.TotHeatRate / heatFuelEffValue;
    PurchAir.TotHeatFuelRate = PurchAir.ZoneTotHeatRate / heatFuelEffValue;
    Real64 coolFuelEffValue = PurchAir.heatFuelEffSched->getCurrentVal();
    PurchAir.ZoneTotCoolFuelRate = PurchAir.ZoneTotCoolRate / coolFuelEffValue;
    PurchAir.TotCoolFuelRate = PurchAir.TotCoolRate / coolFuelEffValue;
```

    *    - (1.2) Fuel Energy Consumption Calculations
    
```
    PurchAir.ZoneTotHeatFuelEnergy = PurchAir.ZoneTotHeatFuelRate * TimeStepSysSec;
    PurchAir.ZoneTotCoolFuelEnergy = PurchAir.ZoneTotCoolFuelRate * TimeStepSysSec;
    PurchAir.TotHeatFuelEnergy = PurchAir.TotHeatFuelRate * TimeStepSysSec;
    PurchAir.TotCoolFuelEnergy = PurchAir.TotCoolFuelRate * TimeStepSysSec;
```

### Add eight new report variables ###

```
   (1) * Zone Ideal Loads Zone Heating Fuel Energy Rate [W] *
   (2) * Zone Ideal Loads Zone Cooling Fuel Energy Rate [W]            
   (3) * Zone Ideal Loads Zone Heating Fuel Energy [J]
   (4) * Zone Ideal Loads Zone Cooling Fuel Energy [J]
   (5) * Zone Ideal Loads Supply Air Total Heating Fuel Energy Rate [W] *
   (6) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy Rate [W] *
   (7) * Zone Ideal Loads Supply Air Total Heating Fuel Energy [J] *
   (8) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy [J] *
```
