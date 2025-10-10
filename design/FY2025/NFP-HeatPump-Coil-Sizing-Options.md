Add Heat Pump Sizing Options Proposal and Design
==================================================

**Richard Raustad, Florida Solar Energy Center**

 - First draft: May 1, 2025
 - Modified Date:

## Justification for New Feature ##

Depending on the equipment model, EnergyPlus sizes heat pump coils to meet a cooling load or uses the larger of cooling and heating to size the coils. This new feature intends to unify the sizing methodology across available heat pump equipment types. Proper heat pump sizing includes methods for sizing in cooling and heating dominated climates and sizing according to specific Standards (e.g., allows for up to 30% over sizing for HPs as described by ACCA Manual S, see reference section). According to ACCA Manual S, for heating dominated climates the cooling coil capacity would not be allowed to exceed 115% - 125%, depending on number of speeds, of the cooling load where supplemental heating coils would be required for more disparate differences in cooling and heating loads. It is not intended that this new feature address that aspect of a Standard. It may be possible to add another sizing option (e.g., \key ACCAManualS) in the new proposed input field for Heating Coil Sizing Method and will be included, if possible, to address that sizing methodology.



## E-mail and  Conference Call Conclusions ##

** na **

## Overview ##

The AirloopHVAC:UnitarySystem uses the larger of the cooling and heating loads to size a heat pump. The global variable DXCoolCap is used to size the heating coil used in a heat pump.

    if (this->m_HeatPump && (state.dataSize->CurZoneEqNum == 0 || !isWSVarSpeedCoolCoil)) {
        EqSizing.AirFlow = true;
        EqSizing.AirVolFlow = max(EqSizing.CoolingAirVolFlow, EqSizing.HeatingAirVolFlow);
        if (this->m_CoolingCoilType_Num != HVAC::Coil_CoolingWaterToAirHPVSEquationFit &&
                this->m_HeatingCoilType_Num != HVAC::Coil_HeatingWaterToAirHPVSEquationFit) {
            EqSizing.Capacity = true;
            EqSizing.DesCoolingLoad = max(EqSizing.DesCoolingLoad, EqSizing.DesHeatingLoad);
            EqSizing.DesHeatingLoad = EqSizing.DesCoolingLoad;
            state.dataSize->DXCoolCap = EqSizing.DesCoolingLoad; <-- **
        }
            
Other heat pump equipment types size the cooling coil and assumes the heating coil is the same size. Similar changes will be made to those equipment models.

## Approach ##

The proposed approach is to add 2 new fields to sizing objects that allow equipment models to choose which load to use for coil sizing. An example of the new fields is shown here for system sizing. Actual objects to be revised is discussed later in the design section. ACCA Manual S describes sizing of heat pumps to be within 130% of the cooling load.

```
Sizing:System,
  A12, \field Heating Coil Sizing Method
       \type choice
       \key UseLegacySizingmethod
       \key CoolingCapacity
       \key HeatingCapacity
       \key GreaterOfHeatingOrCooling
       \key ACCAManualS   <-- if this is easy to implement then why not just include
       \default UseLegacySizingmethod
  N28, \field Maximum Heating Capacity To Cooling Load Sizing Ratio
       \type real
       \units W/W
       \minimum 1.0
       \default 1.0
```

The approach will update areas of code that set DXCoolCap to include the new sizing inputs so that coils size the same way now as before (i.e., relatively few areas of code will be changed) except that the new information is used. It is expected that the answer for legacy input files will not change given that the default value for Heating Coil Sizing Method = UseLegacySizingmethod. This will eliminate the need for transition rules. Another option is to remove the key choice for UseLegacySizingmethod and use a default of CoolingCapacity, however this may require a transition rule for UnitarySystem heat pumps (a system with a DX cooling and DX heating coil of specific types).

## Testing/Validation/Data Source(s): ##

Demonstrate that the new input fields result in specific coil sizes depended on selected inputs. Unit tests will be added to demonstrate the new feature.

## Input Output Reference Documentation ##

Update affected objects with new input fields.

## Engineering Reference ##

Update to include changes/options for coil sizing.

## Example File and Transition Changes ##

An example file will be modified to demonstrate the use of availability schedule. Simulation results will be examined and sample results will be provided.

Transition is not likely required unless review requests a change in the default sizing method. In that case transition rule(s) would be required.

## Proposed Report Variables: ##

None.

## References ##

[ACCA Manual S](https://www.acca.org/standards/technical-manuals/manual-s)

## Design Considerations ##

The following areas of code have been reviewed and are targeted for code revisions:

UnitarySystem.cc

AirloopHVAC:UnitarySystem already uses the maximum cooling or heating load to size the coils. This logic will be updated to use the new user specified value (e.g., cooling, heating, or larger of the two) to set coil size and the new input for Maximum Heating Capacity To Cooling Load Sizing Ratio.

    if (this->m_HeatPump && (state.dataSize->CurZoneEqNum == 0 || !isWSVarSpeedCoolCoil)) {
        EqSizing.AirFlow = true;
        EqSizing.AirVolFlow = max(EqSizing.CoolingAirVolFlow, EqSizing.HeatingAirVolFlow);
        if (this->m_CoolingCoilType_Num != HVAC::Coil_CoolingWaterToAirHPVSEquationFit &&
                this->m_HeatingCoilType_Num != HVAC::Coil_HeatingWaterToAirHPVSEquationFit) {
            EqSizing.Capacity = true;
            
Start new code:

            if (SizeToGreaterOfHeatingOrCooling && EqSizing.DesHeatingLoad > EqSizing.DesCoolingLoad * MaxHeatingCapacityToCoolingLoad) {
                EqSizing.DesCoolingLoad = EqSizing.DesHeatingLoad / MaxHeatingCapacityToCoolingLoad;
            } else if (SizeToHeatingCapacity) {
                if (EqSizing.DesHeatingLoad > EqSizing.DesCoolingLoad * MaxHeatingCapacityToCoolingLoad) {
                    EqSizing.DesCoolingLoad = EqSizing.DesHeatingLoad / MaxHeatingCapacityToCoolingLoad;
                } else {
                // this logic is subject to change as needed (i.e., it's a max ratio, not absolute)
                    EqSizing.DesHeatingLoad = EqSizing.DesCoolingLoad * MaxHeatingCapacityToCoolingLoad;
                }
            } else if (SizeToCoolingCapacity) {
                // this logic is subject to change as needed (i.e., it's a max ratio, not absolute)
                EqSizing.DesHeatingLoad = EqSizing.DesCoolingLoad * MaxHeatingCapacityToCoolingLoad;
            }
            
End new code:

            state.dataSize->DXCoolCap = EqSizing.DesHeatingLoad;
        }
            
Other heat pump equipment types use the cooling load for setting coil size. This code is typically handeled in the coil models and will be updated similarly, or revised entirely using the above logic wherever DXCoolCap is set.

DXCoils.cc is where cooling capacity is used to set heating capacity for non-UnitarySystem heat pump equipment.

    if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_CoolingSingleSpeed || thisDXCoil.DXCoilType_Num == HVAC::CoilDX_CoolingTwoSpeed ||
        thisDXCoil.DXCoilType_Num == HVAC::CoilDX_CoolingTwoStageWHumControl || thisDXCoil.DXCoilType_Num == HVAC::CoilVRF_Cooling ||
        thisDXCoil.DXCoilType_Num == HVAC::CoilVRF_FluidTCtrl_Cooling) {
        state.dataSize->DXCoolCap = thisDXCoil.RatedTotCap(Mode); <-- **
    }

    if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_CoolingTwoStageWHumControl) {
    } else if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_HeatingEmpirical || thisDXCoil.DXCoilType_Num == HVAC::CoilVRF_Heating ||
               thisDXCoil.DXCoilType_Num == HVAC::CoilVRF_FluidTCtrl_Heating) {
        state.dataSize->DataCoolCoilCap = state.dataSize->DXCoolCap; <-- **
    } else if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_HeatPumpWaterHeaterPumped ||
               thisDXCoil.DXCoilType_Num == HVAC::CoilDX_HeatPumpWaterHeaterWrapped) {
    } else if (thisDXCoil.DXCoilType_Num == HVAC::CoilVRF_FluidTCtrl_Cooling) {
    } else if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_MultiSpeedCooling) {
    } else if (thisDXCoil.DXCoilType_Num == HVAC::CoilDX_MultiSpeedHeating) {
    } else {
    }

