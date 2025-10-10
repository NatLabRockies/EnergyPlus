Add Availability Schedule Input Fields to DX Coils
==================================================

**Bereket Nigusse, Florida Solar Energy Center**

 - First draft: March 6, 2025
 - Modified Date: March 20, 2025

## Justification for New Feature ##

Add an availability schedule input field to DX Coil objects that currently lack one. Eleven DX Coils that don’t have an availability schedule were identified, so turning them On or Off when desired is impossible. The availability schedule allows turning the coils On or Off regardless of load. Plans are to add availability schedule input fields to all eleven DX coils identified; however, the implementation scope could be limited depending on the budget.


## E-mail and  Conference Call Conclusions ##

** Put the availability schedule name input field as the second input field for all the seven DX coils **

## Overview ##

EnergyPlus code will be modified to add availability schedule input field to DX coils and adds a logic to turn On or Off depending on the schedule value.

### Current Code ###

(1) Seven DX Coils do not have availability schedule input fields
(2) Table below summarizes the DX Coils missing availability schedule input field

| S. No. | DX Coil Type | EnergyPlus Module | Priorities |
|--|--|--|--|
|1	|Coil:Cooling:DX:VariableSpeed| VariableSpeedCoils | 1 |
|2	|Coil:Heating:DX:VariableSpeed| VariableSpeedCoils | 1 |
|3	|Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit| VariableSpeedCoils | 1 |
|4	|Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit| VariableSpeedCoils | 1 |
|5	|Coil:WaterHeating:AirToWaterHeatPump:VariableSpeed| VariableSpeedCoils | 1 |
|6	|Coil:Cooling:WaterToAirHeatPump:ParameterEstimation| WaterToAirHeatPump | 1 |
|7	|Coil:Heating:WaterToAirHeatPump:ParameterEstimation| WaterToAirHeatPump | 1 |
|8	|Coil:Cooling:WaterToAirHeatPump:EquationFit| WaterToAirHeatPumpSimple|	1 |
|9	|Coil:Heating:WaterToAirHeatPump:EquationFit| WaterToAirHeatPumpSimple|	1 |
|10	|Coil:WaterHeating:AirToWaterHeatPump:Pumped| DXCoils |	2 |
|11	|Coil:WaterHeating:AirToWaterHeatPump:Wrapped| DXCoils | 2 |


(3) **- This enhancement allows operational flexibility **.

## Approach ##

Adds availability schedule new input field and logic that turns On or Off the DX coils based on the availability schedule value. The steps envisioned for this enhancement include:

*(1) Cleanup get inputs and other functions including CPP check for each of the DX coil modules *

*(2) Change the get input function to JSON format when possible *

*(3) Update IDD; add new input field for each of the DX coils identified *

*(4) Add “Availability Schedule Name” new input field to each of the DX coils identified *

*(5) Implement the availability schedule functionality logic in calc function for each of the DX coils identified *

*(6) Demonstrate the change through example and unit tests *

*(7) Add transition rules for each of the DX coils identified *

*(8) Update the latex reference documentation *


### DX Coils Missing Availability Schedule ###

* IDD change snippet for each of the DX Coils to be modified *

* Adds new input field: "Availability Schedule Name" *

* Update the min-fields number *

	`Coil:Cooling:DX:VariableSpeed`
	`Coil:Heating:DX:VariableSpeed`
	`Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit`
	`Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit`
	`Coil:WaterHeating:AirToWaterHeatPump:VariableSpeed`
	`Coil:Cooling:WaterToAirHeatPump:ParameterEstimation`
	`Coil:Heating:WaterToAirHeatPump: ParameterEstimation`
	`Coil:Cooling:WaterToAirHeatPump:EquationFit`
	`Coil:Heating:WaterToAirHeatPump:EquationFit`
	`Coil:WaterHeating:AirToWaterHeatPump:Pumped`
	`Coil:WaterHeating:AirToWaterHeatPump:Wrapped`

```

Coil:Cooling:DX:VariableSpeed,
        \memo Direct expansion (DX) cooling coil and condensing unit (includes electric compressor
        \memo and condenser fan), variable-speed. Optional inputs for moisture evaporation from
        \memo wet coil when compressor cycles off with continuous fan operation. Requires two to
        \memo ten sets of performance data and will interpolate between speeds. Modeled as a
        \memo single coil with variable-speed compressor.
        \min-fields 35
   A1,  \field Name
        \required-field
        \type alpha
        \reference CoolingCoilsDXVariableSpeed
        \reference DesuperHeatingCoilSources
```

   ** Add new input field A2: Availability Schedule Name *

