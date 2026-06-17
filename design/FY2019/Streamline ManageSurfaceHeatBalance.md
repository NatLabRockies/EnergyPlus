Streamline ManageSurfaceHeatBalance
================

**Michael J. Witte, GARD Analytics, Inc.**

 - June 28, 2019, Design - Part 1 (after the fact)
 - Revision Date


## Justification for New Feature ##

Speed!

## E-mail and  Conference Call Conclusions ##

n/a

## Overview ##

This work focuses on streamlining the surface heat balance loops. The general calling tree for this section is:
```
ManageHeatBalance
	InitHeatBalance
	ManageSurfaceHeatBalance
		InitSurfaceHeatBalance
		CalcHeatBalanceOutsideSurf
		CalcHeatBalanceInsideSurf
			CalcInteriorRadExchange
				InitInteriorRadExchange
					FixViewFactors
			CalculateZoneMRT
		ManageAirHeatBalance
		UpdateFinalSurfaceHeatBalance
```

## Design - Part 1 ##

### Code Review - CalcHeatBalanceInsideSurf ###
Initial code review of `CalcHeatBalanceInsideSurf` revealed the the following problems (not exhaustive):

1. HTSurfToResimulate, a list of surfaces to simulate, is rebuilt every time depending on whether `CalcHeatBalanceInsideSurf` was called
for all zones or to resimulate a single zone. The list is build by looping through all surfaces, checking for heat trasfer surfaces and (optionally)
for the correct zone. This could be done once and saved.

2. Loop through all zones and their surfaces looking for zones which have mixed heat transfer algorithms. This could be done once and saved.

3. Many occurrences of `any_eq(HeatTransferAlgosUsed, UseCondFD)` or similar. `any-eq` is an Objexx function which loops through a list looking
for a match. In `CalcHeatBalanceInsideSurf`, these are looking for uses of CondFD, Kiva, HAMT, etc. in order to conditionally call special functions
or do other calculations specific to a certain algorithm. It is a small list (only CTF by default), but lots of gymnastics to find a result
 that could be done once and stored in a bool. The same function used elsewhere in the code for other things.

4. Loop through all surfaces looking for InsideHeatSourceTermSchedule and set QAdditionalHeatSourceInside to a schedule value -
even if there are none in the simulation. Could add a new bool to skip this if there are no inside sources used.

5. Check if windows have an internal source - why are windows even in this list - they're solved elsewhere?

6. Calculates RhoVaporAirIn and HMassConvInFD even if moisture methods are not used.

7. Other thins, such as report variable calculations which don't need to happen inside the iteration loop. Can be
done later, possibly at the end of the timestep.

### Refactoring - CalcHeatBalanceInsideSurf and Related ###

1. For all heat transfer surfaces, add new arrays to hold lists of surfaces (indexes) for loops in `CalcHeatBalanceInsideSurf`:

```
DataSurfaces:: std::vector<int> AllHTSurfaceList;  // List of all heat transfer surfaces
DataSurfaces:: std::vector<int> AllIZSurfaceList;  // List of all interzone heat transfer surfaces

DataHeatBalance::ZoneData:: std::vector<int> ZoneHTSurfaceList; // List of HT surfaces related to this zone (includes adjacent interzone surfaces)
DataHeatBalance::ZoneData:: std::vector<int> ZoneIZSurfaceList; // List of interzone surfaces in this zone
```

Build these lists after all surface input has been processed in `SurfaceGeometry::GetSurfaceData`.

Because I couldn't figure out how to alias the working loop list inside if blocks at the top of `CalcHeatBalanceInsideSurf` and have the
alias still in scope throughout the rest of `CalcHeatBalanceInsideSurf`, I've split the function into two parts. `CalcHeatBalanceInsideSurf` is a
stub which selects the desired surface lists to act on (all surfaces or a single zone) and then passese the appropriate lists as arguments to
`CalcHeatBalanceInsideSurf2` which does all the work.


2. Move the check for mixed heat transfer algorithms up into the one-time section for `calcHeatBalanceInsideSurfFirstTime`.

3. Replace any_eq with simple bools for which algorithms are in use

In `DataHeatBalance` delete the existing integer parameters `UseCTF`, `UseEMPD`, etc. which are scanned repeatedly using `any_eq`.
Replace these with new bools `AnyCTF`, `AnyEMPD`, etc. Set these during intial program setup, primarily in `HeatBalanceManager::GetProjectControlData`
and `SurfaceGeometry::GetSurfaceHeatTransferAlgorithmOverrides`. With these, the various if blocks in `CalcHeatBalanceInsideSurf` (and elsewhere)
 can check a simple bool rather than calling `any_eq`.

4. Add new bools to indicate if any inside or outside source terms have been used.

```
   DataSurfaces:: bool AnyHeatBalanceInsideSourceTerm;  // True if any SurfaceProperty:HeatBalanceSourceTerm inside face used
   DataSurfaces:: bool AnyHeatBalanceOutsideSourceTerm; // True if any SurfaceProperty:HeatBalanceSourceTerm outside face used
```

5. No change for now.

6. Compute these only for applicable surfaces. (` surface.HeatTransferAlgorithm == HeatTransferModel_EMPD) || . . . HeatTransferModel_HAMT`)

7. Move out things that should only happen at the end of the timestep (not repeatedly during iterations), like computing zone window heat gains, etc.

*This is the end of Part 1 work. More remains to be done in `CalcHeatBalanceInsideSurf` before moving on to other functions.*
