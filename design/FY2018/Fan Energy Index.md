# Fan Energy Index

Tianzhen Hong, Kaiyu Sun, Xuan Luo

Lawrence Berkeley National Laboratory

Original: March 24, 2018

Revised: May 12, 2018

Final: June 15, 2018

## Justification for Feature Update
Fan Energy Index (FEI) is an energy-efficiency metric for fans including their motors and drives. This metric provides a standardized and consistent basis for comparing fan energy performance across fan types and sizes at a given fan operating point. Fan specifiers can use it to understand and communicate fan-efficiency design intent, while legislative and regulatory bodies can use it to define energy-efficiency requirements of fans.

ANSI/AMCA Standard 208-18 defines the calculation method for FEI. The standard has been written to support the adoption of energy efficient fans in the energy and construction codes and standards (e.g., ASHRAE Standards 90.1, 189.1, and California Appliances Standards Title 20) and to support utility rebate programs. FEI is a replacement for fan efficiency grade (FEG), a metric currently referenced in model energy codes and standards such as ASHRAE 90.1, ASHRAE 189.1, and IECC and in federal regulations in several Asian countries.

Implementing FEI, based on the ANSI/AMCA standards 208-18 and 207-17, in EnergyPlus will enable its use to support: (1) building energy codes and standards, (2) mechanical engineers to evaluate and select energy efficient fans for various applications, and (3) utilities incentives/rebates programs for fans.

## Fan Energy Index and Calculations
The following algorithms and calculation procedures (the SI unit version) are reproduced from the ANSI/AMCA standards 208-18: Calculation of the Fan Energy Index, and 207-17: Fan System Efficiency and Fan System Input Power Calculation.
The fan energy index (FEI) is defined as a ratio of the electrical input power of a reference fan to the electrical input power of the actual fan for which the FEI is calculated, both calculated at the same duty point, i, which is characterized by a value of airflow (Q<sub>i</sub>) and pressure (P<sub>t,i</sub> or P<sub>s,i</sub>). FEI can be calculated for each point on a fan curve.

$$FEI_{t,i}\;or\;FEI_{s,i}=\frac{Reference Fan Electrical Input Power}{Actual Fan Electrical Input Power} = \frac{FEP_{ref,i}}{FEP_{act,i}}$$ (1)

Where,

FEI<sub>t,i</sub>: FEI based on fan total pressure

FEI<sub>s,i</sub>: FEI based on fan static pressure

We propose to report the design point FEI of the current fan models, including **Fan:SystemModel**, **Fan:ConstantVolume**,**Fan:OnOff**, **Fan:VariableVolume**, and **FanPerformance:NightVentilation**. We will use the user defined Fan objects to simulate the actual fan electrical input power FEP<sub>act,i</sub> at the design operating point. We will calculate the reference fan electrical input power FEP<sub>ref,i</sub> using the following method.

Reference fan shaft power (H<sub>i,ref</sub>) is calculated on a fan total pressure basis or a static pressure basis, depending on the category of the reference fan to calculate FEI. However, in EnergyPlus **Fan:SystemModel**, only total pressure is defined. So the FEI is reportable only for limited category of fans that are calculated based on the fan total pressure, namely Centrifugal Housed, Centrifugal Inline, Centrifugal PRV Supply, Axial Inline, Laboratory Exhaust, Jet Fan and Circulating

The reference fan concept is used to normalize the FEI calculation to a consistent power level independent of fan type, fan drive components or any regulatory requirements. The reference fan electrical input power is a function of airflow and fan pressure. The reference fan is defined as one that requires a certain reference fan shaft power, uses a V-belt drive, has a motor efficiency based on the IE3 level for a four-pole 60 Hz motor and does not have a speed control.

$$FEP_{ref,i}=H_{i,ref}(\frac{1}{\eta_{trans,ref}} )(\frac{1}{\eta_{mtr,ref}})(\frac{1}{\eta_{ctrl,ref}}) $$ (2)

