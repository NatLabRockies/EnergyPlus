Equation-Fit Based Gas Fired Absorption Heat Pump (GAHP) Module
=============================================================

**J. Yuan & M.J. Witte, GARD Analytics**

 - Original Date: April 22, 2022
 - Revised: April 29, 2022
 - Revised: May 20, 2022
 - Revised: May 27, 2022


## Justification for New Feature ##
The goal of the proposed new feature development is to add a new EnergyPlus model for a gas-fired (or fuel-fired) absorption heat pump which represents a potential new product line for space heating and domestic hot water (DHW) heating applications [1]. The equipment is intended to used in cold climates as a heat pump to provided space-heating to the space and hot water heating for domestic hot water. Currently, the heat pump is modeled by the feature requester using the "PlantComponent:UserDefined" object to connect the EMS programs into the plant simulation. This method is not standardized and lack scalability for different connections and equipment configurations. The proposed new feature will address the model needs for convenient and scalable modeling of such gas-fired absorption heat pump (GAHP) systems.

Current EnergyPlus has capabilities to model an absorption chillerheater and a heat pump separately. For example, an existing EnergyPlus object that can provide the basis for the absorption chiller is ChillerHeater:Absorption:DirectFired; and an existing EnergyPlus object that can provide the heat pump modeling capability is HeatPump:PlantLoop:EIR:Cooling/Heating.

ChillerHeater:Absorption:DirectFired: This is an DOE-2 type direct-fired absorption chiller models that allows heating and cooling connections (and operations) at the same time. Direct gas-fired absorption chiller-heater using performance curves. This component models fuel-fired heating and cooling with a possible air-cooled condenser option. However, the heating mode is intended to represent simply a boiler, but not a heat pump. The fundamental difference is that the general ChillerHeater:Absorption:DirectFired has an independent condenser units to reject heat; however, the gas-fired heat pump to be modeled in this study actually will have heat extracted from the condenser and feed to the hot-water or domestic hot water demand loop.

HeatPump:PlantLoop:EIR:Cooling/Heating: An EIR formulated air-to-water (or water-to-water) heat pump model. This component can model a general -purpose heat pump, either air-to-water or water-to-water. However, it does not have a fuel type option; and the heating mode currently does not allow defrost operations. Further, parameter input options are more general for a traditional electric heat pump configuration; but not specifically to a gas-fired absorption heat pump to be modeled in the current NFP. In order to model a gas-fired absorption heat pump, many input fields, including a set of different curves, need to be expanded or allow alternatives in order to fit GAHP in a the current general-purpose heat pump model. Further, the HeatPUmp:PlantLoop:EIR:Cooling/Heating modules can only deal with constant flows for the load modulation; while for the current development, more load modulation options are needed.

In addition, for both of the two existing models ChillerHeater:Absorption:DirectFired and HeatPump:PlantLoop:EIR:Cooling/Heating---since these two models are for a general purpose absorption chiller heater or for a general purpose heat pump model (for both heating and cooling), they can be cumbersome in specifying, sizing, operations, and controls for the GAHP equipment modeling which is mainly in heating mode only. Further, there are more features that is applicable to the gas-fired absorption heat pump but not available in the current ChillerHeater:Absorption:DirectFired and HeatPump:PlantLoop:EIR:Cooling/Heating models, such as defrost options, flow control method, and standing by electricity options.


## E-mail and Conference Call Conclusions ##
### E-mail Communications ###
From 2022-04-22 to 2022-04-29, a few email exchanges with the feature requester (Alex Fridlyand) regarding the first draft NFP and a few related questions. Some comments and additional feedbacks were offered by Alex on the equipment's operation modes, typical application scenarios, typical input parameter values, and output variables.

### Conference Call Communications ###
On 2022-05-04, the NFP was presented to the development during a technicality conference call session. A few comments were gathered regarding: 1) how to reuse current objects, inputs, and code structures when developing the new model (Edwin Lee at NREL); and 2) the application scenarios of the new GAHP (Tianzhen Hong at LBNL).

