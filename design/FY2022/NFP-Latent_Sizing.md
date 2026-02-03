Autosizing for High Latent Loads
================
## Initial New Feature Proposal and Design Documentation ##

**Richard Raustad, FSEC Energy Research Center**

 - Initial NFP Original Date: 4/29/2022
 - Final NFP Revision Date: 7/7/22


## Justification for New Feature ##

**Feature request description:** When the internal latent load is high or dominant, E+ fails to autosize HVAC. An example is indoor growing facilities. Enhancing autosize to accommodate this situation will be a huge assist for modelers working on those facility types.

**Additional information:** Trane has an [Engineers Newsletter, volume 48–3](https://www.trane.com/content/dam/Trane/Commercial/global/products-systems/education-training/engineers-newsletters/airside-design/admapn071en-082019.pdf) describing sizing theory used for growing facilities which basically describes the zone sensible and latent loads in grow facilities and the associated psychrometric charts associated with cooling coil and reheat coil capacity requirements.

## E-mail and  Conference Call Conclusions ##

Mike Witte - 5/4/22 - Depending on the application, the user might want to size a given system on latent-only, sensible-only, or the max of both requirements. Perhaps the sizing options should include a Maximum option?

Resolution: added Latent Load sizing option.

## Summary ##

EnergyPlus currently calculates a zone design sensible supply air mass flow rate based on a predicted zone sensible load and user provided supply air conditions. The following describes a mirrored approach to calculate and report the zone design latent load and latent air mass flow rate. The underlying principal is that the existing calculation of zone sensible load and corresponding **sensible** air mass flow rate provides a zone supply air mass flow rate that meets the peak cooling or heating sensible load. This calculation is based on a user provided zone temperature set point and a design supply air temperature (or delta T). If the calculated peak air mass flow rate is provided to a zone at the user supplied supply air temperature, the peak zone sensible load will be met. A similar approach is proposed for latent sizing and described in this new feature proposal.

## Overview ##

**Current Methodology:**

During zone sizing calculations, E+ currently predicts the zone sensible load based on the active thermostat set point temperature. Latent load is also predicted during zone sizing but currently unused. During zone sizing, the zone set point temperature is maintained at set point to facilitate the calculation of only the zone sensible loads in the zone heat balance (i.e., no inclusion of loads due to zone thermal capacitance or storage other than zone thermostat setup and setback). The impact of a DOAS system can be included in zone sizing calculations, if desired, to provide a more accurate **remaining** zone sensible load. For each design day and each time interval, the zone sensible air mass flow rate is calculated based on the predicted zone sensible load and user specified supply air conditions. The peak zone sensible load, derived from time series data collected during zone sizing, is used to select the zone sensible design air mass flow rate. The results of zone sensible sizing are reported to the zone sizing report (`epluszsz.csv`) as the zone sensible cooling and heating mass flow rates and zone sensible cooling and heating loads (4 reports per zone). The peak zone sensible design air flow rate is subsequently used for zone equipment and cumulatively for air system component sizing.

**Proposed Methodology:**

A similar approach to the existing zone sensible air flow calculation will be used to determine zone latent air flow rate. The zone condition will be maintained at an assumed or humidistat set point to facilitate an accurate prediction of zone latent loads. It is proposed that zone latent sizing occur based on a user input/request, whether or not a humidistat is present, however, if a humidistat is present the zone humidistat will be used to provide the control point. The results of zone latent sizing will be included in the zone sizing report as the zone latent cooling and heating mass flow rates and zone latent cooling and heating loads. For comparison purposes, four additional reports will be added to document the original zone sensible and latent loads prior to application of a DOAS system as Des Heat Load No DOAS, Des Cool Load No DOAS, Des Latent Heat Load No DOAS and Des Latent Cool Load No DOAS (12 reports per zone).

The proposed methodology allows a user to determine both sensible and latent loads via a loads only type simulation ([PR #9402](https://github.com/NatLabRockies/EnergyPlus/pull/9402)). Uncontrolled zones will not have a sensible or latent load and are identified by whether or not a `ZoneHVAC:EquipmentConnections` object is associated with a zone (which identifies the zone supply air node name).

## Approach ##

**Proposed update to Function SizeZoneEquipment**

E+ currently calculates the sensible air mass flow rate required to meet the zone sensible load. The sensible air mass flow rate and user supplied supply air temperature and humidity ratio are passed to the zone as the supply air conditions to be used in the correct step of ZoneTempPredictorCorrector (`CorrectZoneAirTemp` and `CorrectZoneHumRat`). The sensible and latent loads are also passed to `UpdateSystemOutputRequired`, which will inherently return 0 remaining sensible and latent loads (predicted loads are passed as xxxOutputProvided).

    UpdateSystemOutputRequired(state, ActualZoneNum, SysOutputProvided, LatOutputProvided);

This new feature will use similar calculations where a zone latent load is used to calculate a zone heating and cooling latent air mass flow rate based on a user specified zone supply air humidity ratio (or deltaW). One caveat here is that since the latent mass flow rate will usually be different from the sensible mass flow rate, an adjustment to the supply air humidity ratio will be calculated such that both the sensible and latent loads are met each time interval to maintain the zone at the thermostat and humidistat set points. Initial testing has determined that holding the zone at a predetermined humidity level does not significantly impact the zone sensible loads or prediction of zone sensible air mass flow rate.

**New Feature Choice Options**

The `Sizing:System` object has a choice for `Type of Load to Size On`. Valid choices for this field are Sensible, Total and VentilationRequirement. A new key choice will be added for Latent. See Input Description section below.

**Turning Latent Sizing On and Off**

**Zone**

The proposed solution for disabling zone latent sizing is to add new input fields to `Sizing:Zone` that requests the type of load sizing to perform with a default that provides the same answer currently provided. The choice field to disable zone latent sizing is tentatively `Sensible Load Only No Latent Load`. Addtional inputs will provide an assumed humidity set point schedule and supply air moisture as either a humidity ratio or a delta humidity ratio.

**System**

There may be zones connected to an air system that do not have a humidistat or there may be no zones with a humidistat. One or more zones on an air loop may not have significant latent loads, and therefore no humidistat, or a user may not have included humidistats and set the `Type of Load to Size On = Latent` just as a verification of latent loads. For this reason, if a system serves a zone without a humidistat, that zone's humidistat setting will be assumed as 50% RH or other user specified schedule value (see Input Description section below), where the corresponding zone supply air humidity ratio with respect to zone humidity ratio will be used to calculate latent mass flow rate (see Model Design Document section below). Using any choice other than Latent will effectively turn off air system latent sizing.

## Input Description ##

A new choice field will be added to Sizing:Zone to allow control of sizing calculations. Nine additional new fields will be added to mirror the supply air temperature input fields and to specify the assumed zone humidity control point if no humidistat is present. The control point input would be overridden by a zone humidistat if present (i.e., humidistat has priority). These fields could be added just after the zone supply air condition inputs or at end of object to avoid transition. Since changing the zone conditions during zone sizing (i.e., holding a zone humidity level) will change the zone sensible load and resulting zone sensible air flow calculation, even if only slightly, it is proposed that a mechanism be used to turn this feature off completely. When latent loads are not used during sizing, 0's will be reported to the zone sizing results file `epluszsz.csv`. The `Sensible Load Only No Latent Load` field could potentially be removed after example file diffs are scrutinized and changes to existing simulations are deemed small enough to ignore. The following are new fields.

~~~
Sizing:Zone,

  A10,\field Zone Load Sizing Method
      \note Specifies the basis for sizing the zone supply air flow rate.
      \note Zone latent loads will not be used during sizing only when
      \note Zone Load Sizing Method = Sensible Load Only No Latent Load.
      \note For this case the zone humidity level will float according to
      \note the fields Cooling and Heating Design Supply Air Humidity Ratio.
      \note For all other choices the zone humidity level will be controlled.
      \note Sensible Load will use zone sensible air flow rate for zone
      \note component sizing. Latent loads will also be reported during sizing.
      \note Latent Load will use zone latent air flow rate for zone
      \note component sizing. Sensible loads will also be reported during sizing.
      \note Sensible and Latent Load will use the larger of sensible and
      \note latent air flow rate for zone component sizing.
      \note Sensible Load Only No Latent Load or leaving this field blank
      \note will disable zone latent sizing and reporting. Latent loads will
      \note not be reported during sizing (reported as 0's).
      \type choice
      \key Sensible Load
      \key Latent Load
      \key Sensible And Latent Load
      \key Sensible Load Only No Latent Load
      \default Sensible Load Only No Latent Load
  A11,\field Zone Latent Cooling Design Supply Air Humidity Ratio Input Method
      \note Use SupplyAirHumidityRatio to enter the humidity ratio when zone dehumidification
      \note is required. The supply air humidity ratio should be less than the zone humidity
      \note ratio at the zone thermostat and humidistat set point condition.
      \note Use HumidityRatioDifference to enter the difference in humidity ratio from the
      \note zone thermostat and humidistat set point condition.
      \type choice
      \key SupplyAirHumidityRatio
      \key HumidityRatioDifference
      \default SupplyAirHumidityRatio
  N19,\field Zone Dehumidification Design Supply Air Humidity Ratio
      \note Zone Dehumidification Design Supply Air Humidity Ratio is only used when Zone Latent
      \note Cooling Design Supply Air Humidity Ratio Input Method = SupplyAirHumidityRatio.
      \note This input must be less than the zone humidity ratio at the
      \note humidistat set point so that dehumidification can occur.
      \minimum> 0.0
      \type real
      \units kgWater/kgDryAir
  N20,\field Zone Cooling Design Supply Air Humidity Ratio Difference
      \note Zone Dehumidification Design Supply Air Humidity Ratio is only used when Zone Latent
      \note Cooling Design Supply Air Humidity Ratio Input Method = HumidityRatioDifference.
      \note This input is a positive value and defines the difference between the zone humidity
      \note ratio at the thermostat and humidistat set point condition and the supply air
      \note humidity ratio entering the zone.
      \minimum> 0.0
      \type real
      \default 0.005
      \units kgWater/kgDryAir
  A12,\field Zone Latent Heating Design Supply Air Humidity Ratio Input Method
      \note Use SupplyAirHumidityRatio to enter the humidity ratio when zone humidification
      \note is required. The supply air humidity ratio should be greater than the zone humidity
      \note ratio at the zone thermostat and humidistat set point condition.
      \note Use HumidityRatioDifference to enter the difference in humidity ratio from the
      \note zone thermostat and humidistat set point condition.
      \type choice
      \key SupplyAirHumidityRatio
      \key HumidityRatioDifference
      \default SupplyAirHumidityRatio
  N21,\field Zone Humidification Design Supply Air Humidity Ratio
      \note Zone Humidification Design Supply Air Humidity Ratio is only used when Zone Latent
      \note Heating Design Supply Air Humidity Ratio Input Method = SupplyAirHumidityRatio.
      \note This input must be greater than the zone humidity ratio at the
      \note humidistat set point so that humidification can occur.
      \minimum> 0.0
      \type real
      \units kgWater/kgDryAir
  N22,\field Zone Humidification Design Supply Air Humidity Ratio Difference
      \note Zone Humidification Design Supply Air Humidity Ratio is only used when Zone Latent
      \note Heating Design Supply Air Humidity Ratio Input Method = HumidityRatioDifference.
      \note This input is a positive value and defines the difference between the zone humidity
      \note ratio at the thermostat and humidistat set point condition and the supply air
      \note humidity ratio entering the zone.
      \minimum 0.0
      \type real
      \default 0.005
      \units kgWater/kgDryAir
  A13,\field Zone Humidistat Dehumidification Set Point Schedule Name
      \note Enter the zone relative humidity schedule used for zone latent
      \note cooling calculations.
      \note A zone humidistat will take priority over this input.
      \note This field is not used if Zone Load Sizing Method = Sensible Load
      \note Only No Latent Load or a zone humidistat is present.
      \note A default of 50.0 will be used if no schedule is provided and
      \note no humidistat is associated with this zone.
      \type alpha
      \units percent
  A14;\field Zone Humidistat Humidification Set Point Schedule Name
      \note Enter the zone relative humidity schedule used for zone latent
      \note heating calculations.
      \note A zone humidistat will take priority over this input.
      \note This field is not used if Zone Load Sizing Method = Sensible Load
      \note Only No Latent Load or a zone humidistat is present.
      \note A default of 50.0 will be used if no schedule is provided and
      \note no humidistat is associated with this zone.
      \type alpha
      \units percent
~~~

A single key choice will be added to the Sizing:System `Type of Load to Size On` field.

~~~
Sizing:System,

  A2,  \field Type of Load to Size On
       \note Specifies the basis for sizing the system supply air flow rate.
       \note Sensible and Total use the zone sensible design air flow rates to
       \note size the system supply air flow rate. The cooling coil will then be
       \note sized at either the peak Sensible or Total flow rate and conditions.
       \note The heating coil is always sized at the peak sensible heating load.
       \note Latent uses the larger of the zone sensible and latent design air
       \note flow rates for system sizing.
       \note VentilationRequirement uses the system ventilation requirement
       \type choice
       \key Sensible
       \key Latent (new key choice)
       \key Total
       \key VentilationRequirement
       \default Sensible
~~~

## Transition Requirements ##

If new Sizing:Zone input fields are inserted within this object a transition is required to insert 7 blank fields. If these fields are added to the end of the object, no transition is required. The current choice to to not require transition.

## Outputs Description ##

Existing reports for zone air system sensible capacity and zone sensible and latent predicted load are:

    Zone Air System Sensible Heating Energy [J]
    Zone Air System Sensible Cooling Energy [J]
    Zone Air System Sensible Heating Rate [W]
    Zone Air System Sensible Cooling Rate [W]
    Zone Predicted Sensible Load to Setpoint Heat Transfer Rate [W]
    Zone Predicted Sensible Load to Heating Setpoint Heat Transfer Rate [W]
    Zone Predicted Sensible Load to Cooling Setpoint Heat Transfer Rate [W]
    Zone System Predicted Sensible Load to Setpoint Heat Transfer Rate [W]
    Zone System Predicted Sensible Load to Heating Setpoint Heat Transfer Rate [W]
    Zone System Predicted Sensible Load to Cooling Setpoint Heat Transfer Rate [W]
    Zone Predicted Moisture Load Moisture Transfer Rate [kgWater/s]
    Zone Predicted Moisture Load to Humidifying Setpoint Moisture Transfer Rate [kgWater/s]
    Zone Predicted Moisture Load to Dehumidifying Setpoint Moisture Transfer Rate [kgWater/s]
    Zone System Predicted Moisture Load Moisture Transfer Rate [kgWater/s]
    Zone System Predicted Moisture Load to Humidifying Setpoint Moisture Transfer Rate [kgWater/s]
    Zone System Predicted Moisture Load to Dehumidifying Setpoint Moisture Transfer Rate [kgWater/s]

Proposed new latent indicating output:

    Zone Air System Latent Heating Energy [J]
      This output variable represents the latent heating energy in Joules that is
      actually supplied by the system to that zone for the timestep reported.
      This is the sensible heating rate multiplied by the simulation timestep.
      This is calculated and reported from the Correct step in the Zone
      Predictor-Corrector module. This field is not multiplied by zone or group
      multipliers.

    Zone Air System Latent Cooling Energy [J]
      This output variable represents the latent cooling energy in Joules that is
      actually supplied by the system to that zone for the timestep reported.
      This is the sensible cooling rate multiplied by the simulation timestep.
      This is calculated and reported from the Correct step in the Zone
      Predictor-Corrector module. This field is not multiplied by zone or group
      multipliers.

    Zone Air System Latent Heatng Rate [W]
      This output variable represents the latent heating rate in Watts that is
      actually supplied by the system to that zone for the timestep reported.
      This is calculated and reported from the Correct step in the Zone
      Predictor-Corrector module. This field is not multiplied by zone or
      group multipliers.

    Zone Air System Latent Cooling Rate [W]
      This output variable represents the latent cooling rate in Watts that is
      actually supplied by the system to that zone for the timestep reported.
      This is calculated and reported from the Correct step in the Zone
      Predictor-Correctormodule. This field is not multiplied by zone or group
      multipliers.

    Zone Air System Sensible Heat Ratio []
      This is the ratio of zone air system sensible heating or cooling energy or
      rate to the total zone air system heating or cooling energy or rate
      (sensible + Latent). This is calculated and reported from the Correct step
      in the Zone Predictor-Corrector module.

    Zone Air Vapor Pressure Difference [Pa]
      The output variable represents the zone vapor pressure depression or
      difference (VPD) in Pacals of the zone conditions with respect to the
      saturated vapor pressure at the zone air temperature.

The addition of latent sizing calculations opens up other places where an output related to sensible calculations could include the latent equivalent. These additional outputs will not be proposed for this latent sizing effort. For example:

    Zone Heating Setpoint Not Met Time [hr]
    Zone Heating Setpoint Not Met While Occupied Time [hr]
    Zone Cooling Setpoint Not Met Time [hr]
    Zone Cooling Setpoint Not Met While Occupied Time [hr]
    Facility Heating Setpoint Not Met Time [hr]
    Facility Cooling Setpoint Not Met Time [hr]
    Facility Heating Setpoint Not Met While Occupied Time [hr]
    Facility Cooling Setpoint Not Met While Occupied Time [hr]

    Zone Air Terminal Sensible Heating Energy [J]
    Zone Air Terminal Sensible Cooling Energy [J]
    Zone Air Terminal Sensible Heating Rate [W]
    Zone Air Terminal Sensible Cooling Rate [W]

    Zone Thermostat Air Temperature [C]
    Zone Thermostat Heating Setpoint Temperature [C]
    Zone Thermostat Cooling Setpoint Temperature [C]

Include new zone sizing fields in `epluszsz.csv`.

Existing `epluszsz.csv` zone sizing information:

    Des Heat Load [W]          - inludes impact of DOAS simulation
    Des Sens Cool Load [W]     - inludes impact of DOAS simulation
    Des Heat Mass Flow [kg/s]
    Des Cool Mass Flow [kg/s]

Proposed `epluszsz.csv` zone sizing information:

    Des Latent Heat Load [W]   - inludes impact of DOAS simulation
    Des Latent Cool Load [W]   - inludes impact of DOAS simulation
    Des Latent Heat Mass Flow [kg/s]
    Des Latent Cool Mass Flow [kg/s]
    Des Heat Load No DOAS [W]  - original zone load prior to DOAS simulation
    Des Cool Load No DOAS [W]  - original zone load prior to DOAS simulation
    Des Latent Heat Load No DOAS [W] - original latent load prior to DOAS simulation
    Des Latent Cool Load No DOAS [W] - original latent load prior to DOAS simulation

## Testing/Validation/Data Sources ##

Compare a few files using Sizing:Zone Zone Load Sizing Method = Sensible Load and
Zone Load Sizing Method = Sensible Load Only No Latent Load to verify the magnitude of
sizing changes when humidistat control is used during sizing calculations.

Existing example file 5ZoneAirCooled will be compared with and without latent sizing and with added zone latent loads (`OtherEquipment`) to determine impact on air flow sizing. One of the modified files will be saved as an air loop latent sizing example file.

Similar tests will be performed for existing example file PackagedTerminalHeatPump and saved as a zone equipment latent sizing example.

## Input Output Reference Documentation ##

Update Sizing:Zone and Sizing:System to include new input fields in `group-design-objects`.

## Engineering Reference ##

Update sizing discussion, design data arrays, and equations in `zone-design-loads-and-air-flow-rates` and `system-design-loads-and-air-flow-rates`.

## Example File and Transition Changes ##

Include a zone equipment and air loop equipment example file to exercise latent sizing.
Transition requirements are currently not required.

## References ##

 - [Trane Engineering Newsletter volume 48-3](<https://www.trane.com/content/dam/Trane/Commercial/global/products-systems/education-training/engineers-newsletters/airside-design/admapn071en-082019.pdf>)
 - [Murphy, J., Selecting DOAS Equipment with Reserve Capacity, Trane Commercial Business, Ingersoll Rand](https://www.trane.tm/content/dam/Trane/Commercial/global/products-systems/education-training/industry-articles/Apr2010ASHRAE.pdf)





## Model Design Documentation ##

The following code documentation is two fold. One is to document the existing calculations and methodology used during sizing, and the second is to provide a template for the proposed changes. This proposal suggests to duplicate existing sensible calculations with corresponding latent calculations to provide a time series representation of both sensible and latent sizing data.

**Proposed Calculation of Latent Sizing Data**

Regardless of the approach used to size zone or air system components, an accurate representation of zone sensible and latent loads is required. For this reason it is proposed to:

1) hold zone air humidity ratio at the humidistat set point during zone sizing. Otherwise the predicted latent load will include zone moisture capacitance and storage effects.
2) calculate and save latent sizing data similar to existing sensible sizing data
3) user input for sizing method will choose which time interval data is used for component sizing (i.e, sensible load, latent load, sensible and latent load, or sensible load only with no latent load).

**Existing zone sizing calculation for sensible cooling (similar for heating)**

**ZoneEquipmentManager::SizeZoneEquipment** - Zone sizing begins by the predict step in `ZoneTempPredictorCorrector` calculating zone sensible and latent loads and then simulating the DOAS system, if applicable, to determine the amount of zone load offset by the DOAS system. If the DOAS system is included in the zone sizing calculations, only the sensible portion of the DOAS supply air to the zone is accounted for in zone sizing.

    LatOutputProvided = 0.0;
    DOASCpAir = PsyCpAirFnW(DOASSupplyHumRat);
    DOASSysOutputProvided = DOASMassFlowRate * DOASCpAir * (DOASSupplyTemp - Node(ZoneNode).Temp);
    // DOAS simulation does not update latent cooling provided by DOAS
    UpdateSystemOutputRequired(state, ActualZoneNum, DOASSysOutputProvided, LatOutputProvided);

Next the zone cooling supply air temperature and humidity ratio from the `Sizing:Zone` object:

    Sizing:Zone,
      N1, \field Zone Cooling Design Supply Air Temperature
      N5, \field Zone Cooling Design Supply Air Humidity Ratio

are used as the known supply air condition (`Temp, HumRat`), to then calculate the temperature difference (`DeltaTemp`, which could have instead been input by the user) between the supply air temperature and zone air temperature (thermostat set point). Then calculate the `Enthalpy` of the supply air and read the zone sensible load from the predict step, `SysOutputRequired`. Using the supply air specific heat, `CpAir`, calculate the zone sensible air `MassFlowRate`. Save these data in time series arrays for further processing. Repeat for other zones (`ControlledZoneNum`) and for each design day (`CurOverallSimDay`). A final call to `UpdateSystemOutputRequired` is made which will inherently zero out the remaining load for this zone.

    Temp = CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolDesTemp;
    HumRat = CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolDesHumRat;
    DeltaTemp = Temp - Node(ZoneNode).Temp;
    Enthalpy = PsyHFnTdbW(Temp, HumRat);
    SysOutputProvided = ZoneSysEnergyDemand(ActualZoneNum).RemainingOutputRequired;
    CpAir = PsyCpAirFnW(HumRat);
    MassFlowRate = max(SysOutputProvided / (CpAir * DeltaTemp), 0.0);

    UpdateSystemOutputRequired(state, ActualZoneNum, SysOutputProvided, LatOutputProvided);

These data are placed on the zone supply node for use in the correct step of `ZoneTempPredictorCorrector` to calulate the resulting zone temperature and humidity ratio. In current EnergyPlus calculations, the zone temperature is maintained at the thermostat set point temperature while the zone humidity ratio floats according to the humidity ratio entering the zone at the zone supply node **sensible** mass flow rate. It is not important at this time to actually provide the zone the correct mass flow rate, what is important here is to meet the zone sensible load exactly such that the zone temperature remains at the thermostat set point. Update: Zone return air heat gain calculations would be affected by a change in air mass flow rate. An attempt will be made to retain the sensible mass flow rate if possible.

    Node(SupplyAirNode).Temp = Temp;
    Node(SupplyAirNode).HumRat = HumRat;
    Node(SupplyAirNode).Enthalpy = Enthalpy;
    Node(SupplyAirNode).MassFlowRate = MassFlowRate;

Finally, save the time series data. The example shown here is when there is a cooling load.

    if (SysOutputProvided > 0.0) {
    } else if (SysOutputProvided < 0.0) {
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLoad = -SysOutputProvided;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolMassFlow = MassFlowRate;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLoad = 0.0;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatMassFlow = 0.0;
    } else {
    }

    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatZoneTemp = Node(ZoneNode).Temp;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatZoneHumRat = Node(ZoneNode).HumRat;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolZoneTemp = Node(ZoneNode).Temp;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolZoneHumRat = Node(ZoneNode).HumRat;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatOutTemp = OutDryBulbTemp;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatOutHumRat = OutHumRat;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolOutTemp = OutDryBulbTemp;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolOutHumRat = OutHumRat;

**Proposed zone sizing calculations for latent cooling (similar for heating)**

**Step 1:** Save zone sensible and latent loads prior to DAOS simulation.

    // save raw zone loads without impact of outdoor air
    LatOutputProvidedNoDOAS = ZoneSysMoistureDemand(ZoneNum).RemainingOutputRequired;
    SysOutputProvidedNoDOAS = ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired;

**Step 2:** Simulate DOAS system, if applicable according the the `Sizing:Zone` object, and update remaining zone loads

    Sizing:Zone,
      A8, \field Account for Dedicated Outdoor Air System
      \key Yes
      \key No
      \default No


    DOASLatOutputProvided = 0.0;
    DOASCpAir = PsyCpAirFnW(DOASSupplyHumRat);
    DOASSysOutputProvided = DOASMassFlowRate * DOASCpAir * (DOASSupplyTemp - Node(ZoneNode).Temp);
    // example if test to turn off latent calcs (note: this if test may not be needed)
    if (zoneRHSizing) { // new latent sizing code
        DOASLatOutputProvided = DOASMassFlowRate * (DOASSupplyHumRat - Node(ZoneNode).HumRat);
    }

    UpdateSystemOutputRequired(state, ActualZoneNum, DOASSysOutputProvided, DOASLatOutputProvided);

**Step 3:** Repeate zone sensible calulations for zone sensible mass flow rate (not shown, same as existing code above) and add new calculations for zone latent load and mass flow rate.

         Real64 LatentAirMassFlow = 0.0;
        Real64 MoistureLoad = 0.0;
        Real64 HgAir = PsyHgAirFnWTdb(Node(ZoneNode).HumRat, Node(ZoneNode).Temp);
        if (zoneLatentSizing) {
            Real64 ZoneRH = Psychrometrics::PsyRhFnTdbWPb(state, Node(ZoneNode).Temp, Node(ZoneNode).HumRat, state.dataEnvrn->OutBaroPress); // TRANE
            // replicate !deadband flag - zone condition is either below the humidistat or above the dehumidistat set point
            if ((zoneSysMoistureDemand.OutputRequiredToHumidifyingSP > 0.0 && zoneSysMoistureDemand.OutputRequiredToDehumidifyingSP > 0.0) ||
                (zoneSysMoistureDemand.OutputRequiredToHumidifyingSP < 0.0 && zoneSysMoistureDemand.OutputRequiredToDehumidifyingSP < 0.0)) {
                LatOutputProvided = zoneSysMoistureDemand.RemainingOutputRequired;
            }
            // positive LatOutputProvided means humidification load
            Real64 DeltaHumRat = 0.0;
            if (LatOutputProvided < 0.0) {
                if (calcZoneSizing.ZnLatCoolDgnSAMethod == SupplyAirHumidityRatio) {
                    DeltaHumRat = calcZoneSizing.CoolDesDehumHumRat - Node(ZoneNode).HumRat;
                } else {
                    DeltaHumRat = calcZoneSizing.CoolDesHumRatDiff;
                }
            } else if (LatOutputProvided > 0.0) {
                if (calcZoneSizing.ZnLatCoolDgnSAMethod == SupplyAirHumidityRatio) {
                    DeltaHumRat = calcZoneSizing.HeatDesHumidifyHumRat - Node(ZoneNode).HumRat;
                } else {
                    DeltaHumRat = calcZoneSizing.HeatDesHumRatDiff;
                }
            }
            if (std::abs(DeltaHumRat) > DataHVACGlobals::VerySmallMassFlow) LatentAirMassFlow = LatOutputProvided / DeltaHumRat;
            MoistureLoad = LatOutputProvided * HgAir;
      }

**Step 4:** Now that zone sensible and latent mass flow rate is known, and likely different, correct the zone supply air node entering conditions to meet both the thermostat and humidistat set points. Make sure that the correct mass flow rates are saved prior to making any adjustment to zone supply node mass flow rate.

    if (zoneRHSizing) {
            if (MassFlowRate > 0.0) {
                HumRat = Node(ZoneNode).HumRat + LatOutputProvided / MassFlowRate;
                // need to recalculate supply air Temp/Enthalpy based on new supply air HumRat
                CpAir = PsyCpAirFnW(HumRat);
                Temp = (SysOutputProvided / (MassFlowRate * CpAir)) + Node(ZoneNode).Temp;
                Enthalpy = PsyHFnTdbW(Temp, HumRat);
            } else {
                // if there is no sensible load then still need to hold zone RH at set point
                // could look at LatOutputProvided here and just used some temporary mass flow rate?
                if (LatentAirMassFlow > DataHVACGlobals::VerySmallMassFlow) {
                    // no need to recalculate T, Sensible load = 0 so T = T,zone
                    HumRat = Node(ZoneNode).HumRat + LatOutputProvided / LatentAirMassFlow;
                    MassFlowRate = LatentAirMassFlow;
                    Enthalpy = PsyHFnTdbW(Temp, HumRat);
                } else {
                    // supply air T = T,zone; w = w,zone; MassFlowRate = 0
                    LatentAirMassFlow = 0.0;
                }
            }
      }

**Step 5:** The final call to update `UpdateSystemOutputRequired` should 0 out any remaining sensible and latent loads.

    UpdateSystemOutputRequired(state, ActualZoneNum, SysOutputProvided, LatOutputProvided);

Just as is done with existing zone sizing results, the data calculated here must be saved in time interval arrays for post processing.

**Proposed zone sizing variables used for post processing:**

The following section describes the new Struct variables required to implement the latent sizing new feature. These new variables will be added to the code at locations similar to those used for sensible data processing.

Existing Struct variables tracking zone sizing information.

    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLoad
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolMassFlow
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLoad
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatMassFlow

New Struct variables tracking zone sizing information shown as example code in `ZoneEquipmentManager::SizeZoneEquipment`.

    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLatentLoad = MoistureLoad;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLatentMassFlow = LatentAirMassFlow;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLatentLoad = 0.0;
    CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLatentMassFlow = 0.0;

    if (SysOutputProvidedNoDOAS > 0.0) {
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLoadNoDOAS = 0.0;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLoadNoDOAS = SysOutputProvidedNoDOAS;
    } else if (SysOutputProvidedNoDOAS < 0.0) {
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLoadNoDOAS = -SysOutputProvidedNoDOAS;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLoadNoDOAS = 0.0;
    } else {
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLoadNoDOAS = 0.0;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLoadNoDOAS = 0.0;
    }
    if (zoneRHSizing) {
        LatOutputProvidedNoDOAS *= PsyHgAirFnWTdb(Node(ZoneNode).HumRat, Node(ZoneNode).Temp);
        if (LatOutputProvidedNoDOAS > 0.0) {
            CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLatentLoadNoDOAS = 0.0;
            CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLatentLoadNoDOAS =                     LatOutputProvidedNoDOAS;
        } else if (LatOutputProvidedNoDOAS < 0.0) {
            CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLatentLoadNoDOAS = -LatOutputProvidedNoDOAS;
            CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLatentLoadNoDOAS = 0.0;
        }
    } else {
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).CoolLatentLoadNoDOAS = 0.0;
        CalcZoneSizing(CurOverallSimDay, ControlledZoneNum).HeatLatentLoadNoDOAS = 0.0;
    }

**During Day Sizing Data Processing**

For each time interval the sizing data is stored (cooling example shown)

    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolFlowSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolMassFlow * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolLoadSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolLoad * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneTempSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneTemp * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutTempSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutTemp * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneRetTempSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneRetTemp * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneHumRatSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneHumRat * FracTimeStepZone;
    CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutHumRatSeq(TimeStepInDay) +=
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutHumRat * FracTimeStepZone;

**End Day Sizing Data Processing**

The time interval data is averaged based on the `<Sizing:Parameters>` averaging window. Other sizing data also averages the time interval data: CoolLoadSeq, CoolZoneRetTempSeq. Moisture variables are not included in this averaging step and will likely be necessary (e.g., CoolZoneRetHumRatSeq and new latent load variables).

    Sizing:Parameters,
      N3; \field Timesteps in Averaging Window


    // average some of the zone sequences to reduce peakiness
    for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
        if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
        state.dataZoneEquipmentManager->AvgData = 0.0;
        MovingAvg(CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolFlowSeq,
                  NumOfTimeStepInDay, NumTimeStepsInAvg,
                  state.dataZoneEquipmentManager->AvgData);
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolFlowSeq = state.dataZoneEquipmentManager->AvgData;
            }

For each time interval the cooling load is compared to a scalar DesCoolLoad (initializes to 0) and the scalar is incremented to identify the peak cooling load and all associated sizing data with that time interval. The time interval of the peak load is also saved for reporting.

    for (TimeStepIndex = 1; TimeStepIndex <= NumOfTimeStepInDay; ++TimeStepIndex) {
        if (CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolLoadSeq(TimeStepIndex) >
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolLoad) {
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolLoad =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolLoadSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolMassFlow =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolFlowSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).ZoneTempAtCoolPeak =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneTempSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).OutTempAtCoolPeak =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutTempSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).ZoneRetTempAtCoolPeak =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneRetTempSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).ZoneHumRatAtCoolPeak =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolZoneHumRatSeq(TimeStepIndex);
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).OutHumRatAtCoolPeak =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolOutHumRatSeq(TimeStepIndex);

            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).TimeStepNumAtCoolMax = TimeStepIndex;
        }
    }