```
   A3,  \field Indoor Air Inlet Node Name
        \required-field
        \type node
   A4,  \field Indoor Air Outlet Node Name
        \required-field
        \type node
   A5 , \field Availability Schedule Name
        \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
   N1,  \field Number of Speeds
        \units dimensionless
        \type integer
        \minimum 1
        \maximum 10
        \default 2

    ...


   A49, \field Speed 10 Energy Input Ratio Function of Temperature Curve Name
        \type object-list
        \object-list BivariateFunctions
        \note curve = a + b*wb + c*wb**2 + d*odb + e*odb**2 + f*wb*odb
        \note wb = entering wet-bulb temperature (C)
        \note odb = air entering temperature seen by the condenser (C)
   A50; \field Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name
        \type object-list
        \object-list UnivariateFunctions
        \note quadratic curve = a + b*ffa + c*ffa**2
        \note cubic curve = a + b*ffa + c*ffa**2 + d*ffa**3
        \note ffa = Fraction of the full load Air Flow

```


```
Coil:Heating:DX:VariableSpeed,
        \memo Direct expansion (DX) heating coil (air-to-air heat pump) and compressor unit
        \memo (includes electric compressor and outdoor fan), variable-speed, with defrost
        \memo controls. Requires two to ten sets of performance data and will interpolate between
        \memo speeds.
        \min-fields 26
   A1,  \field Name
        \required-field
        \type alpha
        \reference HeatingCoilsDXVariableSpeed
```

   ** Add new input field A2: Availability Schedule Name *

```
   A3,  \field Indoor Air Inlet Node Name
        \required-field
        \type node
   A4,  \field Indoor Air Outlet Node Name
        \required-field
        \type node
   A5 , \field Availability Schedule Name
        \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
   N1,  \field Number of Speeds
        \units dimensionless
        \type integer
        \minimum 1
        \maximum 10
        \default 2

        ...

   A47, \field Speed 10 Energy Input Ratio Function of Temperature Curve Name
        \type object-list
        \object-list BivariateFunctions
        \note curve = a + b*db + c*db**2 + d*oat + e*oat**2 + f*db*oat
        \note db = entering air dry-bulb temperature (C)
        \note oat = air entering temperature seen by the evaporator (C)
   A48; \field Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name
        \type object-list
        \object-list UnivariateFunctions
        \note quadratic curve = a + b*ffa + c*ffa**2
        \note cubic curve = a + b*ffa + c*ffa**2 + d*ffa**3
        \note ffa = Fraction of the full load Air Flow

```


```
Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit,
        \memo Direct expansion (DX) cooling coil for water-to-air heat pump (includes electric
        \memo compressor), variable-speed, equation-fit model. Optional inputs for moisture
        \memo evaporation from wet coil when compressor cycles off with continuous fan operation.
        \memo Equation-fit model uses normalized curves to describe the heat pump performance.
        \memo Requires two to ten sets of performance data and will interpolate between speeds.
        \memo Modeled as a single coil with variable-speed compressor.
        \min-fields 30
   A1,  \field Name
        \required-field
        \type alpha
        \reference CoolingCoilsWaterToAirVSHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
   A2,  \field Water-to-Refrigerant HX Water Inlet Node Name
        \required-field
        \type node
   A3,  \field Water-to-Refrigerant HX Water Outlet Node Name
        \required-field
        \type node
   A4,  \field Indoor Air Inlet Node Name
        \required-field
        \type node
   A5,  \field Indoor Air Outlet Node Name
        \required-field
        \type node
   A6 , \field Availability Schedule Name
        \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
```

```
   N1,  \field Number of Speeds
        \units dimensionless
        \type integer
        \minimum 1
        \maximum 10
        \default 2

    ...

   N71, \field Speed 10 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions
        \units dimensionless
        \type real
        \minimum 0
   A76; \field Speed 10 Waste Heat Function of Temperature Curve Name
        \note optional
        \type object-list
        \object-list BivariateFunctions
        \note curve = a + b*wb + c*wb**2 + d*ewt + e*ewt**2 + f*wb*ewt
        \note wb = entering wet-bulb temperature (C)
        \note ewt = water entering temperature seen by the condenser (C)
```

