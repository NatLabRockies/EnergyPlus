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

### **Overview**
We will create a new top level object called OutdoorAirSystemHVAC. Just as  AirLoopHVAC:OutdoorAirSystem supplies treated outdoor air to a single AirLoopHVAC the new object OutdoorAirSystemHVAC will supply treated outdoor air to multiple AirLoopHVAC's. This would correspond in the real world to a DOAS supplying multiple air handlers. In the EnergyPlus model each AirLoopHVAC will still need an AirLoopHVAC:OutdoorAirSystem. The OutdoorAirSystemHVAC will connect to each AirLoopHVAC through the outdoor air inlet of the OutdoorAir:Mixer contained in its AirLoopHVAC:OutdoorAirSystem.

Since OutdoorAirSystemHVAC will supply multiple AirLoopHVAC's, we need a splitter to divide the output of the OutdoorAirSystemHVAC among the AirLoopHVAC's. Similarly we will need to use a mixer to combine the relief air coming from the AirLoopHVAC's and send it to the OutdoorAirSystemHVAC. We will borrow from AirLoopHVAC:ZoneSplitter and AirLoopHVAC:ZoneMixer but will define new objects OutdoorAirSystemHVAC:Mixer and OutdoorAirSystemHVAC:Splitter.

### **Approach**
The demand side (the AirLoopHVAC's) will set the ventilation air flow rate for the OutdoorAirSystemHVAC. The OutdoorAirSystemHVAC can't be simulated without knowing how much ventilation air the AirLoopHVAC's need. On the other hand the AirLoopHVAC's can't be simulated without knowing the temperature and humidity ratio of the ventilation air being supplied to them. This appears to be a situation that requires iteration between the OutdoorAirSystemHVAC and the AirLoopHVAC's that it is supplying, with an accompanying impact on solution complexity and run time.

*For simple ventilation air calculations*, including basic demand control ventilation, it is possible to separate the ventilation air flow demand calculation from the AirLoopHVAC simulation. This calculation currently resides in the subroutine *CalcOAController* in the module *MixedAir*. The calculation takes place when the *AirLoopHVAC:OutdoorAirSystem*

1. calculate the ventilation demand from all the zones served by the air loops that are served by each  OutdoorAirSystemHVAC;
2. Simulate the OutdoorAirSystemHVAC;
3. simulate the AirLoopHVAC's pretty much as is done currently;

Note that the calculation of the zone exhaust air flow rate will need to be "moved up" in the simulation sequence since the zone exhaust can set a hard lower limit on the ventilation air flow rate for an air loop. This provides an opportunity to move the zone exhaust air into the function *CalcAirFlowSimple* (invoked from *ManageHVAC* and elsewhere). The zone exhaust air flow could then be balanced against infiltration and simple ventilation air flow for all the zones in an air loop. If the zone exhaust exceeds the total of zone infiltration, simple zone ventilation, and air loop ventilation air flow induced infiltration could be apportioned to the zones in the air loop *before* the actual HVAC simulation takes place. This would eliminate the nagging problem of unbalanced zone exhaust flow. **Note:** *isn't this already being done or partially done?*

*For more complex ventilation demand calculations*, such as the system-based ventilation rate procedure, the ventilation demand depends on knowing the system supply air flow rate, which requires simulating the air loop and zone equipment. There are several ways to proceed:

1. use the last HVAC time step values in the ventilation rate procedure calculation;
2. use the last time step values for the air loop ventilation air inlet conditions, simulate the air loops, simulate the OutdoorAirSystemHVAC, then simulate the air loops again;
3. create some general iterative procedure.

Here we will start seeing some impact on run time. Our proposal is to use method 1 to start with

*In summary* there will need to be considerable rearranging of the current high-level HVAC simulation code, but with considerable benefit in increased capability and modest increase on run time or solution complexity. We hope to be able to reuse much of the code currently used for simulating AirLoopHVAC:OutdoorAirSystem's.

*Sizing:* we propose to use the existing *Sizing:System* object for  *OutdoorAirSystemHVAC*. *OutdoorAirSystemHVAC* will be sized to the sum of its air loop design minimum ventilation air flow rates.

### **Testing/Validation Source(s):**
Creating an example / test file for this new capability will be a major task in itself. Once we have this file, we will use it to compare a "do nothing" OutdoorAirSystemHVAC with the case where there is no OutdoorAirSystemHVAC. We can also compare cases where the ventilation air conditioning is done within the air loop versus where it is done by an OutdoorAirSystemHVAC.

### **IO Ref (draft)**

We propose a new object which will be similar to AirLoopHVAC:OutdoorAirSystem. Also new objects OutdoorAirSystemHVAC:Mixer and OutdoorAirSystemHVAC:Splitter.

#### **Proposed Repart Variable Names:**

#### **Propose additions to meters:**

### **EngRef (draft)**
We aren't far enough along yet to do this. The *Air Loop* and *Air Loop Simulation* sections will need extensive revising.

### **Example file and Transition changes:**

### **Other documents:**


