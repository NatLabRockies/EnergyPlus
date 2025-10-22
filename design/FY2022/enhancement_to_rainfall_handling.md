Enhancement of Handling of Rainfall
================

**Xuan Luo, Yujie Xu, Tianzhen Hong**

**Lawrence Berkeley National Laboratory**

 - Original Date: November 5, 2021
 - Modified Date: November 8, 2021

## Justification for New Feature ##

There are three modules that currently allow rainfall/liquid precipitation and water collection inputs in EnergyPlus:
1. Rainfall flag and liquid precipitation amounts on the EPW files
1. *Site:Precipitation* object
1. *RoofIrrigation* object

Among them, rainfall flags in the weather file are used for the outside surface heat balance; *Site:Precipitation* is used for rainwater harvesting (in the *WaterUse:RainCollector* object) and the green roof; RoofIrrigation is used for the green roof; and precipitation amounts in the weather file are not used at all.
We propose the enhancement to integrate the use of weather file hourly precipitation amounts along with two existing input objects (*Site:Precipitation* and RoofIrrigation) to model rainwater harvesting and green roof runoff. If the *Site:Precipitation* values are presented, the values should be used. Otherwise, if there is no *Site:Precipitation* object, then values from the weather file should feed the *WaterUse:RainCollector* object and the green roof object.

## Overview ##

The *Site:Precipitation* values are used in two places in EnergyPlus, namely the rain collector and the roof irrigation. The rain collector, modeled by *WaterUse:RainCollector* object, is used for harvesting rainwater falling on building surfaces. The rainwater is then sent to a *WaterUse:Storage* object. Currently, the use of the rain collector object requires the inclusion of a *Site:Precipitation* object to describe the rates of rainfall. In the green roof module, the soil energy heat released or gained is calculated based on the phase changes of soil water, precipitation heat flux and heat flux due to vertical transport of water in the soil are ignored. The precipitation value is added to the surface soil moisture variable if a *Site:Precipitation* schedule exists. In particular, the green roof run off rate is calculated based on the irrigation rate on top of the precipitation rate.

As local weather stations are providing hourly precipitation for the past decades and the values are being updated in TMY3 data, we propose to modify the logic of using rainfall/liquid precipitation to use the EPW file hourly precipitation data for rain harvesting and green roof when no *Site:Precipitation* schedule exists. In EPW files, the field of “Liquid Precipitation Depth” holds the amount of liquid precipitation (mm) observed at the indicated time for the period indicated in the liquid precipitation quantity field. Used in EnergyPlus simulation, if this value is not missing, it is used and overrides the “precipitation” flag as rainfall. Conversely, if the precipitation flag shows rain and this field is missing or zero, it is set to 1.5 (mm).

## Approach ##

When a *WaterUse:RainCollector* or a *RoofIrrigation* object is defined without a *Site:Precipitation* object presented, we will use the hourly precipitation value defined in the EPW file instead. At the same time, warning messages will be shown indicating EPW precipitation is used in the calculation instead of the *Site:Precipitation* and that the users should make sure the precipitation data in the weather file is valid. When every entry of the liquid precipitation depth is missing in the 8760 hourly period, we will also throw a warning and use the precipitation flag to estimate the precipitation quantity (as 1.5mm). To facilitate more transparency and easier debugging of RainCollector and RoofIrrigation calculation, annual total precipitation, total hours of rainfall will be added to the "Climatic Data Summary" output table.

## Testing/Validation/Data Source(s): ##

Two example files (the *EcoroofOrlando.idf* and the *5ZoneWaterSystems.idf*) will be modified to demonstrate the use of the new feature. Simulation results will be manually checked/benchmarked by comparing the results of using *Site:Precipitation* and *EPW hourly data*.

## IDD Object changes: ##

We will modify the memo in WaterUse:RainCollector that a Site:Precipitation object is not necessarily required anymore.

    WaterUse:RainCollector,
        \memo Used for harvesting rainwater falling on building surfaces. The rainwater is sent to a
        \memo WaterUse:Storage object. ~~In order to use this object it is necessary to also include
        \memo a Site:Precipitation object to describe the rates of rainfall.~~
        \extensible:1 - repeat last field

## Proposed additions to Meters: ##

N/A

## Proposed Report Variables: ##

N/A

## References ##

N/A