```
Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit,
        \memo Direct expansion (DX) heating coil for water-to-air heat pump (includes electric
        \memo compressor), variable-speed, equation-fit model. Equation-fit model uses normalized
        \memo curves to describe the heat pump performance. Requires two to ten sets of performance
        \memo data and will interpolate between speeds.
        \min-fields 23
   A1,  \field Name
        \required-field
        \type alpha
        \reference HeatingCoilsWaterToAirVSHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
   A2,  \field Water-to-Refrigerant HX Water Inlet Node Name
        \required-field
        \type node
   A3,  \field Water-to-Refrigerant HX Water Outlet Node Name
        \required-field
        \type node
   A4,  \field Indoor Air Inlet Node Name
        \required-field
        \type node
   A5,  \field Indoor Air Outlet Node Name
        \required-field
        \type node
   A6 , \field Availability Schedule Name
        \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
```

```
   N1,  \field Number of Speeds
        \units dimensionless
        \type integer
        \minimum 1
        \maximum 10
        \default 2

    ...

   N55, \field Speed 10 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions
        \units dimensionless
        \type real
        \minimum 0
   A76; \field Speed 10 Waste Heat Function of Temperature Curve Name
        \note optional
        \type object-list
        \object-list BivariateFunctions
        \note curve = a + b*db + c*db**2 + d*ewt + e*ewt**2 + f*db*ewt
        \note db = entering air dry-bulb temperature (C)
        \note ewt = water entering temperature seen by the evaporator (C)
```

```
Coil:WaterHeating:AirToWaterHeatPump:VariableSpeed,
       \memo variable-speed Heat pump water heater (VSHPWH) heating coil, air-to-water direct-expansion (DX)
       \memo system which includes a variable-speed water heating coil, evaporator air coil, evaporator
       \memo fan, electric compressor, and water pump. Part of a WaterHeater:HeatPump system.
       \min-fields 34
  A1 , \field Name
       \required-field
       \type alpha
       \reference HeatPumpWaterHeaterDXCoilsVariableSpeed
       \note Unique name for this instance of a variable-speed heat pump water heater DX coil.
```

   ** Add new input field A2: Availability Schedule Name *

```
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
  N1,  \field Number of Speeds
       \units dimensionless
       \type integer
       \minimum 1
       \maximum 10
       \default 1

    ...

  A70, \field Speed 10 COP Function of Air Flow Fraction Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Table:Lookup object can also be used
       \note quadratic curve = a + b*ffa + c*ffa**2
       \note cubic curve = a + b*ffa + c*ffa**2 + d*ffa**3
       \note ffa = Fraction of the full load Air Flow
  A71; \field Speed 10 COP Function of Water Flow Fraction Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Table:Lookup object can also be used
       \note quadratic curve = a + b*ffw + c*ffw**2
       \note cubic curve = a + b*ffw + c*ffw**2 + d*ffw**3
       \note ffw = Fraction of the full load Water Flow
```

```
Coil:Cooling:WaterToAirHeatPump:ParameterEstimation,
        \memo Direct expansion (DX) cooling coil for water-to-air heat pump (includes electric
        \memo compressor), single-speed, parameter estimation model. Optional inputs for moisture
        \memo evaporation from wet coil when compressor cycles off with continuous fan operation.
        \memo Parameter estimation model is a deterministic model that requires a consistent set of
        \memo parameters to describe the operating conditions of the heat pump components.
        \min-fields 32
   A1 , \field Name
        \required-field
        \type alpha
        \reference CoolingCoilsWaterToAirHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
```

   ** Add new input field A2: Availability Schedule Name *

```
   A2 , \field Availability Schedule Name
        \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
```

```
   A3 , \field Compressor Type
        \required-field
        \type choice
        \key Reciprocating
        \key Rotary
        \key Scroll
        \note Parameters 1-5 are as named below.
        \note Parameters 6-10 depend on the type of compressor and fluid.
        \note Refer to the InputOutputReference on the parameters required

    ...

   N22, \field Latent Capacity Time Constant
        \note Time constant for the cooling coil's latent capacity to reach steady state after
        \note startup. Suggested value is 45; zero value means latent degradation model is disabled.
        \type real
        \units s
        \minimum 0.0
        \maximum 500.0
        \default 0.0
   N23; \field Fan Delay Time
        \units s
        \minimum 0.0
        \default 60
        \note Programmed time delay for heat pump fan to shut off after compressor cycle off.
        \note Only required when fan operating mode is cycling
        \note Enter 0 when fan operating mode is continuous
```