Once the peak is known, the peak load air mass flow rate is converted to volume and the coil inlet temperature and humidity ratio are calculated.

    if (CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolMassFlow > 0.0) {
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolVolFlow =
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolMassFlow /
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolDens;
        OAFrac = CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).MinOA /
                 max(CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolVolFlow, SmallMassFlow);
        OAFrac = min(1.0, max(0.0, OAFrac));
        TimeStepAtPeak = CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).TimeStepNumAtCoolMax;
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolCoilInTemp =
            OAFrac * DesDayWeath(CurOverallSimDay).Temp(TimeStepAtPeak) +
            (1.0 - OAFrac) * CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).ZoneTempAtCoolPeak;
        CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolCoilInHumRat =
            OAFrac * DesDayWeath(CurOverallSimDay).HumRat(TimeStepAtPeak) +
            (1.0 - OAFrac) * CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).ZoneHumRatAtCoolPeak;
    }

I am not quite following this final step of the End Day calc. If the design day  (`CalcZoneSizing`) DesignCoolVolFlow is greater than the `CalcFinalZoneSizing` scalar DesCoolVolFlow (initializes to 0), then save design data to the CalcFinalZoneSizing array. Else, if the design day DesCoolLoad is greater than the CalcFinalZoneSizing scalar DesCoolLoad (this would indicate pull-up or pull-down based on a change in supply air delta T?), then save design data to the CalcFinalZoneSizing array. This implies the zone air temperature varies during sizing (since flow is proportional to load for a fixed supply air and zone temperature, and supply air temperature does not change) where is certainly possible that zone temperature changes with thermostat setup and setback and the determination of "peak condition" could be based either on air flow or load. I am not sure at this point why a peak load calculation would allow either indicator to set the peak. Although uncertain of the implications, the proposed latent sizing calculations will mirror the existing sensible sizing calculations.

    // from all the design periods, choose the one needing the most Cooling and save
    // all its design variables in CalcFinalZoneSizing
    if (CalcZoneSizing(state.dataSize->CurOverallSimDay, CtrlZoneNum).DesCoolVolFlow >
        CalcFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow) {
        CalcFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow =
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolVolFlow;
        CalcFinalZoneSizing(CtrlZoneNum).DesCoolLoad =
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolLoad;
        CalcFinalZoneSizing(CtrlZoneNum).DesCoolMassFlow =
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolMassFlow;
        CalcFinalZoneSizing(CtrlZoneNum).CoolDesDay =
            CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolDesDay;
        // other design data removed for brevity
    } else {
        CalcFinalZoneSizing(CtrlZoneNum).DesCoolDens = state.dataEnvrn->StdRhoAir;
        // save design cooling load when the there is design cooling load and the
        // design cooling volume flow rate is zero, i.e., when
        // design cooling volume flow rate is set to zero due to cooling supply air
        // temp greater than zone thermostat temperature
        if (CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolLoad >
            CalcFinalZoneSizing(CtrlZoneNum).DesCoolLoad) {
            CalcFinalZoneSizing(CtrlZoneNum).DesCoolLoad =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).DesCoolLoad;
            CalcFinalZoneSizing(CtrlZoneNum).CoolDesDay =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolDesDay;
            CalcFinalZoneSizing(CtrlZoneNum).CoolLoadSeq =
                CalcZoneSizing(CurOverallSimDay, CtrlZoneNum).CoolLoadSeq;
        // other design data removed for brevity
    }