On 2022-05-11, a conference call meeting was made between the NFP authors and Edwin Lee, and Matt Mitchell at NREL for a further discussion about how to reuse current code structures. A few possible ways were discussed regarding minimize repeated code development. It was recommended to use the existing data/code structure similar to HeatPUmp:PlantLoop:EIR:Cooling/Heating objects by either direct modifying on the existing module or setting up an inheritance relationship; and the new IDD object proposed in the current NFP can still be the way to go regardless of the underlying data/code structure choice.

## Overview ##

The proposed new feature will implement a gas-fire absorption heat pump model that would use equation-fit model with manufacture provided parameters. Currently the requester (GTI) uses a customized plant system component and EMS programs to implement the gas-fired absorption heat pump. The implemented model [1] is as follows:

$$ GAHP Heating Capacity = Rated Heating Capacity \times CAPFT $$

$$ CAPFT = a1 \times Tamb + b1 \times Tamb^{2} +c1 \times Tret + d1 \times Tret^{2} + e1 \times Tret \times Tamb + f1 $$

$$ PLR = \frac{Load}{Capacity} (0.2 <= PLR <= 1) $$

$$ Fuel Usage = \frac{Load \times EIRFT \times EIRFPLR \times EIRDEFROST}{CRF} $$

$$ EIRFT = a2 \times Tamb + b2 \times Tamb^{2} +c2 \times Tret + d2 \times Tret^{2} + e2 \times Tret \times Tamb + f2 $$

$$ EIRFPLR = 0.0865 \times PLR^{2} - 0.006 \times PLR + 0.9814, for 0.25 <= PLR <=1 $$

$$ EIRDEFROST = -0.0011 \times Tamb^{2} - 0.006 \times Tamb + 1.0317, for -8.89^{\circ}C <= Tamb <= 3.33^{\circ}C $$

$$ CRF = 0.4167 \times CR + 0.5833 $$

$$ CR = \frac{PLR}{PLR_{min}}, for 0.2 <= PLR <= 0.25 $$


## Approach ##

The following new IDF object will be added to EnergyPlus to implement the absorption heat pump model, based on a new input data dictionary (IDD) definition has been proposed by GTI (e-mail from Alex Fridlyand on Nov 29, 2021), with a few addition of essential entries. The name of the object is HeatPump:AirToWater:FuelFired:Heating.

### IDF Object for HeatPump:AirToWater:FuelFired:Heating

```
HeatPump:AirToWater:FuelFired:Heating,
    AbsorptionHeatPumpName1, !- Name
    Inlet_Node_Name, !-Water Inlet Node Name
    Outlet_Node_Name, !-Water Outlet Node Name
    Air_Source_Node_Name, ! Air Source Node Name
    Companion_Cooling_Heat_Pump_Name, !-Companion Cooling Heat Pump Name
    NaturalGas, !-Fuel Type
    General, !-End-Use Subcategory
    10000, !-Nominal Heating Capacity
    0.005, !-Design Flow Rate
    60, !-Design Supply Temperature
    11.1, !-Design Temperature Lift
    1.1, ! Sizing Factor
    NotModulated, !-Flow Mode
    DryBulb, !-Outdoor Air Temperature Curve Input Variable
    EnteringCondenser, !-Water Temperature Curve Input Variable
    CAPFT_CurveName, !-Normalized Capacity Function of Temperature Curve Name
    Fuel_EIRFT_CurveName, !-Fuel Energy Input Ratio Function of Temperature Curve name
    Fuel_EIRFPLR_CurveName, !-Fuel Energy Input Ratio Function of PLR Curve Name
    0.1, !-Minimum Part Load Ratio
    1, !-Maximum Part Load Ratio
    Fuel_EIRDefrost_CurveName, !-Fuel Energy Input Ratio Defrost Adjustment Curve name
    OnDemand, !-Defrost Control Type
    , !-Defrost operation time fraction
    5, !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation
    CRF_CurveName, !-Cycling Ratio Factor Curve Name
    900, !-Nominal Auxiliary Electric Power
    Aux_Elec_EIRFT_CurveName, !-Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name
    Aux_EIRPLR_CurveName, !-Auxiliary Electric Energy Input Ratio Function of PLR Curve Name
    20; !-Standby Electric Power
```

