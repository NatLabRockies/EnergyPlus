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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/member.functions.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::BranchNodeConnections {

// Module containing the routines dealing with the Branch/Node Connections (CompSets, etc)

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   May 2005

// PURPOSE OF THIS MODULE:
// This module encapsulates the connection data necessary for some of the checks
// needed in the branch-node data

// Using/Aliasing
using namespace DataLoopNode;
using namespace DataBranchNodeConnections;

static constexpr std::string_view undefined("UNDEFINED");

constexpr std::array<std::string_view, static_cast<int>(DataLoopNode::ConnectionObjectType::Num)> ConnectionObjectTypeNames = {
    "Undefined",
    "AirConditioner:VariableRefrigerantFlow",
    "AirLoopHVAC",
    "AirLoopHVAC:DedicatedOutdoorAirSystem",
    "AirLoopHVAC:ExhaustSystem",
    "AirLoopHVAC:Mixer",
    "AirLoopHVAC:OutdoorAirSystem",
    "AirLoopHVAC:ReturnPath",
    "AirLoopHVAC:ReturnPlenum",
    "AirLoopHVAC:Splitter",
    "AirLoopHVAC:SupplyPath",
    "AirLoopHVAC:SupplyPlenum",
    "AirLoopHVAC:Unitary:Furnace:HeatCool",
    "AirLoopHVAC:Unitary:Furnace:HeatOnly",
    "AirLoopHVAC:UnitaryHeatCool",
    "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass",
    "AirLoopHVAC:UnitaryHeatOnly",
    "AirLoopHVAC:UnitaryHeatPump:AirToAir",
    "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed",
    "AirLoopHVAC:UnitaryHeatPump:WaterToAir",
    "AirLoopHVAC:UnitarySystem",
    "AirLoopHVAC:ZoneMixer",
    "AirLoopHVAC:ZoneSplitter",
    "AirTerminal:DualDuct:ConstantVolume",
    "AirTerminal:DualDuct:ConstantVolume:Cool",
    "AirTerminal:DualDuct:ConstantVolume:Heat",
    "AirTerminal:DualDuct:VAV",
    "AirTerminal:DualDuct:VAV:Cool",
    "AirTerminal:DualDuct:VAV:Heat",
    "AirTerminal:DualDuct:VAV:OutdoorAir",
    "AirTerminal:DualDuct:VAV:OutdoorAir:OutdoorAir",
    "AirTerminal:DualDuct:VAV:OutdoorAir:RecirculatedAir",
    "AirTerminal:SingleDuct:ConstantVolume:CooledBeam",
    "AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam",
    "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction",
    "AirTerminal:SingleDuct:ConstantVolume:NoReheat",
    "AirTerminal:SingleDuct:ConstantVolume:Reheat",
    "AirTerminal:SingleDuct:Mixer",
    "AirTerminal:SingleDuct:ParallelPIU:Reheat",
    "AirTerminal:SingleDuct:SeriesPIU:Reheat",
    "AirTerminal:SingleDuct:UserDefined",
    "AirTerminal:SingleDuct:VAV:HeatAndCool:NoReheat",
    "AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat",
    "AirTerminal:SingleDuct:VAV:NoReheat",
    "AirTerminal:SingleDuct:VAV:Reheat",
    "AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan",
    "AvailabilityManager:DifferentialThermostat",
    "AvailabilityManager:HighTemperatureTurnOff",
    "AvailabilityManager:HighTemperatureTurnOn",
    "AvailabilityManager:LowTemperatureTurnOff",
    "AvailabilityManager:LowTemperatureTurnOn",
    "Boiler:HotWater",
    "Boiler:Steam",
    "Branch",
    "CentralHeatPumpSystem",
    "Chiller:Absorption",
    "Chiller:Absorption:Indirect",
    "Chiller:CombustionTurbine",
    "Chiller:ConstantCOP",
    "Chiller:Electric",
    "Chiller:Electric:EIR",
    "Chiller:Electric:ReformulatedEIR",
    "Chiller:Electric:ASHRAE205",
    "Chiller:EngineDriven",
    "ChillerHeater:Absorption:DirectFired",
    "ChillerHeater:Absorption:DoubleEffect",
    "Coil:Cooling:DX",
    "Coil:Cooling:DX:CurveFit:Speed",
    "Coil:Cooling:DX:MultiSpeed",
    "Coil:Cooling:DX:SingleSpeed",
    "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
    "Coil:Cooling:DX:SubcoolReheat",
    "Coil:Cooling:DX:TwoSpeed",
    "Coil:Cooling:DX:TwoStageWithHumidityControlMode",
    "Coil:Cooling:DX:VariableRefrigerantFlow",
    "Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl",
    "Coil:Cooling:DX:VariableSpeed",
    "Coil:Cooling:Water",
    "Coil:Cooling:Water:DetailedGeometry",
    "Coil:Cooling:WaterToAirHeatPump:EquationFit",
    "Coil:Cooling:WaterToAirHeatPump:ParameterEstimation",
    "Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit",
    "Coil:Heating:DX:MultiSpeed",
    "Coil:Heating:DX:SingleSpeed",
    "Coil:Heating:DX:VariableRefrigerantFlow",
    "Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl",
    "Coil:Heating:DX:VariableSpeed",
    "Coil:Heating:Desuperheater",
    "Coil:Heating:Electric",
    "Coil:Heating:Electric:MultiStage",
    "Coil:Heating:Fuel",
    "Coil:Heating:Gas:MultiStage",
    "Coil:Heating:Steam",
    "Coil:Heating:Water",
    "Coil:Heating:WaterToAirHeatPump:EquationFit",
    "Coil:Heating:WaterToAirHeatPump:ParameterEstimation",
    "Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit",
    "Coil:UserDefined",
    "Coil:WaterHeating:AirToWaterHeatPump:Pumped",
    "Coil:WaterHeating:AirToWaterHeatPump:VariableSpeed",
    "Coil:WaterHeating:AirToWaterHeatPump:Wrapped",
    "Coil:WaterHeating:Desuperheater",
    "CoilSystem:Cooling:DX",
    "CoilSystem:Cooling:DX:HeatExchangerAssisted",
    "CoilSystem:Cooling:Water",
    "CoilSystem:Cooling:Water:HeatExchangerAssisted",
    "CoilSystem:Heating:DX",
    "CoilSystem:IntegratedHeatPump:AirSource",
    "Condenser",
    "CondenserLoop",
    "Connector:Mixer",
    "Connector:Splitter",
    "Controller:OutdoorAir",
    "Controller:WaterCoil",
    "CoolingTower:SingleSpeed",
    "CoolingTower:TwoSpeed",
    "CoolingTower:VariableSpeed",
    "CoolingTower:VariableSpeed:Merkel",
    "Dehumidifier:Desiccant:NoFans",
    "Dehumidifier:Desiccant:System",
    "DistrictCooling",
    "DistrictHeating:Water",
    "DistrictHeating:Steam",
    "Duct",
    "ElectricEquipment:ITE:AirCooled",
    "EvaporativeCooler:Direct:CelDekPad",
    "EvaporativeCooler:Direct:ResearchSpecial",
    "EvaporativeCooler:Indirect:CelDekPad",
    "EvaporativeCooler:Indirect:ResearchSpecial",
    "EvaporativeCooler:Indirect:WetCoil",
    "EvaporativeFluidCooler:SingleSpeed",
    "EvaporativeFluidCooler:TwoSpeed",
    "Fan:ComponentModel",
    "Fan:ConstantVolume",
    "Fan:OnOff",
    "Fan:SystemModel",
    "Fan:VariableVolume",
    "Fan:ZoneExhaust",
    "FluidCooler:SingleSpeed",
    "FluidCooler:TwoSpeed",
    "Generator:CombustionTurbine",
    "Generator:FuelCell:AirSupply",
    "Generator:FuelCell:ExhaustGasToWaterHeatExchanger",
    "Generator:FuelCell:PowerModule",
    "Generator:FuelCell:StackCooler",
    "Generator:FuelCell:WaterSupply",
    "Generator:FuelSupply",
    "Generator:InternalCombustionEngine",
    "Generator:MicroCHP",
    "Generator:MicroTurbine",
    "GroundHeatExchanger:HorizontalTrench",
    "GroundHeatExchanger:Pond",
    "GroundHeatExchanger:Slinky",
    "GroundHeatExchanger:Surface",
    "GroundHeatExchanger:System",
    "HeaderedPumps:ConstantSpeed",
    "HeaderedPumps:VariableSpeed",
    "HeatExchanger:AirToAir:FlatPlate",
    "HeatExchanger:AirToAir:SensibleAndLatent",
    "HeatExchanger:Desiccant:BalancedFlow",
    "HeatExchanger:FluidToFluid",
    "HeatPump:AirToWater:FuelFired:Cooling",
    "HeatPump:AirToWater:FuelFired:Heating",
    "HeatPump:PlantLoop:EIR:Cooling",
    "HeatPump:PlantLoop:EIR:Heating",
    "HeatPump:WaterToWater:EquationFit:Cooling",
    "HeatPump:WaterToWater:EquationFit:Heating",
    "HeatPump:WaterToWater:ParameterEstimation:Cooling",
    "HeatPump:WaterToWater:ParameterEstimation:Heating",
    "Humidifier:Steam:Electric",
    "Humidifier:Steam:Gas",
    "Lights",
    "LoadProfile:Plant",
    "OutdoorAir:Mixer",
    "OutdoorAir:Node",
    "OutdoorAir:NodeList",
    "Pipe:Adiabatic",
    "Pipe:Adiabatic:Steam",
    "Pipe:Indoor",
    "Pipe:Outdoor",
    "Pipe:Underground",
    "PipingSystem:Underground:PipeCircuit",
    "PlantComponent:TemperatureSource",
    "PlantComponent:UserDefined",
    "PlantEquipmentOperation:ChillerHeaterChangeover",
    "PlantEquipmentOperation:ComponentSetpoint",
    "PlantEquipmentOperation:OutdoorDewpointDifference",
    "PlantEquipmentOperation:OutdoorDrybulbDifference",
    "PlantEquipmentOperation:OutdoorWetbulbDifference",
    "PlantEquipmentOperation:ThermalEnergyStorage",
    "PlantLoop",
    "Pump:ConstantSpeed",
    "Pump:ConstantVolume",
    "Pump:VariableSpeed",
    "Pump:VariableSpeed:Condensate",
    "Refrigeration:CompressorRack",
    "Refrigeration:Condenser:AirCooled",
    "Refrigeration:Condenser:EvaporativeCooled",
    "Refrigeration:Condenser:WaterCooled",
    "Refrigeration:GasCooler:AirCooled",
    "SetpointManager:Coldest",
    "SetpointManager:CondenserEnteringReset",
    "SetpointManager:CondenserEnteringReset:Ideal",
    "SetpointManager:FollowGroundTemperature",
    "SetpointManager:FollowOutdoorAirTemperature",
    "SetpointManager:FollowSystemNodeTemperature",
    "SetpointManager:MixedAir",
    "SetpointManager:MultiZone:Cooling:Average",
    "SetpointManager:MultiZone:Heating:Average",
    "SetpointManager:MultiZone:Humidity:Maximum",
    "SetpointManager:MultiZone:Humidity:Minimum",
    "SetpointManager:MultiZone:MaximumHumidity:Average",
    "SetpointManager:MultiZone:MinimumHumidity:Average",
    "SetpointManager:OutdoorAirPretreat",
    "SetpointManager:OutdoorAirReset",
    "SetpointManager:ReturnTemperature:ChilledWater",
    "SetpointManager:ReturnTemperature:HotWater",
    "SetpointManager:Scheduled",
    "SetpointManager:Scheduled:DualSetpoint",
    "SetpointManager:SingleZone:Cooling",
    "SetpointManager:SingleZone:Heating",
    "SetpointManager:SingleZone:Humidity:Maximum",
    "SetpointManager:SingleZone:Humidity:Minimum",
    "SetpointManager:SingleZone:OneStageCooling",
    "SetpointManager:SingleZone:OneStageHeating",
    "SetpointManager:SingleZone:Reheat",
    "SetpointManager:SystemNodeReset:Temperature",
    "SetpointManager:SystemNodeReset:Humidity",
    "SetpointManager:Warmest",
    "SetpointManager:WarmestTemperatureFlow",
    "SolarCollector:FlatPlate:PhotovoltaicThermal",
    "SolarCollector:FlatPlate:Water",
    "SolarCollector:IntegralCollectorStorage",
    "SolarCollector:UnglazedTranspired",
    "SurfaceProperty:LocalEnvironment",
    "SwimmingPool:Indoor",
    "TemperingValve",
    "ThermalStorage:ChilledWater:Mixed",
    "ThermalStorage:ChilledWater:Stratified",
    "ThermalStorage:Ice:Detailed",
    "ThermalStorage:Ice:Simple",
    "WaterHeater:HeatPump",
    "WaterHeater:HeatPump:PumpedCondenser",
    "WaterHeater:HeatPump:WrappedCondenser",
    "WaterHeater:Mixed",
    "WaterHeater:Stratified",
    "WaterUse:Connections",
    "ZoneHVAC:AirDistributionUnit",
    "ZoneHVAC:Baseboard:Convective:Electric",
    "ZoneHVAC:Baseboard:Convective:Water",
    "ZoneHVAC:Baseboard:RadiantConvective:Electric",
    "ZoneHVAC:Baseboard:RadiantConvective:Steam",
    "ZoneHVAC:Baseboard:RadiantConvective:Water",
    "ZoneHVAC:CoolingPanel:RadiantConvective:Water",
    "ZoneHVAC:Dehumidifier:DX",
    "ZoneHVAC:EnergyRecoveryVentilator",
    "ZoneHVAC:EquipmentConnections",
    "ZoneHVAC:EvaporativeCoolerUnit",
    "ZoneHVAC:ExhaustControl",
    "ZoneHVAC:ForcedAir:UserDefined",
    "ZoneHVAC:FourPipeFanCoil",
    "ZoneHVAC:HighTemperatureRadiant",
    "ZoneHVAC:HybridUnitaryHVAC",
    "ZoneHVAC:IdealLoadsAirSystem",
    "ZoneHVAC:LowTemperatureRadiant:ConstantFlow",
    "ZoneHVAC:LowTemperatureRadiant:VariableFlow",
    "ZoneHVAC:OutdoorAirUnit",
    "ZoneHVAC:PackagedTerminalAirConditioner",
    "ZoneHVAC:PackagedTerminalHeatPump",
    "ZoneHVAC:RefrigerationChillerSet",
    "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow",
    "ZoneHVAC:UnitHeater",
    "ZoneHVAC:UnitVentilator",
    "ZoneHVAC:VentilatedSlab",
    "ZoneHVAC:WaterToAirHeatPump",
    "ZoneHVAC:WindowAirConditioner",
    "ZoneProperty:LocalEnvironment",
    "SpaceHVAC:EquipmentConnections",
    "SpaceHVAC:ZoneEquipmentSplitter",
    "SpaceHVAC:ZoneEquipmentMixer"};

constexpr std::array<std::string_view, static_cast<int>(DataLoopNode::ConnectionObjectType::Num)> ConnectionObjectTypeNamesUC = {
    undefined,
    "AIRCONDITIONER:VARIABLEREFRIGERANTFLOW",
    "AIRLOOPHVAC",
    "AIRLOOPHVAC:DEDICATEDOUTDOORAIRSYSTEM",
    "AIRLOOPHVAC:EXHAUSTSYSTEM",
    "AIRLOOPHVAC:MIXER",
    "AIRLOOPHVAC:OUTDOORAIRSYSTEM",
    "AIRLOOPHVAC:RETURNPATH",
    "AIRLOOPHVAC:RETURNPLENUM",
    "AIALOOPHVAC:SPLITTER",
    "AIRLOOPHVAC:SUPPLYPATH",
    "AIRLOOPHVAC:SUPPLYPLENUM",
    "AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL",
    "AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY",
    "AIRLOOPHVAC:UNITARYHEATCOOL",
    "AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS",
    "AIRLOOPHVAC:UNITARYHEATONLY",
    "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR",
    "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED",
    "AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR",
    "AIRLOOPHVAC:UNITARYSYSTEM",
    "AIRLOOPHVAC:ZONEMIXER",
    "AIRLOOPHVAC:ZONESPLITTER",
    "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME",
    "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:COOL",
    "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:HEAT",
    "AIRTERMINAL:DUALDUCT:VAV",
    "AIRTERMINAL:DUALDUCT:VAV:COOL",
    "AIRTERMINAL:DUALDUCT:VAV:HEAT",
    "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR",
    "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:OUTDOORAIR",
    "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:RECIRCULATEDAIR",
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM",
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEBEAM",
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION",
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT",
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT",
    "AIRTERMINAL:SINGLEDUCT:MIXER",
    "AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",
    "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",
    "AIRTERMINAL:SINGLEDUCT:USERDEFINED",
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT",
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT",
    "AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT",
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT",
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN",
    "AVAILABILITYMANAGER:DIFFERENTIALTHERMOSTAT",
    "AVAILABILITYMANAGER:HIGHTEMPERATURETURNOFF",
    "AVAILABILITYMANAGER:HIGHTEMPERATURETURNON",
    "AVAILABILITYMANAGER:LOWTEMPERATURETURNOFF",
    "AVAILABILITYMANAGER:LOWTEMPERATURETURNON",
    "BOILER:HOTWATER",
    "BOILER:STEAM",
    "BRANCH",
    "CENTRALHEATPUMPSYSTEM",
    "CHILLER:ABSORPTION",
    "CHILLER:ABSORPTION:INDIRECT",
    "CHILLER:COMBUSTIONTURBINE",
    "CHILLER:CONSTANTCOP",
    "CHILLER:ELECTRIC",
    "CHILLER:ELECTRIC:EIR",
    "CHILLER:ELECTRIC:REFORMULATEDEIR",
    "CHILLER:ELECTRIC:ASHRAE205",
    "CHILLER:ENGINEDRIVEN",
    "CHILLERHEATER:ABSORPTION:DIRECTFIRED",
    "CHILLERHEATER:ABSORPTION:DOUBLEEFFECT",
    "COIL:COOLING:DX",
    "COIL:COOLING:DX:CURVEFIT:SPEED",
    "COIL:COOLING:DX:MULTISPEED",
    "COIL:COOLING:DX:SINGLESPEED",
    "COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE",
    "COIL:COOLING:DX:SUBCOOLREHEAT",
    "COIL:COOLING:DX:TWOSPEED",
    "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE",
    "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW",
    "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
    "COIL:COOLING:DX:VARIABLESPEED",
    "COIL:COOLING:WATER",
    "COIL:COOLING:WATER:DETAILEDGEOMETRY",
    "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT",
    "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
    "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
    "COIL:HEATING:DX:MULTISPEED",
    "COIL:HEATING:DX:SINGLESPEED",
    "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW",
    "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
    "COIL:HEATING:DX:VARIABLESPEED",
    "COIL:HEATING:DESUPERHEATER",
    "COIL:HEATING:ELECTRIC",
    "COIL:HEATING:ELECTRIC:MULTISTAGE",
    "COIL:HEATING:FUEL",
    "COIL:HEATING:GAS:MULTISTAGE",
    "COIL:HEATING:STEAM",
    "COIL:HEATING:WATER",
    "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT",
    "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
    "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
    "COIL:USERDEFINED",
    "COIL:WATERHEATING:AIRTOWATERHEATPUMP:PUMPED",
    "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED",
    "COIL:WATERHEATING:AIRTOWATERHEATPUMP:WRAPPED",
    "COIL:WATERHEATING:DESUPERHEATER",
    "COILSYSTEM:COOLING:DX",
    "COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED",
    "COILSYSTEM:COOLING:WATER",
    "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
    "COILSYSTEM:HEATING:DX",
    "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE",
    "CONDENSER",
    "CONDENSERLOOP",
    "CONNECTOR:MIXER",
    "CONNECTOR:SPLITTER",
    "CONTROLLER:OUTDOORAIR",
    "CONTROLLER:WATERCOIL",
    "COOLINGTOWER:SINGLESPEED",
    "COOLINGTOWER:TWOSPEED",
    "COOLINGTOWER:VARIABLESPEED",
    "COOLINGTOWER:VARIABLESPEED:MERKEL",
    "DEHUMIDIFIER:DESICCANT:NOFANS",
    "DEHUMIDIFIER:DESICCANT:SYSTEM",
    "DISTRICTCOOLING",
    "DISTRICTHEATING:WATER",
    "DISTRICTHEATING:STEAM",
    "DUCT",
    "ELECTRICEQUIPMENT:ITE:AIRCOOLED",
    "EVAPORATIVECOOLER:DIRECT:CELDEKPAD",
    "EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL",
    "EVAPORATIVECOOLER:INDIRECT:CELDEKPAD",
    "EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL",
    "EVAPORATIVECOOLER:INDIRECT:WETCOIL",
    "EVAPORATIVEFLUIDCOOLER:SINGLESPEED",
    "EVAPORATIVEFLUIDCOOLER:TWOSPEED",
    "FAN:COMPONENTMODEL",
    "FAN:CONSTANTVOLUME",
    "FAN:ONOFF",
    "FAN:SYSTEMMODEL",
    "FAN:VARIABLEVOLUME",
    "FAN:ZONEEXHAUST",
    "FLUIDCOOLER:SINGLESPEED",
    "FLUIDCOOLER:TWOSPEED",
    "GENERATOR:COMBUSTIONTURBINE",
    "GENERATOR:FUELCELL:AIRSUPPLY",
    "GENERATOR:FUELCELL:EXHAUSTGASTOWATERHEATEXCHANGER",
    "GENERATOR:FUELCELL:POWERMODULE",
    "GENERATOR:FUELCELL:STACKCOOLER",
    "GENERATOR:FUELCELL:WATERSUPPLY",
    "GENERATOR:FUELSUPPLY",
    "GENERATOR:INTERNALCOMBUSTIONENGINE",
    "GENERATOR:MICROCHP",
    "GENERATOR:MICROTURBINE",
    "GROUNDHEATEXCHANGER:HORIZONTALTRENCH",
    "GROUNDHEATEXCHANGER:POND",
    "GROUNDHEATEXCHANGER:SLINKY",
    "GROUNDHEATEXCHANGER:SURFACE",
    "GROUNDHEATEXCHANGER:SYSTEM",
    "HEADEREDPUMPS:CONSTANTSPEED",
    "HEADEREDPUMPS:VARIABLESPEED",
    "HEATEXCHANGER:AIRTOAIR:FLATPLATE",
    "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT",
    "HEATEXCHANGER:DESICCANT:BALANCEDFLOW",
    "HEATEXCHANGER:FLUIDTOFLUID",
    "HEATPUMP:AIRTOWATER:FUELFIRED:COOLING",
    "HEATPUMP:AIRTOWATER:FUELFIRED:HEATING",
    "HEATPUMP:PLANTLOOP:EIR:COOLING",
    "HEATPUMP:PLANTLOOP:EIR:HEATING",
    "HEATPUMP:WATERTOWATER:EQUATIONFIT:COOLING",
    "HEATPUMP:WATERTOWATER:EQUATIONFIT:HEATING",
    "HEATPUMP:WATERTOWATER:PARAMETERESTIMATION:COOLING",
    "HEATPUMP:WATERTOWATER:PARAMETERESTIMATION:HEATING",
    "HUMIDIFIER:STEAM:ELECTRIC",
    "HUMIDIFIER:STEAM:GAS",
    "LIGHTS",
    "LOADPROFILE:PLANT",
    "OUTDOORAIR:MIXER",
    "OUTDOORAIR:NODE",
    "OUTDOORAIR:NODELIST",
    "PIPE:ADIABATIC",
    "PIPE:ADIABATIC:STEAM",
    "PIPE:INDOOR",
    "PIPE:OUTDOOR",
    "PIPE:UNDERGROUND",
    "PIPINGSYSTEM:UNDERGROUND:PIPECIRCUIT",
    "PLANTCOMPONENT:TEMPERATURESOURCE",
    "PLANTCOMPONENT:USERDEFINED",
    "PLANTEQUIPMENTOPERATION:CHILLERHEATERCHANGEOVER",
    "PLANTEQUIPMENTOPERATION:COMPONENTSETPOINT",
    "PLANTEQUIPMENTOPERATION:OUTDOORDEWPOINTDIFFERENCE",
    "PLANTEQUIPMENTOPERATION:OUTDOORDRYBULBDIFFERENCE",
    "PLANTEQUIPMENTOPERATION:OUTDOORWETBULBDIFFERENCE",
    "PLANTEQUIPMENTOPERATION:THERMALENERGYSTORAGE",
    "PLANTLOOP",
    "PUMP:CONSTANTSPEED",
    "PUMP:CONSTANTVOLUME",
    "PUMP:VARIABLESPEED",
    "PUMP:VARIABLESPEED:CONDENSATE",
    "REFRIGERATION:COMPRESSORRACK",
    "REFRIGERATION:CONDENSER:AIRCOOLED",
    "REFRIGERATION:CONDENSER:EVAPORATIVECOOLED",
    "REFRIGERATION:CONDENSER:WATERCOOLED",
    "REFRIGERATION:GASCOOLER:AIRCOOLED",
    "SETPOINTMANAGER:COLDEST",
    "SETPOINTMANAGER:CONDENSERENTERINGRESET",
    "SETPOINTMANAGER:CONDENSERENTERINGRESET:IDEAL",
    "SETPOINTMANAGER:FOLLOWGROUNDTEMPERATURE",
    "SETPOINTMANAGER:FOLLOWOUTDOORAIRTEMPERATURE",
    "SETPOINTMANAGER:FOLLOWSYSTEMNODETEMPERATURE",
    "SETPOINTMANAGER:MIXEDAIR",
    "SETPOINTMANAGER:MULTIZONE:COOLING:AVERAGE",
    "SETPOINTMANAGER:MULTIZONE:HEATING:AVERAGE",
    "SETPOINTMANAGER:MULTIZONE:HUMIDITY:MAXIMUM",
    "SETPOINTMANAGER:MULTIZONE:HUMIDITY:MINIMUM",
    "SETPOINTMANAGER:MULTIZONE:MAXIMUMHUMIDITY:AVERAGE",
    "SETPOINTMANAGER:MULTIZONE:MINIMUMHUMIDITY:AVERAGE",
    "SETPOINTMANAGER:OUTDOORAIRPRETREAT",
    "SETPOINTMANAGER:OUTDOORAIRRESET",
    "SETPOINTMANAGER:RETURNTEMPERATURE:CHILLEDWATER",
    "SETPOINTMANAGER:RETURNTEMPERATURE:HOTWATER",
    "SETPOINTMANAGER:SCHEDULED",
    "SETPOINTMANAGER:SCHEDULED:DUALSETPOINT",
    "SETPOINTMANAGER:SINGLEZONE:COOLING",
    "SETPOINTMANAGER:SINGLEZONE:HEATING",
    "SETPOINTMANAGER:SINGLEZONE:HUMIDITY:MAXIMUM",
    "SETPOINTMANAGER:SINGLEZONE:HUMIDITY:MINIMUM",
    "SETPOINTMANAGER:SINGLEZONE:ONESTAGECOOLING",
    "SETPOINTMANAGER:SINGLEZONE:ONESTAGEHEATING",
    "SETPOINTMANAGER:SINGLEZONE:REHEAT",
    "SETPOINTMANAGER:SYSTEMNODERESET:TEMPERATURE",
    "SETPOINTMANAGER:SYSTEMNODERESET:HUMIDITY",
    "SETPOINTMANAGER:WARMEST",
    "SETPOINTMANAGER:WARMESTTEMPERATUREFLOW",
    "SOLARCOLLECTOR:FLATPLATE:PHOTOVOLTAICTHERMAL",
    "SOLARCOLLECTOR:FLATPLATE:WATER",
    "SOLARCOLLECTOR:INTEGRALCOLLECTORSTORAGE",
    "SOLARCOLLECTOR:UNGLAZEDTRANSPIRED",
    "SURFACEPROPERTY:LOCALENVIRONMENT",
    "SWIMMINGPOOL:INDOOR",
    "TEMPERINGVALVE",
    "THERMALSTORAGE:CHILLEDWATER:MIXED",
    "THERMALSTORAGE:CHILLEDWATER:STRATIFIED",
    "THERMALSTORAGE:ICE:DETAILED",
    "THERMALSTORAGE:ICE:SIMPLE",
    "WATERHEATER:HEATPUMP",
    "WATERHEATER:HEATPUMP:PUMPEDCONDENSER",
    "WATERHEATER:HEATPUMP:WRAPPEDCONDENSER",
    "WATERHEATER:MIXED",
    "WATERHEATER:STRATIFIED",
    "WATERUSE:CONNECTIONS",
    "ZONEHVAC:AIRDISTRIBUTIONUNIT",
    "ZONEHVAC:BASEBOARD:CONVECTIVE:ELECTRIC",
    "ZONEHVAC:BASEBOARD:CONVECTIVE:WATER",
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:ELECTRIC",
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM",
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER",
    "ZONEHVAC:COOLINGPANEL:RADIANTCONVECTIVE:WATER",
    "ZONEHVAC:DEHUMIDIFIER:DX",
    "ZONEHVAC:ENERGYRECOVERYVENTILATOR",
    "ZONEHVAC:EQUIPMENTCONNECTIONS",
    "ZONEHVAC:EVAPORATIVECOOLERUNIT",
    "ZONEHVAC:EXHAUSTCONTROL",
    "ZONEHVAC:FORCEDAIR:USERDEFINED",
    "ZONEHVAC:FOURPIPEFANCOIL",
    "ZONEHVAC:HIGHTEMPERATURERADIANT",
    "ZONEHVAC:HYBRIDUNITARYHVAC",
    "ZONEHVAC:IDEALLOADSAIRSYSTEM",
    "ZONEHVAC:LOWTEMPERATURERADIANT:CONSTANTFLOW",
    "ZONEHVAC:LOWTEMPERATURERADIANT:VARIABLEFLOW",
    "ZONEHVAC:OUTDOORAIRUNIT",
    "ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER",
    "ZONEHVAC:PACKAGEDTERMINALHEATPUMP",
    "ZONEHVAC:REFRIGERATIONCHILLERSET",
    "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW",
    "ZONEHVAC:UNITHEATER",
    "ZONEHVAC:UNITVENTILATOR",
    "ZONEHVAC:VENTILATEDSLAB",
    "ZONEHVAC:WATERTOAIRHEATPUMP",
    "ZONEHVAC:WINDOWAIRCONDITIONER",
    "ZONEPROPERTY:LOCALENVIRONMENT",
    "SPACEHVAC:EQUIPMENTCONNECTIONS",
    "SPACEHVAC:ZONEEQUIPMENTSPLITTER",
    "SPACEHVAC:ZONEEQUIPMENTMIXER"};

void RegisterNodeConnection(EnergyPlusData &state,
                            int const NodeNumber,                                // Number for this Node
                            std::string_view const NodeName,                     // Name of this Node
                            DataLoopNode::ConnectionObjectType const ObjectType, // Type of object this Node is connected to (e.g. Chiller:Electric)
                            std::string_view const ObjectName,                   // Name of object this Node is connected to (e.g. MyChiller)
                            DataLoopNode::ConnectionType const ConnectionType,   // Connection Type for this Node (must be valid)
                            NodeInputManager::CompFluidStream const FluidStream, // Count on Fluid Streams
                            bool const IsParent,                                 // True when node is a parent node
                            bool &errFlag,                                       // Will be True if errors already detected or if errors found here
                            std::string_view const InputFieldName                // Input Field Name
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   February 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine registers a node connection in the Node Connection data structure.  This
    // structure is intended to help with HVAC diagramming as well as validation of nodes.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName = "RegisterNodeConnection: ";

    bool ErrorsFoundHere = false;

    if ((ObjectType == DataLoopNode::ConnectionObjectType::Invalid) || (ObjectType == DataLoopNode::ConnectionObjectType::Num)) {
        ShowSevereError(state, "Developer Error: Invalid ObjectType");
        ShowContinueError(state, format("Occurs for Node={}, ObjectName={}", std::string{NodeName}, std::string{ObjectName}));
        ErrorsFoundHere = true;
    }

    std::string_view const objTypeStr = ConnectionObjectTypeNames[static_cast<int>(ObjectType)];
    std::string_view const conTypeStr = ConnectionTypeNames[static_cast<int>(ConnectionType)];

    if ((ConnectionType == DataLoopNode::ConnectionType::Invalid) || (ConnectionType == DataLoopNode::ConnectionType::Num)) {
        ShowSevereError(state, format("{}{}{}", RoutineName, "Invalid ConnectionType=", ConnectionType));
        ShowContinueError(state, format("Occurs for Node={}, ObjectType={}, ObjectName={}", NodeName, objTypeStr, ObjectName));
        ErrorsFoundHere = true;
    }

    bool MakeNew = true;
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Count) {
        if (state.dataBranchNodeConnections->NodeConnections(Count).NodeNumber != NodeNumber) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Count).ObjectType != ObjectType) {
            continue;
        }
        if (!Util::SameString(state.dataBranchNodeConnections->NodeConnections(Count).ObjectName, ObjectName)) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Count).ConnectionType != ConnectionType) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Count).FluidStream != FluidStream) {
            continue;
        }
        if ((state.dataBranchNodeConnections->NodeConnections(Count).ObjectIsParent && !IsParent) ||
            (!state.dataBranchNodeConnections->NodeConnections(Count).ObjectIsParent && IsParent)) {
            ShowSevereError(state, format("{}{}", RoutineName, "Node registered for both Parent and \"not\" Parent"));
            ShowContinueError(state, format("{}{}{}{}{}{}", "Occurs for Node=", NodeName, ", ObjectType=", ObjectType, ", ObjectName=", ObjectName));
            ErrorsFoundHere = true;
        }
        MakeNew = false;
    }
    if (MakeNew) {
        int constexpr NodeConnectionAlloc = 1000;
        ++state.dataBranchNodeConnections->NumOfNodeConnections;
        if (state.dataBranchNodeConnections->NumOfNodeConnections > 1 &&
            state.dataBranchNodeConnections->NumOfNodeConnections > state.dataBranchNodeConnections->MaxNumOfNodeConnections) {
            state.dataBranchNodeConnections->NodeConnections.resize(state.dataBranchNodeConnections->MaxNumOfNodeConnections += NodeConnectionAlloc);
        } else if (state.dataBranchNodeConnections->NumOfNodeConnections == 1) {
            state.dataBranchNodeConnections->NodeConnections.allocate(NodeConnectionAlloc);
            state.dataBranchNodeConnections->MaxNumOfNodeConnections = NodeConnectionAlloc;
        }

        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).NodeNumber = NodeNumber;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).NodeName = NodeName;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).ObjectType = ObjectType;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).ObjectName = ObjectName;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).ConnectionType = ConnectionType;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).FluidStream = FluidStream;
        state.dataBranchNodeConnections->NodeConnections(state.dataBranchNodeConnections->NumOfNodeConnections).ObjectIsParent = IsParent;
    }

    if (has_prefixi(objTypeStr, "AirTerminal:")) {
        if (!InputFieldName.empty()) {
            ++state.dataBranchNodeConnections->NumOfAirTerminalNodes;
            int constexpr EqNodeConnectionAlloc = 100;
            if (state.dataBranchNodeConnections->NumOfAirTerminalNodes > 1 &&
                state.dataBranchNodeConnections->NumOfAirTerminalNodes > state.dataBranchNodeConnections->MaxNumOfAirTerminalNodes) {
                state.dataBranchNodeConnections->AirTerminalNodeConnections.resize(state.dataBranchNodeConnections->MaxNumOfAirTerminalNodes +=
                                                                                   EqNodeConnectionAlloc);
            } else if (state.dataBranchNodeConnections->NumOfAirTerminalNodes == 1) {
                state.dataBranchNodeConnections->AirTerminalNodeConnections.allocate(EqNodeConnectionAlloc);
                state.dataBranchNodeConnections->MaxNumOfAirTerminalNodes = EqNodeConnectionAlloc;
            }

            // Check out AirTerminal inlet/outlet nodes
            bool Found = Util::FindItemInList(NodeName,
                                              state.dataBranchNodeConnections->AirTerminalNodeConnections,
                                              &EqNodeConnectionDef::NodeName,
                                              state.dataBranchNodeConnections->NumOfAirTerminalNodes - 1);
            if (Found != 0) { // Nodename already used
                ShowSevereError(state, fmt::format("{}{}=\"{}\" node name duplicated", RoutineName, ObjectType, ObjectName));
                ShowContinueError(state, format("NodeName=\"{}\", entered as type={}", NodeName, conTypeStr));
                ShowContinueError(state, fmt::format("In Field={}", InputFieldName));
                ShowContinueError(state,
                                  format("NodeName=\"{}\", entered as type={}", NodeName, ConnectionTypeNamesUC[static_cast<int>(ConnectionType)]));
                ShowContinueError(state, format("In Field={}", InputFieldName));
                ShowContinueError(
                    state,
                    format("Already used in {}=\"{}\".", objTypeStr, state.dataBranchNodeConnections->AirTerminalNodeConnections(Found).ObjectName));
                ShowContinueError(
                    state,
                    format(" as type={}, In Field={}",
                           ConnectionTypeNamesUC[static_cast<int>(state.dataBranchNodeConnections->AirTerminalNodeConnections(Found).ConnectionType)],
                           state.dataBranchNodeConnections->AirTerminalNodeConnections(Found).InputFieldName));
                ErrorsFoundHere = true;
            } else {
                state.dataBranchNodeConnections->AirTerminalNodeConnections(state.dataBranchNodeConnections->NumOfAirTerminalNodes).NodeName =
                    NodeName;
                state.dataBranchNodeConnections->AirTerminalNodeConnections(state.dataBranchNodeConnections->NumOfAirTerminalNodes).ObjectType =
                    ObjectType;
                state.dataBranchNodeConnections->AirTerminalNodeConnections(state.dataBranchNodeConnections->NumOfAirTerminalNodes).ObjectName =
                    ObjectName;
                state.dataBranchNodeConnections->AirTerminalNodeConnections(state.dataBranchNodeConnections->NumOfAirTerminalNodes).ConnectionType =
                    ConnectionType;
                state.dataBranchNodeConnections->AirTerminalNodeConnections(state.dataBranchNodeConnections->NumOfAirTerminalNodes).InputFieldName =
                    InputFieldName;
            }
        } else {
            ShowSevereError(state, fmt::format("{}{} , Developer Error: Input Field Name not included.", RoutineName, objTypeStr));
            ShowContinueError(state, "Node names not checked for duplication.");
        }
    }

    if (ErrorsFoundHere) {
        errFlag = true;
    }
}

