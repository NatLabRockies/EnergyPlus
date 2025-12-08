Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### EIO and Initialization Summary Report Changes for WindowConstruction

Two new fields were added to the EIO report for `<WindowConstruction>` entries:
`Conductance (Before Adjusted) {W/m2-K}` and `Convection Coefficient Adjustment Ratio`.
See pull request [#9117](https://github.com/NREL/EnergyPlus/pull/9117/files) for more details.


### EIO and Initialization Summary Report Changes for Daylighting

Daylighting outputs in the EIO and Initialization Summary Report which were by Zone are now by Enclosure or Daylighting Control.
This changes the headers and tags. It may also change the number, naming, and order of entries for each section.

Also, for Sky Daylight Factors, "Sky Type" was missing from the header.

```
! <Zone/Window Adjacency Daylighting Counts>, Zone Name, Number of Exterior Windows, Number of Exterior Windows in Adjacent Zones
Zone/Window Adjacency Daylighting Counts, ZN_1_FLR_1_SEC_1,1,0

! <Zone/Window Adjacency Daylighting Matrix>, Zone Name, Number of Adjacent Zones with Windows,Adjacent Zone Names - 1st 100 (max)
Zone/Window Adjacency Daylighting Matrix, ZN_1_FLR_1_SEC_1,0

! <Sky Daylight Factors>, MonthAndDay, Zone Name, Window Name, Reference Point, Daylight Factor
Sky Daylight Factors,Clear Sky,01/21,ZN_1_FLR_1_SEC_1,ZN_1_FLR_1_SEC_1_WALL_1_WINDOW_1,ZN_1_FLR_1_SEC_1_DAYLREFPT1,0.1352


```

are now

```
! <Enclosure/Window Adjacency Daylighting Counts>, Enclosure Name, Number of Exterior Windows, Number of Exterior Windows in Adjacent Enclosures
Enclosure/Window Adjacency Daylighting Counts, ZN_1_FLR_1_SEC_1,1,0

! <Enclosure/Window Adjacency Daylighting Matrix>, Enclosure Name, Number of Adjacent Enclosures with Windows,Adjacent Enclosure Names - 1st 100 (max)
Enclosure/Window Adjacency Daylighting Matrix, ZN_1_FLR_1_SEC_1,0

! <Sky Daylight Factors>, Sky Type, MonthAndDay, Daylighting Control Name, Enclosure Name, Window Name, Reference Point, Daylight Factor
Sky Daylight Factors,Clear Sky,01/21,ZN_1_FLR_1_SEC_1_DAYLCTRL,ZN_1_FLR_1_SEC_1,ZN_1_FLR_1_SEC_1_WALL_1_WINDOW_1,ZN_1_FLR_1_SEC_1_DAYLREFPT1,0.1352
```

See pull request [#9102](https://github.com/NREL/EnergyPlus/pull/9102/files) for more details.


### DXF Changes for Daylighting Reference Points

The following changes were made to Daylighting reference points in dxf output:
  * Duplicate daylighting reference points are removed
  * The actual reference point names are used in the label, rather than RefPt1, 2, etc.

See pull request [#9102](https://github.com/NREL/EnergyPlus/pull/9102/files) for more details.

### DFS Changes for Daylighting

For `Output:DaylightFactors`, the daylight factors are now reported by Enclosure instead of by Zone, and Enclosure Name is added as a new field along with Zone Name.
This may alter the order and number of entries.

```
This file contains daylight factors for all exterior windows of daylight zones.
MonthAndDay,Zone Name,Window Name,Window State
Hour,Reference Point,Daylight Factor for Clear Sky,Daylight Factor for Clear Turbid Sky,Daylight Factor for Intermediate Sky,Daylight Factor for Overcast Sky
01/21,PERIMETER_BOT_ZN_1,PERIMETER_BOT_ZN_1_WALL_SOUTH_WINDOW,Base Window
```

are now

```
This file contains daylight factors for all exterior windows of daylight enclosures.
MonthAndDay,Enclosure Name,Zone Name,Window Name,Window State
Hour,Reference Point,Daylight Factor for Clear Sky,Daylight Factor for Clear Turbid Sky,Daylight Factor for Intermediate Sky,Daylight Factor for Overcast Sky
01/21,PERIMETER_BOT_ZN_3,PERIMETER_BOT_ZN_3,PERIMETER_BOT_ZN_3_WALL_NORTH_WINDOW,Base Window
```

See pull request [#9102](https://github.com/NREL/EnergyPlus/pull/9102/files) for more details.

### Row Order Changes for Lighting Summary, Daylighting subtable

The row order may change in this subtable.

See pull request [#9102](https://github.com/NREL/EnergyPlus/pull/9102/files) for more details.

### Emission Meter Renaming

A number of meters in the emission system were renamed to use "Electricity" instead of "Electric".  The meters that were renamed are:

- ElectricEmissions:CO2 -> ElectricityEmissions:CO2
- ElectricEmissions:CO -> ElectricityEmissions:CO
- ElectricEmissions:CH4 -> ElectricityEmissions:CH4
- ElectricEmissions:NOx -> ElectricityEmissions:NOx
- ElectricEmissions:N2O -> ElectricityEmissions:N2O
- ElectricEmissions:SO2 -> ElectricityEmissions:SO2
- ElectricEmissions:PM -> ElectricityEmissions:PM
- ElectricEmissions:PM10 -> ElectricityEmissions:PM10
- ElectricEmissions:PM2.5 -> ElectricityEmissions:PM2.5
- ElectricEmissions:NH3 -> ElectricityEmissions:NH3
- ElectricEmissions:NMVOC -> ElectricityEmissions:NMVOC
- ElectricEmissions:Hg -> ElectricityEmissions:Hg
- ElectricEmissions:Pb -> ElectricityEmissions:Pb
- ElectricEmissions:WaterEnvironmentalFactors -> ElectricityEmissions:WaterEnvironmentalFactors
- ElectricEmissions:Nuclear High -> ElectricityEmissions:Nuclear High
- ElectricEmissions:Nuclear Low -> ElectricityEmissions:Nuclear Low
- PurchasedElectricEmissions:Source -> PurchasedElectricityEmissions:Source
- SoldElectricEmissions:Source -> SoldElectricityEmissions:Source

See pull request [#9101](https://github.com/NREL/EnergyPlus/pull/9101) for more details.


### Emission Variable Renaming

Two variables in the emission system were renamed to use "OtherFuel" instead of "CO2".  The variables that were renamed are:

- Environmental Impact OtherFuel1 CO2 Water Consumption Volume -> Environmental Impact OtherFuel1 Water Consumption Volume
- Environmental Impact OtherFuel2 CO2 Water Consumption Volume -> Environmental Impact OtherFuel2 Water Consumption Volume

See pull request [#9089](https://github.com/NREL/EnergyPlus/pull/9089) for more details.

### Addition of Monthly Precipitation Reporting Table

A tabular report of precipitation related variables was added to the
Climatic Data Summary section. It summarizes the following quantities
- Monthly Total Precipitation in mm from the weather file
- Monthly Total Hours of Rain in mm from the weather file
- Monthly Total Precipitation in Site:Precipitation [mm]
- Monthly Total Roof Irrigation Depth in mm
- Monthly Total Rain Collection Volume in m3

Three rows are added to the Weather Statistics File reporting table:
- Annual Total Precipitation [mm]
- Max Hourly Precipitation [mm]
- The month in which max hourly precipitation occurs

See pull request [#9177](https://github.com/NREL/EnergyPlus/pull/9177) for more details.

### Rename output variable "Environment:Site Precipitation Depth"

Renamed the output variable "Environment:Site Precipitation Depth" to "Environment:Liquid Precipitation Depth".

See pull request [#9177](https://github.com/NREL/EnergyPlus/pull/9177) for more details.
### Fenestration Assembly Reporting

Added several items to Envelope Summary in tabular output report:

- For Exterior Fenestration subtable, added Frame and Divider Name
- For Exterior Fenestration subtable, added NFRC product type, assembly u-factor, assembly SHGC, assembly visible transmittance
- Added new Exterior Fenestration Shaded State subtable by construction listing some of the same items as the Exterior Fenestration subtable but when the shading is deployed.

Also added two new EIO lines showing same additions as were added to the tabular output report:

- FenestrationAssembly
- FenestrationShadedState

See pull request [#8740](https://github.com/NREL/EnergyPlus/pull/8740) for more details.
See pull request [#9177](https://github.com/NREL/EnergyPlus/pull/9177) for more details.


### Exhaust System New Output Variables

Five new output variables related to the AirLoopHVAC:Exhaust System have been added along with the new feature development. These new variables are:

- Central Exhaust Fan Mass Flow Rate [kg/s]
- Central Exhaust Fan Volumetric Flow Rate Standard [m3/s]
- Central Exhaust Fan Volumetric Flow Rate Current [m3/s]
- Central Exhaust Fan Power [W]
- Central Exhaust Fan Energy [J]

See pull request [#9209](https://github.com/NREL/EnergyPlus/pull/9209) for more details.

### EIO Sizing information for Central Exhaust Fan
One line is added to the EIO sizing report regarding the central exhaust fan "Design Fan Airflow [m3/s]" sizing containing the following information:
- FanName, FanType, "Design Fan Airflow [m3/s]", Design Flow Rate Value

See pull request [#9209](https://github.com/NREL/EnergyPlus/pull/9209) for more details.
