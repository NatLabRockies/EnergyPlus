EnergyPlus Sizing Reporting Enhancements - Design
==========================

M.J. Witte, February 26, 2018

# Overview #
This design document describes a code contribution from Trane with modifications by GARD to add two new table output reports:

- *Coil Sizing Details Report* - Detailed coil sizing data table contributed by Trane. Report key is "CoilSizingDetails"
- *Coil Sizing Summary subtable in the HVAC Sizing SummaryReport* - This is a subset of the Coil Sizing Details Report.

# New Classes #

Two new classes are added in source file `ReportCoilSizing.hh/cc`:

`CoilSelectionData` holds detailed sizing data for each coil. For some coil types, there will be data fields that are not populated at all or filled with -999 or N/A.

`ReportCoilSelection` holds a set of functions to create and populate the `CoilSelectionData` objects and to write the data to predefined table reports for output.

New functions are:

`createCoilSelectionReportObj` - called from `SimulationManager::ManageSimulation`

`clearCoilSelectionReportObj` - available for unit tests

`finishCoilSummaryReportTable` - called from `OutputReportTabular::WriteTabularReports`

    doFinalProcessingOfCoilData - fill in some of the data fields
	writeCoilSelectionOutput - write the full Coil Sizing Details table
	writeCoilSelectionOutput2 - write the abbreviated Coil Sizing subtable

A variety of public functions called from the various coil sizing routines to populate the data:

	setCoilFinalSizes
	setRatedCoilConditions
	setCoilAirFlow
	setCoilWaterFlowNodeNums(
	setCoilWaterFlowPltSizNum
	setCoilEntAirTemp
	setCoilEntAirHumRat
	setCoilEntWaterTemp
	setCoilLvgWaterTemp
	setCoilWaterDeltaT
	setCoilLvgAirTemp
	setCoilLvgAirHumRat
	setCoilCoolingCapacity
	setCoilHeatingCapacity
	setCoilWaterCoolingCapacity
	setCoilWaterHeaterCapacityNodeNums
	setCoilWaterHeaterCapacityPltSizNum
	setCoilUA
	setCoilReheatMultiplier
	setCoilSupplyFanInfo
	getTimeText
	isCompTypeFan
	isCompTypeCoil
	setZoneLatentLoadCoolingIdealPeak
	setZoneLatentLoadHeatingIdealPeak

Some private functions:

	doAirLoopSetup
	doZoneEqSetup
	doFinalProcessingOfCoilData
	writeCoilSelectionOutput
	writeCoilSelectionOutput2
	getIndexForOrCreateDataObjFromCoilName

# Modifications to Existing Functions

Every coil type calls various `coilSelectionReportObj` functions from their respective init or sizing functions.

`ReportSizingManager::RequestSizing` has new code to set various strings for design day name and date/time, and also makes calls to various `coilSelectionReportObj` functions.

Every HVAC parent for zone equipment and unitary equipment calls `coilSelectionReportObj` functions to set information about the supply fan serving each coil. e.g. `FanCoilUnits::GetFanCoilUnits`.

`OutputReportPredefined::SetPredefinedTables` defines the two new reports and the data variables to hold them.

`OutputReportTabular::WriteTabularReports` calls `finishCoilSummaryReportTable`, `setZoneLatentLoadCoolingIdealPeak`, and `setZoneLatentLoadHeatingIdealPeak`.


`SimAirServingZones::UpdateSysSizing` has new code to save the date/time of system peaks and some additional details about zone loads and conditions at the time of the system peaks.

`EnergyPlusFixture::SetUp` add a call to `createCoilSelectionReportObj`.

Other unit tests which don't use `EnergyPlusFixture` may also need a call to `createCoilSelectionReportObj`.