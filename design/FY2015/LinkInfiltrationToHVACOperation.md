# Link Infiltration to HVAC System Operation - Design

**Michael J. Witte, GARD Analytics, Inc.**

 - Original August 7, 2015
 - Add highlights from [NFP-LinkInfiltrationToHVAC.doc](https://github.com/NREL/EnergyPlusDevSupport/blob/master/DesignDocuments/Proposals/NFP-LinkInfiltrationToHVAC.doc)


## Background
ZoneAirMassFlowConservation was added in v8.2.0 via pull request [#4245](https://github.com/NREL/EnergyPlus/pull/4245). This PR touched 21 source files. Highlights:

   - Added ZoneMixingFlow to DataAirLoop structure
   - Added ZoneMassBalanceHVACReSim to DataHVACGlobales to signal re-simulation when needed
   - Added several arrays to DataHeatBalFanSys:
       ZoneMassBalanceFlag, ZoneInfiltrationFlag, ZoneMassBalanceRepVarFlag, and ZoneReOrder
   - Added ZoneAirMassBalanceSimulation flag to DataHeatBalance
   - Added new fields to DataInfiltration structure:  VolumeFlowRate and MassFlowRate
   - Added new fields to DataMixing structure: DesiredAirFlowRateSaved and MixingMassFlowRate
   - Added a new struct for ZoneAirMassFlowConservation, ZoneAirMassFlow (just holds the fields in the new object)
   - Added a new struct for < ZoneMassConservationData >, MassConservation
   - Moved arrays from HVACManager to DataZoneEquipment: CrossMixingReportFlag, MixingReportFlag, and VentMCP
   - Moved CalcAirFlowSimple() and GetStandAloneERVNodes() from HVACManager to ZoneEquipmentManager
   - Added argument AdjustZoneMixingFlowFlag to CalcAirFlowSimple()
   - Added an additional call to CalcAirFlowSimple() in ZoneEquipmentManager
   - Added flag to HVACStandAloneERV to suppress unbalanced flow warnings
   - Added SetZoneMassConservationFlag() to HeatBalanceAirManager - sets ZoneMassBalanceFlag flags to true
   - Added blocks of code to HeatBalanceAirManager::GetSimpleAirModelInputs to fill new ZoneMixingNum, ZoneReOrder and MassConservation data, and set up new output variables.
   - Added code to HeatBalanceManager::GetProjectControlData to process inputs for new ZoneAirMassFlowConservation object and do some related initializations
   - Added a check to force system re-simulation in SimAirServingZones
if ( ZoneMassBalanceHVACReSim ) SysReSim = true;
   - Added initilization of AirLoopFlow.ZoneMixingFlow in ZoneEquipmentManager
   - Added checks in ZoneEquipmentManager::SimZoneEquipment to supress unbalanced flow warnings
   - Added code to ZoneEquipmentManager::CalcZoneMassBalance to calculate mixing and infiltration flow rates for mass conservation
   - Since CalcAirflowSimple was moved, hard to tell what was changed in it, if anything
   - Added new CalcZoneMixingFlowRateOfReceivingZone() and CalcZoneMixingFlowRateOfSourceZone() in ZoneEquipmentManager


## Approach
Extend the ZoneAirMassFlowConservation feature by adding user inputs to control the maximum and minimum return air flow rate for each zone. The return air flow rate will be set based on user inputs and current system flow status, then the ZoneAirMassFlowConservation algorithm will make any necessary adjustments to mixing and infiltration flow rates.  In the extreme, this feature should make it possible to model a fully pressurized zone with zero return airflow and zero infiltration and incoming mixing flow rates.

## Proposed Input Changes





```
  ZoneHVAC:EquipmentConnections,
    SPACE1-1,                !- Zone Name
    SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name
    SPACE1-1 In Node,        !- Zone Air Inlet Node or NodeList Name
    ,                        !- Zone Air Exhaust Node or NodeList Name
    SPACE1-1 Node,           !- Zone Air Node Name
    SPACE1-1 Out Node,       !- Zone Return Air Node Name
NEW FIELD --> SPACE1-1 Return Flow Schedule,  !- Zone Return Air Flow Rate Fraction Schedule Name
NEW FIELD --> SPACE1-1 In Node;        !- Zone Return Air Flow Rate Basis Node or NodeList Name

  ZoneAirMassFlowConservation,
    Yes,                     !- Adjust Zone Mixing For Zone Air Mass Flow Balance
NEW FIELD NAMDE -->    AdjustInfiltrationFlowAllZones,     !- Infiltration Balancing Method
NEW FIELD --> AllZones;      !- Infiltration Balancing Zones [AllZones or MixingS?ourceZonesOnly]

```

## Data Structures ##
```[C++]
DataHeatBalance.hh
	struct ZoneAirMassFlowConservation
	{
		// Members
		bool EnforceZoneMassBalance;     // flag to enforce zone air mass conservation
		int InfiltrationTreatment;       // determines how infiltration is treated for zone mass balance
		//Note, unique global object

DataHeatBalance.cc
	// Parameter for source zone air flow mass balance infiltration treatment
	int const AddInfiltrationFlow( 1 );
	int const AdjustInfiltrationFlow( 2 );
```
*Add new parameter*
```[C++]
	int const AdjustInfiltrationFlowAllZones( 3 );
```
```[C++]

DataZoneEquipment.hh
	struct EquipConfiguration
	{
		// Members
		std::string ZoneName;
		int ActualZoneNum; // index into the Zone data
		std::string EquipListName;
		int EquipListIndex;
		std::string ControlListName;
		int ZoneNode;
		int ReturnAirNode;
		int NumInletNodes;
		int NumExhaustNodes;
		bool FlowError; // flow error flag
		Array1D_int InletNode; // zone supply air inlet nodes
		Array1D_int ExhaustNode; // zone air exhaust nodes
		int ReturnZonePlenumCondNum; // number of the zone's return air plenum
		int AirLoopNum; // the air loop index for this controlled zone
		int FanOpMode; // =0 if no central sys;
		// -1 if central sys is in cycling fan mode;
		// =2 if central sysis in constant fan mode.
		bool ZonalSystemOnly; // TRUE if served by a zonal system (only)
		bool IsControlled; // True when this is a controlled zone.
		Real64 ZoneExh; // zone exhaust (unbalanced+balanced) mass flow rate [kg/s]
		Real64 ZoneExhBalanced; // balanced zone exhaust mass flow rate [kg/s]
		Real64 PlenumMassFlow; // zone air mass flow rate induced from plenum [kg/s]
		// AirDistUnitCool and AirDistUnitHeat
		// do not correspond with the AIR DISTRIBUTION UNIT object in the zone equipment list.
		// AirDistUnitCool/AirDistUnitHeat, may represent a DIRECT AIR object,
		// or the cold/hot side of AIR DISTRIBUTION
		// UNIT object.  That is both AirDistUnitHeat and AirDistUnitCool are required to describe a dual
		// duct AIR DISTRIBUTION object in the ZoneEquipList.  Although only one AIR DISTRIBUTION UNIT is
		// allowed in ZoneEquipList, two instances of that object may exist in this data structure
		Array1D< AirIn > AirDistUnitHeat; // dimensioned to number of zone inlet nodes
		Array1D< AirIn > AirDistUnitCool; // dimensioned to number of zone inlet nodes.
		bool SupLeakToRetPlen; // True if there is supply duct leak to the
		// plenum (simple duct leakage model)
		bool InFloorActiveElement; // Convection adapation, true if zone has in-floor HVAC
		bool InWallActiveElement; // Convection adapation, true if zone has in-wall HVAC
		bool InCeilingActiveElement; // Convection adapation,
		// true when zone has in-ceiling HVAC
```
*Add new data to EquipConfiguration*
```[C++]
		// return node flow controls
		Array1D_int ReturnFlowBasisNode; // return air flow rate basis nodes
        int ReturnFlowSchedPtr; // return air flow schedule
```

## Code Structure ##

*Existing (not expected to change)*

#### SimulationManager.cc
* Resimulate
   - Note: calls CalcAirFlowSimple

####HeatBalanceManager.cc
* GetProjectControlData
   - Note: Gets input for ZoneAirMassConservation and many other objects (should be separated out)

####HeatBalanceAirManager.cc
* GetAirHeatBalanceInput
   * SetZoneMassConservationFlag
      - Note: sets the zone mass conservation flag to true
* GetSimpleAirModelInputs
   - Note: After processing simple airflow inputs (ZoneVentilation, ZoneInfiltration, etc.) there are several related loops (most of these should probably be separated out from this function?)
      - a loop gets pointers to the mixing objects serving source zones
      - a loop determines which zones are source zones for mixing objects
      - a loop determines which zones are receiving zones for mixing objets
      - a loop gets pointers to the mixing objects serving receiving zones
      - a pair of loops sets up the ZoneReOrder list with receiving zones first then source zones (*not sure what happens if a given zone is both*)
      - a loop sets up output variables related to zone mass conservation
      - a loop sets up Zone Mass Balance Infiltration Air Mass Flow Rate output variable
      - a loop at the end of this routine checks if every source zone has an infiltration object to use for balancing, throws a severe/fatal error if not.
* InitSimpleMixingConvectiveHeatGains
   - Note: After current mixing flows are set, if zone air mass flow balance enforced calculate the fraction of contribution of each mixing object to a zone mixed flow rate (should be separated out?)

####HVACStandAloneERV.cc
* CalcStandAloneERV
   - When ZoneAirMassFlow.EnforceZoneMassBalance is true, unbalanced airflow warning is supressed

####ZoneEquipmentManager.cc
* SimZoneEquipment
   - Calls CalcAirFlowSimple with AdjustZoneMixingFlowFlag
      - adjusts mixing and infiltration flow rates depending on settings
      - uses Infiltration(j).MassFlowRate for adjusted infiltration which as been set in CalcZoneMassBalance same as MassConservation(ZoneNum).InfiltrationMassFlowRate - so why not use that?  That *is* used for *add* infiltration.  Confused here.  Mixing(j).MixingMassFlowRate is used the same way.
   - Unbalanced exhaust flow warnings are supressed if ZoneAirMassFlow.EnforceZoneMassBalance is true
   - Calls CalcZoneMassBalance
      - This is the heart of the air mass flow balance calcs
      - Calls CalcZoneMixingFlowRateOfReceivingZone
         - updates receiving zone mixing flow rates
         - Calls CalcZoneMixingFlowRateOfSourceZone
            - updates source zone mixing flow rates
      - calculates infiltration adjustments
      - calculates BuildingZoneMixingFlow
      - **calculates return air node mass flow rate Node(RetNode).MassFlowRate**
      - iterates to achieve balance up to IterMax (25)

*CalcAirFlowSimple is called from several places*
   - HVACManager.cc(328):		CalcAirFlowSimple();
   - HVACManager.cc(386):				CalcAirFlowSimple( SysTimestepLoop );
   - SimulationManager.cc(2796):		CalcAirFlowSimple( 0, ZoneAirMassFlow.EnforceZoneMassBalance );
     - second argument seems wrong here - this is insides if ( ResimHVAC ) - may be incorrect arguments
     - Checking the history, this used to pass AdjustZoneMixingFlowFlag = true when ZoneAirMassFlow.EnforceZoneMassBalance was true, so this was collapsed (along with other changes to the arguments) to simply pass ZoneAirMassFlow.EnforceZoneMassBalance. The name of this argument is misleading, because it controls adjustments of both mixing and infiltration within CalcAirFlowSimple.  The usage appears to be OK, however.
   - ZoneEquipmentManager.cc(2690):			CalcAirFlowSimple( 0, AdjustZoneMixingFlowFlag );
      - here, AdjustZoneMixingFlowFlag is simply a constant that is always true, the call is within an if ( ZoneAirMassFlow.EnforceZoneMassBalance ) with no else.

## Output ##
*No new outputs are proposed*

Zone Infiltration Mass Flow Rate [kg/s]", ZnAirRpt( Infiltration( Loop ).ZonePtr ).InfilMdot
MCpI_temp = Infiltration(j).VolumeFlowRate \* AirDensity \* CpAir;
ADJUSTInfiltration: Infiltration(j).VolumeFlowRate = Infiltration(j).MassFlowRate / AirDensity;
OR
ADDInfiltration:  Infiltration(j).VolumeFlowRate = Infiltration(j).VolumeFlowRate + MassConservation(NZ).InfiltrationMassFlowRate / AirDensity;

"Zone Mass Balance Infiltration Air Mass Flow Rate [kg/s]", MassConservation(Infiltration(Loop).ZonePtr).InfiltrationMassFlowRate


So, if adjustinfiltration is active,
In CalcZoneMassBalance
MassConservation(ZoneNum).InfiltrationMassFlowRate = ZoneInfiltrationMassFlowRate;
Infiltration(MassConservation(ZoneNum).InfiltrationPtr).MassFlowRate = ZoneInfiltrationMassFlowRate

In CalcAirFlowSimple
Infiltration(j).VolumeFlowRate = Infiltration(j).MassFlowRate / AirDensity;


## Proposed Changes ##
1. Currently, only zones with mixing objects (as source or receiving) are included in the zone mass balance.  Adjustment of infiltration will be extended to all zones when new field in ZoneAirMassFlowConservation is set to AllZones. **CalcZoneMassBalance and probably other functions.**
2. Change meaning of exisiting field "Adjust Zone Mixing for Zone AirMass Flow Balance" to allow balancing of mixing only, infiltration only, or both.  Currently if this is set to no, the mass conservation is off.
3. *This is ok* - Tested 5ZoneAirCooled_ZoneAirMassFlowBalance with mass conservation on and off.  There is an impact on the reported zone infiltration flow rates, but only on the summer design day. It's not clear if infiltration and mixing adjustments are having an impact on the zone heat balance. So no change is required for calls to CalcAirFlowSimple.
4. Add input fields for user control of return air flow rate.  This adusted return flow rate should automatically flow into the zone air mass balance calculations. **DataZoneEquipment::GetZoneEquipmentData1 and CalcZoneMassBalance.**

## Issues Encountered
1. Rescaling of return flow rates in ZoneEquipmentManager::CalcZoneMassBalance is overriding the return air flow rate adjustment - must skip this when using controlled return air or inform this somehow.  Perhaps the return air controls need to be at the air loop level?
2. With mass balance off, getting max iteration errors with return air adjustment.  It's cyclical, and it appears to stem from this code in HVACManager::SimSelectedEquipment.
```
				if ( SimZoneEquipment ) {
					if ( ( IterAir == 1 ) && ( ! FlowMaxAvailAlreadyReset ) ) { // don't do reset if already done in FirstHVACIteration
						ResetTerminalUnitFlowLimits();
						FlowResolutionNeeded = true;
					} else {
						ResolveAirLoopFlowLimits();
						FlowResolutionNeeded = false;
					}
```
This is within the IterAir loop in SimSelectedEquipment and it fires every time IterAir == 1, because FlowMaxAvailAlreadyReset is being set to false within the IterAir loop - seems like thst line should be *out*side the IterAir loop.  No, in fact it should be outside this routine. This code was added for CR7781 [#2561](https://github.com/NREL/EnergyPlus/issues/2561) on Feb 27, 2010 which stemmed from problems with some terminal units scheduled off for part of the year.
https://github.com/NREL/EnergyPlusArchive/commit/c8c4ecb752714b44b8b15de074fad81d9139d685

[ResetTerminalUnitFlowLimits](https://github.com/NREL/EnergyPlus/blame/develop/src/EnergyPlus/HVACManager.cc#L1613) resets all the terminal unit inlet node MassFlowRateMaxAvail (and Min) to the hard design flow rates without any regard to schedules or other things that may be varying.  This is find during FirstHVACIteration, but looks suspicious here.  This probably explains why I couldn't get the VAV terminal unit schedule to stick.  Note that IterAir is repeated up to six times (hardwired iteration limit) and then goes back up to the main HVAC iteration loop (HVACManagerIteration whcih is the one that has a user-controlled max that defaults to 20).