### IDD entry for HeatPump:AirToWater:FuelFired:Heating ###

A HeatPump:AirToWater:FuelFired:Heating object defines the basic inputs for an equation-fit gas-fired (or generally fuel-fired) absorption heat pump.
```
HeatPump:AirToWater:FuelFired:Heating,
  \memo The object defines a fuel-fired absorption heat pump based on equation-fit models.
  \min-fields 10
  A1 , \field Name
       \required-field
       \reference HeatPumpAirToWaterFuelFiredHeatingNames
       \note Name of the fuel fired absorption heat pump system system
  A2 , \field Water Inlet Node Name
       \required-field
       \type node
       \note Inlet node name of the water side connection
  A3 , \field Water Outlet Node Name
       \required-field
       \type node
       \note Outlet node name of the water side connection
  A4 , \field Air Source Node Name
       \type object-list
       \object-list OutdoorAirNodeNames
       \note This is the air source node name, which is the evaporator side of the heat pump in heating mode.
       \note Enter the name of an OutdoorAir:Node object.
  A5 , \field Companion Cooling Heat Pump Name
       \note The name of the companion HeatPump:AirToWater:FuelFired:Cooling object
       \note This field is used for a heat pump with switchable heating and cooling mode.
       \type object-list
       \object-list HeatPumpAirToWaterFuelFiredCoolingNames
  A6 , \field Fuel Type
       \required-field
       \type choice
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Diesel
       \key Gasoline
       \key Coal
       \key OtherFuel1
       \key OtherFuel2
       \default NaturalGas
       \note Fuel Type (NaturalGas, Propane, Gasoline, Diesel etc.)
  A7 , \field End-Use Subcategory
       \type alpha
       \retaincase
       \default General
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
  N1 , \field Nominal Heating Capacity
       \autosizable
       \minimum> 0
       \units W
       \note Nominal Heating Capacity in [W] (autosizeable)
  N2 , \field Design Flow Rate
       \autosizable
       \minimum> 0
       \units m3/s
       \note Design Flow Rate in m3/s (autosizeable)
  N3 , \field Design Supply Temperature
       \default 60
       \units C
       \note Design Supply Temperature in [degree C]
  N4 , \field Design Temperature Lift
       \autosizable
       \default 11.1
       \units deltaC
       \note Design Temperature Lift in [degree C]
  N5 , \field Sizing Factor
       \minimum 1.0
       \default 1.0
       \note Sizing Factor for equipment sizing
  A8 , \field Flow Mode
       \required-field
       \type choice
       \key NotModulated
       \key ConstantFlow
       \key LeavingSetpointModulated
       \default NotModulated
       \note Flow Mode for the water side of the fuel-fired absorption heat pump
  A9 , \field Outdoor Air Temperature Curve Input Variable
       \required-field
       \type choice
       \key DryBulb
       \key WetBulb
       \default DryBulb
       \note Outdoor air temperature curve input variable;
       \note The options are Outdoor Air Dry Bulb or Wet Bulb temperature for curves
  A10, \field Water Temperature Curve Input Variable
       \required-field
       \type choice
       \key EnteringCondenser
       \key LeavingCondenser
       \default EnteringCondenser
       \note Water Temperature curve input variable - Condenser Entering or Leaving Water Temperature for curves
  A11, \field Normalized Capacity Function of Temperature Curve Name
       \required-field
       \type object-list
       \object-list BivariateFunctions
       \note: CAPFT - Normalized Capacity Function of Temperature Curve name,
       \note which is a biquadratic curve or a lookup table.
  A12, \field Fuel Energy Input Ratio Function of Temperature Curve name
       \required-field
       \type object-list
       \object-list BivariateFunctions
       \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve name,
       \note which is a biquadratic curve or a lookup table.
  A13, \field Fuel Energy Input Ratio Function of PLR Curve Name
       \required-field
       \type object-list
       \object-list UnivariateFunctions
       \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve name,
       \note which is a cubic curve or a lookup table.
  N6 , \field Minimum Part Load Ratio
       \minimum> 0.0
       \maximum 1.0
       \default 0.1
       \note Minimum Part Load Ratio (PLR) in between 0 and 1
  N7 , \field Maximum Part Load Ratio
       \minimum> 0.0
       \maximum 1.0
       \default 1.0
       \note Maximum Part Load Ratio (PLR) in between 0 and 1
  A14, \field Fuel Energy Input Ratio Defrost Adjustment Curve name
       \type object-list
       \object-list UnivariateFunctions
       \note EIRDEFROST - Energy Input Ratio Defrost Adjustment Curve name,
       \note which is a cubic curve or a lookup table function of Outdoor Air Temperature.
  A15, \field Defrost Control Type
       \type choice
       \key timed
       \key OnDemand
       \default timed
       \note Defrost operation control type: timed or OnDemand
  N8 , \field Defrost Operation Time Fraction
       \minimum 0
       \maximum 1
       \default 0
       \note Defrost operation time fraction, which will be used for timed defrost control type.
  N9 , \field Maximum Outdoor Dry-bulb Temperature for Defrost Operation
       \units C
       \minimum 0
       \maximum 10
       \default 5
       \note There will be no defrost operation when outdoor air is above this temperature.
  A16, \field Cycling Ratio Factor Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Cycling Ratio Factor (CRF) Curve Name,
       \note which is a cubic curve or a lookup table function of Cycling Ratio (defined as = PLR/PLRmin).
  N10, \field Nominal Auxiliary Electric Power
       \units W
       \minimum 0
       \note Nominal Auxiliary Electric Power in [W]
  A17, \field Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name
       \type object-list
       \object-list BivariateFunctions
       \note Auxiliary Electric EIRFT - Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name,
       \note which is a biquadratic curve or a lookup table.
       \note which accounts for system internal fans, pumps, and electronics
  A18, \field Auxiliary Electric Energy Input Ratio Function of PLR Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Auxiliary Electric EIRFPLR - Auxiliary Electric Energy Input Ratio Function of PLR (Part Load Ratio) Curve Name,
       \note which is a cubic curve or a lookup table.
  N11; \field Standby Electric Power
       \units W
       \minimum 0
       \note Standby Electric Power in [W]
```

