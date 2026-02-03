Automatic Import of Shading from a CSV File
================
**Tianzhen Hong, Xuan Luo, LBNL**

**Original: November 2, 2017**

# Justification for New Feature

EnergyPlus version 8.8 added features to export the calculated shading results to an external CSV file, and allow to use shading schedules to replace internal shading calculations for exterior surfaces. Although the shading schedules can be imported from external CSV files, there needs to define all these schedules in IDF files explicitly as well as adding SurfaceProperty:LocalEnvironment objects for all exterior surfaces, which can be a significant burden for users especially for a model with many exterior surfaces. The proposed new feature will fully automate the process.

# Team Discussion

N/A

# Overview

EnergyPlus v8.8 allows to export all shading calculation results (shading fractions) to an external CSV file “eplusshading.csv”, when the field “Output External Shading Calculation Results” of the object ShadowCalculation is set to Yes. The CSV file has the time-series of shading fractions with exterior surface names represented as the column headers.

# Approaches

We propose to add a new option “ImportedShading” to the field “External Shading Calculation Method” of the ShadowCalculation object, and a new filed “External Shading File Name” to specify the CSV file containing the shading fraction values for exterior surfaces. The default file name is eplusshading.csv, which is the default file name of the exported EnergyPlus shading calculation results. If an exterior surface name is missing from the header of the CSV file, the assumption is no shading.

	ShadowCalculation,
	  A4 , \field External Shading Calculation Method
	       \type choice
	       \key ScheduledShading
	       \key InternalCalculation
	       \key ImportedShading
	       \default InternalCalculation
	       \note If ScheduledShading is chosen, the External Shading Fraction Schedule Name in the SurfaceProperty:LocalEnvironment object is required for each exterior surface.
	       \note If ImportedShading is chosen, the External Shading File Name is required.
	  A5 , \field Output External Shading Calculation Results
	       \type choice
	       \key Yes
	       \key No
	       \default No
	       \note If Yes is chosen, the calculated external shading fraction results will be saved to an external CSV file eplusshading.csv with surface names as the column headers.
	  A6 ; \field External Shading File Name
	       \type alpha
	       \retaincase
	       \note This is required if ImportedShading is chosen.
	       \note default file name is "eplusshading.csv"
	       \note each column header of the CSV file has to be the name of an exterior surface

# Testing/Validation/Data Sources

Create two IDF files and compare simulation outputs. One IDF file has shading schedules defined explicitly while the other file uses the new feature to automate the import of shading results from an external CSV file. Results from both IDF files should be the same.

# Input Output Reference Documentation

Update the IORef with the proposed IDD changes.

# Engineering Reference

N/A

# Output Details and Examples

N/A

# Example Files and Transition Changes

No transition changes are envisioned as a result of the new feature. A new example file will be developed to demonstrate the new feature.

# References

N/A
