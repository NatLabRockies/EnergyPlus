Design Document - Provide Additional LEED Related Features
================

**Jason Glazer, GARD Analytics, Inc.**

 - July 24, 2017


## New Feature Proposal

See the file [NFP-ProvideAdditionalLEEDrelatedFeatures.md](https://github.com/NatLabRockies/EnergyPlus/blob/Update_LEED_Reporting/design/FY2017/NFP-ProvideAdditionalLEEDrelatedFeatures.md) file for details on the justification, overview, and output description.

## Adding Support for End-Use Subcategory in More Objects

The IDD file will be modified to include the new end-use subcategory field for the eight new input objects. Setting up the end-use subcategory requires small changes to the getinput routines in the various files for the new objects that will have that field added. The end-use subcategory string is read in getinput and is used in SetupOutputVariable calls. The files that will be modified are:

 - Boilers.cc (for Boiler:HotWater and Boiler:Steam)
 - ChillerElectricEIR.cc (for Chiller:Electric:EIR)
 - ChillerReformulatedEIR.cc (Chiller:Electric:ReformulatedEIR)
 - PlantChillers.cc (for Chiller:Electric)
 - CondenserLoopTower.cc (for CoolingTower:SingleSpeed, CoolingTower:TwoSpeed, CoolingTower:VariableSpeed, CoolingTower:VariableSpeed:Merkel)
 - WaterThermalTaks.cc (for WaterHeater:Mixed)

## Adding the Schedule-EFLH Table

This table will be added to the LEED Summary report which is produced in OutputReportPredefined.cc file and the OutputReportTabular.cc file. The added table will be defined in the OutputReportPredefined.cc in the SetPredefinedTables() routine. The entries of values will be from a new routine called from FillRemainingPredefinedEntries() in the OutputReportTabular.cc. The call will reference a new routine called ScheduleAnnualFullLoadHours() which will be based on the code in ScheduleAverageHoursPerWeek() which currently calculates the same value on a weekly basis. The ScheduleAverageHoursPerWeek() routine will be refactored to use the new ScheduleAnnualFullLoadHours() routine to remove duplicate code. Additionally, a new routine that counts the hours in a year that have a value greater than zero will be added. This will be performed for all schedules that use schedule-type=fraction. The name of the schedule will be included.


## Adding the Schedule SetPoints Table

This table will be added to the LEED Summary report which is produced in OutputReportPredefined.cc file and the OutputReportTabular.cc file. The added table will be defined in the OutputReportPredefined.cc in the SetPredefinedTables() routine. The entries of values will be from a new routine called from FillRemainingPredefinedEntries() in the OutputReportTabular.cc. For each of the following objects:

- ThermostatSetpoint:SingleHeating
- ThermostatSetpoint:SingleCooling
- ThermostatSetpoint:SingleHeatingOrCooling
- ThermostatSetpoint:DualSetpoint

The schedule will be examined by a new routine that returns the value at a specific time of day and if that value is the same for the entire year for that value or not. This new routine would be part of the ScheduleManager.cc file. This will be performed once per schedule name. This routine will be called for 11am Monday and 11pm Monday to complete the table.


## Modifying the EAp2-4/5 Performance Rating Method Compliance Table

This table is similar to the ABUPS End-Use report but includes energy and demand on the same table and only has electricity, natural gas and other as columns. It is currently created primarily in the WriteBEPSTable() routine. It includes algorithms that explicitly remove some end use subcategories for certain end-uses to try to match USGBC forms. Instead a more generic approach will be taken and each end-use subcategory will be shown individually. The sum of the end-use subcategories will be subtracted from the end-use and that will be reported as all other energy used for that end-use. In addition, all code related to keeping these special end use subcategories separated for the LEED report will be removed in favor of this more generic solution.
