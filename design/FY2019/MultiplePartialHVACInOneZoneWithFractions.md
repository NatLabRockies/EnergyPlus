Multiple/Partial HVAC Control, now with user specified fractions!
=================================================================

**Noel Merket, NREL**

 - 4 January 2019 - Original NFP
 - 9 January 2019 - Revision to use sequential loads (Option B)

## Justification for New Feature ##

During the implementation of Mike Witte's [Improve Control of Multiple HVAC in One Zone](https://github.com/NREL/EnergyPlus/blob/8d7fcb46b41e083a30a872f2b9fc23ca65912c2e/design/FY2017/MultiplePartialHVACInOneZone.md) NFP it was decided to not specify the fraction of the heating or cooling load that each zone equipment should meet and instead make a `UniformLoad` option available which evenly spreads the load between equipment. In some applications, particularly in residential buildings, the load is not evenly distributed between HVAC equipment. An example would be a central dx coil unit for 80% of the house and a window unit for the addition. Specifying HVAC load fractions is required for Home Energy Score, Energy Rating Index, and Weatherization Assistant modeling.

## E-mail and  Conference Call Conclusions ##

1/8/2019 - The initial approach did not appropriately handle more complicated cases such as:

```
ZoneHVAC:EquipmentList,
   Living Zone Equipment_1,               !- Name
   ???                                    !- Load Distribution Scheme
   ZoneHVAC:EnergyRecoveryVentilator,     !- Zone Equipment 1 Object Type
   ERV_1,                                 !- Zone Equipment 1 Name
   1,                                     !- Zone Equipment 1 Cooling Sequence
   1,                                     !- Zone Equipment 1 Heating or No-Load Sequence
   AirTerminal:SingleDuct:Uncontrolled,   !- Zone Equipment 2 Object Type
   CentralAirConditioner_1,               !- Zone Equipment 2 Name
   2,                                     !- Zone Equipment 2 Cooling Sequence
   2,                                     !- Zone Equipment 2 Heating or No-Load Sequence
   AirTerminal:SingleDuct:Uncontrolled,   !- Zone Equipment 3 Object Type
   CentralAirConditioner_2,               !- Zone Equipment 3 Name
   3,                                     !- Zone Equipment 3 Cooling Sequence
   3,                                     !- Zone Equipment 3 Heating or No-Load Sequence
   ZoneHVAC:Dehumidifier:DX,              !- Zone Equipment 4 Object Type
   Dehumidifier_1,                        !- Zone Equipment 4 Name
   4,                                     !- Zone Equipment 4 Cooling Sequence
   4;                                     !- Zone Equipment 4 Heating or No-Load Sequence
```

In that case we'd want the ERV to operate on all the load and add some load as they do. Then we want to be able to split the load between `CentralAirConditioner_1` and `CentralAirConditioner_2` with user specified fractions. Finally we want the dehumidifier to see all the remaining latent load. After some discussion with Mike Witte and Scott Horowitz, we now think the best approach is going to be more like ["Option B"](https://github.com/NREL/EnergyPlus/blob/8d7fcb46b41e083a30a872f2b9fc23ca65912c2e/design/FY2017/MultiplePartialHVACInOneZone.md#option-b---remaining-load) in the original NFP. I have revised the Overview and Approach section to reflect our current thinking.

## Overview and Approach ##


When the `SequentialLoad` load distribution scheme option is selected, it will read the fractions from the following pair of fields that will be added to the `ZoneHVAC:EquipmentList` for each equipment:

```
Zone Equipment <x> Sequential Cooling Fraction
Zone Equipment <x> Sequential Heating or No-Load Fraction
```

Similar to (and uapologetically lifted from) Mike Witte's ["Option B"](https://github.com/NREL/EnergyPlus/blob/8d7fcb46b41e083a30a872f2b9fc23ca65912c2e/design/FY2017/MultiplePartialHVACInOneZone.md#option-b---remaining-load), the fractions will be applied as follows:

The zone equipment manager multiplies the current remaining load by the applicable schedule fraction before calling a given piece of equipment.  Here is an example where the radiant cooling panels meet 25% of the cooling load and the air system meets the remaining load.

```
  ZoneHVAC:EquipmentList,
    SPACE2-1 Eq,             !- Name
    SequentialLoad,          !- Load Distribution Scheme
    ZoneHVAC:CoolingPanel:RadiantConvective:Water,  !- Zone Equipment 1 Object Type
    SPACE2 Cooling Panel,    !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    0.25,                    !- Zone Equipment 1 Sequential Cooling Fraction
    ,                        !- Zone Equipment 1 Sequential Heating or No-Load Fraction
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type
    SPACE2-1 ATU,            !- Zone Equipment 2 Name
    2,                       !- Zone Equipment 2 Cooling Sequence
    2,                       !- Zone Equipment 2 Heating or No-Load Sequence
    1.0,                     !- Zone Equipment 2 Sequential Cooling Fraction
    ;                        !- Zone Equipment 2 Sequential Heating or No-Load Fraction
```

If the current cooling load is 1000W, the cooling panel will be passed a load of 0.25*1000=250W. If the panel can provide this, then the remaining load of 750W is passed to the air distribution unit. If the cooling panel only provides 200W, then 1000-200=800W will be passed to the air distribution unit.

*Pros* - If the first piece of equipment cannot provide the requested load, the next one tries to meet to meet the unmet portion.

*Cons* - The schedule fractions are not intuitive. Because the schedule is applied to the remaining load, the schedule fractions will not add up to 1.0.

While the fractions are less intuitive, it will better be able to handle more complicated scenarios, such as the one described in the Email/Conference Call notes from 1/8.

## Testing/Validation/Data Sources ##

Build example files and check results.

## Input Output Reference Documentation ##

See docs source changes on this branch. (Not done yet, but not hard to imagine).

## Engineering Reference ##

I don't think changes will be required for this document.

## Example File and Transition Changes ##

Proposed example file:

 - Create a copy of the `House-2FurnaceAC-UniformLoad.idf` example file but with this new load distribution scheme.

Transition will be required for `ZoneHVAC:EquipmentList`.


## References ##

- Mike Witte - [Improve Control of Multiple HVAC in One Zone](https://github.com/NREL/EnergyPlus/blob/8d7fcb46b41e083a30a872f2b9fc23ca65912c2e/design/FY2017/MultiplePartialHVACInOneZone.md) NFP.