### IDD entry for HeatPump:AirToWater:FuelFired:Cooling ###

A companion gas fired heat pump entry named HeatPump:AirToWater:FuelFired:Cooling with similar sets of input fields will also be added to the IDD file. The companion coil will be dealing with the input for the cooling mode operations and equipment sizing. For the input fields for the companion HeatPump:AirToWater:FuelFired:Cooling are similar to those for HeatPump:AirToWater:FuelFired:Heating, with the biggest difference being that the defrost options are no longer needed.

```
HeatPump:AirToWater:FuelFired:Cooling,
  \memo The object defines a fuel-fired absorption heat pump based on equation-fit models.
  \min-fields 10
  A1 , \field Name
       \required-field
       \reference HeatPumpAirToWaterFuelFiredCoolingNames
       \note Name of the fuel fired absorption heat pump system system
  A2 , \field Water Inlet Node Name
       \required-field
       \type node
       \note Inlet node name of the water side connection
  A3 , \field Water Outlet Node Name
       \required-field
       \type node
       \note Outlet node name of the water side connection
  A4 , \field Air Source Node Name
       \type object-list
       \object-list OutdoorAirNodeNames
       \note This is the air source node name, which is the condenser side of the heat pump in cooling mode.
       \note Enter the name of an OutdoorAir:Node object.
  A5 , \field Companion Heating Heat Pump Name
       \note The name of the companion HeatPump:AirToWater:FuelFired:Heating object
       \note This field is used for a heat pump with switchable heating and cooling mode.
       \type object-list
       \object-list HeatPumpAirToWaterFuelFiredHeatingNames
  A6 , \field Fuel Type
       \required-field
       \type choice
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Diesel
       \key Gasoline
       \key Coal
       \key OtherFuel1
       \key OtherFuel2
       \default NaturalGas
       \note Fuel Type (NaturalGas, Propane, Gasoline, Diesel etc.)
  A7 , \field End-Use Subcategory
       \type alpha
       \retaincase
       \default General
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
  N1 , \field Nominal Cooling Capacity
       \autosizable
       \minimum> 0
       \units W
       \note Nominal Cooling Capacity in [W] (autosizeable)
  N2 , \field Design Flow Rate
       \autosizable
       \minimum> 0
       \units m3/s
       \note Design Flow Rate in m3/s (autosizeable)
  N3 , \field Design Supply Temperature
       \default 60
       \units C
       \note Design Supply Temperature in [degree C]
  N4 , \field Design Temperature Lift
       \autosizable
       \default 11.1
       \units deltaC
       \note Design Temperature Lift in [degree C]
  N5 , \field Sizing Factor
       \minimum 1.0
       \default 1.0
       \note Sizing Factor for equipment sizing
  A8 , \field Flow Mode
       \required-field
       \type choice
       \key NotModulated
       \key ConstantFlow
       \key LeavingSetpointModulated
       \default NotModulated
       \note Flow Mode for the water side of the fuel-fired absorption heat pump
  A9 , \field Outdoor Air Temperature Curve Input Variable
       \required-field
       \type choice
       \key DryBulb
       \key WetBulb
       \default DryBulb
       \note Outdoor air temperature curve input variable;
       \note The options are Outdoor Air Dry Bulb or Wet Bulb temperature for curves
  A10, \field Water Temperature Curve Input Variable
       \required-field
       \type choice
       \key EnteringCondenser
       \key LeavingCondenser
       \default EnteringCondenser
       \note Water Temperature curve input variable - Condenser Entering or Leaving Water Temperature for curves
  A11, \field Normalized Capacity Function of Temperature Curve Name
       \required-field
       \type object-list
       \object-list BivariateFunctions
       \note: CAPFT - Normalized Capacity Function of Temperature Curve name,
       \note which is a biquadratic curve or a lookup table.
  A12, \field Fuel Energy Input Ratio Function of Temperature Curve name
       \required-field
       \type object-list
       \object-list BivariateFunctions
       \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve name,
       \note which is a biquadratic curve or a lookup table.
  A13, \field Fuel Energy Input Ratio Function of PLR Curve Name
       \required-field
       \type object-list
       \object-list UnivariateFunctions
       \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve name,
       \note which is a cubic curve or a lookup table.
  N6 , \field Minimum Part Load Ratio
       \minimum> 0.0
       \maximum 1.0
       \default 0.1
       \note Minimum Part Load Ratio (PLR) in between 0 and 1
  N7 , \field Maximum Part Load Ratio
       \minimum> 0.0
       \maximum 1.0
       \default 1.0
       \note Maximum Part Load Ratio (PLR) in between 0 and 1
  A14, \field Cycling Ratio Factor Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Cycling Ratio Factor (CRF) Curve Name,
       \note which is a cubic curve or a lookup table function of Cycling Ratio (defined as = PLR/PLRmin).
  N8 , \field Nominal Auxiliary Electric Power
       \units W
       \minimum 0
       \note Nominal Auxiliary Electric Power in [W]
  A15, \field Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name
       \type object-list
       \object-list BivariateFunctions
       \note Auxiliary Electric EIRFT - Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name,
       \note which is a biquadratic curve or a lookup table.
       \note which accounts for system internal fans, pumps, and electronics
  A16, \field Auxiliary Electric Energy Input Ratio Function of PLR Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Auxiliary Electric EIRFPLR - Auxiliary Electric Energy Input Ratio Function of PLR (Part Load Ratio) Curve Name,
       \note which is a cubic curve or a lookup table.
  N9 ; \field Standby Electric Power
       \units W
       \minimum 0
       \note Standby Electric Power in [W]
```

