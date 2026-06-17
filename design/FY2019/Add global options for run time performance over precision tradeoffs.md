# Add Global Options for Run Time Performance over Precision Tradeoffs

Xuan Luo, Tianzhen Hong

Lawrence Berkeley National Laboratory

July 1, 2019

## Justification for Feature Update

We propose a new global object “PerformancePrecisionTradeoffs” as one of the objects for EnergyPlus global setting controls. This object enables users to choose certain options that speed up EnergyPlus simulations, but may lead to small decreases in the accuracy of results.

## Overview

The object is design to provide users with options for performance over precision tradeoffs for existing or future EnergyPlus features. Some techniques and algorithms used in EnergyPlus that help achieve run time performance gain may trigger some small differences in the reported results. The options may include using the caching techniques for some utility functions, and using simplified algorithms for some calculation routines. Table 1 summarizes the performance/precision tradeoffs of two psychrometric functions that currently use the caching technique to save run time. The summary is based the regression test results with 623 EnergyPlus test files. The max diff in annual energy report indicates the maximum error in the annual energy reports in the summary table, out of the total 623 tests. The math diffs indicate the differences in the reported variables and meters by hour or by time step. The tables diffs indicate the differences in the final report tables.

**Table 1. Performance/precision tradeoffs of two psychrometric functions**

Cache flag | Run time performance gain of caching | Max diff in annual energy reports | Number of tests have large math diffs | Number of tests have small math diffs | Number of tests have large table diffs | Number of tests have small table diffs
---------- | ------------------------------------ | --------------------------------- | -------------------------------------  | ------------------------------------ | ----------------------------  | ---------------------------------------
EP_cache_PsyTwbFnTdbWPb  | 2.33% | 0.14% | 39 | 381 | 95 | 572
EP_cache_PsyPsatFnTemp   | 2.20% | 0.19% | 40 | 487 | 87 | 577

The global object, PerformancePrecisionTradeoffs, provides flags to turn performance tricks like these on during regular simulation runs that do not have strict precisions requirement.


## IDD Object (New):

We propose to add a new object to the IDD for global simulation control：

	PerformancePrecisionTradeoffs,
	      \unique-object
	      \memo This object enables users to choose certain options that speed up EnergyPlus simulation,
	      \memo but may lead to small decreases in accuracy of results.
	  A1; \field Use Coil Direct Solutions
	      \note If Yes, an analytical or empirical solution will be used to replace iterations in
	      \note the coil performance calculations.
	      \type choice
	      \key Yes
	      \key No
	      \default No


## IDD Object(s) (Revised):

N/A

## Proposed additions to Meters:

N/A

## Proposed Report Variables:

N/A

## IO Ref (draft):

To be developed.

## EngRef (draft):

To be developed.
