Zone Air Mass Flow Balance Improvement
=====================================

**Bereket Nigusse, FSEC/UCF**

 - Original Date: January 19, 2021
 - Revision Date: January 22, 2021
 - Revision Date: February 4, 2021
 - Revision Date: February 15, 2021

## Justification for New Feature ##

ZoneAirMassFlowConservation is intended to balance airflows but current design is that the procedure adjusts zone mixing and/or infiltration flow rates. User that have complex air transfer models like grocery stores, restaurants, and some big box retailers need the ability to model actual air transfer and account for ventilation movement between spaces. In these cases, users want the infiltration, zone mixing, and exhaust fan flow rates to stay fixed at user specified flows, instead adjust the system return air to balance zone air flow.

This new feature provides an alternative air flow balancing method that maintains the ZoneMixing flows at user specified flow rate to allow user preferred inter-zone air flows by adjusting the zone return air flows and in some cases by modifying user specified zone infiltration air flows.

## E-mail and Conference Call Conclusions ##

### E-mail Communication ###
Brent:
Here is my two cents to frame this.  In the defect file, and the reference building restaurant example file there is a work-around that has been around for over a decade.  I would like to see this project finally do the zone mass balancing needed to remove that work-around.  There should be no need to add a dummy exhaust fan in the source zone, and no need to figure out a messy and confusing schedule for the balancing/unbalancing in the zone exhaust fan in the receiving zone.  The input hack, which I developed way back in the day, doesn’t even work for VAV system, nor if the zone transfer air is larger than the exhaust fan flow.

Bereket Response;
(1) The current zone air mass flow balance does not require dummy exhaust fan either in the source or receiving zone. What you need is specifying ZoneMixing object.
(2) The current zone air mass flow balance does not require schedule for the balancing/unbalancing in the zone exhaust fan in the receiving zone
(3) In general current zone air mass flow balance works better with VAV system but I have not tested the case with zone transfer air is larger than the exhaust fan flow.

Mike:
Bereket,
Adjusting infiltration is one option, but what's really wanted here is reduced return flow from the dining room rather than increased infiltration to the dining room.

Looking at the Engineering Ref section on Zone Air Mass Flow Conservation (see attached pages),
the last equation on the first page says that the dining return flow should be reduced to account for the mixing to the kitchen when ZoneAirmassFlowConservation is active. But it doesn't appear this is happening. mXR and mXS are the mixing flows for receiving and source.

So, why isn't this happening?  Looking at line 3979 in CalcZoneMassBalance there net mixing flow is included in the equation.
https://github.com/NREL/EnergyPlus/blob/0750c2009f67a3e05268b7ae7ab6cba8781826a7/src/EnergyPlus/ZoneEquipmentManager.cc#L3979-L3995

                StdTotalReturnMassFlow = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).TotInletAirMassFlowRate + ZoneMixingNetAirMassFlowRate -
                                         (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).TotExhaustAirMassFlowRate - state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneExhBalanced);

So, maybe ZoneMixingNetAirMassFlowRate isn't getting set correctly?

Yep, it's zero at this point, ZoneMixingNetAirMassFlowRate isn't set until line 4047 and only when
if (ZoneAirMassFlow.InfiltrationTreatment != NoInfiltrationFlow) is true

If that's true, it should get set and then impact the return flow on the next time through the mass balance do loop?

I'll stop here, but I think this needs to be fixed and/or new options added to ZoneAirmassFlowConservation to make this part work. And maybe getting this all to work correctly is "all" that's needed for this new feature. Not saying it will be easy . . . .

Mike

### Conference Call Conclusions ###
Bereket:
Presented the NFP.

Mike:
Currently the infiltration treament method has three choices. But what is really needed in this feature is that balancing the zone air flow by adjusting the return air flow and also make sure that the current method is also working propoerly.

Brent:
It would have been better if the zone air mass flow conservation control is applied zone by zone basis instead of global.

Mike:
For now let's address current needs and only if budget allows zone by zone basis can be considered.

