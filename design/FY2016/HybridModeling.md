Hybrid Model: Zone Capacitance and Infiltration
================

 **Tianzhen Hong, Sang Hoon Lee**
 **Lawrence Berkeley National Laboratory**

 - Original: May 18, 2015
 - Updated: August 25, 2016; October 20, 2016

## Justification for New Feature ##

The proposed new feature introduces a new hybrid model to simulate the energy performance of existing buildings. Hybrid model algorithms use easily measurable zone air temperature to replace highly uncertain and difficult to measure parameters of zone air infiltration rate and zone internal thermal mass in the zone heat balance calculations by solving the reformulated zone heat balance inverse equations. The hybrid approach keeps the virtue of the physics-based model and taking advantage of more measured buildings data which is available nowadays due to the wide use of low-cost sensors and the needs of better controls in existing buildings.

This new feature proposal provides technical details of the hybrid modeling algorithm and its implementation in EnergyPlus. The hybrid modeling simulation feature will improve simulation usability and accuracy for existing buildings, which supports more accurate analysis of energy retrofits.

## Overview ##

The engineering reference of EnergyPlus provides the assumptions used to handle the zone internal thermal mass in the EnergyPlus simulations. EnergyPlus applies the same boundary conditions to both sides of the construction representing the zone internal thermal mass so that there is no temperature difference across the two surfaces. Internal mass with both faces within a same zone is modeled either by creating additional adiabatic surfaces. The surface stores and releases heat only from the inside face of the surface within the zone. There are two ways to model internal thermal mass using EnergyPlus. One way is to add internal mass for zones with predetermined construction types, and the other way is to apply zone capacitance multiplier.

###	Internal thermal mass ###
There are two ways to model internal thermal mass using EnergyPlus. One way is to add internal mass construction for zones, and the other way is to use a zone capacitance multiplier.

(a)	Internal Mass Object

The first method logically describes interior partitions and furniture as internal mass. The EnergyPlus input field, “InternalMass” is used to specify the construction/material parameters and area of items within the space that are important to heat transfer calculations. The surfaces of internal mass only exchange energy with the zone. Internal mass object can represent multiple pieces of internal mass with each piece with a different construction type. Internal mass exchanges energy through both surfaces with the zone by convection. There is no heat transfer across both surfaces of the internal mass construction. As the internal mass construction is ignored geometrically, the approach does not address the solar heat gains through windows.

(b)	Zone Capacitance Multiplier

There is an EnergyPlus object, “ZoneCapacitanceMultiplier:ResearchSpecial”, an advanced feature to control the effective storage capacity of the zone. A capacitance multiplier of 1.0 indicates the capacitance comes from only the air in the zone. The default value is set to 1.0. This multiplier can be adjusted for stability of the simulation or to allow modeling higher or lower levels of thermal damping of behavior over time. This multiplier is linked to the zone predictor-correction algorithm for the simulation by increasing the zone air volume. The multiplier is applied to the base value corresponding to the total capacitance for the zone’s volume of air at current zone (moist) conditions. This multiplier numerically increases or decreases the zone air temperature deviations at the time step for the simulation to account for the additional capacitance in the air loop not specified in the zone. The zone multiplier changes the zone volume used for the air ratio and is constant throughout the simulation.  The current implementation in EnergyPlus assumes a single constant capacitance multiplier for all zones in an energy model.

We will use this multiplier approach to represent the internal thermal mass and change this feature to allow different capacitance multipliers for different zones.

### Infiltration ###
EnergyPlus uses the object, ZoneInfiltration:DesignFlowRate, to represent the infiltration caused by the opening and closing of exterior doors, cracks around windows, and even in very small amount through building elements. Users define the infiltration design air flow rate, an infiltration schedule, and temperature and wind correction coefficients. The infiltration hybrid model derives the time-step zone infiltration air flow rates, using the inverse zone heat balance equation and the zone air temperature data, which consider all complexities of design flow rate, coefficients and climate conditions.

