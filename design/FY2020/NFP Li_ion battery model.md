# Addition of Lithium-ion Battery Chemistry to the Energy Storage Module

**Rohit Chintala**<br>
**Ben Polly**<br>
**Noel Merket**

Original Date: July 2019<br>
Updated: August 2020

## Introduction

The increasing need for decarbonization has resulted in a proliferation of buildings that have incorporated decentralized power production such as photovoltaic (PV) systems to meet their energy needs. Energy storage is a crucial element to consider for such buildings as storage makes it possible to balance energy generation and consumption, in addition to improving the interaction with the electricity grid. An accurate energy storage model thus becomes essential for several research and development activities pertaining to these buildings such as battery design and capacity, sizing of PV systems, building operation control strategies, load flexibility analysis, etc. [1].  EnergyPlus can serve as a valuable tool in this regard. However, there are key limitations in the current electrical storage model which need to be addressed. This document proposes a new feature that would enhance the capabilities of the storage model by improving its accuracy, and furthermore would make it applicable to a wider range of battery chemistries.

## Justification for New Feature Proposal
From among the battery chemistries, Lithium-ion is becoming increasingly popular for use in storing renewable energy. Although, the initial costs are higher, lithium-ion batteries outperform competing technologies such as lead (Pb)-acid in terms of energy delivered and specific power. Furthermore, lithium-ion batteries may be more economical in the long run as they have longer cycle life and lower life cycle costs [2]. EnergyPlus doesn't have a storage model that can accurately simulate Li-ion battery chemistry. Having accurate models that predict various aggregate properties of the battery state allows energy modelers to make more informed decisions about selecting the right storage size and chemistry for the building under consideration. A brief summary of the limitation that exist in the current energy storage model and a description of the proposed model developed by SAM [3] is provided below.

### Constant Charging and Discharging Efficiencies
The electrical storage module in EnergyPlus currently has two models – a Simple Energy Balance model and a Kinetic Battery model. The simple energy model treats the battery as a black-box and counts the energy that is added or removed from it using constant charging and discharging efficiencies. The state of charge $$(Q_{stor})$$ at time step $$t+\Delta t$$ can be written as follows:
$$Q_{stor}^{t+\Delta t} =  Q_{stor}^t+P_{chg}\cdot \epsilon_{chg} \cdot \Delta t$$
where $$P_{chg}$$ is the charging power, and $$\epsilon_{chg}$$ is the charging efficiency. In a more accurate model of a Li-ion battery, however, the charging efficiency is not constant but is dependent on its current state of charge and terminal voltage. Not accurately modeling the battery states or efficiencies results in discrepancies while computing the battery charging and discharging power.  A comparison of the charging power required to charge the battery at a constant rate as computed by the simple energy balance model, and the proposed SAM model is shown in Figure 1.
![](Li_ion_battery_model_images/Simple_vs_SAM.png)
Figure 1: Charging power comparison between Simple Energy Model and SAM at constant C-rate.


### Kinetic Battery Model is not Suited for Li-ion
There are certain characteristics that are exhibited by batteries with Pb-acid chemistries that make them significantly different than others. One such property which makes them unique is the fact that a fraction of the total charge remains chemically bounded at any time. The bound charge must become available before it can be used [3]. The kinetic battery model which is employed by the EnergyPlus storage module assumes that the charge stored in the battery is divided into two tanks, an available charge tank and a bound-charge tank. Not all the charge that is currently stored in the battery is directly available to the load. The portion of the charge that is present in the bound-charge tank can supply electrons only to the available charge tank.
The kinetic battery model described above is more suitable for battery chemistries such as Pb-acid with slow chemical kinetics. Li-ion have batteries have much faster dynamics enabling them to be charged and discharged much more rapidly. The proposed model treats the battery as a single tank of charge, thereby representing Li-ion chemistry more accurately.

### Battery Thermal Model

