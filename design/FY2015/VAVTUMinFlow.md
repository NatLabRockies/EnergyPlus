# **NFP for Defaulting of VAV Box Minimum Flow and Maximum Heating Flow**
## **7/1/2015 Fred Buhl**

###  **Justification for Feature Update**

The minimum air flow setting for VAV terminal boxes has important implications for system control, energy consumption, and occupant comfort. Traditionally minimum fractions have been set to 30 or 40 per cent of maximum flow in order to maintain zone air mixing and prevent central fan stalling. In the past ten years HVAC engineers have started to use minimum stops of as low as 10 per cent. This allows better temperature control without reheating and prevents overcooling. Results from ASHRAE RP-1515 show that considerable energy savings can be achieved with increased occupant comfort by using low minimum flow fractions.

Currently in EnergyPlus the user is required to specify the minimum flow rate for each VAV terminal box. There is no defaulting available. This imposes an unnecessary burden on the user since there may be many terminal units and the user may not know the minimum VAV stop. Most users have no knowledge of correct values for minimum flow fraction or of the implications of minimum flow fraction for system energy use and system control. We propose to add a reasonable defaulting scheme for the minimum VAV flow rate or flow fraction that will reflect modern best practice.

The defaults for maximum heating flow also do not reflect current practice. The default for  *Damper Heating Action* is *Normal*, which means that the damper stays at its  minimum position during heating. With lower minimums this will result in inadequate air flow for heating. The default should be changed to *Reverse*: the damper will be able to partially open during heating. Again the typical user would have no knowledge of how damper action could affect the heating capability of the terminal unit. Most example files use normal action in conjunction with high minimum flows. This reflects outdated practice and will tend to mislead the user.

### **Conference Call Conclusions**

* Mike Witte: Why can't we have an option to set the minimum cfm ratio to the minimum outdoor air ratio?

This is certainly possible. The result of all the outdoor air calculations done in the zone and system sizing calculations is stored in *FinalZoneSizng().MinOA*. This includes the VRP calculations and the result of applying *Zone Maximum Outdoor Air Fraction* from *Sizing:System*. We could add an option to *Zone Minimum Air Flow Method* to have the default set by *MinOA*. For instance: \key MinOutdoorAir. But on thinking this over, I'm
not sure I want to do this. For one thing, this could set the min cfm ratio unrealistically low.

* Can we address the following from Mike Rosenberg at PNNL?

Currently we manually calculate system OA for MZ systems based on Standard 62.1 VRP. To do this, we run a design day simulation to determine primary airflow to each zone first. Then we manually raise the minimum damper position of critical zones to keep the system ventilation efficiency greater than 0.6. This prevents very high percentages of OA for occupancies such as schools, and is typical design practice.  This is a time consuming process and we would like to automate it using the E+ built in system sizing VRP. But we are concerned that without the step of increasing minimum damper position at design we will have very high OA levels. Does the E+ VRP calculation assume minimum damper position and maximum OA? I understand you can increase the damper position dynamically to limit the % OA using the Mechanical Ventilation Controller using the Zone Maximum OA Fraction, but that is not standard practice. In general, we would like an overview of how E+ does the system OA sizing when using VRP method and what happens during the annual simulation. Is it possible we will not get required OA during low load situations?

*Answer:* For the VRP method, for cooling, the ventilation efficiency for multi-path systems is

    SysCoolingEv = 1.0 + Xs * Fb / Fa - ZoneOAFrac * Ep * Fc / Fa;

For single-Path it is

    SysCoolingEv = 1.0 + Xs - ZoneOAFrac;

Clearly ZoneOAFrac is key to the ventilation efficiency. For multi-path systems the denominator of ZoneOAFrac is basically the design cooling volumetric flow rate. This should never be a problem. For single-path systems the denominator is *FinalZoneSizing( CtrlZoneNum ).DesCoolVolFlowMin*, the design minimum cooling flow rate. This is a problem. *DesCoolVolFlowMin* is user input from *Sizing:Zone* items N10 - N12 shown below. Most users will never enter a value for for any of these inputs. In which case *DesCoolVolFlowMin* will be defaulted to .15 cfm/sqft. At a low-end rule-of-thumb 1 cfm/sqft supply air flow rate, this is a min cfm ratio of 0.15 and in most cases it will be lower. This would likely yield a large ZoneOAFrac and thus a low system ventilation efficiency. It might be wise to force the system ventilation efficiency to be greater than 60% by back-calculating a *DesCoolVolFlowMin* that will give a minimum of 60%. It looks like we have all the information needed to do this. This would be a smart default for *Cooling Minimum Air Flow Fraction* in *Sizing:Zone*. This value could then be passed up to the VAV terminal unit for sizing the minimum damper position as described below.

