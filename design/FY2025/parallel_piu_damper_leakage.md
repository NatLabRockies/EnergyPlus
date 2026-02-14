Air Leakage in Parallel Fan-Powered Terminal Units
================

**Jeremy Lerond, Pacific Northwest National Laboratory**

 - Original Date: 04/07/2025
 - Revision Date: 10/10/2025


## Justification for New Feature ##

- Parallel PIU can be perceived as more efficient than series PIU because they don't run as much
- Laboratory and in-situ study have shown that their performance can be impacted by backdraft damper air leakage
- EnergyPlus doesn’t have the capability to model parallel PIU damper leakage which could potentially provide misleading results when comparing designs with both types of air terminals

## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

With a new input, users will be able to simulate the impact of parallel PIU backdraft damper leakage. This approach will consider the two main impacts of air leakage through backdraft dampers: increased primary air flow rate and heat/mass transfer to the zone where the air leaks.

## Approach ##

Assumptions:
- Leakage only occurs when the terminal fan is off
- Leaked air is determined from the leakage fraction
- The damper leakage fraction is expressed as follows: `f_leak = m_dot_leakage / m_dot_primary`
- To meet zone loads, leakage results in an increase in primary air flow rate
- The air escaping from the terminal impacts the heat balance of the thermal zone designated as the destination for the leaks

## Testing/Validation/Data Sources ##

- Simulation using sample files will be run and results will be compared against (Sardoueinasab et al., 2018)
- Unit tests will be added to verify that EnergyPlus takes leakage into account when requested by a user

## Input Output Reference Documentation ##

The following descriptions will be added to the Input Output Reference manual:

```latex
\paragraph{Field: Backdraft Damper Leakage Curve Name}\label{field-backdraft_damper_leakage_fraction_curve_name}

This field is used to indicate how the backdraft damper leakage fraction changes with different primary air flow fractions. The leakage fraction is defined as the ratio of leakage mass flow rate to primary air mass flow rate at a constant supply static pressure setpoint. This curve should describe the leakage fraction as a function of primary air flow fraction (ratio of primary flow fraction to maximum nominal primary air flow rate). For a static pressure setpoint of 62 Pa (0.25 in w.c) a resonable low leakage assumption could be around 3\%, a medium leakage value could be around 5\%, and relatively high leakage could be up to 12\%.

\paragraph{Field: Backdraft Damper Leakage Zone Name}\label{field-backdraft_damper_leakage_zone_name}

This field is used to indicate what zone will be impacted by the air leaking from the backdraft damper of the terminal. The zone should be different than the one served by the terminal.
```

## Input Description ##

The following inputs will be added at the end of `AirTerminal:SingleDuct:ParallelPIU:Reheat`:

```
  A13, \field Backdraft Damper Leakage Fraction Curve Name
       \type object-list
       \object-list UnivariateFunctions
       \note Backdraft damper leakage fraction is the ratio of mass leakage flow rate to primary air flow rate
       \note at a constant static pressure setpoint. This curve should describe the ratio as a function primary
       \note air flow fraction (ratio of primary flow fraction to maximum nominal primary air flow rate).
  A14; \field Backdraft Damper Leakage Zone Name
       \note Name of a zone that will be impacted by the backdraft damper leakage.
       \type object-list
       \object-list ZoneNames
```

## Outputs Description ##

A new output will be added:
```
Output:Variable,*,Zone Air Terminal Backdraft Damper Leakage Mass Flow Rate,hourly; !- HVAC Average [kg/s]
```

## Engineering Reference ##

A new section will be added under the "Zone Equipment and Zone Forced Air Units" to document how leakage for parallel fan-powered terminal is simulated in EnergyPlus.