## Approach ##

The hybrid model algorithms are built upon the physics-based zone heat balance equation  reformulated to solve a partially inverse problem. It should be noted that the hybrid algorithms to be developed are generic and can be adopted by EnergyPlus and other building energy simulation programs.

The basis for the zone air system integration is to formulate heat balances for the zone air and solve the resulting ordinary differential equations.

$$C_z \frac {dT_z} {dt} = ∑Q_i +∑[h_i A_i (T_{si} - T_z)] + ∑[ṁ_i C_p (T_{zi}-T_z)] + ṁ_{inf} C_p (T_o - T_z) + Q_{sys}~~~(1)$$
$Where:$

$∑Q_i$  	= sum of the convective internal loads
$∑[h_i A_i(T_{si} - T_z)]$	= convective heat transfer from the zone surfaces
$T_{si}$	= zone surface temperature
$∑[ṁ_i C_p (T_{zi} - T_z)]$	= heat transfer due to interzone air mixing
$T_z$	= zone air temperature
$ṁ_{inf} C_p (T_o - T_z)$ 	= heat transfer due to outside air infiltration
$T_o$	= outdoor air temperature
$ṁ_{inf} = q_{inf} ρ_{air}$	= infiltration mass flow rate
$q_{inf}$	= infiltration air flow rate
$C_z \frac {dT_z} {dt}$ 	= energy stored in zone air including the internal thermal mass of the zone air node
$C_z = V ρ_{air} C_p C_T$ 	= heat capacity of air per volume including internal thermal mass and zone air
$V$	= Zone volume
$ρ_{air}$	= air density
$C_p$	= zone air specific heat
$C_T$	= sensible heat capacity multiplier
$Q_{sys} = ṁ_{sys} C_p (T_{sup} - T_z )$ 	= air system energy provided to the zone to meet heating and cooling loads
$T_{sup}$	= air system supply air temperature

Equation (1) assumes that the sum of zone loads and air system output equals the change in energy stored in the zone. The internal thermal mass, including furniture, books, and changeable partitions, is assumed to be in thermal equilibrium with the zone air, thus it is added in the zone heat capacitance, $C_z$.  The infiltration airflow rate, $q$ changes for different conditions depending on outdoor temperature, wind speed, and HVAC system operations. The energy provided from systems to the zone is represented as $Q_{sys}$.

The sum of zone loads and the provided air system energy equals the change in energy stored in the zone. Typically, the capacitance $C_z$ would be that of the zone air only. The internal thermal masses, assumed to be in equilibrium with the zone air, are included in this term. EnergyPlus provides algorithms to solve the zone air energy and moisture balance equations defined in the ZoneAirHeatBalanceAlgorithm object. The algorithms use the finite difference approximation or analytical solution to calculate the derivative term with respect to time.

The internal mass in the current EnergyPlus model uses a capacitance multiplier indicating the capacitance as part of the air in the volume of the specified zone. The default value is given as 1.0 as the base value corresponding to the total capacitance for the zone’s volume of air at current zone conditions. Although users can modify the multiplier, there lacks a guideline on what reasonable values to use.

EnergyPlus provides three different heat balance solution algorithms to solve the zone air energy balance equations. These are defined in the Algorithm field in the ZoneAirHeatBalanceAlgorithm object: 3rdOrderBackwardDifference, EulerMethod and AnalyticalSolution. The first two methods to solve Equation use the finite difference approximation while the third uses an analytical solution.

The AnalyticalSolution algorithm is an integration approach. While the 3rd order finite difference approximation provides stability without requiring a prohibitively small time step, the method still has truncation errors and requires a fixed time step length for the previous three simulation time steps. Therefore, different time step lengths for the previous three simulation time steps may make the temperature coefficients invalid.

The AnalyticalSolution algorithm provides a possible way to obtain solutions without truncation errors and independent of time step length. In addition, the algorithm only requires the zone air temperature for one previous time step, instead of three previous time steps as required by the 3rdOrderBackwardDifference algorithm.

