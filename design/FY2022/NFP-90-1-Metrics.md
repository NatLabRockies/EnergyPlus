Update output reporting of mechanical equipment using 90.1-2019 metrics
================

**Tobias Maile, Richard See**

**Digital Alchemy**

 - Original Date: April 14, 2022
 - Updated Date: June 15, 2022

## Justification for New Feature ##

Many new metrics appear in the ASHRAE 90.1-2019 [1] standard. EnergyPlus needs to stay in sync with new metrics and thus needs to be updated.

## Overview ##

We have identified the following commonly used metrics:

| Metric | Standard it is used in | Description | Components | URL |
|--|--|--|--|--|
|SEER2|ASHRAE 90.1-2019|new Seasonal Energy Efficiency Ratio|Air Conditioner Condensing Units<br>Heat Pumps<br>Single Packaged Units<br>Evaporator Coils<br>Gas Furnaces|[2]|
|HSPF2 (AHRI 210/240)|ASHRAE 90.1-2019|new Heating Seasonal Performance Factor|Unitary Air-source Heat Pump|[3]|
|IEER (AHRI 340/360)|ASHRAE 90.1-2019|Integrated Energy Efficiency Ratio|Commercial and Industrial Unitary Air-conditioning and Heat Pump|[4]|
|CEER (ANSI/AHAM RAC-1)|ASHRAE 90.1-2019|Combined Energy Efficiency Ratio|Household Room Air Conditioners|[5]|
|MRE (AHRI 910)|ASHRAE 90.1-2019|Moisture Removal Efficiency|Indoor Pool Dehumidifiers |[6]|
|ISMRE|ASHRAE 90.1-2019|Integrated Seasonal Moisture Removal Efficiency|DOAS Specification|[7]|
|ISCOP (AHRI 920)|ASHRAE 90.1-2019|Integrated Seasonal Coefficient of Performance|Direct Expansion-Dedicated Outdoor Air System Units|[8]|
|UEF (10 CFP 430 Appendix E)|ASHRAE 90.1-2019|Uniform Energy Factor|Water Heaters|[9]|
|FEI (AMCA 208)|ASHRAE 90.1-2019|Fan Energy Index|Fan Energy|[10]|

In the current workorder, we would focus on two metrics only and propose to focus on SEER2 and HSPF2 based on ANRI 210/240 2023.
In order to support the old metrics side by side with the new metrics, we proposed to refactor the calculation of SEER and HSPF2 into separate functions so they can be easily depricated when the time comes.

**1. SEER2**

SEER2 is calculated based on ANSI/AHRI Standard 210/240-2023: applies to: Unitary Air-conditioners and Unitary Air-source Heat Pumps with
capacities less than 65,000 Btu/h.

*For Single Stage Systems*