Where,

H<sub>i,ref</sub>: Reference fan shaft power in kW

η<sub>trans,ref</sub>: Reference fan transmission efficiency

η<sub>mtr,ref</sub>: Reference fan motor efficiency

η<sub>ctrl,ref</sub>: Reference fan motor controller efficiency

#### 1. Reference fan shaft power
Reference fan shaft power (H<sub>i,ref</sub>) is calculated on a fan total pressure basis:

$$H_{i,ref}=\frac{(Q_i + Q_0)(P_{t,i} + P_0 \times \frac{\rho}{\rho_{std}} )}{1000 \times \eta_0}$$ (3)

Where,

Q<sub>i</sub>: Fan air flow in m<sup>3</sup>/s

P<sub>t,i</sub>: Fan total pressure in Pa

ρ: Air density in kg/m<sup>3</sup>

ρ<sub>std</sub>: Standard air density, 1.2 kg/m<sup>3</sup>

Q<sub>0</sub>: 0.118 m<sup>3</sup>/s

P<sub>0</sub>: 100 Pa

η<sub>0</sub>: 66%


#### 2. Reference fan transmission efficiency
For consistency, the reference fan is defined as one having a V-belt drive transmission, regardless of the drive arrangement of the actual fan for which the FEI is calculated. The reference fan transmission efficiency is calculated using the same equations as found in ANSI/AMCA Standard 207 for V-belt drives:

$$\eta_{trans,ref}=0.96(\frac{H_{i,ref}}{H_{i,ref}+1.64})^{.05}$$  (4)

#### 3. Reference fan motor efficiency
The reference fan is defined as having a motor efficiency based on the IE3 level for a four-pole 60 Hz motor. In order to simplify the calculation of part load efficiency for this reference fan motor and to avoid sizing and otherwise identifying a specific motor for this reference fan, a curve fit is used through the IE3 motor efficiency requirements. The result is a reference motor efficiency that varies continuously based on the required motor output power.

Reference fan motor output power:

$$H_{t,ref}=\frac{H_{i,ref}}{\eta_{trans,ref}} $$ (5)

The reference fan motor efficiency is calculated according to Equation (7) using the coefficients A–E found in Table 1:

$$\eta_{mtr,ref} = A\cdot[log_{10} (H_{t,ref})]^4 + B\cdot[log_{10} (H_{t,ref})]^3 +C\cdot[log_{10} (H_{t,ref})]^2 +D\cdot[log_{10} (H_{t,ref})]^1 + E $$(6)

Table 1 Reference Motor Efficiency Coefficients

Type |H<sub>t,ref</sub><185 kW (<250 BHP)  | H<sub>t,ref</sub>≥185 kW (≥250 BHP)
-------------  | -------------  | -------------
A  | -0.003812 | 0
B  | 0.025834  | 0
C  | -0.072577 | 0
D  | 0.125559  | 0
E  | 0.850274  | 0.962

#### 4. Reference fan motor controller efficiency
The reference fan is defined as a constant speed fan. Therefore, the motor controller efficiency is 100%.
$$\eta_{ctrl,ref} = 1 $$ (7)


## Approach

The FEI calculation results will be added to the existing summary report for Equipment Summary | Fans, as shown in Table 2.

Table 2 Fan summary report in EnergyPlus

Case | Type | Total Efficiency [W/W] | Delta Pressure [pa] | Max Air Flow Rate [m<sup>3</sup>/s] | Rated Electric Power [W] | Rated Power Per Max Air Flow Rate [W-s/m3] | Fan Energy Index | End Use
------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | -------------
VAV_1_FAN | Fan:SystemModel | 0.60 | 1017.59 | 15.49 | 26070.79 | 1683.36 |1.00 | Fan Energy
VAV_2_FAN | Fan:SystemModel | 0.62 | 1017.59 | 164.34 | 270814.15 | 1647.92 | 1.00 | Fan Energy