**EndZoneSizingCalc Sizing Data Processing**

    // candidate EMS calling point to customize CalcFinalZoneSizing
    bool anyEMSRan;
    ManageEMS(state, EMSManager::EMSCallFrom::ZoneSizing, anyEMSRan, ObjexxFCL::Optional_int_const());

    // now apply EMS overrides (if any)
    if (AnyEnergyManagementSystemInModel) {
        for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
            if (CalcFinalZoneSizing(CtrlZoneNum).EMSOverrideDesHeatMassOn) {
                if (CalcFinalZoneSizing(CtrlZoneNum).DesHeatMassFlow > 0.0)
                    CalcFinalZoneSizing(CtrlZoneNum).DesHeatMassFlow =
                        CalcFinalZoneSizing(CtrlZoneNum).EMSValueDesHeatMassFlow;
            }
            // and other EMS overrides
        }
    }

And finally, check for zero zone loads for reporting purposes and then report zone sizing results to the `epluszsz.csv` file.

    for (CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
        if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
        if (std::abs(CalcFinalZoneSizing(CtrlZoneNum).DesCoolLoad) <= 1.e-8) {
            ShowWarningError(state, "Calculated design cooling load for zone=" + CalcFinalZoneSizing(CtrlZoneNum).ZoneName + " is zero.");
            ShowContinueError(state, "Check Sizing:Zone and ZoneControl:Thermostat inputs.");
        }
        if (std::abs(CalcFinalZoneSizing(CtrlZoneNum).DesHeatLoad) <= 1.e-8) {
            ShowWarningError(state, "Calculated design heating load for zone=" + CalcFinalZoneSizing(CtrlZoneNum).ZoneName + " is zero.");
            ShowContinueError(state, "Check Sizing:Zone and ZoneControl:Thermostat inputs.");
        }
    }


    print(state.files.zsz, "Time");
    // 4 per zone existing, versus 10 per zone proposed