The electrical storage module in EnergyPlus doesn't model the battery temperature. The thermal model captures the impact of the ambient conditions not only on the battery temperature, but also the amount of heat dissipated by the battery to the surroundings. Furthermore, battery temperature has a big impact on the charge capacity and the calendar degradation. The lifecylce of a battery can be maximized by maintaining the battery temperature at room temperature. There is a significant reduction in the battery lifetime when the temperature instead is much higher or lower than the room temperature. For example, Figure 2  below shows how the  capacity degradation of a typical Li-ion cell under different ambient temperatures. The figures were generated using the Li-ion capacity degradation model developed by Smith et al. in [4]. In order to determine the best dispatch strategy that also maximizes the battery's lifespan, an accurate model of both the capacity and cycle degradation is required. But as is evident from the figure, not modeling the thermal effects causes significant discrepancies in the estimation of these battery states.
![](Li_ion_battery_model_images/battery_temp_degradation.png)
Figure 2: Charge degradation at various ambient temperatures.

### Calendar and Cycle Degradation
There are two main mechanisms by which the battery capacity is reduced over its lifetime as listed below.
* Loss in cyclable Lithium due to the development of a solid-electrolyte interface (SEI) with time
* Mechanical damage to the negative electrode due to cycling

The actual battery capacity can be modeled as the limiting value of these competing mechanisms [5] as shown in the equation below
$$Q_{max} = min(Q_{max}^{Li},Q_{max}^{neg})$$
where $$Q_{max}$$ is the modeled battery capacity, and $$Q_{max}^{Li}$$ and $$Q_{max}^{neg}$$ are the capacities modeled due to the two mechanisms listed above. Each of the two mechanisms depend on several factors such as Depth of Discharge (DoD), number of cycles, battery temperature, calendar time, State of Charge (SoC) etc. The current cycle life model in EnergyPlus, however, does not take into consideration the different mechanisms by which the battery capacity fades.

## Method and Approach

This section briefly describes the theory of the proposed battery model, and the programming approach that can be used to implement it in EnergyPlus.

### Battery Performance and Lifetime Model
The battery performance model is used to describe its aggregate properties which include terminal voltage, state of charge, and temperature at every time step of the simulation. The parameters of the model can be extracted from battery data-sheets. The terminal voltage $V_{term}$ is expressed as shown in the equation below.
$$V_{term} = V_0 - I_{bat}\cdot R - K(Q_{max}/Q - \int I_{bat}\cdot dt ) +a\cdot e^{-BI_{bat}dt}$$
A description of the variables, and the constants and their values corresponding to a Li-ion cell in the equation above are shown in Tables 1 and 2.

Table 1: Description of variables in the terminal voltage equation.
|  Parameter (symbol) | Value |
| ------ | ------ |
|  $$V_{term} (V)$$ | Terminal Voltage |
| $$Q_{max} (Ah)$$ | Battery Capacity |
|  $$Q (Ah)$$  | Battery Charge|
| $$I_{bat}(A)$$ | Battery current |
| $$dt (hr)$$ | Time Step |

Table 2: Parameters of the terminal voltage equation.
|  Parameter (symbol) | Description |Value|
| ------ | ------ |------ |
|  $$V_0$$ |Open Circuit Voltage |3.7348|
| $$R(\omega)$$ | Internal Resistance |0.09|
|  $$K(V)$$  | Polarization Voltage|0.00876|
| $$a(V)$$ | Exponential Zone Amplitude |0.468|
| $$B(Ah)^{-1}$$ | Exponential Zone Time Constant Inverse |3.5294|

The state of charge of the battery $Q_{bat}$ at time step $$k$$ is updated using the equation below
$$Q_{bat}(k) = Q_{bat}(k-1) + \left((I_{bat}\cdot V_{term})\cdot (1/1000)/E_{max} \right)$$
where $$E_{max}$$ is the maximum capacity of the battery in $$kWh$$.

The thermal model is generated by using an energy balance which takes into consideration the heat generated due to the internal resistance, the heat transferred to and from the surrounding, and the thermal storage of the battery and the casing. The output of interest is the battery temperature $$T_{bat}$$ and the equation governing its evolution with time is expressed in the equation below
$$dT_{bat}/dt = (hA(T_{out} - T_{bat}) + I_{bat}^2R)/(m\cdot C_p)$$
where $$T_{out}$$ is the ambient temperature, and $m$ and $C_p$ are the mass and specific heat capacity of the battery.

