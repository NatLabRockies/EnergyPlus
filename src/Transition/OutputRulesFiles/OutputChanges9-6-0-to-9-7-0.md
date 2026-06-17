Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### WindowConstruction EIO Fields Change
Two fields are added to the Description of the Windows Report - "WindowConstruction", namely "Construction Report Conductance (Before Adjusted) {W/m2-K}" and "Convection Coefficient Adjustment Ratio". They represent the glazing-only nominal U (nominal conductance in winter) and the adjustment ratio assuming highly conductive frames are applied along with the glazing.

See PR [#8653](https://github.com/NatLabRockies/EnergyPlus/pull/8653) and PR [#9117 ](https://github.com/NatLabRockies/EnergyPlus/pull/9117).