* Brent Griffith: Can't we default fan minimum flow ratio to be the same as the minimum flow ratios of the terminal units?

It should be straightforward to use *FinalSysSizing().DesCoolVolFlowMin* (see below) to default the central fan minimum flow ratio. But this wouldn't reflect the actual values for minimum air flow fraction that the VAV terminal units end up with as input. There may be defaulted and user inputs. So in the *SizeFan* routine we would need to sum up the terminal unit minimum flow rates and calculate a minimum fan flow ratio. It looks like the terminal unit sizing is done first so this would be fairly straight forward. Or we could do the summing in *SizeSys* (the VAV TU sizing routine) and save the result in *PrimaryAirSystem*.



### **Overview**

#### **VAV box minimum flow ratio**
Currently the VAV box minimum flow ratio is specified in the following fields:

    A5 , \field Zone Minimum Air Flow Input Method
         \required-field
         \type choice
         \key Constant
         \key FixedFlowRate
         \key Scheduled
         \note Constant = Constant Minimum Air Flow Fraction (a
         \note fraction of Maximum Air Flow Rate)
         \note FixedFlowRate = Fixed Minimum Air Flow Rate (a fixed
         \note minimum air volume flow rate)
         \note Scheduled = Scheduled Minimum Air Flow Fraction (a
         \note fraction of Maximum Air Flow)
    N2 , \field Constant Minimum Air Flow Fraction
         \note This field is used if the field Zone Minimum Air Flow
         \note Input Method is Constant
         note If the field Zone Minimum Air Flow Input Method is
         \note Scheduled, then this filed is optional; if a value is
         \note entered, then it is used for sizing normal-action
         \note reheat coils.
         \note If both this field and the following field are
         \note entered, the larger result is used.
    N3 , \field Fixed Minimum Air Flow Rate
         \type real
         \units m3/s
         \note This field is used if the field Zone Minimum Air Flow
         \note Input Method is FixedFlowRate
         \note If the field Zone Minimum Air Flow Input Method is
         \note Scheduled, then this field is optional; if a value is
         \note enrered, then it is used for sizing normal-action
         \note coils.
         \note If both this field and the previous field are entered,
         \note the larger result is used.
    A6 , \field Minimum Air Flow Fraction Schedule Name
         \type object-list
         \object-list ScheduleNames
         \note This field is used if the field Zone Minimum Air Flow
         \note Input Method is Scheduled
         \note Schedule values are fractions, 0.0 to 1.0.
         \note If the field Constant Minimum Air Flow Fraction is
         \note blank, then the average of the minimum and maximum
         \note schedule values is used for sizing normal-action
         \note reheat coils.

If the *Zone Minimum Air Flow Input Method* is *Constant* and *Constant Minimum Air Flow Fraction* is blank there is no error message and the minimum stop is set to zero. For *Normal* action VAV boxes (which stay at the minimum for heating) this results in no heating. For *FixedFlowRate* we get the same result. For *Scheduled* a blank *Minimum Air Flow Fraction Schedule Name* results in a missing schedule error.

Almost all of the VAV example files have the minimum flow fraction set to 0.3. Most users without expert HVAC knowledge will follow this example. We propose to default the *Zone Minimum Air Flow Input Method* field to *Constant* and set *Constant Minimum Air Flow Fraction* to default to *autosize*. We will use RP-1515 as a guide to setting the autosized values. Currently California Standard Title-24 sets the maximum minimum air flow ratio to 20%. ASHRAE Standard 90.1 is doing likewise.

In *Sizing:Zone* we have the following input:

    N10,\field Cooling Minimum Air Flow per Zone Floor Area
      \type real
      \units m3/s-m2
      \minimum 0
      \default .000762
      \note default is .15 cfm/ft2
      \note This input is used if Cooling Design Air Flow Method is
      \note DesignDayWithLimit
    N11,\field Cooling Minimum Air Flow
      \type real
      \units m3/s
      \minimum 0
      \default 0
      \note This input is used if Cooling Design Air Flow Method is
      \note DesignDayWithLimit
    N12,\field Cooling Minimum Air Flow Fraction
      \note fraction of the Cooling design Air Flow Rate
      \type real
      \minimum 0
      \default 0
      \note This input is currently used in sizing the Fan minimum
      \note Flow Rate.
      \note It does not currently affect other component autosizing.