### Output Variables ###

Output variables will reported for operation conditions, such as heating energy (rate), fuel energy (rate), electricity energy (rate)load, PLR, flow rate values.

The output variables are listed as follows:

```
HVAC,average,Fuel-fired Absorption HeatPump Load Side Heat Transfer Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Load Side Heat Transfer Energy [J]
HVAC,average,Fuel-fired Absorption HeatPump Fuel Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Fuel Energy [J]
HVAC,average,Fuel-fired Absorption HeatPump Electricity Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Electricity Energy [J]
HVAC,average,Fuel-fired Absorption HeatPump Mass Flow Rate [m3/s];
HVAC,average,Fuel-fired Absorption HeatPump Volumetric Flow Rate [m3/s];
HVAC,average,Fuel-fired Absorption HeatPump Inlet Temperature [C]
HVAC,average,Fuel-fired Absorption HeatPump Outlet Temperature [C]
```

### Sizing ###

The sizing for a couple of design parameters, such as nominal heating capacity, and the design flow rate.

## Testing and Validation ##

One or two unit tests will be developed to verify that:
1. the new input objects can be processed correctly;
2. the model simulation system results, output variables, and reports are working properly.

## Example File and Transition Changes ##

One new example file will be added to the test suite to demonstrate how to use this feature.

Since the feature is based on completely newly added blocks, an older version would not carry the feature. Therefore a transition program is not needed for converting from earlier versions.

