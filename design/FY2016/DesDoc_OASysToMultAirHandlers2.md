# **Dedicated OA System Serving Multiple Air Handlers**
## **Fred Buhl LBNL 1/20/2016**
### **Justification for Feature Update:**
This new feature will allow EnergyPlus to model an outside air system supplying ventilation air to multiple air handlers.

Currently a DOAS (dedicated outdoor air system) in EnergyPlus is a normal AirLoopHVAC (aka air loop)  object with the recirculated air flow set to zero. In EnergyPlus the air loops are independent of each other: one air loop can not provide air to another. Outside air is provided to each air system by the AirLoopHVAC:OutdoorAirSystem object. Each outdoor air system can supply only one air loop. All this means that EnergyPlus can't simulate a common HVAC layout: a single multi-floor DOAS serving, through an air shaft, multiple single floor air handling units.

### **Conference Call Conclusions**
#### **Jan 06, 2016**
*  Should the DOAS serving multiple air handlers be modeled on the AirLoopHVAC object or the AirLoopHVAC:OutdoorAirSystem? Consensus seemed to be on the AirLoopHVAC:OutdoorAirSystem.
*  The DOAS will be supplying ventilation air. To allow economizer action, each  AirLoopHVAC:OutdoorAirSystem it connects to will need 2 outside air inlets. One for the DOAS connection and one for direct access to outside air for economizer action.
*  Should the DOAS have branches or just in/out nodes like OutdoorAirSystem? I favor directly replicating OutdoorAirSystem.
#### **June 25, 2016**
* Contacted Fred W. Porter at NORESCO. Here are his comments
 * The DOAS AHU mostly needs to control discharge air temps, preheat, dehumidify, possibly w/reheat, and these days utilize energy recovery. The OA flow to each AHU may vary based on DCV and AHU scheduling and so the OA through the DOAS needs to vary at the same time.  The exhaust to the DOAS will almost always include toilet exhaust and may include some relief air. EA back to the DOAS will usually be 20% less than the OA so this has a very significant effect on performance. These units are sometimes the same as the unit supplying OA to zonal fan coils, etc, or directly to a few zones, so the solution should really be a flexible extension of that DOAS representation I think. DOAS units for AHUs typically won’t try to achieve “neutral” DAT however, so usually won’t have reheat coils, wraparound heat pipes, or passive dehumidification wheels. (As soon as I say  typically or usually, I get some design with the exact opposite within a week.) They almost always deliver the air to  the “inlet,” i.e. the mixed air plenum of the recirc AHU.
* Larry Scheier confirmed that TRACE allows economizers on the AHU's in DOAS to AHU configurations. This means we will definitely need to add another outside air node (the economizer node) to the OutdoorAir:Mixer.  Brent Griffith suggested that we add it at the end of the input fields, allowing existing input files to work as is.
* From Fred W. Porter's comments, it is clear that many DOA systems feed zones and zone equipment directly  as well as air handling units. Brent suggested handling this by defining a separate air loop (AirLoopHVAC plus zone demand side) to handle this. This seems like a good solution to me.

### **Overview**
#### Configurations

Given the feedback in the comments section, we will be attempting to simulate a dierct outside air system (DOAS), typically on the building's roof, serving multiple air handling units (AHUs), typically on separate floors. The DOAS can also serve some zones directly. Each AHU will be able to operate in economizer mode by using an additional outside air node for the economizer air (equivalent to an outside air damper that can fully close.

#### EnergyPlus description
We will use the existing EnergyPlus object AirLoopHVAC:OutdoorAirSystem to describe the DOAS serving multiple air handlers. Let's call this use of AirLoopHVAC:OutdoorAirSystem an Outdoor Air System to Multiple Air Handlers (OAStoMAH) vs Outdoor Air System in Single Air Loop (OASinSAL). Just as  AirLoopHVAC:OutdoorAirSystem can currently supply treated outdoor air to a single AirLoopHVAC the new use of  AirLoopHVAC:OutdoorAirSystem will supply treated outdoor air to multiple AirLoopHVAC's. This would correspond in the real world to a DOAS supplying multiple air handlers. In the EnergyPlus model each AirLoopHVAC will still need an AirLoopHVAC:OutdoorAirSystem. But additionally an AirLoopHVAC:OutdoorAirSystem can connect to each AirLoopHVAC through the outdoor air inlet of the OutdoorAir:Mixer contained in its AirLoopHVAC:OutdoorAirSystem.

