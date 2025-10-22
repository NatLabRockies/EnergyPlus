Zone Air Mass Flow Balance Improvement
=====================================

**Bereket Nigusse, FSEC/UCF**

 - Original Date: January 19, 2021
 - Revision Date: January 22, 2021


## Justification for New Feature ##

ZoneAirMassFlowConservation is intended to balance airflows but current design is that the procedure adjusts zone mixing and/or infiltration flow rates. User that have complex air transfer models like grocery stores, restaurants, and some big box retailers need the ability to model actual air transfer and account for ventilation movement between spaces. In these cases, users want the infiltration, zone mixing, and exhaust fan flow rates to stay fixed at user specified flows, instead adjust the system return air to balance zone air flow.

This new feature provides an alternative air flow balancing method that maintains the ZoneMixing flows at user specified flow rate to allow user preferred inter-zone air flows by adjusting the zone return air flows and in some cases by modifying user specified zone infiltration air flows.

## E-mail and Conference Call Conclusions ##

NA

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

         The zone is under positive pressure hence requires non-zero zone return air mass flow rate to balance the zone air flow. The return air flow rate will be adjusted within the bounds of 0 and a maximum zone return air flow rate that balance the zone air mass flow. The zone maximum return flow rate is determined by a check that the sum of zone return air flow of all the zones served does not exceed the design supply air flow rate of the air loop serving the zones.

Case II: [m_{ret} < 0.0]
         The zone is under negative pressure hence requires additional supply air or infiltration air is required to balance the zone air flow. There are two possibilities that can be considered: (A) increasing zone infiltration air flow or (B) increasing the zone supply air flow.
		 The preferred implementation approach is alternative IIA but both alternatives are presented for discussion.

		 Alternative IIA:
		 In alternative IIA the infiltration air flow is increased proportionally and the return air mass flow rate will be reset to zero to balance zone air flow.
		 [m_{inf} += m_{ret};]
		 [m_{ret} = 0.0;]

		 Alternative IIB:
		 In alternative IIB the supply air flow is increased proportionally and the return air mass flow rate will be reset to zero to balance zone air flow.
		 [m_{sup} += m_{ret};]
         [m_{ret} = 0.0;]

		 Alternative IIB may cause convergence problem that leads to exceeding maximum iteration limits or even missing the zone thermostat set-point because of fighting between the HVAC control and zone air mass balance flow adjustment. There could also be flow balancing problem when the HVAC system is scheduled off and the ZoneMixing or zone exhaust fan are active.

The ZoneMixing objects air flows are always maintained at user specified values to allow user defined inter-zone air transfer.


## Testing/Validation/Data Sources ##

(1) Any new subroutine(s) will have a unit test that validate that the subroutine is functioning properly.
(2) Verifies the zone air flow mass balance without altering user specified ZoneMixing object flows.

## Input Output Reference Documentation ##

This new feature will be implemented by adding a new choice key "AdjustZoneReturnFlow” to the existing input field "Adjust Zone Mixing For Zone Air Mass Flow Balance" in the ZoneAirMassFlowConservation object.

The existing input field "Adjust Zone Mixing For Zone Air Mass Flow Balance" better be renamed to "Zone Air Mass Flow Balance Method" and the two existing choice keys needs to be renamed as well for clarity. For example the "Yes" choice key will be replaced with "AdjustZoneMixingFlow" and the "No" choice key with "None". See below modified ZoneAirMassFlowConservation object.

## Input Description ##

See modified "ZoneAirMassFlowConservation" object:

ZoneAirMassFlowConservation,
       \memo Enforces the zone air mass flow balance by adjusting zone mixing object and/or
       \memo infiltration object mass flow rates.
       \memo If either mixing or infiltration is active, then the zone air mass flow
       \memo balance calculation will attempt to enforce conservation of mass for each zone.
       \memo If mixing is "None" and infiltration is "None", then the zone air mass flow
       \memo calculation defaults to assume self-balanced simple flow mixing and infiltration objects.
       \unique-object
       \min-fields 3
  A1,  \field Zone Air Mass Flow Balance Method
       \note If AdjutsZoneMixingFlow, Zone mixing object flow rates are adjusted to balance the zone air
       \note mass flow and additional infiltration air flow may be added if required in order to balance
       \note the zone air mass flow.
	   \note If AdjustZoneReturnFlow, zone return air mass flow is adjusted to balance the zone air flow
	   \note and additional infiltration air flow may be added if required in order to balance the zone
       \note air mass flow. ZoneMixing objects air flow is always maintained at user specified value.
       \type choice
       \key AdjutsZoneMixingFlow
	   \key AdjustZoneReturnFlow
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

IDD change is required to add new choice key to an existing input field, and rename the input field and the key choices. Transition is required.

## References ##

NA