The first 2 inputs are used to set a minimum supply flow rate for a zone even if it has little or no cooling load. In other words they are used to set a minimum for the design cooling supply air flow rate; i.e., setting a minimum for the maximum cooling supply air flow rate. As such they are not relevant to setting a minimum flow or flow fraction for VAV terminal units.

The third input - *Cooling Minimum Air Flow Fraction* - if entered by the user- can be picked up and used for defaulting the minimum flow fraction for VAV terminal units. This input is currently used along with the two previous inputs to set *FinalZoneSizing().DesCoolVolFlowMin* in *UpdateZoneSizing* in *ZoneEquipmentManager*. If *Cooling Minimum Air Flow Fraction* is input it will very likely set the value for *FinalZoneSizing().DesCoolVolFlowMin*.

    FinalZoneSizing( CtrlZoneNum ).DesCoolVolFlowMin = max(
     FinalZoneSizing( CtrlZoneNum ).DesCoolMinAirFlow,
     FinalZoneSizing( CtrlZoneNum ).DesCoolMinAirFlow2,
     FinalZoneSizing( CtrlZoneNum ).DesCoolVolFlow *
     FinalZoneSizing( CtrlZoneNum ).DesCoolMinAirFlowFrac );
  where

    FinalZoneSizing( CtrlZoneNum ).DesCoolMinAirFlow2 =
     FinalZoneSizing( CtrlZoneNum ).DesCoolMinAirFlowPerArea *
     Zone( ZoneIndex ).FloorArea * Zone( ZoneIndex ).Multiplier *
     Zone( ZoneIndex ).ListMultiplier;

*FinalZoneSizing().DesCoolVolFlowMin* is used in *SimAirServingZones* in the Std 62.1 VRP calculations. It is also summed into *FinalSysSizing().DesCoolVolFlowMin* :

     FinalSysSizing( AirLoopNum ).DesCoolVolFlowMin +=
      FinalZoneSizing( CtrlZoneNum ).DesCoolVolFlowMin;
but *FinalSysSizing().DesCoolVolFlowMin* is never used. It was intended to set the minimum air flow rate for system fans.

In summary, *FinalZoneSizing().DesCoolVolFlowMin* can be used to set a default for the minimum air flow or air flow fraction in VAV terminal units.

#### **VAV box maximum heating flow ratio**

Currently the VAV terminal unit maximum heating flow ratio is specified in the following fields:

    A10, \field Damper Heating Action
       \type choice
       \key Normal
       \key Reverse
       \default Normal
    N7 , \field Maximum Flow per Zone Floor Area During Reheat
       \type real
       \units m3/s-m2
       \autocalculatable
       \note Used only when Reheat Coil Object Type =
       \note Coil:Heating:Water and Damper Heating Action = Reverse
       \note When autocalculating, the maximum flow per zone is set to
       \note 0.002032 m3/s-m2 (0.4 cfm/sqft)
       \note This optional field limits the maximum flow allowed in
       \note reheat mode.
       \note If this field and the following field are left blank, the
       \note maximum flow will not be limited.
       \note At no time will the maximum flow rate calculated here
       note exceed the value of Maximum Air Flow Rate.
    N8 , \field Maximum Flow Fraction During Reheat
       \type real
       \autocalculatable
       \note Used only when Reheat Coil Object Type =
       \note Coil:Heating:Water and Damper Heating Action = Reverse
       \note When autocalculating, the maximum flow fraction is set to
       \note the ratio of 0.002032 m3/s-m2 (0.4 cfm/sqft) multiplied
       \note by the zone floor area and the Maximum Air Flow Rate.
       \note This optional field limits the maximum flow allowed in
       \note reheat mode.
       \note If this field and the previous field are left blank, the
       \note maximum flow will not be limited.
       \note At no time will the maximum flow rate calculated here
       \note exceed the value of Maximum Air Flow Rate.