## Input Output Reference Documentation ##

The proposed new feature development will add the following contents to the Input Output Reference document:

### HeatPump:AirToWater:FuelFired:Heating Input Fields ###

HeatPump:AirToWater:FuelFired:Heating is a gas-fired (or fuel fired) absorption heat pump that can be used for space heating and domestic hot water. It is can be considered a more efficient heat source (like a boiler) for hot water loop or DHW loop. It can be used as the heat source for a low temperature radiant heating system.

The HeatPump:AirToWater:FuelFired:Heating will take the following input fields:

#### Field: Name ####
The name of the gas-fired (fuel-fired) absorption heat pump system.

#### Fiedl: ####

#### Field: Water Inlet Node Name ####

Note that here the "water" does not literally mean "water". Rather it is the working fluid in the connected water side plant loop, which is usually some kind of glycol-water solution.

#### Field: Fuel Type ####
This alpha field determines the type of fuel that the chiller uses. The default is NaturalGas. Valid values are NaturalGas, Propane, Diesel, Gasoline, FuelOilNo1, FuelOilNo2, OtherFuel1, OtherFuel2.

#### Field: Sizing Factor ####
This optional numeric field allows the user to specify a sizing factor for this component. The sizing factor is used when the component design inputs are autosized: the autosizing calculations are performed as usual and the results are multiplied by the sizing factor. For this component the inputs that would be altered by the sizing factor are: Nominal Capacity and Design Flow Rate. Sizing factor allows the user to size a component to meet part of the design load while continuing to use the autosizing feature.

#### Field: Companion Cooling Heat Pump Name ####
This optional name field allows the user to specify the companion cooling water to water heat pump for this heating component. This should be the name of an HeatPump:AirToWater:FuelFired:Cooling object defined elsewhere in the input file. Because the cooling and heating models are often intended to represent separate operating modes of one single reversible heat pump, it is useful to identify the corresponding cooling heat pump input object by name. This is currently used with autosizing so that the sizing calculations can be coordinated so that the two companion heat pumps get the same design reference flow rates.

#### Field: Flow Mode ####
This choice field determines how the gas-fired absorption heat pump operates with respect to the intended fluid flow through the device. There are three different choices for specifying operating modes for the intended flow behavior: "NotModulated," "ConstantFlow," and "LeavingSetpointModulated." NotModulated is useful for either variable or constant speed pumping arrangements where the boiler is passive in the sense that although it makes a nominal request for its design flow rate it can operate at varying flow rates. ConstantFlow is useful for constant speed pumping arrangements where the heat pump's request for flow is more strict and can increase the overall loop flow. LeavingSetpointModulated changes the heat pump model to internally vary the flow rate so that the temperature leaving the heat pump matches a setpoint. In all cases the operation of the external plant system can also impact the flow through the heat pump---for example if the relative sizes and operation are such that flow is restricted and the requests cannot be met. The default, if not specified, is NotModulated.

