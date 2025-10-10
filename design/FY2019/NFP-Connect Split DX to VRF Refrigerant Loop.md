﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿Connecting Split DX AHU Components to VRF Refrigeration Loops
================

**Richard Raustad, Florida Solar Energy Center**

 - 11/7/19 - Draft NFP
 - 12/4/19 - several off-line design strategy discussions
 - 12/4/19 - Design Document added at end of NFP
 - 12/5/19 - New OO method of storing air system factory pointers has tentatively been implemented and the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects has been added as a valid main air loop component.

## Justification for New Feature ##

One of the recent developments in the variable refrigerant flow systems (VRF) field is the ability to connect terminal devices other than indoor split fan coils to a VRF refrigerant loop. These new terminal devices include DOAS AHUs and rooftop mounted split DX AHUs for space conditioning and ventilation.

Nearly all VRF applications need ventilation. Previously, ventilation had to be provided by separate packaged rooftop type DOAS equipment since VRF indoor units cannot handle raw outdoor air directly and VRF condensing units could not serve DOAS equipment. In addition, one of the energy efficiency limitations of VRF systems is the lack of an outdoor air economizer cycle. Both challenges have now been solved.

Multiple major manufacturers such as Daikin, Mitsubishi, LG, and Toshiba-Carrier offer the ability to connect split DX DOAS AHUs to VRF condensing units. In addition, Toshiba-Carrier condensing units can be connected to multiple rooftop-mounted split DX AHUs that provide space conditioning and ventilation and can offer outdoor air economizer cycles. These developments extend the application of VRF systems and the energy efficiency of these systems.

Currently, VRF systems in EnergyPlus only allow ZoneHVAC terminals to be connected to a VRF refrigerant loop.

## E-mail and  Conference Call Conclusions ##


## Overview ##

This task will expand the capabilities of the VRF model to allow terminal devices such as split DX DOAS AHUs and split DX AHUs for space conditioning and ventilation to be connected to VRF refrigerant loops. The following figure is excerpt from the Eng. Ref. (Figure 16.51, pg 1163, V9.2) and modified to include VRF air loop and outdoor air equipment (red circle).