Bereket:
I will update the NFP per toda's discussion and add design documentation as well.


###Review comments and responses on Github####:

@rraustad, Comment:
How are constant volume HVAC systems going to respond to these 2 cases? I assume OA will make up the difference between supply and return flow rates. What happens when there is no OA system? In that case the supply and return flows should match
@Nigusse, Response:
I am assuming that the two cases should work for constant volume and VAV systems and have to confirmed through testing.
Regarding "What happens when there is no OA system?", I have not thought enough if it needs a special treatment. One issue is that supply and return flows may not match for zones that has exhaust fan flow, ZoneMixing and Infiltration flows. You have to adjust either the supply, infiltration, or the return flows as needed.

@mjwitte, Comment:
(1) For systems with no outdoor air, the existing return flow calculations force the return to match the corresponding supply flow for a given zone. This balance must be maintained.
(2) For constant volume systems, are you suggesting that the system flow could be increased by this? That's going to run up against the terminal unit max flow rate.
(3) For VAV systems, increasing the supply flow could cause temperature control problems.
@Nigusse, Response:
I suspected I may run into control problem when the supply air flow rates is altered. In some cases return air can be zero and we may not have other variable to change unless infiltration air flow rate is adjusted instead of supply air. In fact my original NFP draft was to adjust return air and infiltration to balance the flow while keeping the ZoneMixing object flow as specified and letting the control system deal with supply air flow rate.
I would rather adjust return and infiltration air flow rates to balance the system and let the control system determine the supply air flow rate. What do you think? @mjwitte
@mjwitte, Response:
That seems safer, but that question should be answered by those who are requesting this feature and the specific use cases that it will be applied to.
@Nigusse, Response:
OK, I will seek input from those requested the feature.

@mjwitte, Comment:
What determines "maximum return air flow rate"?
How will this work when there are multiple return nodes (more than one AHU serving a zone)?
@Nigusse, Response:
I am thinking the maximum will be determined based on the the Node's maximum Mass Flow Rate and will use the same logic for all return nodes connected to the same zone. Not sure if this causes any problem.
@mjwitte, Response:
I don't think return nodes have a defined maximum mass flow rate, but I may be wrong. The only value that might be used would be the overall airloop design flow rate, which will defeat the purpose of this feature when it wants to set return flow > supply.
@Nigusse, Response:
My thought is it allows to set zone return flow > zone supply flow but the AirLoop overall return cannot be set > the overall airloop design flow rate.

@mjwitte, Comment:
How does the proposed method model or give user control over transfer air?
@Nigusse, Response:
Users will specify transfer air using ZoneMixing objects and these flow rates remain the same.
@mjwitte, Response:
OK, but with this method, the ZoneMixing flows will occur no matter what the system flows are. It will be up to the user to schedule the ZoneMixing flows to match the HVAC system operation.
@Nigusse, Response:
Yes, that is my plan.

@Nigusse, final comment:
I will update the NFP based on the feedback and seek input from those requested the feature. @mjwitte and @rraustad Thank you for your review and feedback.


## Overview ##

This work will implement code changes that allow alternative approach of the zone air mass flow balance calculation method that does not alter the simple flow objects mass flow. The fundamental zone mass balance equation remains the same but what this feature requesting is which zone air mass flow component needs to be adjusted to guarantee zone air mass flow conservation.

The zone mixing object flows will always remain at user specified values such that inter-zone air transfer is modeled based on user's preference only.

## Approach ##

Current zone air mass flow conservation equation is formulated to solve the following equation by adjusting the Zone Mixing flow, and zone infiltration flow sequentially.

 0.0 = [m_{sup} - m_{exh} - m_{ret} + m_{zmreceiving} - m_{zmsource} + m_{inf}]

where,
m_{sup} = Zone Supply Air Mass Flow Rate, [kg/s]
m_{exh} = Zone Exhaust Air Mass Flow Rate, [kg/s]
m_{ret} = Zone Return Air Mass Flow Rate, [kg/s]
m_{zmreceiving} = Zone Mixing Receiving Air Mass Flow Rate, [kg/s]
m_{zmsource} = Zone Mixing Source Air Mass Flow Rate, [kg/s]
m_{inf} = Zone Infiltration Air Mass Flow Rate, [kg/s]