```
Coil:Heating:WaterToAirHeatPump:ParameterEstimation,
        \memo Direct expansion (DX) heating coil for water-to-air heat pump (includes electric
        \memo compressor), single-speed, parameter estimation model. Parameter estimation model is
        \memo a deterministic model that requires a consistent set of parameters to describe
        \memo the operating conditions of the heat pump components.
        \min-fields 26
   A1 , \field Name
        \required-field
        \type alpha
        \reference HeatingCoilsWaterToAirHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
```

   ** Add new input field A2: Availability Schedule Name *

```
   A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
   A3 , \field Compressor Type
        \required-field
        \type choice
        \key Reciprocating
        \key Rotary
        \key Scroll
        \note Parameters 1-4 are as named below.
        \note Parameters 5-9 depend on the type of compressor.
        \note Refer to the InputOutputReference on the parameters required

    ...

   N17, \field Source Side Heat Transfer Resistance2
        \note Use when Source Side Fluid Name is an antifreeze
        \note Leave this field blank for Source Side Fluid is Water
        \note Previously part of Parameter 9
        \units W/K
        \minimum 0.0
        \type real
   A9;  \field Part Load Fraction Correlation Curve Name
        \note quadratic curve = a + b*PLR + c*PLR**2
        \note cubic curve = a + b*PLR + c*PLR**2 + d*PLR**3
        \note PLR = part load ratio (heating load/steady state capacity)
        \required-field
        \type object-list
        \object-list UnivariateFunctions
```

```
Coil:Cooling:WaterToAirHeatPump:EquationFit,
        \memo Direct expansion (DX) cooling coil for water-to-air heat pump (includes electric
        \memo compressor), single-speed, equation-fit model. Optional inputs for moisture
        \memo evaporation from wet coil when compressor cycles off with continuous fan operation.
        \memo Equation-fit model uses normalized curves to describe the heat pump performance.
        \min-fields 23
   A1,  \field Name
        \required-field
        \type alpha
        \reference CoolingCoilsWaterToAirHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
        \reference DesuperHeatingWaterOnlySources
```

   ** Add new input field A2: Availability Schedule Name *

```
   A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
   A3,  \field Water Inlet Node Name
        \required-field
        \type node
   A4,  \field Water Outlet Node Name
        \required-field
        \type node
   A5,  \field Air Inlet Node Name
        \required-field
        \type node
   A6,  \field Air Outlet Node Name
        \required-field
        \type node
   N1,  \field Rated Air Flow Rate
        \required-field
        \type real
        \minimum> 0.0
        \units m3/s
        \autosizable

    ...

   N12, \field Latent Capacity Time Constant
        \note Time constant for the cooling coil's latent capacity to reach steady state after
        \note startup. Suggested value is 45; zero value means latent degradation model is disabled.
        \type real
        \units s
        \minimum 0.0
        \maximum 500.0
        \default 0.0
   N13; \field Fan Delay Time
        \units s
        \minimum 0.0
        \default 60
        \note Programmed time delay for heat pump fan to shut off after compressor cycle off.
        \note Only required when fan operating mode is cycling
        \note Enter 0 when fan operating mode is continuous
```

```
Coil:Heating:WaterToAirHeatPump:EquationFit,
        \memo Direct expansion (DX) heating coil for water-to-air heat pump (includes electric
        \memo compressor), single-speed, equation-fit model. Equation-fit model uses normalized
        \memo curves to describe the heat pump performance.
        \min-fields 16
   A1,  \field Name
        \required-field
        \type alpha
        \reference HeatingCoilsWaterToAirHP
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
```

   ** Add new input field A2: Availability Schedule Name *