void OverrideNodeConnectionType(
    EnergyPlusData &state,
    int const NodeNumber,                                // Number for this Node
    std::string const &NodeName,                         // Name of this Node
    DataLoopNode::ConnectionObjectType const ObjectType, // Type of object this Node is connected to (e.g. Chiller:Electric)
    std::string const &ObjectName,                       // Name of object this Node is connected to (e.g. MyChiller)
    DataLoopNode::ConnectionType const ConnectionType,   // Connection Type for this Node (must be valid)
    NodeInputManager::CompFluidStream const FluidStream, // Count on Fluid Streams
    bool const IsParent,                                 // True when node is a parent node
    bool &errFlag                                        // Will be True if errors already detected or if errors found here
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. J. Witte
    //       DATE WRITTEN   June 2016

    // PURPOSE:
    // This subroutine modifies an existing node connection in the Node Connection data structure.  This
    // structure is intended to help with HVAC diagramming as well as validation of nodes. This function
    // is a based on RegisterNodeConnection.

    static constexpr std::string_view RoutineName("ModifyNodeConnectionType: ");

    if ((ConnectionType == DataLoopNode::ConnectionType::Invalid) || (ConnectionType == DataLoopNode::ConnectionType::Num)) {
        ShowSevereError(state, format("{}{}{}", RoutineName, "Invalid ConnectionType=", ConnectionType));
        ShowContinueError(
            state,
            format("Occurs for Node={}, ObjectType={}, ObjectName={}", NodeName, ConnectionTypeNames[static_cast<int>(ObjectType)], ObjectName));
        errFlag = true;
    }

    int Found = 0;
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Count) {
        if (state.dataBranchNodeConnections->NodeConnections(Count).NodeNumber != NodeNumber) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Count).ObjectType != ObjectType) {
            continue;
        }
        if (!Util::SameString(state.dataBranchNodeConnections->NodeConnections(Count).ObjectName, ObjectName)) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Count).FluidStream != FluidStream) {
            continue;
        }
        if ((state.dataBranchNodeConnections->NodeConnections(Count).ObjectIsParent != IsParent)) {
            continue;
        }
        Found = Count;
        break;
    }

    if (Found > 0) {
        state.dataBranchNodeConnections->NodeConnections(Found).ConnectionType = ConnectionType;
    } else {
        ShowSevereError(state, format("{}{}", RoutineName, "Existing node connection not found."));
        ShowContinueError(
            state,
            format("Occurs for Node={}, ObjectType={}, ObjectName={}", NodeName, ConnectionTypeNames[static_cast<int>(ObjectType)], ObjectName));
        errFlag = true;
    }
}

