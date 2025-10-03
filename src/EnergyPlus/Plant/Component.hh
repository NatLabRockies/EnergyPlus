// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef PlantTopologyComponent_hh_INCLUDED
#define PlantTopologyComponent_hh_INCLUDED

#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/EquipAndOperations.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataPlant {

    static constexpr std::array<bool, static_cast<int>(PlantEquipmentType::Num)> PlantEquipmentTypeIsPump = {
        false, //	"Boiler:HotWater"
        false, //	"Boiler:Steam"
        false, //	"Chiller:Absorption"
        false, //	"Chiller:Absorption:Indirect"
        false, //	"Chiller:CombustionTurbine"
        false, //	"Chiller:ConstantCOP"
        false, //	"ChillerHeater:Absorption:DirectFired"
        false, //	"Chiller:Electric"
        false, //	"Chiller:Electric:EIR"
        false, //	"Chiller:Electric:ReformulatedEIR"
        false, //	"Chiller:Electric:ASHRAE205"
        false, //	"Chiller:EngineDriven"
        false, //	"CoolingTower:SingleSpeed"
        false, //	"CoolingTower:TwoSpeed"
        false, //	"CoolingTower:VariableSpeed"
        false, //	"Generator:Fuelcell:ExhaustGastoWaterHeatExchanger"
        false, //	"WaterHeater:HeatPump:PumpedCondenser"
        false, //	"Heatpump:WatertoWater:Equationfit:Cooling"
        false, //	"Heatpump:WatertoWater:Equationfit:Heating"
        false, //	"Heatpump:WatertoWater:ParameterEstimation:Cooling"
        false, //	"Heatpump:WatertoWater:ParameterEstimation:Heating"
        false, //	"Pipe:Adiabatic"
        false, //	"Pipe:Adiabatic:Steam"
        false, //	"Pipe:Outdoor"
        false, //	"Pipe:Indoor"
        false, //	"Pipe:Underground"
        false, //	"DistrictCooling"
        false, //	"DistrictHeating:Water" (steam is at the end)
        false, //	"ThermalStorage:Ice:Detailed"
        false, //	"ThermalStorage:Ice:Simple"
        false, //	"TemperingValve"
        false, //	"WaterHeater:Mixed"
        false, //	"WaterHeater:Stratified"
        true,  //	"Pump:VariableSpeed"
        true,  //	"Pump:ConstantSpeed"
        true,  //	"Pump:VariableSpeed:Condensate"
        true,  //	"HeaderedPumps:VariableSpeed"
        true,  //	"HeaderedPumps:ConstantSpeed"
        false, //	"WaterUse:Connections"
        false, //	"Coil:Cooling:Water"
        false, //	"Coil:Cooling:Water:DetailedGeometry"
        false, //	"Coil:Heating:Water"
        false, //	"Coil:Heating:Steam"
        false, //	"Solarcollector:Flatplate:Water"
        false, //	"LoadProfile:Plant"
        false, //	"GroundHeatExchanger:System"
        false, //	"GroundHeatExchanger:Surface"
        false, //	"GroundHeatExchanger:Pond"
        false, //	"Generator:Microturbine"
        false, //	"Generator:InternalCombustionEngine"
        false, //	"Generator:CombustionTurbine"
        false, //	"Generator:Microchp"
        false, //	"Generator:Fuelcell:StackCooler"
        false, //	"FluidCooler:SingleSpeed"
        false, //	"FluidCooler:TwoSpeed"
        false, //	"EvaporativeFluidCooler:SingleSpeed"
        false, //	"EvaporativeFluidCooler:TwoSpeed"
        false, //	"ThermalStorage:ChilledWater:Mixed"
        false, //	"ThermalStorage:ChilledWater:Stratified"
        false, //	"SolarCollector:FlatPlate:PhotovoltaicThermal"
        false, //	"ZoneHVAC:Baseboard:Convective:Water"
        false, //	"ZoneHVAC:Baseboard:RadiantConvective:Steam"
        false, //	"ZoneHVAC:Baseboard:RadiantConvective:Water"
        false, //	"ZoneHVAC:LowTemperatureRadiant:VariableFlow"
        false, //	"ZoneHVAC:LowTemperatureRadiant:ConstantFlow"
        false, //	"AirTerminal:SingleDuct:ConstantVolume:CooledBeam"
        false, //	"Coil:Heating:WaterToAirHeatPump:EquationFit"
        false, //	"Coil:Cooling:WaterToAirHeatPump:EquationFit"
        false, //	"Coil:Heating:WaterToAirHeatPump:ParameterEstimation"
        false, //	"Coil:Cooling:WaterToAirHeatPump:ParameterEstimation"
        false, //	"Refrigeration:Condenser:WaterCooled"
        false, //	"Refrigeration:CompressorRack"
        false, //	"AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed"
        false, //	"ChillerHeater:Absorption:DoubleEffect"
        false, //	"PipingSystem:Underground:PipeCircuit"
        false, //	"SolarCollector:IntegralCollectorStorage"
        false, //	"Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit"
        false, //	"Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit"
        false, //	"PlantComponent:UserDefined"
        false, //	"Coil:UserDefined"
        false, //	"ZoneHVAC:ForcedAir:UserDefined"
        false, //	"AirTerminal:SingleDuct:UserDefined"
        false, //	"AirConditioner:VariableRefrigerantFlow"
        false, //	"GroundHeatExchanger:HorizontalTrench"
        false, //	"HeatExchanger:FluidToFluid"
        false, //	"PlantComponent:TemperatureSource"
        false, //	"CentralHeatPumpSystem"
        false, //	"AirLoopHVAC:UnitarySystem"
        false, //	"Coil:Cooling:DX:SingleSpeed:ThermalStorage"
        false, //	"CoolingTower:VariableSpeed:Merkel"
        false, //	"SwimmingPool:Indoor"
        false, //	"GroundHeatExchanger:Slinky"
        false, //	"WaterHeater:HeatPump:WrappedCondenser"
        false, //	"AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam"
        false, //	"ZoneHVAC:CoolingPanel:RadiantConvective:Water"
        false, //	"HeatPump:PlantLoop:EIR:Cooling"
        false, //	"HeatPump:PlantLoop:EIR:Heating"
        false  //   "DistrictHeating:Steam"
    };

    static constexpr std::array<DataPlant::CtrlType, static_cast<int>(PlantEquipmentType::Num)> PlantEquipmentCtrlType = {
        DataPlant::CtrlType::HeatingOp, //	"Boiler:HotWater"
        DataPlant::CtrlType::HeatingOp, //	"Boiler:Steam"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Absorption"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Absorption:Indirect"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:CombustionTurbine"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:ConstantCOP"
        DataPlant::CtrlType::DualOp,    //	"ChillerHeater:Absorption:DirectFired"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Electric"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Electric:EIR"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Electric:ReformulatedEIR"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:Electric:ASHRAE205"
        DataPlant::CtrlType::CoolingOp, //	"Chiller:EngineDriven"
        DataPlant::CtrlType::CoolingOp, //	"CoolingTower:SingleSpeed"
        DataPlant::CtrlType::CoolingOp, //	"CoolingTower:TwoSpeed"
        DataPlant::CtrlType::CoolingOp, //	"CoolingTower:VariableSpeed"
        DataPlant::CtrlType::HeatingOp, //	"Generator:Fuelcell:ExhaustGastoWaterHeatExchanger"
        DataPlant::CtrlType::HeatingOp, //	"WaterHeater:HeatPump:PumpedCondenser"
        DataPlant::CtrlType::CoolingOp, //	"Heatpump:WatertoWater:Equationfit:Cooling"
        DataPlant::CtrlType::HeatingOp, //	"Heatpump:WatertoWater:Equationfit:Heating"
        DataPlant::CtrlType::CoolingOp, //	"Heatpump:WatertoWater:ParameterEstimation:Cooling"
        DataPlant::CtrlType::HeatingOp, //	"Heatpump:WatertoWater:ParameterEstimation:Heating"
        DataPlant::CtrlType::Invalid,   //	"Pipe:Adiabatic"
        DataPlant::CtrlType::Invalid,   //	"Pipe:Adiabatic:Steam"
        DataPlant::CtrlType::Invalid,   //	"Pipe:Outdoor"
        DataPlant::CtrlType::Invalid,   //	"Pipe:Indoor"
        DataPlant::CtrlType::Invalid,   //	"Pipe:Underground"
        DataPlant::CtrlType::CoolingOp, //	"DistrictCooling"
        DataPlant::CtrlType::HeatingOp, //	"DistrictHeating:Water" (steam is at the end)
        DataPlant::CtrlType::CoolingOp, //	"ThermalStorage:Ice:Detailed"
        DataPlant::CtrlType::CoolingOp, //	"ThermalStorage:Ice:Simple"
        DataPlant::CtrlType::Invalid,   //	"TemperingValve"
        DataPlant::CtrlType::HeatingOp, //	"WaterHeater:Mixed"
        DataPlant::CtrlType::HeatingOp, //	"WaterHeater:Stratified"
        DataPlant::CtrlType::Invalid,   //	"Pump:VariableSpeed"
        DataPlant::CtrlType::Invalid,   //	"Pump:ConstantSpeed"
        DataPlant::CtrlType::Invalid,   //	"Pump:VariableSpeed:Condensate"
        DataPlant::CtrlType::Invalid,   //	"HeaderedPumps:VariableSpeed"
        DataPlant::CtrlType::Invalid,   //	"HeaderedPumps:ConstantSpeed"
        DataPlant::CtrlType::Invalid,   //	"WaterUse:Connections"
        DataPlant::CtrlType::Invalid,   //	"Coil:Cooling:Water"
        DataPlant::CtrlType::Invalid,   //	"Coil:Cooling:Water:DetailedGeometry"
        DataPlant::CtrlType::Invalid,   //	"Coil:Heating:Water"
        DataPlant::CtrlType::Invalid,   //	"Coil:Heating:Steam"
        DataPlant::CtrlType::HeatingOp, //	"Solarcollector:Flatplate:Water"
        DataPlant::CtrlType::DualOp,    //	"LoadProfile:Plant"
        DataPlant::CtrlType::DualOp,    //	"GroundHeatExchanger:System"
        DataPlant::CtrlType::DualOp,    //	"GroundHeatExchanger:Surface"
        DataPlant::CtrlType::DualOp,    //	"GroundHeatExchanger:Pond"
        DataPlant::CtrlType::HeatingOp, //	"Generator:Microturbine"
        DataPlant::CtrlType::HeatingOp, //	"Generator:InternalCombustionEngine"
        DataPlant::CtrlType::HeatingOp, //	"Generator:CombustionTurbine"
        DataPlant::CtrlType::HeatingOp, //	"Generator:Microchp"
        DataPlant::CtrlType::HeatingOp, //	"Generator:Fuelcell:StackCooler"
        DataPlant::CtrlType::CoolingOp, //	"FluidCooler:SingleSpeed"
        DataPlant::CtrlType::CoolingOp, //	"FluidCooler:TwoSpeed"
        DataPlant::CtrlType::CoolingOp, //	"EvaporativeFluidCooler:SingleSpeed"
        DataPlant::CtrlType::CoolingOp, //	"EvaporativeFluidCooler:TwoSpeed"
        DataPlant::CtrlType::CoolingOp, //	"ThermalStorage:ChilledWater:Mixed"
        DataPlant::CtrlType::CoolingOp, //	"ThermalStorage:ChilledWater:Stratified"
        DataPlant::CtrlType::HeatingOp, //	"SolarCollector:FlatPlate:PhotovoltaicThermal"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:Baseboard:Convective:Water"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:Baseboard:RadiantConvective:Steam"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:Baseboard:RadiantConvective:Water"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:LowTemperatureRadiant:VariableFlow"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:LowTemperatureRadiant:ConstantFlow"
        DataPlant::CtrlType::Invalid,   //	"AirTerminal:SingleDuct:ConstantVolume:CooledBeam"
        DataPlant::CtrlType::Invalid,   //	"Coil:Heating:WaterToAirHeatPump:EquationFit"
        DataPlant::CtrlType::Invalid,   //	"Coil:Cooling:WaterToAirHeatPump:EquationFit"
        DataPlant::CtrlType::Invalid,   //	"Coil:Heating:WaterToAirHeatPump:ParameterEstimation"
        DataPlant::CtrlType::Invalid,   //	"Coil:Cooling:WaterToAirHeatPump:ParameterEstimation"
        DataPlant::CtrlType::HeatingOp, //	"Refrigeration:Condenser:WaterCooled"
        DataPlant::CtrlType::Invalid,   //	"Refrigeration:CompressorRack"
        DataPlant::CtrlType::Invalid,   //	"AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed"
        DataPlant::CtrlType::DualOp,    //	"ChillerHeater:Absorption:DoubleEffect"
        DataPlant::CtrlType::Invalid,   //	"PipingSystem:Underground:PipeCircuit"
        DataPlant::CtrlType::HeatingOp, //	"SolarCollector:IntegralCollectorStorage"
        DataPlant::CtrlType::Invalid,   //	"Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit"
        DataPlant::CtrlType::Invalid,   //	"Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit"
        DataPlant::CtrlType::DualOp,    //	"PlantComponent:UserDefined"
        DataPlant::CtrlType::Invalid,   //	"Coil:UserDefined"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:ForcedAir:UserDefined"
        DataPlant::CtrlType::Invalid,   //	"AirTerminal:SingleDuct:UserDefined"
        DataPlant::CtrlType::Invalid,   //	"AirConditioner:VariableRefrigerantFlow"
        DataPlant::CtrlType::DualOp,    //	"GroundHeatExchanger:HorizontalTrench"
        DataPlant::CtrlType::DualOp,    //	"HeatExchanger:FluidToFluid"
        DataPlant::CtrlType::DualOp,    //	"PlantComponent:TemperatureSource"
        DataPlant::CtrlType::DualOp,    //	"CentralHeatPumpSystem"
        DataPlant::CtrlType::Invalid,   //	"AirLoopHVAC:UnitarySystem"
        DataPlant::CtrlType::HeatingOp, //	"Coil:Cooling:DX:SingleSpeed:ThermalStorage"
        DataPlant::CtrlType::CoolingOp, //	"CoolingTower:VariableSpeed:Merkel"
        DataPlant::CtrlType::Invalid,   //	"SwimmingPool:Indoor"
        DataPlant::CtrlType::DualOp,    //	"GroundHeatExchanger:Slinky"
        DataPlant::CtrlType::HeatingOp, //	"WaterHeater:HeatPump:WrappedCondenser"
        DataPlant::CtrlType::Invalid,   //	"AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam"
        DataPlant::CtrlType::Invalid,   //	"ZoneHVAC:CoolingPanel:RadiantConvective:Water"
        DataPlant::CtrlType::CoolingOp, //	"HeatPump:PlantLoop:EIR:Cooling"
        DataPlant::CtrlType::HeatingOp, //	"HeatPump:PlantLoop:EIR:Heating"
        DataPlant::CtrlType::HeatingOp  //   "DistrictHeating:Steam"
    };

    struct CompData
    {
        // Members
        std::string TypeOf;                           // The 'keyWord' identifying  component type
        DataPlant::PlantEquipmentType Type;           // Reference the "TypeOf" parameters in DataPlant
        std::string Name;                             // Component name
        int CompNum;                                  // Component ID number
        DataBranchAirLoopPlant::ControlType FlowCtrl; // flow control for splitter/mixer (ACTIVE/PASSIVE/BYPASS)
        LoopFlowStatus FlowPriority;                  // status for overall loop flow determination
        bool ON;                                      // TRUE = designated component or operation scheme available
        bool Available;                               // TRUE = designated component or operation scheme available
        std::string NodeNameIn;                       // Component inlet node name
        std::string NodeNameOut;                      // Component outlet node name
        int NodeNumIn;                                // Component inlet node number
        int NodeNumOut;                               // Component outlet node number
        Real64 MyLoad;                                // Distributed Load
        Real64 MaxLoad;                               // Maximum load
        Real64 MinLoad;                               // Minimum Load
        Real64 OptLoad;                               // Optimal Load
        Real64 SizFac;                                // Sizing Fraction
        DataPlant::OpScheme CurOpSchemeType;          // updated pointer to
        // Plant()%OpScheme(CurOpSchemeType)...
        int NumOpSchemes;      // number of schemes held in the pointer array
        int CurCompLevelOpNum; // pointer to the OpScheme array defined next
        // PlantLoop()%LoopSide()%Branch()%Comp()%OpScheme(curOpSchemePtr)
        Array1D<OpSchemePtrData> OpScheme;                // Pointers to component on lists
        Real64 EquipDemand;                               // Component load request based on inlet temp and outlet SP
        bool EMSLoadOverrideOn;                           // EMS is calling to override load dispatched to component
        Real64 EMSLoadOverrideValue;                      // EMS value to use for load when overridden [W] always positive.
        DataPlant::HowMet HowLoadServed;                  // nature of component in terms of how it can meet load
        Real64 MinOutletTemp;                             // Component exit lower limit temperature
        Real64 MaxOutletTemp;                             // Component exit upper limit temperature
        bool FreeCoolCntrlShutDown;                       // true if component was shut down because of free cooling
        Real64 FreeCoolCntrlMinCntrlTemp;                 // current control temp value for free cooling controls
        DataPlant::FreeCoolControlMode FreeCoolCntrlMode; // type of sensor used for free cooling controls
        int FreeCoolCntrlNodeNum;                         // chiller condenser inlet node number for free cooling controls
        int IndexInLoopSidePumps;                         // If I'm a pump, this tells my index in PL(:)%LS(:)%Pumps
        Real64 TempDesCondIn;
        Real64 TempDesEvapOut;
        PlantComponent *compPtr;
        EnergyPlus::PlantLocation location;

        // Default Constructor
        CompData()
            : Type(DataPlant::PlantEquipmentType::Invalid), CompNum(0), FlowCtrl(DataBranchAirLoopPlant::ControlType::Invalid),
              FlowPriority(LoopFlowStatus::Invalid), ON(false), Available(false), NodeNumIn(0), NodeNumOut(0), MyLoad(0.0), MaxLoad(0.0),
              MinLoad(0.0), OptLoad(0.0), SizFac(0.0), CurOpSchemeType(DataPlant::OpScheme::Invalid), NumOpSchemes(0), CurCompLevelOpNum(0),
              EquipDemand(0.0), EMSLoadOverrideOn(false), EMSLoadOverrideValue(0.0), HowLoadServed(DataPlant::HowMet::Invalid), MinOutletTemp(0.0),
              MaxOutletTemp(0.0), FreeCoolCntrlShutDown(false), FreeCoolCntrlMinCntrlTemp(0.0),
              FreeCoolCntrlMode(DataPlant::FreeCoolControlMode::Invalid), FreeCoolCntrlNodeNum(0), IndexInLoopSidePumps(0), TempDesCondIn(0.0),
              TempDesEvapOut(0.0), compPtr(nullptr)
        {
        }

        void initLoopEquip(EnergyPlusData &state, bool const GetCompSizFac);
        //                This function is called before the main simulation calls for component models when the plant sizing is being conducted. The
        //                component model needs to be set up such that when this function is called, the getInput, initialization and sizing routines
        //                are run, but the calculation routine does not. When called, most supply side component models need to return values for the
        //                minimum, maximum, and optimal capacities (in terms of loop loads that the device can meet). For plant components with more
        //                than one connection to a plant loop, a leading loop connection must be determined and the component sizing routine called
        //                with initLoopEquip is called for only that plant loop. For a example, a chiller only calls its sizing routine when called
        //                from a the chilled water loop and does not call it sizing routine when called from the condenser loop.

        void simulate(EnergyPlusData &state, bool FirstHVACIteration);

        void oneTimeInit(EnergyPlusData &state) const;

        static CompData &getPlantComponent(EnergyPlusData &state, PlantLocation const &plantLoc);

        Real64 getDynamicMaxCapacity(EnergyPlusData &state) const;
    };
} // namespace DataPlant
} // namespace EnergyPlus

#endif