The hybrid modeling approach uses the AnalyticalSolution for internal thermal mass inverse calculation and the 3rdOrderBackwardDifference for infiltration inverse calculation. EnergyPlus Code for these heat balance algorithms are referenced to the ZoneTempPredictorCorrector module.

(a) Internal thermal mass hybrid modeling method

The formulation of the solution scheme starts with a heat balance on the zone air using the AnalyticalSolution method used in EnergyPlus. The method calculates the time-series zone air temperature, $T_z$ reformulating Equation (1) as shown below in Equation (2).

$$T_z^t = [T_z^{t-δt} - \frac {∑Q_i + ∑(h_i A_i T_{si}) + ∑(ṁ_i C_p T_{zi}) +ṁ_{inf} C_p T_o + ṁ_{sys} C_pT_{sup}^t} {∑(h_i A_i)  + ∑(ṁ_i C_p) + ṁ_{inf} C_p + ṁ_sys C_p}]  \\ \times e^{ - \frac {∑(h_i A_i) + ∑(ṁ_i C_p ṁ_{inf} C_p)+ ṁ_{sys} C_p} {C_z^t } δt} \\ + \frac {∑Q_i +∑(h_i A_i T_{si}) + ∑(ṁ_i C_p T_{zi}) + ṁ_{inf} C_p T_o + ṁ_{sys} C_p T_{sup}^t} {∑(h_i A_i) +∑(ṁ_i C_p)  + ṁ_{inf} C_p + ṁ_{sys} C_p }~~~(2)$$

The approach in this hybrid modeling method will derive the internal mass by solving the heat capacity of zone air, $C_z$. The derivation is based on the inverse modeling method replacing the input of internal thermal mass with the measured zone air temperature, $T_z$. The zone air temperature is the only additional requirement for the proposed approach.

The hybrid modeling is a generic approach and applicable to any building with very limited amount of measured data. It is expected to have inputs of the measured interval zone temperature data for at least one week. Zone heat capacity is an important component for buildings as it stabilizes internal temperatures, thus at least one week of the measured internal temperature can capture the stored heat in the internal thermal mass.

Now, $T_z^t$  and $T_z^{t-δt}$ are given, thus, the inverse model of Equation (2) for the variable, $C_z^t$ is expressed as following.

$$C_z^t = - \frac {[∑(h_i A_i) + ∑(ṁ_i C_p ṁ_{inf} C_p) + ṁ_{sys} C_p ] δt}{ ln⁡ \left[ \frac { T_z^t - \frac { ∑Q_i +∑(h_i A_i T_{si}) + ∑(ṁ_i C_p T_{zi}) + ṁ_{inf} C_p T_o + ṁ_{sys} C_p T_{sup}^t} {∑(h_i A_i)  + ∑(ṁ_i C_p)  + ṁ_{inf} C_p + ṁ_{sys} C_p}} {T_z^{t-δt} - \frac {∑Q_i +∑(h_i A_i T_si)  + ∑(ṁ_i C_p T_zi) + ṁ_{inf} C_p T_o + ṁ_{sys} C_p T_{sup}^t} {∑(h_i A_i) + ∑(ṁ_i C_p)  + ṁ_{inf} C_p + ṁ_{sys} C_p}} \right]}~~~(3)$$

From Equation (3), replacing zone temperature, $T_z$ with the measured zone temperature data, then the thermal mass term, $C_z^t$ can be calculated.  Then, the temperature capacity multiplier which is used to represent internal mass (IM) multiplier, $C_T^t$ is calculated for each time step using Equation (4).

$$C_T^t = \frac{C_z^t} {Vρ_{air} C_p}~~~ (4)$$

