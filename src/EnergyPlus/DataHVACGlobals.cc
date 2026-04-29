// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

// EnergyPlus Headers
#include <EnergyPlus/DataHVACGlobals.hh>

namespace EnergyPlus {

namespace HVAC {

    // MODULE INFORMATION:
    //       MODIFIED       Craig Wray 22Aug2010 Added Fan Component Model

    // PURPOSE OF THIS MODULE:
    // This data-only module is a repository for HVAC variables which are considered
    // to be "global" in nature in EnergyPlus.
    constexpr std::array<std::string_view, (int)FanType::Num> fanTypeNames = {
        "Fan:ConstantVolume", "Fan:VariableVolume", "Fan:OnOff", "Fan:ZoneExhaust", "Fan:ComponentModel", "Fan:SystemModel"};

    constexpr std::array<std::string_view, (int)FanType::Num> fanTypeNamesUC = {
        "FAN:CONSTANTVOLUME", "FAN:VARIABLEVOLUME", "FAN:ONOFF", "FAN:ZONEEXHAUST", "FAN:COMPONENTMODEL", "FAN:SYSTEMMODEL"};

    constexpr std::array<std::string_view, (int)UnitarySysType::Num> unitarySysTypeNames = {"AirLoopHVAC:Unitary:Furnace:HeatOnly",
                                                                                            "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                                                                            "AirLoopHVAC:UnitaryHeatOnly",
                                                                                            "AirLoopHVAC:UnitaryHeatCool",
                                                                                            "AirLoopHVAC:UnitaryHeatPump:AirToAir",
                                                                                            "AirLoopHVAC:UnitaryHeatPump:WaterToAir",
                                                                                            "AirLoopHVAC:UnitarySystem"};

    constexpr std::array<std::string_view, (int)UnitarySysType::Num> unitarySysTypeNamesUC = {"AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY",
                                                                                              "AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL",
                                                                                              "AIRLOOPHVAC:UNITARYHEATONLY",
                                                                                              "AIRLOOPHVAC:UNITARYHEATCOOL",
                                                                                              "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR",
                                                                                              "AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR",
                                                                                              "AIRLOOPHVAC:UNITARYSYSTEM"};

    constexpr std::array<std::string_view, (int)WaterFlow::Num> waterFlowNames = {"Cycling", "Constant", "ConstantOnDemand"};

    constexpr std::array<std::string_view, (int)WaterFlow::Num> waterFlowNamesUC = {"CYCLING", "CONSTANT", "CONSTANTONDEMAND"};

    constexpr std::array<std::string_view, (int)OATType::Num> oatTypeNames = {"WetBulbTemperature", "DryBulbTemperature"};
    constexpr std::array<std::string_view, (int)OATType::Num> oatTypeNamesUC = {"WETBULBTEMPERATURE", "DRYBULBTEMPERATURE"};

    constexpr std::array<std::string_view, (int)MixerType::Num> mixerTypeLocNames = {"InletSide", "SupplySide"};
    constexpr std::array<std::string_view, (int)MixerType::Num> mixerTypeLocNamesUC = {"INLETSIDE", "SUPPLYSIDE"};