The FEI will also be added to the EIO file as a report variable.

## Testing/Validation/Data Source(s):
EnergyPlus simulation results will be manually checked and compared with those from the AMCA FEI calculation spreadsheet using the same input in the EnergyPlus model.

## IDD Object (New):
N/A

## IDD Object(s) (Revised):
N/A

## Proposed additions to Meters:
N/A

## Proposed Report Variables:
A new report variable, Fan Energy Index, will be added.

## IO Ref (draft):
To be developed.

## EngRef (draft):
To be developed.

## Example File:
An example file will be developed to demonstrate the use of the new feature.

## Team Discussions so far
The team provided suggestions on the simplification of the fields required for the FEI calculation. We discussed with Mike Witte and Neal Kruis via a phone call to address the reviewers’ comments. We agreed to (1) use EnergyPlus simulated fan design point performance as the actual fan electrical input power and (2) use the new **Fan:SystemModel** for report.

## Email Discussions so far
1. Q: What types of operating point(s) are needed to calculate and report FEI?

There are three choices: only the design operating point, only the annual weighted average, and both. In EnergyPlus, the FEI will be calculated at the system time-step (e.g., from 15 to 1 minute).

Craig Wray, March 27
>The operating point(s) used to calculate and report FEI needs to come from either user specifications for pressure and flow at the desired point(s), or from a system curve input. AMCA will need to decide whether reporting an average FEI is meaningful or whether FEI at specific operating point(s) is better (e.g., to show that FEIs are within a desired range – perhaps minimums and maximums). I suspect that the latter is the better choice, but both have some merit (especially if averaging is weighted by airflow part-load ratios)

Michael Ivanovich, April 18
> One of the selling points of FEI is that it is an application-style metric that seeks to optimize efficiency at selected operating points, so one would expect comparison of operating points find an optimal fan (highest FEI within construction and budget constraints).

Mike Wolf, April 18
> I suggest using the “design operating point”.  This is what is used to calculate the FEI.  Follow-up question; what is current used to model fan energy consumption?

- A: We decided to use the design operating point.

2. Q: Do we overwrite the EnergyPlus fan energy calculation using the newly added FEI algorithms? Currently in EnergyPlus users input the fan and motor efficiency. With FEI, those efficiencies are calculated and no longer user input.

