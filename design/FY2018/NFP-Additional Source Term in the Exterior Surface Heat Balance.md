Additional Source Term in the Exterior Surface Heat Balance
================
**Tianzhen Hong, Xuan Luo, LBNL**

**Original: November 4, 2017**

# Justification for New Feature
Some heat transfer processes can be complicated and not necessary the required weather parameters (e.g., rain, ice on roof) are available in the commonly used TMY weather files. Currently EnergyPlus does not support importing the pre-calculated results of these heat transfer processes. The proposed new feature will allow an additional source term to be included in the surface heat balance calculation.

# Team Discussion

Proposed and discussed briefly at the annual EnergyPlus team conference call.

# Overview

The new feature enables importing the pre-calculated results of other heat transfer processes, such as evaporative cooling envelope. An additional heat source term defined as a surface property would enable the consideration of these processes to be imported as schedules in the exterior surface heat balance calculation in EnergyPlus.

# Approach

We propose to add a new object SurfaceProperty:HeatBalanceSourceTerm which specifies an surface with the additional heat source term described by a schedule of heat rates (can be positive or negative values) in W/m<sup>2</sup>. The heat rates are pre-calculated outside EnergyPlus. A heat source can be added to either or both the inside and outside of the same surface.

	SurfaceProperty:HeatBalanceSourceTerm,
		   \memo Allows an additional heat source term to be added to the inside or outside surface boundary.
		   \memo A heat source can be added	to either or both the inside and outside of the same surface.
		   \min-fields 3
	  A1 , \field Surface Name
		   \required-field
		   \type object-list
		   \object-list SurfaceNames
	  A2 , \field Inside Surface Heat Source Term Schedule Name
		   \type object-list
		   \object-list ScheduleNames
		   \note The value of this schedule is the source term value for the inside face of this surface
		   \note If this field is left blank, no inside surface source term will be applied.
		   \note The schedule values are heat rate per surface area (W/m2), when positive
		   \note schedule values indicate heat gain and negative values indicates loss.
	  A3 ; \field Outside Surface Heat Source Term Schedule Name
		   \type object-list
		   \object-list ScheduleNames
		   \note The value of this schedule is the source term value for the outside face of this surface
		   \note If this field is left blank, no outside surface source term will be applied.
		   \note The schedule values are heat rate per surface area (W/m2), when positive
		   \note schedule values indicate heat gain and negative values indicates loss.


# Testing/Validation/Data Sources

Manually inspect EnergyPlus outputs of the new example file.

# Input Output Reference Documentation

Update the IORef with the proposed IDD changes.

# Engineering Reference

N/A

# Output Details and Examples

N/A

# Example Files and Transition Changes

No transition changes are envisioned as a result of the new feature. A new example file will be developed to demonstrate the new feature.

# References

S.E.G. Jayamaha, N.E. Wijeysundera, S.K. Chou. Effect of rain on the heat gain through building walls in tropical climates, Building and Environment, 32(5): 465-477, 1997.

L. Zhang, R. Zhang, Y. Zhang, T. Hong, Q. Meng, Y. Feng, The impact of evaporation from porous tile on roof thermal performance: A case study of Guangzhou's climatic conditions. Energy and Buildings, 2016.