```
   A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
   A3,  \field Water Inlet Node Name
        \required-field
        \type node
   A4,  \field Water Outlet Node Name
        \required-field
        \type node
   A5,  \field Air Inlet Node Name
        \required-field
        \type node
   A6,  \field Air Outlet Node Name
        \required-field
        \type node
   N1,  \field Rated Air Flow Rate
        \required-field
        \type real
        \minimum> 0.0
        \units m3/s
        \autosizable

    ...

   A8,  \field Heating Power Consumption Curve Name
        \required-field
        \type object-list
        \object-list QuadvariateFunctions
   A9;  \field Part Load Fraction Correlation Curve Name
        \note quadratic curve = a + b*PLR + c*PLR**2
        \note cubic curve = a + b*PLR + c*PLR**2 + d*PLR**3
        \note PLR = part load ratio (heat load/steady state capacity)
        \required-field
        \type object-list
        \object-list UnivariateFunctions
```


```
Coil:WaterHeating:AirToWaterHeatPump:Pumped,
       \memo Heat pump water heater (HPWH) heating coil, air-to-water direct-expansion (DX)
       \memo system which includes a water heating coil, evaporator air coil, evaporator
       \memo fan, electric compressor, and water pump. Part of a WaterHeater:HeatPump:PumpedCondenser system.
       \min-fields 23
  A1 , \field Name
       \required-field
       \type alpha
       \reference HeatPumpWaterHeaterDXCoilsPumped
       \note Unique name for this instance of a heat pump water heater DX coil.
```

   ** Add new input field A2: Availability Schedule Name *

```
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
  N1 , \field Rated Heating Capacity
       \required-field
       \type real
       \units W
       \minimum> 0
       \note Heating capacity at the rated inlet air temperatures, rated condenser inlet
       \note water temperature, rated air flow rate, and rated water flow rate.
       \note Can optionally include condenser pump heat.

    ...

  A17, \field Heating COP Function of Water Flow Fraction Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Heating COP modifier curve (function of water flow fraction) should be quadratic or cubic.
       \note Quadratic curve = a + b(ff) + c(ff)^2.
       \note Cubic curve = a + b(ff) + c(ff)^2 + d(ff)^3.
       \note ff = fraction of the rated condenser water flow rate.
       \note Use curve coefficients of 1,0,0 or leave this field blank when neglecting performance impacts
       \note due to variations in water flow rate fraction.
  A18; \field Part Load Fraction Correlation Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Part Load Fraction Correlation (function of part load ratio) should be quadratic or cubic.
       \note Quadratic curve = a + b(PLR) + c(PLR)^2.
       \note Cubic curve = a + b(PLR) + c(PLR)^2 + d(PLR)^3.
       \note PLR = part load ratio (heating delivered/steady state heating capacity).
       \note Use curve coefficients of 1,0,0 or leave this field blank when neglecting performance impacts
       \note due to variations in part load ratio.
```

```
Coil:WaterHeating:AirToWaterHeatPump:Wrapped,
       \memo Heat pump water heater (HPWH) heating coil, air-to-water direct-expansion (DX)
       \memo system which includes a water heating coil, evaporator air coil, evaporator
       \memo fan, electric compressor, and water pump. Part of a WaterHeater:HeatPump:WrappedCondenser system.
       \min-fields 16
  A1 , \field Name
       \required-field
       \type alpha
       \reference HeatPumpWaterHeaterDXCoilsWrapped
       \note Unique name for this instance of a heat pump water heater DX coil.
```

   ** Add new input field A2: Availability Schedule Name *

```
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this DX coil. Schedule value > 0 means the DX coil is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
```

```
  N1 , \field Rated Heating Capacity
       \required-field
       \type real
       \units W
       \minimum> 0
       \note Heating capacity at the rated inlet air temperatures, rated condenser inlet
       \note water temperature, rated air flow rate, and rated water flow rate.
       \note Can optionally include condenser pump heat.

    ...

  A11, \field Heating COP Function of Air Flow Fraction Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Heating COP modifier curve (function of air flow fraction) should be quadratic or cubic.
       \note Quadratic curve = a + b(ff) + c(ff)^2.
       \note Cubic curve = a + b(ff) + c(ff)^2 + d(ff)^3.
       \note ff = fraction of the rated evaporator air flow rate.
       \note Use curve coefficients of 1,0,0 or leave this field blank when neglecting performance impacts
       \note due to variations in air flow rate fraction.
  A12; \field Part Load Fraction Correlation Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Part Load Fraction Correlation (function of part load ratio) should be quadratic or cubic.
       \note Quadratic curve = a + b(PLR) + c(PLR)^2.
       \note Cubic curve = a + b(PLR) + c(PLR)^2 + d(PLR)^3.
       \note PLR = part load ratio (heating delivered/steady state heating capacity).
       \note Use curve coefficients of 1,0,0 or leave this field blank when neglecting performance impacts
       \note due to variations in part load ratio.
```