Since the OAStoMAH will supply multiple AirLoopHVAC's, we need a splitter to divide its output among the AirLoopHVAC's. Similarly we will need to use a mixer to combine the relief air coming from the AirLoopHVAC's and send it to the OAStoMAH. We will borrow from AirLoopHVAC:ZoneSplitter and AirLoopHVAC:ZoneMixer but will define new objects AirLoopHVAC:OutdoorAirSystem:Mixer and AirLoopHVAC:OutdoorAirSystem:Splitter. As noted above, we will add an additional outside air node to Outdoor:AirMixer to allow for economizer action.

### **Approach**
The demand side (the AirLoopHVAC's) will set the ventilation air flow rate for the OAStoMAH. The OAStoMAH can't be simulated without knowing how much ventilation air the AirLoopHVAC's need. On the other hand the AirLoopHVAC's can't be simulated without knowing the temperature and humidity ratio of the ventilation air being supplied to them. This appears to be a situation that requires iteration between the OAStoMAH and the AirLoopHVAC's that it is supplying, with an accompanying impact on solution complexity and run time.

*For simple ventilation air calculations*, including basic demand control ventilation, it is possible to separate the ventilation air flow demand calculation from the AirLoopHVAC simulation. This calculation currently resides in the subroutine *CalcOAController* in the module *MixedAir*. The calculation takes place when the *AirLoopHVAC:OutdoorAirSystem* is simulated during the simulation of the air loop. We propose to separate this calculation out of CalcOAController and place it in a separate module. The plan is then to:

1. calculate the ventilation demand from all the zones served by the air loops that are served by each  OAStoMAH;
2. Simulate the OAStoMAH;
3. simulate the AirLoopHVAC's pretty much as is done currently;

Note that the calculation of the zone exhaust air flow rate will need to be "moved up" in the simulation sequence since the zone exhaust can set a hard lower limit on the ventilation air flow rate for an air loop. This provides an opportunity to move the zone exhaust air into the function *CalcAirFlowSimple* (invoked from *ManageHVAC* and elsewhere). The zone exhaust air flow could then be balanced against infiltration and simple ventilation air flow for all the zones in an air loop. If the zone exhaust exceeds the total of zone infiltration, simple zone ventilation, and air loop ventilation air flow induced infiltration could be apportioned to the zones in the air loop *before* the actual HVAC simulation takes place. This would eliminate the nagging problem of unbalanced zone exhaust flow. **Note:** *isn't this already being done or partially done?*

*For more complex ventilation demand calculations*, such as the system-based ventilation rate procedure, the ventilation demand depends on knowing the system supply air flow rate, which requires simulating the air loop and zone equipment. There are several ways to proceed:

1. use the last HVAC time step values in the ventilation rate procedure calculation;
2. use the last time step values for the air loop ventilation air inlet conditions, simulate the air loops, simulate the OutdoorAirSystemHVAC, then simulate the air loops again;
3. create some general iterative procedure.

Here we will start seeing some impact on run time. Our proposal is to use method 1 to start with

*In summary* there will need to be considerable rearranging of the current high-level HVAC simulation code, but with considerable benefit in increased capability and modest increase on run time or solution complexity. We hope to be able to reuse much of the code currently used for simulating AirLoopHVAC:OutdoorAirSystem's.

*Sizing:* we propose to use the existing *Sizing:System* object for  OAStoMAHs. The OAStoMAHs will be sized to the sum of its air loop design minimum ventilation air flow rates.
### **Implementation**
#### High Level Scheme
* Simulate AirLoopHVAC:OutdoorAirSystem:Mixer's. Call the data array OASysMixer.
  1.  Loop over the AirLoopHVAC:OutdoorAirSystem:Mixer's
       * Loop over the inlet nodes (air loop OA mixer relief nodes). Combine the inlet flows, temperatures, humidity ratios etc. and put the results on the outlet node (an OAStoMAH return node.
* Loop over AirLoopHVAC:OutdoorAirSystem's. If it is a OAStoMAH, simulate it. If it is a OASinSAL, skip.
  1. Loop over PrimaryAirSystem's served by each OAStoMAH.
      * Call SimOAController().
  2. Sum the AirLoopFlow().MinOutAir flows and place on the inlet node of the OAStoMAH.
  3. Simulate the OAStoMAH. Place the outlet node results on the corresponding inlet node of the AirLoopHVAC:OutdoorAirSystem:Splitter or the outside air node of a DOAS air loop.
  4. Simulate the AirLoopHVAC:OutdoorAirSystem:Splitter.
       * Pass the conditions ( temperature, humidity ratio, etc.) on to the outlet nodes. Split the flow according to the AirLoopFlow().MinOutAir flows already calculated.

We're done! Now we move on to the existing calculation.

Except there are some issues.

1. As noted before, we're going to have to precalculate  exhaust air. The AirLoopHVAC:OutdoorAirSystem:Splitter will be able to accept air flow from exhaust fans. And SimOAController() needs to know the exhaust flow.
2. The current AirLoopHVAC:OutdoorAirSystem simulation doesn't simulate Controller:WaterCoil's. Instead it "moves them up" to to the encompassing air loop's controller list. It puts them first in the list so they are simulated first, so all is well. Obviously this won't work for OAStoMAH's, although we can retain it for OASinSAL's. We will need to call the Controller:WaterCoil iteration scheme in order to control the water coils. This adds some complexity to the use of AirLoopHVAC:OutdoorAirSystem's, but should not be too difficult.
#### Schematic Input

    OutdoorAirSysHVAC
      Name
      Controller List Name
      Equipment List Name
      Availability Manager List Name
      Splitter Name
      Mixer Name

    OutdoorAirSysHVAC:Mixer
      Name
      List of Air Loops

    OutdoorAirSysHVAC:Splitter
      List of Air Loops

*Question*: Do we need "List of Air Loops" twice? Aren't the the same? Then it could go at the end of OutdoorAirSysHVAC and we could automatically generate the splitter and mixer. This would mess up auto-drawing the configuration? Notice we are already auto-generating all node connections. Nodes would not occur as input in this scheme. They would be extracted from the components and the air loops. This means that every component that could occur in a DOAS would need a *GetMyOutletAirNode* function. Is this the route that we are going? Otherwise we will need some node name inputs. Note that AirLoopHVAC:ZoneSplitter etc. have node inputs. Should we just copy them for consistency? Connector:Splitter and Connector:Mixer use branches, which OutdoorAirSysHVAC doesn't have.

*Answer* The lists in the Mixer and Splitter might not be the same. For instance, we might want to add exhaust fan flow to the mixer. Then it would be a list of air loops and exhaust fans. It was agreed that use of node names in the input should be deprecated.



*Question*: What about DOAS direct to zone? You've only mentioned air loops.

*Answer* These will zones need to be lumped into a "do nothing" air loop by the user. I suspect component Duct will come in handy.

#### Schematic Input: 2nd try
Instead of defining a new OutAirSysHVAC object, I've decided to reuse AirLoopHVAC:OutdoorAirSystem. We will add 2 optional fields on the end for the splitter and mixer. We will distinguish the 2 types (serving 1 air loop or serving multiple air loops) in a type field in the data structure. Our input will now look like:

    AirLoopHVAC:OutdoorAirSystem
      Name
      Controller List Name
      Equipment List Name
      Availability Manager List Name
      Splitter Name
      Mixer Name

    AirLoopHVAC:OutdoorAirSystem:Splitter
      Name
      List of Air Loops

    AirLoopHVAC:OutdoorAirSystem:Mixer
      Name
      List of Air loops and exhaust fan outlet nodes



### **Testing/Validation Source(s):**
Creating an example / test file for this new capability will be a major task in itself. We will most likely start with RefBldgMediumOfficeNew2004_Chicago. Once we have this file, we will use it to compare a "do nothing" OutdoorAirSystemHVAC with the case where there is no OutdoorAirSystemHVAC. We can also compare cases where the ventilation air conditioning is done within the air loop versus where it is done by an OutdoorAirSystemHVAC.

### **IO Ref (draft)**

We propose a new object which will be similar to AirLoopHVAC:OutdoorAirSystem. Also new objects OutdoorAirSystemHVAC:Mixer and OutdoorAirSystemHVAC:Splitter.

#### **Proposed Repart Variable Names:**

#### **Proposed additions to meters:**

### **EngRef (draft)**
We aren't far enough along yet to do this. The *Air Loop* and *Air Loop Simulation* sections will need extensive revising.

### **Example file and Transition changes:**

### **Other documents:**