**Application of Latent Sizing to Component Sizing**

Component sizing is currently based on a calculation of zone sensible supply air mass flow rate, a predicted zone sensible load, and a zone thermostat and supply air temperature. The peak air mass flow rate can meet the peak zone sensible load at the user specified supply air temperature. Mdot, sens = Qsens / (Cp * (Tsupply - Tzone)).

Latent sizing would be based on this same principle of calculating a zone latent air mass flow rate to meet the peak latent load. The final zone design air mass flow rate will either be selected, based on user input, as the peak sensible air mass flow (`Sensible Load` or `Sensible Load Only No Latent Load` sizing), the peak latent air mass flow (`Latent Load` sizing) or the maximum of the sensible and latent air mass flow (`Sensible And Latent Load` sizing). Mdot, lat = Qlat / (w, supply - w, zone).

The zone latent load is independent of zone sensible load, save any dependence of moist air properties (specific heat) on resultant load calculations. I assume zone sensible loads will change slightly if zone humidity ratio changes during sizing (i.e., zone humidity will now be held constant). I intend to prove that assumption, and that the changes are small.

The proposed result of zone sizing is to record both a zone sensible mass flow rate and sensible load, and a latent mass flow rate and latent load. These time series data are the basis for zone and air system component sizing. With the addition of zone latent load sizing calculations other components should be more accurately sized to meet the imposed loads (e.g., DOAS systems or zone/air system cooling coils).