So these inputs are already well defaulted. But *Sizing:Zone* has very similar inputs:

    N14,\field Heating Maximum Air Flow per Zone Floor Area
      \type real
      \units m3/s-m2
      \minimum 0
      \default .002032
      \note default is .40 cfm/ft2
      \note This field is used to size the heating design flow rate
      \note when Heating Design Air Flow Method = Flow/Zone.
      \note This input is used for autosizing components when Heating
      \note Design Air Flow Method = DesignDayWithLimit.
    N15,\field Heating Maximum Air Flow
      \type real
      \units m3/s
      \minimum 0
      \default .1415762
      \note default is 300 cfm
      \note This input is used for autosizing components when Heating
      \note Design Air Flow Method = DesignDayWithLimit.
    N16,\field Heating Maximum Air Flow Fraction
      \note fraction of the Heating Design Air Flow Rate
      \type real
      \minimum 0
      \default 0.3
      \note This input is used for autosizing components when Heating
      \note Design Air Flow Method = DesignDayWithLimit.

which are well defaulted also. In this case we just have to pick up any user input values from *Zone:Sizing* and use them to override the defaults set by the terminal unit object.
### **Approach**

#### **VAV box minimum flow ratio**
We will input the VAV terminal unit minimum flow rate with the following logical steps, listed from highest to lowest priority of choice.

1. *Zone Minimum Air Flow Input Method* will be defaulted to *Constant*.

2.  If the user inputs a value (*Constant*, *FixedFlowRate* or *Scheduled*) for *Zone Minimum Air Flow Input Method* and inputs a value in the appropriate place in the following 3 fields, or if *Zone Minimum Air Flow Input Method* is blank (defaulted to *Constant*) and the user enters a value in *Constant Minimum Air Flow Fraction* the program will use the user entered value.

3. If the 3 fields *Constant Minimum Air Flow Fraction*, *Fixed Minimum Air Flow Rate*, and *Minimum Air Flow Fraction Schedule Name* are blank the program will attempt to get a default from the *Sizing:Zone* object. First the program will check whether the appropriate *Sizing:Zone* object exists. Next it will check whether field *Cooling Minimum Air Flow Fraction* in *Sizing:Zone* has been entered by the user. A flag for user input for this field will have to added to the Zone Sizing arrays. If these 2 conditions are met, the minimum flow ratio will be set to a value derived from *FinalZoneSizing().DesCoolVolFlowMin*.

4. If no zone sizing calculation was performed, or the user didn't enter a value for *Cooling Minimum Air Flow Fraction* in *Sizing:Zone*, the value .000762 m2/m3 (.15 cfm/sqft) times the zone floor area will be used to derive a minimum flow ratio. We will impose a minimum of 0.1 and a maximum of 0.4 on the resulting value. We will also impose a minimum flow ratio set by the zone design ventilation requirement. Steps 3 and  4 will be performed in the terminal unit *Sizing* function, while the actual IDD fields will be set to *autocalculate*.

**Note** The VAV air mass flow rate is checked against the current (time step) ventilation requirement if *Design Specification Outdoor Air Object Name* is entered in the VAV terminal unit input. When the field is blank the minimum air flow isn't checked against the ventilation requirement. We will leave this unchanged.

#### **VAV box maximum heating flow ratio**

1. *Damper Heating Action* will be defaulted to *Reverse*.

2. If the user enters a value for *Maximum Flow per Zone Floor Area During Reheat* or *Maximum Flow Fraction During Reheat*  and *Damper Heating Action* is *Reverse* the program will use the user entered value.

3. If  *Damper Heating Action* is *Reverse* and *Maximum Flow per Zone Floor Area During Reheat* and *Maximum Flow Fraction During Reheat* are blank  the program will attempt to get a default from the *Sizing:Zone* object. First the program will check whether the appropriate *Sizing:Zone* object exists. Next the program will check whether *Heating Maximum Air Flow per Zone Floor Area* or *Heating Maximum Air Flow* or *Heating Maximum Air Flow Fraction* along with the proper *Heating Design Air Flow Method* have been entered by the user. A flag for user input for these fields will have to added to the Zone Sizing arrays. If these two conditions have been met, a default value will be derived from *FinalZoneSizing( ).DesHeatVolFlowMax*.

4. If no zone sizing calculation was performed, or the user didn't enter a value for any of the 3 *Zone:Sizing*  inputs, the current defaults will be used. Note that in the IDD *Maximum Flow per Zone Floor Area During Reheat* and *Maximum Flow Fraction During Reheat* defaults will be set to *autocalculate*. The defaulting described above will be done in the relevant *Sizing* function.

