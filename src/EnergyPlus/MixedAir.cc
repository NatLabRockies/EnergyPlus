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

// C++ Headers
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/Humidifiers.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PhotovoltaicThermalCollectors.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/TranspiredCollector.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UserDefinedComponents.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus::MixedAir {

// Module containing the routines dealing with the mixed air portion
// of the HVAC air loop.

// MODULE INFORMATION:
//       AUTHOR         Fred Buhl
//       DATE WRITTEN   October 1998
//       MODIFIED       Shirey/Raustad FSEC, June/Aug 2003, Jan 2004
//                      Lawrie, March 2006 - Module order (per template)
//                      Craig Wray 22Aug2010 - Added Fan ComponentModel
//                      Chandan Sharma, FSEC, 25Aug 2011 - Added ProportionalControl
//                           to enhance CO2 based DCV control
//                      Feb 2013 Bereket Nigusse, FSEC
//                        Added DX Coil Model For 100% OA systems
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// simulate the mixed air portion of the EPlus air loop.

// METHODOLOGY EMPLOYED:
// An algorithmic controller will be employed - there is no attempt to
// simulate real controllers for the economizer. The mixed air controller
// will sense various node conditions and set some node flow rates.  Mixed
// air components will operate with predetermined flow rates.

// Using/Aliasing
using namespace DataLoopNode;
using namespace DataAirLoop;
using namespace DataEnvironment;
using namespace DataSizing;
using namespace FaultsManager;

constexpr std::array<std::string_view, static_cast<int>(ControllerKind::Num)> ControllerKindNamesUC{"CONTROLLER:WATERCOIL", "CONTROLLER:OUTDOORAIR"};

constexpr std::array<std::string_view, static_cast<int>(MixedAirControllerType::Num)> MixedAirControllerTypeNames{
    "Controller:OutdoorAir", "ZoneHVAC:EnergyRecoveryVentilator:Controller"};

constexpr std::array<std::string_view, static_cast<int>(CMO::Num)> CurrentModuleObjects{"None",
                                                                                        "AirLoopHVAC:OutdoorAirSystem",
                                                                                        "AirLoopHVAC:OutdoorAirSystem:EquipmentList",
                                                                                        "AirLoopHVAC:ControllerList",
                                                                                        "AvailabilityManagerAssignmentList",
                                                                                        "Controller:OutdoorAir",
                                                                                        "ZoneHVAC:EnergyRecoveryVentilator:Controller",
                                                                                        "Controller:MechanicalVentilation",
                                                                                        "OutdoorAir:Mixer"};

constexpr std::array<std::string_view, static_cast<int>(DataSizing::SysOAMethod::Num)> SOAMNamesUC{"ZONESUM",
                                                                                                   "STANDARD62.1VENTILATIONRATEPROCEDURE",
                                                                                                   "INDOORAIRQUALITYPROCEDURE",
                                                                                                   "PROPORTIONALCONTROLBASEDONOCCUPANCYSCHEDULE",
                                                                                                   "INDOORAIRQUALITYPROCEDUREGENERICCONTAMINANT",
                                                                                                   "INDOORAIRQUALITYPROCEDURECOMBINED",
                                                                                                   "PROPORTIONALCONTROLBASEDONDESIGNOCCUPANCY",
                                                                                                   "PROPORTIONALCONTROLBASEDONDESIGNOARATE",
                                                                                                   "STANDARD62.1SIMPLIFIEDPROCEDURE",
                                                                                                   "STANDARD62.1VENTILATIONRATEPROCEDUREWITHLIMIT"};

constexpr std::array<std::string_view, static_cast<int>(SimAirServingZones::CompType::Num)> CompTypeNamesUC{
    "OUTDOORAIR:MIXER",
    "FAN:CONSTANTVOLUME",
    "FAN:VARIABLEVOLUME",
    "COIL:COOLING:WATER",
    "COIL:HEATING:WATER",
    "COIL:HEATING:STEAM",
    "COIL:COOLING:WATER:DETAILEDGEOMETRY",
    "COIL:HEATING:ELECTRIC",
    "COIL:HEATING:FUEL",
    "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
    "COIL:HEATING:DESUPERHEATER",
    "COILSYSTEM:COOLING:DX",
    "HEATEXCHANGER:AIRTOAIR:FLATPLATE",
    "DEHUMIDIFIER:DESICCANT:NOFANS",
    "SOLARCOLLECTOR:UNGLAZEDTRANSPIRED",
    "EVAPORATIVECOOLER:DIRECT:CELDEKPAD",
    "AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY",
    "AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL",
    "HUMIDIFIER:STEAM:ELECTRIC",
    "DUCT",
    "AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS",
    "AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED",
    "FAN:COMPONENTMODEL",
    "COILSYSTEM:HEATING:DX",
    "COIL:USERDEFINED",
    "FAN:SYSTEMMODEL",
    "AIRLOOPHVAC:UNITARYSYSTEM",
    "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW",
    "SOLARCOLLECTOR:FLATPLATE:PHOTOVOLTAICTHERMAL",
    "COILSYSTEM:COOLING:WATER"};

static constexpr std::array<std::string_view, static_cast<int>(DataSizing::SysOAMethod::Num)> printSysOAMethod{
    "ZoneSum,",
    "Standard62.1VentilationRateProcedure,",
    "IndoorAirQualityProcedure,",
    "ProportionalControlBasedOnOccupancySchedule,",
    "IndoorAirQualityGenericContaminant,",
    "IndoorAirQualityProcedureCombined,",
    "ProportionalControlBasedOnDesignOccupancy,",
    "ProportionalControlBasedOnDesignOARate,",
    "Standard62.1SimplifiedProcedure,",
    "Standard62.1VentilationRateProcedureWithLimit,"};

Real64 OAGetFlowRate(EnergyPlusData &state, int OAPtr)
{
    Real64 FlowRate(0);
    if ((OAPtr > 0) && (OAPtr <= state.dataMixedAir->NumOAControllers) && (state.dataEnvrn->StdRhoAir != 0)) {
        FlowRate = state.dataMixedAir->OAController(OAPtr).OAMassFlow / state.dataEnvrn->StdRhoAir;
    }
    return FlowRate;
}
Real64 OAGetMinFlowRate(EnergyPlusData &state, int OAPtr)
{
    Real64 MinFlowRate(0);
    if ((OAPtr > 0) && (OAPtr <= state.dataMixedAir->NumOAControllers)) {
        MinFlowRate = state.dataMixedAir->OAController(OAPtr).MinOA;
    }
    return MinFlowRate;
}
void OASetDemandManagerVentilationState(EnergyPlusData &state, int OAPtr, bool aState)
{
    if ((OAPtr > 0) && (OAPtr <= state.dataMixedAir->NumOAControllers)) {
        state.dataMixedAir->OAController(OAPtr).ManageDemand = aState;
    }
}
void OASetDemandManagerVentilationFlow(EnergyPlusData &state, int OAPtr, Real64 aFlow)
{
    if ((OAPtr > 0) && (OAPtr <= state.dataMixedAir->NumOAControllers)) {
        state.dataMixedAir->OAController(OAPtr).DemandLimitFlowRate = aFlow * state.dataEnvrn->StdRhoAir;
    }
}
int GetOAController(EnergyPlusData &state, std::string const &OAName)
{
    int CurrentOAController(0);
    for (int i = 1; i <= state.dataMixedAir->NumOAControllers; i++) {
        if (OAName == state.dataMixedAir->OAController(i).Name) {
            CurrentOAController = i;
            break;
        }
    }
    return CurrentOAController;
}

void ManageOutsideAirSystem(EnergyPlusData &state, std::string const &OASysName, bool const FirstHVACIteration, int const AirLoopNum, int &OASysNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Manage the outside air system

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    if (OASysNum == 0) {
        OASysNum = Util::FindItemInList(OASysName, state.dataAirLoop->OutsideAirSys);
        if (OASysNum == 0) {
            ShowFatalError(state, format("ManageOutsideAirSystem: AirLoopHVAC:OutdoorAirSystem not found={}", OASysName));
        }
    }

    InitOutsideAirSys(state, OASysNum, AirLoopNum);

    SimOutsideAirSys(state, OASysNum, FirstHVACIteration, AirLoopNum);
}

void SimOASysComponents(EnergyPlusData &state, int const OASysNum, bool const FirstHVACIteration, int const AirLoopNum)
{
    auto &CompType = state.dataMixedAir->CompType;
    auto &CompName = state.dataMixedAir->CompName;
    bool ReSim(false);
    bool Sim(true);
    bool OAHeatCoil(false);
    bool OACoolCoil(false);
    bool OAHX(false);

    for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents; ++CompNum) {
        CompType = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(CompNum);
        CompName = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(CompNum);
        SimOAComponent(state,
                       CompType,
                       CompName,
                       state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum),
                       FirstHVACIteration,
                       state.dataAirLoop->OutsideAirSys(OASysNum).ComponentIndex(CompNum),
                       AirLoopNum,
                       Sim,
                       OASysNum,
                       OAHeatCoil,
                       OACoolCoil,
                       OAHX);
        if (OAHX) {
            ReSim = true;
        }
    }
    // if there were heat exchangers and/or desiccant wheel in the OA path, need to simulate again in reverse
    // order to propagate the air flow and conditions out the relief air path to the relief air exit node
    if (ReSim) {
        for (int CompNum = state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents - 1; CompNum >= 1; --CompNum) {
            CompType = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(CompNum);
            CompName = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(CompNum);
            SimOAComponent(state,
                           CompType,
                           CompName,
                           state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum),
                           FirstHVACIteration,
                           state.dataAirLoop->OutsideAirSys(OASysNum).ComponentIndex(CompNum),
                           AirLoopNum,
                           Sim,
                           OASysNum,
                           OAHeatCoil,
                           OACoolCoil,
                           OAHX);
        }
        // now simulate again propagate current temps back through OA system
        for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents; ++CompNum) {
            CompType = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(CompNum);
            CompName = state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(CompNum);
            SimOAComponent(state,
                           CompType,
                           CompName,
                           state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum),
                           FirstHVACIteration,
                           state.dataAirLoop->OutsideAirSys(OASysNum).ComponentIndex(CompNum),
                           AirLoopNum,
                           Sim,
                           OASysNum,
                           OAHeatCoil,
                           OACoolCoil,
                           OAHX);
        }
    }
}

void SimOutsideAirSys(EnergyPlusData &state, int const OASysNum, bool const FirstHVACIteration, int const AirLoopNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Simulate the controllers and components in the outside air system.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    state.dataSize->CurOASysNum = OASysNum;
    auto &CurrentOASystem(state.dataAirLoop->OutsideAirSys(OASysNum));
    if (state.dataAirLoop->OutsideAirSys(OASysNum).AirLoopDOASNum == -1) {
        SimOAController(state, CurrentOASystem.OAControllerName, CurrentOASystem.OAControllerIndex, FirstHVACIteration, AirLoopNum);
    }
    SimOASysComponents(state, OASysNum, FirstHVACIteration, AirLoopNum);

    if (state.dataMixedAir->MyOneTimeErrorFlag(OASysNum)) {
        bool FatalErrorFlag(false);
        if (CurrentOASystem.NumControllers - CurrentOASystem.NumSimpleControllers > 1) {
            ShowWarningError(
                state,
                format("AirLoopHVAC:OutdoorAirSystem {} has more than 1 outside air controller; only the 1st will be used", CurrentOASystem.Name));
        }
        for (int CompNum = 1; CompNum <= CurrentOASystem.NumComponents; ++CompNum) {
            auto &CompType = CurrentOASystem.ComponentType(CompNum);
            auto &CompName = CurrentOASystem.ComponentName(CompNum);
            if (Util::SameString(CompType, "OutdoorAir:Mixer")) {
                int OAMixerNum = Util::FindItemInList(CompName, state.dataMixedAir->OAMixer);
                int OAControllerNum = CurrentOASystem.OAControllerIndex;
                if (state.dataMixedAir->OAController(OAControllerNum).MixNode != state.dataMixedAir->OAMixer(OAMixerNum).MixNode) {
                    ShowSevereError(
                        state, format("The mixed air node of Controller:OutdoorAir=\"{}\"", state.dataMixedAir->OAController(OAControllerNum).Name));
                    ShowContinueError(state,
                                      format("should be the same node as the mixed air node of OutdoorAir:Mixer=\"{}\".",
                                             state.dataMixedAir->OAMixer(OAMixerNum).Name));
                    ShowContinueError(state,
                                      format("Controller:OutdoorAir mixed air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAController(OAControllerNum).MixNode)));
                    ShowContinueError(state,
                                      format("OutdoorAir:Mixer mixed air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OAMixerNum).MixNode)));
                    FatalErrorFlag = true;
                }
                if (state.dataMixedAir->OAController(OAControllerNum).RelNode != state.dataMixedAir->OAMixer(OAMixerNum).RelNode) {
                    ShowSevereError(
                        state, format("The relief air node of Controller:OutdoorAir=\"{}\"", state.dataMixedAir->OAController(OAControllerNum).Name));
                    ShowContinueError(state,
                                      format("should be the same node as the relief air node of OutdoorAir:Mixer=\"{}\".",
                                             state.dataMixedAir->OAMixer(OAMixerNum).Name));
                    ShowContinueError(state,
                                      format("Controller:OutdoorAir relief air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAController(OAControllerNum).RelNode)));
                    ShowContinueError(state,
                                      format("OutdoorAir:Mixer relief air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OAMixerNum).RelNode)));
                    FatalErrorFlag = true;
                }
                if (state.dataMixedAir->OAController(OAControllerNum).RetNode != state.dataMixedAir->OAMixer(OAMixerNum).RetNode) {
                    ShowSevereError(
                        state, format("The return air node of Controller:OutdoorAir=\"{}\"", state.dataMixedAir->OAController(OAControllerNum).Name));
                    ShowContinueError(state,
                                      format("should be the same node as the return air node of OutdoorAir:Mixer=\"{}\".",
                                             state.dataMixedAir->OAMixer(OAMixerNum).Name));
                    ShowContinueError(state,
                                      format("Controller:OutdoorAir return air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAController(OAControllerNum).RetNode)));
                    ShowContinueError(state,
                                      format("OutdoorAir:Mixer return air node=\"{}\".",
                                             state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OAMixerNum).RetNode)));
                    FatalErrorFlag = true;
                }
            }
        }
        state.dataMixedAir->MyOneTimeErrorFlag(OASysNum) = false;
        if (FatalErrorFlag) {
            ShowFatalError(state, "Previous severe error(s) cause program termination");
        }
    }

    state.dataSize->CurOASysNum = 0;
    if (state.dataAirLoop->OutsideAirSys(OASysNum).AirLoopDOASNum == -1) {
        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysComponentsSimulated = true;
    }
}

void SimOAComponent(EnergyPlusData &state,
                    std::string const &CompType,                    // the component type
                    std::string const &CompName,                    // the component Name
                    SimAirServingZones::CompType const CompTypeNum, // Component Type -- Integerized for this module
                    bool const FirstHVACIteration,
                    int &CompIndex,
                    int const AirLoopNum, // air loop index for economizer lockout coordination
                    bool const Sim,       // if TRUE, simulate component; if FALSE, just set the coil existence flags
                    int const OASysNum,   // index to outside air system
                    bool &OAHeatingCoil,  // TRUE indicates a heating coil has been found
                    bool &OACoolingCoil,  // TRUE indicates a cooling coil has been found
                    bool &OAHX)           // TRUE indicates a heat exchanger has been found
{

    // SUBROUTINE INFORMATION
    //             AUTHOR:  Russ Taylor, Dan Fisher, Fred Buhl
    //       DATE WRITTEN:  Oct 1997
    //           MODIFIED:  Dec 1997 Fred Buhl, D Shirey Feb/Sept 2003
    //                      Nov 2004 M. J. Witte, GARD Analytics, Inc.
    //                        Add DXSystem:AirLoop as valid OA system equipment
    //                        Work supported by ASHRAE research project 1254-RP

    // PURPOSE OF THIS SUBROUTINE:
    // Calls the individual air loop component simulation routines

    // SUBROUTINE LOCAL VARIABLE DEFINITIONS
    OAHeatingCoil = false;
    OACoolingCoil = false;
    OAHX = false;
    HVAC::FanOp fanOp;
    Real64 sensOut = 0.0;
    int constexpr zoneOAUnitNum = -1;
    Real64 constexpr OAUCoilOutTemp = 0.0;
    bool constexpr ZoneEquipFlag = false;
    bool HeatingActive = false; // why isn't this returning that a coil is active?
    bool CoolingActive = false;

    switch (CompTypeNum) {
    case SimAirServingZones::CompType::OAMixer_Num: { // OutdoorAir:Mixer
        if (Sim) {
            SimOAMixer(state, CompName, CompIndex);
        }
        break;
    }
    case SimAirServingZones::CompType::Fan_Simple_CV:        // Fan:ConstantVolume
    case SimAirServingZones::CompType::Fan_Simple_VAV:       // Fan:VariableVolume
    case SimAirServingZones::CompType::Fan_System_Object:    // Fan:SystemModel
    case SimAirServingZones::CompType::Fan_ComponentModel: { // Fan:ComponentModel
        if (Sim) {
            if (CompIndex == 0) { // TODO: get rid of this stuff
                CompIndex = Fans::GetFanIndex(state, CompName);
                assert(CompIndex > 0);
            }

            state.dataFans->fans(CompIndex)->simulate(state, FirstHVACIteration);
        }
    } break;
    case SimAirServingZones::CompType::WaterCoil_Cooling: { // Coil:Cooling:Water
        if (Sim) {
            // get water coil and controller data if not called previously
            if (CompIndex == 0) {
                WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
            }
            // iterate on OA sys controller and water coil at the same time
            if (!state.dataWaterCoils->WaterCoil(CompIndex).heatRecoveryCoil) {
                SimAirServingZones::SolveWaterCoilController(state,
                                                             FirstHVACIteration,
                                                             AirLoopNum,
                                                             CompName,
                                                             CompIndex,
                                                             state.dataWaterCoils->WaterCoil(CompIndex).ControllerName,
                                                             state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex,
                                                             false);
                // set flag to tell HVAC controller it will be simulated only in SolveWaterCoilController()
                state.dataHVACControllers->ControllerProps(state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex).BypassControllerCalc = true;
            } else {
                WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
            }
        } else {
            // This is not working as intended ... don't want to include the HR coil in sizing.
            // But if the water coil is called to get this index, then the controller is called to set the
            // controller index and the simulation sizes the controller before the cooling coil.
            // Pushing this aspect forward to a follow up issue where the
            // controller index call is moved out of water coils getInput.
            // if (CompIndex == 0) {
            //    bool errFound = false;
            //    CompIndex = WaterCoils::GetWaterCoilIndex(state, CompType, CompName, errFound);
            //    if (errFound) ShowFatalError(state, "SimOAComponent: Program terminates for preceding reason.");
            // }
            // if (!state.dataWaterCoils->WaterCoil(CompIndex).heatRecoveryCoil) OACoolingCoil = true;
            // should not include heat recovery coils in sizing since heat transfer at peak cooling is minimal.
            OACoolingCoil = true;
        }
    } break;
    case SimAirServingZones::CompType::WaterCoil_SimpleHeat: { // Coil:Heating:Water
        if (Sim) {
            // get water coil and controller data if not called previously
            if (CompIndex == 0) {
                WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
            }
            // iterate on OA sys controller and water coil at the same time
            SimAirServingZones::SolveWaterCoilController(state,
                                                         FirstHVACIteration,
                                                         AirLoopNum,
                                                         CompName,
                                                         CompIndex,
                                                         state.dataWaterCoils->WaterCoil(CompIndex).ControllerName,
                                                         state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex,
                                                         false);
            // set flag to tell HVAC controller it will be simulated only in SolveWaterCoilController()
            state.dataHVACControllers->ControllerProps(state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex).BypassControllerCalc = true;
        }
        OAHeatingCoil = true;
    } break;
    case SimAirServingZones::CompType::SteamCoil_AirHeat: { // Coil:Heating:Steam
        if (Sim) {
            SteamCoils::SimulateSteamCoilComponents(state, CompName, FirstHVACIteration, CompIndex, 0.0);
        }
        OAHeatingCoil = true;
    } break;
    case SimAirServingZones::CompType::WaterCoil_DetailedCool: { // Coil:Cooling:Water:DetailedGeometry
        if (Sim) {
            // get water coil and controller data if not called previously
            if (CompIndex == 0) {
                WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompIndex);
            }
            // iterate on OA sys controller and water coil at the same time
            SimAirServingZones::SolveWaterCoilController(state,
                                                         FirstHVACIteration,
                                                         AirLoopNum,
                                                         CompName,
                                                         CompIndex,
                                                         state.dataWaterCoils->WaterCoil(CompIndex).ControllerName,
                                                         state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex,
                                                         false);
            // set flag to tell HVAC controller it will be simulated only in SolveWaterCoilController()
            state.dataHVACControllers->ControllerProps(state.dataWaterCoils->WaterCoil(CompIndex).ControllerIndex).BypassControllerCalc = true;
        }
        OACoolingCoil = true;
    } break;
    case SimAirServingZones::CompType::Coil_ElectricHeat: // Coil:Heating:Electric
    case SimAirServingZones::CompType::Coil_GasHeat: {    // Coil:Heating:Fuel
        if (Sim) {
            //     stand-alone coils are temperature controlled (do not pass QCoilReq in argument list, QCoilReq overrides temp SP)
            HeatingCoils::SimulateHeatingCoilComponents(state, CompName, FirstHVACIteration, _, CompIndex);
        }
        OAHeatingCoil = true;
    } break;
    case SimAirServingZones::CompType::WaterCoil_CoolingHXAsst: { // CoilSystem:Cooling:Water:HeatExchangerAssisted
        if (Sim) {
            // get water coil and controller data if not called previously
            if (CompIndex == 0) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                    state, CompName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, CompIndex, HVAC::FanOp::Continuous);
            }
            // iterate on OA sys controller and water coil at the same time
            SimAirServingZones::SolveWaterCoilController(state,
                                                         FirstHVACIteration,
                                                         AirLoopNum,
                                                         CompName,
                                                         CompIndex,
                                                         state.dataHVACAssistedCC->HXAssistedCoil(CompIndex).ControllerName,
                                                         state.dataHVACAssistedCC->HXAssistedCoil(CompIndex).ControllerIndex,
                                                         true);
            // set flag to tell HVAC controller it will be simulated only in SolveWaterCoilController()
            state.dataHVACControllers->ControllerProps(state.dataHVACAssistedCC->HXAssistedCoil(CompIndex).ControllerIndex).BypassControllerCalc =
                true;
        }
        OACoolingCoil = true;
    } break;
    case SimAirServingZones::CompType::DXSystem:             // CoilSystem:Cooling:DX
    case SimAirServingZones::CompType::CoilSystemWater:      // CoilSystem:Cooling:Water
    case SimAirServingZones::CompType::UnitarySystemModel: { // AirloopHVAC:UnitarySystem
        if (Sim) {
            Real64 latOut = 0.0;     // does the air loop not need to know what the latent capacity is?
            int compNum = CompIndex; // use local so return value of compNum from simulate call does not overwrite CompIndex
            state.dataAirLoop->OutsideAirSys(OASysNum).compPointer[compNum]->simulate(state,
                                                                                      CompName,
                                                                                      FirstHVACIteration,
                                                                                      AirLoopNum,
                                                                                      compNum,
                                                                                      HeatingActive,
                                                                                      CoolingActive,
                                                                                      zoneOAUnitNum,
                                                                                      OAUCoilOutTemp,
                                                                                      ZoneEquipFlag,
                                                                                      sensOut,
                                                                                      latOut);
        }
        if (state.dataMixedAir->MyOneTimeCheckUnitarySysFlag(OASysNum) && CompTypeNum == SimAirServingZones::CompType::UnitarySystemModel) {
            UnitarySystems::UnitarySys::getUnitarySysHeatCoolCoil(state, CompName, OACoolingCoil, OAHeatingCoil, 0);
            UnitarySystems::UnitarySys::checkUnitarySysCoilInOASysExists(state, CompName, 0);
            if (Sim) {
                state.dataMixedAir->MyOneTimeCheckUnitarySysFlag(OASysNum) = false;
            }
        } else {
            OACoolingCoil = true;
        }
    } break;
    case SimAirServingZones::CompType::DXHeatPumpSystem: { // CoilSystem:IntegratedHeatPump:AirSource
        if (Sim) {
            HVACDXHeatPumpSystem::SimDXHeatPumpSystem(state, CompName, FirstHVACIteration, AirLoopNum, CompIndex);
        }
        OAHeatingCoil = true;
    } break;
    case SimAirServingZones::CompType::CoilUserDefined: { // Coil:UserDefined
        if (Sim) {
            UserDefinedComponents::SimCoilUserDefined(state, CompName, CompIndex, AirLoopNum, OAHeatingCoil, OACoolingCoil);
        }
    } break;
    case SimAirServingZones::CompType::HeatXchngr: {
        // HeatExchanger:AirToAir:FlatPlate, HeatExchanger:AirToAir:SensibleAndLatent, HeatExchanger:Desiccant:BalancedFlow
        if (Sim) {
            Real64 AirloopPLR = 1;
            if (state.dataAirLoop->OutsideAirSys(OASysNum).AirLoopDOASNum > -1) {
                fanOp = HVAC::FanOp::Continuous;
            } else {
                if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp == HVAC::FanOp::Cycling) {
                    fanOp = HVAC::FanOp::Cycling;
                } else {
                    fanOp = HVAC::FanOp::Continuous;
                }

                if (fanOp == HVAC::FanOp::Cycling) {
                    // HX's in the OA system can be troublesome given that the OA flow rate is not necessarily proportional to air loop PLR
                    // adding that user input for branch flow rate, HX nominal flow rate, OA system min/max flow rate will not necessarily be
                    // perfectly input, a compromise is used for OA sys HX's as the ratio of flow to max. Issue #4298.
                    //                    AirloopPLR = AirLoopFlow( AirLoopNum ).FanPLR;
                    AirloopPLR = state.dataMixedAir->OAController(OASysNum).OAMassFlow / state.dataMixedAir->OAController(OASysNum).MaxOAMassFlowRate;
                }
            }
            if (state.dataAirLoop->OutsideAirSys(OASysNum).AirLoopDOASNum > -1) {
                HeatRecovery::SimHeatRecovery(state, CompName, FirstHVACIteration, CompIndex, fanOp, AirloopPLR, _, _, _, _, _);
            } else {
                HeatRecovery::SimHeatRecovery(state,
                                              CompName,
                                              FirstHVACIteration,
                                              CompIndex,
                                              fanOp,
                                              AirloopPLR,
                                              _,
                                              _,
                                              _,
                                              state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HeatRecoveryBypass,
                                              state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HighHumCtrlActive);
            }
        }
        OAHX = true;
    } break;
    case SimAirServingZones::CompType::Desiccant: { // Dehumidifier:Desiccant:NoFans,  Dehumidifier:Desiccant:NoFans, Dehumidifier:Desiccant:System
        if (Sim) {
            DesiccantDehumidifiers::SimDesiccantDehumidifier(state, CompName, FirstHVACIteration, CompIndex);
        }
        OAHX = true;
    } break;
    case SimAirServingZones::CompType::Humidifier: { // Humidifier:Steam:Electric Humidifier:Steam:Gas
        if (Sim) {
            Humidifiers::SimHumidifier(state, CompName, FirstHVACIteration, CompIndex);
        }
    } break;
    case SimAirServingZones::CompType::Unglazed_SolarCollector: { // SolarCollector:UnglazedTranspired
        if (Sim) {
            TranspiredCollector::SimTranspiredCollector(state, CompName, CompIndex);
        }
    } break;
    case SimAirServingZones::CompType::PVT_AirBased: { // SolarCollector:FlatPlate:PhotovoltaicThermal
        if (Sim) {
            if (CompIndex == 0) {
                CompIndex = PhotovoltaicThermalCollectors::getPVTindexFromName(state, CompName);
            }
            PhotovoltaicThermalCollectors::simPVTfromOASys(state, CompIndex, FirstHVACIteration);
        }
    } break;
    case SimAirServingZones::CompType::EvapCooler: { // EvaporativeCooler:Direct:CelDekPad, EvaporativeCooler:Indirect:CelDekPad
        // EvaporativeCooler:Indirect:WetCoil, EvaporativeCooler:Indirect:ResearchSpecial
        if (Sim) {
            EvaporativeCoolers::SimEvapCooler(state, CompName, CompIndex);
        }
    } break;
    case SimAirServingZones::CompType::ZoneVRFasAirLoopEquip: { // ZoneHVAC:TerminalUnit:VariableRefrigerantFlow
        if (Sim) {
            int ControlledZoneNum = 0;
            int constexpr OAUnitNum = 0;
            Real64 constexpr OAUCoilOutTemp = 0.0;
            bool constexpr ZoneEquipment = false;
            Real64 sysOut = 0.0;
            Real64 latOut = 0.0;
            HVACVariableRefrigerantFlow::SimulateVRF(state,
                                                     CompName,
                                                     FirstHVACIteration,
                                                     ControlledZoneNum,
                                                     CompIndex,
                                                     HeatingActive,
                                                     CoolingActive,
                                                     OAUnitNum,
                                                     OAUCoilOutTemp,
                                                     ZoneEquipment,
                                                     sysOut,
                                                     latOut);
        } else {
            HVACVariableRefrigerantFlow::isVRFCoilPresent(state, CompName, OACoolingCoil, OAHeatingCoil);
        }
    } break;
    default:
        ShowFatalError(state, format("Invalid Outside Air Component={}", CompType));
    }
}

void SimOAMixer(EnergyPlusData &state, std::string const &CompName, int &CompIndex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Simulate an Outside Air Mixer component

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int OAMixerNum;

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    if (CompIndex == 0) {
        OAMixerNum = Util::FindItemInList(CompName, state.dataMixedAir->OAMixer);
        CompIndex = OAMixerNum;
        if (OAMixerNum == 0) {
            ShowFatalError(state, format("SimOAMixer: OutdoorAir:Mixer not found={}", CompName));
        }
    } else {
        OAMixerNum = CompIndex;
    }

    auto &mixer = state.dataMixedAir->OAMixer(OAMixerNum);

    mixer.InitOAMixer(state);

    mixer.CalcOAMixer(state);

    mixer.UpdateOAMixer(state);
}

void SimOAController(EnergyPlusData &state, std::string const &CtrlName, int &CtrlIndex, bool const FirstHVACIteration, int const AirLoopNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Simulate an Outside Air Controller component

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int OAControllerNum;

    if ((state.dataMixedAir->GetOAControllerInputFlag) &&
        (AirLoopNum > 0)) { // Gets input for object  first time Sim routine is called from an airloop
        GetOAControllerInputs(state);
        state.dataMixedAir->GetOAControllerInputFlag = false;
    }

    // check that the economizer staging operation EconomizerFirst is only used with an sensible load-based controlled AirLoopHVAC:UnitarySystem
    if (AirLoopNum > 0) {
        auto &primaryAirSystems = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
        if (primaryAirSystems.EconomizerStagingCheckFlag == false) {
            OAControllerNum = Util::FindItemInList(CtrlName, state.dataMixedAir->OAController);
            if (state.dataMixedAir->OAController(OAControllerNum).EconomizerStagingType == HVAC::EconomizerStagingType::EconomizerFirst) {
                bool sensLoadCtrlUnitarySystemFound = false;
                for (int BranchNum = 1; BranchNum <= primaryAirSystems.NumBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= primaryAirSystems.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (primaryAirSystems.Branch(BranchNum).Comp(CompNum).CompType_Num == SimAirServingZones::CompType::UnitarySystemModel) {
                            std::string_view unitarySystemName = primaryAirSystems.Branch(BranchNum).Comp(CompNum).Name;
                            int unitarySystemNum = Util::FindItemInList(
                                unitarySystemName, state.dataUnitarySystems->unitarySys, state.dataUnitarySystems->numUnitarySystems);
                            if (state.dataUnitarySystems->unitarySys[unitarySystemNum - 1].m_ControlType ==
                                UnitarySystems::UnitarySys::UnitarySysCtrlType::Load) {
                                if (state.dataUnitarySystems->unitarySys[unitarySystemNum - 1].m_CoolingCoilType_Num ==
                                        HVAC::CoilDX_MultiSpeedCooling ||
                                    state.dataUnitarySystems->unitarySys[unitarySystemNum - 1].m_CoolingCoilType_Num ==
                                        HVAC::Coil_CoolingAirToAirVariableSpeed ||
                                    state.dataUnitarySystems->unitarySys[unitarySystemNum - 1].m_CoolingCoilType_Num == HVAC::CoilDX_Cooling) {
                                    sensLoadCtrlUnitarySystemFound = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (!sensLoadCtrlUnitarySystemFound) {
                    ShowWarningError(
                        state,
                        format(
                            "SimOAController: EconomizerFirst was selected in the \"{}\" Controller:OutdoorAir object but the air loop it belongs to "
                            "does not include an AirLoopHVAC:UnitarySystem with a \"Load\" Control Type input and cooling coil of one of the "
                            "following types: Coil:Cooling:DX:MultiSpeed,"
                            " Coil:Cooling:DX:VariableSpeed, or Coil:Cooling:DX. EconomizerFirst will not be enforced.",
                            state.dataMixedAir->OAController(OAControllerNum).Name));
                }
            }
            primaryAirSystems.EconomizerStagingCheckFlag = true;
        }
    }

    if (CtrlIndex == 0) {
        if (state.dataMixedAir->NumOAControllers > 0) {
            OAControllerNum = Util::FindItemInList(CtrlName, state.dataMixedAir->OAController);
        } else {
            OAControllerNum = 0;
        }
        CtrlIndex = OAControllerNum;
        if (OAControllerNum == 0) {
            ShowFatalError(state, format("SimOAController: Outside Air Controller not found={}", CtrlName));
        }
    } else {
        OAControllerNum = CtrlIndex;
    }

    InitOAController(state, OAControllerNum, FirstHVACIteration, AirLoopNum);

    state.dataMixedAir->OAController(OAControllerNum).CalcOAController(state, AirLoopNum, FirstHVACIteration);
    state.dataMixedAir->OAController(OAControllerNum).UpdateOAController(state);
}

// Get Input Section of the Module
//******************************************************************************

void GetOutsideAirSysInputs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Input the Outside Air System data and store it in the OutsideAirSys array.

    // METHODOLOGY EMPLOYED:
    // Use the Get routines from the InputProcessor module.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetOutsideAirSysInputs: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false);
    int NumNums;      // Number of real numbers returned by GetObjectItem
    int NumAlphas;    // Number of alphanumerics returned by GetObjectItem
    int TotalArgs(0); // Total number of alpha and numeric arguments (max) for a
    int IOStat;
    Array1D<Real64> NumArray;
    Array1D_string AlphArray;
    Array1D_string cAlphaFields;   // Alpha field names
    Array1D_string cNumericFields; // Numeric field names
    Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.

    if (!state.dataMixedAir->GetOASysInputFlag) {
        return;
    }

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::OASystem)], TotalArgs, NumAlphas, NumNums);
    int MaxNums = NumNums;
    int MaxAlphas = NumAlphas;
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::AirLoopEqList)], TotalArgs, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::ControllerList)], TotalArgs, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);

    AlphArray.allocate(MaxAlphas);
    cAlphaFields.allocate(MaxAlphas);
    NumArray.dimension(MaxNums, 0.0);
    cNumericFields.allocate(MaxNums);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNums, true);

    std::string_view CurrentModuleObject = CurrentModuleObjects[static_cast<int>(CMO::ControllerList)];
    state.dataMixedAir->NumControllerLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    state.dataMixedAir->ControllerLists.allocate(state.dataMixedAir->NumControllerLists);

    for (int Item = 1; Item <= state.dataMixedAir->NumControllerLists; ++Item) {

        // create a reference for convenience
        auto &thisControllerList(state.dataMixedAir->ControllerLists(Item));
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Item,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);
        thisControllerList.Name = AlphArray(1); // no need to check if AlphaArray(1) is empty since Json will catch missing required fields
        thisControllerList.NumControllers = (NumAlphas - 1) / 2;
        thisControllerList.ControllerType.dimension(thisControllerList.NumControllers, ControllerKind::Invalid);
        thisControllerList.ControllerName.allocate(thisControllerList.NumControllers);
        int AlphaNum = 2;
        for (int CompNum = 1; CompNum <= thisControllerList.NumControllers; ++CompNum) {
            // Json will catch any object types that are not the correct key choice of Controller:OutdoorAir or Controller:WaterCoil
            thisControllerList.ControllerType(CompNum) =
                static_cast<ControllerKind>(getEnumValue(ControllerKindNamesUC, Util::makeUPPER(AlphArray(AlphaNum))));
            thisControllerList.ControllerName(CompNum) = AlphArray(AlphaNum + 1);
            // loop over all previous controller lists to check if this controllers is also present on previous controllers
            for (int previousListNum = 1; previousListNum < Item; ++previousListNum) {
                // loop over each of the controllers listed for this list
                auto &previousList(state.dataMixedAir->ControllerLists(previousListNum));
                for (int PreviousListControllerNum = 1; PreviousListControllerNum <= previousList.NumControllers; ++PreviousListControllerNum) {
                    if ((previousList.ControllerType(PreviousListControllerNum) == thisControllerList.ControllerType(CompNum)) &&
                        (previousList.ControllerName(PreviousListControllerNum) == thisControllerList.ControllerName(CompNum))) {
                        ShowSevereError(state, format("Controller instance repeated in multiple {} objects", CurrentModuleObject));
                        ShowContinueError(state, format("Found in {} = {}", CurrentModuleObject, thisControllerList.Name));
                        ShowContinueError(state, format("Also found in {} = {}", CurrentModuleObject, previousList.Name));
                        ErrorsFound = true;
                    }
                }
            }
            AlphaNum += 2;
        }
    }

    CurrentModuleObject = CurrentModuleObjects[static_cast<int>(CMO::OASystem)];

    state.dataAirLoop->NumOASystems = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    state.dataAirLoop->OutsideAirSys.allocate(state.dataAirLoop->NumOASystems);
    state.dataSize->OASysEqSizing.allocate(state.dataAirLoop->NumOASystems);
    state.dataMixedAir->ControllerListUniqueNames.reserve(static_cast<unsigned>(state.dataAirLoop->NumOASystems));
    state.dataMixedAir->MyOneTimeErrorFlag.dimension(state.dataAirLoop->NumOASystems, true);
    state.dataMixedAir->MyOneTimeCheckUnitarySysFlag.dimension(state.dataAirLoop->NumOASystems, true);
    state.dataMixedAir->initOASysFlag.dimension(state.dataAirLoop->NumOASystems, true);

    for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
        auto &OASys = state.dataAirLoop->OutsideAirSys(OASysNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 OASysNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);
        OASys.Name = AlphArray(1); // no need to check if AlphaArray(1) is empty since Json will catch missing required fields
        if (!AlphArray(2).empty()) {
            GlobalNames::IntraObjUniquenessCheck(
                state, AlphArray(2), CurrentModuleObject, cAlphaFields(2), state.dataMixedAir->ControllerListUniqueNames, ErrorsFound);
        }
        OASys.ControllerListName = AlphArray(2);
        OASys.ComponentListName = AlphArray(3);

        BranchNodeConnections::TestCompSet(state, CurrentModuleObject, OASys.Name, "UNDEFINED", "UNDEFINED", "Air Nodes");

        if (!lAlphaBlanks(3)) {
            int ListNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(
                state, CurrentModuleObjects[static_cast<int>(CMO::AirLoopEqList)], OASys.ComponentListName);
            if (ListNum > 0) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, CurrentModuleObjects[static_cast<int>(CMO::AirLoopEqList)], ListNum, AlphArray, NumAlphas, NumArray, NumNums, IOStat);
                int NumInList = (NumAlphas - 1) / 2;
                OASys.NumComponents = NumInList;
                OASys.ComponentName.allocate(NumInList);
                OASys.ComponentType.allocate(NumInList);
                OASys.ComponentTypeEnum.dimension(NumInList, SimAirServingZones::CompType::Invalid);
                OASys.ComponentIndex.dimension(NumInList, 0);
                OASys.InletNodeNum.dimension(NumInList, 0);
                OASys.OutletNodeNum.dimension(NumInList, 0);
                OASys.compPointer.resize(NumInList + 1, nullptr);
                for (int InListNum = 1; InListNum <= NumInList; ++InListNum) {
                    OASys.ComponentName(InListNum) = AlphArray(InListNum * 2 + 1);
                    OASys.ComponentType(InListNum) = AlphArray(InListNum * 2);

                    // Add equipment to component sets array
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         OASys.Name,
                                                         OASys.ComponentType(InListNum),
                                                         OASys.ComponentName(InListNum),
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
            } else {
                ShowSevereError(
                    state,
                    format("{} = \"{}\" invalid {}=\"{}\" not found.", CurrentModuleObject, OASys.Name, cAlphaFields(3), OASys.ComponentListName));
                ErrorsFound = true;
            }
        } else {
            ShowSevereError(state, format("{} = \"{}\" invalid {} is blank and must be entered.", CurrentModuleObject, OASys.Name, cAlphaFields(3)));
            ErrorsFound = true;
        }

        int ListNum = 0;
        int NumSimpControllers = 0; // number of Controller:Simple objects in an OA System
        if (!lAlphaBlanks(2)) {
            ListNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(
                state, CurrentModuleObjects[static_cast<int>(CMO::ControllerList)], OASys.ControllerListName);
            if (ListNum > 0) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, CurrentModuleObjects[static_cast<int>(CMO::ControllerList)], ListNum, AlphArray, NumAlphas, NumArray, NumNums, IOStat);
                int NumInList = (NumAlphas - 1) / 2;
                OASys.NumControllers = NumInList;
                OASys.ControllerName.allocate(NumInList);
                OASys.ControllerType.allocate(NumInList);
                OASys.controllerTypeEnum.dimension(NumInList, DataAirLoop::ControllerKind::Invalid);
                OASys.ControllerIndex.dimension(NumInList, 0);
                for (int InListNum = 1; InListNum <= NumInList; ++InListNum) {
                    OASys.ControllerName(InListNum) = AlphArray(InListNum * 2 + 1);
                    OASys.ControllerType(InListNum) = AlphArray(InListNum * 2);
                    OASys.controllerTypeEnum(InListNum) =
                        static_cast<DataAirLoop::ControllerKind>(getEnumValue(ControllerKindNamesUC, OASys.ControllerType(InListNum)));
                    // only count Controller:OutdoorAir types as valid simple controllers
                    if (OASys.controllerTypeEnum(InListNum) != DataAirLoop::ControllerKind::OutdoorAir) {
                        ++NumSimpControllers;
                    }
                }
            } else {
                ShowSevereError(state,
                                format("{} = \"{}\" invalid {}=\"{}\" not found.", CurrentModuleObject, AlphArray(1), cAlphaFields(2), AlphArray(2)));
                ErrorsFound = true;
            }
        }
        OASys.ControllerListNum = ListNum;
        OASys.NumSimpleControllers = NumSimpControllers;
    }

    for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
        auto &OASys = state.dataAirLoop->OutsideAirSys(OASysNum);
        for (int CompNum = 1; CompNum <= OASys.NumComponents; ++CompNum) {
            OASys.ComponentTypeEnum(CompNum) = static_cast<SimAirServingZones::CompType>(getEnumValue(CompTypeNamesUC, OASys.ComponentType(CompNum)));
            if (OASys.ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::Fan_System_Object) {
                // construct fan object
                OASys.ComponentIndex(CompNum) = Fans::GetFanIndex(state, OASys.ComponentName(CompNum));
            } else if (OASys.ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::CoilSystemWater ||
                       OASys.ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::UnitarySystemModel ||
                       OASys.ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::DXSystem) {
                OASys.ComponentIndex(CompNum) = CompNum;
            } else if (OASys.ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::Invalid) {
                std::string const thisComp = OASys.ComponentType(CompNum);
                if (thisComp == "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT" || thisComp == "HEATEXCHANGER:DESICCANT:BALANCEDFLOW") {
                    OASys.ComponentTypeEnum(CompNum) = SimAirServingZones::CompType::HeatXchngr;
                } else if (thisComp == "DEHUMIDIFIER:DESICCANT:SYSTEM") {
                    OASys.ComponentTypeEnum(CompNum) = SimAirServingZones::CompType::Desiccant;
                } else if (thisComp == "EVAPORATIVECOOLER:INDIRECT:CELDEKPAD" || thisComp == "EVAPORATIVECOOLER:INDIRECT:WETCOIL" ||
                           thisComp == "EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL" || thisComp == "EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL") {
                    OASys.ComponentTypeEnum(CompNum) = SimAirServingZones::CompType::EvapCooler;
                } else if (thisComp == "HUMIDIFIER:STEAM:GAS") {
                    OASys.ComponentTypeEnum(CompNum) = SimAirServingZones::CompType::Humidifier;
                } else {
                    ShowSevereError(
                        state,
                        format("{} = \"{}\" invalid Outside Air Component=\"{}\".", CurrentModuleObject, AlphArray(1), OASys.ComponentType(CompNum)));
                    ErrorsFound = true;
                }
            }
        }

        // loop through the controllers in the controller list for OA system and save the pointer to the OA controller index
        for (int OAControllerNum = 1; OAControllerNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumControllers; ++OAControllerNum) {
            if (state.dataAirLoop->OutsideAirSys(OASysNum).controllerTypeEnum(OAControllerNum) == DataAirLoop::ControllerKind::OutdoorAir) {
                state.dataAirLoop->OutsideAirSys(OASysNum).OAControllerName =
                    state.dataAirLoop->OutsideAirSys(OASysNum).ControllerName(OAControllerNum);
                break;
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found in getting {}.", RoutineName, CurrentModuleObject));
    }

    AlphArray.deallocate();
    cAlphaFields.deallocate();
    NumArray.deallocate();
    cNumericFields.deallocate();
    lAlphaBlanks.deallocate();
    lNumericBlanks.deallocate();

    state.dataMixedAir->GetOASysInputFlag = false;

    // once GetOASysInputFlag is set to false other calls to objects can occur without worry that GetOutsideAirSysInputs will be called again
    // now get the pointer for UnitarySystem - doing this earlier can cause recursion which trips IntraObjUniquenessCheck warnings
    for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
        for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents; ++CompNum) {
            if (state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::CoilSystemWater ||
                state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::UnitarySystemModel ||
                state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(CompNum) == SimAirServingZones::CompType::DXSystem) {
                state.dataAirLoop->OutsideAirSys(OASysNum).compPointer[CompNum] = UnitarySystems::UnitarySys::factory(
                    state, HVAC::UnitarySysType::Unitary_AnyCoilType, state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(CompNum), false, 0);
            }
        }
    }
}

