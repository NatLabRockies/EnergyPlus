Variable Speed DX Coil Enhancements
================

**B. Griffith, Energy Archmage Company, for GARD/NREL/DOE**

 - 11/18/2016, first draft
 - 2/13/2017, revised to reflect unitary system changes


## Justification for New Feature ##

Usability is improved when all low-level component models can be used in all the various systems and parent HVAC devices that might use such a device.  There are many places in EnergyPlus that allow single speed DX coils but not variable speed DX coils.  This project aims to make available the option of using the variable speed air-source DX cooling and heating coils in six different objects that do not currently allow them.  Using the variable speed coil model with just one speed level also makes it a single-speed DX coil model, with a slightly different formulation, and it should be allowed.  This first step of "connecting the wires" to the VS coil models will facilitate any follow on work to implement variable controls, where appropriate.  Interface developers can streamline DX coil library data using one input formulation for both single-speed and variable-speed DX coils and still have access to all the models addressed here.

## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

This project aims to add Coil:Cooling:DX:VariableSpeed (and Coil:Heating:DX:VariableSpeed in one case) to the following IDF objects:

    - Coil:Heating:Desuperheater
    - Coil:WaterHeating:Desuperheater
    - CoilSystem:Cooling:DX:HeatExchangerAssisted
    - ZoneHVAC:WindowAirConditioner
    - AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass
    - Dehumidifier:Desiccant:System

Although the model inputs will now allow for the VS DX coil models, in some cases they will still only load the coil as if it was a single speed.  Will try to make some progress but full VS controls for everything here will likely require a follow on phase of development.  For the desuperheater applications when the VS coil providing the waste heat is already able to operate with variable control then the available desuperheat will be adjusted accordingly.  Support for air-to-air VS coil models in AirLoopHVAC:UnitarySystem will be improved.

## Approach ##

Modify individual parent models to add support for the VS air source DX coil types.  Implementation will not attempt major refactoring of all the different parent models but will work within the existing patterns.  Source code will be modified to revise get input, coil information processing, and coil simulation calls, etc.  in the following source code files:

    - HeatingCoils.cc ( air heating desuperheater )
    - WaterThermalTanks.cc ( water heating desuperheater )
    - HVACHXAssistedCoolingCoil.cc ( HX assisted DX coil system )
    - WindowAC.cc ( window air conditioner )
    - HVACUnitaryBypassVAV.cc ( change over bypass packaged systems )
    - DesiccantDehumidifiers.cc ( desiccant system with coordinated control and desuperheater regen heat options from upstream DX coil)

Create test files using AirLoopHVAC:UnitarySystem with setpoint-based control that run Coil:Cooling:DX:VariableSpeed and Coil:Heating:DX:VariableSpeed and fix any problems. Create variations using Coil:Heating:Desuperheater and CoilSystem:Cooling:DX:HeatExchangerAssisted and fix any problems found related to using VS coil models in HeatRecovery.cc and HVACUnitarySystem.cc.

## Testing/Validation/Data Sources ##

Variable speed coil example files that mimic the single-speed coil example files will be created and results compared.

## Input Output Reference Documentation ##

I/O reference documentation revised to show that the VS coils are allowed, in these files: group-desiccant-dehumidifiers.tex, group-unitary-equipment.tex, group-zone-forced-air-units.tex, and group-heating-and-cooling-coils.tex

## Input Description ##

The input changes are summarized by the following IDD changes:

  - in Coil:Cooling:DX:VariableSpeed
	add  \reference DesuperHeatingCoilSources to Name field.  Will get used by name field object-list in Coil:Heating:Desuperheater and Coil:WaterHeating:Desuperheater.

  - in Coil:Heating:Desuperheater,
	input field called Heating Source Object Type, add \key Coil:Cooling:DX:VariableSpeed

  - in Coil:WaterHeating:Desuperheater,
	input field called Heating Source Object Type, add \key Coil:Cooling:DX:VariableSpeed

  - in CoilSystem:Cooling:DX:HeatExchangerAssisted,
	input field called Cooling Coil Object Type, add \key Coil:Cooling:DX:VariableSpeed.
	input field called Cooling Coil Name, add \object-list CoolingCoilsDXVariableSpeed

  - in ZoneHVAC:WindowAirConditioner,
	input field called Cooling Coil Object Type, add \key Coil:Cooling:DX:VariableSpeed
	input field called DX Cooling Coil Name, add \object-list CoolingCoilsDXVariableSpeed

  - in AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass,
	input field called Cooling Coil Object Type, add \key Coil:Cooling:DX:VariableSpeed
	input field called Cooling Coil Name, add \object-list CoolingCoilsDXVariableSpeed
	input field called Heating Coil Object Type, add \key Coil:Heating:DX:VariableSpeed
	input field called Heating Coil Name, add \object-list HeatingCoilsDXVariableSpeed

  - in Dehumidifier:Desiccant:System,
	input field called Companion Cooling Coil Object Type, add \key Coil:Cooling:DX:VariableSpeed
	input field called Companion Cooling Coil Name, add \object-list CoolingCoilsDXVariableSpeed



## Outputs Description ##

No new outputs.

## Engineering Reference ##

No new models, just connecting existing models together.

## Example File and Transition Changes ##

Approximately seven new test files that demonstrate using variable speed DX coils in these applications.

5Zone_Unitary_HXAssistedCoil
5Zone_Unitary_VSDesuperheater
5Zone_Unitary_VSDesuperheatWaterHeater
DesiccantDehumidifierWithAirToAirCoil
WindowACAirToAir.idf

## References ##

none.