**Question** Currently if both terminal unit inputs are blank, the maximum reheat air flow is limited only by the maximum air flow. How shall we propagate this input into our new scheme? Should we leave *Reverse* alone and introduce a *ReverseWithLimits*?

Yes - it was agreed during the conference call that a new choice *ReverseWithLimits* would be added to *Damper Heating Action*.




### **Testing/Validation/Data Sources:**
We will compare results with new defaulted inputs with results from user inputs of the same values. We will compare newly defaulted input results with old results in the example files.

### **IO Ref (draft):**
*required*

### **IDD Object(New)**
*Include as appropriate*

### **IDD Object(s) (Revised):**
    A5 , \field Zone Minimum Air Flow Input Method
       \type choice
       \key Constant
       \key FixedFlowRate
       \key Scheduled
       \key MinOA
       \note Constant = Constant Minimum Air Flow Fraction (a fraction
       \note of Maximum Air
       \note Flow Rate)
       \note FixedFlowRate = Fixed Minimum Air Flow Rate (a fixed
       \note minimum air volume flow
       \note rate)
       \note Scheduled = Scheduled Minimum Air Flow Fraction
       \note (a fraction of Maximum Air Flow
       \default Constant
    N2 , \field Constant Minimum Air Flow Fraction
       \type real
       \ autosizable
       \default autosize
       \note This field is used if the field Zone Minimum Air Flow
       \note Input Method is Constant
       \note If the field Zone Minimum Air Flow Input Method is
       \note Scheduled, then this field is optional; if a value is
       \note entered, then it is used for sizing normal-action
       \note reheat coils.
       \note If both this field and the following field are entered,
       \note the larger result is used.
    N3 , \field Fixed Minimum Air Flow Rate
       \type real
       \units m3/s
       \autosizable
       \default autosize
       \note This field is used if the field Zone Minimum Air Flow
       \note Input Method is FixedFlowRate.
       \note If the field Zone Minimum Air Flow Input Method is
       \note Scheduled, then this field is optional; if a value is
       \note entered, then it is used for sizing normal-action
       \note reheat coils.
       \note If both this field and the previous field are entered, the
       \note larger result is used.
    A6 , \field Minimum Air Flow Fraction Schedule Name
       \type object-list
       \object-list ScheduleNames
       \note This field is used if the field Zone Minimum Air Flow
       \note Input Method is Scheduled
       \note Schedule values are fractions, 0.0 to 1.0.
       \note If the field Constant Minimum Air Flow Fraction is blank,
       \note then the average of the minimum and maximum schedule
       \note values is used for sizing normal-action reheat coils.
.....

    A10, \field Damper Heating Action
       \type choice
       \key Normal
       \key Reverse
       \key ReverseWithLimits
       \default ReverseWithLimits
    N7 , \field Maximum Flow per Zone Floor Area During Reheat
       \type real
       \units m3/s-m2
       \autocalculatable
       \default autocalculate
       \note Used only when Reheat Coil Object Type =
       \note Coil:Heating:Water and Damper Heating Action = Reverse
       \note When autocalculating, the maximum flow per zone is set to
       \note 0.002032 m3/s-m2 (0.4 cfm/sqft)
       \note This optional field limits the maximum flow allowed in
       \note reheat mode.
       \note If this field and the following field are left blank, the
       \note maximum flow will not be limited.
       \note At no time will the maximum flow rate calculated here
       note exceed the value of Maximum Air Flow Rate.
    N8 , \field Maximum Flow Fraction During Reheat
       \type real
       \autocalculatable
       \default autocalculate
       \note Used only when Reheat Coil Object Type =
       \note Coil:Heating:Water and Damper Heating Action = Reverse
       \note When autocalculating, the maximum flow fraction is set to
       \note the ratio of 0.002032 m3/s-m2 (0.4 cfm/sqft) multiplied
       \note by the zone floor area and the Maximum Air Flow Rate.
       \note This optional field limits the maximum flow allowed in
       \note reheat mode.
       \note If this field and the previous field are left blank, the
       \note maximum flow will not be limited.
       \note At no time will the maximum flow rate calculated here
       \note exceed the value of Maximum Air Flow Rate.

### **Proposed Report Variables**
### **Proposed Additions to Meters**
### **Eng Ref (draft):**
*required*
### **Example File and Transition Changes**
The example files will remain unchanged. There will be no transition changes.
### **Other Documents**
See https://escholarship.org/uc/item/3jn5m7kg for ASHRAE RP-1515 report.