The default value of $C_T^t$ is 1.0. Ideally the zone heat capacity remains constant for the same condition of the internal environment in the zone heat balance equation. However, measured temperatures are not the same as the simulated zone air temperatures which is the result of the energy simulation in Equation (2). This causes the IM multiplier, $C_T^t$, the result from the inverse model is not constant during the course of the simulation period. The hybrid model will determine a time span when $|T_z^t - T_z^{t-δt}| > 0.1°C$ that $C_z^t$ remains more constant. IM multiplier calculations are only done when the zone air temperature difference between timesteps meets the condition. This filter is needed for more reliable inverse calculation to avoid the anomaly conditions due to the use of the inverse model.

(b) Infiltration hybrid modeling method

This section provides technical details to derive algorithms for estimation of the infiltration airflow rates. The development of the infiltration hybrid modeling algorithm is consistent with the EnergyPlus source code. The source code module, ZoneEquipmentManager, contains the simplified infiltration algorithm as shown in Equation (5).

$$Infiltration = Infiltration_{design} F_{schedule} [A+B|T_z −T_o|+C (WindSpeed)+D(Windspeed^2 )]~~~(5)$$
$Where:$
$A$ = Constant term coefficient
$B$ = Temperature term coefficient
$C$ = Velocity term coefficient
$D$ = Velocity squared coefficient

The simple method has an empirical correlation that modifies the infiltration as a function of wind speed and temperature difference across the envelope. The difficulty in using this equation is determining valid coefficients for each building type in each location. The simplified infiltration models consider the wind speed on zone altitude, and the variation in infiltration heat loss based on the wind velocity. These coefficients vary and provide very different results that cause great uncertainty in determining which numbers to use. EnergyPlus allows users to input these coefficients, however it is not easy to identify correct ones for typical modeling practices.

The infiltration hybrid modeling approach derives the infiltration mass flow rate, $ṁ_{inf} C_p$ by reformulating the zone air heat balance algorithm. For the inverse modeling of the heat balance algorithm for the infiltration hybrid modeling, 3rdOrderBackwardDifference method is used. The inverse model to derive the internal mass flow, $ṁ_{inf} C_p$ using the AnalyticalSolution cannot be realized in a mathematical form when deriving the infiltration mass flow rate. The time-series zone air temperature, $T_z$ using the 3rd order method is shown in Equation (6).

$$T_z^t = \\ \frac {∑Q_i +∑(h_i A_i T_{si}) + ∑(m ̇_i C_p T_{zi}) + ṁ_{inf} C_p T_o + ṁ_{sys} C_p T_{sup}^t - \frac {C_z} {δt}(-3T_z^{t-δt} + \frac {3} {2} T_z^{t-2δt} - \frac {1} {3} T_z^{t-3δt})} { \frac {11} {6} \frac {C_z} {δt} + ∑(h_i A_i)  + ∑(ṁ_i C_p) + ṁ_{inf} C_p + ṁ_{sys} C_p}~~~(6)$$

Equation (7) shows the inverse algorithm for infiltration hybrid modeling method to derive the zone infiltration mass flow rate using the measured air temperature.

$$ṁ_{inf} C_p = \frac {∑Q_i + ∑(h_i A_i T_{si}) + ∑(ṁ_i C_p T_{zi}) + ṁ_{sys} C_p T_{sup}^t - \frac {C_z} {δt} (-3T_z^{t-δt} + \frac {3} {2} T_z^{t-2δt} - \frac {1} {3} T_z^{t-3δt}) - T_z^t [ \frac {11} {6} \frac {C_z} {δt} + ∑(h_i A_i) + ∑(ṁ_i C_p) + ṁ_{sys} C_p]} {T_z^t - T_o}~~~(7)$$

The infiltration air flow rate, q_inf is calculate from the derived infiltration mass flow rate, $ṁ_{inf} C_p$, in Equation (8):

$$q_{inf}= \frac {ṁ_{inf} C_p} {ρ_{air} C_p}~~~(8)$$

