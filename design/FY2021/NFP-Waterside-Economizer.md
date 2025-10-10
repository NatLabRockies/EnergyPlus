NFP Waterside Economizer
================

**R. Raustad, FSEC/UCF**

 - Original Date - 1/13/21
 - Revision Date - 1/20/21 to address comments

**B. Nigusse, FSEC/UCF**
 - Revision Date - 4/13/21 expand design document and scope review


**Task Description**:

**Ability to properly model and control water-side economizers for self contained units** - Ability to model water side economizer in AirloopHVAC:UnitarySystem which enables free cooling and switching of DX compressor in that object.
## Justification for New Feature ##


Manufacturers have a configuration option that includes free cooling using a cooling coil dedicated to waterside economizing connected to the condenser plant (e.g., cooling towers). The cooling coil is upstream of a DX cooling coil which reduces the DX compressor energy use in cooling mode. EnergyPlus does not currently have a mechanism to properly model control of this configuration.

## E-mail and  Conference Call Conclusions ##

Brent 1/15/21

I would suggest to change CoilSystem:Cooling:DX to CoilSystem:Cooling and allow that parent to use chilled water coils.  Seems hard to justify yet-another-coil-system object.  Or add the controls needed to unitary system and model with two unitary objects in series.

MJWitte 1/19/21

