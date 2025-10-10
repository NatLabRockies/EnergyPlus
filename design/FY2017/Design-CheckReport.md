Design Document - Checksums Tabular Report
================

**Jason Glazer, GARD Analytics, Inc.**

 - November 21, 2016


## New Feature Proposal

See the file NFP-CheckReport.md file for details on the justification, overview, and output description.

## Overview

The current files that are involved in the production of the Zone Component Load Summary report are:

1. OutputReportTabular.cc [WriteZoneLoadComponentTable(), ComputeLoadComponentDecayCurve(), GatherComponentLoadsSurface(), GatherComponentLoadsHVAC(),ComputeDelayedComponents()]

2. HeatBalanceSurfaceManager.cc [GatherComponentLoadsSurfAbsFact(), InitIntSolarDistribution(), ReportSurfaceHeatBalance(), CalcHeatBalanceInsideSurf() ]

3. SizingManger.cc [ManageSizing()]

These same routines will largely be reused to add to the current Zone Component Load Summary report as well as new routines to produce the Airloop Load Component Summary and Facility Load Component Summary reports.

Ventilation loads will not be included for the Zone Component Load Summary or Facility Load Component Summary reports since they are not being calculated during zone sizing calculations. They will be included for the Airloop Load Component Summary report.

## Associating Areas with Load Components

For each row of the Estimated Peak Load Components tables that make sense to be associated with an area those areas will be shown as well as the total load per area. The areas for people, lights, equipment and other internal loads will use the floor area of the zone while areas related to specific surfaces such as exterior wall, ground Contact wall, etc., will be use the appropriate area for that function. The areas will be pulled from the appropriate array.

## Select Peak Times for AirLoop and Facility

Similar to the zone sizing results, the system sizing results are stored in the FinalSysSizing() array that contains the name of the cooling and heating design days. The particular time is not currently stored in that array but will be added. No facility level sizing summary is currently performed in EnergyPlus but sum of all zone peak  will be combined to derive a facilty sensible load peak time.

## Computation of Other Peak Component Loads

Currently, the WriteZoneLoadComponentTable() routine in OutputReportTabular.cc repeats calculations related to the impact of the for each zone for the selected heating and cooling design day and peak time. This routine will be refactored so that the calculation can be repeated for any design day and at any peak time. This will enable the same routine to be used for the computation of the peak components for the zone at the zone peak as well as the airloop peak and facility peak using the appropriate peak time.

## Associating Zones with AirLoops

The DataAirLoop.hh defines a struct for the AirLoopZoneInfo array that associates a list of zones with each airloop. The list of zones will appear in the AirLoops Component Loads Summary report as a table of zones.

## Computing Other Check values

The new results fields in the Peak Conditions and Engineering Checks tables are either directly available from the sizing routine results or are derivable by combining those values with appropriate areas or flowrates.