For the infiltration mode of the hybrid model simulation, the calculation is only done when the zone air temperature difference between the current and previous timestep is less than 0.1°C and the zone air and outdoor air temperature difference is greater than 5°C as depicted $|T_z^t - T_o^t| > 5.0°C$ and $|T_z^t - T_z^{t-δt}| < 0.1°C$

(c) Assumptions

The current hybrid model implementation applies to periods when HVAC systems are off, i.e. spaces are in free floating mode. However, this is not a limitation of the hybrid modeling but rather based on the assumption that measured energy delivered by HVAC systems is not easily available. If the hybrid mode simulation is used when HVAC system operates, it requires data of the supply air temperature and supply air volume to derive $Q_{sys}$.

The challenge of the hybrid model algorithm lies when both infiltration and internal mass parameters are unknown. From previous validation study using the DOE reference models, the IM multiplier of 8 reflects a typical office internal mass environment. It is recommended to use an IM multiplier of 3 to 6 for light offices, 6 – 10 for typical offices, and 10 – 15 for heavy mass office configurations. For the use of hybrid model when internal mass and infiltration inputs cannot be estimated, the default input of the IM multiplier of 8 is used to derive infiltration air flow rate. Once the hybrid simulation for the infiltration mode is done, the derived infiltration is used in the hybrid modeling internal mass mode to correct the IM multiplier. This newly calculated multiplier represents the real internal mass configuration.

(d) Simulation Process

The hybrid modeling feature uses a new flag, “Hybrid Modeling Flag” in the sizing and primary simulation in the simulation manager. “Hybrid Modeling Flag” is triggered by inputs in the new object, “HybridModel:Zone”. The flag triggers the hybrid model simulation that calculates the zone temperature capacitance multipliers or infiltration air flow rates depending on the user’s input in the object, and the  zone air temperature data input in the Schedule:File objects. The simulation steps will be as follows depending on the inputs of the HybridModel:Zone object.

When the “Calculate Zone Air Infiltration Rate” flag and the “Calculate Zone Internal Thermal Mass” flag are both set to YES:

1.	Hybrid model simulation for infiltration using the default IM multiplier of 8
2.	Hybrid model simulation for IM multiplier using the calculated infiltration output
3.	Normal energy simulation with the calculated IM multiplier and infiltration

When the “Calculate Zone Air Infiltration Rate” flag is set to YES and the “Calculate Zone Internal Thermal Mass” flag is set to NO:

1.	Hybrid model simulation for infiltration using user’s IM multiplier input
2.	Normal energy simulation with the calculated infiltration

When the “Calculate Zone Air Infiltration Rate” flag is set to NO and the “Calculate Zone Internal Thermal Mass” flag is set to YES:

1.	Hybrid model simulation for IM multiplier using user’s infiltration input
2.	Normal energy simulation with the calculated IM multiplier

## IDD Objects (New) ##

We propose to create a new object HybridModel:Zone. The new object defines inputs for the proposed hybrid modeling algorithms for individual zones.

```
HybridModel:Zone,
      \memo Zones with measured air temperature data and a range of dates.
      \memo A range of dates may differ from zones.
  A1, \field Name
      \required-field
      \type alpha
  A2, \field Zone or ZoneList Name
      \required-field
      \type object-list
      \object-list ZoneAndZoneListNames
  A3, \field Calculate Zone Internal Thermal Mass
      \note Use measured temperature data to calculate zone temperature capacity multiplier
	  \type choice
      \key NO
      \key YES
	  \default NO
  A4, \field Calculate Zone Air Infiltration Rate
      \note Use measured temperature data to calculate zone air infiltration air flow rate
	  \type choice
      \key NO
      \key YES
	  \default NO
  A5, \field Zone Measured Air Temperature Schedule Name
      \required-field
      \type object-list
      \object-list ScheduleNames
	  \note from Schedule:File
  N1, \field Begin Month
      \required-field
      \minimum 1
      \maximum 12
      \type integer
  N2, \field Begin Day of Month
      \required-field
      \minimum 1
      \maximum 31
      \type integer
  N3, \field End Month
      \required-field
      \minimum 1
      \maximum 12
      \type integer
  N4; \field End Day of Month
      \required-field
      \minimum 1
      \maximum 31
      \type integer
```