The existing procedure first attempts to solve the above equation by adjusting the Zone Mixing flow only and if that does not satisfy the above equation, then the user specified zone infiltration air flow rate is adjusted as needed.  The zone infiltration air flow can be adjusted using two different methods depending user preference. The zone infiltration air flow adjusting method is a choice input field in the ZoneAirMassFlowConservation object.

The requested feature will be implemented by adjusting the return air mass flow but the return air mass flow rate can be varied between the minimum value of zero and a maximum flow rate limited by the airloop design supply flow rate. Since varying the return air mass flow provides limited flexibility in achieving balanced flow for the range of practical applications, the zone air flow mass balance requires two different solution scenarios: one when the zone is under positive pressure and when the zone is under negative pressure. When the zone is under positive pressure, then the zone return air flow will be adjusted to balance the zone flow; whereas when the zone is under negative pressure zone infiltration air flow will be adjusted or increased.

Solving the zone air flow mass balance equation for zone return air mass flow:

m_{ret} = [m_{sup} - m_{exh} + m_{zmreceiving} - m_{zmsource} + m_{inf}]

Depending on the sign of the value of return air mass flow rate calculated using the above equation two different cases that require different solution scheme are formulated:

Case I:  [m_{ret} >= 0.0]

         The zone is under positive pressure hence requires non-zero zone return air mass flow rate to balance the zone air flow. The return air flow rate will be adjusted within the bounds of 0 and a maximum zone return air flow rate that balance the zone air mass flow. The zone maximum return flow rate is determined by the design supply air flow rate of the air loop serving the zones.

		 For zone mixing objects serveing zones in a single airloop:
         [m_{ret} <= min(m_{ret}, m_supply_design_airloop)]

		 For zone mixing objects serveing zones in two or more airloops:
		 [m_{ret} <= min(m_{ret}, (m_{supply_design_airloop} - m_{airloopZoneMixingNetOutFlow}))]

		 where,
		 [m_{airloopZoneMixingNetOutFlow}] = net outflow of zone mixing air flow from one airloop to another airloop, [kg/s].

Case II: [m_{ret} < 0.0]
         The zone is under negative pressure hence requires additional supply air or infiltration air is required to balance the zone air flow. There are two possibilities that can be considered: (A) increasing zone infiltration air flow or (B) increasing the zone supply air flow.
		 The preferred implementation approach is alternative IIA but both alternatives are presented for discussion.

		 Alternative IIA:
		 In alternative IIA the infiltration air flow is increased proportionally and the return air mass flow rate will be reset to zero to balance zone air flow.
		 [m_{inf} += m_{ret};]
		 [m_{ret} = 0.0;]

		 The zone air infiltration rate will be determined based user selection from three infiltration treatment method available.

The ZoneMixing objects air flows are always maintained at user specified values to allow user defined inter-zone air transfer.

Furthermore, first the zone return air flow is estimated without the zone air infiltration term and then zone air infiltration terms is added to balance the zone air flow if needed as a second step. This approach gives the
option of balancing the zone air flow with return air flow adjustment only, or with little change to the user specified zone infiltration air flows.

Proposed Approach Limitation:
For a case when a zone has exhaust fan flow, the zone has no user specified infiltration, and the HVAC system is off, then the zone air flow cannot be balanced unless the zone exhaust fan flow is less than the net zone mixing flow.

## Testing/Validation/Data Sources ##

(1) Any new subroutine(s) will have a unit test that validate that the subroutine is functioning properly.
(2) Verifies the zone air flow mass balance without altering user specified ZoneMixing object flows.

## Input Output Reference Documentation ##

This new feature will be implemented by adding a new choice key "AdjustZoneReturnFlow” to the existing input field "Adjust Zone Mixing For Zone Air Mass Flow Balance" in the ZoneAirMassFlowConservation object.