void GetOAControllerInputs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998
    //       MODIFIED       Shirey/Raustad FSEC, June 2003, Jan 2004
    //                      Mangesh Basarkar, 06/2011: Getting zone OA specifications from Design Specification Object
    //                      Tianzhen Hong, 3/2012: getting zone air distribution effectiveness and secondary recirculation
    //                       from DesignSpecification:ZoneAirDistribution objects

    // PURPOSE OF THIS SUBROUTINE
    // Input the OAController data and store it in the OAController array.
    // Input the Ventilation:Mechanical data and store it in the VentilationMechanical array.
    //  Condense Ventilation:Mechanical data array to include only unique zones specified for each instance of this object.

    // METHODOLOGY EMPLOYED:
    // Use the Get routines from the InputProcessor module.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetOAControllerInputs: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetOAControllerInputs";  // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumArg;    // Number of arguments from GetObjectDefMaxArgs call
    int NumNums;   // Number of real numbers returned by GetObjectItem
    int NumAlphas; // Number of alphanumerics returned by GetObjectItem
    int IOStat;    // Status of GetObjectItem call
    Array1D<Real64> NumArray;
    Array1D_string AlphArray;
    std::string_view CurrentModuleObject; // Object type for getting and messages
    Array1D_string cAlphaFields;          // Alpha field names
    Array1D_string cNumericFields;        // Numeric field names
    Array1D_bool lAlphaBlanks;            // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;          // Logical array, numeric field input BLANK = .TRUE.
    bool ErrorsFound(false);              // Flag identifying errors found during get input

    // First, call other get input routines in this module to make sure data is filled during this routine.
    if (state.dataMixedAir->GetOASysInputFlag) { // Gets input for object  first time Sim routine is called
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }
    if (state.dataMixedAir->GetOAMixerInputFlag) { // Gets input for object  first time Sim routine is called
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    FaultsManager::CheckAndReadFaults(state);

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::OAController)], NumArg, NumAlphas, NumNums);
    int MaxAlphas = NumAlphas;
    int MaxNums = NumNums;
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::ERVController)], NumArg, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)], NumArg, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);

    AlphArray.allocate(MaxAlphas);
    NumArray.dimension(MaxNums, 0.0);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNums, true);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNums);

    // Count OAcontrollers and ERVcontrollers and allocate arrays
    AllocateOAControllers(state);

    // If there are ERV controllers, they have been filled before now NumOAControllers includes the count of NumERVControllers
    if (state.dataMixedAir->NumOAControllers > state.dataMixedAir->NumERVControllers) {
        CurrentModuleObject = CurrentModuleObjects[static_cast<int>(CMO::OAController)];
        int currentOAControllerNum = 0;
        for (int OutAirNum = state.dataMixedAir->NumERVControllers + 1; OutAirNum <= state.dataMixedAir->NumOAControllers; ++OutAirNum) {
            ++currentOAControllerNum;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     currentOAControllerNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataMixedAir->OAControllerUniqueNames, AlphArray(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            ProcessOAControllerInputs(state,
                                      CurrentModuleObject,
                                      OutAirNum,
                                      AlphArray,
                                      NumAlphas,
                                      NumArray,
                                      NumNums,
                                      lNumericBlanks,
                                      lAlphaBlanks,
                                      cAlphaFields,
                                      cNumericFields,
                                      ErrorsFound);

            // add applicable faults identifier to avoid string comparison at each time step
            //  loop through each fault for each OA controller and determine economizer faults
            for (int i = 1; i <= state.dataFaultsMgr->NumFaultyEconomizer; ++i) {
                if (state.dataFaultsMgr->FaultsEconomizer(i).ControllerTypeEnum != iController_AirEconomizer) {
                    continue;
                }
                if (Util::SameString(state.dataMixedAir->OAController(OutAirNum).Name, state.dataFaultsMgr->FaultsEconomizer(i).ControllerName)) {
                    state.dataFaultsMgr->FaultsEconomizer(i).ControllerID = OutAirNum;
                    ++state.dataMixedAir->OAController(OutAirNum).NumFaultyEconomizer;
                }
            }
            //  loop through each fault for each OA controller to determine faulty counts
            state.dataMixedAir->OAController(OutAirNum).EconmizerFaultNum.allocate(state.dataMixedAir->OAController(OutAirNum).NumFaultyEconomizer);
            if (state.dataMixedAir->OAController(OutAirNum).NumFaultyEconomizer > 0) {
                for (int j = 0, i = 1; i <= state.dataFaultsMgr->NumFaultyEconomizer; ++i) {
                    if (state.dataFaultsMgr->FaultsEconomizer(i).ControllerTypeEnum != iController_AirEconomizer) {
                        continue;
                    }
                    if (Util::SameString(state.dataMixedAir->OAController(OutAirNum).Name, state.dataFaultsMgr->FaultsEconomizer(i).ControllerName)) {
                        state.dataMixedAir->OAController(OutAirNum).EconmizerFaultNum(++j) = i;
                    }
                }
            }
        } // LOOP FOR OutAirNum

        if (ErrorsFound) {
            AlphArray.deallocate();
            NumArray.deallocate();
            lNumericBlanks.deallocate();
            lAlphaBlanks.deallocate();
            cAlphaFields.deallocate();
            cNumericFields.deallocate();
            ShowFatalError(state, format("{}Errors found in getting {} inputs.", RoutineName, CurrentModuleObject));
        }
    }

    state.dataMixedAir->GetOAControllerInputFlag = false;

    // Process Controller:MechanicalVentilation objects
    CurrentModuleObject = CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)];
    state.dataMixedAir->NumVentMechControllers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    if (state.dataMixedAir->NumVentMechControllers > 0) {
        state.dataMixedAir->VentilationMechanical.allocate(state.dataMixedAir->NumVentMechControllers);
        for (int VentMechNum = 1; VentMechNum <= state.dataMixedAir->NumVentMechControllers; ++VentMechNum) {
            auto &thisVentilationMechanical(state.dataMixedAir->VentilationMechanical(VentMechNum));
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     VentMechNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

            int MechVentZoneCount = 0;

            int NumGroups = (NumAlphas + NumNums - 5) / 3; // Number of extensible input groups of the VentilationMechanical object
            if (mod((NumAlphas + NumNums - 5), 3) != 0) {
                ++NumGroups;
            }
            thisVentilationMechanical.Name = AlphArray(1); // no need to check if AlphaArray(1) is empty since Json will catch missing required fields

            if (lAlphaBlanks(2)) {
                thisVentilationMechanical.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisVentilationMechanical.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Adding new flag for DCV
            if (Util::SameString(AlphArray(3), "Yes")) {
                thisVentilationMechanical.DCVFlag = true;
            } else if (Util::SameString(AlphArray(3), "No") || lAlphaBlanks(3)) {
                thisVentilationMechanical.DCVFlag = false;
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid value {}=\"{}\".", CurrentModuleObject, AlphArray(1), cAlphaFields(3), AlphArray(3)));
                ShowContinueError(state, "...Valid values are \"Yes\" or \"No\".");
                ErrorsFound = true;
            }

            // System outdoor air method
            thisVentilationMechanical.SystemOAMethod = static_cast<DataSizing::SysOAMethod>(getEnumValue(SOAMNamesUC, Util::makeUPPER(AlphArray(4))));

            if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQP ||
                thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc ||
                thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc ||
                thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate ||
                thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQPCOM) {
                if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    ShowSevereError(
                        state,
                        format(
                            "{}=\"{}\" valid {}=\"{}\" requires CO2 simulation.", CurrentModuleObject, AlphArray(1), cAlphaFields(2), AlphArray(2)));
                    ShowContinueError(state, "The choice must be Yes for the field Carbon Dioxide Concentration in ZoneAirContaminantBalance");
                    ErrorsFound = true;
                }
            }

            if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQPGC ||
                thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQPCOM) {
                if (!state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" valid {}=\"{}\" requires generic contaminant simulation.",
                                           CurrentModuleObject,
                                           AlphArray(1),
                                           cAlphaFields(2),
                                           AlphArray(2)));
                    ShowContinueError(state, "The choice must be Yes for the field Generic Contaminant Concentration in ZoneAirContaminantBalance");
                    ErrorsFound = true;
                }
            }

            if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::Invalid) { // If specified incorrectly, show errors
                thisVentilationMechanical.SystemOAMethod = DataSizing::SysOAMethod::ZoneSum;
                ShowWarningError(state,
                                 format("{}=\"{}\" incorrect specification for {}, the ZoneSum method will be used.",
                                        CurrentModuleObject,
                                        AlphArray(1),
                                        cAlphaFields(4)));
                // ErrorsFound=.TRUE.
            }

            // Zone maximum outdoor air fraction
            thisVentilationMechanical.ZoneMaxOAFraction = NumArray(1);

            EPVector<int> designSpecOAObjIndex;           // index of the design specification outdoor air object
            EPVector<std::string> ventMechZoneOrListName; // Zone or Zone List to apply mechanical ventilation rate
            EPVector<int> designSpecZoneADObjIndex;       // index of the design specification zone air distribution object
            ventMechZoneOrListName.allocate(NumGroups);
            designSpecOAObjIndex.dimension(NumGroups, 0);
            designSpecZoneADObjIndex.dimension(NumGroups, 0);

            //   First time through find the total number of zones requiring mechanical ventilation
            //   May include duplicate zones. Will check for duplicate zones further down in this subroutine.
            for (int groupNum = 1; groupNum <= NumGroups; ++groupNum) {
                ventMechZoneOrListName(groupNum) = AlphArray((groupNum - 1) * 3 + 5);

                //     Getting OA details from design specification OA object
                if (!lAlphaBlanks((groupNum - 1) * 3 + 6)) {
                    std::string_view designSpecOAObjName = AlphArray((groupNum - 1) * 3 + 6);
                    int ObjIndex = Util::FindItemInList(designSpecOAObjName, state.dataSize->OARequirements);
                    designSpecOAObjIndex(groupNum) = ObjIndex;
                    if (ObjIndex == 0) {
                        ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, CurrentModuleObject, thisVentilationMechanical.Name));
                        ShowContinueError(state, format("... not found {}=\"{}\".", cAlphaFields((groupNum - 1) * 3 + 6), designSpecOAObjName));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Design Specification Outdoor Air Object Name blank", CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(state, format("For Zone=\"{}\".", ventMechZoneOrListName(groupNum)));
                    ShowContinueError(state, "This field either needs to be filled in in this object or Sizing:Zone object.");
                    ShowContinueError(state, "For this run, default values for these fields will be used.");
                }

                // Get zone air distribution details from design specification Zone Air Distribution object
                if (!lAlphaBlanks((groupNum - 1) * 3 + 7)) {
                    std::string_view designSpecZoneADObjName = AlphArray((groupNum - 1) * 3 + 7);
                    int ObjIndex = Util::FindItemInList(designSpecZoneADObjName, state.dataSize->ZoneAirDistribution);
                    designSpecZoneADObjIndex(groupNum) = ObjIndex;
                    if (ObjIndex == 0) {
                        // Cannot find the design specification Zone Air Distribution object
                        ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, CurrentModuleObject, thisVentilationMechanical.Name));
                        ShowContinueError(state, format("... not found {}=\"{}\".", cAlphaFields((groupNum - 1) * 3 + 7), designSpecZoneADObjName));
                        ErrorsFound = true;
                    }
                }

                int ZoneNum = Util::FindItemInList(ventMechZoneOrListName(groupNum), state.dataHeatBal->Zone);
                if (ZoneNum > 0) {
                    ++MechVentZoneCount;
                } else {
                    int ZoneListNum = Util::FindItemInList(ventMechZoneOrListName(groupNum), state.dataHeatBal->ZoneList);
                    if (ZoneListNum > 0) {
                        MechVentZoneCount += state.dataHeatBal->ZoneList(ZoneListNum).NumOfZones;
                    } else {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\" invalid {} not found.", CurrentModuleObject, AlphArray(1), cAlphaFields((groupNum - 1) * 3 + 5)));
                        ShowContinueError(state, format("Missing {} = {}", cAlphaFields((groupNum - 1) * 3 + 5), ventMechZoneOrListName(groupNum)));
                        ErrorsFound = true;
                    }
                }
            }

            thisVentilationMechanical.NumofVentMechZones = MechVentZoneCount;

            // Now allocate and store unique zone and associated ventilation rate information
            thisVentilationMechanical.VentMechZone.allocate(MechVentZoneCount);

            MechVentZoneCount = 0;

            //   Loop through zone names and list of zone names, remove duplicate zones, and store designspec names and indexes
            for (int groupNum = 1; groupNum <= NumGroups; ++groupNum) {
                int ZoneNum = Util::FindItemInList(ventMechZoneOrListName(groupNum), state.dataHeatBal->Zone);
                if (ZoneNum > 0) {
                    if (std::any_of(thisVentilationMechanical.VentMechZone.begin(),
                                    thisVentilationMechanical.VentMechZone.end(),
                                    [ZoneNum](auto const &vmZone) { return vmZone.zoneNum == ZoneNum; })) {
                        //          Disregard duplicate zone names, show warning and do not store data for this zone
                        ShowWarningError(state,
                                         format("Zone name = {} for {} object = {}",
                                                ventMechZoneOrListName(groupNum),
                                                CurrentModuleObject,
                                                thisVentilationMechanical.Name));
                        ShowContinueError(state, "is specified more than once. The first ventilation values specified for this zone will be used");
                        ShowContinueError(state, "and the rest will be ignored. Simulation will continue..");
                    } else {
                        //          Store unique zone names
                        ++MechVentZoneCount;
                        auto &thisMechVentZone = thisVentilationMechanical.VentMechZone(MechVentZoneCount);
                        thisMechVentZone.zoneNum = ZoneNum;
                        thisMechVentZone.name = state.dataHeatBal->Zone(ZoneNum).Name;

                        // Populating new temp array to hold design spec OA object for each zone
                        if (designSpecOAObjIndex(groupNum) > 0) {
                            thisMechVentZone.ZoneDesignSpecOAObjIndex = designSpecOAObjIndex(groupNum);
                        } else {
                            if (state.dataGlobal->DoZoneSizing) {
                                int ObjIndex = Util::FindItemInList(
                                    ventMechZoneOrListName(groupNum), state.dataSize->ZoneSizingInput, &ZoneSizingInputData::ZoneName);
                                if (ObjIndex > 0) {
                                    thisMechVentZone.ZoneDesignSpecOAObjIndex = state.dataSize->ZoneSizingInput(ObjIndex).ZoneDesignSpecOAIndex;
                                }
                            }
                        }
                        // Zone Air Distribution inputs
                        if (designSpecZoneADObjIndex(groupNum) > 0) {
                            // new DCV inputs
                            thisMechVentZone.ZoneDesignSpecADObjIndex = designSpecZoneADObjIndex(groupNum);
                        } else {
                            if (state.dataGlobal->DoZoneSizing) {
                                int ObjIndex = Util::FindItemInList(
                                    ventMechZoneOrListName(groupNum), state.dataSize->ZoneSizingInput, &ZoneSizingInputData::ZoneName);
                                if (ObjIndex > 0) {
                                    thisMechVentZone.ZoneDesignSpecADObjIndex = state.dataSize->ZoneSizingInput(ObjIndex).ZoneAirDistributionIndex;
                                }
                            }
                        }
                    }
                } else {
                    //       Not a zone name, must be a zone list
                    int ZoneListNum = Util::FindItemInList(ventMechZoneOrListName(groupNum), state.dataHeatBal->ZoneList);
                    if (ZoneListNum > 0) {
                        for (int ScanZoneListNum = 1; ScanZoneListNum <= state.dataHeatBal->ZoneList(ZoneListNum).NumOfZones; ++ScanZoneListNum) {
                            // check to make sure zone name is unique (not listed more than once)...
                            int zoneNum2 = state.dataHeatBal->ZoneList(ZoneListNum).Zone(ScanZoneListNum);
                            if (std::any_of(thisVentilationMechanical.VentMechZone.begin(),
                                            thisVentilationMechanical.VentMechZone.end(),
                                            [zoneNum2](auto const &vmZone) { return vmZone.zoneNum == zoneNum2; })) {
                                //             Disregard duplicate zone names, show warning and do not store data for this zone
                                ShowWarningError(state,
                                                 format("Zone name = {} in ZoneList = {} for {} object = {}",
                                                        state.dataHeatBal->Zone(zoneNum2).Name,
                                                        ventMechZoneOrListName(groupNum),
                                                        CurrentModuleObject,
                                                        thisVentilationMechanical.Name));
                                ShowContinueError(state, "is a duplicate. The first ventilation values specified for this zone will be used ");
                                ShowContinueError(state, "and the rest will be ignored. The simulation will continue...");
                            } else {
                                //           Store data for each zone name from zone list (duplicate zone names accounted for in
                                //           HeatBalanceManager)
                                ++MechVentZoneCount;
                                auto &thisMechVentZone = thisVentilationMechanical.VentMechZone(MechVentZoneCount);
                                thisMechVentZone.zoneNum = zoneNum2;
                                thisMechVentZone.name = state.dataHeatBal->Zone(zoneNum2).Name;
                                // Populating new temp array to hold design spec OA object for each zone
                                if (designSpecOAObjIndex(groupNum) > 0) {
                                    thisMechVentZone.ZoneDesignSpecOAObjIndex = designSpecOAObjIndex(groupNum);
                                } else {
                                    if (state.dataGlobal->DoZoneSizing) {
                                        int ObjIndex = Util::FindItemInList(
                                            state.dataHeatBal->Zone(zoneNum2).Name, state.dataSize->ZoneSizingInput, &ZoneSizingInputData::ZoneName);
                                        if (ObjIndex > 0) {
                                            thisMechVentZone.ZoneDesignSpecOAObjIndex =
                                                state.dataSize->ZoneSizingInput(ObjIndex).ZoneDesignSpecOAIndex;
                                        }
                                    }
                                }

                                if (designSpecZoneADObjIndex(groupNum) > 0) {
                                    // new DCV inputs
                                    thisMechVentZone.ZoneDesignSpecADObjIndex = designSpecZoneADObjIndex(groupNum);
                                } else {
                                    if (state.dataGlobal->DoZoneSizing) {
                                        int ObjIndex = Util::FindItemInList(
                                            state.dataHeatBal->Zone(zoneNum2).Name, state.dataSize->ZoneSizingInput, &ZoneSizingInputData::ZoneName);
                                        if (ObjIndex > 0) {
                                            thisMechVentZone.ZoneDesignSpecADObjIndex =
                                                state.dataSize->ZoneSizingInput(ObjIndex).ZoneAirDistributionIndex;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (MechVentZoneCount <= 0) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", invalid input. At least one Zone or ZoneList Name must be entered.",
                                       RoutineName,
                                       CurrentModuleObject,
                                       thisVentilationMechanical.Name));
                ErrorsFound = true;
            }

            //   Overwrite previous number of zones with number that does not include duplicates
            thisVentilationMechanical.NumofVentMechZones = MechVentZoneCount;

            // Loop over zones and fill OA and AD specs, if none were found, use defaults
            for (int ventMechZoneNum = 1; ventMechZoneNum <= MechVentZoneCount; ++ventMechZoneNum) {
                auto &thisVentMechZone = thisVentilationMechanical.VentMechZone(ventMechZoneNum);
                if (thisVentMechZone.ZoneDesignSpecOAObjIndex == 0) {
                    // use defaults
                    thisVentMechZone.ZoneDesignSpecOAObjIndex = DataSizing::getDefaultOAReq(state);
                    ShowWarningError(state, format("{}{}=\"{}", RoutineName, CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(
                        state, format("Cannot locate a matching DesignSpecification:OutdoorAir object for Zone=\"{}\".", thisVentMechZone.name));
                    ShowContinueError(state, "Using default OA of 0.00944 m3/s-person and 0.0 m3/s-m2.");
                }
                assert(thisVentMechZone.ZoneDesignSpecOAObjIndex > 0);
                auto &curOARequirements = state.dataSize->OARequirements(thisVentMechZone.ZoneDesignSpecOAObjIndex);
                Real64 zoneOAAreaRate = curOARequirements.desFlowPerZoneArea(state, thisVentMechZone.zoneNum);
                Real64 zoneOAPeopleRate = curOARequirements.desFlowPerZonePerson(state, thisVentMechZone.zoneNum);
                // Get OA schedules - only used for system methods
                if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQP ||
                    thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQPGC ||
                    thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::IAQPCOM) {
                    bool notAllSame = false;
                    thisVentMechZone.zoneOASched = curOARequirements.getZoneFlowFracSched(state, notAllSame);
                    if (notAllSame) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", mixed schedules found for Zone={}.",
                                                RoutineName,
                                                CurrentModuleObject,
                                                thisVentilationMechanical.Name,
                                                thisVentMechZone.name));
                    }
                }
                if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc ||
                    thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc ||
                    thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                    bool notAllSame = false;
                    thisVentMechZone.zonePropCtlMinRateSched = curOARequirements.getZonePropCtlMinRateSched(state, notAllSame);
                    if (notAllSame) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", mixed schedules found for Zone={}.",
                                                RoutineName,
                                                CurrentModuleObject,
                                                thisVentilationMechanical.Name,
                                                thisVentMechZone.name));
                    }
                }
                if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                    if (zoneOAPeopleRate == 0.0 && zoneOAAreaRate == 0.0) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", invalid input with System Outdoor Air Method = ProportionalControlBasedOnDesignOARate.",
                                               RoutineName,
                                               CurrentModuleObject,
                                               thisVentilationMechanical.Name));
                        ShowContinueError(state,
                                          " The values of Outdoor Air Flow per Person and Outdoor Air Flow per Zone Floor Area in the same "
                                          "object can not be zero.");
                        ErrorsFound = true;
                    }
                }
                if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc) {
                    if (curOARequirements.desFlowPerACH(state) > 0.0 || curOARequirements.desFlowPerZone(state) > 0.0) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", inappropriate outdoor air method", CurrentModuleObject, thisVentilationMechanical.Name));
                        ShowContinueError(
                            state, format("Inappropriate method for Design Specification Outdoor Air Object Name=\"{}\".", curOARequirements.Name));
                        ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                        ShowContinueError(state,
                                          "Since System Outdoor Air Method= ProportionalControlBasedOnOccupancySchedule\", AirChanges/Hour or "
                                          "Flow/Zone outdoor air methods are not valid. Simulation continues.... ");
                    }
                }
                if (thisVentilationMechanical.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc) {
                    if (curOARequirements.desFlowPerACH(state) > 0.0 || curOARequirements.desFlowPerZone(state) > 0.0) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", inappropriate outdoor air method", CurrentModuleObject, thisVentilationMechanical.Name));
                        ShowContinueError(
                            state, format("Inappropriate method for Design Specification Outdoor Air Object Name=\"{}\".", curOARequirements.Name));
                        ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                        ShowContinueError(state,
                                          "Since System Outdoor Air Method= ProportionalControlBasedOnDesignOccupancy\", AirChanges/Hour or "
                                          "Flow/Zone outdoor air methods are not valid. Simulation continues.... ");
                    }
                }
                int zoneAirDistObjIndex = thisVentMechZone.ZoneDesignSpecADObjIndex;
                if (zoneAirDistObjIndex > 0) {
                    auto const &curZoneAirDistribution(state.dataSize->ZoneAirDistribution(zoneAirDistObjIndex));
                    thisVentMechZone.ZoneADEffCooling = curZoneAirDistribution.ZoneADEffCooling;
                    thisVentMechZone.ZoneADEffHeating = curZoneAirDistribution.ZoneADEffHeating;
                    thisVentMechZone.zoneADEffSched = curZoneAirDistribution.zoneADEffSched;
                    thisVentMechZone.ZoneSecondaryRecirculation = curZoneAirDistribution.ZoneSecondaryRecirculation;
                } else { // use defaults
                    thisVentMechZone.ZoneADEffCooling = 1.0;
                    thisVentMechZone.ZoneADEffHeating = 1.0;
                    thisVentMechZone.ZoneSecondaryRecirculation = 0.0;
                    ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(
                        state,
                        format("Cannot locate a matching DesignSpecification:ZoneAirDistribution object for Zone=\"{}\".", thisVentMechZone.name));
                    ShowContinueError(state, "Using default zone air distribution effectiveness of 1.0 for heating and cooling.");
                }

                // Error check to see if a single duct air terminal is assigned to a zone that has zone secondary recirculation
                if (thisVentMechZone.ZoneSecondaryRecirculation > 0.0) {
                    int ZoneNum = thisVentMechZone.zoneNum;
                    if (ZoneNum > 0) {
                        int EquipListIndex = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).EquipListIndex;
                        if (EquipListIndex > 0) {
                            for (int EquipListNum = 1; EquipListNum <= state.dataZoneEquip->NumOfZoneEquipLists; ++EquipListNum) {
                                if (EquipListNum == EquipListIndex) {
                                    for (int EquipNum = 1; EquipNum <= state.dataZoneEquip->ZoneEquipList(EquipListNum).NumOfEquipTypes; ++EquipNum) {
                                        if (Util::SameString(state.dataZoneEquip->ZoneEquipList(EquipListNum).EquipTypeName(EquipNum),
                                                             "ZONEHVAC:AIRDISTRIBUTIONUNIT")) {
                                            for (int ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                                                if (Util::SameString(state.dataZoneEquip->ZoneEquipList(EquipListNum).EquipName(EquipNum),
                                                                     state.dataDefineEquipment->AirDistUnit(ADUNum).Name)) {
                                                    if ((state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolNoReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVNoReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheatVSFan) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVNoReheat) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolCooledBeam) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolFourPipeBeam) ||
                                                        (state.dataDefineEquipment->AirDistUnit(ADUNum).EquipTypeEnum(EquipNum) ==
                                                         DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir)) {
                                                        ShowWarningError(state,
                                                                         format("{}=\"{}\", inappropriate use of Zone secondary recirculation",
                                                                                CurrentModuleObject,
                                                                                thisVentilationMechanical.Name));
                                                        ShowContinueError(state,
                                                                          "A zone secondary recirculation fraction is specified for zone served by ");
                                                        ShowContinueError(state,
                                                                          format("...terminal unit \"{}\" , that indicates a single path system",
                                                                                 state.dataDefineEquipment->AirDistUnit(ADUNum).Name));
                                                        ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                                                        ShowContinueError(state, "...The zone secondary recirculation for that zone was set to 0.0");
                                                        thisVentMechZone.ZoneSecondaryRecirculation = 0.0;
                                                    }
                                                    goto EquipLoop_exit;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        EquipLoop_exit:;
                        }
                    }
                }
                if (zoneOAPeopleRate <= 0.0 && thisVentilationMechanical.DCVFlag) {
                    ShowWarningError(state, format("{}=\"{}\", Zone OA/person rate", CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                    ShowContinueError(state,
                                      format("Zone outside air per person rate not set in Design Specification Outdoor Air Object=\"{}\".",
                                             state.dataSize->OARequirements(thisVentMechZone.ZoneDesignSpecOAObjIndex).Name));
                }

                if (zoneOAAreaRate < 0.0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid Outdoor Air flow per area", CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                    ShowContinueError(state,
                                      format("invalid Outdoor Air flow per area specified in object=\"{}\". Value must be >= 0.0.",
                                             state.dataSize->OARequirements(thisVentMechZone.ZoneDesignSpecOAObjIndex).Name));
                    ErrorsFound = true;
                }
                if (zoneOAPeopleRate < 0.0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid Outdoor Air flow per person", CurrentModuleObject, thisVentilationMechanical.Name));
                    ShowContinueError(state, format("For Zone=\"{}\".", thisVentMechZone.name));
                    ShowContinueError(state,
                                      format("invalid Outdoor Air flow per person specified in object \"{}\". Value must be >= 0.0.",
                                             state.dataSize->OARequirements(thisVentMechZone.ZoneDesignSpecOAObjIndex).Name));
                    ErrorsFound = true;
                }
            }
        }

        // Link OA controller object with mechanical ventilation object
        for (int OAControllerNum = 1; OAControllerNum <= state.dataMixedAir->NumOAControllers; ++OAControllerNum) {
            state.dataMixedAir->OAController(OAControllerNum).VentMechObjectNum = Util::FindItemInList(
                state.dataMixedAir->OAController(OAControllerNum).VentilationMechanicalName, state.dataMixedAir->VentilationMechanical);
            if (state.dataMixedAir->OAController(OAControllerNum).VentMechObjectNum == 0 &&
                !state.dataMixedAir->OAController(OAControllerNum).VentilationMechanicalName.empty()) {
                ShowSevereError(state,
                                format("{}=\"{}\", non-match to Controller:OutdoorAir",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAController(OAControllerNum).VentilationMechanicalName));
                ShowContinueError(
                    state, format("Invalid specified in Controller:OutdoorAir object = {}", state.dataMixedAir->OAController(OAControllerNum).Name));
                ShowContinueError(state,
                                  format("{} object name must match the {} object name specified in Controller:OutdoorAir.",
                                         CurrentModuleObject,
                                         CurrentModuleObject));
                ErrorsFound = true;
            }
        }

        // write to .eio file
        static constexpr std::string_view Format_700(
            "!<Controller:MechanicalVentilation>,Name,Availability Schedule Name,Demand Controlled Ventilation "
            "{Yes/No},System Outdoor Air Method,Zone Maximum Outdoor Air Fraction,Number of Zones,Zone Name,DSOA "
            "Name,DSZAD Name");
        print(state.files.eio, "{}\n", Format_700);
        for (auto const &ventMech : state.dataMixedAir->VentilationMechanical) {
            print(state.files.eio, " Controller:MechanicalVentilation,{},{},", ventMech.Name, ventMech.availSched ? ventMech.availSched->Name : "");

            print(state.files.eio, format("{},", yesNoNames[(int)ventMech.DCVFlag]));

            if (ventMech.SystemOAMethod != DataSizing::SysOAMethod::Invalid) {
                print(state.files.eio, printSysOAMethod[(int)ventMech.SystemOAMethod]);
            } else {
                print(state.files.eio, "Invalid/Unknown,");
            }

            print(state.files.eio, "{:.2R},", ventMech.ZoneMaxOAFraction);
            print(state.files.eio, "{}", ventMech.NumofVentMechZones);

            for (int jZone = 1; jZone <= ventMech.NumofVentMechZones; ++jZone) {
                auto &thisVentMechZone = ventMech.VentMechZone(jZone);
                std::string const dsoaName = thisVentMechZone.ZoneDesignSpecOAObjIndex > 0
                                                 ? state.dataSize->OARequirements(thisVentMechZone.ZoneDesignSpecOAObjIndex).Name
                                                 : "";
                std::string const adName = thisVentMechZone.ZoneDesignSpecADObjIndex > 0
                                               ? state.dataSize->ZoneAirDistribution(thisVentMechZone.ZoneDesignSpecADObjIndex).Name
                                               : "";
                print(state.files.eio, ",{},{},{}", state.dataHeatBal->Zone(thisVentMechZone.zoneNum).Name, dsoaName, adName);
            }
            print(state.files.eio, "\n");
        }

    } // Number of Mechanical Ventilation Objects > 0

    AlphArray.deallocate();
    NumArray.deallocate();
    lNumericBlanks.deallocate();
    lAlphaBlanks.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found when getting {} inputs.", RoutineName, CurrentModuleObject));
    }
}