void CheckNodeConnections(EnergyPlusData &state, bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine processes the node connection data structure looking at:
    // 1.  In the NodeConnections list, for any node which appears as a sensor or an
    // actuator, the same node must also appear in the connections list at least once
    // as a node type which is not sensor or actuator or outsideair.
    // 2.  In the NodeConnections list, for any node which appears as a setpoint, the
    // same node must also appear in the connections list at least once as a node type
    // which is not a setpoint or outsideair.
    // 3.  Every ZoneInlet must appear as an outlet from something, otherwise it will
    // do nothing.
    // 4.  Every ZoneExhaust must appear as an inlet to something,
    // otherwise it will do nothing.
    // 5.  Every inlet node should match either an Outlet, ZoneReturn, ZoneExhaust, ReliefAir,
    // or OutsideAir node.
    //  With the current data structure, when checking inlets:
    //    a)  If an InletNode's object is AirLoopHVAC, CondenserLoop, or PlantLoop, then skip the test.
    //    b)  If an InletNode's object is not one of the above types, it is valid if the
    //        same node name appears as an INLET to an AirLoopHVAC, CondenserLoop, or PlantLoop.
    // 6.  Any given node can only be an inlet once in the list of Non-Parent Node Connections
    // 7.  Any given node can only be an outlet once in the list of Non-Parent Node Connections
    // 8.  non-parent outlet nodes -- must never be an outlet more than once
    // 9.  nodes of type OutsideAirReference must be registered as DataLoopNode::NodeConnectionType::OutsideAir
    // 10. fluid streams cannot have multiple inlet/outlet nodes on same component
    // 11. zone nodes may not be used as anything else except as a setpoint, sensor or actuator node

    // METHODOLOGY EMPLOYED:
    // Needs description, as appropriate.

    // Using/Aliasing

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool IsValid;
    bool IsInlet;
    bool IsOutlet;
    bool MatchedAtLeastOne;
    int ErrorCounter;
    Array1D_int FluidStreamInletCount;
    Array1D_int FluidStreamOutletCount;
    Array1D_int NodeObjects;
    Array1D_bool FluidStreamCounts;

    ErrorCounter = 0;

    //  Check 1 -- check sensor and actuator nodes
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::Sensor) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if ((state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Actuator) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Sensor)) {
                continue;
            }

            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", Sensor node did not find a matching node of appropriate type (other than "
                                   "Actuator or Sensor).",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
            ErrorsFound = true;
        }
    }

    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::Actuator) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }

            if ((state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Actuator) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Sensor) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::OutsideAir)) {
                continue;
            }

            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", Actuator node did not find a matching node of appropriate type (other than "
                                   "Actuator, Sensor, OutsideAir).",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
            ErrorsFound = true;
        }
    }

    // Check 2 -- setpoint nodes
    // Check 2a -- setpoint node must also be an inlet or an outlet (CR8212)
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::SetPoint) {
            continue;
        }
        IsValid = false;
        IsInlet = false;
        IsOutlet = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if ((state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::SetPoint) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::OutsideAir)) {
                continue;
            }

            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Inlet) {
                IsInlet = true;
            } else if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Outlet) {
                IsOutlet = true;
            }
            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", Setpoint node did not find a matching node of appropriate type (other than "
                                   "Setpoint, OutsideAir).",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
            ErrorsFound = true;
        }
        if (!IsInlet && !IsOutlet) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", Setpoint node did not find a matching node of type Inlet or Outlet.",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));
            ShowContinueError(state, "It appears this node is not part of the HVAC system.");

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
        }
    }

    // Check 2a -- setpoint node must also be an inlet or an outlet (CR8212)

    // Check 3 -- zone inlet nodes -- must be an outlet somewhere
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::ZoneInlet) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::Outlet) {
                continue;
            }
            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", ZoneInlet node did not find an outlet node.",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
        }
    }

    // Check 4 -- zone exhaust nodes -- must be an inlet somewhere
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::ZoneExhaust) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::Inlet) {
                continue;
            }
            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", ZoneExhaust node did not find a matching inlet node.",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
        }
    }

    // Check 5 -- return plenum induced air outlet nodes -- must be an inlet somewhere
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::InducedAir) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::Inlet) {
                continue;
            }
            IsValid = true;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("Node Connection Error, Node=\"{}\", Return plenum induced air outlet node did not find a matching inlet node.",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
            ErrorsFound = true;
        }
    }

    // Check 6 -- every inlet should have a matching outlet, zonereturn, zoneexhaust, induced air, reliefair or outsideair
    //    a)  If an InletNode's object is AirLoopHVAC, CondenserLoop, or PlantLoop, then skip the test.
    //    b)  If an InletNode's object is not one of the above types, it is valid if the
    //        same node name appears as an INLET to an AirLoopHVAC, CondenserLoop, or PlantLoop.
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::Inlet) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType == DataLoopNode::ConnectionObjectType::AirLoopHVAC ||
            state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType == DataLoopNode::ConnectionObjectType::CondenserLoop ||
            state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType == DataLoopNode::ConnectionObjectType::PlantLoop) {
            continue;
        }
        IsValid = false;
        MatchedAtLeastOne = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }

            if ((state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Outlet) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::ZoneReturn) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::ZoneExhaust) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::InducedAir) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::ReliefAir) ||
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::OutsideAir)) {
                MatchedAtLeastOne = true;
                continue;
            }

            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Inlet &&
                (state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType == DataLoopNode::ConnectionObjectType::AirLoopHVAC ||
                 state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType == DataLoopNode::ConnectionObjectType::CondenserLoop ||
                 state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType == DataLoopNode::ConnectionObjectType::PlantLoop)) {
                MatchedAtLeastOne = true;
                continue;
            }
            IsValid = false;
        }
        if (!IsValid && !MatchedAtLeastOne) {
            ShowSevereError(state,
                            format("{}{}{}",
                                   "Node Connection Error, Node=\"",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName,
                                   R"(", Inlet node did not find an appropriate matching "outlet" node.)"));
            ShowContinueError(state, "If this is an outdoor air inlet node, it must be listed in an OutdoorAir:Node or OutdoorAir:NodeList object.");

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));
            ++ErrorCounter;
        }
    }

    // Check 7 -- non-parent inlet nodes -- must never be an inlet more than once
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        // Only non-parent node connections
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectIsParent) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::Inlet) {
            continue;
        }
        for (int Loop2 = Loop1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectIsParent) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::Inlet) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber ==
                state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber) {
                ShowSevereError(state,
                                format("Node Connection Error, Node=\"{}\", The same node appears as a non-parent Inlet node more than once.",
                                       state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectName));
                ++ErrorCounter;
                break;
            }
        }
    }

    // Check 8 -- non-parent outlet nodes -- must never be an outlet more than once
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        // Only non-parent node connections
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectIsParent) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::Outlet) {
            continue;
        }
        IsValid = true;
        for (int Loop2 = Loop1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectIsParent) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::Outlet) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber ==
                state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber) {
                // Skip if one of the
                ShowSevereError(state,
                                format("Node Connection Error, Node=\"{}\", The same node appears as a non-parent Outlet node more than once.",
                                       state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectName));

                ++ErrorCounter;
                break;
            }
        }
    }

    // Check 9 -- nodes of type OutsideAirReference must be registered as DataLoopNode::NodeConnectionType::OutsideAir
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::OutsideAirReference) {
            continue;
        }
        IsValid = false;
        for (int Loop2 = 1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeNumber !=
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeNumber) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType != DataLoopNode::ConnectionType::OutsideAir) {
                continue;
            }
            IsValid = true;
            break;
        }
        if (!IsValid) {
            ShowSevereError(state,
                            format("{}{}{}",
                                   "Node Connection Error, Node=\"",
                                   state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName,
                                   R"(", Outdoor Air Reference did not find an appropriate "outdoor air" node.)"));
            ShowContinueError(state, "This node must be listed in an OutdoorAir:Node or OutdoorAir:NodeList object in order to set its conditions.");

            ShowContinueError(state,
                              format("Reference Object={}, Name={}",
                                     ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                                     state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));

            ++ErrorCounter;
        }
    }

    // Check 10 -- fluid streams cannot have multiple inlet/outlet nodes on same component
    //  can have multiple inlets with one outlet or vice versa but cannot have multiple both inlet and outlet
    if (state.dataBranchNodeConnections->NumOfNodeConnections > 0) {
        int MaxFluidStream = static_cast<int>(maxval(state.dataBranchNodeConnections->NodeConnections, &NodeConnectionDef::FluidStream));
        FluidStreamInletCount.allocate(MaxFluidStream);
        FluidStreamOutletCount.allocate(MaxFluidStream);
        FluidStreamCounts.allocate(MaxFluidStream);
        NodeObjects.allocate(state.dataBranchNodeConnections->NumOfNodeConnections + 1);
        FluidStreamInletCount = 0;
        FluidStreamOutletCount = 0;
        NodeObjects = 0;
        FluidStreamCounts = false;
        // Following code relies on node connections for single object type/name being grouped together
        int Object = 1;
        int EndConnect = 0;
        int NumObjects = 2;
        NodeObjects(1) = 1;
        while (Object < state.dataBranchNodeConnections->NumOfNodeConnections) {
            if (state.dataBranchNodeConnections->NodeConnections(Object).ObjectType !=
                    state.dataBranchNodeConnections->NodeConnections(Object + 1).ObjectType ||
                state.dataBranchNodeConnections->NodeConnections(Object).ObjectName !=
                    state.dataBranchNodeConnections->NodeConnections(Object + 1).ObjectName) {
                EndConnect = Object + 1;
                NodeObjects(NumObjects) = EndConnect;
                // if (Object + 1 < state.dataBranchNodeConnections->NumOfNodeConnections) ++NumObjects;
                ++NumObjects;
            }
            ++Object;
        }
        NodeObjects(NumObjects) = state.dataBranchNodeConnections->NumOfNodeConnections + 1;
        // NodeObjects now contains each consecutive object...
        for (Object = 1; Object <= NumObjects - 1; ++Object) {
            IsValid = true;
            FluidStreamInletCount = 0;
            FluidStreamOutletCount = 0;
            FluidStreamCounts = false;
            int Loop1 = NodeObjects(Object);
            if (state.dataBranchNodeConnections->NumOfNodeConnections < 2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectIsParent) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType == DataLoopNode::ConnectionType::Inlet) {
                ++FluidStreamInletCount(static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).FluidStream));
            } else if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType == DataLoopNode::ConnectionType::Outlet) {
                ++FluidStreamOutletCount(static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).FluidStream));
            }
            for (int Loop2 = Loop1 + 1; Loop2 <= NodeObjects(Object + 1) - 1; ++Loop2) {
                if (state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectIsParent) {
                    continue;
                }
                if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Inlet) {
                    ++FluidStreamInletCount(static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop2).FluidStream));
                } else if (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Outlet) {
                    ++FluidStreamOutletCount(static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop2).FluidStream));
                }
            }
            for (int Loop2 = 1; Loop2 <= MaxFluidStream; ++Loop2) {
                if (FluidStreamInletCount(Loop2) > 1 && FluidStreamOutletCount(Loop2) > 1) {
                    IsValid = false;
                    FluidStreamCounts(Loop2) = true;
                }
            }
            if (!IsValid) {

                ShowSevereError(
                    state,
                    format("(Developer) Node Connection Error, Object={}:{}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));

                ShowContinueError(state, "Object has multiple connections on both inlet and outlet fluid streams.");
                for (int Loop2 = 1; Loop2 <= MaxFluidStream; ++Loop2) {
                    if (FluidStreamCounts(Loop2)) {
                        ShowContinueError(state, format("...occurs in Fluid Stream [{}].", Loop2));
                    }
                }
                ++ErrorCounter;
                ErrorsFound = true;
            }
        }
        FluidStreamInletCount.deallocate();
        FluidStreamOutletCount.deallocate();
        FluidStreamCounts.deallocate();
        NodeObjects.deallocate();
    }

    // Check 11 - zone nodes may not be used as anything else except as a setpoint, sensor or actuator node
    for (int Loop1 = 1; Loop1 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop1) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop1).ConnectionType != DataLoopNode::ConnectionType::ZoneNode) {
            continue;
        }
        IsValid = true;
        for (int Loop2 = Loop1; Loop2 <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop2) {
            if (Loop1 == Loop2) {
                continue;
            }
            if (state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName ==
                state.dataBranchNodeConnections->NodeConnections(Loop2).NodeName) {

                if ((state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Actuator) ||
                    (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::Sensor) ||
                    (state.dataBranchNodeConnections->NodeConnections(Loop2).ConnectionType == DataLoopNode::ConnectionType::SetPoint)) {
                    continue;
                }

                ShowSevereError(state,
                                format("Node Connection Error, Node Name=\"{}\", The same zone node appears more than once.",
                                       state.dataBranchNodeConnections->NodeConnections(Loop1).NodeName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Object Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop1).ObjectName));

                ShowContinueError(
                    state,
                    format("Reference Object={}, Object Name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectType)],
                           state.dataBranchNodeConnections->NodeConnections(Loop2).ObjectName));

                ++ErrorCounter;
                ErrorsFound = true;
            }
        }
    }

    state.dataBranchNodeConnections->NumNodeConnectionErrors += ErrorCounter;
}