1. I like adding CoilSystem:Cooling:Water because it can be used for more than this application.
2. In theory, one could use AirloopHVAC:UnitarySystem with just one coil and no fan, but it still wouldn't have the extra control offset field, and it's become so large that a focused object makes more sense.
3. One could overthink (not me, of course) the naming and think about how all of the existing CoilSystem objects relate to each other. Some of them have controls in them, some do not (*:HeatExchangerAssisted).
4. These all started with CoilSystem:Cooling:DX because we needed a way to control a dx coil that was loose on the branch. Using CoilsSystem for this was perhaps a confusing choice because of the overlap with CoilSystem:*:HeatExchangerAssisted.
5. Some consolidation might be nice, but why? Look at how hard it's been to accomplish that with the DX coils and UnitarySystems. Oh, right, we haven't. So, perhaps better to keep the set of objects and move towards a common code base for them (i.e., they're all part of the same class).
6. So, I guess I'm convincing myself that the proposed object name is ok as-is.
7. I'm fine with leaving off the sensor node.
8. The MultiMode dehumidification control maybe should have a different name. It's only for activating an HXassisted coil, right?

RR: Yes, MultiMode would be used for the HX. If that choice were used the HX would cycle on based on need, otherwise it would always be on. HeatExchangerControlled may be more accurate.

MJWitte: Or HeatExchangerControl or ActivateHeatExchanger. I think including HeatExchanger in the option name is good. If some other way of controlling humidity comes along then a new option can be added.

MJWitte Comment on Github: 04/29/21
@rraustad has demonstrated a method for the AirLoopHVACX:UnitarySystem code to simulate CoilSystem:Cooling:DX in #8729. It should be possible to do the same here with the small addition of support for the cutoff
temperature difference. That means this feature will not need a new source file, just a new input function in UnitarySystem. This may reduce (or eliminate) the need for changes in other source files as well.

MJWitte added that, my vision of this is that CoilSystem:Cooling:Water would be just an abbreviated synonym for AirloopHVAC:UnitarySystem. It would instantiate a unitarysystem internally and that code will do all the work.
With that approach, there's no need for CoilWaterSystem.hh or cc

B Bigusse on Github: 04/29/21: Agreed to use Mike's suggestion. This appraoch is a minimal code changes in the get input to track that the unitarysystem is intended as a water-side economizer, sets a flag if the condition is favorable
for economizer to operate and if logics to turn off the UnitarySystem if the condition is not favorable. So, the new codes added does not change any existing controls but sets a control flgas to be able to turn on or
off the system coil depending on the condition.


## Overview ##

The waterside economizer is used as a pre-cooling coil in packaged DX equipment.

**The following description was excerpt from Trane Product Catalog PKG-PRC024D-EN**.

The waterside economizer substantially reduces the compressor energy requirements because
it uses the cooling water before it enters the condensers. Disadvantages include higher airside pressure drop and a higher head on condenser water pumps.

![Figure 1 Schematic of waterside economizer](NFP-Waterside-Economizer-Fig01.PNG)

**Figure 1 Schematic of waterside economizer (Extracted from Trane Product Catalog)**


**Unit Operating Limits**

**Airflow**:
The minimum recommended airflow for proper VAV system staging and temperature control is 35 percent of nominal design airflow. Adjusting VAV boxes with the appropriate minimum settings prevents the self-contained unit from operating in a surge condition at airflows below this point. Continuous operation in a surge condition can cause fan failure.
Note: Contact your local Trane sales office for minimum airflow conditions.

**Water Flow**: Use 3 gpm/ton for optimum unit capacity and efficiency. Use 2.5 or 2 gpm/ton to reduce pump energy, cooling tower, and piping costs. However, these reduced water flows may impact unit capacity and efficiency by one or two percent. Consult factory for unit specific water flow ranges.

**Supply Air Temperature Control Unit Sequence of Operation
1** — Occupied Supply Air Temperature Control Cooling/Waterside Economizer
During occupied cooling mode, the waterside economizer option and mechanical cooling are used to control the supply air temperature. The supply air temperature setpoint is user defined.
After the fans run for 5 minutes, and a call for cooling is calculated, the Cooling Tower/Pump Command is turned on. After the Condenser Water flow switch closes proving flow, the
Compressors and/or Economizer will be allowed to start.
Waterside economizing enables when the unit’s entering water temperature is less than the
entering mixed air temperature. This is set at Waterside Economizer Enable Offset setpoint. The factory default is 7°F, but can be adjusted by the user.

The economizer acts as the first stage of cooling. If the economizer is unable to maintain the supply air setpoint, the unit controller will bring on the compressors as required to meet the setpoint. If the unit does not include an economizer, only mechanical cooling will satisfy cooling requirements.

**Discharge Air Cooling Setpoint** The Discharge Air Cooling Setpoint default valve of 55 degrees and the unit will control to this value when in the cooling mode. The setpoint can be adjusted 3 different ways.

 - Reset Based on Return Air Temperature: The Discharge Air Temperature can be reset based off the return air temperature which will work to keep this temepature between its heating and cooling setpoints.
 - Reset Based on Zone Temperature: The Discharge Air Temperature can be reset based off the return air temperature which will work to keep this temepature between its heating and cooling setpoints.
 - Reset Based on Zone Temperature: Zone reset is applied to the zone(s) in a building that tends to overcool or overheat. The supply air temperature setpoint is adjusted based on the temperature of the critical zone(s).This can have the effect of improving comfort and/or lowering energy usage.The user-defined parameters are the same as for outdoor air reset.

**End Trane Product Catalog excerpt**.


**Note**: The set point control methods will remain the responsibility of existing EnergyPlus SetPointManagers.

## Approach ##

The waterside economizer is simply a water coil with special controls. The coil is not active if the entering coil air temperature is not sufficiently higher than the entering water temperature by some user specified value (e.g., 7 °F or 3.89 °C) and is controlled to a set point temperature. These controls must be "actuated" by some means. Since the existing UnitarySystem already has 3 coils within the parent, the initial  proposal is to wrap the existing water cooling coils in a parent object and place this new object upstream of the UnitarySystem to achieve the goal of free cooling. There are several coil wrappers in EnergyPlus as follows:

    CoilSystem:Cooling:DX
    A6,  \field Cooling Coil Object Type
         \key Coil:Cooling:DX:SingleSpeed
         \key CoilSystem:Cooling:DX:HeatExchangerAssisted
         \key Coil:Cooling:DX:TwoSpeed
         \key Coil:Cooling:DX:TwoStageWithHumidityControlMode
         \key Coil:Cooling:DX:VariableSpeed
         \key Coil:Cooling:DX:SingleSpeed:ThermalStorage

    CoilSystem:Heating:DX,
    A3,  \field Heating Coil Object Type
         \key Coil:Heating:DX:SingleSpeed
         \key Coil:Heating:DX:VariableSpeed

    CoilSystem:Cooling:Water:HeatExchangerAssisted,
    A2 , \field Heat Exchanger Object Type
         \key HeatExchanger:AirToAir:FlatPlate
         \key HeatExchanger:AirToAir:SensibleAndLatent
    A4 , \field Cooling Coil Object Type
         \key Coil:Cooling:Water
         \key Coil:Cooling:Water:DetailedGeometry

    CoilSystem:Cooling:DX:HeatExchangerAssisted,
    A2 , \field Heat Exchanger Object Type
         \key HeatExchanger:AirToAir:FlatPlate
         \key HeatExchanger:AirToAir:SensibleAndLatent
         \key HeatExchanger:Desiccant:BalancedFlow
    A4 , \field Cooling Coil Object Type
         \key Coil:Cooling:DX:SingleSpeed
         \key Coil:Cooling:DX:VariableSpeed

It may be time to begin consolodating these exising wrappers as done with the Coil:Cooling:DX object. The name would simply be CoilSystem and contain any type of coil with an optional heat exchangera available for cooling coils. This big of a change may be too big for this task, however, keeping this in mind for this new object is highly desireable.

**Instead of incorporating a waterside economizer coil object into a packaged system, a generic method of providing free cooling is discussed here. The new object would be configured as a condenser plant demand side component and placed upstream of any existing coil types. A new wrapper will be developed to include the water coils. This wrapper could be specific to a new waterside economizer coil object or be more generic to allow branch water coils to be simulated without using the Controller:WaterCoil object. The more generic method is described here.**

**Note**: This new object may also be used on the demand side of a cooling plant to support existing coil types.

## Input Description ##


CoilSystem:Cooling:Water,
       \memo Virtual container component that consists of a water cooling coil
       \memo and its associated controls. This control object supports the
       \memo available water coil types and may be placed directly on an
       \memo air loop branch or in an outdoor air equipment list.
       \min-fields 6
   A1, \field Name
       \required-field
       \reference WaterCoilSystemName
       \type alpha
       \reference-class-name validBranchEquipmentTypes
       \reference validBranchEquipmentNames
       \reference-class-name validOASysEquipmentTypes
       \reference validOASysEquipmentNames
   A2, \field Air Inlet Node Name
       \required-field
       \type node
   A3, \field Air Outlet Node Name
       \required-field
       \type node
   A4, \field Availability Schedule Name
       \note Availability schedule name for this system. Schedule value > 0
       \note means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
   A5, \field Cooling Coil Object Type
       \type choice
       \required-field
       \key Coil:Cooling:Water
       \key Coil:Cooling:Water:DetailedGeometry
       \key CoilSystem:Cooling:Water:HeatExchangerAssisted
   A6, \field Cooling Coil Name
       \required-field
       \type object-list
       \object-list CoolingCoilsWater
   A7, \field Dehumidification Control Type
       \type choice
       \key None
       \key Multimode
       \key CoolReheat
       \default None
       \note None = meet sensible load only
       \note Multimode = activate water coil and meet sensible load.
       \note If no sensible load exists, and Run on Latent Load = Yes, and a latent
       \note load exists, the coil will operate to meet the latent load.
       \note If the latent load cannot be met the heat exchanger will be activated.
       \note IF Run on Latent Load = No, the heat exchanger will always be active.
       \note This control mode either switches the coil mode or allows the heat exchanger to
       \note be turned on and off based on the zone dehumidification requirements. Valid
       \note only with cooling coil type CoilSystem:Cooling:Water:HeatExchangerAssisted.
       \note CoolReheat = cool beyond the dry-bulb setpoint as required to meet the
       \note humidity setpoint. Valid with all cooling coil types. When a heat exchanger
       \note assisted cooling coil is used, the heat exchanger is locked on at all times.
       \note For all dehumidification controls, the max
       \note humidity setpoint on the Sensor Node is used.
       \note SetpointManager:SingleZone:Humidity:Maximum,
       \note SetpointManager:MultiZone:Humidity:Maximum, or
       \note SetpointManager:MultiZone:MaximumHumidity:Average, and
       \note SetpointManager:OutdoorAirPretreat (optional) objects.
   A8, \field Run on Sensible Load
       \type choice
       \key Yes
       \key No
       \default Yes
       \note If Yes, unit will run if there is a sensible load.
       \note If No, unit will not run if there is only a sensible load.
       \note Dehumidification controls will be active if specified.
   A9, \field Run on Latent Load
       \type choice
       \key Yes
       \key No
       \default No
       \note If Yes, unit will run if there is a latent load.
       \note even if there is no sensible load.
       \note If No, unit will not run only if there is a latent load.
       \note Dehumidification controls will be active if specified.
   N1; \field Minimum Air To Water Temperature Offset
       \note Coil will turn on as required when inlet air temperature is above
       \note water temperature by amount of offset. To model a waterside
       \note economizer connect to condenser loop and increase offset as desired.
       \type real
       \units C
       \minimum 0.0
       \default 0.0

Several considerations need to be resolved for this new object.

 - Should a Sensor Node Name input field be included, or force use of the object or coil outlet node (recommended and agreed to via comments)
 - Should a controller tolerance input field be included or assume a typical value of 0.001 for temperature control and 0.00001 for humidity control (no comments, assumed OK)
 - If the CoilSystem objects are consolidated, additional inputs would be needed for Use Outdoor Air DX Cooling Coil (A11) and Outdoor Air DX Cooling Coil Leaving Minimum Air Temperature (N1) to replicate all inputs for CoilSystem:Cooling:DX. Additional inputs would also be required for heat exchanger type and name. (discouraged via comments)

## Testing/Validation/Data Sources ##

Compare results of existing coil types with this new object. Also test performance of the new waterside economizer model with object attached to a condenser loop and upstream of a UnitarySystem.

## Input Output Reference Documentation ##

Add new IO section for new object.

## Outputs Description ##

	Uses existing unitary system report variables

	May be add additional new variables as needed:
	**Cooling Coil Water-Side Economizer Status**

## Engineering Reference ##

Add new Engineering section for new object.

## Example File and Transition Changes ##

No transition required. New example file(s) will be included.

## References ##

None.

## Design Document ##

The UnitarySystem is the only air-side parent that uses the pointer/factory method. The pointer of the class HVACSystemData. The same appraoach will be used for CoilWaterSystems.

SimAirServingZones::SimAirLoopComponent

    void SimAirLoopComponent(EnergyPlusData &state,
                             std::string const &CompName,
                             int CompType_Num,
                             bool FirstHVACIteration,
                             int AirLoopNum,       // Primary air loop number
                             int &CompIndex,
                             HVACSystemData *CompPointer); // HVACSystemData pointer



### UniatrySystem.hh ###
Modification to the UnitarySystem header file:

UnitarySystem.hh

namespace UnitarySystems {

    struct UnitarySysInputSpec
    {

        UnitarySysInputSpec();

        ~UnitarySysInputSpec()
        {
        }
    };

    struct UnitarySys : HVACSystemData
    {

        **Adds new private member variables:**

        bool m_waterSideEconomizerFlag;  // true if water-side economizer coil is active
        Real64 m_minAirToWaterTempOffset; // coil entering air to entering water temp offset

    public:
        // SZVAV variables

        **Adds new public member variables:**
        bool runWaterSideEconomizer;  // true if water-side economizer conditioon is favorbale
        int WaterSideEconomizerStatus; // water side economizer status flag, report variable

    public:
        UnitarySys(); // constructor

        ~UnitarySys() // destructor
        {
        }

        ** Adds new get input function for CoilSystem:Cooling:Water object**
        static void getCoilWaterSystemInputData(
            EnergyPlusData &state, std::string const &CoilSysName, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound);

    };

} // namespace UnitarySystems

struct UnitarySystemsData : BaseGlobalStruct
{

    // MODULE PARAMETER DEFINITIONS

    ** Adds new member variables for coilsystem**
    bool getCoilWaterSysInputOnceFlag = true;
    std::string const coilSysCoolingWaterObjectName = "CoilSystem:Cooling:Water";
    int numCoilWaterSystems = 0;

    void clear_state() override
    {
        getCoilWaterSysInputOnceFlag = true;
        numCoilWaterSystems = 0;
    }

    // Default Constructor
    UnitarySystemsData() = default;
};


### UniatrySystem.cc ###
UnitarySystem.cc

** Add Adds new get input function for CoilSystem:Cooling:Water object**
    void UnitarySys::getCoilWaterSystemInputData(
        EnergyPlusData &state, std::string const &CoilSysName, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound)
    {

        static const std::string routineName("UnitarySys::getCoilWaterSystemInputData: ");
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(state.dataUnitarySystems->coilSysCoolingWaterObjectName);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {

                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());

                if (!UtilityRoutines::SameString(CoilSysName, thisObjectName) && !state.dataUnitarySystems->getCoilWaterSysInputOnceFlag) continue;

                state.dataInputProcessing->inputProcessor->markObjectAsUsed(state.dataUnitarySystems->coilSysCoolingWaterObjectName, thisObjectName);
                ++state.dataUnitarySystems->numCoilWaterSystems;
                ++state.dataUnitarySystems->numUnitarySystems;

                std::string cCurrentModuleObject = state.dataUnitarySystems->coilSysCoolingWaterObjectName;

				set input values here as follows:
                UnitarySysInputSpec input_specs;

                input_specs.name = thisObjectName;
                input_specs.control_type = "Setpoint";
                input_specs.name = UtilityRoutines::MakeUPPERCase(thisObjectName);
                input_specs.air_inlet_node_name = UtilityRoutines::MakeUPPERCase(fields.at("air_inlet_node_name"));
                input_specs.air_outlet_node_name = UtilityRoutines::MakeUPPERCase(fields.at("air_outlet_node_name"));

                ...

                // now translate to UnitarySystem
                UnitarySys thisSys;
                thisSys.UnitType = cCurrentModuleObject;

                // set water-side economizer flag
                thisSys.m_waterSideEconomizerFlag = true;
                int sysNum = state.dataUnitarySystems->numUnitarySystems;
                thisSys.processInputSpec(state, input_specs, sysNum, errorsFound, ZoneEquipment, ZoneOAUnitNum);
                sysNum = getUnitarySystemIndex(state, thisObjectName);

                if (sysNum == -1) {
                    state.dataUnitarySystems->unitarySys.push_back(thisSys);
                } else {
                    state.dataUnitarySystems->unitarySys[sysNum] = thisSys;
                }
            }
            // at this point all CoilWaterSys objects must be read
            state.dataUnitarySystems->getCoilWaterSysInputOnceFlag = false;
        }
    }