## IDD Objects (Revised) ##

There needs a revision to the existing ZoneCapacitanceMultiplier:ResearchSpecial object by adding Zone Name or Zone List Name field. This enables different multipliers for different zones.

```
ZoneCapacitanceMultiplier:ResearchSpecial,
      \format singleLine
      \memo Multiplier altering the relative capacitance of the air compared to an empty zone
      \min-fields 4
 A1,  \field Name
      \required-field
 A2,  \field Zone or ZoneList Name
      \required-field
      \type object-list
      \object-list ZoneAndZoneListNames
 N1,  \field Temperature Capacity Multiplier
      \type real
      \default 1.0
      \minimum> 0.0
      \note Used to alter the capacitance of zone air with respect to heat or temperature
 N2,  \field Humidity Capacity Multiplier
      \type real
      \default 1.0
      \minimum> 0.0
      \note Used to alter the capacitance of zone air with respect to moisture or humidity ratio
 N3,  \field Carbon Dioxide Capacity Multiplier
      \type real
      \default 1.0
      \minimum> 0.0
      \note Used to alter the capacitance of zone air with respect to zone air carbon dioxide concentration
 N4;  \field Generic Contaminant Capacity Multiplier
      \type real
      \default 1.0
      \minimum> 0.0
      \note Used to alter the capacitance of zone air with respect to zone air generic   contaminant concentration
```

## Outputs Description (new) ##

Variable: Zone Temperature Capacity Multiplier
Units: N/A
Variable reference: Zone( Loop ). HMMultiplierAverage
Index type key: Zone
Variable type key: Average
Keyed value: Zone( Loop ).Name

Variable: Infiltration Air Change per Hour
Units: N/A
Variable reference: Zone( Loop ). InfilOAAirChangeRateHM
Index type key: Zone
Variable type key: Average
Keyed value: Zone( Loop ).Name

## Input Output Reference Documentation ##

To be provided for the different mode of the hybrid modeling simulation.

## Testing/Validation/Data Sources ##

The testing and validation of the hybrid model were done using a custom version of EnergyPlus 8.5 in which the hybrid modeling features were implemented. Details of the testing and validation are available in the technical reports (Lee and Hong 2016; Lee and Hong 2015).

## Engineering Reference ##

To be provided based on the overview of the hybrid algorithm described above.

## Example File ##

HybridModel_5Zone.idf includes the new HybridModel:Zone object and the modified ZoneCapacitanceMultiplier:ResearchSpecial object. HybridModel_5Zone_MeasuredAirTemperatureData.csv includes zone air temperatures.

## E-mail and  Conference Call Conclusions ##

**Mike Witte, May 29, 2015**

Interesting concept.  Please make the title more specific, like "Hybrid Zone Capacitance Model".  From the initial title I assumed it was going to be about some sort of HVAC equipment.

- Reply:
	The title was changed to “Hybrid Model: Zone Capacitance and Infiltration”.

Perhaps I missed something in the first read through, but it seems the capacitance term could vary wildly over the course of the simulation even though that is a somewhat-fixed property (albeit unknown) of the measured building.  I guess the intent is that this feature is used as a preprocessing step and then you would use the output to determine a fixed value for a given zone's Temperature Capacity Multiplier?
Reply: Yes. This feature is a preprocessing step to determine the temperature capacitance multipliers and infiltration flow rate based on user’s measured temperature inputs.

By adding a new group "HybridModel:" this implies you envision other types of hybrid models?  I'm not convinced that Hybrid is the right term for this, especially since we already use "Hybrid" in the AvailabilityManager:HybridVentilation to mean something different. This adds to an evergrowing collection of special objects that control various aspects of the simulation, so I'm concerned about the proliferation of these.