void AllocateOAControllers(EnergyPlusData &state)
{
    // PURPOSE OF THIS SUBROUTINE:
    // Allocate the OA controller arrays which are shared by Controller:OutdoorAir and ZoneHVAC:EnergyRecoveryVentilator:Controller

    if (state.dataMixedAir->AllocateOAControllersFlag) {
        state.dataMixedAir->NumOAControllers =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObjects[static_cast<int>(CMO::OAController)]);
        state.dataMixedAir->NumERVControllers =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObjects[static_cast<int>(CMO::ERVController)]);
        state.dataMixedAir->NumOAControllers += state.dataMixedAir->NumERVControllers;
        state.dataMixedAir->OAController.allocate(state.dataMixedAir->NumOAControllers);
        state.dataMixedAir->OAControllerUniqueNames.reserve(static_cast<unsigned>(state.dataMixedAir->NumOAControllers));
        state.dataMixedAir->AllocateOAControllersFlag = false;
    }
}

void GetOAMixerInputs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Input the OAMixer data and store it in the OAMixer array.

    // METHODOLOGY EMPLOYED:
    // Use the Get routines from the InputProcessor module.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetOAMixerInputs: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    int NumNums;                   // Number of REAL(r64) numbers returned by GetObjectItem
    int NumAlphas;                 // Number of alphanumerics returned by GetObjectItem
    int NumArg;                    // Number of arguments from GetObjectDefMaxArgs call
    Array1D<Real64> NumArray;      // array that holds numeric input values
    Array1D_string AlphArray;      // array that holds alpha input values
    Array1D_string cAlphaFields;   // Alpha field names
    Array1D_string cNumericFields; // Numeric field names
    Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
    bool ErrorsFound(false);

    if (!state.dataMixedAir->GetOAMixerInputFlag) {
        return;
    }

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, CurrentModuleObjects[static_cast<int>(CMO::OAMixer)], NumArg, NumAlphas, NumNums);

    AlphArray.allocate(NumAlphas);
    NumArray.dimension(NumNums, 0.0);
    lNumericBlanks.dimension(NumNums, true);
    lAlphaBlanks.dimension(NumAlphas, true);
    cAlphaFields.allocate(NumAlphas);
    cNumericFields.allocate(NumNums);

    std::string_view const CurrentModuleObject = CurrentModuleObjects[static_cast<int>(CMO::OAMixer)];

    state.dataMixedAir->NumOAMixers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    if (state.dataMixedAir->NumOAMixers > 0) {

        state.dataMixedAir->OAMixer.allocate(state.dataMixedAir->NumOAMixers);
        int IOStat;

        for (int OutAirNum = 1; OutAirNum <= state.dataMixedAir->NumOAMixers; ++OutAirNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     OutAirNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);
            // no need to check if AlphaArray(1) is empty since Json will catch missing required fields
            state.dataMixedAir->OAMixer(OutAirNum).Name = AlphArray(1);
            state.dataMixedAir->OAMixer(OutAirNum).MixNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                                 AlphArray(2),
                                                                                                 ErrorsFound,
                                                                                                 DataLoopNode::ConnectionObjectType::OutdoorAirMixer,
                                                                                                 AlphArray(1),
                                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                                 ObjectIsNotParent);
            //  Set connection type to 'Inlet', because this is not necessarily directly from
            //  outside air.  Outside Air Inlet Node List will set the connection to outside air
            state.dataMixedAir->OAMixer(OutAirNum).InletNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::OutdoorAirMixer,
                                                    AlphArray(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    ObjectIsNotParent);
            state.dataMixedAir->OAMixer(OutAirNum).RelNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                                 AlphArray(4),
                                                                                                 ErrorsFound,
                                                                                                 DataLoopNode::ConnectionObjectType::OutdoorAirMixer,
                                                                                                 AlphArray(1),
                                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                                 DataLoopNode::ConnectionType::ReliefAir,
                                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                                 ObjectIsNotParent);
            state.dataMixedAir->OAMixer(OutAirNum).RetNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                                 AlphArray(5),
                                                                                                 ErrorsFound,
                                                                                                 DataLoopNode::ConnectionObjectType::OutdoorAirMixer,
                                                                                                 AlphArray(1),
                                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                                 DataLoopNode::ConnectionType::Inlet,
                                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                                 ObjectIsNotParent);
            // Check for dupes in the four nodes.
            if (state.dataMixedAir->OAMixer(OutAirNum).MixNode == state.dataMixedAir->OAMixer(OutAirNum).InletNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(3),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).InletNode),
                                       cAlphaFields(2)));
                ErrorsFound = true;
            } else if (state.dataMixedAir->OAMixer(OutAirNum).MixNode == state.dataMixedAir->OAMixer(OutAirNum).RelNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(4),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).RelNode),
                                       cAlphaFields(2)));
                ErrorsFound = true;
            } else if (state.dataMixedAir->OAMixer(OutAirNum).MixNode == state.dataMixedAir->OAMixer(OutAirNum).RetNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(5),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).RetNode),
                                       cAlphaFields(2)));
                ErrorsFound = true;
            }

            if (state.dataMixedAir->OAMixer(OutAirNum).InletNode == state.dataMixedAir->OAMixer(OutAirNum).RelNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(4),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).RelNode),
                                       cAlphaFields(3)));
                ErrorsFound = true;
            } else if (state.dataMixedAir->OAMixer(OutAirNum).InletNode == state.dataMixedAir->OAMixer(OutAirNum).RetNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(5),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).RetNode),
                                       cAlphaFields(3)));
                ErrorsFound = true;
            }

            if (state.dataMixedAir->OAMixer(OutAirNum).RelNode == state.dataMixedAir->OAMixer(OutAirNum).RetNode) {
                ShowSevereError(state,
                                format("{} = {} {} = {} duplicates the {}.",
                                       CurrentModuleObject,
                                       state.dataMixedAir->OAMixer(OutAirNum).Name,
                                       cAlphaFields(5),
                                       state.dataLoopNodes->NodeID(state.dataMixedAir->OAMixer(OutAirNum).RetNode),
                                       cAlphaFields(4)));
                ErrorsFound = true;
            }
            BranchNodeConnections::TestCompSet(
                state, CurrentModuleObject, state.dataMixedAir->OAMixer(OutAirNum).Name, AlphArray(3), AlphArray(2), "Air Nodes");
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found in getting {}", RoutineName, CurrentModuleObject));
    }

    state.dataMixedAir->GetOAMixerInputFlag = false;
}