VariableSpeedCoil.cc uses a similar approach.

    if (varSpeedCoil.VSCoilType == HVAC::Coil_CoolingWaterToAirHPVSEquationFit ||
            varSpeedCoil.VSCoilType == HVAC::Coil_CoolingAirToAirVariableSpeed) {
        state.dataSize->DXCoolCap = varSpeedCoil.RatedCapCoolTotal;
    }


The proposed approach to unify heat pump sizing methodology is to add new fields to Sizing:Zone and/or DesignSpecification:ZoneHVAC:Sizing, and Sizing:System. The new Heating Coil Sizing Method field will provide a choice for sizing the heat pump heating coil to equal the cooling or heating load or to use the greater load. Some equipment currently use CoolingCapacity and other equipment use GreaterOfHeatingOrCooling so selection of a default value may cause conflicts in legacy input files.

    Sizing:Zone,
      A20, \field Heating Coil Sizing Method
           \type choice
           \key CoolingCapacity
           \key HeatingCapacity
           \key GreaterOfHeatingOrCooling
           \default CoolingCapacity
      N30, \field Maximum Heating Capacity To Cooling Load Sizing Ratio
           \type real
           \units W/W
           \minimum 1.0
           \default 1.0

    DesignSpecification:ZoneHVAC:Sizing,
      A7,  \field Heating Coil Sizing Method
           \type choice
           \key CoolingCapacity
           \key HeatingCapacity
           \key GreaterOfHeatingOrCooling
           \default CoolingCapacity
      N19; \field Maximum Heating Capacity To Cooling Load Sizing Ratio
           \type real
           \units W/W
           \minimum 1.0
           \default 1.0
 
    Sizing:System,
      A12, \field Heating Coil Sizing Method
           \type choice
           \key CoolingCapacity
           \key HeatingCapacity
           \key GreaterOfHeatingOrCooling
           \default CoolingCapacity
      N28, \field Maximum Heating Capacity To Cooling Load Sizing Ratio
           \type real
           \units W/W
           \minimum 1.0
           \default 1.0

These are the existing objects that will require code changes and whether those objects already include a heating coil multiplier/sizing ratio. The new field for Maximum Heating Capacity To Cooling Load Sizing Ratio (which defaults to 1.0) will be incorporated into any changes made to implement this new feature. Note that this new sizing ratio is a limit and not a multiplier, heaeting and cooling coil sizing should limit to this ratio as opposed to forcing this sizing ratio.

    AirloopHVAC:UnitarySystem,
      N1 , \field DX Heating Coil Sizing Ratio
      
    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,
      N10, \field Rated Heating Capacity Sizing Ratio


    AirLoopHVAC:UnitaryHeatPump:AirToAir,
    AirLoopHVAC:UnitaryHeatPump:WaterToAir,
    AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass,
    AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,