The existing input field "Adjust Zone Mixing For Zone Air Mass Flow Balance" better be renamed to "Zone Air Mass Flow Balance Method" and the two existing choice keys needs to be renamed as well for clarity. For example the "Yes" choice key will be replaced with "AdjustZoneMixingFlow" and the "No" choice key with "None". See below modified ZoneAirMassFlowConservation object.

## Input Description ##

See modified "ZoneAirMassFlowConservation" object:

ZoneAirMassFlowConservation,
       \memo Enforces the zone air mass flow balance by either adjusting zone mixing object flow only,
       \memo adjusting zone total return flow only, zone mixing and the zone total return flows,
       \memo or adjusting the zone total return and zone mixing object flows. Zone infiltration flow air
       \memo flow is increased or decreased depending user selection in the infiltration treatment method.
       \memo If either of zone mixing or zone return flow adjusting methods or infiltration is active,
       \memo then the zone air mass flow balance calculation will attempt to enforce conservation of
       \memo mass for each zone. If flow balancing method is "None" and infiltration is "None", then the
       \memo zone air mass flow calculation defaults to assume self-balanced simple flow mixing and
       \memo infiltration objects.
       \unique-object
       \min-fields 3
  A1,  \field Adjust Zone Mixing and Return For Air Mass Flow Balance
       \note If "AdjustMixingOnly", zone mixing object flow rates are adjusted to balance the zone air mass
       \note flow and zone infiltration air flow may be increased or decreased if required in order to balance
       \note the zone air mass flow. If "AdjustReturnOnly", zone total return flow rate is adjusted to balance
       \note the zone air mass flow and zone infiltration air flow may be increased or decreased if required
       \note in order to balance the zone air mass flow. If "AdjustMixingThenReturn", first the zone mixing
       \note objects flow rates are adjusted to balance the zone air flow, second zone total return flow rate
       \note is adjusted and zone infiltration air flow may be increased or decreased if required in order to
       \note balance the zone air mass flow. If "AdjustReturnThenMixing", first zone total return flow rate is
	   \note adjusted to balance the zone air flow, second the zone mixing object flow rates are adjusted and
	   \note infiltration air flow may be increased or decreased if required in order to balance the zone
	   \note air mass flow.
       \type choice
       \key AdjustMixingOnly
       \key AdjustReturnOnly
       \key AdjustMixingThenReturn
       \key AdjustReturnThenMixing
       \key None
       \default None
  A2,  \field Infiltration Balancing Method
       \note This input field allows user to choose how zone infiltration flow is treated during
       \note the zone air mass flow balance calculation.
       \type choice
       \key AddInfiltrationFlow
       \key AdjustInfiltrationFlow
       \key None
       \default AddInfiltrationFlow
       \note AddInfiltrationFlow may add infiltration to the base flow specified in the
       \note infiltration object to balance the zone air mass flow. The additional infiltration
       \note air mass flow is not self-balanced. The base flow is assumed to be self-balanced.
       \note AdjustInfiltrationFlow may adjust the base flow calculated using
       \note the base flow specified in the infiltration object to balance the zone air mass flow. If it
       \note If no adjustment is required, then the base infiltration is assumed to be self-balanced.
       \note None will make no changes to the base infiltration flow.
  A3;  \field Infiltration Balancing Zones
       \note This input field allows user to choose which zones are included in infiltration balancing.
       \note MixingSourceZonesOnly allows infiltration balancing only in zones which as source zones for mixing
       \note which also have an infiltration object defined.
       \note AllZones allows infiltration balancing in any zone which has an infiltration object defined.
       \type choice
       \key MixingSourceZonesOnly
       \key AllZones
       \default MixingSourceZonesOnly

## Outputs Description ##

No new output variables will be implemented.

## Engineering Reference ##

Will be updated as needed.

## Example File and Transition Changes ##

One new example file will be added to the test suite to demonstrate this feature is functioning properly.

IDD change is required to add new choice keys to an existing input field, and rename the input field and the key choices. Transition is required.