void ProcessOAControllerInputs(EnergyPlusData &state,
                               std::string_view const CurrentModuleObject,
                               int const OutAirNum,
                               Array1D_string const &AlphArray,
                               int const NumAlphas,
                               Array1D<Real64> const &NumArray,
                               int const NumNums,
                               Array1D_bool const &lNumericBlanks, // Unused
                               Array1D_bool const &lAlphaBlanks,
                               Array1D_string const &cAlphaFields,
                               Array1D_string const &cNumericFields, // Unused
                               bool &ErrorsFound                     // If errors found in input
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998
    //       MODIFIED       Shirey/Raustad FSEC, June 2003, Jan 2004
    //                      Mangesh Basarkar, 06/2011: Getting zone OA specifications from Design Specification Object
    //                      Tianzhen Hong, 3/2012: getting zone air distribution effectiveness and secondary recirculation
    //                       from DesignSpecification:ZoneAirDistribution objects
    //       RE-ENGINEERED  MJW: Split out processing controller:outdoorair input to facilitate unit testing, Feb 2015

    // PURPOSE OF THIS SUBROUTINE
    // Input the OAController data and store it in the OAController array.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetOAControllerInputs: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetOAControllerInputs";

    ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

    state.dataMixedAir->OAController(OutAirNum).Name = AlphArray(1);
    state.dataMixedAir->OAController(OutAirNum).ControllerType = MixedAirControllerType::ControllerOutsideAir;
    state.dataMixedAir->OAController(OutAirNum).MaxOA = NumArray(2);
    state.dataMixedAir->OAController(OutAirNum).MinOA = NumArray(1);
    state.dataMixedAir->OAController(OutAirNum).MixNode =
        NodeInputManager::GetOnlySingleNode(state,
                                            AlphArray(4),
                                            ErrorsFound,
                                            DataLoopNode::ConnectionObjectType::ControllerOutdoorAir,
                                            AlphArray(1),
                                            DataLoopNode::NodeFluidType::Air,
                                            DataLoopNode::ConnectionType::Sensor,
                                            NodeInputManager::CompFluidStream::Primary,
                                            ObjectIsNotParent);
    state.dataMixedAir->OAController(OutAirNum).OANode = NodeInputManager::GetOnlySingleNode(state,
                                                                                             AlphArray(5),
                                                                                             ErrorsFound,
                                                                                             DataLoopNode::ConnectionObjectType::ControllerOutdoorAir,
                                                                                             AlphArray(1),
                                                                                             DataLoopNode::NodeFluidType::Air,
                                                                                             DataLoopNode::ConnectionType::Actuator,
                                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                                             ObjectIsNotParent);
    if (!OutAirNodeManager::CheckOutAirNodeNumber(state, state.dataMixedAir->OAController(OutAirNum).OANode)) {
        ShowWarningError(state,
                         format("{}=\"{}\": {}=\"{}\" is not an OutdoorAir:Node.", CurrentModuleObject, AlphArray(1), cAlphaFields(5), AlphArray(5)));
        ShowContinueError(state, "Confirm that this is the intended source for the outdoor air stream.");
    }
    if (Util::SameString(AlphArray(6), "NoEconomizer")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::NoEconomizer;
    } else if (Util::SameString(AlphArray(6), "FixedDryBulb")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::FixedDryBulb;
    } else if (Util::SameString(AlphArray(6), "FixedEnthalpy")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::FixedEnthalpy;
    } else if (Util::SameString(AlphArray(6), "FixedDewPointAndDryBulb")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::FixedDewPointAndDryBulb;
    } else if (Util::SameString(AlphArray(6), "DifferentialDryBulb")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::DifferentialDryBulb;
    } else if (Util::SameString(AlphArray(6), "DifferentialEnthalpy")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::DifferentialEnthalpy;
    } else if (Util::SameString(AlphArray(6), "DifferentialDryBulbAndEnthalpy")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::DifferentialDryBulbAndEnthalpy;
    } else if (Util::SameString(AlphArray(6), "ElectronicEnthalpy")) {
        state.dataMixedAir->OAController(OutAirNum).Econo = EconoOp::ElectronicEnthalpy;
    } else {
        ShowSevereError(state, format("{}=\"{}\" invalid {}=\"{}\" value.", CurrentModuleObject, AlphArray(1), cAlphaFields(6), AlphArray(6)));
        ErrorsFound = true;
    }
    // Bypass choice - Added by Amit for new feature implementation
    if (Util::SameString(AlphArray(7), "ModulateFlow")) {
        state.dataMixedAir->OAController(OutAirNum).EconBypass = false;
    } else if (Util::SameString(AlphArray(7), "MinimumFlowWithBypass")) {
        state.dataMixedAir->OAController(OutAirNum).EconBypass = true;
    } else {
        ShowSevereError(state, format("{}=\"{}\" invalid {}=\"{}\" value.", CurrentModuleObject, AlphArray(1), cAlphaFields(7), AlphArray(7)));
        ErrorsFound = true;
    }

    if (Util::SameString(AlphArray(9), "NoLockout")) {
        state.dataMixedAir->OAController(OutAirNum).Lockout = LockoutType::NoLockoutPossible;
    } else if (Util::SameString(AlphArray(9), "LockoutWithHeating")) {
        state.dataMixedAir->OAController(OutAirNum).Lockout = LockoutType::LockoutWithHeatingPossible;
    } else if (Util::SameString(AlphArray(9), "LockoutWithCompressor")) {
        state.dataMixedAir->OAController(OutAirNum).Lockout = LockoutType::LockoutWithCompressorPossible;
    } else {
        ShowSevereError(state, format("{}=\"{}\" invalid {}=\"{}\" value.", CurrentModuleObject, AlphArray(1), cAlphaFields(9), AlphArray(9)));
        ErrorsFound = true;
    }
    if (Util::SameString(AlphArray(10), "FixedMinimum")) {
        state.dataMixedAir->OAController(OutAirNum).FixedMin = true;
    } else {
        state.dataMixedAir->OAController(OutAirNum).FixedMin = false;
    }
    if (lNumericBlanks(3)) {
        state.dataMixedAir->OAController(OutAirNum).TempLim = HVAC::BlankNumeric;
    } else {
        state.dataMixedAir->OAController(OutAirNum).TempLim = NumArray(3);
    }

    if (lNumericBlanks(4)) {
        state.dataMixedAir->OAController(OutAirNum).EnthLim = HVAC::BlankNumeric;
    } else {
        state.dataMixedAir->OAController(OutAirNum).EnthLim = NumArray(4);
    }
    if (lNumericBlanks(5)) {
        state.dataMixedAir->OAController(OutAirNum).DPTempLim = HVAC::BlankNumeric;
    } else {
        state.dataMixedAir->OAController(OutAirNum).DPTempLim = NumArray(5);
    }

    if (lNumericBlanks(6)) {
        state.dataMixedAir->OAController(OutAirNum).TempLowLim = HVAC::BlankNumeric;
    } else {
        state.dataMixedAir->OAController(OutAirNum).TempLowLim = NumArray(6);
    }

    if (!lAlphaBlanks(8)) {
        state.dataMixedAir->OAController(OutAirNum).EnthalpyCurvePtr = Curve::GetCurveIndex(state, AlphArray(8)); // convert curve name to number
        if (state.dataMixedAir->OAController(OutAirNum).EnthalpyCurvePtr == 0) {
            ShowSevereError(state,
                            format("{}=\"{}\" invalid {}=\"{}\" not found.", CurrentModuleObject, AlphArray(1), cAlphaFields(8), AlphArray(8)));
            ErrorsFound = true;
        } else {
            // Verify Curve Object, only legal types are Quadratic and Cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 state.dataMixedAir->OAController(OutAirNum).EnthalpyCurvePtr, // Curve index
                                                 {1},                                                          // Valid dimensions
                                                 RoutineName,                                                  // Routine name
                                                 CurrentModuleObject,                                          // Object Type
                                                 state.dataMixedAir->OAController(OutAirNum).Name,             // Object Name
                                                 cAlphaFields(8));                                             // Field Name
        }
    }

    state.dataMixedAir->OAController(OutAirNum).RelNode =
        NodeInputManager::GetOnlySingleNode(state,
                                            AlphArray(2),
                                            ErrorsFound,
                                            DataLoopNode::ConnectionObjectType::ControllerOutdoorAir,
                                            AlphArray(1),
                                            DataLoopNode::NodeFluidType::Air,
                                            DataLoopNode::ConnectionType::Actuator,
                                            NodeInputManager::CompFluidStream::Primary,
                                            ObjectIsNotParent);
    state.dataMixedAir->OAController(OutAirNum).RetNode =
        NodeInputManager::GetOnlySingleNode(state,
                                            AlphArray(3),
                                            ErrorsFound,
                                            DataLoopNode::ConnectionObjectType::ControllerOutdoorAir,
                                            AlphArray(1),
                                            DataLoopNode::NodeFluidType::Air,
                                            DataLoopNode::ConnectionType::Sensor,
                                            NodeInputManager::CompFluidStream::Primary,
                                            ObjectIsNotParent);

    if (lAlphaBlanks(11)) {
    } else if ((state.dataMixedAir->OAController(OutAirNum).minOASched = Sched::GetSchedule(state, AlphArray(11))) == nullptr) {
        ShowSevereItemNotFound(state, eoh, cAlphaFields(11), AlphArray(11));
        ErrorsFound = true;
    }

    // Changed by Amit for new feature implementation
    if (lAlphaBlanks(12)) {
    } else if ((state.dataMixedAir->OAController(OutAirNum).minOAflowSched = Sched::GetSchedule(state, AlphArray(12))) == nullptr) {
        ShowSevereItemNotFound(state, eoh, cAlphaFields(12), AlphArray(12));
        ErrorsFound = true;
    }

    if (lAlphaBlanks(13)) {
    } else if ((state.dataMixedAir->OAController(OutAirNum).maxOAflowSched = Sched::GetSchedule(state, AlphArray(13))) == nullptr) {
        ShowSevereItemNotFound(state, eoh, cAlphaFields(13), AlphArray(13));
        ErrorsFound = true;
    }
    state.dataMixedAir->OAController(OutAirNum).VentilationMechanicalName = AlphArray(14);

    //   Check for a time of day economizer control schedule
    state.dataMixedAir->OAController(OutAirNum).economizerOASched = Sched::GetSchedule(state, AlphArray(15));

    //   High humidity control option can be used with any economizer flag
    if (Util::SameString(AlphArray(16), "Yes")) {

        state.dataMixedAir->OAController(OutAirNum).HumidistatZoneNum = Util::FindItemInList(AlphArray(17), state.dataHeatBal->Zone);

        // Get the node number for the zone with the humidistat
        if (state.dataMixedAir->OAController(OutAirNum).HumidistatZoneNum > 0) {
            bool AirNodeFound = false;
            bool AirLoopFound = false;
            bool OASysFound = false;
            for (int ControlledZoneNum = 1; ControlledZoneNum <= state.dataGlobal->NumOfZones; ++ControlledZoneNum) {
                if (ControlledZoneNum != state.dataMixedAir->OAController(OutAirNum).HumidistatZoneNum) {
                    continue;
                }
                //           Find the controlled zone number for the specified humidistat location
                state.dataMixedAir->OAController(OutAirNum).NodeNumofHumidistatZone =
                    state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //           Determine which OA System uses this OA Controller
                int OASysIndex = 0;
                for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
                    for (int OAControllerNum = 1; OAControllerNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumControllers; ++OAControllerNum) {
                        if (!Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ControllerType(OAControllerNum), CurrentModuleObject) ||
                            !Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ControllerName(OAControllerNum),
                                              state.dataMixedAir->OAController(OutAirNum).Name)) {
                            continue;
                        }
                        OASysIndex = OASysNum;
                        OASysFound = true;
                        break;
                    }
                    if (OASysFound) {
                        break;
                    }
                }
                //           Determine if controller is on air loop served by the humidistat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    if (AirLoopNumber > 0 && OASysIndex > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      state.dataAirLoop->OutsideAirSys(OASysIndex).Name) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        "AirLoopHVAC:OutdoorAirSystem")) {
                                    continue;
                                }
                                AirLoopFound = true;
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (int HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                            if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum !=
                                state.dataMixedAir->OAController(OutAirNum).HumidistatZoneNum) {
                                continue;
                            }
                            AirNodeFound = true;
                            break;
                        }
                    } else {
                        if (OASysIndex == 0) {
                            ShowSevereError(
                                state,
                                format("Did not find an AirLoopHVAC:OutdoorAirSystem for {} = \"{}\"",
                                       MixedAirControllerTypeNames[static_cast<int>(state.dataMixedAir->OAController(OutAirNum).ControllerType)],
                                       state.dataMixedAir->OAController(OutAirNum).Name));
                            ErrorsFound = true;
                        }
                    }
                }
            }
            if (!AirNodeFound) {
                ShowSevereError(state,
                                format("Did not find Air Node (Zone with Humidistat), {} = \"{}\"",
                                       MixedAirControllerTypeNames[static_cast<int>(state.dataMixedAir->OAController(OutAirNum).ControllerType)],
                                       state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("Specified {} = {}", cAlphaFields(17), AlphArray(17)));
                ShowContinueError(state,
                                  "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Humidistat object must be specified for this zone.");
                ErrorsFound = true;
            }
            if (!AirLoopFound) {
                ShowSevereError(state,
                                format("Did not find correct Primary Air Loop for {} = \"{}\"",
                                       MixedAirControllerTypeNames[static_cast<int>(state.dataMixedAir->OAController(OutAirNum).ControllerType)],
                                       state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("{} = {} is not served by this Primary Air Loop equipment.", cAlphaFields(17), AlphArray(17)));
                ErrorsFound = true;
            }
        } else {
            ShowSevereError(state,
                            format("Did not find Air Node (Zone with Humidistat), {} = \"{}\"",
                                   MixedAirControllerTypeNames[static_cast<int>(state.dataMixedAir->OAController(OutAirNum).ControllerType)],
                                   state.dataMixedAir->OAController(OutAirNum).Name));
            ShowContinueError(state, format("Specified {} = {}", cAlphaFields(17), AlphArray(17)));
            ShowContinueError(state,
                              "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Humidistat object must be specified for this zone.");
            ErrorsFound = true;
        }

        state.dataMixedAir->OAController(OutAirNum).HighRHOAFlowRatio = NumArray(7);
        if (state.dataMixedAir->OAController(OutAirNum).HighRHOAFlowRatio <= 0.0 && NumNums > 6) {
            ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
            ShowContinueError(state, format(" {} must be greater than 0.", cNumericFields(7)));
            ShowContinueError(state, format(" {} is reset to 1 and the simulation continues.", cNumericFields(7)));
            state.dataMixedAir->OAController(OutAirNum).HighRHOAFlowRatio = 1.0;
        }

        if (Util::SameString(AlphArray(16), "Yes") && state.dataMixedAir->OAController(OutAirNum).FixedMin) {
            if (state.dataMixedAir->OAController(OutAirNum).MaxOA > 0.0 && state.dataMixedAir->OAController(OutAirNum).MinOA != AutoSize) {
                Real64 OAFlowRatio = state.dataMixedAir->OAController(OutAirNum).MinOA / state.dataMixedAir->OAController(OutAirNum).MaxOA;
                if (state.dataMixedAir->OAController(OutAirNum).HighRHOAFlowRatio < OAFlowRatio) {
                    ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
                    ShowContinueError(state, "... A fixed minimum outside air flow rate and high humidity control have been specified.");
                    ShowContinueError(
                        state,
                        format("... The {} is less than the ratio of the outside air controllers minimum to maximum outside air flow rate.",
                               cNumericFields(7)));
                    ShowContinueError(
                        state, format("... Controller {} = {:.4T} m3/s.", cNumericFields(1), state.dataMixedAir->OAController(OutAirNum).MinOA));
                    ShowContinueError(
                        state, format("... Controller {} = {:.4T} m3/s.", cNumericFields(2), state.dataMixedAir->OAController(OutAirNum).MaxOA));
                    ShowContinueError(state, format("... Controller minimum to maximum flow ratio = {:.4T}.", OAFlowRatio));
                    ShowContinueError(state,
                                      format("... {} = {:.4T}.", cNumericFields(7), state.dataMixedAir->OAController(OutAirNum).HighRHOAFlowRatio));
                }
            }
        }

        if (NumAlphas >= 18) {
            if (Util::SameString(AlphArray(18), "Yes")) {
                state.dataMixedAir->OAController(OutAirNum).ModifyDuringHighOAMoisture = false;
            } else if (Util::SameString(AlphArray(18), "No")) {
                state.dataMixedAir->OAController(OutAirNum).ModifyDuringHighOAMoisture = true;
            } else {
                ShowSevereError(state,
                                format("{} \"{}\", invalid field value", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("...{}=\"{}\" - valid values are \"Yes\" or \"No\".", cAlphaFields(18), AlphArray(18)));
                ErrorsFound = true;
            }
        } else {
            if (state.dataMixedAir->OAController(OutAirNum).Econo == EconoOp::NoEconomizer) {
                state.dataMixedAir->OAController(OutAirNum).ModifyDuringHighOAMoisture = true;
            } else {
                state.dataMixedAir->OAController(OutAirNum).ModifyDuringHighOAMoisture = false;
                ShowWarningError(state,
                                 format("{} \"{}\", missing field value", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("...{} will default to Yes when {}= \"Yes\"", cAlphaFields(18), cAlphaFields(16)));
            }
        }

    } else if (Util::SameString(AlphArray(16), "No") || lAlphaBlanks(16)) {
        if (NumAlphas >= 18) {
            if (!Util::SameString(AlphArray(18), "Yes") && !Util::SameString(AlphArray(18), "No")) {
                ShowSevereError(state,
                                format("{} \"{}\", invalid field value", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("...{}=\"{}\" - valid values are \"Yes\" or \"No\".", cAlphaFields(18), AlphArray(18)));
                ErrorsFound = true;
            }
        }
    } else { // Invalid field 16
        ShowSevereError(state, format("{} \"{}\", invalid field value", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
        ShowContinueError(state, format("...{}=\"{}\" - valid values are \"Yes\" or \"No\".", cAlphaFields(16), AlphArray(16)));
        ErrorsFound = true;
        if (NumAlphas >= 18) {
            if (!Util::SameString(AlphArray(18), "Yes") && !Util::SameString(AlphArray(18), "No")) {
                ShowSevereError(state,
                                format("{} \"{}\", invalid field value", CurrentModuleObject, state.dataMixedAir->OAController(OutAirNum).Name));
                ShowContinueError(state, format("...{}=\"{}\" - valid values are \"Yes\" or \"No\".", cAlphaFields(18), AlphArray(18)));
                ErrorsFound = true;
            }
        }
    }

    if (NumAlphas > 18) {
        if (!lAlphaBlanks(19)) {
            if (Util::SameString(AlphArray(19), "BypassWhenWithinEconomizerLimits")) {
                state.dataMixedAir->OAController(OutAirNum).HeatRecoveryBypassControlType = HVAC::BypassWhenWithinEconomizerLimits;
            } else if (Util::SameString(AlphArray(19), "BypassWhenOAFlowGreaterThanMinimum")) {
                state.dataMixedAir->OAController(OutAirNum).HeatRecoveryBypassControlType = HVAC::BypassWhenOAFlowGreaterThanMinimum;
            } else {
                ShowWarningError(state, format("{}=\"{}\" invalid {}=\"{}\".", CurrentModuleObject, AlphArray(1), cAlphaFields(19), AlphArray(19)));
                ShowContinueError(state, "...assuming \"BypassWhenWithinEconomizerLimits\" and the simulation continues.");
                state.dataMixedAir->OAController(OutAirNum).HeatRecoveryBypassControlType = HVAC::BypassWhenWithinEconomizerLimits;
            }
        }
    }

    if (NumAlphas > 19) {
        if (!lAlphaBlanks(20)) {
            if (Util::SameString(AlphArray(20), "EconomizerFirst")) {
                state.dataMixedAir->OAController(OutAirNum).EconomizerStagingType = HVAC::EconomizerStagingType::EconomizerFirst;
            } else {
                state.dataMixedAir->OAController(OutAirNum).EconomizerStagingType = HVAC::EconomizerStagingType::InterlockedWithMechanicalCooling;
            }
        }
    }

    if (Util::SameString(AlphArray(16), "Yes") && state.dataMixedAir->OAController(OutAirNum).Econo == EconoOp::NoEconomizer) {
        ShowWarningError(state,
                         format("{} \"{}\"",
                                MixedAirControllerTypeNames[static_cast<int>(state.dataMixedAir->OAController(OutAirNum).ControllerType)],
                                state.dataMixedAir->OAController(OutAirNum).Name));
        ShowContinueError(state, format("...Economizer operation must be enabled when {} is set to YES.", cAlphaFields(16)));
        ShowContinueError(state, "...The high humidity control option will be disabled and the simulation continues.");
    }

    state.dataMixedAir->OAController(OutAirNum).MixedAirSPMNum =
        SetPointManager::GetMixedAirNumWithCoilFreezingCheck(state, state.dataMixedAir->OAController(OutAirNum).MixNode);
}

// End of Get Input subroutines for the Module
//******************************************************************************

// Beginning Initialization Section of the Module
//******************************************************************************

void InitOutsideAirSys(EnergyPlusData &state, int const(OASysNum), int const AirLoopNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Initialize the OutsideAirSys data structure

    if (state.dataAirLoop->OutsideAirSys(OASysNum).AirLoopDOASNum > -1) {
        return;
    }

    if (state.dataMixedAir->initOASysFlag(OASysNum)) {
        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysNum = OASysNum;
        state.dataMixedAir->initOASysFlag(OASysNum) = false;
    }
}

void InitOAController(EnergyPlusData &state, int const OAControllerNum, bool const FirstHVACIteration, int const AirLoopNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998
    //       MODIFIED       Shirey/Raustad FSEC, June/Aug 2003, Feb 2004
    //                      Tianzhen Hong, Feb 2009 for DCV
    //                      Tianzhen Hong, Aug 2013 for economizer faults

    // PURPOSE OF THIS SUBROUTINE
    // Initialize the OAController data structure with input node data

    bool ErrorsFound = false;

    auto &thisOAController(state.dataMixedAir->OAController(OAControllerNum));

    if (state.dataMixedAir->InitOAControllerOneTimeFlag) {
        state.dataMixedAir->OAControllerMyOneTimeFlag.dimension(state.dataMixedAir->NumOAControllers, true);
        state.dataMixedAir->OAControllerMyEnvrnFlag.dimension(state.dataMixedAir->NumOAControllers, true);
        state.dataMixedAir->OAControllerMySizeFlag.dimension(state.dataMixedAir->NumOAControllers, true);
        state.dataMixedAir->MechVentCheckFlag.dimension(state.dataMixedAir->NumOAControllers, true);
        state.dataMixedAir->InitOAControllerSetPointCheckFlag.dimension(state.dataMixedAir->NumOAControllers, true);
        state.dataMixedAir->InitOAControllerOneTimeFlag = false;
    }
    if (state.dataMixedAir->OAControllerMyOneTimeFlag(OAControllerNum)) {
        // Determine Inlet node index for OAController, not a user input for controller, but is obtained from OutsideAirSys and OAMixer
        switch (thisOAController.ControllerType) {
        case MixedAirControllerType::ControllerOutsideAir: {
            int thisOASys = 0;
            for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
                // find which OAsys has this controller
                int found = Util::FindItemInList(thisOAController.Name,
                                                 state.dataAirLoop->OutsideAirSys(OASysNum).ControllerName,
                                                 isize(state.dataAirLoop->OutsideAirSys(OASysNum).ControllerName));
                if (found != 0) {
                    thisOASys = OASysNum;
                    state.dataAirLoop->OutsideAirSys(thisOASys).OAControllerIndex = GetOAController(state, thisOAController.Name);
                    break; // we found it
                }
            }
            if (thisOASys == 0) {
                ShowSevereError(state, format("InitOAController: Did not find OAController=\"{}\".", thisOAController.Name));
                ShowContinueError(state, "in list of valid OA Controllers.");
                ErrorsFound = true;
            }
            int thisNumForMixer = Util::FindItem(CurrentModuleObjects[static_cast<int>(CMO::OAMixer)],
                                                 state.dataAirLoop->OutsideAirSys(thisOASys).ComponentType,
                                                 isize(state.dataAirLoop->OutsideAirSys(thisOASys).ComponentType));
            if (thisNumForMixer != 0) {
                std::string_view const equipName = state.dataAirLoop->OutsideAirSys(thisOASys).ComponentName(thisNumForMixer);
                int thisMixerIndex = Util::FindItemInList(equipName, state.dataMixedAir->OAMixer);
                if (thisMixerIndex != 0) {
                    thisOAController.InletNode = state.dataMixedAir->OAMixer(thisMixerIndex).InletNode;
                } else {
                    ShowSevereError(state, format("InitOAController: Did not find OAMixer=\"{}\".", equipName));
                    ShowContinueError(state, "in list of valid OA Mixers.");
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, "InitOAController: Did not find OutdoorAir:Mixer Component=\"OutdoorAir:Mixer\".");
                ShowContinueError(state, "in list of valid OA Components.");
                ErrorsFound = true;
            }

            if (thisOAController.InletNode == 0) { // throw an error
                ShowSevereError(
                    state,
                    format("InitOAController: Failed to find proper inlet node for OutdoorAir:Mixer and Controller = {}", thisOAController.Name));
                ErrorsFound = true;
            }
        } break;
        case MixedAirControllerType::ControllerStandAloneERV: {
            // set the inlet node to also equal the OA node because this is a special controller for economizing stand alone ERV
            // with the assumption that equipment is bypassed....
            thisOAController.InletNode = thisOAController.OANode;
        } break;
        default: {
            ShowSevereError(state,
                            format("InitOAController: Failed to find ControllerType: {}",
                                   MixedAirControllerTypeNames[static_cast<int>(thisOAController.ControllerType)]));
            ErrorsFound = true;
        } break;
        }

        state.dataMixedAir->OAControllerMyOneTimeFlag(OAControllerNum) = false;
    }

    if (!state.dataGlobal->SysSizingCalc && state.dataMixedAir->InitOAControllerSetPointCheckFlag(OAControllerNum) &&
        state.dataHVACGlobal->DoSetPointTest && !FirstHVACIteration) {
        int MixedAirNode = thisOAController.MixNode;
        if (MixedAirNode > 0) {
            //      IF (OAController(OAControllerNum)%Econo == 1 .AND. .NOT. AirLoopControlInfo(AirLoopNum)%CyclingFan) THEN
            if (thisOAController.Econo > EconoOp::NoEconomizer && state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AnyContFan) {
                if (state.dataLoopNodes->Node(MixedAirNode).TempSetPoint == SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowSevereError(state, format("MixedAir: Missing temperature setpoint for economizer controller {}", thisOAController.Name));
                        ShowContinueError(state, format("Node Referenced (by Controller)={}", state.dataLoopNodes->NodeID(MixedAirNode)));
                        ShowContinueError(
                            state, "  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at the mixed air node.");
                        state.dataHVACGlobal->SetPointErrorFlag = true;
                    } else {
                        // add call to check node in EMS
                        EMSManager::CheckIfNodeSetPointManagedByEMS(
                            state, MixedAirNode, HVAC::CtrlVarType::Temp, state.dataHVACGlobal->SetPointErrorFlag);
                        if (state.dataHVACGlobal->SetPointErrorFlag) {
                            ShowSevereError(state,
                                            format("MixedAir: Missing temperature setpoint for economizer controller {}", thisOAController.Name));
                            ShowContinueError(state, format("Node Referenced (by Controller)={}", state.dataLoopNodes->NodeID(MixedAirNode)));
                            ShowContinueError(state,
                                              "  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at the "
                                              "mixed air node.");
                            ShowContinueError(state, "Or add EMS Actuator to provide temperature setpoint at this node");
                        }
                    }
                }
            }
        }

        state.dataMixedAir->InitOAControllerSetPointCheckFlag(OAControllerNum) = false;
    }

    if (!state.dataGlobal->SysSizingCalc && state.dataMixedAir->OAControllerMySizeFlag(OAControllerNum)) {
        thisOAController.SizeOAController(state);
        if (AirLoopNum > 0) {
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OACtrlNum = OAControllerNum;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OACtrlName = thisOAController.Name;
            if (thisOAController.Lockout == LockoutType::LockoutWithHeatingPossible) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithHeating = true;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor = false;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanNotLockoutEcono = false;
            } else if (thisOAController.Lockout == LockoutType::LockoutWithCompressorPossible) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithHeating = false;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor = true;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanNotLockoutEcono = false;
            } else {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithHeating = false;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor = false;
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanNotLockoutEcono = true;
            }
        }
        if ((thisOAController.MaxOA - thisOAController.MinOA) < -HVAC::SmallAirVolFlow) {
            ShowSevereError(state, format("For Controller:OutdoorAir: {}", thisOAController.Name));
            ShowContinueError(state,
                              format("  maximum outdoor air flow rate ({:.4R}) < minimum outdoor air flow rate ({:.4R})",
                                     thisOAController.MaxOA,
                                     thisOAController.MinOA));
            ShowContinueError(state,
                              "  To set the minimum outside air flow rate use the \"Design (minimum) outdoor air flow rate\" field in the "
                              "Sizing:System object");
            ErrorsFound = true;
        }

        if (AirLoopNum > 0) {
            Real64 DesSupplyVolFlowRate = state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply / state.dataEnvrn->StdRhoAir;
            if ((thisOAController.MinOA - DesSupplyVolFlowRate) > 0.0001) {
                ShowWarningError(state,
                                 format("InitOAController: Minimum Outdoor Air Flow Rate for Controller:OutdoorAir={} is greater than Design Supply "
                                        "Air Flow Rate for AirLoopHVAC={}.",
                                        thisOAController.Name,
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Name));
                ShowContinueError(state,
                                  format("...Minimum Outdoor Air Flow Rate={:.6R} will be reset to loop Design Supply Air Flow Rate={:.6R}",
                                         thisOAController.MinOA,
                                         DesSupplyVolFlowRate));
                thisOAController.MinOA = DesSupplyVolFlowRate;
            } else if ((thisOAController.MinOA - DesSupplyVolFlowRate) > 0.0) {
                // If difference is tiny, reset silently
                thisOAController.MinOA = DesSupplyVolFlowRate;
            }
            if ((thisOAController.MaxOA - DesSupplyVolFlowRate) > 0.0001) {
                ShowWarningError(state,
                                 format("InitOAController: Maximum Outdoor Air Flow Rate for Controller:OutdoorAir={} is greater than Design Supply "
                                        "Air Flow Rate for AirLoopHVAC={}.",
                                        thisOAController.Name,
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Name));
                ShowContinueError(state,
                                  format("...Maximum Outdoor Air Flow Rate={:.6R} will be reset to loop Design Supply Air Flow Rate={:.6R}",
                                         thisOAController.MaxOA,
                                         DesSupplyVolFlowRate));
                thisOAController.MaxOA = DesSupplyVolFlowRate;
            } else if ((thisOAController.MaxOA - DesSupplyVolFlowRate) > 0.0) {
                // If difference is tiny, reset silently
                thisOAController.MaxOA = DesSupplyVolFlowRate;
            }

            // Check if system has a Sizing:System object and a sizing run has been done
            bool SizingDesRunThisAirSys = false;
            CheckThisAirSystemForSizing(state, AirLoopNum, SizingDesRunThisAirSys);

            // Get design outdoor air flow rate
            if (SizingDesRunThisAirSys && thisOAController.VentMechObjectNum > 0) {
                state.dataMixedAir->VentilationMechanical(thisOAController.VentMechObjectNum).SysDesOA =
                    state.dataSize->FinalSysSizing(AirLoopNum).DesOutAirVolFlow;
            }
        }

        state.dataMixedAir->OAControllerMySizeFlag(OAControllerNum) = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataMixedAir->OAControllerMyEnvrnFlag(OAControllerNum)) {
        Real64 RhoAirStdInit = state.dataEnvrn->StdRhoAir;
        thisOAController.MinOAMassFlowRate = thisOAController.MinOA * RhoAirStdInit;
        thisOAController.MaxOAMassFlowRate = thisOAController.MaxOA * RhoAirStdInit;
        state.dataMixedAir->OAControllerMyEnvrnFlag(OAControllerNum) = false;
        state.dataLoopNodes->Node(thisOAController.OANode).MassFlowRateMax = thisOAController.MaxOAMassFlowRate;

        // predefined reporting
        if (thisOAController.Econo > EconoOp::NoEconomizer) {
            std::string_view const equipName = thisOAController.Name;
            // 90.1 descriptor for economizer controls. Changed by Amit for New Feature implementation
            if (thisOAController.Econo == EconoOp::DifferentialEnthalpy) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoKind, equipName, "DifferentialEnthalpy");
            } else if (thisOAController.Econo == EconoOp::DifferentialDryBulb) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoKind, equipName, "DifferentialDryBulb");
            } else if (thisOAController.Econo == EconoOp::FixedEnthalpy) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoKind, equipName, "FixedEnthalpy");
            } else if (thisOAController.Econo == EconoOp::FixedDryBulb) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoKind, equipName, "FixedDryBulb");
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoKind, equipName, "Other");
            }

            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoMinOA, equipName, thisOAController.MinOA);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoMaxOA, equipName, thisOAController.MaxOA);
            // EnergyPlus input echos for economizer controls. Changed by Amit for new feature implementation
            if (thisOAController.Econo == EconoOp::DifferentialDryBulb) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "Yes");
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "No");
            }
            if (thisOAController.Econo == EconoOp::DifferentialEnthalpy) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "Yes");
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "No");
            }
            if (thisOAController.Econo == EconoOp::FixedDryBulb) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, thisOAController.TempLim);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "-");
            }
            if (thisOAController.Econo == EconoOp::FixedEnthalpy) {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, thisOAController.EnthLim);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoRetTemp, equipName, "-");
            }
            for (auto curOaSys : state.dataAirLoop->OutsideAirSys) {
                for (auto curCtrlName : curOaSys.ControllerName) {
                    if (curCtrlName == thisOAController.Name) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEcoOAsysNm, equipName, curOaSys.Name);
                    }
                }
            }
        }
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataMixedAir->OAControllerMyEnvrnFlag(OAControllerNum) = true;
    }

    if (state.dataMixedAir->MechVentCheckFlag(OAControllerNum)) {
        // Make these checks only once at the beginning of the simulation

        // Make sure all air loop zones and air loop zones with people objects are covered by mechanical ventilation
        // Issue a warning only if the zone is not accounted for in the associated mechanical ventilation object
        if (thisOAController.VentMechObjectNum > 0) {
            auto &vent_mech(state.dataMixedAir->VentilationMechanical(thisOAController.VentMechObjectNum));

            // Make sure all zones with mechanical ventilation are on the correct air loop
            int TempMechVentArrayCounter = 0;
            for (int NumMechVentZone = 1; NumMechVentZone <= vent_mech.NumofVentMechZones; ++NumMechVentZone) {
                auto &thisMechVentZone = vent_mech.VentMechZone(NumMechVentZone);
                int ZoneNum = thisMechVentZone.zoneNum;
                auto const &zone(state.dataHeatBal->Zone(ZoneNum));
                bool FoundZone = false;

                for (int AirLoopZoneInfoZoneNum = 1; AirLoopZoneInfoZoneNum <= state.dataAirLoop->AirLoopZoneInfo(AirLoopNum).NumZones;
                     ++AirLoopZoneInfoZoneNum) {
                    int NumZone = state.dataAirLoop->AirLoopZoneInfo(AirLoopNum).ActualZoneNumber(AirLoopZoneInfoZoneNum);
                    if (ZoneNum == NumZone) {
                        FoundZone = true;
                        ++TempMechVentArrayCounter;
                        if (TempMechVentArrayCounter < NumMechVentZone) { // Copy to lower index
                            auto &tempMechVentZone = vent_mech.VentMechZone(TempMechVentArrayCounter);
                            tempMechVentZone.zoneNum = thisMechVentZone.zoneNum;
                            tempMechVentZone.ZoneDesignSpecOAObjIndex = thisMechVentZone.ZoneDesignSpecOAObjIndex;
                            tempMechVentZone.zoneOASched = thisMechVentZone.zoneOASched;
                            tempMechVentZone.zonePropCtlMinRateSched = thisMechVentZone.zonePropCtlMinRateSched;

                            // new DCV
                            tempMechVentZone.ZoneDesignSpecADObjIndex = thisMechVentZone.ZoneDesignSpecADObjIndex;
                            tempMechVentZone.ZoneADEffCooling = thisMechVentZone.ZoneADEffCooling;
                            tempMechVentZone.ZoneADEffHeating = thisMechVentZone.ZoneADEffHeating;
                            tempMechVentZone.zoneADEffSched = thisMechVentZone.zoneADEffSched;
                            tempMechVentZone.ZoneSecondaryRecirculation = thisMechVentZone.ZoneSecondaryRecirculation;
                        }
                        break;
                    }
                }
                if (!FoundZone) {
                    ShowWarningError(state,
                                     format("Zone name = {} in {} object name = {} is not on the same air loop as Controller:OutdoorAir = {}",
                                            zone.Name,
                                            CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)],
                                            thisOAController.VentilationMechanicalName,
                                            thisOAController.Name));
                    ShowContinueError(state, "This zone will not be used and the simulation will continue...");
                }
            }

            // Shrink final arrays to conserve environment space
            if (TempMechVentArrayCounter < vent_mech.NumofVentMechZones) {
                vent_mech.VentMechZone.resize(TempMechVentArrayCounter);
                vent_mech.NumofVentMechZones = TempMechVentArrayCounter;
            }

            // predefined report
            for (int jZone = 1; jZone <= vent_mech.NumofVentMechZones; ++jZone) {
                auto &thisMechVentZone = vent_mech.VentMechZone(jZone);
                std::string const zoneName = state.dataHeatBal->Zone(thisMechVentZone.zoneNum).Name;
                auto &dsoa = state.dataSize->OARequirements(thisMechVentZone.ZoneDesignSpecOAObjIndex);

                // Loop through spaces if DesignSpecification:OutdoorAir:Spacelist, or just once for simple DSOA
                auto writeDSOAToPredefined = [&state, &vent_mech, &thisMechVentZone](const OARequirementsData &dsoa,
                                                                                     const std::string &zoneOrSpaceName) {
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDCVventMechName, zoneOrSpaceName, vent_mech.Name);
                    OutputReportPredefined::PreDefTableEntry(state,
                                                             state.dataOutRptPredefined->pdchDCVType,
                                                             zoneOrSpaceName,
                                                             SysOAMethodNames[static_cast<int>(vent_mech.SystemOAMethod)]);
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchDCVperPerson, zoneOrSpaceName, dsoa.OAFlowPerPerson, 6);
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchDCVperArea, zoneOrSpaceName, dsoa.OAFlowPerArea, 6);
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchDCVperZone, zoneOrSpaceName, dsoa.OAFlowPerZone, 6);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDCVperACH, zoneOrSpaceName, dsoa.OAFlowACH, 6);
                    OutputReportPredefined::PreDefTableEntry(state,
                                                             state.dataOutRptPredefined->pdchDCVMethod,
                                                             zoneOrSpaceName,
                                                             OAFlowCalcMethodNames[static_cast<int>(dsoa.OAFlowMethod)]);
                    if (dsoa.oaFlowFracSched != nullptr) {
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchDCVOASchName, zoneOrSpaceName, dsoa.oaFlowFracSched->Name);
                    }
                    if (thisMechVentZone.zoneADEffSched != nullptr) {
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchDCVZoneADEffSchName, zoneOrSpaceName, thisMechVentZone.zoneADEffSched->Name);
                    } else {
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchDCVZoneADEffCooling, zoneOrSpaceName, thisMechVentZone.ZoneADEffCooling, 2);
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchDCVZoneADEffHeating, zoneOrSpaceName, thisMechVentZone.ZoneADEffHeating, 2);
                    }
                };
                if (dsoa.numDSOA == 0) {
                    writeDSOAToPredefined(dsoa, zoneName);
                } else {
                    for (int n = 0; n < dsoa.numDSOA; ++n) {
                        writeDSOAToPredefined(state.dataSize->OARequirements(dsoa.dsoaIndexes[n]), format("{}:{}", zoneName, dsoa.dsoaSpaceNames[n]));
                    }
                }
            }

            // Fill People index lists if needed
            if (vent_mech.SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc) {
                for (int peopleNum = 1; peopleNum <= state.dataHeatBal->TotPeople; ++peopleNum) {
                    for (auto &thisMechVentZone : vent_mech.VentMechZone) {
                        if (state.dataHeatBal->People(peopleNum).ZonePtr == thisMechVentZone.zoneNum) {
                            thisMechVentZone.peopleIndexes.push_back(peopleNum);
                            break;
                        }
                    }
                }
            }

            // Check to see if any zones on an air loop are not accounted for by a mechanical ventilation object
            for (int AirLoopZoneInfoZoneNum = 1; AirLoopZoneInfoZoneNum <= state.dataAirLoop->AirLoopZoneInfo(AirLoopNum).NumZones;
                 ++AirLoopZoneInfoZoneNum) {
                int NumZone = state.dataAirLoop->AirLoopZoneInfo(AirLoopNum).ActualZoneNumber(AirLoopZoneInfoZoneNum);
                bool FoundAreaZone = false;
                bool FoundPeopleZone = false;
                for (int NumMechVentZone = 1; NumMechVentZone <= vent_mech.NumofVentMechZones; ++NumMechVentZone) {
                    auto const &thisMechVentZone = vent_mech.VentMechZone(NumMechVentZone);
                    int ZoneNum = thisMechVentZone.zoneNum;
                    if (ZoneNum == NumZone) {
                        FoundAreaZone = true;
                        if (state.dataSize->OARequirements(thisMechVentZone.ZoneDesignSpecOAObjIndex).desFlowPerZonePerson(state, ZoneNum) > 0.0) {
                            FoundPeopleZone = true;
                        }
                        break;
                    }
                }
                if (!FoundAreaZone) {
                    ShowWarningError(state,
                                     format("Zone name = {} is not accounted for by {} object name = {}",
                                            state.dataHeatBal->Zone(NumZone).Name,
                                            CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)],
                                            thisOAController.VentilationMechanicalName));
                    ShowContinueError(state, "Ventilation per unit floor area has not been specified for this zone, which is connected to");
                    ShowContinueError(
                        state, format("the air loop served by Controller:OutdoorAir = {}. Simulation will continue...", thisOAController.Name));
                }
                if (!FoundPeopleZone) {
                    // Loop through people objects to see if this zone has a people object and only then show a warning
                    for (int PeopleNum = 1; PeopleNum <= state.dataHeatBal->TotPeople; ++PeopleNum) {
                        if (state.dataHeatBal->People(PeopleNum).ZonePtr == NumZone) {
                            if (!FoundAreaZone) {
                                ShowWarningError(state,
                                                 format("PEOPLE object for zone = {} is not accounted for by {} object name = {}",
                                                        state.dataHeatBal->Zone(NumZone).Name,
                                                        CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)],
                                                        thisOAController.VentilationMechanicalName));
                                ShowContinueError(
                                    state,
                                    format(
                                        "A \"PEOPLE\" object has been specified in the idf for this zone, but it is not included in this {} Object.",
                                        CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)]));
                                ShowContinueError(state,
                                                  format("Check {} object. Simulation will continue.",
                                                         CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)]));
                            }
                        }
                    }
                } else { // People > 0, check to make sure there is a people statement in the zone
                    FoundAreaZone = false;
                    for (int PeopleNum = 1; PeopleNum <= state.dataHeatBal->TotPeople; ++PeopleNum) {
                        if (state.dataHeatBal->People(PeopleNum).ZonePtr != NumZone) {
                            continue;
                        }
                        FoundAreaZone = true;
                        break;
                    }
                    if (!FoundAreaZone) {
                        ShowWarningError(state,
                                         format("{} = \"{}\", Zone=\"{}\".",
                                                CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)],
                                                thisOAController.VentilationMechanicalName,
                                                state.dataHeatBal->Zone(NumZone).Name));
                        ShowContinueError(state,
                                          "No \"PEOPLE\" object has been specified in the idf for this zone, but the ventilation rate is > 0 in "
                                          "this Controller:MechanicalVentilation Object.");
                        ShowContinueError(state, "Check ventilation rate in Controller:MechanicalVentilation object.  Simulation will continue.");
                    }
                }
            }
        }

        state.dataMixedAir->MechVentCheckFlag(OAControllerNum) = false;
    }
    //****

    // Perform a one time initialization of AirloopHVAC OA System report variables
    // If AirloopHVAC objects are used, NumPrimaryAirSys > 0 and the initialization proceeds and then sets
    // SetUpAirLoopHVACVariables to .FALSE. so this is never done again and only the first IF is checked
    // each time through Init. If for some reason the primary air system have not yet been read in, this
    // code waits for the air loop data to be available before performing the report variable initialization.
    // If AirloopHVAC objects are not used, NumPrimaryAirSys is always equal to 0 and only these
    // two IF statements are checked each time through Init (e.g., if StandAloneERV controllers are used
    // without AirloopHVAC objects).
    if (state.dataMixedAir->InitOAControllerSetUpAirLoopHVACVariables) {
        if (AirLoopNum > 0) {
            // Added code to report (TH, 10/20/2008):
            //   air economizer status (1 = on, 0 = off or does not exist), and actual and minimum outside air fraction (0 to 1)
            std::string airloopName; // Temporary equipment name
            for (int OAControllerLoop = 1; OAControllerLoop <= state.dataMixedAir->NumOAControllers; ++OAControllerLoop) {
                auto &loopOAController(state.dataMixedAir->OAController(OAControllerLoop));

                // Find the outside air system that has the OA controller
                if (loopOAController.ControllerType == MixedAirControllerType::ControllerStandAloneERV) {
                    continue; // ERV controller not on airloop
                }
                bool OASysFound = false;
                int thisOASys = 0;
                for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
                    for (int OAControllerLoop2 = 1; OAControllerLoop2 <= state.dataAirLoop->OutsideAirSys(OASysNum).NumControllers;
                         ++OAControllerLoop2) {
                        if (Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ControllerName(OAControllerLoop2), loopOAController.Name)) {
                            thisOASys = OASysNum;
                            OASysFound = true;
                            break;
                        }
                    }
                    if (OASysFound) {
                        break;
                    }
                }

                int airLoopNum = 0;
                bool AirLoopFound = false;
                if (thisOASys <= 0) {
                    // Check outside air system name
                    ShowWarningError(state, format("Cannot find the AirLoopHVAC:OutdoorAirSystem for the OA Controller: {}", loopOAController.Name));
                } else {
                    // Find the primary air loop that has the outside air system
                    for (int thisAirLoop = 1; thisAirLoop <= state.dataHVACGlobal->NumPrimaryAirSys; ++thisAirLoop) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(thisAirLoop).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(thisAirLoop).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(thisAirLoop).Branch(BranchNum).Comp(CompNum).Name,
                                                      state.dataAirLoop->OutsideAirSys(thisOASys).Name) ||
                                    !Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(thisAirLoop).Branch(BranchNum).Comp(CompNum).TypeOf,
                                                      "AirLoopHVAC:OutdoorAirSystem")) {
                                    continue;
                                }
                                AirLoopFound = true;
                                airLoopNum = thisAirLoop;
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        if (AirLoopFound) {
                            break;
                        }
                    }
                }
                // Check primary air loop name
                if (AirLoopFound && airLoopNum > 0) {
                    airloopName = state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Name; // OutsideAirSys(OASysIndex)%Name
                } else {
                    ShowWarningError(state, format("Cannot find the primary air loop for the OA Controller: {}", loopOAController.Name));
                    airloopName = "AirLoop not found";
                }

                //    Note use of OAControllerLoop here to keep DO Loop index separate from InitOAController local variable
                // CurrentModuleObject='AirLoopHVAC'
                SetupOutputVariable(state,
                                    "Air System Outdoor Air Economizer Status",
                                    Constant::Units::None,
                                    loopOAController.EconomizerStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Heat Recovery Bypass Status",
                                    Constant::Units::None,
                                    loopOAController.HeatRecoveryBypassStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Heat Recovery Bypass Heating Coil Activity Status",
                                    Constant::Units::None,
                                    loopOAController.HRHeatingCoilActive,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);
                SetupOutputVariable(state,
                                    "Air System Outdoor Air Heat Recovery Bypass Minimum Outdoor Air Mixed Air Temperature",
                                    Constant::Units::C,
                                    loopOAController.MixedAirTempAtMinOAFlow,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air High Humidity Control Status",
                                    Constant::Units::None,
                                    loopOAController.HighHumCtrlStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Limiting Factor",
                                    Constant::Units::None,
                                    loopOAController.OALimitingFactorReport,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Flow Fraction",
                                    Constant::Units::None,
                                    loopOAController.OAFractionRpt,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Minimum Flow Fraction",
                                    Constant::Units::None,
                                    loopOAController.MinOAFracLimit,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Outdoor Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    loopOAController.OAMassFlow,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Mixed Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    loopOAController.MixMassFlow,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Relief Air Heat Transfer Rate",
                                    Constant::Units::W,
                                    loopOAController.RelTotalLossRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Relief Air Sensible Heat Transfer Rate",
                                    Constant::Units::W,
                                    loopOAController.RelSensiLossRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                SetupOutputVariable(state,
                                    "Air System Relief Air Latent Heat Transfer Rate",
                                    Constant::Units::W,
                                    loopOAController.RelLatentLossRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airloopName);

                if (loopOAController.MixedAirSPMNum > 0) {
                    SetupOutputVariable(state,
                                        "Air System Outdoor Air Maximum Flow Fraction",
                                        Constant::Units::None,
                                        loopOAController.MaxOAFracBySetPoint,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        airloopName);
                }

                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSInternalVariable(
                        state, "Outdoor Air Controller Maximum Mass Flow Rate", loopOAController.Name, "[kg/s]", loopOAController.MaxOAMassFlowRate);
                    SetupEMSInternalVariable(
                        state, "Outdoor Air Controller Minimum Mass Flow Rate", loopOAController.Name, "[kg/s]", loopOAController.MinOAMassFlowRate);
                    SetupEMSActuator(state,
                                     "Outdoor Air Controller",
                                     loopOAController.Name,
                                     "Air Mass Flow Rate",
                                     "[kg/s]",
                                     loopOAController.EMSOverrideOARate,
                                     loopOAController.EMSOARateValue);
                }

                if (loopOAController.VentMechObjectNum > 0 && airLoopNum > 0) {
                    SetupOutputVariable(state,
                                        "Air System Outdoor Air Mechanical Ventilation Requested Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        loopOAController.MechVentOAMassFlowRequest,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        airloopName);
                    if (!state.dataMixedAir->VentilationMechanical(loopOAController.VentMechObjectNum).DCVFlag) {
                        state.dataAirLoop->AirLoopControlInfo(airLoopNum).AirLoopDCVFlag = false;
                    }
                }
            }

            state.dataMixedAir->InitOAControllerSetUpAirLoopHVACVariables = false;
        }
    }

    // Each time step
    if (FirstHVACIteration) {
        // Mixed air setpoint. Set by a setpoint manager.
        if (thisOAController.ControllerType == MixedAirControllerType::ControllerOutsideAir) {
            if (state.dataLoopNodes->Node(thisOAController.MixNode).TempSetPoint > SensedNodeFlagValue) {
                thisOAController.MixSetTemp = state.dataLoopNodes->Node(thisOAController.MixNode).TempSetPoint;
            } else {
                thisOAController.MixSetTemp = thisOAController.TempLowLim;
            }
        } else {
            // Stand Alone ERV does not require a temperature setpoint schedule, make setpoint equal to lower economizer limit
            thisOAController.MixSetTemp = thisOAController.TempLowLim;
        }
    }

    // Each iteration

    if (thisOAController.ControllerType == MixedAirControllerType::ControllerOutsideAir) {
        // zone exhaust mass flow is saved in AirLoopFlow%ZoneExhaust
        // the zone exhaust mass flow that is said to be balanced by simple air flows is saved in AirLoopFlow%ZoneExhaustBalanced
        if (AirLoopNum > 0) {
            thisOAController.ExhMassFlow =
                max(0.0, state.dataAirLoop->AirLoopFlow(AirLoopNum).SupFlow - state.dataAirLoop->AirLoopFlow(AirLoopNum).SysRetFlow);
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ZoneExhMassFlow = thisOAController.ExhMassFlow;
            if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).LoopFlowRateSet && !FirstHVACIteration) {
                // if flow rate has been specified by a manager, set it to the specified value
                thisOAController.MixMassFlow =
                    state.dataAirLoop->AirLoopFlow(AirLoopNum).ReqSupplyFrac * state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
                // state.dataLoopNodes->Node(thisOAController.RetNode).MassFlowRate = thisOAController.MixMassFlow - thisOAController.ExhMassFlow;
            } else {
                thisOAController.MixMassFlow = state.dataLoopNodes->Node(thisOAController.RetNode).MassFlowRate + thisOAController.ExhMassFlow;

                // The following was commented out after discussion on PR 7382, it can be reopened for discussion anytime
                // found this equation results in flow that exceeds the design flow rate when there is exhaust flow rate is greater than
                // the design supply air flow rate. Capped the mixed air flow rate at design supply air flow rate, issue #77379
                // thisOAController.MixMassFlow = Node(thisOAController.RetNode).MassFlowRate + thisOAController.ExhMassFlow;
                // thisOAController.MixMassFlow =
                //     min(Node(thisOAController.RetNode).MassFlowRate + thisOAController.ExhMassFlow, AirLoopFlow(AirLoopNum).DesSupply);
            }
        } else {
            thisOAController.ExhMassFlow = 0.0;
            thisOAController.MixMassFlow = state.dataLoopNodes->Node(thisOAController.RetNode).MassFlowRate;
        }
        if (state.dataLoopNodes->Node(thisOAController.MixNode).MassFlowRateMaxAvail <= 0.0) {
            thisOAController.MixMassFlow = 0.0;
        }
    } else {
        // Mixed and exhaust flow rates are passed through to model CONTROLLER:STAND ALONE ERV in SimOAController
        thisOAController.OAMassFlow = thisOAController.MaxOAMassFlowRate;
        thisOAController.MixMassFlow = thisOAController.MaxOAMassFlowRate;
        thisOAController.ExhMassFlow = state.dataLoopNodes->Node(thisOAController.RetNode).MassFlowRate;
    }
    thisOAController.ExhMassFlow = max(thisOAController.ExhMassFlow, 0.0);

    // Outside air values
    thisOAController.OATemp = state.dataLoopNodes->Node(thisOAController.OANode).Temp;
    thisOAController.OAEnth = state.dataLoopNodes->Node(thisOAController.OANode).Enthalpy;
    thisOAController.OAPress = state.dataLoopNodes->Node(thisOAController.OANode).Press;
    thisOAController.OAHumRat = state.dataLoopNodes->Node(thisOAController.OANode).HumRat;

    // Inlet air values (on OA input side)
    thisOAController.InletTemp = state.dataLoopNodes->Node(thisOAController.InletNode).Temp;
    thisOAController.InletEnth = state.dataLoopNodes->Node(thisOAController.InletNode).Enthalpy;
    thisOAController.InletPress = state.dataLoopNodes->Node(thisOAController.InletNode).Press;
    thisOAController.InletHumRat = state.dataLoopNodes->Node(thisOAController.InletNode).HumRat;

    // Return air values
    thisOAController.RetTemp = state.dataLoopNodes->Node(thisOAController.RetNode).Temp;
    thisOAController.RetEnth = state.dataLoopNodes->Node(thisOAController.RetNode).Enthalpy;

    // Check sensors faults for the air economizer
    EconoOp iEco = thisOAController.Econo;
    if (state.dataFaultsMgr->AnyFaultsInModel && (iEco != EconoOp::NoEconomizer)) {
        for (int i = 1; i <= thisOAController.NumFaultyEconomizer; ++i) {
            int j = thisOAController.EconmizerFaultNum(i);
            Real64 rSchVal = 0.0;
            if (state.dataFaultsMgr->FaultsEconomizer(j).availSched->getCurrentVal() > 0.0) {
                rSchVal = 1.0;
                if (state.dataFaultsMgr->FaultsEconomizer(j).severitySched != nullptr) {
                    rSchVal = state.dataFaultsMgr->FaultsEconomizer(j).severitySched->getCurrentVal();
                }
            } else {
                continue; // no fault
            }

            Real64 rOffset = rSchVal * state.dataFaultsMgr->FaultsEconomizer(j).Offset;

            if (std::abs(rOffset) < 0.000000001) {
                continue;
            }

            // ECONOMIZER - outdoor air dry-bulb temperature sensor offset
            switch (iEco) {
            case EconoOp::FixedDryBulb:
            case EconoOp::DifferentialDryBulb:
            case EconoOp::FixedDewPointAndDryBulb:
            case EconoOp::ElectronicEnthalpy:
            case EconoOp::DifferentialDryBulbAndEnthalpy: {
                if (state.dataFaultsMgr->FaultsEconomizer(j).type == FaultType::TemperatureSensorOffset_OutdoorAir) {
                    // FaultModel:TemperatureSensorOffset:OutdoorAir
                    thisOAController.OATemp += rOffset;
                    thisOAController.InletTemp += rOffset;
                }
            } break;
            default:
                break;
            }

            // ECONOMIZER - outdoor air humidity ratio sensor offset. really needed ???
            switch (iEco) {
            case EconoOp::FixedDewPointAndDryBulb:
            case EconoOp::ElectronicEnthalpy: {
                if (state.dataFaultsMgr->FaultsEconomizer(j).type == FaultType::HumiditySensorOffset_OutdoorAir) {
                    // FaultModel:HumiditySensorOffset:OutdoorAir
                    thisOAController.OAHumRat += rOffset;
                    thisOAController.InletHumRat += rOffset;
                }
            } break;
            default:
                break;
            }

            // ECONOMIZER - outdoor air enthalpy sensor offset
            switch (iEco) {
            case EconoOp::FixedEnthalpy:
            case EconoOp::ElectronicEnthalpy:
            case EconoOp::DifferentialDryBulbAndEnthalpy: {
                if (state.dataFaultsMgr->FaultsEconomizer(j).type == FaultType::EnthalpySensorOffset_OutdoorAir) {
                    // FaultModel:EnthalpySensorOffset:OutdoorAir
                    thisOAController.OAEnth += rOffset;
                    thisOAController.InletEnth += rOffset;
                }
            } break;
            default:
                break;
            }

            // ECONOMIZER - return air dry-bulb temperature sensor offset
            switch (iEco) {
            case EconoOp::DifferentialDryBulb:
            case EconoOp::DifferentialDryBulbAndEnthalpy: {
                if (state.dataFaultsMgr->FaultsEconomizer(j).type == FaultType::TemperatureSensorOffset_ReturnAir) {
                    // FaultModel:TemperatureSensorOffset:ReturnAir
                    thisOAController.RetTemp += rOffset;
                }
            } break;
            default:
                break;
            }

            // ECONOMIZER - return air enthalpy sensor offset
            switch (iEco) {
            case EconoOp::ElectronicEnthalpy:
            case EconoOp::DifferentialDryBulbAndEnthalpy: {
                if (state.dataFaultsMgr->FaultsEconomizer(j).type == FaultType::EnthalpySensorOffset_ReturnAir) {
                    // FaultModel:EnthalpySensorOffset:ReturnAir
                    thisOAController.RetEnth += rOffset;
                }
            } break;
            default:
                break;
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Error in {}; program terminated", CurrentModuleObjects[static_cast<int>(CMO::OAController)]));
    }
} // namespace MixedAir

void OAMixerProps::InitOAMixer(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Initialize the OAMixer data structure with input node data

    int RetNode = this->RetNode;
    int InletNode = this->InletNode;
    int RelNode = this->RelNode;

    // Return air stream data
    this->RetTemp = state.dataLoopNodes->Node(RetNode).Temp;
    this->RetHumRat = state.dataLoopNodes->Node(RetNode).HumRat;
    this->RetEnthalpy = state.dataLoopNodes->Node(RetNode).Enthalpy;
    this->RetPressure = state.dataLoopNodes->Node(RetNode).Press;
    this->RetMassFlowRate = state.dataLoopNodes->Node(RetNode).MassFlowRate;
    // Outside air stream data
    this->OATemp = state.dataLoopNodes->Node(InletNode).Temp;
    this->OAHumRat = state.dataLoopNodes->Node(InletNode).HumRat;
    this->OAEnthalpy = state.dataLoopNodes->Node(InletNode).Enthalpy;
    this->OAPressure = state.dataLoopNodes->Node(InletNode).Press;
    this->OAMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
    // Relief air data
    this->RelMassFlowRate = state.dataLoopNodes->Node(RelNode).MassFlowRate;
}

void OAControllerProps::CalcOAController(EnergyPlusData &state, int const AirLoopNum, bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998
    //       MODIFIED       Shirey/Raustad FSEC, June 2003
    //                      Tianzhen Hong, Feb 2009 for new DCV
    //                      Brent Griffith ,EMS override of OA rate
    //                      Mangesh Basarkar, 06/2011: Modifying outside air calculation based on DCV flag
    //                      Chandan Sharma, FSEC, 25Aug 2011 - Added ProportionalControl
    //                           to enhance CO2 based DCV control
    //                      Tianzhen Hong, March 2012, zone maximum OA fraction - a TRACE feature
    //                      Tianzhen Hong, March 2012, multi-path VRP based on ASHRAE 62.1-2010

    // PURPOSE OF THIS SUBROUTINE
    // Determine the outside air flow

    // REFERENCES:
    // DOE-2.1E Supplement pages 3.97 - 3.100
    // BLAST User Reference pages 183 - 186
    // ASHRAE Standard 62.1-2010

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcOAController: ");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 OASignal = 0.0;                  // Outside air flow rate fraction (0.0 to 1.0)
    bool AirLoopCyclingFan = false;         // Type of air loop fan (TRUE if Fan:OnOff)
    bool HighHumidityOperationFlag = false; // TRUE if zone humidistat senses a high humidity condition

    if (AirLoopNum > 0) {
        AirLoopCyclingFan = (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp == HVAC::FanOp::Cycling);
    } else {
        AirLoopCyclingFan = false;
    }

    this->OALimitingFactor = OALimitFactor::None; // oa controller limiting factor

    // Check for no flow
    if (this->MixMassFlow <= HVAC::SmallMassFlow) {

        this->OAMassFlow = 0.0;     // outside air mass flow rate
        this->RelMassFlow = 0.0;    // relief air mass flow rate
        this->MixMassFlow = 0.0;    // mixed air mass flow rate
        this->MinOAFracLimit = 0.0; // minimum OA fraction limit

        this->EconomizerStatus = 0;                                                    // economizer status for reporting
        this->HeatRecoveryBypassStatus = 0;                                            // HR bypass status for reporting
        this->HRHeatingCoilActive = 0;                                                 // resets report variable
        this->MixedAirTempAtMinOAFlow = state.dataLoopNodes->Node(this->RetNode).Temp; // track return T
        this->HighHumCtrlStatus = 0;                                                   // high humidity control status for reporting
        this->OAFractionRpt = 0.0;                                                     // actual OA fraction for reporting

        this->EconoActive = false;       // DataAirLoop variable (OA Controllers)
        this->HighHumCtrlActive = false; // DataAirLoop variable (OA Controllers)

        // also reset air loop data for use by other routines
        if (AirLoopNum > 0) {
            auto &curAirLoopControlInfo(state.dataAirLoop->AirLoopControlInfo(AirLoopNum));
            auto &curAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));

            curAirLoopControlInfo.EconoActive = false;        // DataAirLoop variable (AirloopHVAC)
            curAirLoopControlInfo.HeatRecoveryBypass = false; // DataAirLoop variable (AirloopHVAC)
            curAirLoopControlInfo.HighHumCtrlActive = false;  // DataAirLoop variable (AirloopHVAC)
            curAirLoopControlInfo.ResimAirLoopFlag = false;   // DataAirLoop variable (AirloopHVAC)
            curAirLoopFlow.OAFrac = 0.0;                      // DataAirLoop variable (AirloopHVAC)
            curAirLoopFlow.OAMinFrac = 0.0;                   // DataAirLoop variable (AirloopHVAC)
            curAirLoopFlow.MinOutAir = 0.0;
            curAirLoopFlow.OAFlow = 0.0;
        }
        return;
    }

    Real64 OutAirMinFrac = 0.0; // Local variable used to calculate min OA fraction
    if (AirLoopNum > 0) {
        auto const &curAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));
        if (curAirLoopFlow.DesSupply >= HVAC::SmallAirVolFlow) {
            OutAirMinFrac = this->MinOAMassFlowRate / curAirLoopFlow.DesSupply;
        }
    } else {
        if (this->MaxOA >= HVAC::SmallAirVolFlow) {
            OutAirMinFrac = this->MinOA / this->MaxOA;
        }
    }
    Real64 MinOASchedVal = 1.0; // value of the minimum outside air schedule
    if (this->minOASched != nullptr) {
        MinOASchedVal = this->minOASched->getCurrentVal();
        MinOASchedVal = min(max(MinOASchedVal, 0.0), 1.0);
        OutAirMinFrac *= MinOASchedVal;
        this->OALimitingFactor = OALimitFactor::Limits;
    }

    // Get outside air mass flow rate calculated by mechanical ventilation object [kg/s]
    Real64 MechVentOutsideAirMinFrac = 0.0; // fraction of OA specified by mechanical ventilation object
    if (AirLoopNum > 0 && this->VentMechObjectNum != 0) {
        auto const &curAirLoopControlInfo(state.dataAirLoop->AirLoopControlInfo(AirLoopNum));
        auto const &curAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));

        // Get system supply air flow rate
        Real64 SysSA = 0.0; // System supply air mass flow rate [kg/s]
        if (curAirLoopControlInfo.LoopFlowRateSet) {
            // if flow rate has been specified by a manager, set it to the specified value
            // DesSupply and SupFlow are mass flow rate in kg/s
            SysSA = curAirLoopFlow.ReqSupplyFrac * curAirLoopFlow.DesSupply;
        } else {
            SysSA = curAirLoopFlow.SupFlow;
        }

        this->MechVentOAMassFlowRequest = state.dataMixedAir->VentilationMechanical(this->VentMechObjectNum).CalcMechVentController(state, SysSA);
        MechVentOutsideAirMinFrac = this->MechVentOAMassFlowRequest / curAirLoopFlow.DesSupply;
        if (curAirLoopFlow.FanPLR > 0.0) {
            MechVentOutsideAirMinFrac *= curAirLoopFlow.FanPLR;
            this->MechVentOAMassFlowRequest *= curAirLoopFlow.FanPLR;
        }
    } else {
        this->MechVentOAMassFlowRequest = 0.0;
    }
    //****** use greater of Mechanical Ventilation Outside Air fraction and OutAirMinFrac
    if ((MechVentOutsideAirMinFrac > 0.0) && (OutAirMinFrac > MechVentOutsideAirMinFrac)) {
        if (!state.dataGlobal->WarmupFlag) {
            if (this->CountMechVentFrac == 0) {
                ++this->CountMechVentFrac;
                ShowWarningError(
                    state,
                    format("{}Minimum OA fraction > Mechanical Ventilation Controller request for Controller:OutdoorAir={}, Min OA fraction is used.",
                           RoutineName,
                           this->Name));
                ShowContinueError(state,
                                  "This may be overriding desired ventilation controls. Check inputs for Minimum Outdoor Air Flow Rate, Minimum "
                                  "Outdoor Air Schedule Name and Controller:MechanicalVentilation");
                ShowContinueErrorTimeStamp(
                    state, format("Minimum OA fraction = {:.4R}, Mech Vent OA fraction = {:.4R}", OutAirMinFrac, MechVentOutsideAirMinFrac));
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               "Controller:OutdoorAir=\"" + this->Name +
                                                   "\": Min OA fraction > Mechanical ventilation OA fraction, continues...",
                                               this->IndexMechVentFrac,
                                               OutAirMinFrac,
                                               OutAirMinFrac);
            }
        }
    }
    if (MechVentOutsideAirMinFrac > OutAirMinFrac) {
        OutAirMinFrac = MechVentOutsideAirMinFrac;
        this->OALimitingFactor = OALimitFactor::DCV;
    }

    OutAirMinFrac = min(max(OutAirMinFrac, 0.0), 1.0);

    // At this point, OutAirMinFrac is still based on AirLoopFlow.DesSupply
    if (AirLoopNum > 0) {
        auto &curAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));

        curAirLoopFlow.MinOutAir = OutAirMinFrac * curAirLoopFlow.DesSupply;

        // calculate mixed air temp at min OA flow rate
        Real64 ReliefMassFlowAtMinOA = max(curAirLoopFlow.MinOutAir - this->ExhMassFlow, 0.0);
        Real64 RecircMassFlowRateAtMinOAFlow = max(state.dataLoopNodes->Node(this->RetNode).MassFlowRate - ReliefMassFlowAtMinOA, 0.0);
        if ((RecircMassFlowRateAtMinOAFlow + curAirLoopFlow.MinOutAir) > 0.0) {
            Real64 RecircTemp = state.dataLoopNodes->Node(this->RetNode).Temp; // return air temp used for custom economizer control calculation
            this->MixedAirTempAtMinOAFlow =
                (RecircMassFlowRateAtMinOAFlow * RecircTemp + curAirLoopFlow.MinOutAir * state.dataLoopNodes->Node(this->OANode).Temp) /
                (RecircMassFlowRateAtMinOAFlow + curAirLoopFlow.MinOutAir);
        } else {
            this->MixedAirTempAtMinOAFlow = state.dataLoopNodes->Node(this->RetNode).Temp;
        }
    }

    // Economizer
    this->CalcOAEconomizer(state, AirLoopNum, OutAirMinFrac, OASignal, HighHumidityOperationFlag, FirstHVACIteration);

    this->OAMassFlow = OASignal * this->MixMassFlow;

    // Do not allow OA to be below Ventilation:Mechanical flow rate or above mixed mass flow rate
    if (AirLoopNum > 0 && VentMechObjectNum != 0) {
        if (this->MechVentOAMassFlowRequest > this->OAMassFlow) {
            this->OAMassFlow = min(this->MechVentOAMassFlowRequest, this->MixMassFlow);
        }
    }

    // Do not allow OA to be below Exh for controller:outside air
    if (this->ControllerType == MixedAirControllerType::ControllerOutsideAir) {
        if (this->ExhMassFlow > this->OAMassFlow) {
            this->OAMassFlow = this->ExhMassFlow;
            this->OALimitingFactor = OALimitFactor::Exhaust;
        }
    }

    // if fixed minimum, don't let go below min OA
    if (this->FixedMin) {
        // cycling fans allow "average" min OA to be below minimum
        if (!AirLoopCyclingFan) {
            Real64 minOASchedMassFlowRate = this->MinOAMassFlowRate * MinOASchedVal;
            if (minOASchedMassFlowRate > this->OAMassFlow) {
                this->OAMassFlow = minOASchedMassFlowRate;
                this->OALimitingFactor = OALimitFactor::Limits;
            }
        }
    }

    // Apply Minimum Fraction of Outdoor Air Schedule
    if (this->minOAflowSched != nullptr) {
        Real64 MinOAflowfracVal = this->minOAflowSched->getCurrentVal();
        MinOAflowfracVal = min(max(MinOAflowfracVal, 0.0), 1.0);
        OutAirMinFrac = max(MinOAflowfracVal, OutAirMinFrac);
        Real64 minOAFracMassFlowRate = this->MixMassFlow * MinOAflowfracVal;
        if (minOAFracMassFlowRate > this->OAMassFlow) {
            this->OAMassFlow = minOAFracMassFlowRate;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
    }

    // Apply Maximum Fraction of Outdoor Air Schedule
    Real64 currentMaxOAMassFlowRate = this->MaxOAMassFlowRate;
    if (this->maxOAflowSched != nullptr) {
        Real64 MaxOAflowfracVal = this->maxOAflowSched->getCurrentVal();
        MaxOAflowfracVal = min(max(MaxOAflowfracVal, 0.0), 1.0);
        currentMaxOAMassFlowRate = min(this->MaxOAMassFlowRate, this->MixMassFlow * MaxOAflowfracVal);
        OutAirMinFrac = min(MaxOAflowfracVal, OutAirMinFrac);
        if (currentMaxOAMassFlowRate < this->OAMassFlow) {
            this->OAMassFlow = currentMaxOAMassFlowRate;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
    }

    // Don't let the OA flow be > than the max OA limit. OA for high humidity control is allowed to be greater than max OA.
    // Night Ventilation has priority and may override an OASignal > 1 high humidity condition with OASignal = 1
    if (HighHumidityOperationFlag) {
        Real64 maxOAMassFlow = this->MaxOAMassFlowRate * max(1.0, OASignal);
        if (maxOAMassFlow < this->OAMassFlow) {
            this->OAMassFlow = maxOAMassFlow;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
    } else {
        if (this->MaxOAMassFlowRate < this->OAMassFlow) {
            this->OAMassFlow = this->MaxOAMassFlowRate;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
    }

    if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingSizing && (this->ManageDemand) && (this->OAMassFlow > this->DemandLimitFlowRate)) {
        this->OAMassFlow = this->DemandLimitFlowRate;
        this->OALimitingFactor = OALimitFactor::DemandLimit;
    }
    if (this->EMSOverrideOARate) {
        this->OAMassFlow = this->EMSOARateValue;
        this->OALimitingFactor = OALimitFactor::EMS;
    }

    // Don't let OA flow be > mixed air flow.
    // Seems if RAB (return air bypass) that this should be don't let OA flow be > design supply flow but that causes other issues
    if (this->MixMassFlow < this->OAMassFlow) {
        this->OAMassFlow = this->MixMassFlow;
        this->OALimitingFactor = OALimitFactor::MixedAir;
    }

    // save the min outside air flow fraction and max outside air mass flow rate
    if (AirLoopNum > 0) {
        auto &curAirLoopControlInfo(state.dataAirLoop->AirLoopControlInfo(AirLoopNum));
        auto &curAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));

        curAirLoopFlow.OAMinFrac = OutAirMinFrac;
        if (this->FixedMin) {
            curAirLoopFlow.MinOutAir = min(OutAirMinFrac * curAirLoopFlow.DesSupply, this->MixMassFlow);
        } else {
            curAirLoopFlow.MinOutAir = OutAirMinFrac * this->MixMassFlow;
        }
        if (this->MixMassFlow > 0.0) {
            curAirLoopFlow.OAFrac = this->OAMassFlow / this->MixMassFlow;
            curAirLoopFlow.OAFlow = this->OAMassFlow;
        } else {
            curAirLoopFlow.OAFrac = 0.0;
            curAirLoopFlow.OAFlow = 0.0;
        }
        this->MinOAFracLimit = OutAirMinFrac;
        if (HighHumidityOperationFlag && OASignal > 1.0) {
            curAirLoopFlow.MaxOutAir = this->MaxOAMassFlowRate * OASignal;
        } else {
            curAirLoopFlow.MaxOutAir = currentMaxOAMassFlowRate;
        }

        // MJW - Not sure if this is necessary but keeping it for now
        if (curAirLoopControlInfo.HeatingActiveFlag && curAirLoopControlInfo.EconomizerFlowLocked) {
            // The airloop needs to be simulated again so that the heating coil & HX can be resimulated
            if (curAirLoopControlInfo.HeatRecoveryResimFlag && curAirLoopControlInfo.OASysComponentsSimulated) {
                curAirLoopControlInfo.ResimAirLoopFlag = true;
                curAirLoopControlInfo.HeatRecoveryResimFlag = false;
                curAirLoopControlInfo.HeatRecoveryResimFlag2 = true;
                // on the first iteration, air loop heating coils have not be simulated so HeatingCoilActive=FALSE
                // on the second iteration, the heating coils could have been on, but logic tests here could deactivate heating coil
                // reset heating coil active status and HX since logic tests may turn off heating coil
                // the ResimAirLoopFlag will force another iteration and things should line up on subsequent iterations
                curAirLoopControlInfo.HeatingActiveFlag = false;
                this->HRHeatingCoilActive = 0;
                curAirLoopControlInfo.HeatRecoveryBypass = true;
                this->HeatRecoveryBypassStatus = 1;
            } else if (curAirLoopControlInfo.HeatRecoveryResimFlag2) {
                curAirLoopControlInfo.ResimAirLoopFlag = true;
                curAirLoopControlInfo.HeatRecoveryResimFlag2 = false;
            } else {
                curAirLoopControlInfo.ResimAirLoopFlag = false;
            }
        } else if (curAirLoopControlInfo.HeatingActiveFlag) {
            this->HRHeatingCoilActive = 1;
        } else {
            this->HRHeatingCoilActive = 0;
        }

    } // if (AirLoopNum > 0)

    // Set the relief air flow rate (must be done last to account for changes in OAMassFlow
    this->RelMassFlow = max(this->OAMassFlow - this->ExhMassFlow, 0.0);

    // Save OA fraction for reporting
    if (this->MixMassFlow > 0) {
        this->OAFractionRpt = this->OAMassFlow / this->MixMassFlow;
    } else {
        if (this->OAMassFlow > 0) {
            this->OAFractionRpt = OASignal;
        } else {
            this->OAFractionRpt = 0.0;
        }
    }
    this->RelTemp = this->RetTemp;
    this->RelEnth = this->RetEnth;
    this->RelSensiLossRate =
        this->RelMassFlow * Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat) * (this->RelTemp - state.dataEnvrn->OutDryBulbTemp);
    this->RelTotalLossRate = this->RelMassFlow * (this->RelEnth - state.dataEnvrn->OutEnthalpy);
    this->RelLatentLossRate = this->RelTotalLossRate - this->RelSensiLossRate;
    this->OALimitingFactorReport = static_cast<int>(OALimitingFactor);
}

