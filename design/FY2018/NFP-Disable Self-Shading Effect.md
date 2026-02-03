Disable Self-Shading Effect
================
**Tianzhen Hong, Xuan Luo, LBNL**

**Original: November 2, 2017**

# Justification for New Feature

Building energy modeling for LEED certification requires to disable self-shading effect in the baseline model, which gives credit to the proposed building with self-shading. For example, an L-shape building that is oriented correctly can provide significant self-shading and ASHRAE Standard 90.1 wants to reward that design choice. The proposed new feature will allow users to disable self-shading effect in EnergyPlus.

# Team Discussion

Some feedback from Mike Witte and Jason Glazer.

# Overview

Currently in EnergyPlus, if the Solar Distribution is not set to MinimalShadowing, shading from detached shading, wings, overhangs, and exterior surfaces of all zones are computed. To disable self-shading effect, we would bypass the shading by exterior surfaces of all zones, which are by default created in EnergyPlus: one shading surface for each exterior surface of all zones. This is straightforward, except when a single IDF file contains two or more detached buildings (which cannot be detected by EnergyPlus). In this case, the shading of one building’s exterior surfaces on other buildings should be counted even when self-shading is disabled.

# Approaches

We propose to add a fields to the ShadowCalculation object to disable self-shading effect from exterior surfaces from all zones, or from a subset of zones. To address the cases with multiple buildings in a single EnergyPlus model (IDF file), two flags are defined to enable the maximal flexibility of various interpretation of self-shading: one to disable shading between zones of a same zone group, the other to disable shading between different zone groups.

	ShadowCalculation,
	  A6 , \field Disable Shading Within A Zone Group
	       \note If Yes, for all surfaces in a targeted Zone Group,
	       \note the shading effect by all other exterior surfaces of all zones within this Zone Group is disabled.
	       \note The zones in a self-shading group will not self-shade each other, but they will cast shadows on all other zones in the model.
	       \note Any zones not listed in a self-shading group will shade any zone in the model.
	       \note If either Disable Shading Within A Zone Group = Yes or Disable Shading Between Zone Groups = Yes, but not both,
	       \note specify Self-Shading Groups to limit self-shading.
	       \note If both Disable Shading Within A Zone Group and Disable Shading Between Zone Groups are Yes,
		   \note all self-shading effect by all zones will be disabled.
	       \type choice
	       \key Yes
	       \key No
	       \default No
	  A7 , \field Disable Shading Between Zone Groups
	       \note If Yes, for all surfaces in a targeted Zone Group,
	       \note the shading effect by other exterior surfaces of all zones in other Zone Groups is disabled.
	       \note If either Disable Shading Within A Zone Group = Yes or Disable Shading Between Zone Groups = Yes, but not both,
	       \note specify Self-Shading Groups to limit self-shading.
	       \note The zones in a self-shading group will still self-shade each other, but they will not cast shadows on all other zones in the model.
	       \note Any zones not listed in a self-shading group will shade any zone in the model.
	       \note If both Disable Shading Within A Zone Group and Disable Shading Between Zone Groups are Yes,
		   \note all self-shading effect by all zones will be disabled.
	       \type choice
	       \key Yes
	       \key No
	       \default No
	  A8 , \field Shading Group 1 ZoneList Name
	       \note The shading zones group specifies the self-shading group defined in field:Disable Shading Within A Zone Group and
	       \note field:Disable Shading Between Zone Groups
	       \type object-list
	       \object-list ZoneListNames
	  A9 , \field Shading Group 2 ZoneList Name
	       \type object-list
	       \object-list ZoneListNames
      ...


# Testing/Validation/Data Sources

Manually inspect EnergyPlus outputs of shading calculations.

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
