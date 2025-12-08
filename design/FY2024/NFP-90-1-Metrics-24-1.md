# Additional ASHRAE Metrics Reporting in EnergyPlus 24-1
**NFP/Design -- Digital Alchemy**

**Revisions**
-   New Feature Proposal -- 15-Nov23
-   New Feature Design & Updates -- 15-Nov-23; 18-Dec-23

## Justification for New Feature
Many new metrics were established in the ASHRAE 90.1-2019 \[1\] standard.
EnergyPlus should be updated to predict these new metrics for equipment used in simulation.

## Overview
We have identified and investigated the following commonly used
equipment metrics, defined by various industry organizations and used in
ASHRAE 90.1-2019:

|#| Metric <br/>(Defining Org) | Used in | Description | Equipment<br/> Measured | URl |
|--|--|--|--|--|--|
|1 |SEER2 <br/> [(AHRI 210/240-2023)](https://www.ahrinet.org/search-standards/ahri-210240-2023-2020-performance-rating-unitary-air-conditioning-air-source-heat) | ASHRAE 90.1-2019 | (new) Seasonal Energy Efficiency Ratio | Air Conditioner Condensing Units<br/> Heat Pumps<br/> Single Packaged Units<br/> Evaporator /Coils<br/> Gas Furnaces |	[[2]](https://www.ahrinet.org/search-standards/ahri-210240-2023-2020-performance-rating-unitary-air-conditioning-air-source-heat) |
|2	| HSPF2 <br/> [(AHRI 210/240-2023)](https://www.ahrinet.org/search-standards/ahri-210240-2023-2020-performance-rating-unitary-air-conditioning-air-source-heat) | ASHRAE 90.1-2019	|(new) Heating Seasonal Performance Factor|	Unitary Air-source Heat Pump	|[[3]](https://www.ahrinet.org/search-standards/ahri-210240-2023-2020-performance-rating-unitary-air-conditioning-air-source-heat)|
|3	| IEER <br/> [(AHRI 340/360-2022)](https://www.ahrinet.org/search-standards/ahri-340360-i-p2022-performance-rating-commercial-and-industrial-unitary-air) | ASHRAE 90.1-2019 | Integrated Energy Efficiency Ratio | Commercial and Industrial Unitary Air-conditioning and Heat Pump |[[4]](https://www.ahrinet.org/search-standards/ahri-340360-i-p2022-performance-rating-commercial-and-industrial-unitary-air) |
|4	|CEER <br/>[(ANSI/AHAM RAC-1-2020)](https://www.aham.org/ItemDetail?iProductCode=20010&Category=MADSTD) | ASHRAE 90.1-2019	|Combined Energy Efficiency Ratio	|Household Room Air Conditioners	|[[5]](https://www.aham.org/ItemDetail?iProductCode=20010&Category=MADSTD)|
|5	|MRE <br/> [(AHRI 910-2020)](https://www.ahrinet.org/search-standards/ahri-910-i-p2014-performance-rating-indoor-pool-dehumidifiers) | ASHRAE 90.1-2019|	Moisture Removal Efficiency	|Indoor Pool Dehumidifiers	|[[6]](https://www.ahrinet.org/search-standards/ahri-910-i-p2014-performance-rating-indoor-pool-dehumidifiers)|
|6	|ISMRE <br/>[(AHRI 920-2020)](https://global.ihs.com/doc_detail.cfm?document_name=AHRI%20920%20I%2DP&item_s_key=00662234) | ASHRAE 90.1-2019	|Integrated Seasonal Moisture Removal Efficiency|	DOAS Specification	|[[7]](https://global.ihs.com/doc_detail.cfm?document_name=AHRI%20920%20I%2DP&item_s_key=00662234)|
|7	|ISCOP <br/> [(AHRI 920-2020)](https://global.ihs.com/doc_detail.cfm?document_name=AHRI%20920%20I%2DP&item_s_key=00662234) | ASHRAE 90.1-2019 |Integrated Seasonal Coefficient Of Performance	|Direct Expansion-Dedicated Outdoor Air System Units (DX DOAS Units)	|[[8]](https://global.ihs.com/doc_detail.cfm?document_name=AHRI%20920%20I%2DP&item_s_key=00662234)|
|8	|UEF <br/> [(CFR-430-2016 Appendix E)](https://www.govinfo.gov/app/details/CFR-2016-title10-vol3/CFR-2016-title10-vol3-part430-subpartB-appE) | ASHRAE 90.1-2019|	Uniform Energy Factor|	Water Heaters	|[[9]](https://www.govinfo.gov/app/details/CFR-2016-title10-vol3/CFR-2016-title10-vol3-part430-subpartB-appE)|
|9	|FEI  <br/>[(AMCA 208-2018)](https://www.techstreet.com/amca/searches/35955048) | ASHRAE 90.1-2019 |	fan energy index	| Fan Energy |	[[10]](https://www.techstreet.com/amca/searches/35955048) |

In previous Task Orders, we implemented support for the following calculations and reporting:
-   SEER2/HSPF2 -- support for these Metrics was implemented/released in EnergyPlus 22-2, based on the 2023 version of the ANSI/AHRI 210-240 standard.
    -   SEER2 - for the SingleSpeed, MultiSpeed, and CurveFit:Speed variants of DX Cooling coils.
    -   HSPF2 - for the SingleSpeed and MultiSpeed DX Heating coils. All of this was implemented based on the 2023 version of the ANSI/AHRI 210-240 standard.
-   EER/IEER -- support for these Metrics was implemented/released in EnergyPlus 23-1, based on the 2022 version of the AHRI Standard 340-360
    -   EER - for the SingleSpeed, TwoSpeed, MultiSpeed, VariableSpeed, and CurveFit:Speed variants of DX Cooling coils.
    -   IEER - for the SingleSpeed, TwoSpeed, MultiSpeed, VariableSpeed, and CurveFit:Speed variants of DX Cooling coils.

In this Task Order we propose the following scope of work:
-   Implement SEER2 -- for the following configurations and calculation variants, as defined in the 2023 version of the ANSI/AHRI Standard 210-240:
    - Coil:Cooling:DX:TwoSpeed -- supporting 4 different calculation paths, determined by the building cooling load and the Coil's cooling capacity
    - Coil:Cooling:DX:VariableSpeed -- supporting 3 different calculation paths for several configurations, ranging from 1 speed to 10 speeds, and also depending on the building cooling load and the coil's cooling capacity at each speed.
-   Refactoring of standards calculation and reporting code, which has been implemented by various team members over the past 15 years
-   Improved handling of metrics calculation/reporting, based on Coil cooling capacity.  The applicable metric standard rating is dependent on the coil's cooling capacity.  For example, the applicable standard rating for coils with a capacity of less than 65K Btu/hr is SEER2 (AHRI Standard 210/240), whereas for coils with capacities from 65K Btu/hr and 135K Btu/hr, it is IEER (AHRI Standard 340/360)

## SEER2 = Seasonal Energy Efficiency Rating (2023)
SEER2 is calculated based on ANSI/AHRI Standard 210/240-2023: applies to: Unitary Air-conditioners and Unitary Air-source Heat Pumps with
capacities less than 65,000 Btu/h.

### FOR TWO STAGE SYSTEMS

|Two Stage Systems|
|--|
|Graphical representation of SEER2:<br><img src="https://user-images.githubusercontent.com/49325382/173099074-caeb59f7-d3b8-45ab-b8a7-5367506ff7f6.png" width=75% height=75%>|

|Variable|Formulas|
|--|--|
|SEER2|<img src="https://user-images.githubusercontent.com/49325382/173098299-1b89804d-d8a3-4a75-8db3-8ad3bbdc8e36.png" width=40% height=40%>|
|q(t<sub>j</sub>) and E(t<sub>j</sub>) are calculated for the following temperature bins|<img src="https://user-images.githubusercontent.com/49325382/173098780-28c8a2ef-b13f-46e3-956e-6540477912dc.png" width=70% height=70%>|
|BL(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173107798-68ba98a9-e5d8-4a07-9a18-da6b1145bf5b.png" width=90% height=90%>||
|q<sub>LOW</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173108139-86c76624-cf0a-4452-a0e9-3e7379898df4.png" width=85% height=85%>||
|P<sub>LOW</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173108210-583a6e5a-5b81-40cf-8d85-dcd31b0358a0.png" width=85% height=85%>||
|q<sub>Full</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173108349-8f3ffe8c-3960-4b1d-8d42-a649556b0e5c.png" width=85% height=85%>||
|P<sub>Full</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173108376-c5732907-dd1b-46fc-89a3-1dcaf3ca59a2.png" width=85% height=85%>||
## Case 1
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173108719-2512fa72-daf7-4ce3-a2d3-c54a9fee1b31.png" width=60% height=60%>|Building load is less than Low Stage capacity|
||<img src="https://user-images.githubusercontent.com/49325382/173108824-f8bee0dc-5ba6-48e0-a8f6-0e8719e28305.png" width=70% height=70%>|
|C<sub>D</sub><sup>C</sup><br>Default Cooling Degradation Coefficient|Default: 0.2<br>If Tests C and D are not performed or C<sub>D</sub><sup>C</sup> is greater than default use default value|
## Case 2
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173109144-f7d78157-6f28-496c-a148-e4262494bb20.png" width=120% height=120%>|Building load is greater than Low Stage capacity, but less than Full Stage capacity<br>and the unit cycles between Low Stage operation and Full Stage operation|
||<img src="https://user-images.githubusercontent.com/49325382/173109198-c817551d-8507-44ea-8f0f-f070b5cf695e.png" width=65% height=65%>|
## Case 3
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173109144-f7d78157-6f28-496c-a148-e4262494bb20.png" width=120% height=120%>|Building load is greater than Low Stage capacity, but less than Full Stage capacity<br>and the unit cycles between off and Full Stage operation|
||<img src="https://user-images.githubusercontent.com/49325382/173109574-80e66076-9575-413b-a465-416d82ea1db3.png" width=60% height=60%>|
|C<sub>D</sub><sup>c,Full</sup><br>Default Cooling Degradation Coefficient|Default: 0.2<br>If Tests C and D are not conducted set C<sub>D</sub><sup>c,Full</sup> to the default value or use the following forumlar:<br><img src="https://user-images.githubusercontent.com/49325382/173117743-27b547d9-8b76-4056-9f91-7c655b387df5.png" width=40% height=40%><br>If the test is conducted use the following forumlar:<br><img src="https://user-images.githubusercontent.com/49325382/173117884-9f2f26f4-8ed4-496d-b6d2-4c30beca8b1d.png" width=40% height=40%><br><img src="https://user-images.githubusercontent.com/49325382/173117960-61d85889-c575-4b0c-b8a3-e32d591ca7d2.png" width=40% height=40%>|
## Case 4
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173118064-7b60a4a5-97f4-4527-9e30-f9f1362c2c76.png" width=50% height=50%><img src="https://user-images.githubusercontent.com/49325382/173118087-143d1ad3-8563-44b9-a5c0-1ebb16791c6c.png" width=50% height=50%>|Building load is greater than or equal to the unit capacity|
||<img src="https://user-images.githubusercontent.com/49325382/173118193-28b2fc85-c7a9-437b-8998-32d7f8a10ae2.png" width=50% height=50%>|

### FOR VARIABLE SPEED SYSTEMS
 |Variable Speed Systems|
|--|
|Graphical representation of SEER2:<br><img src="https://user-images.githubusercontent.com/49325382/173118907-586d86c5-5b44-45a2-99fd-667391a0c7ac.png" width=75% height=75%>|

|Variable|Formulas|
|--|--|
|SEER2|<img src="https://user-images.githubusercontent.com/49325382/173120318-d5df3faf-7196-4bb8-badc-ca3e56166dce.png" width=30% height=30%>|
|q<sub>LOW</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122263-ae5eecb5-1e19-47b0-94d1-4fa087e9a8bb.png" width=80% height=80%>|
|P<sub>LOW</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122290-8dd0658a-acad-4137-8cfe-abfac6e49e4c.png" width=75% height=75%>|
|q<sub>Int-Bin</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122372-093c6157-2438-4cbc-9191-6f995fe53f0b.png" width=60% height=60%>|
|E<sub>Int-Bin</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122395-cfec8d6e-9db2-4b88-955c-0b3ba7d29df4.png" width=66% height=66%>|
|q<sub>Int</sub>(t<sub>j</sub>)|<img src="(https://user-images.githubusercontent.com/49325382/173122508-574ace81-85a7-4c2f-b00b-d3e1b6d812d9.png" width=90% height=90%><br><img src="https://user-images.githubusercontent.com/49325382/173122561-5ae042d0-28a6-4fd9-812d-f9486abe1066.png" width=90% height=90%>|
|q<sub>Int</sub>(87)|<img src="https://user-images.githubusercontent.com/49325382/173122633-20787666-fa61-4ca2-b960-92528f8cf9bc.png" width=75% height=75%>|
|P<sub>Int</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122777-aa94c891-7248-4448-a89c-93cfcef07301.png" width=90% height=90%>|
|P<sub>LOW</sub>(87)|<img src="https://user-images.githubusercontent.com/49325382/173122879-6a4382de-bd4b-4ed4-8f17-880d1adb0dc4.png" width=72% height=72%>|
## Case 1
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173123094-073047bf-c555-4b7c-b826-fdd95ea3a13e.png" width=28% height=28%><br><img src="https://user-images.githubusercontent.com/49325382/173123126-76a4e1c1-6dce-4fcc-8f24-1405024f3392.png" width=68% height=68%>|Building load is no greater than unit capacity at low speed|
||<img src="https://user-images.githubusercontent.com/49325382/173125098-d13579fb-6db4-4db1-bf89-54f217a378dd.png" width=85% height=85%>|
|C<sub>D</sub><sup>C,low</sup>|<img src="https://user-images.githubusercontent.com/49325382/173123971-9e334ba4-4922-4a0a-bb93-fd85bccc4952.png" width=50% height=50%><br><img src="https://user-images.githubusercontent.com/49325382/173124040-b8ea5f77-d0ad-4cd8-b141-36d59162e9fc.png" width=55% height=55%><br>Substitute Test G and I for test C and D
|C<sub>D</sub><sup>C</sup><br>Default Cooling Degradation Coefficient|Default: 0.25<br>If Tests G and I are not performed or C<sub>D</sub><sup>C</sup> is greater than default use default value|
## Case 2
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173124209-db6b6044-efe8-4e7f-88f7-aae795049240.png" width=140% height=140%>|Building load can be matched by modulating the compressor<br>speed between low speed and full speed|
|q<sub>Int-Bin</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122372-093c6157-2438-4cbc-9191-6f995fe53f0b.png" width=60% height=60%>|
|E<sub>Int-Bin</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173122395-cfec8d6e-9db2-4b88-955c-0b3ba7d29df4.png" width=68% height=68%>|
||<img src="https://user-images.githubusercontent.com/49325382/173109198-c817551d-8507-44ea-8f0f-f070b5cf695e.png" width=90% height=90%>||
|EER<sub>Int-Bin</sub>(t<sub>j</sub>)|<img src="https://user-images.githubusercontent.com/49325382/173124302-997c297d-555b-4ed2-afd5-6351d53642ba.png" width=135% height=135%>|
## Case 3
|Variable|Formulas|
|--|--|
|<img src="https://user-images.githubusercontent.com/49325382/173124546-69a9d1f7-570a-422a-9b02-05aff8282289.png" width=90% height=90%>|Building load is equal to or greater than unit capacity at full stage|
||!<img src="https://user-images.githubusercontent.com/49325382/173118193-28b2fc85-c7a9-437b-8998-32d7f8a10ae2.png" width=45% height=45%>|

## Test Conditions: <br/>
!<img src="https://user-images.githubusercontent.com/78803858/205347858-a130fb11-c23b-4a34-bdc4-2a32e7d8d263.png" width=85% height=85%><br/><br/>
!<img src="https://user-images.githubusercontent.com/78803858/205354762-fdce0568-e15d-47f0-8966-cbeeb88df9c4.png" width=85% height=85%><br/>

**Example Test Points Used for EER Rating Points**<br/>
!<img src="https://user-images.githubusercontent.com/78803858/205357978-5b1096e7-3b38-44b4-a870-8881c911296e.png" width=85% height=85%><br/>

## Approach
The EnergyPlus components for this scope of work are: Coil:Cooling:DX:TwoSpeed and Coil:Cooling:DX:VariableSpeed (for which calculations must support from 1 to 10 speeds). The TwoSpeed implementation will be new, as the standard calculations are different from OneSpeed and VariableSpeed.  The VariableSpeed implementation will have some similarities to the previous implementation for MultiSpeed, but will be more involved as it must adapt to number of speeds ranging from 1 to 10.
Unit tests will be implemented for many, if not all variations -- to ensure the calculation is correct and to fag any regressions that might occur in the future.

Once the metrics are calculated we can integrate them into the corresponding report code of the equipment summary table.
This work will all be done by extending the implementation in StandardRatings.hh & StandardRatings.cc.  Our initial estimate for the scope of this work includes the following:
-   Expand MultiSpeedDXCoolingCoilStandardRatings to report SEER2 values for the following E+ coils:
    -   Coil:Cooling:DX:TwoSpeed
    -   Coil:Cooling:DX:VariableSpeed

-   Extend ReportDXCoilRating to report SEER2 for these Coils
-   CheckCurveLimitsForStandardRatings
-   Ensure existing unit tests pass with changes
-   Add new tests where new cases have been added

Given the budget in this round of development, implementation of HSPF2 calculation & reporting for variable speed DX heating coils, will not be possible.  We hope that this can be done in the EnergyPlus 24-2 implementation cycle.

## Testing/Validation/Data Source(s)
Example files will be simulated to confirm calculation and reporting of the these metrics. Simulation results will be compared with values generated by a third-party implementations.
Peer reviewer input on this is eagerly invited.
Unit tests for the Continuous Integration (CI) pipeline will also be updated/extended to confirm the calculation of the metrics and ensure the code produces correct results and does not fail when future code changes are merged.

## Input Output Reference Documentation
The Input Output Documentation will be expanded to include the new metric. There will be an update in two places: first on the related section of the EnergyPlus components for which the metric is calculated and second in the reports section, specifically the Equipment Summary report. This will be done for (2) coil types listed above.
We will also add a disclaimer in the sections for these Coils: \"It is not reasonable to expect AHRI ratings calculated from model inputs to match the actual ratings in the AHRI directory. This is largely because it is common practice for manufacturers to underrate their equipment for marketing reasons and/or to build in some safety margin in case DOE audits their ratings.\"

## Input Description
Fields To support Calculation of SEER2 were added to these Coils in EnergyPlus 22-2, to represent the static pressure on the fans (Power per flow rate).
- Coil:Cooling:DX:TwoSpeed -- for EER/IEER (E+ 23-1) & SEER2 (E+ 23-2)
- Coil:Cooling:DX:VariableSpeed -- for EER/IEER (E+ 23-1) & SEER2 (E+ 23-2)

Therefore, we will not need to add any new fields in this development cycle.

## Outputs Description
The EPlusout-EIO.tex files will be updated to document these metrics for (2) Coil types listed above -- that have been added to the HTML report generated by EnergyPlus after simulation.  These updates will document the fields in the tables named “DX Cooling Coil Standard Ratings 2017” and “DX Cooling Coil Standard Ratings 2023.”

## Engineering Reference
The Engineering Reference will be updated with a high-level description of the calculations with links/references to Formulae defined in the standard - for the (2) coil types listed above.

## Example Files and Transition Changes
Example files for Coil:Cooling:DX:TwoSpeed in which the condenser is air-cooled:
    -   Coil:Cooling:DX:TwoSpeed.idf
    -   ASHRAE9012016_OfficeMedium_Denver.idf
    -   RefBldgPrimarySchoolNew2004_Chicago.idf
Example files for Coil:Cooling:DX:VariableSpeed in which the condenser is air-cooled:
    -   5Zone_Unitary_HXAssistedCoil.idf
    -   HeatPumpVSAS.idf
    -   PackagedTerminalAirConditionerVSAS.idf
    -   PackagedTerminalHeatPumpVSAS.idf

## E-mail and Conference Call Conclusions ##
[15-Nov-23] During NFP presentation, posed the question about whether EnergyPlus support simulation of the 'lock-out' feature described in Case 3 of the Two-Stage calculations section of the AHRI Standard 210/240.  As loosely described, this feature avoids unnessary cycling through low stage in very hot outdoor temperatures -- instead cycling direct to Full stage.  Mike Witte suggested that we pose this question for R.Raustad and N.Bereket in email.
[1-Dec-23] Response from R.Raustad covered the following:
- Data for both Coil:Cooling:DX:TwoSpeed and Coil:Cooling:DX:VariableSpeed in EnergyPlus - include fields for 'Minimum Outdoor Dry-Bulb Temperature for Compressor operation.  However we are looking for a high temperature at which Low Stage operation is skipped; cycling instead directly to Full Stage.
- ZoneControl:Thermostat:StagedDualSetpoint can be used to cycle between off and full stage operation.
[2-Dec-23] Response from N.Bereket included the following:
- Calculation for Coil:Cooling:DX:MultiSpeed and Coil:Cooling:DX:VariableSpeed will both use calculations described in the standard for Variable Speed systems.
- Coil:Cooling:DX:TwoSpeed implementation in EnergyPlus may be more realistic because it does address multi-stage compressors
- Equations in the AHRI standard approximate results measured in test labs and include some assumptions and approximations.  Consequently, some adjustment may be required.

## Acknowledgments
Thanks to R.Raustad and N.Bereket for their responses to questions raised in the NFP review.

## References
\[1\] ASHRAE, ANSI/ASHRAE/IES Standard 90.1-2019: Energy Standard for
Buildings Except Low-Rise Residential Buildings. 2019.

\[2\] Jonestone Supply: The SEER2 Guide. 2022. URL: [AHRI 210/240-202](https://seer2.com/)

\[3\] AHRI: Air Conditioning, Heating & Refrigeration Institute. 2023
(2020) Standard for Performance Rating of Unitary Air-conditioning &
Air-source Heat Pump Equipment. 2020. URL: [AHRI 210/240-2023](https://www.ahrinet.org/Portals/\_Appleseed/documents/Standards/AHRI%20Standard%20210.240-2023%20(2020).pdf)

\[4\] AHRI: AHRI Standard 340/360-2022 (I-P): 2022 Standard for
Performance Rating of Commercial and Industrial Unitary Air-conditioning
and Heat Pump Equipment. 2019. URL: [AHRI 340/360-2022](https://www.ahrinet.org/Portals/Standards/AHRI%20Standard%20340-360-2022%20(I-P).pdf)

\[5\] ANSI/AHAM: ANSI/AHAM RAC-1-2015: Room Air Conditioners. 2015. URL: [ANSI/AHAM RAC-1-2020](https://webstore.ansi.org/standards/aham/ansiahamrac2015)

\[6\] ANSI/AHAM: ANSI/AHRI 910-I-P-2014: Performance Rating of Indoor
Pool Dehumidifiers. 2014. URL: [AHRI 910-2020](http://www.ahrinet.org/App_Content/ahri/files/STANDARDS/AHRI/AHRI_Standard_910_I-P_2014.pdf)

\[7\] HPAC Engineering. Understanding AHRI 920 and ISMRE Ratings for
Most Efficient DOAS Specification. 2018. URL:[AHRI 920-2020](https://www.hpac.com/columns/managing-facilities/article/20929562/understanding-ahri-920-and-ismre-ratings-for-most-efficient-doas-specification)

\[8\] ANSI/AHAM: ANSI/AHRI 920-I-P-2015: 2015 Standard for Performance
Rating of DX-Dedicated Outdoor Air System Units. 2015.<br/>&ensp;&ensp; URL: [AHRI 920-2020](https://www.ahrinet.org/App_Content/ahri/files/STANDARDS/ANSI/ANSI_AHRI_Standard_920_I-P_2015.pdf)

\[9\] 10 CFR Appendix E to Subpart B of Part 430 - Uniform Test Method
for Measuring the Energy Consumption of Water Heaters. 2016.<br/>&ensp;&ensp; URL: [FR-430-2016 Appendix E](https://www.govinfo.gov/app/details/CFR-2016-title10-vol3/CFR-2016-title10-vol3-part430-subpartB-appE)

\[10\] ANSI/AMCA Standard 208, Calculation of the Fan Energy Index.
2019. URL: [AMCA 208-2018](https://www.amca.org/news/press-releases/ansi/amca-standard-208,-calculation-of-the-fan-energy-index,-available-for-free-download.html&usg=AOvVaw2eZxHxgdq183Y9WsZlgjTB)

\[11\] Differences between SEER and HSOPF and SEER2 and HSPF2 URL: [Presentation](https://www.ahrinet.org/Portals/AHRI%20CAC%20HP%20Appendix%20M1%20Presentation%20-%20AHR%20Expo.pdf)