## References ##

NA


## Design Document ##

This new feature revises modules: HeatBalanceManager, ZoneEquipmentManage, DataHeatBalance, and HeatBalanceAirManager.


### HeatBalanceManager ###

The code change includes updating the GetProjectControlData() to allow new choice keys and renaming an existing input field and its choice keys.


#### Current Get Control Data: ####
{
	{
		auto const SELECT_CASE_var(AlphaName(1));
		if (SELECT_CASE_var == "YES") {
			ZoneAirMassFlow.BalanceMixing = true;
			ZoneAirMassFlow.EnforceZoneMassBalance = true;
			AlphaName(1) = "Yes";
		} else if (SELECT_CASE_var == "NO") {
			ZoneAirMassFlow.BalanceMixing = false;
			AlphaName(1) = "No";
		} else {
			ZoneAirMassFlow.BalanceMixing = false;
			AlphaName(1) = "No";
			ShowWarningError(state, CurrentModuleObject + ": Invalid input of " + cAlphaFieldNames(1) + ". The default choice is assigned = No");
		}
	}

}

#### Proposed Get Control Data: ####

 - Replace choice key "Yes" with "AdjutsMixingOnly"
 - Replace choice key "No" with "None"
 - Add new choice key "AdjustReturnOnly"
 - Add new choice key "AdjustMixingThenReturn"
 - Add new choice key "AdjustReturnThenMixing"

 Sets the zone air mass flow conservation enforcing methods as follows:

{
	auto const SELECT_CASE_var(AlphaName(1));
	if (SELECT_CASE_var == "ADJUSTMIXINGONLY") {
		ZoneAirMassFlow.ZoneFlowAdjustment = AdjustMixingOnly;
		ZoneAirMassFlow.EnforceZoneMassBalance = true;
		AlphaName(1) = "AdjustMixingOnly";
	} else if (SELECT_CASE_var == "ADJUSTRETURNONLY") {
		ZoneAirMassFlow.ZoneFlowAdjustment = AdjustReturnOnly;
		ZoneAirMassFlow.EnforceZoneMassBalance = true;
		AlphaName(1) = "AdjustReturnOnly";
	} else if (SELECT_CASE_var == "ADJUSTMIXINGTHENRETURN") {
		ZoneAirMassFlow.ZoneFlowAdjustment = AdjustMixingThenReturn;
		ZoneAirMassFlow.EnforceZoneMassBalance = true;
		AlphaName(1) = "AdjustMixingThenReturn";
	} else if (SELECT_CASE_var == "ADJUSTRETURNTHENMIXING") {
		ZoneAirMassFlow.ZoneFlowAdjustment = AdjustReturnThenMixing;
		ZoneAirMassFlow.EnforceZoneMassBalance = true;
		AlphaName(1) = "AdjustReturnThenMixing";
	} else if (SELECT_CASE_var == "NONE") {
		ZoneAirMassFlow.ZoneFlowAdjustment = NoAdjustReturnAndMixing;
		AlphaName(1) = "None";
	} else {
		ZoneAirMassFlow.ZoneFlowAdjustment = NoAdjustReturnAndMixing;
		AlphaName(1) = "None";
		ShowWarningError(state, CurrentModuleObject + ": Invalid input of " + cAlphaFieldNames(1) + ". The default choice is assigned = None");
	}
}


####DataHeatBalance###

Add new member variable *ZoneFlowAdjustment* to ZoneAirMassFlowConservation struct.

Added parameters for zone air flow mass balancing methods selection variables to DataHeatBalance module.
    AdjustMixingOnly;
    AdjustReturnOnly;
    AdjustMixingThenReturn;
    AdjustReturnThenMixing;
    NoAdjustReturnAndMixing;

### HeatBalanceAirManager ###

The code revision includes updating the SetZoneMassConservationFlag() function to allow to reset flag variables for AdjustZoneReturnFlow method.

The code revision includes updating allow zonemixing and infiltrtion flow report variables for the new method in GetSimpleAirModelInputs() function.


