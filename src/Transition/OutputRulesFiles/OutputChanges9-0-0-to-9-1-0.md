Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Additional field for Equipment Summary:Pumps report

A new column "End Use Subcategory" has been added to the Equipment Summary:Pumps report

See [7100](https://github.com/NatLabRockies/EnergyPlus/pull/7100)

### Renamed field for Equipment Summary:Fans report

The column "End Use" has been renamed to "End Use Subcategory" in the Equipment Summary:Fans report

See [7068](https://github.com/NatLabRockies/EnergyPlus/pull/7068)