## Testing/Validation/Data Source(s): ##

Demonstrate that the availability schedule input field scheduled off duplicates the current results using exact set of inputs. Unit tests will be added to demonstrate the new feature.


## Input Output Reference Documentation ##

Update the "group-heating-and-cooling-coils.tex" file by adding the new input field description for each of the DX coils identified. The following latex content will be added depending on the
DX coil type:

```
\paragraph{Field: Availability Schedule Name}\label{field-availability-schedule-name-16-001}

This alpha field defines the name of the schedule (ref: Schedule) that denotes whether the DX heating coil can run during a given time period.
A schedule value of 0 indicates that the DX heating coil is off for that time period. If the schedule's value is \textgreater{} 0 (usually 1 is used),
the coil is On duing that time period. If this field is blank, the schedule has values of 1 for all time periods. Schedule values must be \textgreater{}= 0 and \textless{}= 1.


\paragraph{Field: Availability Schedule Name}\label{field-availability-schedule-name-16-002}

This alpha field defines the name of the schedule (ref: Schedule) that denotes whether the DX cooling coil can run during a given time period.
A schedule value of 0 indicates that the DX cooling coil is off for that time period. If the schedule's value is \textgreater{} 0 (usually 1 is used),
the coil is On duing that time period. If this field is blank, the schedule has values of 1 for all time periods. Schedule values must be \textgreater{}= 0 and \textless{}= 1.


\paragraph{Field: Availability Schedule Name}\label{field-availability-schedule-name-16-003}

This alpha field defines the name of the schedule (ref: Schedule) that denotes whether the DX Water Heating coil can run during a given time period.
A schedule value of 0 indicates that the DX Water Heating coil is off for that time period. If the schedule's value is \textgreater{} 0 (usually 1 is used),
the coil is On duing that time period. If this field is blank, the schedule has values of 1 for all time periods. Schedule values must be \textgreater{}= 0 and \textless{}= 1.

```


## Engineering Reference ##

As needed.


## Example File and Transition Changes ##

An example file will be modified to demonstrate the use of availability schedule. Simulation results will be examined and sample results will be provided.

Transition is required and transition rules will be created.


## Proposed Report Variables: ##

N/A.


## References ##

N/A

## Design ##


### Module Clean up ###

*(1) Cleanup the module and do CPP Check *

```
    a. VariableSpeedCoils
    b. WaterToAirHeatPump
    c. WaterToAirHeatPumpSimple
    d. DXCoils {if budget allows}
```

### Get Input ###

(1) * Modify the get input function to JSON format when possible *

```
    a. GetVarSpeedCoilInput()
    b. GetWatertoAirHPInput()
    c. GetSimpleWatertoAirHPInput()
    d. GetDXCoils()  {if budget allows}
```

*(2) Update IDD and adjust the min-fields count *
*(3) Add the new input field for each DX Coil *
*    - Add "avail_schedule" member variable for each module as needed *


### Modify Calculate Functions ###

*(1) Implement the availability schedule functionality in calc function for each DX coil: *
*(2) Functions impacted by the logics of availability schedule control:

```
    a. CalcVarSpeedCoilCooling()
    b. CalcVarSpeedCoilHeating()
    c. CalcVarSpeedHPWH()
    d. CalcHPCoolingSimple()
    e. CalcHPHeatingSimple()
    f. CalcWatertoAirHPCooling()
    g. CalcWatertoAirHPHeating()
	h. CalcHPWHDXCoil()  {if budget allows}
```


    * (2.1) Modify the calc code that truns the DX coils On or Off based on schedule value. See sample psuedo code below: *

	```
         if (state.dataVariableSpeedCoils->VarSpeedCoil(DXCoilNum).avail_schedule =< 0 ) {
        ... (2.1.1) skip all the DX coil calculation steps
	    ... (2.1.2) re-set the associated report variables
         }
    ```