The initial approach will be to choose a load and mass flow rate based on user inputs. The chosen data will be stored in the FinalZoneSizing.DesCoolMassFlowRate and FinalZoneSizing.DesCoolVolFlow variables (same for heating) at the end of the sizing calculation. This method is chosen simply to avoid carrying multiple variables throughout component sizing. Changes to code due to the addition of latent sizing would be complete at the end of zone sizing.

Since there is a significant amount of code that utilizes the sizing information, the new latent sizing arrays will be substituted into the existing final sizing arrays and scalar variables based on the user choice of sizing method (i.e., sensible [do nothing], latent [use latent data], sensible or latent [use greater of either]). This substitution will occur at the very end of zone sizing just after eplusout.zsz gets written and right before the calc arrays get moved to the final arrays. The example below shows this concept where latent arrays and scalars are substituted into the final calc data if the user chooses to size on latent loads. If the user chooses to size on the greater of sensible or latent loads, the latent arrays will be substituted accordingly.

The sizing information reported to eplusout.zsz has just been written to report sensible and latent loads. Example substitution logic is shown below. This occurs at the very end of `ZoneEquipmentManager::UpdateZoneSizing`.

    case DataGlobalConstants::CallIndicator::EndZoneSizingCalc: {

    ** process sizing results and write results to eplusout.zsz **

    ** now perform the substitution **


    // Move sizing data into final calc sizing array according to sizing method

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {

        if (!state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) continue;
        if (!state.dataSize->CalcFinalZoneSizing(zoneNum).zoneLatentSizing) continue;
        auto &calcFinalZoneSizing = state.dataSize->CalcFinalZoneSizing(zoneNum);

        if (calcFinalZoneSizing.zoneSizingMethod == ZoneSizing::Latent) {
            calcFinalZoneSizing.DesCoolVolFlow = calcFinalZoneSizing.DesCoolLatentVolFlow;
            calcFinalZoneSizing.cCoolDDDate = calcFinalZoneSizing.cLatentCoolDDDate;
            calcFinalZoneSizing.CoolDDNum = calcFinalZoneSizing.LatentCoolDDNum;
            calcFinalZoneSizing.CoolFlowSeq = calcFinalZoneSizing.ZoneCoolLatentMassFlowSeq;
            // add everything else needed for component sizing

            calcFinalZoneSizing.DesHeatVolFlow = calcFinalZoneSizing.DesHeatLatentVolFlow;
            calcFinalZoneSizing.cHeatDDDate = calcFinalZoneSizing.cLatentHeatDDDate;
            calcFinalZoneSizing.HeatDDNum = calcFinalZoneSizing.LatentHeatDDNum;
            calcFinalZoneSizing.HeatFlowSeq = calcFinalZoneSizing.ZoneHeatLatentMassFlowSeq;
            // add everything else needed for component sizing

        } else if (calcFinalZoneSizing.zoneSizingMethod == ZoneSizing::SensibleAndLatent) {

            if (calcFinalZoneSizing.DesCoolLatentVolFlow > calcFinalZoneSizing.DesCoolVolFlow) {
                calcFinalZoneSizing.DesCoolVolFlow = calcFinalZoneSizing.DesCoolLatentVolFlow;
                calcFinalZoneSizing.cCoolDDDate = calcFinalZoneSizing.cLatentCoolDDDate;
                calcFinalZoneSizing.CoolDDNum = calcFinalZoneSizing.LatentCoolDDNum;
                calcFinalZoneSizing.CoolFlowSeq = calcFinalZoneSizing.ZoneCoolLatentMassFlowSeq;
                // add everything else needed for component sizing
            }

            if (calcFinalZoneSizing.DesHeatLatentVolFlow > calcFinalZoneSizing.DesHeatVolFlow) {
                calcFinalZoneSizing.DesHeatVolFlow = calcFinalZoneSizing.DesHeatLatentVolFlow;
                calcFinalZoneSizing.cHeatDDDate = calcFinalZoneSizing.cLatentHeatDDDate;
                calcFinalZoneSizing.HeatDDNum = calcFinalZoneSizing.LatentHeatDDNum;
                calcFinalZoneSizing.HeatFlowSeq = calcFinalZoneSizing.ZoneHeatLatentMassFlowSeq;
                // add everything else needed for component sizing
            }

        }
    }

Existing code follows where the calc arrays are moved to the final sizing arrays:

    // Move data from Calc arrays to user modified arrays

    for (std::size_t i = 0; i < state.dataSize->ZoneSizing.size(); ++i) {
        auto &z(state.dataSize->ZoneSizing[i]);
        auto &c(state.dataSize->CalcZoneSizing[i]);
        z.CoolDesDay = c.CoolDesDay;
        z.HeatDesDay = c.HeatDesDay;
        .. snip..
    }

    for (std::size_t i = 0; i < state.dataSize->FinalZoneSizing.size(); ++i) {
        auto &z(state.dataSize->FinalZoneSizing[i]);
        auto &c(state.dataSize->CalcFinalZoneSizing[i]);
        z.CoolDesDay = c.CoolDesDay;
        z.HeatDesDay = c.HeatDesDay;
        .. snip..
    }

More final processing occurs at this point of EndZoneSizingCalc. This final processing code will be reviewed to determine the best location for the substitution described here and whether the final processing code needs updating or correction.

At this point, latent sizing processing should be complete.