    constexpr std::array<std::string_view, (int)CoilType::Num> coilTypeNames = {"Coil:Cooling:DX:SingleSpeed",
                                                                                "Coil:Heating:DX:SingleSpeed",
                                                                                "Coil:Cooling:DX:TwoSpeed",
                                                                                "CoilSystem:Cooling:DX:HeatExchangerAssisted",
                                                                                "Coil:Cooling:DX:TwoStageWithHumidityControlMode",
                                                                                "Coil:WaterHeating:AirToWaterHeatPump:Pumped",
                                                                                "Coil:WaterHeating:AirToWaterHeatPump:Wrapped",
                                                                                "Coil:Cooling:DX:MultiSpeed",
                                                                                "Coil:Heating:DX:MultiSpeed",
                                                                                "Coil:Heating:Fuel",
                                                                                "Coil:Heating:Gas:MultiStage",
                                                                                "Coil:Heating:Electric",
                                                                                "Coil:Heating:Electric:MultiStage",
                                                                                "Coil:Heating:Desuperheater",
                                                                                "Coil:Cooling:Water",
                                                                                "Coil:Cooling:Water:DetailedGeometry",
                                                                                "Coil:Heating:Water",
                                                                                "Coil:Heating:Steam",
                                                                                "CoilSystem:Cooling:Water:HeatExchangerAssisted",
                                                                                "Coil:Cooling:WaterToAirHeatPump:ParameterEstimation",
                                                                                "Coil:Heating:WaterToAirHeatPump:ParameterEstimation",
                                                                                "Coil:Cooling:WaterToAirHeatPump:EquationFit",
                                                                                "Coil:Heating:WaterToAirHeatPump:EquationFit",
                                                                                "Coil:Cooling:DX:VariableRefrigerantFlow",
                                                                                "Coil:Heating:DX:VariableRefrigerantFlow",
                                                                                "Coil:UserDefined",
                                                                                "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                                                                "Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit",
                                                                                "Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit",
                                                                                "Coil:Cooling:DX:VariableSpeed",
                                                                                "Coil:Heating:DX:VariableSpeed",
                                                                                "Coil:WaterHeating:AirToWaterHeatPump:VariableSpeed",
                                                                                "Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl",
                                                                                "Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl",
                                                                                "Coil:Cooling:DX",
                                                                                "Coil:Cooling:DX:SubcoolReheat",
                                                                                "Coil:Cooling:DX:CurveFit:Speed"
#ifdef GET_OUT
                                                                                ,
                                                                                "CoilSystem:IntegratedHeatPump:AirSource",
                                                                                "CoilSystem:Cooling:DX",
                                                                                "CoilSystem:Heating:DX"
#endif // GET_OUT
    };

    constexpr std::array<std::string_view, (int)CoilType::Num> coilTypeNamesUC = {"COIL:COOLING:DX:SINGLESPEED",
                                                                                  "COIL:HEATING:DX:SINGLESPEED",
                                                                                  "COIL:COOLING:DX:TWOSPEED",
                                                                                  "COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED",
                                                                                  "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE",
                                                                                  "COIL:WATERHEATING:AIRTOWATERHEATPUMP:PUMPED",
                                                                                  "COIL:WATERHEATING:AIRTOWATERHEATPUMP:WRAPPED",
                                                                                  "COIL:COOLING:DX:MULTISPEED",
                                                                                  "COIL:HEATING:DX:MULTISPEED",
                                                                                  "COIL:HEATING:FUEL",
                                                                                  "COIL:HEATING:GAS:MULTISTAGE",
                                                                                  "COIL:HEATING:ELECTRIC",
                                                                                  "COIL:HEATING:ELECTRIC:MULTISTAGE",
                                                                                  "COIL:HEATING:DESUPERHEATER",
                                                                                  "COIL:COOLING:WATER",
                                                                                  "COIL:COOLING:WATER:DETAILEDGEOMETRY",
                                                                                  "COIL:HEATING:WATER",
                                                                                  "COIL:HEATING:STEAM",
                                                                                  "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
                                                                                  "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
                                                                                  "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
                                                                                  "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                                                  "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                                                  "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW",
                                                                                  "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW",
                                                                                  "COIL:USERDEFINED",
                                                                                  "COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE",
                                                                                  "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
                                                                                  "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
                                                                                  "COIL:COOLING:DX:VARIABLESPEED",
                                                                                  "COIL:HEATING:DX:VARIABLESPEED",
                                                                                  "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED",
                                                                                  "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
                                                                                  "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
                                                                                  "COIL:COOLING:DX",
                                                                                  "COIL:COOLING:DX:SUBCOOLREHEAT",
                                                                                  "COIL:COOLING:DX:CURVEFIT:SPEED"
#ifdef GET_OUT
                                                                                  ,
                                                                                  "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE",
                                                                                  "COILSYSTEM:COOLING:DX",
                                                                                  "COILSYSTEM:HEATING:DX"
#endif // GET_OUT
    };