```latex
\subsection{Air Leakage in Parallel Fan-Powered Terminal Units}\label{parallel_piu_leakage}
As described in (O'Neal et al., 2016) and (Sardoueinasab et al., 2018), backdraft damper leakage in parallel fan-powered terminal units can be characterized using a leakage fraction at a constant static pressure setpoint:

\begin{equation}
{f_{leakage}} = \frac{\dot m_{leakage}}{\dot m_{primary}}
\end{equation}

Leakage is only simulated when the terminal fan is off. The impact of the simulated leakage is twofold:
\begin{enumerate}
    \item To make up for the leaked air, the primary air flow rate should be increased to meet the zone load
    \item The air escaping from the terminal impacts the heat balance of the thermal zone designated as the destination for the leaks
\end{enumerate}

The new primary flow rate is adjusted as follows:
\begin{equation}
{\dot m_{primary, adj}} = {\dot m_{primary}} \times \frac{1}{1 - {f_{leakage}}}
\end{equation}

The new primary flow rate won't exceed the maximum primary flow rate of the terminal.

Since leakage only occurs during dead-band and cooling operation (i.e., not during heating operation), the new primary air flow rate doesn't have an impact on the terminal mixer and heater. Leaks are accounted for after the terminal has been simulated, outlet nodes are updated then. The last step is to account for the thermal impact of the leakage on the thermal zone designated as the destination for the leaks. When an `AirLoopHVAC:ReturnPlenum`, this is handled in a similar fashion as leakage simulation using the Simplified Duct Leakage Model, see \ref{implementation-000}. Since users can select the thermal zone designated as the destination for the leaks, this feature also supports models that use an AirLoopHVAC:ZoneMixer as the return path. The leakage flow rate (at the conditions of each destination zone) is added to the mixer to calculate the new mixed air conditions.

\subsubsection{References}\label{references_parallel_piu_leakage}
D.L. O'Neal, J.L. Edmondson, Characterizing air leakage in parallel fan-powered terminal units, ASHRAE Trans., 122 (1) (2016), pp. 343-353

Zahra Sardoueinasab, Peng Yin, Dennis O'Neal, Energy modeling and analysis of inherent air leakage from parallel fan-powered terminal units using EMS in EnergyPlus, (2018), Energy and Buildings, https://doi.org/10.1016/j.enbuild.2018.07.019
```

## Example File and Transition Changes ##

- No transition rules are required
- A new example file will be added to showcase this new feature The new file will include example curves to model leakage based on data included in (O'neal et al., 2016):

```
Curve:Linear,
    low_0p25_in_wc,          !- Name
    -.006083039,             !- Coefficient1 Constant
    0.036612602,             !- Coefficient2 x
    0.63,                    !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

Curve:Linear,
    medium_0p25_in_wc,       !- Name
    0.039928941,             !- Coefficient1 Constant
    0.009555789,             !- Coefficient2 x
    0.63,                    !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

Curve:Linear,
    high_0p25_in_wc,         !- Name
    0.117130811,             !- Coefficient1 Constant
    -.001792284,             !- Coefficient2 x
    0.63,                    !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

Curve:Linear,
    low_0p50_in_wc,          !- Name
    -.023960304,             !- Coefficient1 Constant
    0.078345929,             !- Coefficient2 x
    0.51,                    !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

Curve:Linear,
    medium_0p50_in_wc,       !- Name
    0.027345369,             !- Coefficient1 Constant
    0.04614075,              !- Coefficient2 x
    0.5,                     !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

Curve:Linear,
    high_0p50_in_wc,         !- Name
    0.094531603,             !- Coefficient1 Constant
    0.07094517,              !- Coefficient2 x
    0.5,                     !- Minimum Value of x
    1,                       !- Maximum Value of x
    ,                        !- Minimum Curve Output
    ,                        !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type
```

## References ##

- Energy modeling and analysis of inherent air leakage from parallel fan-powered terminal units using EMS in EnergyPlus, Zahra Sardoueinasab, Peng Yin, Dennis O'Neal, (2018), Energy and Buildings, https://doi.org/10.1016/j.enbuild.2018.07.019
- Characterizing air leakage in parallel fan-powered terminal units, D.L. O'Neal, J.L. Edmondson, ASHRAE Trans., 122 (1) (2016), pp. 343-353