bool IsParentObject(EnergyPlusData &state, DataLoopNode::ConnectionObjectType const ComponentType, std::string const &ComponentName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS FUNCTION:
    // This routine determines if a component name is a parent node.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // Return value
    bool IsParent = false; // True if this combination is a parent

    for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Loop) {
        if (state.dataBranchNodeConnections->NodeConnections(Loop).ObjectType == ComponentType &&
            state.dataBranchNodeConnections->NodeConnections(Loop).ObjectName == ComponentName) {
            if (state.dataBranchNodeConnections->NodeConnections(Loop).ObjectIsParent) {
                IsParent = true;
            }
            break;
        }
    }
    if (!IsParent) {
        IsParent = IsParentObjectCompSet(state, ComponentType, ComponentName);
    }

    return IsParent;
}

int WhichParentSet(EnergyPlusData &state, DataLoopNode::ConnectionObjectType const ComponentType, std::string const &ComponentName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS FUNCTION:
    // This routine determines which parent node list (number) for a given component name
    // and type.

    // Return value
    int WhichOne = 0;

    for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumOfActualParents; ++Loop) {
        if (state.dataBranchNodeConnections->ParentNodeList(Loop).ComponentType == ComponentType &&
            state.dataBranchNodeConnections->ParentNodeList(Loop).ComponentName == ComponentName) {
            WhichOne = Loop;
            break;
        }
    }

    return WhichOne;
}