    constexpr std::array<bool, (int)CoilType::Num> coilTypeIsCooling = {
        true,  // DXCoolingSingleSpeed,
        false, // DXHeatingEmpirical,
        true,  // DXCoolingTwoSpeed,
        true,  // DXCoolingHXAssisted,
        true,  // DXCoolingTwoStageWHumControl,
        false, // DXHeatPumpWaterHeaterPumped,
        false, // DXHeatPumpWaterHeaterWrapped,
        true,  // DXMultiSpeedCooling,
        false, // DXMultiSpeedHeating,
        false, // HeatingGasOrOtherFuel,
        false, // HeatingGasMultiStage,
        false, // HeatingElectric,
        false, // HeatingElectricMultiStage,
        false, // HeatingDesuperheater,
        true,  // CoolingWater,
        true,  // CoolingWaterDetailed,
        false, // HeatingWater,
        false, // HeatingSteam,
        true,  // WaterCoolingHXAssisted,
        true,  // CoolingWaterToAirHP,
        false, // HeatingWaterToAirHP,
        true,  // CoolingWaterToAirHPSimple,
        false, // HeatingWaterToAirHPSimple,
        true,  // VRFCooling,
        false, // VRFHeating,
        false, // UserDefined,
        true,  // DXPackagedThermalStorageCooling,
        true,  // CoolingWaterToAirHPVSEquationFit,
        false, // HeatingWaterToAirHPVSEquationFit,
        true,  // CoolingAirToAirVariableSpeed,
        false, // HeatingAirToAirVariableSpeed,
        false, // DXHeatPumpWaterHeaterVariableSpeed,
        true,  // VRFFluidTCtrlCooling,
        false, // VRFFluidTCtrlHeating,
        true,  // CoolingDX,
        true,  // DXSubcoolReheat,
        true,  // CoolingDXCurveFit,
#ifdef GET_OUT
        true, // IHPAirSource,
        true, // CoolingSystemDX,
        false // HeatingSystemDX
#endif        // GET_OUT
    };

    constexpr std::array<bool, (int)CoilType::Num> coilTypeIsHeating = {
        false, // CoolingDXSingleSpeed,
        true,  // HeatingDXSingleSpeed,
        false, // CoolingDXTwoSpeed,
        false, // DXCoolingHXAssisted,
        false, // DXCoolingTwoStageWHumControl,
        true,  // DXHeatPumpWaterHeaterPumped,
        true,  // DXHeatPumpWaterHeaterWrapped,
        false, // CoolingDXMultiSpeed,
        true,  // HeatingDXMultiSpeed,
        true,  // HeatingGasOrOtherFuel,
        true,  // HeatingGasMultiStage,
        true,  // HeatingElectric,
        true,  // HeatingElectricMultiStage,
        true,  // HeatingDesuperheater,
        false, // CoolingWater,
        false, // CoolingWaterDetailed,
        true,  // HeatingWater,
        true,  // HeatingSteam,
        false, // WaterCoolingHXAssisted,
        false, // CoolingWaterToAirHP,
        true,  // HeatingWaterToAirHP,
        false, // CoolingWaterToAirHPSimple,
        true,  // HeatingWaterToAirHPSimple,
        false, // CoolingVRF,
        true,  // HeatingVRF,
        false, // UserDefined,
        false, // CoolingDXPackagedThermalStorage,
        false, // CoolingWAHPVariableSpeedEquationFit,
        true,  // HeatingWAHPVariableSpeedEquationFit,
        false, // CoolingDXVariableSpeed,
        true,  // HeatingDXVariableSpeed,
        true,  // WaterHeatingAWHPVariableSpeed,
        false, // CoolingVRFFluidTCtrl,
        true,  // HeatingVRFFluidTCtrl,
        false, // CoolingDX,
        false, // DXSubcoolReheat,
        false, // CoolingDXCurveFit
#ifdef GET_OUT
        true,  // IHP air source
        false, // CoolingSystemDX
        true   // HeatingSystemDX
#endif         // GET_OUT
    };