Real64 VentilationMechanicalProps::CalcMechVentController(EnergyPlusData &state,
                                                          Real64 SysSA // System supply air mass flow rate [kg/s]
)
{
    static constexpr std::string_view RoutineName("CalcMechVentController: ");
    static std::string_view const &CurrentModuleObject(CurrentModuleObjects[static_cast<int>(CMO::MechVentilation)]);

    // new local variables for DCV
    // Zone OA flow rate based on each calculation method [m3/s]
    Real64 ZoneOA;           // Zone OA flow rate [m3/s]
    Real64 ZoneOAFrac;       // Zone OA fraction (as a fraction of actual supply air flow rate)
    Real64 SysOAuc;          // System uncorrected OA flow rate
    Real64 SysOA;            // System supply OA volume flow rate [m3/s]
    Real64 SysEv;            // System ventilation efficiency
    Real64 NodeTemp;         // node temperature
    Real64 NodeHumRat;       // node humidity ratio
    Real64 ZoneMaxCO2 = 0.0; // Breathing-zone CO2 concentration
    Real64 ZoneMinCO2 = 0.0; // Minimum CO2 concentration in zone
    Real64 ZoneOAMin = 0.0;  // Minimum Zone OA flow rate when the zone is unoccupied (i.e. ZoneOAPeople = 0)
    Real64 ZoneOAMax = 0.0;  // Maximum Zone OA flow rate (ZoneOAPeople + ZoneOACalc[static_cast<int>(DataSizing::OAFlowCalcMethod::PerArea)])
    Real64 MechVentOAMassFlow = 0.0;

    // Apply mechanical ventilation only when it is available/allowed
    if (this->availSched->getCurrentVal() > 0) {
        Real64 SysOAMassFlow = 0.0; // System supply OA mass flow rate [kg/s]
        if (this->SystemOAMethod == DataSizing::SysOAMethod::IAQP) {
            // IAQP for CO2 control
            for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                int ZoneNum = thisMechVentZone.zoneNum;
                SysOAMassFlow +=
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP * thisMechVentZone.zoneOASched->getCurrentVal();
            }
            MechVentOAMassFlow = SysOAMassFlow;
        } else if (this->SystemOAMethod == DataSizing::SysOAMethod::IAQPGC) {
            // IAQP for generic contaminant control
            for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                int ZoneNum = thisMechVentZone.zoneNum;
                SysOAMassFlow +=
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP * thisMechVentZone.zoneOASched->getCurrentVal();
            }
            MechVentOAMassFlow = SysOAMassFlow;
        } else if (this->SystemOAMethod == DataSizing::SysOAMethod::IAQPCOM) {
            // IAQP for both CO2 and generic contaminant control
            SysOAMassFlow = 0.0;
            for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                int ZoneNum = thisMechVentZone.zoneNum;
                SysOAMassFlow +=
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP * thisMechVentZone.zoneOASched->getCurrentVal();
            }
            MechVentOAMassFlow = SysOAMassFlow;
            SysOAMassFlow = 0.0;
            for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                int ZoneNum = thisMechVentZone.zoneNum;
                SysOAMassFlow +=
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP * thisMechVentZone.zoneOASched->getCurrentVal();
            }
            MechVentOAMassFlow = max(SysOAMassFlow, MechVentOAMassFlow);
        } else {
            // for system OA methods: Zone_Sum, VRP, CO2 methods
            // new code for DCV method complying with the VRP defined in ASHRAE Standard 62.1-2010
            bool const useOccSch = this->DCVFlag && (this->SystemOAMethod != DataSizing::SysOAMethod::ProportionalControlDesOcc);

            // Loop through each zone first to calc uncorrected system OA flow rate
            SysOAuc = 0.0; // [m3/s]
            SysOA = 0.0;   // [m3/s]
            for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                int const ZoneNum = thisMechVentZone.zoneNum;

                // Use calcDesignSpecificationOutdoorAir function to support simple DSOA and DSOA:SpaceList
                bool const useOASch = true;
                bool const perPersonNotSet = false;  // placeholder function argument
                bool const maxOAVolFlowFlag = false; // placeholder function argument
                int constexpr spaceNum = 0;          // placeholder function argument
                bool const calcIAQMethods = false;   // If the zone or space OA method is IAQProcedure, PCOccSch or PCDesOcc then return zero
                thisMechVentZone.zoneOABZ = DataSizing::calcDesignSpecificationOutdoorAir(state,
                                                                                          thisMechVentZone.ZoneDesignSpecOAObjIndex,
                                                                                          ZoneNum,
                                                                                          useOccSch,
                                                                                          useOASch,
                                                                                          perPersonNotSet,
                                                                                          maxOAVolFlowFlag,
                                                                                          spaceNum,
                                                                                          calcIAQMethods);

                if (this->SystemOAMethod == DataSizing::SysOAMethod::ZoneSum) {
                    // Sum the zone OA flow rates and done
                    SysOA += thisMechVentZone.zoneOABZ;
                } else {
                    // Calc the uncorrected system OA flow rate - VRP and ProportionalControl
                    SysOAuc += thisMechVentZone.zoneOABZ;
                }
            }

            // get system supply air flow rate
            if (this->SystemOAMethod == DataSizing::SysOAMethod::VRP || this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc ||
                this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc ||
                this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate ||
                this->SystemOAMethod == DataSizing::SysOAMethod::VRPL) {

                // System supply air flow rate is always greater than or equal the system outdoor air flow rate
                if ((SysSA > 0.0) && (SysSA < (SysOAuc * state.dataEnvrn->StdRhoAir))) {
                    SysSA = SysOAuc * state.dataEnvrn->StdRhoAir;
                }

                // calc Xs - average outdoor air fraction
                if (SysSA > 0.0) {
                    Xs = (SysOAuc * state.dataEnvrn->StdRhoAir) / SysSA;
                } else {
                    Xs = 0.0;
                }

                // Loop through each zone again
                SysEv = 2.0; // starting with a big fraction
                for (int ZoneIndex = 1; ZoneIndex <= this->NumofVentMechZones; ++ZoneIndex) {
                    auto &thisMechVentZone = this->VentMechZone(ZoneIndex);
                    int ZoneNum = thisMechVentZone.zoneNum;
                    int ZoneEquipConfigNum = ZoneNum; // correspondence - 1:1 of ZoneEquipConfig to Zone index
                    Real64 ZoneEz = 0.0;              // Zone air distribution effectiveness

                    // Assign references
                    auto &curZone(state.dataHeatBal->Zone(ZoneNum));
                    auto &curZoneSysEnergyDemand(state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneEquipConfigNum));

                    // use the ventilation rate procedure in ASHRAE Standard 62.1-2007
                    // Calc the zone supplied OA flow rate counting the zone air distribution effectiveness
                    //  First check whether the zone air distribution effectiveness schedule exists, if yes uses it;
                    //   otherwise uses the inputs of zone distribution effectiveness in cooling mode or heating mode
                    if (thisMechVentZone.zoneADEffSched != nullptr) {
                        // Get schedule value for the zone air distribution effectiveness
                        ZoneEz = thisMechVentZone.zoneADEffSched->getCurrentVal();
                    } else {
                        Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).TotalOutputRequired;

                        // Zone in cooling mode
                        if (ZoneLoad < 0.0) {
                            ZoneEz = thisMechVentZone.ZoneADEffCooling;
                        }

                        // Zone in heating mode
                        if (ZoneLoad > 0.0) {
                            ZoneEz = thisMechVentZone.ZoneADEffHeating;
                        }
                    }
                    if (ZoneEz <= 0.0) {
                        // Enforce defaults
                        ZoneEz = 1.0;
                    }

                    // Calc zone supply OA flow rate
                    if (this->SystemOAMethod == DataSizing::SysOAMethod::VRP || this->SystemOAMethod == DataSizing::SysOAMethod::VRPL) {
                        // the VRP case
                        ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                    } else if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc ||
                               this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc ||
                               this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                        // Check whether "Carbon Dioxide Control Availability Schedule" for ZoneControl:ContaminantController is specified
                        if (curZone.zoneContamControllerSched != nullptr) {
                            // Check the availability schedule value for ZoneControl:ContaminantController
                            Real64 ZoneContamControllerSchedVal = curZone.zoneContamControllerSched->getCurrentVal();
                            if (ZoneContamControllerSchedVal > 0.0) {
                                auto &oaReq = state.dataSize->OARequirements(thisMechVentZone.ZoneDesignSpecOAObjIndex);
                                Real64 const zoneOAPeople = oaReq.oaFlowPeople(state, ZoneNum, useOccSch);
                                if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                                    ZoneOAMax = thisMechVentZone.zoneOABZ / ZoneEz;
                                    if (thisMechVentZone.zonePropCtlMinRateSched != nullptr) {
                                        ZoneOAMin = ZoneOAMax * thisMechVentZone.zonePropCtlMinRateSched->getCurrentVal();
                                    } else {
                                        ZoneOAMin = ZoneOAMax;
                                    }
                                    if (ZoneOAMax < ZoneOAMin) {
                                        ZoneOAMin = ZoneOAMax;
                                        ++this->OAMaxMinLimitErrorCount;
                                        if (this->OAMaxMinLimitErrorCount < 2) {
                                            ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                            ShowContinueError(
                                                state,
                                                format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOARate, maximum zone "
                                                       "outdoor air rate ({:.4R}), is not greater than minimum zone outdoor air rate ({:.4R}).",
                                                       ZoneOAMax,
                                                       ZoneOAMin));
                                            ShowContinueError(state,
                                                              " The minimum zone outdoor air rate is set to the maximum zone outdoor air rate. "
                                                              "Simulation continues...");
                                            ShowContinueErrorTimeStamp(state, "");
                                        } else {
                                            ShowRecurringWarningErrorAtEnd(
                                                state,
                                                format("{} = \"{}\", For System Outdoor Air Method = ProportionalControlBasedOnDesignOARate, maximum "
                                                       "zone outdoor air rate is not greater than minimum zone outdoor air rate. Error continues...",
                                                       CurrentModuleObject,
                                                       this->Name),
                                                this->OAMaxMinLimitErrorIndex);
                                        }
                                    }
                                } else {
                                    Real64 const zoneOAArea = oaReq.oaFlowArea(state, ZoneNum);
                                    ZoneOAMin = zoneOAArea / ZoneEz;
                                    ZoneOAMax = (zoneOAArea + zoneOAPeople) / ZoneEz;
                                }

                                if (zoneOAPeople > 0.0) {
                                    if (state.dataContaminantBalance->ZoneCO2GainFromPeople(ZoneNum) > 0.0) {
                                        if (curZone.zoneMinCO2Sched != nullptr) {
                                            // Take the schedule value of "Minimum Carbon Dioxide Concentration Schedule Name"
                                            // in the ZoneControl:ContaminantController
                                            ZoneMinCO2 = curZone.zoneMinCO2Sched->getCurrentVal();
                                        } else {
                                            ZoneMinCO2 = state.dataContaminantBalance->OutdoorCO2;
                                        }

                                        // Calculate zone maximum target CO2 concentration in PPM
                                        if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc) {
                                            // Accumulate CO2 generation from people at design occupancy and current activity level
                                            Real64 CO2PeopleGeneration = 0.0;
                                            for (int const PeopleNum : thisMechVentZone.peopleIndexes) {
                                                CO2PeopleGeneration += state.dataHeatBal->People(PeopleNum).NumberOfPeople *
                                                                       state.dataHeatBal->People(PeopleNum).CO2RateFactor *
                                                                       state.dataHeatBal->People(PeopleNum).activityLevelSched->getCurrentVal();
                                            }
                                            ZoneMaxCO2 = state.dataContaminantBalance->OutdoorCO2 +
                                                         (CO2PeopleGeneration * curZone.Multiplier * curZone.ListMultiplier * 1.0e6) / ZoneOAMax;
                                        } else if (curZone.zoneMaxCO2Sched != nullptr) {
                                            ZoneMaxCO2 = curZone.zoneMaxCO2Sched->getCurrentVal();
                                        } else {
                                            ZoneMaxCO2 = state.dataContaminantBalance->OutdoorCO2 +
                                                         (state.dataContaminantBalance->ZoneCO2GainFromPeople(ZoneNum) * curZone.Multiplier *
                                                          curZone.ListMultiplier * 1.0e6) /
                                                             ZoneOAMax;
                                        }

                                        if (ZoneMaxCO2 <= ZoneMinCO2) {
                                            ++this->CO2MaxMinLimitErrorCount;
                                            if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc) {
                                                if (this->CO2MaxMinLimitErrorCount < 2) {
                                                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                                    ShowContinueError(
                                                        state,
                                                        format("For System Outdoor Air Method = ProportionalControlBasedOnOccupancySchedule, "
                                                               "maximum target CO2 concentration ({:.2R}), is not greater than minimum target "
                                                               "CO2 concentration ({:.2R}).",
                                                               ZoneMaxCO2,
                                                               ZoneMinCO2));
                                                    ShowContinueError(state,
                                                                      "\"ProportionalControlBasedOnOccupancySchedule\" will not be modeled. "
                                                                      "Default \"Standard62.1VentilationRateProcedure\" will be modeled. Simulation "
                                                                      "continues...");
                                                    ShowContinueErrorTimeStamp(state, "");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(state,
                                                                                   format("{} = \"{}\", For System Outdoor Air Method = "
                                                                                          "ProportionalControlBasedOnOccupancySchedule, maximum "
                                                                                          "target CO2 concentration is not greater than minimum "
                                                                                          "target CO2 concentration. Error continues...",
                                                                                          CurrentModuleObject,
                                                                                          this->Name),
                                                                                   this->CO2MaxMinLimitErrorIndex);
                                                }
                                            }
                                            if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc) {
                                                if (this->CO2MaxMinLimitErrorCount < 2) {
                                                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                                    ShowContinueError(
                                                        state,
                                                        format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOccupancy, "
                                                               "maximum target CO2 concentration ({:.2R}), is not greater than minimum target "
                                                               "CO2 concentration ({:.2R}).",
                                                               ZoneMaxCO2,
                                                               ZoneMinCO2));
                                                    ShowContinueError(state,
                                                                      "\"ProportionalControlBasedOnDesignOccupancy\" will not be modeled. "
                                                                      "Default \"Standard62.1VentilationRateProcedure\" will be modeled. Simulation "
                                                                      "continues...");
                                                    ShowContinueErrorTimeStamp(state, "");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(state,
                                                                                   format("{} = \"{}\", For System Outdoor Air Method = "
                                                                                          "ProportionalControlBasedOnDesignOccupancy, maximum "
                                                                                          "target CO2 concentration is not greater than minimum "
                                                                                          "target CO2 concentration. Error continues...",
                                                                                          CurrentModuleObject,
                                                                                          this->Name),
                                                                                   this->CO2MaxMinLimitErrorIndex);
                                                }
                                            }
                                            if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                                                if (this->CO2MaxMinLimitErrorCount < 2) {
                                                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                                    ShowContinueError(
                                                        state,
                                                        format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOARate, maximum "
                                                               "target CO2 concentration ({:.2R}), is not greater than minimum target CO2 "
                                                               "concentration ({:.2R}).",
                                                               ZoneMaxCO2,
                                                               ZoneMinCO2));
                                                    ShowContinueError(
                                                        state,
                                                        "\"ProportionalControlBasedOnDesignOARate\" will not be modeled. Default "
                                                        "\"Standard62.1VentilationRateProcedure\" will be modeled. Simulation continues...");
                                                    ShowContinueErrorTimeStamp(state, "");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(state,
                                                                                   format("{} = \"{}\", For System Outdoor Air Method = "
                                                                                          "ProportionalControlBasedOnDesignOARate, maximum target "
                                                                                          "CO2 concentration is not greater than minimum target CO2 "
                                                                                          "concentration. Error continues...",
                                                                                          CurrentModuleObject,
                                                                                          this->Name),
                                                                                   this->CO2MaxMinLimitErrorIndex);
                                                }
                                            }

                                            ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                                        } else {

                                            if (state.dataContaminantBalance->ZoneAirCO2(ZoneNum) <= ZoneMinCO2) {
                                                // Zone air CO2 concentration is less than minimum zone CO2 concentration, set the Zone OA flow
                                                // rate to minimum Zone OA flow rate when the zone is unoccupied
                                                ZoneOA = ZoneOAMin;
                                            } else if (state.dataContaminantBalance->ZoneAirCO2(ZoneNum) >= ZoneMaxCO2) {
                                                // Zone air CO2 concentration is greater than maximum zone CO2 concentration, set the Zone OA flow
                                                // rate to maximum Zone OA flow rate (i.e.
                                                // ZoneOACalc[static_cast<int>(DataSizing::OAFlowCalcMethod::PerArea)] + ZoneOAPeople)
                                                ZoneOA = ZoneOAMax;
                                            } else {
                                                // Zone air CO2 concentration is between maximum and minimum limits of zone CO2 concentration,
                                                // set Zone OA flow rate by proportionally adjusting between ZoneOAMin and ZoneOAMax
                                                ZoneOA = ZoneOAMin +
                                                         (ZoneOAMax - ZoneOAMin) * ((state.dataContaminantBalance->ZoneAirCO2(ZoneNum) - ZoneMinCO2) /
                                                                                    (ZoneMaxCO2 - ZoneMinCO2));
                                            }
                                        }
                                    } else {
                                        if (state.dataGlobal->DisplayExtraWarnings) {
                                            ++this->CO2GainErrorCount;
                                            if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc) {
                                                if (this->CO2GainErrorCount < 2) {
                                                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                                    ShowContinueError(
                                                        state,
                                                        format("For System Outdoor Air Method = ProportionalControlBasedOnOccupancySchedule, CO2 "
                                                               "generation from people is not greater than zero. Occurs in Zone =\"{}\". ",
                                                               curZone.Name));
                                                    ShowContinueError(state,
                                                                      "\"ProportionalControlBasedOnOccupancySchedule\" will not be modeled. "
                                                                      "Default \"Standard62.1VentilationRateProcedure\" will be modeled. Simulation "
                                                                      "continues...");
                                                    ShowContinueErrorTimeStamp(state, "");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        format("{} = \"{}\", For System Outdoor Air Method = "
                                                               "ProportionalControlBasedOnOccupancySchedule, "
                                                               "CO2 generation from people is not greater than zero. Error continues...",
                                                               CurrentModuleObject,
                                                               this->Name),
                                                        this->CO2GainErrorIndex);
                                                }
                                            }
                                            if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc) {
                                                if (this->CO2GainErrorCount < 2) {
                                                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, this->Name));
                                                    ShowContinueError(
                                                        state,
                                                        format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOccupancy, CO2 "
                                                               "generation from people is not greater than zero. Occurs in Zone =\"{}\". ",
                                                               curZone.Name));
                                                    ShowContinueError(state,
                                                                      "\"ProportionalControlBasedOnDesignOccupancy\" will not be modeled. "
                                                                      "Default \"Standard62.1VentilationRateProcedure\" will be modeled. Simulation "
                                                                      "continues...");
                                                    ShowContinueErrorTimeStamp(state, "");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        format(
                                                            "{} = \"{}\", For System Outdoor Air Method = ProportionalControlBasedOnDesignOccupancy, "
                                                            "CO2 generation from people is not greater than zero. Error continues...",
                                                            CurrentModuleObject,
                                                            this->Name),
                                                        this->CO2GainErrorIndex);
                                                }
                                            }
                                        }
                                        ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                                    }
                                } else {
                                    // zoneOAPeople is less than or equal to zero
                                    ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                                }
                            } else {
                                // ZoneControl:ContaminantController is scheduled off (not available)
                                ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                            }
                        } else {
                            // "Carbon Dioxide Control Availability Schedule" for ZoneControl:ContaminantController not found
                            ZoneOA = thisMechVentZone.zoneOABZ / ZoneEz;
                        }
                        SysOA = SysOA + ZoneOA;
                    }

                    // Get the zone supply air flow rate
                    Real64 ZoneSA = 0.0; // Zone supply air flow rate
                    Real64 ZonePA = 0.0; // Zone primary air flow rate
                    Ep = 1.0;
                    if (ZoneEquipConfigNum > 0) {
                        auto &curZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum);
                        for (int InNodeIndex = 1; InNodeIndex <= curZoneEquipConfig.NumInletNodes; ++InNodeIndex) {
                            // Assume primary air is always stored at the AirDistUnitCool (cooling deck if dual duct)
                            int PriNode = curZoneEquipConfig.AirDistUnitCool(InNodeIndex).InNode; // primary node of zone terminal unit
                            Real64 MassFlowRate = 0.0;
                            if (PriNode > 0) {
                                NodeTemp = state.dataLoopNodes->Node(PriNode).Temp;
                                NodeHumRat = state.dataLoopNodes->Node(PriNode).HumRat;
                                MassFlowRate = state.dataLoopNodes->Node(PriNode).MassFlowRate;
                            }
                            // total primary air to terminal units of the zone
                            if (MassFlowRate > 0.0) {
                                ZonePA +=
                                    MassFlowRate / Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, NodeTemp, NodeHumRat);
                            }

                            // or InletNode = ZoneEquipConfig(ZoneEquipConfigNum)%AirDistUnitCool(InNodeIndex)%OutNode
                            int InletNode = curZoneEquipConfig.InletNode(InNodeIndex); // outlet node of zone terminal unit
                            MassFlowRate = 0.0;
                            if (InletNode > 0) {
                                NodeTemp = state.dataLoopNodes->Node(InletNode).Temp;
                                NodeHumRat = state.dataLoopNodes->Node(InletNode).HumRat; // ZoneAirHumRat(ZoneNum)
                                MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
                            }
                            // total supply air to the zone
                            if (MassFlowRate > 0.0) {
                                ZoneSA +=
                                    MassFlowRate / Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, NodeTemp, NodeHumRat);
                            }
                        }

                        // calc zone primary air fraction
                        if (ZoneSA > 0.0) {
                            Ep = ZonePA / ZoneSA;
                        }
                        if (Ep > 1.0) {
                            Ep = 1.0;
                        }
                    }

                    // Calc the zone OA fraction = Zone OA flow rate / Zone supply air flow rate
                    if (ZoneSA > 0.0) {
                        ZoneOAFrac = ZoneOA / ZoneSA;
                        // Zone OA fraction cannot be more than 1
                        if (ZoneOAFrac > 1.0) {
                            ZoneOAFrac = 1.0;
                        }
                    } else {
                        ZoneOAFrac = 0.0;
                    }

                    // added for TRACE - zone maximum OA fraction - calculate the adjustment factor for the TU/zone supply air flow
                    // only for VRP system OA method
                    curZoneSysEnergyDemand.SupplyAirAdjustFactor = 1.0;

                    if (this->SystemOAMethod == DataSizing::SysOAMethod::VRP || this->SystemOAMethod == DataSizing::SysOAMethod::VRPL) {
                        if (ZoneOAFrac > this->ZoneMaxOAFraction) {
                            if (this->ZoneMaxOAFraction > 0.0) {
                                curZoneSysEnergyDemand.SupplyAirAdjustFactor = ZoneOAFrac / this->ZoneMaxOAFraction;
                            } else {
                                curZoneSysEnergyDemand.SupplyAirAdjustFactor = 1.0;
                            }

                            // cap zone OA fraction at the maximum specified
                            ZoneOAFrac = this->ZoneMaxOAFraction;
                        }
                    }

                    // Zone air secondary recirculation fraction
                    Er = thisMechVentZone.ZoneSecondaryRecirculation;
                    if (Er > 0.0) {
                        // multi-path ventilation system using VRP
                        Fa = Ep + (1.0 - Ep) * Er;
                        Fb = Ep;
                        Fc = 1.0 - (1.0 - ZoneEz) * (1.0 - Er) * (1.0 - Ep);

                        // Calc zone ventilation efficiency
                        if (Fa > 0.0) {
                            Evz = 1.0 + Xs * Fb / Fa - ZoneOAFrac * Ep * Fc / Fa;
                        } else {
                            Evz = 1.0;
                        }
                    } else {
                        // single-path ventilation system
                        Evz = 1.0 + Xs - ZoneOAFrac;
                    }

                    // calc system ventilation efficiency = Minimum of zone ventilation efficiency
                    if (Evz < 0.0) {
                        Evz = 0.0;
                    }
                    if (Evz < SysEv) {
                        SysEv = Evz;
                    }

                } // zone loop

                // Calc the system supply OA flow rate counting the system ventilation efficiency
                if (SysEv <= 0.0) {
                    SysEv = 1.0;
                }

                // Calc system outdoor air requirement
                if (this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlSchOcc ||
                    this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOcc ||
                    this->SystemOAMethod == DataSizing::SysOAMethod::ProportionalControlDesOARate) {
                    SysOA = SysOA / SysEv;
                } else if (this->SystemOAMethod == DataSizing::SysOAMethod::VRPL && this->SysDesOA > 0.0) {
                    // Limit system OA to design OA minimum flow rate, as per ASHRAE Guideline 36-2018 Section 5.16.3.1
                    // If no system sizing run is done (i.e. no Sizing:System) the design outdoor air flow rate is not known
                    SysOA = min(SysOAuc / SysEv, this->SysDesOA);
                } else {
                    SysOA = SysOAuc / SysEv;
                }
            }

            // Finally calc the system supply OA mass flow rate
            MechVentOAMassFlow = SysOA * state.dataEnvrn->StdRhoAir;
        }
    }
    return MechVentOAMassFlow;
}