void GetParentData(EnergyPlusData &state,
                   DataLoopNode::ConnectionObjectType const ComponentType,
                   std::string const &ComponentName,
                   std::string &InletNodeName,
                   int &InletNodeNum,
                   std::string &OutletNodeName,
                   int &OutletNodeNum,
                   bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets node data for a given Parent Component Type and Name Name.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrInObject = false;

    InletNodeName = std::string();
    InletNodeNum = 0;
    OutletNodeName = std::string();
    OutletNodeNum = 0;
    ErrInObject = false;

    int Which = WhichParentSet(state, ComponentType, ComponentName);
    if (Which != 0) {
        InletNodeName = state.dataBranchNodeConnections->ParentNodeList(Which).InletNodeName;
        OutletNodeName = state.dataBranchNodeConnections->ParentNodeList(Which).OutletNodeName;
        // Get Node Numbers
        InletNodeNum =
            Util::FindItemInList(InletNodeName, state.dataLoopNodes->NodeID({1, state.dataLoopNodes->NumOfNodes}), state.dataLoopNodes->NumOfNodes);
        OutletNodeNum =
            Util::FindItemInList(OutletNodeName, state.dataLoopNodes->NodeID({1, state.dataLoopNodes->NumOfNodes}), state.dataLoopNodes->NumOfNodes);
    } else if (IsParentObjectCompSet(state, ComponentType, ComponentName)) {
        Which = WhichCompSet(state, ComponentType, ComponentName);
        if (Which != 0) {
            InletNodeName = state.dataBranchNodeConnections->CompSets(Which).InletNodeName;
            OutletNodeName = state.dataBranchNodeConnections->CompSets(Which).OutletNodeName;
            InletNodeNum = Util::FindItemInList(
                InletNodeName, state.dataLoopNodes->NodeID({1, state.dataLoopNodes->NumOfNodes}), state.dataLoopNodes->NumOfNodes);
            OutletNodeNum = Util::FindItemInList(
                OutletNodeName, state.dataLoopNodes->NodeID({1, state.dataLoopNodes->NumOfNodes}), state.dataLoopNodes->NumOfNodes);
        } else {
            ErrInObject = true;
            ShowWarningError(state,
                             format("GetParentData: Component Type={}, Component Name={} not found.",
                                    ConnectionObjectTypeNames[static_cast<int>(ComponentType)],
                                    ComponentName));
        }
    } else {
        ErrInObject = true;
        ShowWarningError(state,
                         format("GetParentData: Component Type={}, Component Name={} not found.",
                                ConnectionObjectTypeNames[static_cast<int>(ComponentType)],
                                ComponentName));
    }

    if (ErrInObject) {
        ErrorsFound = true;
    }
}

bool IsParentObjectCompSet(EnergyPlusData &state, DataLoopNode::ConnectionObjectType const ComponentType, std::string const &ComponentName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS FUNCTION:
    // This routine determines if a component name is a parent node.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // Return value
    bool IsParent = false; // True if this combination is a parent

    for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumCompSets; ++Loop) {
        if (state.dataBranchNodeConnections->CompSets(Loop).ParentObjectType == ComponentType &&
            state.dataBranchNodeConnections->CompSets(Loop).ParentCName == ComponentName) {
            IsParent = true;
            break;
        }
    }

    return IsParent;
}

int WhichCompSet(EnergyPlusData &state, DataLoopNode::ConnectionObjectType const ComponentType, std::string const &ComponentName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS FUNCTION:
    // This routine determines which comp set (number) for a given component name
    // and type.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // Return value
    int WhichOne = 0;

    for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumCompSets; ++Loop) {
        if (state.dataBranchNodeConnections->CompSets(Loop).ComponentObjectType == ComponentType &&
            state.dataBranchNodeConnections->CompSets(Loop).CName == ComponentName) {
            WhichOne = Loop;
            break;
        }
    }

    return WhichOne;
}