    constexpr std::array<bool, (int)CoilType::Num> coilTypeIsHeatPump = {
        false, // DXCoolingSingleSpeed,
        true,  // HeatingDXSingleSpeed,
        false, // DXCoolingTwoSpeed,
        false, // DXCoolingHXAssisted,
        false, // DXCoolingTwoStageWHumControl,
        false, // HeatPumpWaterHeaterDXPumped,
        false, // HeatPumpWaterHeaterDXWrapped,
        false, // CoolingDXMultiSpeed,
        true,  // HeatingDXMultiSpeed,
        false, // HeatingGasOrOtherFuel,
        false, // HeatingGasMultiStage,
        false, // HeatingElectric,
        false, // HeatingElectricMultiStage,
        false, // HeatingDesuperheater,
        false, // CoolingWater,
        false, // CoolingWaterDetailed,
        false, // HeatingWater,
        false, // HeatingSteam,
        false, // CoolingWaterHXAssisted,
        false, // CoolingWAHP,
        true,  // HeatingWAHP,
        false, // CoolingWAHPSimple,
        true,  // HeatingWAHPSimple,
        false, // CoolingVRF,
        false, // HeatingVRF,
        false, // UserDefined,
        false, // CoolingDXPackagedThermalStorage,
        false, // CoolingWAHPVariableSpeedEquationFit,
        true,  // HeatingWAHPVariableSpeedEquationFit,
        false, // CoolingDXVariableSpeed,
        true,  // HeatingDXVariableSpeed,
        false, // WaterHeatingAWHPVariableSpeed,
        false, // CoolingVRFFluidTCtrl,
        false, // HeatingVRFFluidTCtrl,
        false, // CoolingDX,
        false, // DXSubcoolReheat,
        false, // DXCurveFitSpeed
#ifdef GET_OUT
        true, // IHPAirSource
        true, // CoolingSystemDX
        true, // HeatingSystemDX
#endif        // GET_OUT
    };

    constexpr std::array<std::string_view, (int)HXType::Num> hxTypeNames = {
        "HeatExchanger:AirToAir:FlatPlate", "HeatExchanger:AirToAir:SensibleAndLatent", "HeatExchanger:Desiccant:BalancedFlow"};

    constexpr std::array<std::string_view, (int)HXType::Num> hxTypeNamesUC = {
        "HEATEXCHANGER:AIRTOAIR:FLATPLATE", "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT", "HEATEXCHANGER:DESICCANT:BALANCEDFLOW"};

    constexpr std::array<std::string_view, (int)MixerType::Num> mixerTypeNames = {"AirTerminal:SingleDuct:InletSideMixer",
                                                                                  "AirTerminal:SingleDuct:SupplySideMixer"};

    constexpr std::array<std::string_view, (int)MixerType::Num> mixerTypeNamesUC = {"AIRTERMINAL:SINGLEDUCT:INLETSIDEMIXER",
                                                                                    "AIRTERMINAL:SINGLEDUCT:SUPPLYSIDEMIXER"};

#ifdef GET_OUT
    constexpr std::array<std::string_view, (int)ZoneEquipType::Num> zoneEquipTypeNamesUC = {"DUMMY",
                                                                                            "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW",
                                                                                            "ZONEHVAC:ENERGYRECOVERYVENTILATOR",
                                                                                            "ZONEHVAC:FOURPIPEFANCOIL",
                                                                                            "ZONEHVAC:OUTDOORAIRUNIT",
                                                                                            "ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER",
                                                                                            "ZONEHVAC:PACKAGEDTERMINALHEATPUMP",
                                                                                            "ZONEHVAC:UNITHEATER",
                                                                                            "ZONEHVAC:UNITVENTILATOR",
                                                                                            "ZONEHVAC:VENTILATEDSLAB",
                                                                                            "ZONEHVAC:WATERTOAIRHEATPUMP",
                                                                                            "ZONEHVAC:WINDOWAIRCONDITIONER",
                                                                                            "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:ELECTRIC",
                                                                                            "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER",
                                                                                            "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM",
                                                                                            "ZONEHVAC:BASEBOARD:CONVECTIVE:ELECTRIC",
                                                                                            "ZONEHVAC:BASEBOARD:CONVECTIVE:WATER",
                                                                                            "ZONEHVAC:HIGHTEMPERATURERADIANT",
                                                                                            "ZONEHVAC:DEHUMIDIFIER:DX",
                                                                                            "ZONEHVAC:IDEALLOADSAIRSYSTEM",
                                                                                            "ZONEHVAC:REFRIGERATIONCHILLERSET",
                                                                                            "ZONEHVAC:HYBRIDUNITARYHVAC",
                                                                                            "FAN:ZONEEXHAUST",
                                                                                            "WATERHEATER:HEATPUMP",
                                                                                            "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME",
                                                                                            "AIRTERMINAL:DUALDUCT:VAV",
                                                                                            "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION",
                                                                                            "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN",
                                                                                            "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT",
                                                                                            "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM",
                                                                                            "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR",
                                                                                            "AIRLOOPHVACRETURNAIR"};