Craig Wray, March 27
> Whether the fan system component efficiencies used now in E+ calcs should be replaced by the ones from the FEI calcs is something that needs further discussion. In particular, the “variable speed” fan models in E+ with specification of wire-to-air (mistakenly called “fan total”) and motor efficiencies only (with the imbedded assumption that the efficiencies vary based on a 4th order polynomial of unknown genesis) likely should not be used anymore, unless the polynomial is generated from “known” data.
Instead, one can already use the fan component model in E+ to specify and represent component efficiency variations with part-load over the entire operating range. For example, fan efficiency depends on the locus of operating points determined by the intersection of the fan performance power map (function of pressure and flow) and the system curve(s) (pressure vs flow). At least for VAV supply fans, the resulting fan efficiencies typically are not constant over the entire operating range. E+ contains a dimensionless model for fan efficiency variation, which I developed.  E+ also has a system curve model that Max Sherman and I developed (see https://pubarchive.lbl.gov/islandora/object/ir%3A154432). The Engineering Reference Manual and the I/O Manual provide further info in this regard (see attached excerpts – current versions on the Internet have severe equation formatting problems and poor quality graphics in these sections).
Note that AMCA 207 and 208 do not provide explicit models for fan efficiency variation or for the system curve. AMCA instead requires that the fan system electric input power be specified for each operating point or that fan shaft power be specified for each operating point (which in turn implicitly determines the fan efficiency at each point). For other components, the AMCA calcs does provide models. These inputs, of course, could be used instead of those already in the fan component model, if that is the desired approach. Perhaps the best path forward at this time is to allow the user to decide based on available data, with some guidance (especially from AMCA).

Mike Wolf, April 18
> I would say, yes, use FEI because fan and motor efficiency will be more accurately represented as part of FEI.

- A: We decided to overwrite the original fan model related user input and calculation logic.

3. New FEI object v.s. existing referring performance report

Mike Witte, April 20
> Would the metered electric consumption in the simulation still be calculated by one of the existing Fan:* models? As I read the NFP for the first time, I was expecting to see a new fan object in the proposal, something like **Fan:FanEnergyIndexModel**.

Jason Glazor, April 25
> If this does represent a better fan energy model, than perhaps a new **Fan:FanEnergyIndex** object should be added that is compatible with other fan objects and can be used where those can be used would make sense. 

Neal Kruis, April 25
> I want to strongly discourage any new fan models. If this really requires a new capability it should be added to the **Fan:System** model to further encourage the consolidation of objects.

Mike Witte, April 25
> A possible path is to have a Fan object which references a **Fan:Performance:SystemModel** or **Fan:Performance:FEIModel** or something to that effect.

- A: In the current proposed IDD, we suggested a report variable that refers to a Fan object. It indeed makes more sense to have a Fan object referring the FEI model, as the new inputs in the FEI object would overwrite the original fan model inputs. However, the second way require a transition change in the five existing Fan objects Fan:SystemModel, Fan:ConstanVolume, Fan:OnOff, Fan:VariableVolume, Fan:ComponentModel.

Neal Kruis, May 1

> I'm not opposed to having a separate object that contains inputs specific to the FEI calculation for a given Fan object. You could have a field in each fan object that points to the FEI inputs. This way there would be minimal changes to the existing models. Where possible, you should rely on the performance characteristics (e.g., power/capacity output, curves) in the existing Fan:* objects. Specifically, I'm looking at:
"Motor Part Load Efficiency Curve"
"Motor Nameplate Output Power"
"VFD Output Capacity"
I believe each of these can be derived from the existing Fan:* models (though it might be tricky). Otherwise, I feel that you are asking for too similar of inputs that would be too confusing for most users to actually use this feature effectively.
The other thing to consider is the eventual (fingers-crossed) arrival of the Fan:Performance:ASHRAE205 object. This will not have any performance curves, but should be able to be used to generate an FEI as well.

- A: We modified the object and named it FanPerformance:FEIModel to be consistent with an existing FanPerformance object. We added clarification that the corresponding parameters would be overwritten by the FEI model inputs.

4. Reasonable assumptions to reduce user input

Jason Glazor, April 25
> It would be great if what was reported was based on the most reasonable assumptions and did not require additional user input for it to appear. If the user wants to specify some of the inputs that you are suggesting, maybe a second number could be shown that represents these special conditions. This is a valuable output and if possible should not require a deep understanding of the rating procedure to appear for the user.

Lixing Gu, April 26
> I like Larry’s opinion to support Jason’s comments to report FEI based on the most reasonable assumptions and did not require additional user input for it to appear. 

- A: We added the assumptions in the calculation methods for the reference and actual fans to Pages 11-12.

5. Output

Jason Glazor, April 25
> It is difficult to understand exactly what is being proposed in the NFP but if possible it would be great if a output for the FEI was created for any fans used in EnergyPlus. I agree that should  appear in Equipment Summary under Fans in the tabular report. I think it should probably also appear in the EIO file.

- A: We agreed to export the FEI in the EIO file.

## Acknowledgment

We appreciate support from Michael Ivanovich of AMCA and Craig Wray.

## References

ANSI/AMCA Standard 207-17: Fan System Efficiency and Fan System Input Power Calculation, 2017.

ANSI/AMCA Standard 208-18: Calculation of the Fan Energy Index, 2018.

## Appendices

From AMCA 208:

Table A.2 - Fan Types, Test Configurations and FEI Pressure Basis

From AMCA 207:

Tables A1, A2 (a, b), A3

Tables B1, B2

Tables C1, C2

Tables D1, D2