### SimAirServingZones::GetAirPathData ###
Adds a "CoilSystem:Cooling:Water" coil type and a pointer to the airpath data

SimAirServingZones::GetAirPathData


    } else if (componentType == "AIRLOOPHVAC:UNITARYSYSTEM") {
        state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).Branch(BranchNum).
            Comp(CompNum).CompType_Num = UnitarySystemModel;
        UnitarySystems::UnitarySys thisSys;
        state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).Branch(BranchNum).
            Comp(CompNum).compPointer =
            thisSys.factory(state,
                            DataHVACGlobals::UnitarySys_AnyCoilType,
                            state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).
                                Branch(BranchNum).Comp(CompNum).Name,
                            false,
                            0);

    **New factory call: uses unitarysystem factory call**

    } else if (componentType == "COILSYSTEM:COOLING:WATER") {
        state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).Branch(BranchNum).
            Comp(CompNum).CompType_Num = CoilWaterSystem;
        CoilWaterSystems::CoilWaterSys thisSys;
        state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).Branch(BranchNum).
            Comp(CompNum).compPointer =
            thisSys.factory(state,
                            DataHVACGlobals::CoilWaterSys,
                            state.dataAirSystemsData->PrimaryAirSystems(AirSysNum).
                                Branch(BranchNum).Comp(CompNum).Name,
                            false,
                            0);

    } else {}...