int GetNumChildren(EnergyPlusData &state, DataLoopNode::ConnectionObjectType const ComponentType, std::string const &ComponentName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This routine counts the number of children for a parent Component Set.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // Return value
    int NumChildren;

    NumChildren = 0;
    if (IsParentObject(state, ComponentType, ComponentName)) {
        for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumCompSets; ++Loop) {
            if (state.dataBranchNodeConnections->CompSets(Loop).ParentObjectType == ComponentType &&
                state.dataBranchNodeConnections->CompSets(Loop).ParentCName == ComponentName) {
                ++NumChildren;
            }
        }
    }

    return NumChildren;
}

void GetComponentData(EnergyPlusData &state,
                      DataLoopNode::ConnectionObjectType const ComponentType,
                      std::string const &ComponentName,
                      bool &IsParent, // true or false
                      int &NumInlets,
                      Array1D_string &InletNodeNames,
                      Array1D_int &InletNodeNums,
                      Array1D<NodeInputManager::CompFluidStream> &InletFluidStreams,
                      int &NumOutlets,
                      Array1D_string &OutletNodeNames,
                      Array1D_int &OutletNodeNums,
                      Array1D<NodeInputManager::CompFluidStream> &OutletFluidStreams)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets data for a given Component Type and Name Name.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    if (allocated(InletNodeNames)) {
        InletNodeNames.deallocate();
    }
    if (allocated(InletNodeNums)) {
        InletNodeNums.deallocate();
    }
    if (allocated(InletFluidStreams)) {
        InletFluidStreams.deallocate();
    }
    if (allocated(OutletNodeNames)) {
        OutletNodeNames.deallocate();
    }
    if (allocated(OutletNodeNums)) {
        OutletNodeNums.deallocate();
    }
    if (allocated(OutletFluidStreams)) {
        OutletFluidStreams.deallocate();
    }

    NumInlets = 0;
    NumOutlets = 0;

    IsParent = false;
    for (int Which = 1; Which <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Which) {
        if (state.dataBranchNodeConnections->NodeConnections(Which).ObjectType != ComponentType ||
            state.dataBranchNodeConnections->NodeConnections(Which).ObjectName != ComponentName) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Which).ObjectIsParent) {
            IsParent = true;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Which).ConnectionType == DataLoopNode::ConnectionType::Inlet) {
            ++NumInlets;
        } else if (state.dataBranchNodeConnections->NodeConnections(Which).ConnectionType == DataLoopNode::ConnectionType::Outlet) {
            ++NumOutlets;
        }
    }

    InletNodeNames.allocate(NumInlets);
    InletNodeNums.allocate(NumInlets);
    InletFluidStreams.allocate(NumInlets);
    OutletNodeNames.allocate(NumOutlets);
    OutletNodeNums.allocate(NumOutlets);
    OutletFluidStreams.allocate(NumOutlets);

    InletNodeNames = std::string();
    InletNodeNums = 0;
    InletFluidStreams = NodeInputManager::CompFluidStream::Invalid;
    OutletNodeNames = std::string();
    OutletNodeNums = 0;
    OutletFluidStreams = NodeInputManager::CompFluidStream::Invalid;
    NumInlets = 0;
    NumOutlets = 0;

    for (int Which = 1; Which <= state.dataBranchNodeConnections->NumOfNodeConnections; ++Which) {
        if (state.dataBranchNodeConnections->NodeConnections(Which).ObjectType != ComponentType ||
            state.dataBranchNodeConnections->NodeConnections(Which).ObjectName != ComponentName) {
            continue;
        }
        if (state.dataBranchNodeConnections->NodeConnections(Which).ConnectionType == DataLoopNode::ConnectionType::Inlet) {
            ++NumInlets;
            InletNodeNames(NumInlets) = state.dataBranchNodeConnections->NodeConnections(Which).NodeName;
            InletNodeNums(NumInlets) = state.dataBranchNodeConnections->NodeConnections(Which).NodeNumber;
            InletFluidStreams(NumInlets) = state.dataBranchNodeConnections->NodeConnections(Which).FluidStream;
        } else if (state.dataBranchNodeConnections->NodeConnections(Which).ConnectionType == DataLoopNode::ConnectionType::Outlet) {
            ++NumOutlets;
            OutletNodeNames(NumOutlets) = state.dataBranchNodeConnections->NodeConnections(Which).NodeName;
            OutletNodeNums(NumOutlets) = state.dataBranchNodeConnections->NodeConnections(Which).NodeNumber;
            OutletFluidStreams(NumOutlets) = state.dataBranchNodeConnections->NodeConnections(Which).FluidStream;
        }
    }
}

void GetChildrenData(EnergyPlusData &state,
                     DataLoopNode::ConnectionObjectType const ComponentType,
                     std::string const &ComponentName,
                     int &NumChildren,
                     EPVector<DataLoopNode::ConnectionObjectType> &ChildrenCType,
                     Array1D_string &ChildrenCName,
                     Array1D_string &InletNodeName,
                     Array1D_int &InletNodeNum,
                     Array1D_string &OutletNodeName,
                     Array1D_int &OutletNodeNum,
                     bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets children data for given parent node.

    // METHODOLOGY EMPLOYED:
    // Traverses CompSet structure.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    EPVector<DataLoopNode::ConnectionObjectType> ChildCType;
    Array1D_string ChildCName;
    Array1D_string ChildInNodeName;
    Array1D_string ChildOutNodeName;
    Array1D_int ChildInNodeNum;
    Array1D_int ChildOutNodeNum;
    Array1D_bool ChildMatched;

    bool ErrInObject;

    std::fill(ChildrenCType.begin(), ChildrenCType.end(), DataLoopNode::ConnectionObjectType::Invalid);
    ChildrenCName = std::string();
    InletNodeName = std::string();
    InletNodeNum = 0;
    OutletNodeName = std::string();
    OutletNodeNum = 0;
    ErrInObject = false;

    if (!IsParentObject(state, ComponentType, ComponentName)) {
        ShowWarningError(state,
                         format("GetChildrenData: Requested Children Data for non Parent Node={}:{}.",
                                ConnectionObjectTypeNames[static_cast<int>(ComponentType)],
                                ComponentName));
        ErrorsFound = true;

    } else if ((NumChildren = GetNumChildren(state, ComponentType, ComponentName)) == 0) {
        ShowWarningError(state,
                         format("GetChildrenData: Parent Node has no children, node={}:{}.",
                                ConnectionObjectTypeNames[static_cast<int>(ComponentType)],
                                ComponentName));

    } else {
        int ParentInletNodeNum;
        int ParentOutletNodeNum;
        std::string ParentInletNodeName;
        std::string ParentOutletNodeName;
        GetParentData(
            state, ComponentType, ComponentName, ParentInletNodeName, ParentInletNodeNum, ParentOutletNodeName, ParentOutletNodeNum, ErrInObject);
        ChildCType.allocate(NumChildren);
        ChildCName.allocate(NumChildren);
        ChildInNodeName.allocate(NumChildren);
        ChildOutNodeName.allocate(NumChildren);
        ChildInNodeNum.allocate(NumChildren);
        ChildOutNodeNum.allocate(NumChildren);
        ChildMatched.allocate(NumChildren);

        std::fill(ChildCType.begin(), ChildCType.end(), DataLoopNode::ConnectionObjectType::Invalid);
        ChildCName = std::string();
        ChildInNodeName = std::string();
        ChildOutNodeName = std::string();
        ChildInNodeNum = 0;
        ChildOutNodeNum = 0;
        ChildMatched = false;

        int CountNum = 0;
        for (int Loop = 1; Loop <= state.dataBranchNodeConnections->NumCompSets; ++Loop) {
            auto const &compSet = state.dataBranchNodeConnections->CompSets(Loop);
            if (compSet.ParentObjectType == ComponentType && compSet.ParentCName == ComponentName) {
                ++CountNum;
                ChildCType(CountNum) = compSet.ComponentObjectType;
                ChildCName(CountNum) = compSet.CName;
                ChildInNodeName(CountNum) = compSet.InletNodeName;
                ChildOutNodeName(CountNum) = compSet.OutletNodeName;
                // Get Node Numbers
                ChildInNodeNum(CountNum) = Util::FindItemInList(ChildInNodeName(CountNum), state.dataLoopNodes->NodeID);
                ChildOutNodeNum(CountNum) = Util::FindItemInList(ChildOutNodeName(CountNum), state.dataLoopNodes->NodeID);
            }
        }

        if (CountNum != NumChildren) {
            ShowSevereError(state, "GetChildrenData: Counted nodes not equal to GetNumChildren count");
            ErrorsFound = true;

        } else {
            // Children arrays built.  Now "sort" for flow connection order(?)
            // FindIntInList is 0-based and FindItemInList is 1-based .. great!
            int ParentInletNodeIndex = 0;
            for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                if (ChildInNodeNum(Loop) == ParentInletNodeNum) {
                    ParentInletNodeIndex = Loop;
                    break;
                }
            }

            int ParentOutletNodeIndex = 0;
            for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                if (ChildOutNodeNum(Loop) == ParentOutletNodeNum) {
                    ParentOutletNodeIndex = Loop;
                    break;
                }
            }

            // Parent inlet node matches one of the inlet-nodes of the sub-components
            if (ParentInletNodeIndex > 0) {
                int MatchInNodeNum = ParentInletNodeNum;
                CountNum = 0;

                while (CountNum < NumChildren) {
                    int MatchInNodeIndex = 0;
                    for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                        if (ChildInNodeNum(Loop) == MatchInNodeNum && !ChildMatched(Loop)) {
                            MatchInNodeIndex = Loop;
                            break;
                        }
                    }

                    if (MatchInNodeIndex == 0) { // The chain is broken
                        break;
                    }

                    ++CountNum;
                    ChildrenCType(CountNum) = ChildCType(MatchInNodeIndex);
                    ChildrenCName(CountNum) = ChildCName(MatchInNodeIndex);
                    InletNodeName(CountNum) = ChildInNodeName(MatchInNodeIndex);
                    InletNodeNum(CountNum) = ChildInNodeNum(MatchInNodeIndex);
                    OutletNodeName(CountNum) = ChildOutNodeName(MatchInNodeIndex);
                    OutletNodeNum(CountNum) = ChildOutNodeNum(MatchInNodeIndex);
                    ChildMatched(MatchInNodeIndex) = true;
                    MatchInNodeNum = ChildOutNodeNum(MatchInNodeIndex);
                }

                for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                    if (ChildMatched(Loop)) {
                        continue;
                    }
                    ++CountNum;
                    ChildrenCType(CountNum) = ChildCType(Loop);
                    ChildrenCName(CountNum) = ChildCName(Loop);
                    InletNodeName(CountNum) = ChildInNodeName(Loop);
                    InletNodeNum(CountNum) = ChildInNodeNum(Loop);
                    OutletNodeName(CountNum) = ChildOutNodeName(Loop);
                    OutletNodeNum(CountNum) = ChildOutNodeNum(Loop);
                }

                // Parent outlet node matches one of the outlet-nodes of the sub-components
            } else if (ParentOutletNodeIndex > 0) {
                int MatchOutNodeNum = ParentOutletNodeNum;
                CountNum = NumChildren + 1;

                while (CountNum > 1) {
                    int MatchOutNodeIndex = 0;
                    for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                        if (ChildOutNodeNum(Loop) == MatchOutNodeNum && !ChildMatched(Loop)) {
                            MatchOutNodeIndex = Loop;
                            break;
                        }
                    }

                    if (MatchOutNodeIndex == 0) { // The chain is broken
                        break;
                    }

                    --CountNum;
                    ChildrenCType(CountNum) = ChildCType(MatchOutNodeIndex);
                    ChildrenCName(CountNum) = ChildCName(MatchOutNodeIndex);
                    InletNodeName(CountNum) = ChildInNodeName(MatchOutNodeIndex);
                    InletNodeNum(CountNum) = ChildInNodeNum(MatchOutNodeIndex);
                    OutletNodeName(CountNum) = ChildOutNodeName(MatchOutNodeIndex);
                    OutletNodeNum(CountNum) = ChildOutNodeNum(MatchOutNodeIndex);
                    ChildMatched(MatchOutNodeIndex) = true;
                    MatchOutNodeNum = ChildInNodeNum(MatchOutNodeIndex);
                }

                CountNum = 0;
                for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                    if (ChildMatched(Loop)) {
                        continue;
                    }
                    ++CountNum;
                    ChildrenCType(CountNum) = ChildCType(Loop);
                    ChildrenCName(CountNum) = ChildCName(Loop);
                    InletNodeName(CountNum) = ChildInNodeName(Loop);
                    InletNodeNum(CountNum) = ChildInNodeNum(Loop);
                    OutletNodeName(CountNum) = ChildOutNodeName(Loop);
                    OutletNodeNum(CountNum) = ChildOutNodeNum(Loop);
                }
            } else {
                // No sub-component is connected to either the parent
                // component's inlet- or outlet- nodes? Just copy the
                // sub-components in the order in which they appear in
                // the CompSets

                for (int Loop = 1; Loop <= NumChildren; ++Loop) {
                    if (ChildMatched(Loop)) {
                        continue;
                    }
                    ChildrenCType(Loop) = ChildCType(Loop);
                    ChildrenCName(Loop) = ChildCName(Loop);
                    InletNodeName(Loop) = ChildInNodeName(Loop);
                    InletNodeNum(Loop) = ChildInNodeNum(Loop);
                    OutletNodeName(Loop) = ChildOutNodeName(Loop);
                    OutletNodeNum(Loop) = ChildOutNodeNum(Loop);
                }
            }
        }

        ChildCType.deallocate();
        ChildCName.deallocate();
        ChildInNodeName.deallocate();
        ChildOutNodeName.deallocate();
        ChildInNodeNum.deallocate();
        ChildOutNodeNum.deallocate();
        ChildMatched.deallocate();
    }

    if (ErrInObject) {
        ErrorsFound = true;
    }
}