|Single Stage Systems|
|--|
|Graphical representation of SEER2:<br>![grafik](https://user-images.githubusercontent.com/49325382/173094322-3b9a650f-cccc-4e0f-a9ae-559c60d860e6.png)|

|Variable|Formulas|Code|
|--|--|--|
|SEER2|![grafik](https://user-images.githubusercontent.com/49325382/173093830-56c75bd8-8e3b-49b5-b58f-0788f717e479.png)|TBD|
|PLF(0.5)|![grafik](https://user-images.githubusercontent.com/49325382/173094047-5a2d4c02-a8b6-4690-9bfc-b337637dd780.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173094092-27b80da1-3fd4-415a-963f-f0e4d11ccf64.png)|TBD|
|C<sub>D</sub><sup>C</sup><br>Default Cooling Degradation Coefficient|Default: 0.2<br>If Tests C and D are not performed or C<sub>D</sub><sup>C</sup> is greater than default use default value|TBD|

*For Two Stage Systems*

|Two Stage Systems|
|--|
|Graphical representation of SEER2:<br>![grafik](https://user-images.githubusercontent.com/49325382/173099074-caeb59f7-d3b8-45ab-b8a7-5367506ff7f6.png)|

|Variable|Formulas|Code|
|--|--|--|
|SEER2|![grafik](https://user-images.githubusercontent.com/49325382/173098299-1b89804d-d8a3-4a75-8db3-8ad3bbdc8e36.png)|TBD|
|q(t<sub>j</sub>) and E(t<sub>j</sub>) are calculated for the following temperature bins|![grafik](https://user-images.githubusercontent.com/49325382/173098780-28c8a2ef-b13f-46e3-956e-6540477912dc.png)|
|BL(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173107798-68ba98a9-e5d8-4a07-9a18-da6b1145bf5b.png)||
|q<sub>LOW</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173108139-86c76624-cf0a-4452-a0e9-3e7379898df4.png)||
|P<sub>LOW</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173108210-583a6e5a-5b81-40cf-8d85-dcd31b0358a0.png)||
|q<sub>Full</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173108349-8f3ffe8c-3960-4b1d-8d42-a649556b0e5c.png)||
|P<sub>Full</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173108376-c5732907-dd1b-46fc-89a3-1dcaf3ca59a2.png)||
|Case 1<br>![grafik](https://user-images.githubusercontent.com/49325382/173108719-2512fa72-daf7-4ce3-a2d3-c54a9fee1b31.png)|Building load is less than Low Stage capacity||
||![grafik](https://user-images.githubusercontent.com/49325382/173108824-f8bee0dc-5ba6-48e0-a8f6-0e8719e28305.png)||
|C<sub>D</sub><sup>C</sup><br>Default Cooling Degradation Coefficient|Default: 0.2<br>If Tests C and D are not performed or C<sub>D</sub><sup>C</sup> is greater than default use default value||
|Case 2<br>![grafik](https://user-images.githubusercontent.com/49325382/173109144-f7d78157-6f28-496c-a148-e4262494bb20.png)|Building load is greater than Low Stage capacity, but less than Full Stage capacity and the unit cycles between Low Stage operation and Full Stage operation||
||![grafik](https://user-images.githubusercontent.com/49325382/173109198-c817551d-8507-44ea-8f0f-f070b5cf695e.png)||
|Case 3<br>![grafik](https://user-images.githubusercontent.com/49325382/173109144-f7d78157-6f28-496c-a148-e4262494bb20.png)|Building load is greater than Low Stage capacity, but less than Full Stage capacity and the unit cycles between off and Full Stage operation||
||![grafik](https://user-images.githubusercontent.com/49325382/173109574-80e66076-9575-413b-a465-416d82ea1db3.png)||
|C<sub>D</sub><sup>c,Full</sup><br>Default Cooling Degradation Coefficient|Default: 0.2<br>If Tests C and D are not conducted set C<sub>D</sub><sup>c,Full</sup> to the default value or use the following forumlar:<br>![grafik](https://user-images.githubusercontent.com/49325382/173117743-27b547d9-8b76-4056-9f91-7c655b387df5.png)<br>If the test is conducted use the following forumlar:<br>![grafik](https://user-images.githubusercontent.com/49325382/173117884-9f2f26f4-8ed4-496d-b6d2-4c30beca8b1d.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173117960-61d85889-c575-4b0c-b8a3-e32d591ca7d2.png)||
|Case 4<br>![grafik](https://user-images.githubusercontent.com/49325382/173118064-7b60a4a5-97f4-4527-9e30-f9f1362c2c76.png)![grafik](https://user-images.githubusercontent.com/49325382/173118087-143d1ad3-8563-44b9-a5c0-1ebb16791c6c.png)|Building load is greater than or equal to the unit capacity||
||![grafik](https://user-images.githubusercontent.com/49325382/173118193-28b2fc85-c7a9-437b-8998-32d7f8a10ae2.png)||

*For Variable Speed System*
 |Variable Speed Systems|
|--|
|Graphical representation of SEER2:<br>![grafik](https://user-images.githubusercontent.com/49325382/173118907-586d86c5-5b44-45a2-99fd-667391a0c7ac.png)|

|Variable|Formulas|Code|
|--|--|--|
|SEER2|![grafik](https://user-images.githubusercontent.com/49325382/173120318-d5df3faf-7196-4bb8-badc-ca3e56166dce.png)|TBD|
|q<sub>LOW</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122263-ae5eecb5-1e19-47b0-94d1-4fa087e9a8bb.png)||
|P<sub>LOW</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122290-8dd0658a-acad-4137-8cfe-abfac6e49e4c.png)||
|q<sub>Int-Bin</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122372-093c6157-2438-4cbc-9191-6f995fe53f0b.png)||
|E<sub>Int-Bin</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122395-cfec8d6e-9db2-4b88-955c-0b3ba7d29df4.png)||
|q<sub>Int</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122508-574ace81-85a7-4c2f-b00b-d3e1b6d812d9.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173122561-5ae042d0-28a6-4fd9-812d-f9486abe1066.png)||
|q<sub>Int</sub>(87)|![grafik](https://user-images.githubusercontent.com/49325382/173122633-20787666-fa61-4ca2-b960-92528f8cf9bc.png)||
|P<sub>Int</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122777-aa94c891-7248-4448-a89c-93cfcef07301.png)||
|P<sub>LOW</sub>(87)|![grafik](https://user-images.githubusercontent.com/49325382/173122879-6a4382de-bd4b-4ed4-8f17-880d1adb0dc4.png)||
|Case 1<br>![grafik](https://user-images.githubusercontent.com/49325382/173123094-073047bf-c555-4b7c-b826-fdd95ea3a13e.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173123126-76a4e1c1-6dce-4fcc-8f24-1405024f3392.png)|Building load is no greater than unit capacity at low speed||
||![grafik](https://user-images.githubusercontent.com/49325382/173125098-d13579fb-6db4-4db1-bf89-54f217a378dd.png)||
|C<sub>D</sub><sup>C,low</sup>|![grafik](https://user-images.githubusercontent.com/49325382/173123971-9e334ba4-4922-4a0a-bb93-fd85bccc4952.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173124040-b8ea5f77-d0ad-4cd8-b141-36d59162e9fc.png)<br>Substitute Test G and I for test C and D||
|C<sub>D</sub><sup>C</sup><br>Default Cooling Degradation Coefficient|Default: 0.25<br>If Tests G and I are not performed or C<sub>D</sub><sup>C</sup> is greater than default use default value||
|Case 2<br>![grafik](https://user-images.githubusercontent.com/49325382/173124209-db6b6044-efe8-4e7f-88f7-aae795049240.png)|Building load can be matched by modulating the compressor speed between low speed and full speed||
|q<sub>Int-Bin</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122372-093c6157-2438-4cbc-9191-6f995fe53f0b.png)||
|E<sub>Int-Bin</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173122395-cfec8d6e-9db2-4b88-955c-0b3ba7d29df4.png)||
||![grafik](https://user-images.githubusercontent.com/49325382/173109198-c817551d-8507-44ea-8f0f-f070b5cf695e.png)||
|EER<sub>Int-Bin</sub>(t<sub>j</sub>)|![grafik](https://user-images.githubusercontent.com/49325382/173124302-997c297d-555b-4ed2-afd5-6351d53642ba.png)||
|Case 3<br>![grafik](https://user-images.githubusercontent.com/49325382/173124546-69a9d1f7-570a-422a-9b02-05aff8282289.png)|Building load is equal to or greater than unit capacity at full stage||
||![grafik](https://user-images.githubusercontent.com/49325382/173118193-28b2fc85-c7a9-437b-8998-32d7f8a10ae2.png)||

**2. HSPF2**

HSPF2 is calculated based on ANSI/AHRI Standard 210/240-2023: applies to: Unitary Air-conditioners and Unitary Air-source Heat Pumps with
capacities less than 65,000 Btu/h

Calculation is done as follows (more details in ANSI/AHRI 210/240):

**HSPF2**

*For Single Stage Systems*

|Single Stage Systems|
|--|
|Graphical representation of HSPFs:<br>![grafik](https://user-images.githubusercontent.com/49325382/173143481-5eed06c5-27fd-4738-9949-521e1c78f4ba.png)|

|Variable|Formulas|Code|
|--|--|--|
|HSPF2|![grafik](https://user-images.githubusercontent.com/49325382/173143637-e0040636-ead7-4f8c-aa98-a5201e186dca.png)|TBD|
||![grafik](https://user-images.githubusercontent.com/49325382/173143734-78cfcda6-dbcd-4033-9580-d4e5147d9fe2.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173143858-d7f16ea6-9d41-4ff5-aefe-e32f367a99d5.png)||
|F<sub>def</sub>|for systems with Demand-defrost Control System<br>![grafik](https://user-images.githubusercontent.com/49325382/173144123-bfa55c31-413d-4377-86a2-e4da531d5081.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173144315-ea4fb04c-2352-4648-bc46-703458a7657a.png)||
|F<sub>def</sub>|for other systems<br>![grafik](https://user-images.githubusercontent.com/49325382/173144257-3a009b3a-7905-49cb-98fe-d6851f24fbeb.png)||
|| Single Stage System with Either a Fixed-Speed Indoor Blower or a Constant-Air-Volume-Rate Indoor Blower, or a Single-Speed Coil-Only System Heat Pump||
|![grafik](https://user-images.githubusercontent.com/49325382/173144574-ec155a4e-1917-4456-b348-90e9ba87c3ce.png)|![grafik](https://user-images.githubusercontent.com/49325382/173144598-c7c56aed-afaa-4911-a500-4f45f54af809.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173144622-90dffae7-a817-4f9c-b688-cd4843583643.png)|![grafik](https://user-images.githubusercontent.com/49325382/173144644-54b20a35-6ddd-4ccb-9e54-50f05490a8f1.png)||
||If neither the H4<sub>boost</sub> test nor the H4<sub>full</sub> test is conducted|![grafik](https://user-images.githubusercontent.com/49325382/173145543-d562c0b7-ae25-47b9-9b13-6f43ace9867d.png)|
|![grafik](https://user-images.githubusercontent.com/49325382/173145438-88aaa2c9-cb24-43bd-95ed-e57bae36da6a.png)|![grafik](https://user-images.githubusercontent.com/49325382/173145463-da6d5482-b86c-4511-a1d4-48e47206c18b.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173146291-5635848c-fc68-49d3-86e8-124f5be6b4d0.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173145491-99190c89-1674-4980-aa2f-923006269832.png)|![grafik](https://user-images.githubusercontent.com/49325382/173145503-1b470006-bda0-4a6f-beae-3c8e19219c9b.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173146324-d1728aaa-0476-430f-bdbb-f2b6e99c5dfd.png)||
||If either the H4<sub>boost</sub> or H4<sub>full</sub> test is conducted||
|![grafik](https://user-images.githubusercontent.com/49325382/173145778-de104f9f-f368-46df-9046-74f50aa7bdb6.png)|![grafik](https://user-images.githubusercontent.com/49325382/173145797-4b5258b7-b11f-4218-b2d8-d2e674409c88.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173146706-3d91abf4-8e99-4348-a749-508333692d8b.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173145821-e1177647-fa2e-448d-97b2-abe134088385.png)|![grafik](https://user-images.githubusercontent.com/49325382/173145845-dea513e2-d8bc-4f41-8eb6-972d2f9c27a4.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173146726-a1043835-e19c-45ce-b07f-e44ec1a3a3f4.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173145868-75bd05d0-2964-4491-bd3e-68c80b027a96.png)|![grafik](https://user-images.githubusercontent.com/49325382/173145884-ff9b9f1b-54a2-4b18-aefd-b8293f853713.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173146745-c6a3b2de-d4a1-4442-982f-875858822653.png)||
|E(j<sub>j</sub>|![grafik](https://user-images.githubusercontent.com/49325382/173147003-00395c1c-0399-482b-bda7-09edcaaa99cf.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173147126-494aa8eb-4f35-4fef-8165-77c8cae2d3a0.png)|||
|![grafik](https://user-images.githubusercontent.com/49325382/173147162-0fc3b8ce-0438-4eed-879d-c11be8a75161.png)|![grafik](https://user-images.githubusercontent.com/49325382/173147243-d80ee99c-9829-43a0-b0c2-ba49effe268c.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173147267-6b8fb525-7cce-486c-802d-1a47d0fa70fb.png)|![grafik](https://user-images.githubusercontent.com/49325382/173147308-7d795754-15e0-4d57-8713-077a7e10d7de.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173147296-f0d0ed83-1942-41ae-be17-282a605e6cad.png)|![grafik](https://user-images.githubusercontent.com/49325382/173147331-3ca83043-0388-4b8c-983c-07335cf03603.png)||
||Additional Steps for Calculating the HSPF2 of a Heat Pump Having a Single-Speed Compressor and a Variable-Speed, Variable-Air-Volume-Rate Indoor Blower ||
||Additional Steps for Calculating the HSPF2 for MIB ||

*For Two-Stage Systems*

|Two-Stage Systems|
|--|
|Graphical representation of HSPFs:<br>![grafik](https://user-images.githubusercontent.com/49325382/173147668-100574da-546e-4aee-acfc-3b352de64e53.png)|

|Variable|Formulas|Code|
|--|--|--|
|HSPF2|![grafik](https://user-images.githubusercontent.com/49325382/173148257-b9a2bf26-24c6-41db-8d49-a402f3623fdb.png)|TBD|
||![grafik](https://user-images.githubusercontent.com/49325382/173150846-51c7ca5a-8b80-407c-8c52-9b3b9b284151.png)|![grafik](https://user-images.githubusercontent.com/49325382/173150866-0857b901-bc10-4d21-b311-2be30f75536d.png)|
||![grafik](https://user-images.githubusercontent.com/49325382/173150930-1bdbf0fe-eb55-4385-9579-4cd402279503.png)||
|Case I<br>![grafik](https://user-images.githubusercontent.com/49325382/173151062-826c01ea-e608-4c59-88de-82e482cba0bc.png)|Building load is less than Low Stage capacity|||
||![grafik](https://user-images.githubusercontent.com/49325382/173151119-adc0c360-5e5f-4b68-bc17-0206e34092d7.png)<br>![grafik](https://user-images.githubusercontent.com/49325382/173151205-09079ab5-adad-4749-a0e9-38fd2aa3e27c.png)||
|Case II<br>![grafik](https://user-images.githubusercontent.com/49325382/173151354-94db543b-52a7-4400-a39e-52dbd8543006.png)|Building load is greater than the Low Stage capacity, but less than the Full Stage capacity, and the unit cycles between Low Stage operation and Full Stage operation.||
||![grafik](https://user-images.githubusercontent.com/49325382/173151403-1d906be4-2682-4f0d-bffd-f4ff3a4a1b2d.png)||
|Case III<br>![grafik](https://user-images.githubusercontent.com/49325382/173151523-af30bd15-fd24-4217-a3af-68eb0aac7318.png)|Building load is greater than the Low Stage capacity, but less than the Full Stage capacity, and the unit cycles between off and Full Stage operation.||
||![grafik](https://user-images.githubusercontent.com/49325382/173151557-ca671412-25ab-4e94-8223-9b9a476d4116.png)||
|![grafik](https://user-images.githubusercontent.com/49325382/173151660-63520e45-2322-4d90-8000-86778a5142ff.png)|When the building load is greater than the unit capacity||
||![grafik](https://user-images.githubusercontent.com/49325382/173151685-fa5c0bc4-9b62-4d5f-bf47-5f60131a859b.png)||


*For Variable Speed System*

|Variable Speed System|
|--|
|Graphical representation of HSPFs:<br>![grafik](https://user-images.githubusercontent.com/49325382/173147965-8f0fc7e8-5808-490c-8e7d-f36bc7e8abad.png)|

|Variable|Formulas|Code|
|--|--|--|
|HSPF2|![grafik](https://user-images.githubusercontent.com/49325382/173148257-b9a2bf26-24c6-41db-8d49-a402f3623fdb.png)|TBD|
||![grafik](https://user-images.githubusercontent.com/49325382/173148304-e6399e4d-2be1-4d94-9221-70a5d6e28c4a.png)||
|Case I<br>![grafik](https://user-images.githubusercontent.com/49325382/173150588-e80b6ad4-59c0-4736-9c69-63480e9dbdd3.png)|Building Load is less than the capacity of the unit at the Low Compressor Speed||
|Case II<br>![grafik](https://user-images.githubusercontent.com/49325382/173150412-f1325086-f758-40c8-82c4-116db328732b.png)|Building load can be matched by modulating the compressor speed between low speed and full speed||
||![grafik](https://user-images.githubusercontent.com/49325382/173150460-8e50e95d-ba7a-45c6-98ab-4257ac2382f7.png)||
|Case III<br>![grafik](https://user-images.githubusercontent.com/49325382/173149950-452a7920-0ccd-4ea8-b316-90e563a49037.png)|Building Load is greater than the capacity of the unit at the Full Compressor Speed||
||![grafik](https://user-images.githubusercontent.com/49325382/173150028-3732a2c7-bff3-4616-abb9-e1e3d735cea5.png)|

There are also other variations, but I think they are not needed. E.g., Heat pumps having a Heat Comfort Controller, Heat Pump Having a Two-capacity Compressor or Heat Pump Having a Triple-Capacity Compressor

These temperature bin values changed in the newer version, thus we need to define two arrays one for the older and one for the newer version.

## Approach ##

For each of the two metrics, we will identify a set of EnergyPlus components for which the metric applies. Replicate the code pattern of existing metrics for the new metric and write unit tests to ensure the calculation is correct. Once the metrics are calculated we can integrate them into the corresponding report code of the equipment summary table. For now we want to add those two new metrics (SEER2 and HSPF2) in addition to the existing ones (SEER and HSPF) and explain the difference in a footnote.
- Extend the implementation in StandardRatings.hh & StandardRatings.cc
   - for SEER2 by adding code to the following functions
     - CalcDXCoilStandardRating
     - SingleSpeedDXCoolingCoilStandardRatings
     - DXCoolingCoilDataCenterStandardRatings
     - MultiSpeedDXCoolingCoilStandardRatings
     - ReportDXCoilRating
     - CheckCurveLimitsForStandardRatings
   - for HSPF2 by adding code to the following functions
     - CalcDXCoilStandardRating
     - SingleSpeedDXHeatingCoilStandardRatings
     - MultiSpeedDXHeatingCoilStandardRatings
     - ReportDXCoilRating
     - CheckCurveLimitsForStandardRatings
   - Add unit tests for both metrics into StandardRatings.unit.cc



## Testing/Validation/Data Source(s) ##

Unit tests will confirm and test the calculation of the metrics to ensure the code produces correct results and does not fail.
Two example files will be modified to demonstrate the use of the new metrics. Simulation results will be manually checked/benchmarked using excel spreadsheet with input and output from EnergyPlus runs.

## Input Output Reference Documentation ##

The InputOutput Documentation will be expanded to include the new metrics. There will be an update in two places, first on the related section of the EnergyPlus components for which the metric is calculated and second in the reports section, specifically the Equipment Summary report. Add a displcaimer:
"It is not reasonable to expect AHRI ratings calculated from model inputs to match the actual ratings in the AHRI directory. This is largely because it is common practice for manufacturers to underrate their equipment for marketing reasons and/or to build in some safety margin in case DOE audits their ratings."

## Input Description ##

There is no additional input needed.

## Outputs Description ##

There will be an update to the reports to include the metrics as described above.

## Engineering Reference ##

The EngineeringReference will be updated to include the formulas used to calculate the two metrics and be referenced from the corresponding EnergyPlus components.

## Example Files and Transition Changes ##

No new example files are needed.
No transition change is required.

## E-mail and Conference Call Conclusions ##

from Neal Kruis (Slack): Add disclaimer that it is not "not reasonable to expect AHRI ratings calculated from model inputs to match the actual ratings in the AHRI directory. This is largely because it is common practice for manufacturers to underrate their equipment for marketing reasons and/or to build in some safety margin in case DOE audits their ratings."

from Jason Glazer (PR): "As you are adding these new metrics to the EquipmentSummary report, please leave the existing SEER and HSPF as well and clarify the source of both in footnotes to the tables."

Comments from call on 6/15/2022:

- focus on generic DX cooling coil first and then look into the older specialied coils
- required quic and timely feedback through hardcoding some parameters first to the caluclation going (with some known assumptions at this point)
- look into unit tests with manufacturer data if possible
- test the implementation by comparing the old version of the metrics with the new implementation (are value within anitipated range)


## Acknowledgments ##

TBD

## References ##

[1]	ASHRAE, ANSI/ASHRAE/IES Standard 90.1-2019: Energy Standard for Buildings Except Low-Rise Residential Buildings. 2019.

[2]	Jonestone Supply: The SEER2 Guide. 2022. URL: https://seer2.com/

[3] AHRI: Air Conditioning, Heating & Refrigeration Institute. 2023 (2020) Standard for Performance Rating of Unitary Air-conditioning & Air-source Heat Pump
Equipment. 2020. URL: https://www.ahrinet.org/Portals/_Appleseed/documents/Standards/AHRI%20Standard%20210.240-2023%20(2020).pdf

[4] AHRI: AHRI Standard 340/360-2022 (I-P): 2022 Standard for Performance Rating of Commercial and Industrial Unitary Air-conditioning and Heat Pump Equipment. 2019. URL: https://www.ahrinet.org/Portals/Standards/AHRI%20Standard%20340-360-2022%20(I-P).pdf

[5] ANSI/AHAM: ANSI/AHAM RAC-1-2015: Room Air Conditioners. 2015. https://webstore.ansi.org/standards/aham/ansiahamrac2015

[6] ANSI/AHAM: ANSI/AHRI 910-I-P-2014: Performance Rating of Indoor Pool Dehumidifiers. 2014. http://www.ahrinet.org/App_Content/ahri/files/STANDARDS/AHRI/AHRI_Standard_910_I-P_2014.pdf

[7] HPAC Engineering. Understanding AHRI 920 and ISMRE Ratings for Most Efficient DOAS Specification. 2018. https://www.hpac.com/columns/managing-facilities/article/20929562/understanding-ahri-920-and-ismre-ratings-for-most-efficient-doas-specification

[8] ANSI/AHAM: ANSI/AHRI 920-I-P-2015: 2015 Standard for Performance Rating of DX-Dedicated Outdoor Air System Units. 2015. https://www.ahrinet.org/App_Content/ahri/files/STANDARDS/ANSI/ANSI_AHRI_Standard_920_I-P_2015.pdf

[9] 10 CFR Appendix E to Subpart B of Part 430 - Uniform Test Method for Measuring the Energy Consumption of Water Heaters. 2016. https://www.govinfo.gov/app/details/CFR-2016-title10-vol3/CFR-2016-title10-vol3-part430-subpartB-appE

[10] ANSI/AMCA Standard 208, Calculation of the Fan Energy Index. 2019. https://www.amca.org/news/press-releases/ansi/amca-standard-208,-calculation-of-the-fan-energy-index,-available-for-free-download.html&usg=AOvVaw2eZxHxgdq183Y9WsZlgjTB

[11] Differences between SEER and HSOPF and SEER2 and HSPF2 https://www.ahrinet.org/Portals/AHRI%20CAC%20HP%20Appendix%20M1%20Presentation%20-%20AHR%20Expo.pdf