Figure 1: ![Figure 1](https://github.com/NREL/EnergyPlus/blob/NFP---Connect-split-DX-to-VRF-refrigerant-loop/design/FY2019/VRFTU%20Allowed%20in%20Airloop-OASys.png) Variable Refrigerant Flow Heat Pump (draw through fan placement)

## Approach ##

The common connection of VRF TU's and the VRF condenser is the ZoneTerminalUnitList object. All ZoneHVAC:TerminalUnit:VariableRefrigerantFlow objects connected to a single VRF condenser (AirConditioner:VariableRefrigerantFlow or AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:\*) are listed in the ZoneTerminalUnitList object and this objects name is entered as an input in the VRF condenser object.

The most straight-forward approach appears to be allowing the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object to be connected as zone (existing), air loop and outdoor air system equipment. Using this methodology, limited changes to the AirConditioner:VariableRefrigerantFlow and AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:\* objects are anticipated.

## Controls:
  ZoneHVAC Equipment: no changes (currently load based control)
  AirloopHVAC Equipment: requires thermostat control zone input, load based control
  OutdoorAir Equipment: controlled to outlet node set point temperature (and humidity ratio?)

## Testing/Validation/Data Sources ##

New test files will be created using VRF TUs in the air loop and outdoor air system.

## Input Output Reference Documentation ##

IO Ref text will be updated to include these new capabilities.

## Input Description ##

Changes to the IDD appear to be minimal as no new objects will be added to E+. The only changes to model inputs are the allowed location of the TU object and thermostat location for controlling air loop equipment.

```
ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,
        \memo Zone terminal unit with variable refrigerant flow (VRF) DX cooling and heating coils
        \memo (air-to-air heat pump). The VRF terminal units are served by an
        \memo AirConditioner:VariableRefrigerantFlow or
        \memo AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:* system.
        \min-fields 19
  A1 ,  \field Zone Terminal Unit Name
        \required-field
        \type alpha
        \reference ZoneTerminalUnitNames
        \reference DOAToZonalUnit
        \reference ZoneEquipmentNames

        -- new references as follows (not yet tested) --
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
        \reference-class-name validOASysEquipmentTypes
        \reference validOASysEquipmentNames

        -- new field --
  A19;  \field Controlling Zone or Thermostat Location
        \note Used only for AirloopHVAC equipment on a main branch
        \type object-list
        \object-list ZoneNames
        \note Zone name where thermostat is located. Required for control of air loop equipment.
```

## Outputs Description ##

No new outputs are anticipated.

## Engineering Reference ##

New schematic diagram (similar to above) and corresponding text description.

## Example File and Transition Changes ##

New example file will be included.
No transition required.

## References ##

    Manufacturer  Documentation
    ------------  -------------------------------------------------------------------
    Daikin        https://www.daikinac.com/content/commercial/ventillation-units/dvs-dedicated-outside-air-system
    Mitsubishi    https://www.mitsubishipro.com/products/city-multi-vrf/ventilation/premisys-dedicated-outdoor-air-systems
    Mitsubishi    http://meus1.mylinkdrive.com/item/PremiSys.html
    LG            https://lghvac.com/commercial/product-type/?productTypeId=a2x44000003XR0s&iscommercial=true
    Carrier       https://www.carrier.com/commercial/en/us/products/variable-refrigerant-flow/toshiba-carrier-vrf-products/toshiba-carrier-indoor-units/40qq/


## Design Documentation ##

Adding another air loop component, in this case ZoneHVAC:TerminalUnit:VariableRefrigerantFlow, perpetuates historical programming of individual component calls without regard to future code changes for air loop equipment and code maintenance. The Simulate call arguments for various air loop models use a CompIndex for array based access. The UnitarySystem was the first air loop model to use a pointer based Sim call, however, this pointer was not accessible to all air loop equipment (e.g. VRF, ChangeoverBypass).

The declaration of the pointer, used by upper level managers (i.e., Air loop equipment, OA systems, zone equipment), was specific to the UnitarySystem and this declaration is no longer adequate.

DataAirLoop.hh

    struct OutsideAirSysProps
        std::vector<UnitarySystems::UnitarySys *> compPointer;

DataAirSystems.hh

    struct AirLoopCompData // data for an individual component
        UnitarySys *compPointer; // pointer to UnitarySystem

DataZoneEquipment.hh

    struct EquipList
        std::vector<UnitarySystems::UnitarySys *> compPointer;

At the time that UnitarySystem was refactored to use the factory/pointer method, the set up for the air loop branch object changed from:

    } else if (componentType == "AIRLOOPHVAC:UNITARYSYSTEM") {
        PrimaryAirSystem(AirSysNum).Branch(BranchNum).Comp(CompNum).CompType_Num = UnitarySystem;

to:

    } else if (componentType == "AIRLOOPHVAC:UNITARYSYSTEM") {
        PrimaryAirSystem(AirSysNum).Branch(BranchNum).Comp(CompNum).CompType_Num = UnitarySystemModel;
        UnitarySystems::UnitarySys thisSys;
        PrimaryAirSystem(AirSysNum).Branch(BranchNum).Comp(CompNum).compPointer = thisSys.factory(
                                DataHVACGlobals::UnitarySys_AnyCoilType, PrimaryAirSystem(AirSysNum).Branch(BranchNum).Comp(CompNum).Name, false, 0);

And the Sim call was changed from this:

    } else if (SELECT_CASE_var == UnitarySystem) { // 'AirLoopHVAC:UnitarySystem'
        SimUnitarySystem(CompName, FirstHVACIteration, AirLoopNum, CompIndex, HeatingActive, CoolingActive);

to:

    } else if (SELECT_CASE_var == UnitarySystemModel) { // 'AirLoopHVAC:UnitarySystem'
        CompPointer->simulate(CompName, FirstHVACIteration, AirLoopNum, CompIndex, HeatingActive, CoolingActive, OAUnitNum, OAUCoilOutTemp, ZoneEquipFlag);

where: CompPointer = PrimaryAirSystem(AirSysNum).Branch(BranchNum).Comp(CompNum).compPointer as provided by the UnitarySystem factory.

As is done with plant equipment, a new class will be created such that a pointer can be declared in air systems managers for use by all types of air system equipment. This class is defined in a new header file, DataHVACSystems.hh. Virtual functions are included where this pointer is used to access model specific information from outside the scope of UnitarySystem (e.g., the Sim routine and get functions for air nodes used by air loop DOAS systems). Note that unit tests will need to be updated where this pointer is used to set or access object data and model functions.

```
#ifndef DataHVACSystems_hh_INCLUDED
#define DataHVACSystems_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// base class for all HVAC systems
class HVACSystemData
{

public:

    // Default Constructor
    HVACSystemData()
    {
    }

    virtual void simulate(std::string const &Name,
        bool const firstHVACIteration,
        int const &AirLoopNum,
        int &CompIndex,
        bool &HeatActive,
        bool &CoolActive,
        int const OAUnitNum,         // If the system is an equipment of OutdoorAirUnit
        Real64 const OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
        bool const ZoneEquipment,    // TRUE if called as zone equipment
        Real64 &sysOutputProvided,   // sensible output at supply air node
        Real64 &latOutputProvided    // latent output at supply air node
    ) = 0;

    virtual void sizeSystem(bool const FirstHVACIteration, int const AirLoopNum) = 0;
    virtual int getAirInNode(std::string const &UnitarySysName, int const ZoneOAUnitNum) = 0;
    virtual int getAirOutNode(std::string const &UnitarySysName, int const ZoneOAUnitNum) = 0;

};


} // namespace EnergyPlus

#endif // DataHVACSystems_hh_INCLUDED
```

This class is inherited by the HVAC model where a factory call returns a pointer stored in a local array for later use.

UnitarySystems.hh

    struct UnitarySys : HVACSystemData

and it's the HVACSystemData class that is used to declare the pointer array in various managers (instead of using the UnitarySys class).


DataAirLoop.hh

    struct OutsideAirSysProps
        std::vector<HVACSystemData *> compPointer;

DataAirSystems.hh

    struct AirLoopCompData // data for an individual component
        HVACSystemData *compPointer; // pointer to HVAC system

DataZoneEquipment.hh

    struct EquipList
        std::vector<HVACSystemData *> compPointer;

Instead of CompIndex being used on the call to the Sim function, the compPointer accesses that data and Sim function directly (as long as all model Sim functions are unified with the same arguments). Eventually, CompIndex will no longer be needed.

Now that the VRF terminal unit is allowed in the air loop, it should simply be a matter of inheriting the new class and storing the pointer in the same array previously used only for UnitarySystems. Similar changes to other HVAC equipment can now be made during ongoing refactoring efforts. The good news is that since the UnitarySystem can be used anywhere in the air simulation (i.e., air loops, OA systems, zone equipment, and zone OA units) this new OO method (with the help of reviewers updates) can be used with any HVAC equipment type.