#### Field: Water Temperature Curve Input Variable ####
This field sets the second independent variable in the three temperature dependent performance curves to either the leaving or entering condenser water temperature. Manufacturers express the performance of their heat pumps using either the leaving condenser water temperature or the entering condenser water temperature. Valid choices for this field are: LeavingCondenser or EnteringCondenser. It is important that the performance curves and this field are consistent with each other. The default is EnteringCondenser.

#### Field: Maximum Part Load Ratio ####
A positive fraction that represents the maximum heating output possible when operated continually at rated temperature conditions divided by the available heating capacity at those same conditions. If greater than 1.0, the equipment is typically thought of as capable of being overloaded. The default is 1.0.

#### Field: ####

An example of the HeatPump:AirToWater:FuelFired:Heating input object is like this:

```
HeatPump:AirToWater:FuelFired:Heating,
    AbsorptionHeatPumpName1, !- Name
    Inlet_Node_Name, !-Water Inlet Node Name
    Outlet_Node_Name, !-Water Outlet Node Name
    Air_Source_Node_Name, ! Air Source Node Name
    Companion_Cooling_Heat_Pump_Name, !-Companion Cooling Heat Pump Name
    NaturalGas, !-Fuel Type
    General, !-End-Use Subcategory
    10000, !-Nominal Heating Capacity
    0.005, !-Design Flow Rate
    60, !-Design Supply Temperature
    11.1, !-Design Temperature Lift
    1.1, ! Sizing Factor
    NotModulated, !-Flow Mode
    DryBulb, !-Outdoor Air Temperature Curve Input Variable
    EnteringCondenser, !-Water Temperature Curve Input Variable
    CAPFT_CurveName, !-Normalized Capacity Function of Temperature Curve Name
    Fuel_EIRFT_CurveName, !-Fuel Energy Input Ratio Function of Temperature Curve name
    Fuel_EIRFPLR_CurveName, !-Fuel Energy Input Ratio Function of PLR Curve Name
    0.1, !-Minimum Part Load Ratio
    1, !-Maximum Part Load Ratio
    Fuel_EIRDefrost_CurveName, !-Fuel Energy Input Ratio Defrost Adjustment Curve name
    OnDemand, !-Defrost Control Type
    , !-Defrost operation time fraction
    5, !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation
    CRF_CurveName, !-Cycling Ratio Factor Curve Name
    900, !-Nominal Auxiliary Electric Power
    Aux_Elec_EIRFT_CurveName, !-Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name
    Aux_EIRPLR_CurveName, !-Auxiliary Electric Energy Input Ratio Function of PLR Curve Name
    20; !-Standby Electric Power
```

### HeatPump:AirToWater:FuelFired:Cooling Input Fields ###


## Input Description ##

See the Input Output Reference documentation contents update above.


## Outputs Description ##

The following output will be added the to the new gas-fired (fuel-fired) absorption heat pump system:

```
HVAC,average,Fuel-fired Absorption HeatPump Load Side Heating Transfer Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Load Side Heating Transfer Energy [W]
HVAC,average,Fuel-fired Absorption HeatPump Fuel Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Fuel Energy [J]
HVAC,average,Fuel-fired Absorption HeatPump Electricity Rate [W]
HVAC,sum,Fuel-fired Absorption HeatPump Electricity Energy [J]
HVAC,average,Fuel-fired Absorption HeatPump Mass Flow Rate [m3/s];
HVAC,average,Fuel-fired Absorption HeatPump Volumetric Flow Rate [m3/s];
HVAC,average,Fuel-fired Absorption HeatPump Inlet Temperature [C]
HVAC,average,Fuel-fired Absorption HeatPump Outlet Temperature [C]
```

## Engineering Reference ##

The fundamental methods and equations used for the model, as well as the original references will be added to the the Engineering Reference.

## Designs ##

### New data struct ###
A new data struct `EIRFuelFiredHeatPump` will be created, which is derived (and inherited) from the existing data struct named `EIRPlantLoopHeatPump`. This will allow the code to use a similar calling structure to simulate the equipment and seamlessly plug into the upper level plant loop simulations.