The lifetime model is adopted from the paper by Smith et.al [4]. The model predicts the battery capacity degradation with time and use. The maximum capacity is computed by taking into consideration two mechanisms as was described in the previous section, the loss of cyclable Lithium due to formation of SEI $$(Q_{max}^{Li}$$, and the mechanical damage of the negative electrode due to the charge/discharge cycles $$(Q_{max}^{neg})$$. The battery capacity due to each of the mechanisms is shown in the equations below.
$$Q_{max}^{Li} = d_0 \left[b_0 - b_1t^{1/2} - b_2N - b_3(1-exp(-t/\tau_{b_3})\right]$$

$$Q_{max}^{neg} = d_0 \left[c_0^2 - 2c_2c_0N\right]^{1/2}$$
The parameters $d_0,b_1, b_2, b_3, c_0, c_2$ depend on several factors such as depth of discharge, state of charge, number of cycles etc. These relationships were determined by Smith et.al in [4]. The capacity at any point in time is the limiting value of the estimates from the two mechanisms as shown in the equation below.
$$Q_{max} = min(Q_{max}^{Li}, Q_{max}^{neg})$$

### Programming Approach
The System Advisor Model (SAM) already has a battery module based on the models described in the previous section. In order to incorporate this  module in EnergyPlus, the battery model in SAM and all its dependencies can be imported into the third party library in EnergyPlus. The battery model and its dependencies are contained in two folders of the SAM repository, the 'ssc' folder and the 'shared' folder. The link to the github repository of the SAM model is below.

https://github.com/NatLabRockies/ssc

The following libraries from the SAM repository would be imported into EnergyPlus.
1.	cmod_battery.h
2.	common.h
3.	core.h
4.	lib_battery.h
5.	lib_battery_dispatch.h
6.	lib_battery_powerflow.h
7.	lib_power_electronics.h
8.	lib_resilience.h
9.	lib_shared_inverter.h
10.	lib_time.h
11.	lib_util.h
12.	lib_utility_rate.h
13.	lib_weatherfile.h
14.	6par_newton.h

A new class lifetime_calendar_t::runLithiumIonLifetimeModel has been programmed in the lib_battery module of SAM to model the capacity degradation of a lithium-ion battery based on the model described above by Smith et.al. [4]. The model outputs the updated capacity of the battery at every timestep taking into consideration the cycling history, state of charge, depth of discharge, and the ambient temperature to which the battery is exposed.

The electrical storage model in EnergyPlus is present in the file ElectricPowerServiceManager.cc. The storage model in this file currently has just two cases corresponding to the simple battery model (simpleBucketStorage) and the Kinetic Battery Model (kiBaMBattery). By importing the aforementioned libraries from SAM, a more detailed and accurate capacity degaradation model for Lithium ion can be incorporated into EnergyPlus.

## Inputs and Outputs
The inputs and outputs that would be required to incorporate the Li-ion battery performance and lifetime degradation models are shown below.

### Inputs

* Field: Name
*  Availability schedule name
*  Zone name
*  Mass
*  Effective specific heat capacity
*  Effective conduction coefficient
*  Number of battery modules in series
*  Number of battery modules in parallel
*  Initial fractional state of charge
* Module cut-off voltage
*  Battery nominal capacity
*  Battery usable capacity
*  Real power max continuous
*  Real power peak charging
*  Real power peak discharging
*  Nominal voltage

### Outputs

* Electric storage charge state
*  Electric storage charge fraction
*  Electric storage charge power
*  Electric storage charge energy
*  Electric storage discharge power
*  Electric storage discharge energy
*  Electric storage total current
*  Electric storage total voltage
*  Electric storage thermal loss rate
*  Electric storage Thermal loss energy
*  Electric storage temperature
* Electric storage degradation fraction
*  Electric storage charge capacity
*  Electric storage internal resistance

## References

- [1] Berrueta, Alberto, et al. "A comprehensive model for lithium-ion batteries: From the physical principles to an electrical model." Energy 144 (2018): 286-300.
- [2] Diouf, Boucar, and Ramchandra Pode. "Potential of lithium-ion batteries in renewable energy." Renewable Energy 76 (2015): 375-380
- [3] DiOrio, Nicholas, et al. Technoeconomic modeling of battery energy storage in SAM. No. NREL/TP-6A20-64641. National Renewable Energy Lab.(NREL), Golden, CO (United States), 2015.
- [4] Smith, Kandler, et al. "Life prediction model for grid-connected Li-ion battery energy storage system." 2017 American Control Conference (ACC). IEEE, 2017
- [5] Santhanagopalan, Shriram, et al. Design and analysis of large lithium-ion battery systems. Artech House, 2014.