void OAControllerProps::CalcOAEconomizer(EnergyPlusData &state,
                                         int const AirLoopNum,
                                         Real64 const OutAirMinFrac,
                                         Real64 &OASignal,
                                         bool &HighHumidityOperationFlag,
                                         bool const FirstHVACIteration)
{
    int constexpr MaxIte(500);             // Maximum number of iterations
    Real64 constexpr Acc(0.0001);          // Accuracy of result
    bool AirLoopEconoLockout;              // Economizer lockout flag
    bool AirLoopNightVent;                 // Night Ventilation flag for air loop
    bool EconomizerOperationFlag;          // TRUE if OA economizer is active
    Real64 EconomizerAirFlowScheduleValue; // value of economizer operation schedule (push-button type control schedule)
    Real64 MaximumOAFracBySetPoint;        // The maximum OA fraction due to freezing cooling coil check
    Real64 OutAirSignal;                   // Used to set OA mass flow rate
    Real64 minOAFrac;

    if (AirLoopNum > 0) {
        // Check lockout with heating for any airloop - will lockout economizer even on airloops without a unitary system
        if (this->Lockout == LockoutType::LockoutWithHeatingPossible) {
            // For all system types (even ones that don't set AirLoopEconoLockout) lock out economizer if unfavorable for heating
            if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CheckHeatRecoveryBypassStatus &&
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysComponentsSimulated) {

                if (this->MixedAirTempAtMinOAFlow <= state.dataLoopNodes->Node(this->MixNode).TempSetPoint) {
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconomizerFlowLocked = true;
                    // this->OAMassFlow = AirLoopFlow( AirLoopNum ).MinOutAir;
                    // AirLoopFlow( AirLoopNum ).OAFrac = this->OAMassFlow / this->MixMassFlow;
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoLockout = true;
                } else {
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconomizerFlowLocked = false;
                    this->HRHeatingCoilActive = 0;
                }
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CheckHeatRecoveryBypassStatus = false;
            }
        }
    }

    if (AirLoopNum > 0) {
        AirLoopEconoLockout = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoLockout;
        AirLoopNightVent = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).NightVent;
    } else {
        AirLoopEconoLockout = false;
        AirLoopNightVent = false;
    }

    // Define an outside air signal
    if (this->MixedAirSPMNum > 0) {
        this->CoolCoilFreezeCheck = SetPointManager::GetCoilFreezingCheckFlag(state, this->MixedAirSPMNum);
    } else {
        this->CoolCoilFreezeCheck = false;
    }

    if (std::abs(this->RetTemp - this->InletTemp) > HVAC::SmallTempDiff) {
        OutAirSignal = (this->RetTemp - this->MixSetTemp) / (this->RetTemp - this->InletTemp);
        if (this->CoolCoilFreezeCheck) {
            this->MaxOAFracBySetPoint = 0.0;
            MaximumOAFracBySetPoint = OutAirSignal;
        }
    } else {
        if (this->RetTemp - this->MixSetTemp < 0.0) {
            if (this->RetTemp - this->InletTemp >= 0.0) {
                OutAirSignal = -1.0;
            } else {
                OutAirSignal = 1.0;
            }
        } else {
            if (this->RetTemp - this->InletTemp >= 0.0) {
                OutAirSignal = 1.0;
            } else {
                OutAirSignal = -1.0;
            }
        }
    }
    OutAirSignal = min(max(OutAirSignal, OutAirMinFrac), 1.0);

    // If no economizer, set to minimum and disable economizer and high humidity control
    if (this->Econo == EconoOp::NoEconomizer) {
        OutAirSignal = OutAirMinFrac;
        EconomizerOperationFlag = false;
        EconomizerAirFlowScheduleValue = 0.0;
        HighHumidityOperationFlag = false;
    } else if (this->MaxOA < HVAC::SmallAirVolFlow) {
        OutAirSignal = OutAirMinFrac;
        EconomizerOperationFlag = false;
        EconomizerAirFlowScheduleValue = 0.0;
        HighHumidityOperationFlag = false;
    } else if (AirLoopEconoLockout) {
        OutAirSignal = OutAirMinFrac;
        EconomizerOperationFlag = false;
        EconomizerAirFlowScheduleValue = 0.0;
        HighHumidityOperationFlag = false;
    } else {
        // Changed by Amit for new implementation
        // Otherwise do the limit checks
        EconomizerOperationFlag = true;
        // Outside air temp greater than mix air setpoint
        if (this->InletTemp > this->MixSetTemp) {
            OutAirSignal = 1.0;
        }
        // Return air temp limit
        if (this->Econo == EconoOp::DifferentialDryBulb) {
            if (this->InletTemp > this->RetTemp) {
                OutAirSignal = OutAirMinFrac;
                EconomizerOperationFlag = false;
            }
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // Return air enthalpy limit
        if (this->Econo == EconoOp::DifferentialEnthalpy) {
            if (this->InletEnth > this->RetEnth) {
                OutAirSignal = OutAirMinFrac;
                EconomizerOperationFlag = false;
            }
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // Outside air temperature limit
        if (this->Econo == EconoOp::FixedDryBulb) {
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // Fixed Enthalpy limit
        if (this->Econo == EconoOp::FixedEnthalpy) {
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // FIXED DEW POINT AND DRY BULB TEMPERATURE STRATEGY
        if (this->Econo == EconoOp::FixedDewPointAndDryBulb) {
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // ELECRONIC ENTHALPY, HUMIDITY RATIO CURVE
        if (this->Econo == EconoOp::ElectronicEnthalpy) {
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }
        // Differential dry bulb and enthalpy strategy
        if (this->Econo == EconoOp::DifferentialDryBulbAndEnthalpy) {
            if (this->InletTemp > this->RetTemp) {
                OutAirSignal = OutAirMinFrac;
                EconomizerOperationFlag = false;
            }
            if (this->InletEnth > this->RetEnth) {
                OutAirSignal = OutAirMinFrac;
                EconomizerOperationFlag = false;
            }
            this->Checksetpoints(state, OutAirMinFrac, OutAirSignal, EconomizerOperationFlag);
        }

        if (this->TempLowLim != HVAC::BlankNumeric && this->OATemp < this->TempLowLim) {
            OutAirSignal = OutAirMinFrac;
            EconomizerOperationFlag = false;
        }
        // Increase air flow for humidity control
        // (HumidistatZoneNum is greater than 0 IF High Humidity Control Flag = YES, checked in GetInput)
        if (this->HumidistatZoneNum > 0) {
            //   IF humidistat senses a moisture load check to see if modifying air flow is appropriate, otherwise disable modified air flow
            if (state.dataZoneEnergyDemand->ZoneSysMoistureDemand(this->HumidistatZoneNum).TotalOutputRequired < 0.0) {
                //     IF OAController is not allowed to modify air flow during high outdoor humrat condition, then disable modified air flow
                //     if indoor humrat is less than or equal to outdoor humrat
                if (!this->ModifyDuringHighOAMoisture &&
                    (state.dataLoopNodes->Node(this->NodeNumofHumidistatZone).HumRat - this->OAHumRat) <= HVAC::SmallHumRatDiff) {
                    HighHumidityOperationFlag = false;
                } else {
                    HighHumidityOperationFlag = true;
                }
            } else {
                HighHumidityOperationFlag = false;
            }
        } else {
            HighHumidityOperationFlag = false;
        }

        // Check time of day economizer schedule, enable economizer if schedule value > 0
        EconomizerAirFlowScheduleValue = 0.0;
        if (this->economizerOASched != nullptr) {
            EconomizerAirFlowScheduleValue = this->economizerOASched->getCurrentVal();
            if (EconomizerAirFlowScheduleValue > 0.0) {
                EconomizerOperationFlag = true;
                OutAirSignal = 1.0;
            }
        }
    }

    // OutAirSignal will not give exactly the correct mixed air temperature (equal to the setpoint) since
    // it was calculated using the approximate method of sensible energy balance. Now we have to get the
    // accurate result using a full mass, enthalpy and moisture balance and iteration.
    if (OutAirSignal > OutAirMinFrac && OutAirSignal < 1.0 && this->MixMassFlow > HVAC::VerySmallMassFlow &&
        this->ControllerType == MixedAirControllerType::ControllerOutsideAir && !AirLoopNightVent) {
        int SolFla; // Flag of solver

        if (AirLoopNum > 0) {

            if (state.dataAirLoop->OutsideAirSys(state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysNum).NumComponents == 1) {
                // no need to simulate OA System if only a mixer is used in the OutsideAirSystem

                auto f = [&state, this](Real64 const OASignal) {
                    Real64 const OAMassFlowRate = OASignal * this->MixMassFlow;
                    Real64 const RecircMassFlowRate = max(this->MixMassFlow - OAMassFlowRate, 0.0);
                    Real64 const RecircEnth = state.dataLoopNodes->Node(this->RetNode).Enthalpy;
                    Real64 const RecircHumRat = state.dataLoopNodes->Node(this->RetNode).HumRat;
                    Real64 const MixEnth =
                        (RecircMassFlowRate * RecircEnth + OAMassFlowRate * state.dataLoopNodes->Node(this->OANode).Enthalpy) / this->MixMassFlow;
                    Real64 const MixHumRat =
                        (RecircMassFlowRate * RecircHumRat + OAMassFlowRate * state.dataLoopNodes->Node(this->OANode).HumRat) / this->MixMassFlow;
                    Real64 const MixTemp = Psychrometrics::PsyTdbFnHW(MixEnth, MixHumRat);
                    return state.dataLoopNodes->Node(this->MixNode).TempSetPoint - MixTemp;
                };

                General::SolveRoot(state, Acc, MaxIte, SolFla, OASignal, f, OutAirMinFrac, 1.0);
                if (SolFla < 0) {
                    OASignal = OutAirSignal;
                }

            } else {

                // simulate OA System if equipment exists other than the mixer (e.g., heating/cooling coil, HX, etc.)

                // 1 - check min OA flow result
                if (this->FixedMin) {
                    state.dataLoopNodes->Node(this->OANode).MassFlowRate =
                        min(max(this->ExhMassFlow, OutAirMinFrac * state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply),
                            state.dataLoopNodes->Node(this->MixNode).MassFlowRate);
                    state.dataLoopNodes->Node(this->RelNode).MassFlowRate =
                        max(state.dataLoopNodes->Node(this->OANode).MassFlowRate - this->ExhMassFlow, 0.0);
                    // save actual OA flow frac for use as min value for RegulaFalsi call
                    minOAFrac = max(OutAirMinFrac, state.dataLoopNodes->Node(this->OANode).MassFlowRate / this->MixMassFlow);
                } else {
                    state.dataLoopNodes->Node(this->OANode).MassFlowRate =
                        max(this->ExhMassFlow, OutAirMinFrac * state.dataLoopNodes->Node(this->MixNode).MassFlowRate);
                    state.dataLoopNodes->Node(this->RelNode).MassFlowRate =
                        max(state.dataLoopNodes->Node(this->OANode).MassFlowRate - this->ExhMassFlow, 0.0);
                    // save actual OA flow frac for use as min value for RegulaFalsi call
                    minOAFrac = max(OutAirMinFrac, state.dataLoopNodes->Node(this->OANode).MassFlowRate / this->MixMassFlow);
                }
                SimOASysComponents(state, state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysNum, FirstHVACIteration, AirLoopNum);
                Real64 lowFlowResiduum = state.dataLoopNodes->Node(this->MixNode).TempSetPoint - state.dataLoopNodes->Node(this->MixNode).Temp;

                // 2 - check max OA flow result
                state.dataLoopNodes->Node(this->OANode).MassFlowRate = max(this->ExhMassFlow, state.dataLoopNodes->Node(this->MixNode).MassFlowRate);
                state.dataLoopNodes->Node(this->RelNode).MassFlowRate =
                    max(state.dataLoopNodes->Node(this->OANode).MassFlowRate - this->ExhMassFlow, 0.0);
                SimOASysComponents(state, state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysNum, FirstHVACIteration, AirLoopNum);
                Real64 highFlowResiduum = state.dataLoopNodes->Node(this->MixNode).TempSetPoint - state.dataLoopNodes->Node(this->MixNode).Temp;

                // 3 - test to ensure RegulaFalsi can find an answer
                if ((sign(lowFlowResiduum) == sign(highFlowResiduum))) {
                    OASignal = OutAirSignal;
                } else {
                    // 4 - find result

                    auto f = [&state, this, FirstHVACIteration, AirLoopNum](Real64 const OASignal) {
                        Real64 const MixMassFlowRate = this->MixMassFlow;
                        int const OASysNum = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).OASysNum;
                        Real64 localExhMassFlow = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ZoneExhMassFlow;
                        Real64 const OAMassFlowRate = max(localExhMassFlow, OASignal * MixMassFlowRate);
                        state.dataLoopNodes->Node(this->OANode).MassFlowRate = OAMassFlowRate; // set OA node mass flow rate
                        state.dataLoopNodes->Node(this->RelNode).MassFlowRate =
                            max(OAMassFlowRate - localExhMassFlow, 0.0); // set relief node mass flow rate to maintain mixer continuity calcs
                        SimOASysComponents(state, OASysNum, FirstHVACIteration, AirLoopNum);
                        return state.dataLoopNodes->Node(this->MixNode).TempSetPoint - state.dataLoopNodes->Node(this->MixNode).Temp;
                    };

                    General::SolveRoot(state, (Acc / 10.0), MaxIte, SolFla, OASignal, f, minOAFrac, 1.0);
                    if (SolFla < 0) { // if RegulaFalsi fails to find a solution, returns -1 or -2, set to existing OutAirSignal
                        OASignal = OutAirSignal;
                    }
                }
            }

        } else {

            auto f = [&state, this](Real64 const OASignal) {
                Real64 const MixMassFlowRate = this->MixMassFlow;
                Real64 OAMassFlowRate = OASignal * MixMassFlowRate;
                Real64 RecircMassFlowRate = max(MixMassFlowRate - OAMassFlowRate, 0.0);
                Real64 RecircEnth = state.dataLoopNodes->Node(this->RetNode).Enthalpy;
                Real64 RecircHumRat = state.dataLoopNodes->Node(this->RetNode).HumRat;
                Real64 MixEnth =
                    (RecircMassFlowRate * RecircEnth + OAMassFlowRate * state.dataLoopNodes->Node(this->OANode).Enthalpy) / MixMassFlowRate;
                Real64 MixHumRat =
                    (RecircMassFlowRate * RecircHumRat + OAMassFlowRate * state.dataLoopNodes->Node(this->OANode).HumRat) / MixMassFlowRate;
                Real64 MixTemp = Psychrometrics::PsyTdbFnHW(MixEnth, MixHumRat);
                return state.dataLoopNodes->Node(this->MixNode).TempSetPoint - MixTemp;
            };

            General::SolveRoot(state, Acc, MaxIte, SolFla, OASignal, f, OutAirMinFrac, 1.0);
            if (SolFla < 0) {
                OASignal = OutAirSignal;
            }
        }

    } else {
        OASignal = OutAirSignal;
    }

    // Economizer choice "Bypass" forces minimum OA except when high humidity air flow is active based on indoor RH
    if (this->EconBypass && EconomizerAirFlowScheduleValue == 0.0) {
        OASignal = OutAirMinFrac;
    }

    // Set outdoor air signal based on OA flow ratio if high humidity air flow is enabled
    if (HighHumidityOperationFlag) {
        if (this->MixMassFlow > 0.0) {
            //   calculate the actual ratio of outside air to mixed air so the magnitude of OA during high humidity control is correct
            OASignal = max(OutAirMinFrac, (this->HighRHOAFlowRatio * this->MaxOAMassFlowRate / this->MixMassFlow));
            this->OALimitingFactor = OALimitFactor::HighHum;
        }
    }

    if (this->CoolCoilFreezeCheck) {
        MaximumOAFracBySetPoint = min(max(MaximumOAFracBySetPoint, 0.0), 1.0);
        this->MaxOAFracBySetPoint = MaximumOAFracBySetPoint;

        // This should not be messing with OutAirMinFrac, freeze protection should only limit economizer operation
        // if (MaximumOAFracBySetPoint < OutAirMinFrac) {
        // OutAirMinFrac = MaximumOAFracBySetPoint;
        //    if (AirLoopNum > 0) AirLoopFlow(AirLoopNum).MinOutAir = OutAirMinFrac * this->MixMassFlow;
        //}
        if (MaximumOAFracBySetPoint < OASignal) {
            OASignal = MaximumOAFracBySetPoint;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
        if (OutAirMinFrac > OASignal) {
            OASignal = OutAirMinFrac;
            this->OALimitingFactor = OALimitFactor::Limits;
        }
    }

    if (AirLoopNum > 0) {

        // Set the air loop economizer and high humidity control flags.
        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive = EconomizerOperationFlag;
        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HighHumCtrlActive = HighHumidityOperationFlag;
        if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconomizerFlowLocked) {
            this->OAMassFlow = state.dataAirLoop->AirLoopFlow(AirLoopNum).MinOutAir;
            state.dataAirLoop->AirLoopFlow(AirLoopNum).OAFrac = this->OAMassFlow / this->MixMassFlow;
            state.dataAirLoop->AirLoopFlow(AirLoopNum).OAFlow = this->OAMassFlow;
        }

        // Check heat exchanger bypass control
        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HeatRecoveryBypass = false;
        this->HeatRecoveryBypassStatus = 0;
        if (EconomizerOperationFlag) {
            if (this->HeatRecoveryBypassControlType == HVAC::BypassWhenWithinEconomizerLimits) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HeatRecoveryBypass = true;
                this->HeatRecoveryBypassStatus = 1;
            } else if (this->HeatRecoveryBypassControlType == HVAC::BypassWhenOAFlowGreaterThanMinimum) {
                Real64 OAMassFlowMin = OutAirMinFrac * state.dataAirLoop->AirLoopFlow(AirLoopNum).DesSupply;
                Real64 OAMassFlowActual = OASignal * this->MixMassFlow;
                Real64 reasonablySmallMassFlow = 1e-6;
                if (OAMassFlowActual > (OAMassFlowMin + reasonablySmallMassFlow)) {
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).HeatRecoveryBypass = true;
                    this->HeatRecoveryBypassStatus = 1;
                }
            }
        }
    }

    // Set economizer report variable and status flag
    if (this->Econo == EconoOp::NoEconomizer) {
        // No economizer
        this->EconomizerStatus = 0;
        this->EconoActive = false;
    } else {
        // With economizer.
        if (EconomizerOperationFlag) {
            // Economizer is enabled
            this->EconomizerStatus = 1;
            this->EconoActive = true;
            if ((OASignal > OutAirMinFrac) && !HighHumidityOperationFlag) {
                this->OALimitingFactor = OALimitFactor::Economizer;
            }
        } else {
            // Economizer is disabled
            this->EconomizerStatus = 0;
            this->EconoActive = false;
        }
    }

    // Night ventilation control overrides economizer and high humidity control.
    if (AirLoopNightVent) {
        OASignal = 1.0;
        this->OALimitingFactor = OALimitFactor::NightVent;
    }

    // Set high humidity control report variable and status flag
    if (HighHumidityOperationFlag) {
        this->HighHumCtrlStatus = 1;
        this->HighHumCtrlActive = true;
    } else {
        this->HighHumCtrlStatus = 0;
        this->HighHumCtrlActive = false;
    }
}

void OAMixerProps::CalcOAMixer(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Calculate the mixed air flow and conditions

    // Define a recirculation mass flow rate
    Real64 RecircMassFlowRate = this->RetMassFlowRate - this->RelMassFlowRate;
    // In certain low flow conditions the return air mass flow rate can be below the outside air value established
    //  by the user.  This check will ensure that this condition does not result in non-physical air properties.
    if (RecircMassFlowRate < 0.0) {
        RecircMassFlowRate = 0.0;
        this->RelMassFlowRate = this->RetMassFlowRate;
    }

    // Pass through the return air conditions to the relief air stream.  The return air is "split" to
    // the relief air and the recirculation air.
    this->RelTemp = this->RetTemp;
    this->RelHumRat = this->RetHumRat;
    this->RelEnthalpy = this->RetEnthalpy;
    this->RelPressure = this->RetPressure;
    Real64 RecircPressure = this->RetPressure;
    Real64 RecircEnthalpy = this->RetEnthalpy;
    Real64 RecircHumRat = this->RetHumRat;
    // The recirculation air and the outside air are mixed to form the mixed air stream
    this->MixMassFlowRate = this->OAMassFlowRate + RecircMassFlowRate;
    // Check for zero flow
    if (this->MixMassFlowRate <= HVAC::VerySmallMassFlow) {
        this->MixEnthalpy = this->RetEnthalpy;
        this->MixHumRat = this->RetHumRat;
        this->MixPressure = this->RetPressure;
        this->MixTemp = this->RetTemp;
        return;
    }

    this->MixEnthalpy = (RecircMassFlowRate * RecircEnthalpy + this->OAMassFlowRate * this->OAEnthalpy) / this->MixMassFlowRate;
    this->MixHumRat = (RecircMassFlowRate * RecircHumRat + this->OAMassFlowRate * this->OAHumRat) / this->MixMassFlowRate;
    this->MixPressure = (RecircMassFlowRate * RecircPressure + this->OAMassFlowRate * this->OAPressure) / this->MixMassFlowRate;
    // Mixed air temperature is calculated from the mixed air enthalpy and humidity ratio.
    this->MixTemp = Psychrometrics::PsyTdbFnHW(this->MixEnthalpy, this->MixHumRat);

    // Check for saturation temperature > dry-bulb temperature and modify temperature at constant enthalpy
    Real64 T_sat = Psychrometrics::PsyTsatFnHPb(state, this->MixEnthalpy, this->MixPressure);
    if (this->MixTemp < T_sat) {
        this->MixTemp = T_sat;
        this->MixHumRat = Psychrometrics::PsyWFnTdbH(state, T_sat, this->MixEnthalpy);
    }
}

// End of Calculation/Simulation Section of the Module
//******************************************************************************

// Beginning Sizing Section of the Module
//******************************************************************************

void OAControllerProps::SizeOAController(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   September 2001

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing OAController Components for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static std::string_view const &CurrentModuleObject(CurrentModuleObjects[static_cast<int>(CMO::OAController)]);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound = false;
    if (this->MaxOA == AutoSize) {

        if (state.dataSize->CurSysNum > 0) {

            switch (this->ControllerType) {
            case MixedAirControllerType::ControllerOutsideAir: {
                CheckSysSizing(state, CurrentModuleObject, this->Name);
                switch (state.dataSize->CurDuctType) {
                case HVAC::AirDuctType::Cooling: {
                    this->MaxOA = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesCoolVolFlow;
                } break;
                case HVAC::AirDuctType::Heating: {
                    this->MaxOA = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesHeatVolFlow;
                } break;
                case HVAC::AirDuctType::Main:
                case HVAC::AirDuctType::Other:
                default: {
                    this->MaxOA = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } break;
                }
            } break;
            case MixedAirControllerType::ControllerStandAloneERV: {
            } break;
            default:
                break;
            }

        } else if (state.dataSize->CurZoneEqNum > 0) {

            switch (this->ControllerType) {
            case MixedAirControllerType::ControllerOutsideAir: {
                CheckZoneSizing(state, CurrentModuleObject, this->Name);
                this->MaxOA = max(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow,
                                  state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow);
            } break;
            case MixedAirControllerType::ControllerStandAloneERV: {
            } break;
            default:
                break;
            }
        }

        if (this->MaxOA < HVAC::SmallAirVolFlow) {
            this->MaxOA = 0.0;
        }

        BaseSizer::reportSizerOutput(state, CurrentModuleObject, this->Name, "Maximum Outdoor Air Flow Rate [m3/s]", this->MaxOA);
    }

    if (this->MinOA == AutoSize) {

        if (state.dataSize->CurSysNum > 0) {

            CheckSysSizing(state, CurrentModuleObject, this->Name);
            if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow >= HVAC::SmallAirVolFlow) {
                this->MinOA = min(state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow, this->MaxOA);
            } else {
                this->MinOA = 0.0;
            }
        }

        BaseSizer::reportSizerOutput(state, CurrentModuleObject, this->Name, "Minimum Outdoor Air Flow Rate [m3/s]", this->MinOA);

        if (this->HumidistatZoneNum > 0 && this->FixedMin) {
            if (this->MaxOA > 0.0) {
                Real64 OAFlowRatio = this->MinOA / this->MaxOA;
                if (this->HighRHOAFlowRatio < OAFlowRatio) {
                    ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, this->Name));
                    ShowContinueError(state, "... A fixed minimum outdoor air flow rate and high humidity control have been specified.");
                    ShowContinueError(state,
                                      "... The High Humidity Outdoor Air Flow Ratio is less than the ratio of the outdoor air controllers "
                                      "minimum to maximum outside air flow rate.");
                    ShowContinueError(state, format("... Controller minimum flow rate = {:.4T} m3/s.", this->MinOA));
                    ShowContinueError(state, format("... Controller maximum flow rate = {:.4T} m3/s.", this->MaxOA));
                    ShowContinueError(state, format("... Controller minimum to maximum flow ratio = {:.4T}.", OAFlowRatio));
                    ShowContinueError(state, format("... High humidity control flow ratio = {:.4T}.", this->HighRHOAFlowRatio));
                }
            }
        }
    }
    // If there is an outside air system, loop over components in the OA system; pass the design air flow rate
    // to the coil components that don't have design air flow as an input.
    if (state.dataSize->CurOASysNum > 0) {
        for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(state.dataSize->CurOASysNum).NumComponents; ++CompNum) {
            std::string const &CompType = state.dataAirLoop->OutsideAirSys(state.dataSize->CurOASysNum).ComponentType(CompNum);
            std::string const &CompName = state.dataAirLoop->OutsideAirSys(state.dataSize->CurOASysNum).ComponentName(CompNum);
            if (Util::SameString(CompType, "COIL:COOLING:WATER:DETAILEDGEOMETRY") || Util::SameString(CompType, "COIL:HEATING:WATER") ||
                Util::SameString(CompType, "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED")) {
                std::string CoilName;
                std::string CoilType;

                if (Util::SameString(CompType, "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED")) {
                    CoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, CompType, CompName, ErrorsFound);
                    CoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, CompType, CompName, ErrorsFound);
                } else {
                    CoilName = CompName;
                    CoilType = CompType;
                }
                WaterCoils::SetCoilDesFlow(state, CoilType, CoilName, this->MinOA, ErrorsFound);
            }
        } // End of component loop
    }
    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