Two new plant loop equipment type named `HeatPumpFuelFiredHeating` and `HeatPumpFuelFiredCooling` will also be created as the new plant loop category names for the HeatPump:AirToWater:FuelFired:Heating/Cooling objects.

Following new member variables to the child `EIRFuelFiredHeatPump` struct to accommodate the additional input field, output variables, and intermediate variables need for the newly added HeatPump:AirToWater:FuelFired:Heating/Cooling object:
```
        // New additions for GAHP only
        std::string fuelTypeStr = "";
        DataGlobalConstants::ResourceType fuelType = DataGlobalConstants::ResourceType::None; // resource type assignment
        std::string endUseSubcat = "";                                                        // identifier use for the end use subcategory
        DataPlant::FlowMode flowMode = DataPlant::FlowMode::Invalid;
        Real64 desSupplyTemp = 60.0;
        Real64 desTempLift = 11.1;
        int oaTempCurveInputVar = 0;
        int waterTempCurveInputVar = 0;
        Real64 minPLR = 0.1;
        Real64 maxPLR = 1.0;

        int defrostEIRCurveIndex = 0;
        int defrostType = 0;
        Real64 defrostOpTimeFrac = 0.0;
        Real64 defrostMaxOADBT = 5.0;

        int cycRatioCurveIndex = 0;
        Real64 nominalAuxElecPower = 0.0;
        int auxElecEIRFoTempCurveIndex = 0;
        int auxElecEIRFoPLRCurveIndex = 0;
        Real64 standbyElecPower = 0.0;

        // new output variables for derived class only
        std::string GAHPFuelTypeForOutputVariable = "";
        Real64 loadSideVolumeFlowRate = 0.0;
        Real64 fuelUsage = 0.0;
        Real64 fuelEnergy = 0.0;
```

A few existing methods (functions) in the parent needs to be overridden for the child struct, such as:
```
        // Override parent methods to be declared
        void doPhysics(EnergyPlusData &state, Real64 currentLoad);
        void resetReportingVariables();
        static PlantComponent *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType hp_type_of_num, const std::string &hp_name);
        static void pairUpCompanionCoils(EnergyPlusData &state);
        static void processInputForEIRPLHP(EnergyPlusData &state);
        void oneTimeInit(EnergyPlusData &state);
```

### Overriden functions ###
The overridden functions will have their own versions for the `EIRFuelFiredHeatPump` object. But still they still could fit into the calling structure of the parent `EIRPlantLoopHeatPump` object. A few major functions that need to be overridden are introduced here:

#### processInputForEIRPLHP() ####

This function will be revised in the new child struct to process the input fields for the HeatPump:AirToWater:FuelFired:Heating/Cooling objects. It will read input information for the system's connection, equipment sizing, and operation mode.

#### sizeLoadSide() ####

This function will be revised in the new child struct to size the GAHP equipment, such as the nominal heat capacity and design flow rate.

#### pairUpCompanionCoils() ####
This function will be revised in the new child struct to accommodate the cooling/heating pairing relationship the GAHP.

#### doPhysics() ####
This function will be revised in the new child struct to model the GAHP. Compared to the parent version, the energy flows associated with fuel other than electricity will also be calculated, as well as the additional operation features needed to the fuel-fired heat pump.

#### oneTimeInit() ####
This function will be revised in the new child struct to accommodate the output variables for the GAHP. Compared to the parent version, the energy flows associated with fuel will also be processed and reported here.

#### resetReportingVariables() ####
This function will be revised in the new child struct to reset reporting variables, especially the energy values for a new timestep. For the GAHP, fuel energy related variables needs to be added to this overridden function.

#### Others function(s) ####
Any other functions that inovles a special treatment for the new fuel-fired heat pump would need to be overridden from the parent heat pump struct.
## Reference ##

[1] Fridlyand, Alex; Glanville, Paul; and Garrabrant, Michael, 2021. "Pathways to Decarbonization of Residential Heating," International High Performance Buildings Conference. Paper 354. https://docs.lib.purdue.edu/ihpbc/354).