    constexpr std::array<std::string_view, (int)ZoneEquipType::Num> zoneEquipTypeNames = {"DUMMY",
                                                                                          "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow",
                                                                                          "ZoneHVAC:EnergyRecoveryVentilator",
                                                                                          "ZoneHVAC:FourPipeFanCoil",
                                                                                          "ZoneHVAC:OutdoorAirUnit",
                                                                                          "ZoneHVAC:PackagedTerminalAirConditioner",
                                                                                          "ZoneHVAC:PackagedTerminalHeatPump",
                                                                                          "ZoneHVAC:UnitHeater",
                                                                                          "ZoneHVAC:UnitVentilator",
                                                                                          "ZoneHVAC:VentilatedSlab",
                                                                                          "ZoneHVAC:WaterToAirHeatPump",
                                                                                          "ZoneHVAC:WindowAirConditioner",
                                                                                          "ZoneHVAC:Baseboard:RadiantConvective:Electric",
                                                                                          "ZoneHVAC:Baseboard:RadiantConvective:Water",
                                                                                          "ZoneHVAC:Baseboard:RadiantConvective:Steam",
                                                                                          "ZoneHVAC:Baseboard:Convective:Electric",
                                                                                          "ZoneHVAC:Baseboard:Convective:Water",
                                                                                          "ZoneHVAC:HighTemperatureRadiant",
                                                                                          "ZoneHVAC:Dehumidifier:DX",
                                                                                          "ZoneHVAC:IdealLoadsAirSystem",
                                                                                          "ZoneHVAC:RefrigerationChillerSet",
                                                                                          "ZoneHVAC:HybridUnitaryHVAC",
                                                                                          "Fan:ZoneExhaust",
                                                                                          "WaterHeater:HeatPump",
                                                                                          "AirTerminal:DualDuct:ConstantVolume",
                                                                                          "AirTerminal:DualDuct:VAV",
                                                                                          "AirTerminal:SingleDuct:ConstantVolume:Reheat",
                                                                                          "AirTerminal:SingleDuct:ConstantVolume:NoReheat",
                                                                                          "AirTerminal:SingleDuct:VAV:Reheat",
                                                                                          "AirTerminal:SingleDuct:VAV:NoReheat",
                                                                                          "AirTerminal:SingleDuct:SeriesPIU:Reheat",
                                                                                          "AirTerminal:SingleDuct:ParallelPIU:Reheat",
                                                                                          "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction",
                                                                                          "AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan",
                                                                                          "AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat",
                                                                                          "AirTerminal:SingleDuct:VAV:HeatAndCool:NoReheat",
                                                                                          "AirTerminal:SingleDuct:ConstantVolume:CooledBeam",
                                                                                          "AirTerminal:DualDuct:VAV:OutdoorAir",
                                                                                          "AirLoopHVACReturnAir"};
#endif // GET_OUT
} // namespace HVAC

} // namespace EnergyPlus