void SetUpCompSets(EnergyPlusData &state,
                   std::string_view ParentType,       // Parent Object Type
                   std::string_view ParentName,       // Parent Object Name
                   std::string_view CompType,         // Component Type
                   std::string_view CompName,         // Component Name
                   std::string_view InletNode,        // Inlet Node Name
                   std::string_view OutletNode,       // Outlet Node Name
                   std::string_view const Description // Description
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2001

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up "Component Sets" as input in the branch
    // lists.  These can be used later to verify that the proper names and
    // inlet/outlet nodes have been input.  This routine assumes that identical
    // "CompSets" cannot be used in multiple places and issues a warning if they are.

    std::string ParentTypeUC = Util::makeUPPER(ParentType);
    std::string CompTypeUC = Util::makeUPPER(CompType);
    // TODO: Refactor this away by passing in enums
    DataLoopNode::ConnectionObjectType ParentTypeEnum =
        static_cast<DataLoopNode::ConnectionObjectType>(getEnumValue(ConnectionObjectTypeNamesUC, ParentTypeUC));
    assert(ParentTypeEnum != DataLoopNode::ConnectionObjectType::Invalid);

    DataLoopNode::ConnectionObjectType ComponentTypeEnum =
        static_cast<DataLoopNode::ConnectionObjectType>(getEnumValue(ConnectionObjectTypeNamesUC, CompTypeUC));
    assert(ComponentTypeEnum != DataLoopNode::ConnectionObjectType::Invalid);

    int Found = 0;

    // See if Component-Nodes set is already there - should be unique
    // Try to fill in blanks (passed in as undefined
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
        if (CompName != state.dataBranchNodeConnections->CompSets(Count).CName) {
            continue;
        }
        if (ComponentTypeEnum != DataLoopNode::ConnectionObjectType::Undefined) {
            if (ComponentTypeEnum != state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType) {
                continue;
            }
        }
        // Component name matches, component type matches or is undefined
        if (InletNode != undefined) {
            if (state.dataBranchNodeConnections->CompSets(Count).InletNodeName != undefined) {
                if (InletNode != state.dataBranchNodeConnections->CompSets(Count).InletNodeName) {
                    continue;
                }
            } else {
                state.dataBranchNodeConnections->CompSets(Count).InletNodeName = InletNode;
            }
        }
        if (OutletNode != undefined) {
            if (state.dataBranchNodeConnections->CompSets(Count).OutletNodeName != undefined) {
                if (OutletNode != state.dataBranchNodeConnections->CompSets(Count).OutletNodeName) {
                    continue;
                }
            } else {
                state.dataBranchNodeConnections->CompSets(Count).OutletNodeName = OutletNode;
            }
        }
        //  See if something undefined and set here
        if (state.dataBranchNodeConnections->CompSets(Count).ParentObjectType == DataLoopNode::ConnectionObjectType::Undefined &&
            state.dataBranchNodeConnections->CompSets(Count).ParentCName == undefined) {
            // Assume this is a further definition for this compset
            state.dataBranchNodeConnections->CompSets(Count).ParentObjectType = ParentTypeEnum;
            state.dataBranchNodeConnections->CompSets(Count).ParentCName = ParentName;
            if (!Description.empty()) {
                state.dataBranchNodeConnections->CompSets(Count).Description = Description;
            }
            Found = Count;
            break;
        }
    }
    if (Found == 0) {
        for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
            Found = 0;
            // Test if inlet node has been used before as an inlet node
            // If the matching node name does not belong to the parent object, then error
            // For example a fan may share the same inlet node as the furnace object which is its parent
            if (InletNode != state.dataBranchNodeConnections->CompSets(Count).InletNodeName) {
                continue;
                // If parent type is undefined then no error
            } else if ((ParentTypeEnum == DataLoopNode::ConnectionObjectType::Undefined) ||
                       (state.dataBranchNodeConnections->CompSets(Count).ParentObjectType == DataLoopNode::ConnectionObjectType::Undefined)) {
                // If node name is undefined then no error
            } else if (InletNode != undefined) {
                // If the matching node name does not belong to the parent or child object, then error
                // For example a fan may share the same inlet node as the furnace object which is its parent
                if ((ParentTypeEnum == state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType) &&
                    (ParentName == state.dataBranchNodeConnections->CompSets(Count).CName)) {
                    // OK - The duplicate inlet node belongs to this component's parent
                } else if ((ComponentTypeEnum == state.dataBranchNodeConnections->CompSets(Count).ParentObjectType) &&
                           (CompName == state.dataBranchNodeConnections->CompSets(Count).ParentCName)) {
                    // OK - The duplicate inlet node belongs to a child of this component
                } else {
                    // Due to possibility of grandparents or more, if the matching node name
                    // belongs to a component that appears as a parent, then OK
                    int Found2 = 0;
                    for (int Count2 = 1; Count2 <= state.dataBranchNodeConnections->NumCompSets; ++Count2) {
                        if ((state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType ==
                             state.dataBranchNodeConnections->CompSets(Count2).ParentObjectType) &&
                            (state.dataBranchNodeConnections->CompSets(Count).CName ==
                             state.dataBranchNodeConnections->CompSets(Count2).ParentCName)) {
                            Found2 = 1;
                        }
                        if ((ComponentTypeEnum == state.dataBranchNodeConnections->CompSets(Count2).ParentObjectType) &&
                            (CompName == state.dataBranchNodeConnections->CompSets(Count2).ParentCName)) {
                            Found2 = 1;
                        }
                    }
                    if (Found2 == 0) {
                        ShowWarningError(state, format("Node used as an inlet more than once: {}", InletNode));
                        ShowContinueError(
                            state,
                            format("  Used by: {}, name={}",
                                   ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ParentObjectType)],
                                   state.dataBranchNodeConnections->CompSets(Count).ParentCName));
                        ShowContinueError(
                            state,
                            format("  as inlet for: {}, name={}",
                                   ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)],
                                   state.dataBranchNodeConnections->CompSets(Count).CName));
                        ShowContinueError(state, format("{}{}{}", "  and  by     : ", ParentTypeUC + ", name=", ParentName));
                        ShowContinueError(state, format("{}{}{}", "  as inlet for: ", CompTypeUC + ", name=", CompName));
                    }
                }
            }
            // Test if outlet node has been used before as an outlet node
            // If the matching node name does not belong to the parent or child object, then error
            // For example a fan may share the same outlet node as the furnace object which is its parent
            if (OutletNode != state.dataBranchNodeConnections->CompSets(Count).OutletNodeName) {
                continue;
                // If parent type is undefined then no error
            } else if ((ParentTypeEnum == DataLoopNode::ConnectionObjectType::Undefined) ||
                       (state.dataBranchNodeConnections->CompSets(Count).ParentObjectType == DataLoopNode::ConnectionObjectType::Undefined)) {
                // If node name is undefined then no error
            } else if (OutletNode != undefined) {
                if ((ParentTypeEnum == state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType) &&
                    (ParentName == state.dataBranchNodeConnections->CompSets(Count).CName)) {
                    // OK - The duplicate outlet node belongs to this component's parent
                } else if ((ComponentTypeEnum == state.dataBranchNodeConnections->CompSets(Count).ParentObjectType) &&
                           (CompName == state.dataBranchNodeConnections->CompSets(Count).ParentCName)) {
                    // OK - The duplicate outlet node belongs to a child of this component
                } else {
                    // Due to possibility of grandparents or more, if the matching node name
                    // belongs to a component that appears as a parent, then OK
                    int Found2 = 0;
                    for (int Count2 = 1; Count2 <= state.dataBranchNodeConnections->NumCompSets; ++Count2) {
                        if ((state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType ==
                             state.dataBranchNodeConnections->CompSets(Count2).ParentObjectType) &&
                            (state.dataBranchNodeConnections->CompSets(Count).CName ==
                             state.dataBranchNodeConnections->CompSets(Count2).ParentCName)) {
                            Found2 = 1;
                        }
                        if ((ComponentTypeEnum == state.dataBranchNodeConnections->CompSets(Count2).ParentObjectType) &&
                            (CompName == state.dataBranchNodeConnections->CompSets(Count2).ParentCName)) {
                            Found2 = 1;
                        }
                    }
                    // This rule is violated by dual duct units, so let it pass
                    if (Found2 == 0) {
                        std::string_view const CType =
                            ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)];
                        if ((!has_prefixi(CType, "AirTerminal:DualDuct:")) && (!has_prefixi(CompTypeUC, "AirTerminal:DualDuct:"))) {
                            ShowWarningError(state, format("Node used as an outlet more than once: {}", OutletNode));
                            ShowContinueError(
                                state,
                                format("  Used by: {}, name={}",
                                       ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ParentObjectType)],
                                       state.dataBranchNodeConnections->CompSets(Count).ParentCName));
                            ShowContinueError(
                                state,
                                format(
                                    "  as outlet for: {}, name={}",
                                    ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)],
                                    state.dataBranchNodeConnections->CompSets(Count).CName));
                            ShowContinueError(state, format("{}{}{}", "  and  by     : ", ParentTypeUC + ", name=", ParentName));
                            ShowContinueError(state, format("{}{}{}", "  as outlet for: ", CompTypeUC + ", name=", CompName));
                        }
                    }
                }
            }
            if (ComponentTypeEnum != state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType &&
                ComponentTypeEnum != DataLoopNode::ConnectionObjectType::Undefined) {
                continue;
            }
            if (CompName != state.dataBranchNodeConnections->CompSets(Count).CName) {
                continue;
            }
            Found = Count;
            break;
        }
    }
    if (Found == 0) {
        state.dataBranchNodeConnections->CompSets.resize(++state.dataBranchNodeConnections->NumCompSets);
        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).ParentObjectType = ParentTypeEnum;

        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).ParentCName = ParentName;
        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).ComponentObjectType = ComponentTypeEnum;
        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).CName = CompName;
        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).InletNodeName =

            Util::makeUPPER(InletNode); // TODO: Fix this....
        state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).OutletNodeName =
            Util::makeUPPER(OutletNode); // TODO: Fix this....

        if (!Description.empty()) {
            state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).Description = Description;
        } else {
            state.dataBranchNodeConnections->CompSets(state.dataBranchNodeConnections->NumCompSets).Description = undefined;
        }
    }
}