### ZoneEquipmentManage ###

Most of code revisions are done in this module. The module code revision includes resetting either ZoneMassBalanceFlag() or AdjustZoneMassFlowFlag variable depending on the method for use with CalcAirFlowSimple() function.

Do not allow the ZoneMixing object flow adjustment for the new method as shown in the proposed code snippet:

##### Current Code: #####
                    // Set zone mixing incoming mass flow rate
                    if ((Iteration == 0) || !ZoneAirMassFlow.BalanceMixing) {
                        ZoneMixingAirMassFlowRate = MixingMassFlowZone(ZoneNum);
                    } else {
                        ZoneMixingAirMassFlowRate =
                            max(0.0,
                                ZoneReturnAirMassFlowRate + state.dataZoneEquip->ZoneEquipConfig(ZoneNum).TotExhaustAirMassFlowRate -
                                    state.dataZoneEquip->ZoneEquipConfig(ZoneNum).TotInletAirMassFlowRate + MassConservation(ZoneNum).MixingSourceMassFlowRate);
                    }

##### Proposed Code: #####
                    // Set zone mixing incoming mass flow rate
                    if ((Iteration == 0) || ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustReturnOnly ||
                        ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustReturnThenMixing) {
                        ZoneMixingAirMassFlowRate = MixingMassFlowZone(ZoneNum);
                    } else {
                        ZoneMixingAirMassFlowRate = max(0.0, ZoneReturnAirMassFlowRate + TotExhaustAirMassFlowRate -
                                TotInletAirMassFlowRate + MassConservation(ZoneNum).MixingSourceMassFlowRate);
                    }



##### Additional Proposed Code: #####
                if (ZoneAirMassFlow.EnforceZoneMassBalance) {

                    if (ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustMixingOnly ||
                        ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustMixingThenReturn) {
</span>
                        if (ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustMixingThenReturn) {
</span>
                        }
                        // Set zone infiltration air flow rate
                        CalcZoneInfiltrationFlows(state, ZoneNum, ZoneReturnAirMassFlowRate);

                    } else if (ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustReturnOnly ||
                               ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustReturnThenMixing) {
</span>
                        if (ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustReturnThenMixing) {
</span>
                        }
                        // Set zone infiltration air flow rate
                        CalcZoneInfiltrationFlows(state, ZoneNum, ZoneReturnAirMassFlowRate);
                    } else {
                        // if infiltration treatment method is not None
                        // Set zone infiltration air flow rate
                        CalcZoneInfiltrationFlows(state, ZoneNum, ZoneReturnAirMassFlowRate);
					}
                }

##### Refactoring Current Code: #####

// this require refactoring the existing infiltration flow update code section in new function CalcZoneInfiltrationFlows() as follows:

    void CalcZoneInfiltrationFlows(EnergyPlusData &state,
                                   int const ZoneNum,                // current zone index
                                   Real64 &ZoneReturnAirMassFlowRate // zone total zone return air mass flow rate
    )
	{
	</span>
	}


##### Modify CalcZoneReturnFlows() Code: #####

This function will be revised:
 - reset the return air flow rate calculated using the air flow mass balance


##### Refactoring Current Code: #####

- add a check that the return air flow rate does not exceed the air loop supply design flow rate.
- adds a new function that returns the design supply of the airloop that contains the zone

    void ZoneReturnFlowsMaximum(EnergyPlusData &state, int const ZoneNum,
         Real64 &MaximumZoneReturnMassFlow // maximum zone total return air mass flow rate
    )
    {
        MaximumZoneReturnMassFlow = 0.0;
        // sets the zone return node maximum flow rate to the airloop design supply flow rate
        for (int returnNum = 1; returnNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++returnNum) {
            int airLoop = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeAirLoopNum(returnNum);
            if (airLoop > 0) {
                auto &thisAirLoopFlow(state.dataAirLoop->AirLoopFlow(airLoop));
                MaximumZoneReturnMassFlow = thisAirLoopFlow.DesSupply;
            }
        }
    }