// End of Sizing Section of the Module
//******************************************************************************

// Beginning Update/Reporting Section of the Module
//******************************************************************************

void OAControllerProps::UpdateOAController(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998
    //       MODIFIED       Shirey/Raustad FSEC, June 2003

    // PURPOSE OF THIS SUBROUTINE
    // Move the results of CalcOAController to the affected nodes

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int OutAirNodeNum = this->OANode;
    int InletAirNodeNum = this->InletNode;
    int RelAirNodeNum = this->RelNode;
    int RetAirNodeNum = this->RetNode;

    if (this->ControllerType == MixedAirControllerType::ControllerOutsideAir) {
        // The outside air controller sets the outside air flow rate and the relief air flow rate
        if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingSizing && (this->ManageDemand) &&
            (this->OAMassFlow > this->DemandLimitFlowRate)) {
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRate = this->DemandLimitFlowRate;
            state.dataLoopNodes->Node(InletAirNodeNum).MassFlowRate = this->DemandLimitFlowRate;
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRateMaxAvail = this->DemandLimitFlowRate;
        } else {
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRate = this->OAMassFlow;
            state.dataLoopNodes->Node(InletAirNodeNum).MassFlowRate = this->OAMassFlow;
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRateMaxAvail = this->OAMassFlow;
        }
        state.dataLoopNodes->Node(RelAirNodeNum).MassFlowRate = this->RelMassFlow;
    } else {
        // The ERV controller sets the supply and secondary inlet node information for the Stand Alone ERV
        // Currently, the Stand Alone ERV only has constant air flows (supply and exhaust), and these are
        // already set in HVACStandAloneERV.cc (subroutine init). Therefore, these flow assignments below are
        // currently redundant but may be useful in the future as mass flow rates can vary based on the controller signal.
        if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingSizing && (this->ManageDemand) &&
            (this->OAMassFlow > this->DemandLimitFlowRate)) {
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRate = this->DemandLimitFlowRate;
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRateMaxAvail = this->DemandLimitFlowRate;
        } else {
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRate = this->OAMassFlow;
            state.dataLoopNodes->Node(OutAirNodeNum).MassFlowRateMaxAvail = this->OAMassFlow;
        }
        state.dataLoopNodes->Node(RetAirNodeNum).MassFlowRate = state.dataLoopNodes->Node(this->RetNode).MassFlowRate;
        state.dataLoopNodes->Node(RetAirNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(this->RetNode).MassFlowRate;
    }
}