void TestInletOutletNodes(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2001

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine tests the branches to see if a duplicate inlet node
    // exists under a different name in the sequence; likewise for outlet.

    Array1D_bool AlreadyNoted;

    // Test component sets created by branches
    AlreadyNoted.dimension(state.dataBranchNodeConnections->NumCompSets, false);
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
        for (int Other = 1; Other <= state.dataBranchNodeConnections->NumCompSets; ++Other) {
            if (Count == Other) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).InletNodeName != state.dataBranchNodeConnections->CompSets(Other).InletNodeName) {
                continue;
            }
            if (AlreadyNoted(Count)) {
                continue;
            }
            //  All other values must match
            if (state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType !=
                    state.dataBranchNodeConnections->CompSets(Other).ComponentObjectType ||
                state.dataBranchNodeConnections->CompSets(Count).CName != state.dataBranchNodeConnections->CompSets(Other).CName ||
                state.dataBranchNodeConnections->CompSets(Count).OutletNodeName != state.dataBranchNodeConnections->CompSets(Other).OutletNodeName) {
                AlreadyNoted(Other) = true;
                ShowWarningError(state,
                                 format("Node used as an inlet more than once: {}", state.dataBranchNodeConnections->CompSets(Count).InletNodeName));
                ShowContinueError(
                    state,
                    format("  Used by: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ParentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Count).ParentCName));
                ShowContinueError(
                    state,
                    format("  as inlet for: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Other).ComponentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Other).CName));
                ShowContinueError(
                    state,
                    format("  and by: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Other).ParentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Other).ParentCName));
                ShowContinueError(
                    state,
                    format("  as inlet for: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Count).CName));
            }
        }
    }

    AlreadyNoted = false;
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
        for (int Other = 1; Other <= state.dataBranchNodeConnections->NumCompSets; ++Other) {
            if (Count == Other) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).OutletNodeName != state.dataBranchNodeConnections->CompSets(Other).OutletNodeName) {
                continue;
            }
            if (AlreadyNoted(Count)) {
                continue;
            }
            //  All other values must match
            if (state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType !=
                    state.dataBranchNodeConnections->CompSets(Other).ComponentObjectType ||
                state.dataBranchNodeConnections->CompSets(Count).CName != state.dataBranchNodeConnections->CompSets(Other).CName ||
                state.dataBranchNodeConnections->CompSets(Count).InletNodeName != state.dataBranchNodeConnections->CompSets(Other).InletNodeName) {
                AlreadyNoted(Other) = true;
                ShowWarningError(
                    state, format("Node used as an outlet more than once: {}", state.dataBranchNodeConnections->CompSets(Count).OutletNodeName));
                ShowContinueError(
                    state,
                    format("  Used by: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ParentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Count).ParentCName));
                ShowContinueError(
                    state,
                    format("  as outlet for: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Other).ComponentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Other).CName));
                ShowContinueError(
                    state,
                    format("  and by: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Other).ParentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Other).ParentCName));
                ShowContinueError(
                    state,
                    format("  as outlet for: {}, name={}",
                           ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)],
                           state.dataBranchNodeConnections->CompSets(Count).CName));
            }
        }
    }

    AlreadyNoted.deallocate();
}

void TestCompSet(EnergyPlusData &state,
                 std::string_view const CompType, // Component Type
                 std::string_view CompName,       // Component Name
                 std::string const &InletNode,    // Inlet Node Name
                 std::string const &OutletNode,   // Outlet Node Name
                 std::string const &Description   // Description of Node Pair (for warning message)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   November 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Register a child component in the CompSets data structure.
    // NOTE:  This function was originally designed to test the stored "Component Sets" to
    // see if there was one of this combination in there.  Thus the name "TestCompSet".
    // However, this was based on a false assumption that input would always be gotten
    // first for the parent object, then for the child object.  But this is often not the
    // case.  Ultimately, the name of this function should be changed or it should be merged
    // into SetUpCompSets.
    // Until then, this function does the following:
    //   a)  Search CompSets for this combination of component type, component name,
    //       inlet node and outlet node.  If component type/name match and the existing
    //       node names are UNDEFINED, this compset is assumed to be a match.
    //   b)  If found, fill in any missing data such as node names or node description
    //   c)  If not found, call SetUpCompSets (with parent type and name UNDEFINED)
    //       to add a new item in the CompSets array

    std::string CompTypeUC = Util::makeUPPER(CompType);

    // TODO: Refactor this away by passing in enums
    DataLoopNode::ConnectionObjectType ComponentTypeEnum =
        static_cast<DataLoopNode::ConnectionObjectType>(getEnumValue(ConnectionObjectTypeNamesUC, CompTypeUC));
    assert(ComponentTypeEnum != DataLoopNode::ConnectionObjectType::Invalid);

    // See if Already there
    int Found = 0;
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
        if ((ComponentTypeEnum != state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType) &&
            (state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType != DataLoopNode::ConnectionObjectType::Undefined)) {
            continue;
        }
        if (CompName != state.dataBranchNodeConnections->CompSets(Count).CName) {
            continue;
        }
        if ((InletNode != state.dataBranchNodeConnections->CompSets(Count).InletNodeName) &&
            (state.dataBranchNodeConnections->CompSets(Count).InletNodeName != undefined) && (InletNode != undefined)) {
            continue;
        }
        if ((OutletNode != state.dataBranchNodeConnections->CompSets(Count).OutletNodeName) &&
            (state.dataBranchNodeConnections->CompSets(Count).OutletNodeName != undefined) && (OutletNode != undefined)) {
            continue;
        }

        Found = Count;
        break;
    }

    if (Found == 0) {
        SetUpCompSets(state, undefined, undefined, CompType, CompName, InletNode, OutletNode, Description);
    } else {
        // Fill in node names and component type for previously undefined values:
        //   If the parent object did not specify a component type or inlet or outlet node, then that value
        //   is UNDEFINED in CompSets.  When a component calls TestCompSet, the comp type and inlet and
        //   outlet nodes are known, so they can be filled in for future reference.
        if (state.dataBranchNodeConnections->CompSets(Found).ComponentObjectType == DataLoopNode::ConnectionObjectType::Undefined) {
            state.dataBranchNodeConnections->CompSets(Found).ComponentObjectType = ComponentTypeEnum;
        }
        if (state.dataBranchNodeConnections->CompSets(Found).InletNodeName == undefined) {
            state.dataBranchNodeConnections->CompSets(Found).InletNodeName = InletNode;
        }
        if (state.dataBranchNodeConnections->CompSets(Found).OutletNodeName == undefined) {
            state.dataBranchNodeConnections->CompSets(Found).OutletNodeName = OutletNode;
        }
        if (state.dataBranchNodeConnections->CompSets(Found).Description == undefined) {
            state.dataBranchNodeConnections->CompSets(Found).Description = Description;
        }
    }
}

void TestCompSetInletOutletNodes(EnergyPlusData &state, bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine tests the comp sets to see if a duplicate comp name
    // exists under a different set of inlet/outlet nodes.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_bool AlreadyNoted;

    // Test component sets created by branches
    AlreadyNoted.dimension(state.dataBranchNodeConnections->NumCompSets, false);
    for (int Count = 1; Count <= state.dataBranchNodeConnections->NumCompSets; ++Count) {
        for (int Other = 1; Other <= state.dataBranchNodeConnections->NumCompSets; ++Other) {
            if (Count == Other) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType ==
                DataLoopNode::ConnectionObjectType ::SolarCollectorUnglazedTranspired) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType !=
                    state.dataBranchNodeConnections->CompSets(Other).ComponentObjectType ||
                state.dataBranchNodeConnections->CompSets(Count).CName != state.dataBranchNodeConnections->CompSets(Other).CName) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).Description != state.dataBranchNodeConnections->CompSets(Other).Description) {
                if (state.dataBranchNodeConnections->CompSets(Count).Description != undefined &&
                    state.dataBranchNodeConnections->CompSets(Other).Description != undefined) {
                    continue;
                }
            }
            if (state.dataBranchNodeConnections->CompSets(Count).InletNodeName == state.dataBranchNodeConnections->CompSets(Other).InletNodeName) {
                continue;
            }
            if (state.dataBranchNodeConnections->CompSets(Count).OutletNodeName == state.dataBranchNodeConnections->CompSets(Other).OutletNodeName) {
                continue;
            }
            if (AlreadyNoted(Count)) {
                continue;
            }
            //  All other values must match
            AlreadyNoted(Other) = true;
            ShowSevereError(state, "Same component name and type has differing Node Names.");
            ShowContinueError(
                state,
                format("  Component: {}, name={}",
                       ConnectionObjectTypeNames[static_cast<int>(state.dataBranchNodeConnections->CompSets(Count).ComponentObjectType)],
                       state.dataBranchNodeConnections->CompSets(Count).CName));
            ShowContinueError(state,
                              format("   Nodes, inlet: {}, outlet: {}",
                                     state.dataBranchNodeConnections->CompSets(Count).InletNodeName,
                                     state.dataBranchNodeConnections->CompSets(Count).OutletNodeName));
            ShowContinueError(state,
                              format(" & Nodes, inlet: {}, outlet: {}",
                                     state.dataBranchNodeConnections->CompSets(Other).InletNodeName,
                                     state.dataBranchNodeConnections->CompSets(Other).OutletNodeName));
            ShowContinueError(state,
                              format("   Node Types:   {} & {}",
                                     state.dataBranchNodeConnections->CompSets(Count).Description,
                                     state.dataBranchNodeConnections->CompSets(Other).Description));
            ErrorsFound = true;
        }
    }

    AlreadyNoted.deallocate();
}

void GetNodeConnectionType(EnergyPlusData &state, int const NodeNumber, EPVector<DataLoopNode::ConnectionType> &NodeConnectType, bool &errFlag)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   Jan 2007

    // PURPOSE OF THIS FUNCTION:
    // This function provides a connection type with given node number

    Array1D_int ListArray;
    Array1D_string ConnectionTypes(15);

    for (int nodetype = 1; nodetype < static_cast<int>(ConnectionType::Num); ++nodetype) {
        ConnectionTypes(nodetype) = ConnectionTypeNames[nodetype];
    }

    if (allocated(NodeConnectType)) {
        NodeConnectType.deallocate();
    }

    int NumInList;
    FindAllNodeNumbersInList(
        NodeNumber, state.dataBranchNodeConnections->NodeConnections, state.dataBranchNodeConnections->NumOfNodeConnections, NumInList, ListArray);

    NodeConnectType.allocate(NumInList);

    if (NumInList > 0) {
        for (int NodeConnectIndex = 1; NodeConnectIndex <= NumInList; ++NodeConnectIndex) {
            NodeConnectType(NodeConnectIndex) = state.dataBranchNodeConnections->NodeConnections(ListArray(NodeConnectIndex)).ConnectionType;
        }
    } else {
        if (NodeNumber > 0) {
            ShowWarningError(state, format("Node not found = {}.", state.dataLoopNodes->NodeID(NodeNumber)));
        } else {
            ShowWarningError(state, "Invalid node number passed = 0.");
        }
        errFlag = true;
    }
}

void FindAllNodeNumbersInList(int const WhichNumber,
                              EPVector<DataBranchNodeConnections::NodeConnectionDef> const &NodeConnections,
                              int const NumItems,
                              int &CountOfItems,            // Number of items found
                              Array1D_int &AllNumbersInList // Index array to all numbers found
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad
    //       DATE WRITTEN   January 2007

    // PURPOSE OF THIS FUNCTION:
    // This function looks up a number(integer) in a similar list of
    // items and returns the index of the item in the list, if
    // found.

    CountOfItems = 0;

    if (allocated(AllNumbersInList)) {
        AllNumbersInList.deallocate();
    }

    for (int Count = 1; Count <= NumItems; ++Count) {
        if (WhichNumber == NodeConnections(Count).NodeNumber) {
            ++CountOfItems;
        }
    }

    if (CountOfItems > 0) {

        AllNumbersInList.dimension(CountOfItems, 0);
        CountOfItems = 0;

        for (int Count = 1; Count <= NumItems; ++Count) {
            if (WhichNumber == NodeConnections(Count).NodeNumber) {
                ++CountOfItems;
                AllNumbersInList(CountOfItems) = Count;
            }
        }
    }
}

} // namespace EnergyPlus::BranchNodeConnections