ShadowCalculation
SurfaceConvectionAlgorithm:Inside
SurfaceConvectionAlgorithm:Outside
HeatBalanceAlgorithm
HeatBalanceSettings:ConductionFiniteDifference
ZoneAirHeatBalanceAlgorithm
ZoneAirContaminantBalance
ZoneAirMassFlowConservation
ZoneCapacitanceMultiplier:ResearchSpecial
Timestep
ConvergenceLimits

- Reply:
	Yes. We propose the new group based on assumption of more hybrid models in the future. For example, there can be a new chiller object using a hybrid model that combines a physics model with measurement data.

	For the current NFP, We only work with a multiplier for zone volume temperature capacitance multiplier, ZoneVolCapMultpSens. This is linked to the following EnegryPlus source code modules:

	DataHeatBalance
	DisplacementVentMgr
	HeatBalanceManager
	UFADManager
	ZoneTempPredictorCorrector

Speaking of these, is this hybrid model impacted by the ZoneAirHeatBalanceAlgorithm and ZoneAirMassFlowConservation options?

- Reply:
	It is only impacted by the ZoneAirHeatBalanceAlgorithm option.

Also, will it work with a RoomAirModel other than the well-stirred "Mixing" model?
Reply: No. The hybrid model assumes a well-mixed room air model, which has a single zone air temperature for the entire space.

I understand the proposed approach of a field in the Zone object to override the global setting of the Temperature Capacity Multiplier which follows the pattern used for convection algorithms.  But perhaps it would be better to change ZoneCapacitanceMultiplier:ReasearchSpecial to add a Zone or ZoneList Name field (which could apply to all zones if left blank).  Not sure what's better in the long run.  The DesignBuilder group has previously requested making this object be zone-by-zone.

- Reply:
	Yes. We added the Zone Name or Zone List Name to the ZoneCapacitanceMultiplier:ReasearchSpecial object to enable different multipliers for different zones.

**Lixing Gu, May 29, 2015**

I agree with Mike's comments. I think to avoid using Hybrid. Instead, I prefer to use "Effective Zone Capacitance Model". The main purpose of this NFP is to use the capacity modifier to catch unknown internal mass impact.

- Reply:
	The title was changed accordingly. See previous comments.

I have a couple of concerns. Do you use a fixed modifier? I see several conditions (system on and off) to calculate the modifier. When different values are obtained, which value do you select? If variable modifier is used in different time steps, it may not fit a physical model.
Reply: Yes. We use an average multiplier. The multiplier calculations are only done meeting the conditions described above to avoid the anomaly conditions due to the use of the inverse model. The calculated internal mass multiplier is the average value for those filtered timesteps.

In addition, moisture modifier is more important than temperature modifier, because moisture modifier can catch more unknowns, which may not be measurable. Do you plan to propose it? Some people, include us, use moisture modifier to catch convective moisture transfer between surfaces and zone air, if a tool is unable to simulate convective moisture transfer, such as DOE-2.

- Reply:
	No. The current hybrid modeling of internal mass is limited to the temperature modifier.

**Edwin, June 26, 2015**

Internal mass objects are participating in the zone air heat balance and the long wave radiant exchange.  However, they don’t directly interact with the solar heat because internal mass objects don’t have a specific location in space.  So the inclination is to just use regular ‘surfaces’ placed in the space so that they absorb solar.  However, imagine putting a “table” in a space with a window.  Because the solar cannot see the floor, the zone is no longer convex by the strictest definition, so solar calculations will be wrong, and the solar will actually hit both the table and the floor underneath.  Which unrealistically makes the zone cooling load higher than possible.  So, the key is a better solar model that properly clips for internal surfaces, or at least a better solar distribution model that puts some solar directly on internal mass objects in the space.

- Reply:
	From this feature development, the internal mass is represented in the zone temperature capacitance multiplier, which only corrects the zone air heat capacity reflecting heat stored in the internal mass. Assumptions are not different from the approach used in InternalMass object, which ignores the geometrical construction of the internal mass, and do not contribute to the heat transfer across surfaces and the solar heat gain through windows.