### SimAirServingZones::SimAirLoopComponent ###

Adds three new argument to SimAirLoopComponent() to be able to retrive a compPointer during Sim():

    void SimAirLoopComponent(EnergyPlusData &state,
                             std::string const &CompName, // the component Name
                             int CompType_Num,            // numeric equivalent for component type
                             bool FirstHVACIteration,     // TRUE if first full HVAC iteration in an HVAC timestep
                             int AirLoopNum,              // Primary air loop number
                             int &CompIndex,              // numeric pointer for CompType/CompName -- passed back from other routines
                             HVACSystemData *CompPointer, // equipment actual pointer
                             int const &airLoopNum,       // index to AirloopHVAC
                             int const &branchNum,        // index to AirloopHVAC branch
                             int const &compNum           // index to AirloopHVAC branch component
    );


Adds new simulate call using override for HVACSystemData::simulate:

SimAirServingZones::SimAirLoopComponent
        {
            auto const SELECT_CASE_var(CompType_Num);

            ...

            } else if (SELECT_CASE_var == DXSystem) { // CoilSystem:Cooling:DX'
                SimDXCoolingSystem(state, CompName, FirstHVACIteration,
                                   AirLoopNum, CompIndex, _, _, QActual);
                if (QActual > 0.0) CoolingActive = true; // determine if coil is ON

            } else if (SELECT_CASE_var == DXHeatPumpSystem) {// CoilSystem:Heating:DX
                SimDXHeatPumpSystem(state, CompName, FirstHVACIteration, AirLoopNum,
                                    CompIndex, _, _, QActual);
                if (QActual > 0.0) HeatingActive = true; // determine if coil is ON

            } else if (SELECT_CASE_var == CoilUserDefined) { // Coil:UserDefined
                SimCoilUserDefined(state, CompName, CompIndex, AirLoopNum,
                                   HeatingActive, CoolingActive);

            } else if (SELECT_CASE_var == UnitarySystemModel) {
                // 'AirLoopHVAC:UnitarySystem'
                Real64 sensOut = 0.0;
                Real64 latOut = 0.0;
                CompPointer->simulate(state,
                                      CompName,
                                      FirstHVACIteration,
                                      AirLoopNum,
                                      CompIndex,
                                      HeatingActive,
                                      CoolingActive,
                                      OAUnitNum,
                                      OAUCoilOutTemp,
                                      ZoneEquipFlag,
                                      sensOut,
                                      latOut);

            **New simulate call for CoilWaterSystem using override for HVACSystemData::simulate:**

            else if (SELECT_CASE_var == CoilSystemWater) { // 'CoilSystem:Cooling:Water'

				if (CompPointer == nullptr) {
					UnitarySystems::UnitarySys thisSys;
					CompPointer = thisSys.factory(state, DataHVACGlobals::UnitarySys_AnyCoilType, CompName, false, 0);
					// temporary fix for saving pointer, eventually apply to UnitarySystem 25 lines down
					state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Branch(branchNum).Comp(compNum).compPointer = CompPointer;
				}
                Real64 sensOut = 0.0;
                Real64 latOut = 0.0;
                CompPointer->simulate(state,
                                      CompName,
                                      FirstHVACIteration,
                                      AirLoopNum,
                                      CompIndex,
                                      HeatingActive,
                                      CoolingActive,
                                      OAUnitNum,
                                      OAUCoilOutTemp,
                                      ZoneEquipFlag,
                                      sensOut,
                                      latOut);


Add component type for 'CoilSystem:Cooling:Water' in Primary Air Loop
    constexpr int Fan_System_Object(28);
    constexpr int UnitarySystemModel(29);
    constexpr int ZoneVRFasAirLoopEquip(30);

    Add component type here
    constexpr int CoilWaterSystems(31);


### DataPlant.cc ###
    Add "CoilSystem:Cooling:Water" system cooling coil to the 1D String lists:

	SimPlantEquipTypes
	ccSimPlantEquipTypes
	ValidLoopEquipTypes

	Change NumSimPlantEquipTypes(96) to NumSimPlantEquipTypes(97);

	Add parameter index for "CoilSystem:Cooling:Water" component
    constexpr int TypeOf_CoilSystemCoolingWater(97);

    Set loop type to "Condenser" for now
	LoopType::Condenser


    for (CompNum = 1; CompNum <= state.dataPlnt->PlantLoop(LoopNum).LoopSide(LoopSideNum)
	                                  .Branch(BranchNum).TotalComponents; ++CompNum) {
					// set up some references
					auto &this_comp_type(CompTypes(CompNum));
					auto &this_comp(state.dataPlnt->PlantLoop(LoopNum)
						  .LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum));
					...

					} else if (UtilityRoutines::SameString(this_comp_type,
					    "Coil:Heating:Steam")) {
						this_comp.TypeOf_Num = TypeOf_CoilSteamAirHeating;
						this_comp.CurOpSchemeType = DemandOpSchemeType;

                    **New demand side condenser equipment "CoilSystem:Cooling:Water"**

					} else if (UtilityRoutines::SameString(this_comp_type,
					    "CoilSystem:Cooling:Water")) {
						this_comp.TypeOf_Num = TypeOf_CoilSystemCoolingWater;
						this_comp.CurOpSchemeType = DemandOpSchemeType;
					...



### MixedAir.cc ###

Add similate calling point under SimOAComponent() function

    {
        auto const SELECT_CASE_var(CompTypeNum);

        if (SELECT_CASE_var == OAMixer_Num) { // 'OutdoorAir:Mixer'
            if (Sim) {
                SimOAMixer(state, CompName, FirstHVACIteration, CompIndex);
            }

        ...

        } else if (SELECT_CASE_var == UnitarySystemModel) { // AirLoopHVAC:UnitarySystem
            if (Sim) {
            }
            if (state.dataMixedAir->MyOneTimeCheckUnitarySysFlag(OASysNum)) {
            }

		**New simulate call for CoilWaterSystem**
		} else if (SELECT_CASE_var == CoilWaterSystem) { // // 'CoilSystem:Cooling:Water'

                if (state.dataAirLoop->OutsideAirSys(OASysNum).compPointer[CompIndex] == nullptr) {
                    UnitarySystems::UnitarySys thisSys;
                    state.dataAirLoop->OutsideAirSys(OASysNum).compPointer[CompIndex] =
                        thisSys.factory(state, DataHVACGlobals::UnitarySys_AnyCoilType, CompName, false, 0);
                    UnitarySystems::UnitarySys::checkUnitarySysCoilInOASysExists(state, CompName, 0);
                }

	            if (Sim) {
                bool HeatingActive = false;
                bool CoolingActive = false;
                Real64 OAUCoilOutTemp = 0.0;
                bool ZoneEquipFlag = false;
                Real64 sensOut = 0.0;
                Real64 latOut = 0.0;
                state.dataAirLoop->OutsideAirSys(OASysNum).compPointer[CompIndex]->simulate(state,
                                                                                            CompName,
                                                                                            FirstHVACIteration,
                                                                                            AirLoopNum,
                                                                                            CompIndex,
                                                                                            HeatingActive,
                                                                                            CoolingActive,
                                                                                            CompIndex,
                                                                                            OAUCoilOutTemp,
                                                                                            ZoneEquipFlag,
                                                                                            sensOut,
                                                                                            latOut);
            }

        } else if (SELECT_CASE_var == DXHeatPumpSystem) {
            if (Sim) {
                SimDXHeatPumpSystem(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
            }
            OAHeatingCoil = true;
        }

		...

    }

Add component type for 'CoilSystem:Cooling:Water' in Mixed Air
    constexpr int Fan_System_Object(22);
    constexpr int UnitarySystemModel(23);
    constexpr int VRFTerminalUnit(24);

    Add component type here
    constexpr int CoilWaterSystems(25);