void OAMixerProps::UpdateOAMixer(EnergyPlusData &state) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Oct 1998

    // PURPOSE OF THIS SUBROUTINE
    // Move the results of CalcOAMixer to the affected nodes

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int MixNode = this->MixNode;
    int RelNode = this->RelNode;
    int RetNode = this->RetNode;
    // Move mixed air data to the mixed air node
    state.dataLoopNodes->Node(MixNode).MassFlowRate = this->MixMassFlowRate;
    state.dataLoopNodes->Node(MixNode).Temp = this->MixTemp;
    state.dataLoopNodes->Node(MixNode).HumRat = this->MixHumRat;
    state.dataLoopNodes->Node(MixNode).Enthalpy = this->MixEnthalpy;
    state.dataLoopNodes->Node(MixNode).Press = this->MixPressure;
    state.dataLoopNodes->Node(MixNode).MassFlowRateMaxAvail = this->MixMassFlowRate;
    // Move the relief air data to the relief air node
    state.dataLoopNodes->Node(RelNode).MassFlowRate = this->RelMassFlowRate;
    state.dataLoopNodes->Node(RelNode).Temp = this->RelTemp;
    state.dataLoopNodes->Node(RelNode).HumRat = this->RelHumRat;
    state.dataLoopNodes->Node(RelNode).Enthalpy = this->RelEnthalpy;
    state.dataLoopNodes->Node(RelNode).Press = this->RelPressure;
    state.dataLoopNodes->Node(RelNode).MassFlowRateMaxAvail = this->RelMassFlowRate;

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        state.dataLoopNodes->Node(RelNode).CO2 = state.dataLoopNodes->Node(RetNode).CO2;
        if (this->MixMassFlowRate <= HVAC::VerySmallMassFlow) {
            state.dataLoopNodes->Node(MixNode).CO2 = state.dataLoopNodes->Node(RetNode).CO2;
        } else {
            state.dataLoopNodes->Node(MixNode).CO2 =
                ((state.dataLoopNodes->Node(RetNode).MassFlowRate - state.dataLoopNodes->Node(RelNode).MassFlowRate) *
                     state.dataLoopNodes->Node(RetNode).CO2 +
                 this->OAMassFlowRate * state.dataContaminantBalance->OutdoorCO2) /
                this->MixMassFlowRate;
        }
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        state.dataLoopNodes->Node(RelNode).GenContam = state.dataLoopNodes->Node(RetNode).GenContam;
        if (this->MixMassFlowRate <= HVAC::VerySmallMassFlow) {
            state.dataLoopNodes->Node(MixNode).GenContam = state.dataLoopNodes->Node(RetNode).GenContam;
        } else {
            state.dataLoopNodes->Node(MixNode).GenContam =
                ((state.dataLoopNodes->Node(RetNode).MassFlowRate - state.dataLoopNodes->Node(RelNode).MassFlowRate) *
                     state.dataLoopNodes->Node(RetNode).GenContam +
                 this->OAMassFlowRate * state.dataContaminantBalance->OutdoorGC) /
                this->MixMassFlowRate;
        }
    }
}

// End of Sizing Section of the Module
//******************************************************************************

// Beginning Utility Section of the Module
//******************************************************************************

Array1D_int GetOAMixerNodeNumbers(EnergyPlusData &state,
                                  std::string const &OAMixerName, // must match OA mixer names for the OA mixer type
                                  bool &ErrorsFound               // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   June 2006

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given OA mixer and returns the node numbers.  If
    // incorrect OA mixer name is given, ErrorsFound is returned as true
    // as zero.

    // Return value
    Array1D_int OANodeNumbers(4); // return OA mixer nodes

    // Obtains and Allocates OA mixer related parameters from input file
    if (state.dataMixedAir->GetOAMixerInputFlag) { // First time subroutine has been entered
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int WhichOAMixer = Util::FindItemInList(OAMixerName, state.dataMixedAir->OAMixer);
    if (WhichOAMixer != 0) {
        OANodeNumbers(1) = state.dataMixedAir->OAMixer(WhichOAMixer).InletNode;
        OANodeNumbers(2) = state.dataMixedAir->OAMixer(WhichOAMixer).RelNode;
        OANodeNumbers(3) = state.dataMixedAir->OAMixer(WhichOAMixer).RetNode;
        OANodeNumbers(4) = state.dataMixedAir->OAMixer(WhichOAMixer).MixNode;
    }

    if (WhichOAMixer == 0) {
        ShowSevereError(state, format("GetOAMixerNodeNumbers: Could not find OA Mixer = \"{}\"", OAMixerName));
        ErrorsFound = true;
        OANodeNumbers = 0;
    }

    return OANodeNumbers;
}

int GetNumOAMixers(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of OA mixers is returned.

    if (state.dataMixedAir->GetOAMixerInputFlag) { // First time subroutine has been entered
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    return state.dataMixedAir->NumOAMixers;
}

int GetNumOAControllers(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of OA Controllers is returned.

    if (state.dataMixedAir->AllocateOAControllersFlag) {
        // Make sure OAControllers are allocated
        AllocateOAControllers(state);
    }

    return state.dataMixedAir->NumOAControllers;
}

int GetOAMixerReliefNodeNumber(EnergyPlusData &state, int const OAMixerNum) // Which Mixer
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the relief node number of indicated mixer is returned.

    if (state.dataMixedAir->GetOAMixerInputFlag) { // First time subroutine has been entered
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    if (OAMixerNum > state.dataMixedAir->NumOAMixers) {
        ShowFatalError(state,
                       format("GetOAMixerReliefNodeNumber: Requested Mixer #={}, which is > number of OA Mixers={}",
                              OAMixerNum,
                              state.dataMixedAir->NumOAMixers));
    }

    return state.dataMixedAir->OAMixer(OAMixerNum).RelNode;
}

int GetOASysControllerListIndex(EnergyPlusData &state, int const OASysNumber) // OA Sys Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 2007

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the Controller List index of the indicated OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNumber).ControllerListNum;
}

int GetOASysNumSimpControllers(EnergyPlusData &state, int const OASysNumber) // OA Sys Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 2007

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of Controller:Simple objects in the OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNumber).NumSimpleControllers;
}

int GetOASysNumHeatingCoils(EnergyPlusData &state, int const OASysNumber) // OA Sys Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2007

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of heating coils in the OA System is returned.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    bool Sim(false);
    bool FirstHVACIteration(false);
    bool OAHeatingCoil(false);
    bool OACoolingCoil(false);
    int AirLoopNum(0);
    bool OAHX(false);

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    int NumHeatingCoils = 0;
    for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNumber).NumComponents; ++CompNum) {
        std::string const &CompType = state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentType(CompNum);
        std::string const &CompName = state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentName(CompNum);
        SimOAComponent(state,
                       CompType,
                       CompName,
                       state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentTypeEnum(CompNum),
                       FirstHVACIteration,
                       state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentIndex(CompNum),
                       AirLoopNum,
                       Sim,
                       OASysNumber,
                       OAHeatingCoil,
                       OACoolingCoil,
                       OAHX);
        if (OAHeatingCoil) {
            ++NumHeatingCoils;
        }
    }

    return NumHeatingCoils;
}

int GetOASysNumHXs(EnergyPlusData &state, int const OASysNumber)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl, Rongpeng Zhang
    //       DATE WRITTEN   Oct. 2015

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of heat recovery exchangers in the OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    int NumHX = 0;

    auto const &componentType_Num = state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentTypeEnum;
    for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNumber).NumComponents; ++CompNum) {
        SimAirServingZones::CompType const componentTypeNum = componentType_Num(CompNum);
        if (SimAirServingZones::CompType::HeatXchngr == componentTypeNum || SimAirServingZones::CompType::Desiccant == componentTypeNum) {
            ++NumHX;
        }
    }

    return NumHX;
}

int GetOASysNumCoolingCoils(EnergyPlusData &state, int const OASysNumber) // OA Sys Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2007

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of cooling coils in the OA System is returned.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    bool Sim(false);
    bool FirstHVACIteration(false);
    bool OAHeatingCoil(false);
    bool OACoolingCoil(false);
    int AirLoopNum(0);
    bool OAHX(false);

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    int NumCoolingCoils = 0;
    for (int CompNum = 1; CompNum <= state.dataAirLoop->OutsideAirSys(OASysNumber).NumComponents; ++CompNum) {
        std::string const &CompType = state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentType(CompNum);
        std::string const &CompName = state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentName(CompNum);
        SimOAComponent(state,
                       CompType,
                       CompName,
                       state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentTypeEnum(CompNum),
                       FirstHVACIteration,
                       state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentIndex(CompNum),
                       AirLoopNum,
                       Sim,
                       OASysNumber,
                       OAHeatingCoil,
                       OACoolingCoil,
                       OAHX);
        if (OACoolingCoil) {
            ++NumCoolingCoils;
        }
    }

    return NumCoolingCoils;
}

int GetOASystemNumber(EnergyPlusData &state, std::string const &OASysName) // OA Sys Name
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the OA System number of indicated OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return Util::FindItemInList(OASysName, state.dataAirLoop->OutsideAirSys);
}

int FindOAMixerMatchForOASystem(EnergyPlusData &state, int const OASysNumber) // Which OA System
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the matched mixer number is found.
    // Note -- only the first is looked at for an Outside Air System.

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int OAMixerNumber = 0;
    if (OASysNumber > 0 && OASysNumber <= state.dataAirLoop->NumOASystems) {
        for (int OACompNum = 1; OACompNum <= state.dataAirLoop->OutsideAirSys(OASysNumber).NumComponents; ++OACompNum) {
            if (Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentType(OACompNum), "OUTDOORAIR:MIXER")) {
                OAMixerNumber =
                    Util::FindItemInList(state.dataAirLoop->OutsideAirSys(OASysNumber).ComponentName(OACompNum), state.dataMixedAir->OAMixer);
                break;
            }
        }
    }

    return OAMixerNumber;
}

int GetOAMixerIndex(EnergyPlusData &state, std::string const &OAMixerName) // Which Mixer
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 2010

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the mixer index of indicated mixer is returned.

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int OAMixerIndex = Util::FindItem(OAMixerName, state.dataMixedAir->OAMixer);

    if (OAMixerIndex == 0) {
        ShowSevereError(state, format("GetOAMixerIndex: Could not find OutdoorAir:Mixer, Name=\"{}\"", OAMixerName));
    }

    return OAMixerIndex;
}

int GetOAMixerInletNodeNumber(EnergyPlusData &state, int const OAMixerNumber) // Which Mixer
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the mixer inlet node number of indicated mixer is returned.

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int OAMixerInletNodeNumber = 0;
    if (OAMixerNumber > 0 && OAMixerNumber <= state.dataMixedAir->NumOAMixers) {
        OAMixerInletNodeNumber = state.dataMixedAir->OAMixer(OAMixerNumber).InletNode;
    }

    return OAMixerInletNodeNumber;
}

int GetOAMixerReturnNodeNumber(EnergyPlusData &state, int const OAMixerNumber) // Which Mixer
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   December 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the mixer return node number of indicated mixer is returned.

    // METHODOLOGY EMPLOYED:
    // followed Linda Lawrie's GetOAMixerInletNodeNumber

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int OAMixerReturnNodeNumber = 0;
    if (OAMixerNumber > 0 && OAMixerNumber <= state.dataMixedAir->NumOAMixers) {
        OAMixerReturnNodeNumber = state.dataMixedAir->OAMixer(OAMixerNumber).RetNode;
    }

    return OAMixerReturnNodeNumber;
}

int GetOAMixerMixedNodeNumber(EnergyPlusData &state, int const OAMixerNumber) // Which Mixer
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   December 2006

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the mixer mixed air node number of indicated mixer is returned.

    if (state.dataMixedAir->GetOAMixerInputFlag) {
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    int OAMixerMixedNodeNumber = 0;
    if (OAMixerNumber > 0 && OAMixerNumber <= state.dataMixedAir->NumOAMixers) {
        OAMixerMixedNodeNumber = state.dataMixedAir->OAMixer(OAMixerNumber).MixNode;
    }

    return OAMixerMixedNodeNumber;
}

bool CheckForControllerWaterCoil(EnergyPlusData &state,
                                 DataAirLoop::ControllerKind ControllerType, // should be passed in as UPPERCASE
                                 std::string const &ControllerName           // should be passed in as UPPERCASE
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS FUNCTION:
    // This routine checks the controller list for existence of the reference coil.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    int OnControllerList = false;

    for (int Num = 1; Num <= state.dataMixedAir->NumControllerLists; ++Num) {
        for (int CompNum = 1; CompNum <= state.dataMixedAir->ControllerLists(Num).NumControllers; ++CompNum) {

            if (state.dataMixedAir->ControllerLists(Num).ControllerType(CompNum) != ControllerType) {
                continue;
            }
            if (!Util::SameString(state.dataMixedAir->ControllerLists(Num).ControllerName(CompNum), ControllerName)) {
                continue;
            }
            OnControllerList = true;
            break;
        }
    }

    return OnControllerList;
}

void CheckControllerLists(EnergyPlusData &state, bool &ErrFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // This routine checks for a "dangling" controller list (AirLoopHVAC:ControllerList).
    // It must be either found on a AirLoopHVAC or AirLoopHVAC:OutdoorAirSystem.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static std::string const CurrentModuleObject("AirLoopHVAC:ControllerList");
    static std::string const AirLoopObject("AirLoopHVAC");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;
    int NumNumbers;
    int IOStat;

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    int NumControllers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    int NumAirLoop = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, AirLoopObject);
    std::string_view AirLoopName = "";

    for (int Item = 1; Item <= NumControllers; ++Item) {

        state.dataInputProcessing->inputProcessor->getObjectItem(
            state, CurrentModuleObject, Item, state.dataIPShortCut->cAlphaArgs, NumAlphas, state.dataIPShortCut->rNumericArgs, NumNumbers, IOStat);
        std::string const ControllerListName = state.dataIPShortCut->cAlphaArgs(1);
        int Count = 0;

        // Check AirLoopHVAC -- brute force, get each AirLoopHVAC

        for (int Loop = 1; Loop <= NumAirLoop; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, AirLoopObject, Loop, state.dataIPShortCut->cAlphaArgs, NumAlphas, state.dataIPShortCut->rNumericArgs, NumNumbers, IOStat);
            if (state.dataIPShortCut->cAlphaArgs(2) != ControllerListName) {
                continue;
            }
            if (++Count == 1) {
                AirLoopName = state.dataIPShortCut->cAlphaArgs(1);
            }
        }

        //  Now check AirLoopHVAC and AirLoopHVAC:OutdoorAirSystem
        int Found = 0;
        if (state.dataAirLoop->NumOASystems > 0) {
            Found = Util::FindItemInList(ControllerListName, state.dataAirLoop->OutsideAirSys, &OutsideAirSysProps::ControllerListName);
            if (Found > 0) {
                ++Count;
            }
        }

        if (Count == 0) {
            ShowSevereError(state,
                            format("{}=\"{}\" is not referenced on a AirLoopHVAC or AirLoopHVAC:OutdoorAirSystem object.",
                                   CurrentModuleObject,
                                   ControllerListName));
            ErrFound = true;
        } else if (Count > 1) {
            ShowSevereError(state,
                            format("{}=\"{}\" has too many references on AirLoopHVAC or AirLoopHVAC:OutdoorAirSystem objects.",
                                   CurrentModuleObject,
                                   ControllerListName));
            if (Found > 0) {
                ShowContinueError(state, format("...AirLoopHVAC:OutdoorAirSystem=\"{}\".", state.dataAirLoop->OutsideAirSys(Found).Name));
            }
            ShowContinueError(state, format("...also on AirLoopHVAC=\"{}\".", AirLoopName));
            ErrFound = true;
        }
    }
}

void CheckOAControllerName(
    EnergyPlusData &state, std::string &OAControllerName, std::string const &ObjectType, std::string const &FieldName, bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS SUBROUTINE:
    // When OA Controller data is gotten from other routines, must check to make sure
    // new name doesn't duplicate.  (Essentially a pass through to call Verify Name)
    // Currently, this is only called from HVACStandAlongERV::GetStandaloneERV()

    if (state.dataMixedAir->AllocateOAControllersFlag) {
        // Make sure OAControllers are allocated
        AllocateOAControllers(state);
    }

    GlobalNames::VerifyUniqueInterObjectName(
        state, state.dataMixedAir->OAControllerUniqueNames, OAControllerName, ObjectType, FieldName, ErrorsFound);
}

void OAControllerProps::Checksetpoints(EnergyPlusData &state,
                                       Real64 const OutAirMinFrac,   // Local variable used to calculate min OA fraction
                                       Real64 &OutAirSignal,         // Used to set OA mass flow rate
                                       bool &EconomizerOperationFlag // logical used to show economizer status
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Amit bhansali
    //       DATE WRITTEN   August 2008?

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine checks the setpoints of the upper limits of temperatures, limit enthalpy
    // Limit dew point, Enthalpy curve

    if (this->TempLim != HVAC::BlankNumeric && this->OATemp > this->TempLim) {
        OutAirSignal = OutAirMinFrac;
        EconomizerOperationFlag = false;
    }
    // Outside air enthalpy limit
    if (this->EnthLim != HVAC::BlankNumeric && this->OAEnth > this->EnthLim) {
        OutAirSignal = OutAirMinFrac;
        EconomizerOperationFlag = false;
    }

    if (this->DPTempLim != HVAC::BlankNumeric) {
        Real64 OADPTemp = Psychrometrics::PsyTdpFnWPb(state, this->OAHumRat, this->OAPress);
        if (OADPTemp > this->DPTempLim) {
            OutAirSignal = OutAirMinFrac;
            EconomizerOperationFlag = false;
        }
    }

    if (this->EnthalpyCurvePtr > 0) {
        if (this->OAHumRat > Curve::CurveValue(state, this->EnthalpyCurvePtr, this->OATemp)) {
            OutAirSignal = OutAirMinFrac;
            EconomizerOperationFlag = false;
        }
    }
}

int GetNumOASystems(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2010

    // PURPOSE OF THIS FUNCTION:
    // Get Number of OA Systems, After making sure get input is done

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->NumOASystems;
}

int GetOACompListNumber(EnergyPlusData &state, int const OASysNum) // OA Sys Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Heejin Cho
    //       DATE WRITTEN   November 2010

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the OA System number of indicated OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents;
}

std::string GetOACompName(EnergyPlusData &state,
                          int const OASysNum, // OA Sys Number
                          int const InListNum // In-list Number
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Heejin Cho
    //       DATE WRITTEN   November 2010

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of heating coils in the OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(InListNum);
}

std::string GetOACompType(EnergyPlusData &state,
                          int const OASysNum, // OA Sys Number
                          int const InListNum // In-list Number
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Heejin Cho
    //       DATE WRITTEN   November 2010

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of heating coils in the OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(InListNum);
}

SimAirServingZones::CompType GetOACompTypeNum(EnergyPlusData &state,
                                              int const OASysNum, // OA Sys Number
                                              int const InListNum // In-list Number
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Heejin Cho
    //       DATE WRITTEN   November 2010

    // PURPOSE OF THIS FUNCTION:
    // After making sure get input is done, the number of heating coils in the OA System is returned.

    if (state.dataMixedAir->GetOASysInputFlag) {
        GetOutsideAirSysInputs(state);
        state.dataMixedAir->GetOASysInputFlag = false;
    }

    return state.dataAirLoop->OutsideAirSys(OASysNum).ComponentTypeEnum(InListNum);
}

int GetOAMixerNumber(EnergyPlusData &state, std::string const &OAMixerName // must match OA mixer names for the OA mixer type
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   Feb. 2018

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given OA mixer and returns the OAMixer number.  If
    // incorrect OA mixer name is given, ErrorsFound is returned as true

    // Obtains and Allocates OA mixer related parameters from input file
    if (state.dataMixedAir->GetOAMixerInputFlag) { // First time subroutine has been entered
        GetOAMixerInputs(state);
        state.dataMixedAir->GetOAMixerInputFlag = false;
    }

    return Util::FindItemInList(OAMixerName, state.dataMixedAir->OAMixer);
}
// End of Utility Section of the Module
//******************************************************************************

} // namespace EnergyPlus::MixedAir