**Lixing Gu, Sep 29, 2016**

I need some clarifications from Simulation Process:

When the “Calculate Zone Air Infiltration Rate” flag and the “Calculate Zone Internal Thermal Mass” flag are both set to YES:

1. Hybrid model simulation for infiltration using the default IM multiplier of 8
2. Hybrid model simulation for IM multiplier using the calculated infiltration output
3. Normal energy simulation with the calculated IM multiplier and infiltration

Do you have any iteration to recalculate infiltration after getting IM multiplier in Step 2, if Step 2 gets IM Multiplier much beyond 8?

- Reply:

It does not require the iteration process to recalulate infiltration. The sensitivity study from the validation report shows that infiltration is not sensitive to multipliers between 1 and 20. The base IM multiplier 8 is selected as it reflects a typical office internal mass environment. Although the calculated multiplier might differ from 8 at Step 2, the result from Step 2 is the IM multiplier to be used for energy simulation.

When Step 3 is performed, which algorithm in ZoneAirHeatBalanceAlgorithm should be selected? Since you use both 3rdOrderBackwardDifference and AnalyticalSolution in Step 1 and 2, do you allow users to select EulerMethod?

- Reply:

After all hybrid simulations are done, the energy simulation does not limit the type of ZoneAirHeatBalanceAlgorithm method.

**Mike Witte, Oct 06, 2016**

1.  I should be careful what I ask for.  You took my suggestion to add a Zone or ZoneList Name field to ZoneCapacitanceMultiplier:ResearchSpecial, and it is proposed as a required field.  To follow the pattern of other objects that apply to a zone or zonelist, there should also be a name field (e.g. People, or ZoneControl:Thermostat).  So, this would become:
ZoneCapacitanceMultiplier:ResearchSpecial,
      \format singleLine
      \memo Multiplier altering the relative capacitance of the air compared to an empty zone
      \min-fields 4
 A1,  \field Name
      \required-field
  A2, \field Zone or ZoneList Name
      \required-field
      \type object-list
      \object-list ZoneAndZoneListNames
 N1,  \field Temperature Capacity Multiplier
      \type real
      \default 1.0
      \minimum> 0.0

- Reply:

We agree there should be a new field Name in the object of ZoneCapacitanceMultiplier:ResearchSpecial, to be consistent with other similar objects that apply to a zone or zonelist.

2.  If ZoneAirMassFlowConservation is used in the same simulations as HybridModel:Zone, then they cannot both actively adjust infiltration.  This will need an error check to see what the infiltration setting is for both of these objects.

- Reply:

The ZoneAirMassFlowConservation should not be activated during the Hybrid Modeling simulations. We will describe this clearly in the Engineering Reference. If the IDF file includes both ZoneAirMassFlowConservation and HybridModeling, we would deactivate the ZoneAirMassFlowConservation and provide a warning message to the user.

3.  If you have not done so, be sure to check the RoomAirModelType for any hybrid zones and throw and error if it is set to anything other than "Mixing".

- Reply:

The model will check the RoomAirModelType for each zone and enforce it to be "Mixing" if Hybrid Modeling is performed for that zone. A warning message will be provided if the RoomAirModelType is changed during the process.

## Conference Call Conclusions ##

Revise original NFP to address reviewers’ comments.

## Other Conference Call Topics (not in scope of current proposal) ##

N/A

## Transition changes ##

Rules will be provided for the modified ZoneCapacitanceMultiplier:ResearchSpecial object.

## Other documents ##

N/A

## References ##

Lee, Sang Hoon, and Tianzhen Hong. 2015. Validation Report for the Hybrid Modeling Approach Part 1: Internal Thermal Mass.

Lee, Sang Hoon, and Tianzhen Hong. 2016. Validation of the Hybrid Modeling Using Measured Data from the FLEXLAB Experiment.
