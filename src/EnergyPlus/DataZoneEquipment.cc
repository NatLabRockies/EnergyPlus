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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::DataZoneEquipment {

// MODULE INFORMATION
//             AUTHOR:  Russ Taylor
//       DATE WRITTEN:  June 1998

// PURPOSE OF THIS MODULE:
// This module contains variable declarations for zone equipment configuration data

Array1D_string const cValidSysAvailManagerCompTypes(NumValidSysAvailZoneComponents,
                                                    {"ZoneHVAC:FourPipeFanCoil",
                                                     "ZoneHVAC:PackagedTerminalHeatPump",
                                                     "ZoneHVAC:PackagedTerminalAirConditioner",
                                                     "ZoneHVAC:WaterToAirHeatPump",
                                                     "ZoneHVAC:WindowAirConditioner",
                                                     "ZoneHVAC:UnitHeater",
                                                     "ZoneHVAC:UnitVentilator",
                                                     "ZoneHVAC:EnergyRecoveryVentilator",
                                                     "ZoneHVAC:VentilatedSlab",
                                                     "ZoneHVAC:OutdoorAirUnit",
                                                     "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow",
                                                     "ZoneHVAC:IdealLoadsAirSystem",
                                                     "ZoneHVAC:EvaporativeCoolerUnit",
                                                     "ZoneHVAC:HybridUnitaryHVAC"});

constexpr std::array<std::string_view, static_cast<int>(ZoneEquipType::Num)> zoneEquipTypeNamesUC = {
    "DUMMY", // DUMMY,

    "ZONEHVAC:FOURPIPEFANCOIL",                      // FanCoilFourPipe
    "ZONEHVAC:PACKAGEDTERMINALHEATPUMP",             //   PackagedTerminalHeatPump
    "ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER",       //  PackagedTerminalAirConditioner
    "ZONEHVAC:WATERTOAIRHEATPUMP",                   //   PackagedTerminalHeatPumpWaterToAir
    "ZONEHVAC:WINDOWAIRCONDITIONER",                 //  WindowAirConditioner
    "ZONEHVAC:UNITHEATER",                           //  UnitHeater
    "ZONEHVAC:UNITVENTILATOR",                       //  UnitVentilator
    "ZONEHVAC:ENERGYRECOVERYVENTILATOR",             // EnergyRecoveryVentilator
    "ZONEHVAC:VENTILATEDSLAB",                       // VentilatedSlab
    "ZONEHVAC:OUTDOORAIRUNIT",                       // OutdoorAirUnit
    "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW", //  VariableRefrigerantFlowTerminal
    "ZONEHVAC:IDEALLOADSAIRSYSTEM",                  // IdealLoadsAirSystem
    "ZONEHVAC:EVAPORATIVECOOLERUNIT",                // EvaporativeCooler
    "ZONEHVAC:HYBRIDUNITARYHVAC",                    // HybridEvaporativeCooler,

    // last zone equipment type to use zone availability manager. The above list must not change or
    // NumValidSysAvailZoneComponents must also change.

    "ZONEHVAC:AIRDISTRIBUTIONUNIT",                            // AirDistributionUnit
    "ZONEHVAC:BASEBOARD:CONVECTIVE:WATER",                     // BaseboardWaterConvective
    "ZONEHVAC:BASEBOARD:CONVECTIVE:ELECTRIC",                  // BaseboardElectricConvective
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM",              // BaseboardSteam
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER",              // BaseboardWater
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:ELECTRIC",           // BaseboardElectric
    "ZONEHVAC:HIGHTEMPERATURERADIANT",                         // HighTempRadiant
    "ZONEHVAC:LOWTEMPERATURERADIANT:CONSTANTFLOW",             //  LowTempRadiantConstFlow
    "ZONEHVAC:LOWTEMPERATURERADIANT:VARIABLEFLOW",             //  LowTempRadiantVarFlow
    "ZONEHVAC:LOWTEMPERATURERADIANT:ELECTRIC",                 //  LowTempRadiantElectric
    "FAN:ZONEEXHAUST",                                         // ExhaustFan
    "HEATEXCHANGER:AIRTOAIR:FLATPLATE",                        // HeatExchanger
    "WATERHEATER:HEATPUMP:PUMPEDCONDENSER",                    //  HeatPumpWaterHeaterPumpedCondenser
    "WATERHEATER:HEATPUMP:WRAPPEDCONDENSER",                   //  HeatPumpWaterHeaterWrappedCondenser
    "ZONEHVAC:DEHUMIDIFIER:DX",                                //  DXDehumidifier
    "ZONEHVAC:REFRIGERATIONCHILLERSET",                        // RefrigerationAirChillerSet
    "ZONEHVAC:FORCEDAIR:USERDEFINED",                          // UserDefinedVACForcedAir
    "ZONEHVAC:COOLINGPANEL:RADIANTCONVECTIVE:WATER",           // CoolingPanel
    "AIRLOOPHVAC:UNITARYSYSTEM",                               // UnitarySystem
    "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME",                     // AirTerminalDualDuctConstantVolume
    "AIRTERMINAL:DUALDUCT:VAV",                                // AirTerminalDualDuctVAV
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT",            // AirTerminalSingleDuctConstantVolumeReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT",          // AirTerminalSingleDuctConstantVolumeNoReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT",                       // AirTerminalSingleDuctVAVReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT",                     // AirTerminalSingleDuctVAVNoReheat
    "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",                 // AirTerminalSingleDuctSeriesPIUReheat
    "AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",               // AirTerminalSingleDuctParallelPIUReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION", // AirTerminalSingleDuctCAVFourPipeInduction
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN",      // AirTerminalSingleDuctVAVReheatVariableSpeedFan
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT",           // AirTerminalSingleDuctVAVHeatAndCoolReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT",         // AirTerminalSingleDuctVAVHeatAndCoolNoReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM",        // AirTerminalSingleDuctConstantVolumeCooledBeam
    "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR",                     // AirTerminalDualDuctVAVOutdoorAir
    "AIRLOOPHVACRETURNAIR"                                     // AirLoopHVACReturnAir
};

static constexpr std::array<std::string_view, static_cast<int>(LoadDist::Num)> LoadDistNamesUC = {
    "SEQUENTIALLOAD", "UNIFORMLOAD", "UNIFORMPLR", "SEQUENTIALUNIFORMPLR"};

static constexpr std::array<std::string_view, static_cast<int>(ZoneEquipTstatControl::Num)> zoneEquipTstatControlNamesUC = {
    "SINGLESPACE", "MAXIMUM", "IDEAL"};

static constexpr std::array<std::string_view, static_cast<int>(SpaceEquipSizingBasis::Num)> spaceEquipSizingBasisNamesUC = {
    "DESIGNCOOLINGLOAD", "DESIGNHEATINGLOAD", "FLOORAREA", "VOLUME", "PERIMETERLENGTH"};

void GetZoneEquipmentData(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1997
    //       MODIFIED       Aug 2003, FCW: set ZoneEquipConfig number for each zone

    // PURPOSE OF THIS SUBROUTINE:
    // Get all the system related equipment which may be attached to
    // a zone

    // Using/Aliasing
    using NodeInputManager::CheckUniqueNodeNames;
    using NodeInputManager::CheckUniqueNodeNumbers;
    using NodeInputManager::EndUniqueNodeCheck;
    using NodeInputManager::GetNodeNums;
    using NodeInputManager::GetOnlySingleNode;
    using NodeInputManager::InitUniqueNodeCheck;
    using namespace DataLoopNode;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetZoneEquipmentData: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;
    int NumNums;
    int IOStat;
    Array1D_string AlphArray;
    Array1D<Real64> NumArray;
    int MaxAlphas;
    int MaxNums;
    int NumParams;
    Array1D_int NodeNums;
    bool IsNotOK;                    // Flag to verify nam
    std::string CurrentModuleObject; // Object type for getting and error messages
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

    struct EquipListAudit
    {
        // Members
        std::string ObjectType;
        std::string ObjectName;
        int OnListNum;

        // Default Constructor
        EquipListAudit() : OnListNum(0)
        {
        }
    };
    // Object Data
    Array1D<EquipListAudit> ZoneEquipListAcct;

    // Look in the input file for zones with air loop and zone equipment attached

    int numControlledZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:EquipmentConnections");
    int numControlledSpaces = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "SpaceHVAC:EquipmentConnections");
    state.dataZoneEquip->NumOfZoneEquipLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(
        state, "ZoneHVAC:EquipmentList"); // Look for lists of equipment data - there should
    // be as many of these as there are controlled zones
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNums);
    NodeNums.dimension(NumParams, 0);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneHVAC:EquipmentList", NumParams, NumAlphas, NumNums);
    MaxAlphas = NumAlphas;
    MaxNums = NumNums;
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneHVAC:EquipmentConnections", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "SpaceHVAC:EquipmentConnections", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC:SupplyPath", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC:ReturnPath", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    AlphArray.allocate(MaxAlphas);
    NumArray.dimension(MaxNums, 0.0);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNums);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNums, true);

    if (!allocated(state.dataZoneEquip->SupplyAirPath)) {
        // Look for and read in the air supply path
        // component (splitters) information for each zone
        state.dataZoneEquip->NumSupplyAirPaths = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:SupplyPath");
        state.dataZoneEquip->SupplyAirPath.allocate(state.dataZoneEquip->NumSupplyAirPaths);
    }

    if (!allocated(state.dataZoneEquip->ReturnAirPath)) {
        // Look for and read in the air return path
        // component (mixers & plenums) information for each zone
        state.dataZoneEquip->NumReturnAirPaths = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ReturnPath");
        state.dataZoneEquip->ReturnAirPath.allocate(state.dataZoneEquip->NumReturnAirPaths);
    }

    state.dataZoneEquip->ZoneEquipConfig.allocate(state.dataGlobal->NumOfZones); // Allocate the array containing the configuration data for each zone
    if (state.dataHeatBal->doSpaceHeatBalanceSizing || state.dataHeatBal->doSpaceHeatBalanceSimulation) {
        state.dataZoneEquip->spaceEquipConfig.allocate(
            state.dataGlobal->numSpaces); // Allocate the array containing the configuration data for each space
    }
    state.dataZoneEquip->ZoneEquipList.allocate(state.dataGlobal->NumOfZones);
    state.dataZoneEquip->ZoneEquipAvail.dimension(state.dataGlobal->NumOfZones, Avail::Status::NoAction);
    state.dataZoneEquip->UniqueZoneEquipListNames.reserve(state.dataGlobal->NumOfZones);

    if (state.dataZoneEquip->NumOfZoneEquipLists != numControlledZones) {
        ShowSevereError(state,
                        format("{}Number of Zone Equipment lists [{}] not equal Number of Controlled Zones [{}]",
                               RoutineName,
                               state.dataZoneEquip->NumOfZoneEquipLists,
                               numControlledZones));
        ShowContinueError(state, "..Each Controlled Zone [ZoneHVAC:EquipmentConnections] must have a corresponding (unique) ZoneHVAC:EquipmentList");
        ShowFatalError(state, "GetZoneEquipment: Incorrect number of zone equipment lists");
    }

    if (numControlledZones > state.dataGlobal->NumOfZones) {
        ShowSevereError(state,
                        format("{}Number of Controlled Zone objects [{}] greater than Number of Zones [{}]",
                               RoutineName,
                               numControlledZones,
                               state.dataGlobal->NumOfZones));
        ShowFatalError(state, format("{}Too many ZoneHVAC:EquipmentConnections objects.", RoutineName));
    }

    InitUniqueNodeCheck(state, "ZoneHVAC:EquipmentConnections");

    int overallEquipCount = 0;
    int locTermUnitSizingCounter = 0; // will increment for every zone inlet node

    // auto &Zone(state.dataHeatBal->Zone);

    for (int controlledZoneLoop = 1; controlledZoneLoop <= numControlledZones; ++controlledZoneLoop) {
        CurrentModuleObject = "ZoneHVAC:EquipmentConnections";
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 controlledZoneLoop,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); // Get Equipment | data for one zone

        int zoneNum = Util::FindItemInList(AlphArray(1), state.dataHeatBal->Zone);
        std::string_view zsString = "Zone";

        if (zoneNum == 0) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Requested Controlled {} not among {}s, remaining items for this object not processed.", zsString, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        auto &thisZone = state.dataHeatBal->Zone(zoneNum);
        // Is this a duplicate for the same zone?
        if (thisZone.IsControlled) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Duplicate Controlled {} entered, only one {} per {} is allowed.", zsString, CurrentModuleObject, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        thisZone.IsControlled = true;
        bool isSpace = false;
        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
        processZoneEquipmentInput(state,
                                  CurrentModuleObject,
                                  zoneNum,
                                  isSpace,
                                  locTermUnitSizingCounter,
                                  overallEquipCount,
                                  thisZoneEquipConfig,
                                  AlphArray,
                                  cAlphaFields,
                                  lAlphaBlanks,
                                  NodeNums);
        thisZone.SystemZoneNodeNumber = thisZoneEquipConfig.ZoneNode;
    } // end loop over controlled zones
    for (int controlledSpaceLoop = 1; controlledSpaceLoop <= numControlledSpaces; ++controlledSpaceLoop) {
        CurrentModuleObject = "SpaceHVAC:EquipmentConnections";
        if (!state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            ShowWarningError(
                state,
                format("{} requires \"Do Space Heat Balance for Simulation = Yes\" in ZoneAirHeatBalanceAlgorithm. {} objects will be ignored.",
                       CurrentModuleObject,
                       CurrentModuleObject));
            break;
        }
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 controlledSpaceLoop,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); // Get Equipment | data for one zone

        int spaceNum = Util::FindItemInList(AlphArray(1), state.dataHeatBal->space);
        std::string_view zsString = "Space";

        if (spaceNum == 0) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Requested Controlled {} not among {}s, remaining items for this object not processed.", zsString, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        auto &thisSpace = state.dataHeatBal->space(spaceNum);
        int zoneNum = thisSpace.zoneNum;
        if (!state.dataHeatBal->Zone(zoneNum).IsControlled) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisSpace.Name));
            ShowContinueError(state,
                              format("..Zone Name={} is not a controlled zone. A ZoneHVAC:EquipmentConnections object is required for this zone.",
                                     state.dataHeatBal->Zone(zoneNum).Name));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        // Is this a duplicate for the same space?
        if (thisSpace.IsControlled) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Duplicate Controlled {} entered, only one {} per {} is allowed.", zsString, CurrentModuleObject, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        thisSpace.IsControlled = true;
        bool isSpace = true;
        auto &thisSpaceEquipConfig = state.dataZoneEquip->spaceEquipConfig(spaceNum);
        processZoneEquipmentInput(state,
                                  CurrentModuleObject,
                                  spaceNum,
                                  isSpace,
                                  locTermUnitSizingCounter,
                                  overallEquipCount,
                                  thisSpaceEquipConfig,
                                  AlphArray,
                                  cAlphaFields,
                                  lAlphaBlanks,
                                  NodeNums);
        thisSpace.SystemZoneNodeNumber = thisSpaceEquipConfig.ZoneNode;
    } // end loop over controlled spaces

    if (state.dataHeatBal->doSpaceHeatBalanceSizing || state.dataHeatBal->doSpaceHeatBalanceSimulation) {
        // Auto-assign the system node name for spaces in controlled zones that do not have a SpaceHVAC:EquipmentConnections input
        for (auto &thisZone : state.dataHeatBal->Zone) {
            if (!thisZone.IsControlled) {
                continue;
            }
            int spaceCount = 0;
            for (int spaceNum : thisZone.spaceIndexes) {
                ++spaceCount;
                if (state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber == 0) {
                    std::string spaceNodeName = format("{}-Space {}", state.dataLoopNodes->NodeID(thisZone.SystemZoneNodeNumber), spaceCount);
                    int spaceNodeNum = GetOnlySingleNode(state,
                                                         spaceNodeName,
                                                         state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                                                         AlphArray(1),
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::ZoneNode,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsNotParent);
                    state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber = spaceNodeNum;
                }
            }
        }
    }
    // Allocate TermUnitSizing array and set zone number
    if (locTermUnitSizingCounter > 0) {
        state.dataSize->NumAirTerminalUnits = locTermUnitSizingCounter;
        state.dataSize->TermUnitSizing.allocate(state.dataSize->NumAirTerminalUnits);
        for (int loopZoneNum = 1; loopZoneNum <= state.dataGlobal->NumOfZones; ++loopZoneNum) {
            {
                auto &thisZoneEqConfig = state.dataZoneEquip->ZoneEquipConfig(loopZoneNum);
                for (int loopNodeNum = 1; loopNodeNum <= thisZoneEqConfig.NumInletNodes; ++loopNodeNum) {
                    state.dataSize->TermUnitSizing(thisZoneEqConfig.AirDistUnitCool(loopNodeNum).TermUnitSizingIndex).CtrlZoneNum = loopZoneNum;
                }
            }
        }
    }
    if (state.dataZoneEquip->GetZoneEquipmentDataErrorsFound) {
        ShowWarningError(state, format("{}{}, duplicate items NOT CHECKED due to previous errors.", RoutineName, CurrentModuleObject));
        overallEquipCount = 0;
    }
    if (overallEquipCount > 0) {
        ZoneEquipListAcct.allocate(overallEquipCount);
        overallEquipCount = 0;
        for (int Loop1 = 1; Loop1 <= numControlledZones; ++Loop1) {
            for (int Loop2 = 1; Loop2 <= state.dataZoneEquip->ZoneEquipList(Loop1).NumOfEquipTypes; ++Loop2) {
                ++overallEquipCount;
                ZoneEquipListAcct(overallEquipCount).ObjectType = state.dataZoneEquip->ZoneEquipList(Loop1).EquipType(Loop2);
                ZoneEquipListAcct(overallEquipCount).ObjectName = state.dataZoneEquip->ZoneEquipList(Loop1).EquipName(Loop2);
                ZoneEquipListAcct(overallEquipCount).OnListNum = Loop1;
            }
        }
        // Now check for uniqueness
        for (int Loop1 = 1; Loop1 <= overallEquipCount; ++Loop1) {
            for (int Loop2 = Loop1 + 1; Loop2 <= overallEquipCount; ++Loop2) {
                if (ZoneEquipListAcct(Loop1).ObjectType != ZoneEquipListAcct(Loop2).ObjectType ||
                    ZoneEquipListAcct(Loop1).ObjectName != ZoneEquipListAcct(Loop2).ObjectName) {
                    continue;
                }
                // Duplicated -- not allowed
                ShowSevereError(state, format("{}{}, duplicate items in ZoneHVAC:EquipmentList.", RoutineName, CurrentModuleObject));
                ShowContinueError(state,
                                  format("Equipment: Type={}, Name={}", ZoneEquipListAcct(Loop1).ObjectType, ZoneEquipListAcct(Loop1).ObjectName));
                ShowContinueError(state,
                                  format("Found on List=\"{}\".", state.dataZoneEquip->ZoneEquipList(ZoneEquipListAcct(Loop1).OnListNum).Name));
                ShowContinueError(
                    state,
                    format("Equipment Duplicated on List=\"{}\".", state.dataZoneEquip->ZoneEquipList(ZoneEquipListAcct(Loop2).OnListNum).Name));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
        ZoneEquipListAcct.deallocate();
    }

    // map ZoneEquipConfig%EquipListIndex to ZoneEquipList%Name

    for (int ControlledZoneLoop = 1; ControlledZoneLoop <= state.dataGlobal->NumOfZones; ++ControlledZoneLoop) {
        state.dataZoneEquip->GetZoneEquipmentDataFound = Util::FindItemInList(
            state.dataZoneEquip->ZoneEquipList(ControlledZoneLoop).Name, state.dataZoneEquip->ZoneEquipConfig, &EquipConfiguration::EquipListName);
        if (state.dataZoneEquip->GetZoneEquipmentDataFound > 0) {
            state.dataZoneEquip->ZoneEquipConfig(state.dataZoneEquip->GetZoneEquipmentDataFound).EquipListIndex = ControlledZoneLoop;
        }
    } // end loop over controlled zones

    EndUniqueNodeCheck(state, "ZoneHVAC:EquipmentConnections");

    auto &ip = state.dataInputProcessing->inputProcessor;

    CurrentModuleObject = "SpaceHVAC:ZoneEquipmentSplitter";
    auto instances = ip->epJSON.find(CurrentModuleObject);
    if (instances != ip->epJSON.end()) {
        if (!state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            ShowWarningError(
                state,
                format("{} requires \"Do Space Heat Balance for Simulation = Yes\" in ZoneAirHeatBalanceAlgorithm. {} objects will be ignored.",
                       CurrentModuleObject,
                       CurrentModuleObject));
        } else {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, CurrentModuleObject);
            auto &instancesValue = instances.value();
            int numZoneEqSplitters = instancesValue.size();
            state.dataZoneEquip->zoneEquipSplitter.resize(numZoneEqSplitters);
            int zeqSplitterNum = -1;
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++zeqSplitterNum;
                auto const &objectFields = instance.value();
                auto &thisZeqSplitter = state.dataZoneEquip->zoneEquipSplitter[zeqSplitterNum];
                thisZeqSplitter.Name = Util::makeUPPER(instance.key());
                thisZeqSplitter.spaceEquipType = DataLoopNode::ConnectionObjectType::SpaceHVACZoneEquipmentSplitter;
                ip->markObjectAsUsed(CurrentModuleObject, instance.key());

                std::string zoneName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_name");
                int zoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
                if (zoneNum == 0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZeqSplitter.Name));
                    ShowContinueError(state, format("..Zone Name={} not found, remaining items for this object not processed.", zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }
                if (!state.dataHeatBal->Zone(zoneNum).IsControlled) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZeqSplitter.Name));
                    ShowContinueError(
                        state,
                        format("..Zone Name={} is not a controlled zone. A ZoneHVAC:EquipmentConnections object is required for this zone.",
                               zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }

                processZoneEquipSplitterInput(state, CurrentModuleObject, zeqSplitterNum, zoneNum, objectSchemaProps, objectFields, thisZeqSplitter);
            } // end loop over zone equipment splitters
        }
    }

    CurrentModuleObject = "SpaceHVAC:ZoneEquipmentMixer";
    instances = ip->epJSON.find(CurrentModuleObject);
    if (instances != ip->epJSON.end()) {
        if (!state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            ShowWarningError(
                state,
                format("{} requires \"Do Space Heat Balance for Simulation = Yes\" in ZoneAirHeatBalanceAlgorithm. {} objects will be ignored.",
                       CurrentModuleObject,
                       CurrentModuleObject));
        } else {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, CurrentModuleObject);
            auto &instancesValue = instances.value();
            int numZoneEqMixers = instancesValue.size();
            state.dataZoneEquip->zoneEquipMixer.resize(numZoneEqMixers);
            int zeqMixerNum = -1;
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++zeqMixerNum;
                auto const &objectFields = instance.value();
                auto &thisZeqMixer = state.dataZoneEquip->zoneEquipMixer[zeqMixerNum];
                thisZeqMixer.Name = Util::makeUPPER(instance.key());
                thisZeqMixer.spaceEquipType = DataLoopNode::ConnectionObjectType::SpaceHVACZoneEquipmentMixer;
                ip->markObjectAsUsed(CurrentModuleObject, instance.key());

                std::string zoneName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_name");
                int zoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
                if (zoneNum == 0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZeqMixer.Name));
                    ShowContinueError(state, format("..Zone Name={} not found, remaining items for this object not processed.", zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }
                if (!state.dataHeatBal->Zone(zoneNum).IsControlled) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZeqMixer.Name));
                    ShowContinueError(
                        state,
                        format("..Zone Name={} is not a controlled zone. A ZoneHVAC:EquipmentConnections object is required for this zone.",
                               zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }

                processZoneEquipMixerInput(state, CurrentModuleObject, zoneNum, objectSchemaProps, objectFields, thisZeqMixer);
            } // end loop over zone equipment mixers
        }
    }

    CurrentModuleObject = "SpaceHVAC:ZoneReturnMixer";
    instances = ip->epJSON.find(CurrentModuleObject);
    if (instances != ip->epJSON.end()) {
        if (!state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            ShowWarningError(
                state,
                format("{} requires \"Do Space Heat Balance for Simulation = Yes\" in ZoneAirHeatBalanceAlgorithm. {} objects will be ignored.",
                       CurrentModuleObject,
                       CurrentModuleObject));
        } else {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, CurrentModuleObject);
            auto &instancesValue = instances.value();
            int numZoneRetMixers = instancesValue.size();
            state.dataZoneEquip->zoneReturnMixer.resize(numZoneRetMixers);
            int zeqRetNum = -1;
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++zeqRetNum;
                auto const &objectFields = instance.value();
                auto &thisZretMixer = state.dataZoneEquip->zoneReturnMixer[zeqRetNum];
                thisZretMixer.Name = Util::makeUPPER(instance.key());
                thisZretMixer.spaceEquipType = DataLoopNode::ConnectionObjectType::SpaceHVACZoneReturnMixer;
                ip->markObjectAsUsed(CurrentModuleObject, instance.key());

                std::string zoneName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_name");
                int zoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
                if (zoneNum == 0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZretMixer.Name));
                    ShowContinueError(state, format("..Zone Name={} not found, remaining items for this object not processed.", zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }
                if (!state.dataHeatBal->Zone(zoneNum).IsControlled) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, thisZretMixer.Name));
                    ShowContinueError(
                        state,
                        format("..Zone Name={} is not a controlled zone. A ZoneHVAC:EquipmentConnections object is required for this zone.",
                               zoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    continue;
                }

                processZoneReturnMixerInput(state, CurrentModuleObject, zoneNum, objectSchemaProps, objectFields, zeqRetNum);
            } // end loop over zone return mixers
        }
    }

    CurrentModuleObject = "AirLoopHVAC:SupplyPath";
    for (int PathNum = 1; PathNum <= state.dataZoneEquip->NumSupplyAirPaths; ++PathNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 PathNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); //  data for one zone
        state.dataZoneEquip->SupplyAirPath(PathNum).Name = AlphArray(1);
        state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents = nint((double(NumAlphas) - 2.0) / 2.0);

        state.dataZoneEquip->SupplyAirPath(PathNum).InletNodeNum = GetOnlySingleNode(state,
                                                                                     AlphArray(2),
                                                                                     state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::AirLoopHVACSupplyPath,
                                                                                     AlphArray(1),
                                                                                     DataLoopNode::NodeFluidType::Air,
                                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                                     ObjectIsParent);

        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum = DataZoneEquipment::AirLoopHVACZone::Invalid;
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).SplitterIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).PlenumIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);

        int Counter = 3;

        for (int CompNum = 1; CompNum <= state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents; ++CompNum) {

            if ((AlphArray(Counter) == "AIRLOOPHVAC:ZONESPLITTER") || (AlphArray(Counter) == "AIRLOOPHVAC:SUPPLYPLENUM")) {

                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType(CompNum) = AlphArray(Counter);
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName(CompNum) = AlphArray(Counter + 1);
                ValidateComponent(state,
                                  state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType(CompNum),
                                  state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName(CompNum),
                                  IsNotOK,
                                  CurrentModuleObject);
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).SplitterIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).PlenumIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum(CompNum) =
                    (AirLoopHVACZone)getEnumValue(AirLoopHVACTypeNamesUC, AlphArray(Counter));
            } else {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cAlphaFields(1), state.dataZoneEquip->SupplyAirPath(PathNum).Name));
                ShowContinueError(state, format("Unhandled component type =\"{}\".", AlphArray(Counter)));
                ShowContinueError(state, R"(Must be "AirLoopHVAC:ZoneSplitter" or "AirLoopHVAC:SupplyPlenum")");
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }

            Counter += 2;
        }

        state.dataZoneEquip->SupplyAirPath(PathNum).NumOutletNodes = 0;
        state.dataZoneEquip->SupplyAirPath(PathNum).NumNodes = 0;

    } // end loop over supply air paths

    CurrentModuleObject = "AirLoopHVAC:ReturnPath";
    for (int PathNum = 1; PathNum <= state.dataZoneEquip->NumReturnAirPaths; ++PathNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 PathNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); //  data for one zone
        state.dataZoneEquip->ReturnAirPath(PathNum).Name = AlphArray(1);
        state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents = nint((double(NumAlphas) - 2.0) / 2.0);

        state.dataZoneEquip->ReturnAirPath(PathNum).OutletNodeNum = GetOnlySingleNode(state,
                                                                                      AlphArray(2),
                                                                                      state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                                                      DataLoopNode::ConnectionObjectType::AirLoopHVACReturnPath,
                                                                                      AlphArray(1),
                                                                                      DataLoopNode::NodeFluidType::Air,
                                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                                      ObjectIsParent);

        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum = DataZoneEquipment::AirLoopHVACZone::Invalid;
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);

        int Counter = 3;

        for (int CompNum = 1; CompNum <= state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents; ++CompNum) {

            if ((AlphArray(Counter) == "AIRLOOPHVAC:ZONEMIXER") || (AlphArray(Counter) == "AIRLOOPHVAC:RETURNPLENUM")) {

                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum) = AlphArray(Counter);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum) = AlphArray(Counter + 1);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex(CompNum) = 0;
                ValidateComponent(state,
                                  state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum),
                                  state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum),
                                  IsNotOK,
                                  CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("In {} = {}", CurrentModuleObject, state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum(CompNum) =
                    static_cast<AirLoopHVACZone>(getEnumValue(AirLoopHVACTypeNamesUC, AlphArray(Counter)));
            } else {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cAlphaFields(1), state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                ShowContinueError(state, format("Unhandled component type =\"{}\".", AlphArray(Counter)));
                ShowContinueError(state, R"(Must be "AirLoopHVAC:ZoneMixer" or "AirLoopHVAC:ReturnPlenum")");
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }

            Counter += 2;
        }

    } // end loop over return air paths

    AlphArray.deallocate();
    NumArray.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
    lAlphaBlanks.deallocate();
    lNumericBlanks.deallocate();

    if (state.dataZoneEquip->GetZoneEquipmentDataErrorsFound) {
        ShowFatalError(state, format("{}Errors found in getting Zone Equipment input.", RoutineName));
    }
}

void processZoneEquipmentInput(EnergyPlusData &state,
                               std::string_view zoneEqModuleObject,
                               int const zoneOrSpaceNum,
                               bool const isSpace,
                               int &locTermUnitSizingCounter,
                               int &overallEquipCount,
                               DataZoneEquipment::EquipConfiguration &thisEquipConfig,
                               Array1D_string &AlphArray,
                               Array1D_string &cAlphaFields, // Alpha field names
                               Array1D_bool &lAlphaBlanks,   // Logical array, alpha field input BLANK = .TRUE.
                               Array1D_int &NodeNums)
{
    static constexpr std::string_view RoutineName("processZoneEquipmentInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "processZoneEquipmentInput";

    int spaceFieldShift = 0;
    if (isSpace) {
        spaceFieldShift = -1;
    }

    ErrorObjectHeader eoh{routineName, zoneEqModuleObject, AlphArray(1)};

    thisEquipConfig.IsControlled = true;
    thisEquipConfig.ZoneName = AlphArray(1); // for x-referencing with the geometry data

    bool IsNotOK = false;
    GlobalNames::IntraObjUniquenessCheck(
        state, AlphArray(2), zoneEqModuleObject, cAlphaFields(2), state.dataZoneEquip->UniqueZoneEquipListNames, IsNotOK);
    if (IsNotOK) {
        ShowContinueError(state, format("..another Controlled Zone has been assigned that {}.", cAlphaFields(2)));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }
    thisEquipConfig.EquipListName = AlphArray(2); // the name of the list containing all the zone eq.
    std::string const InletNodeListName = AlphArray(3 + spaceFieldShift);
    std::string const ExhaustNodeListName = AlphArray(4 + spaceFieldShift);
    thisEquipConfig.ZoneNode = GetOnlySingleNode(state,
                                                 AlphArray(5 + spaceFieldShift),
                                                 state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                 DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                                                 AlphArray(1),
                                                 DataLoopNode::NodeFluidType::Air,
                                                 DataLoopNode::ConnectionType::ZoneNode,
                                                 NodeInputManager::CompFluidStream::Primary,
                                                 DataLoopNode::ObjectIsNotParent); // all zone air state variables are
    if (thisEquipConfig.ZoneNode == 0) {
        ShowSevereError(state, format("{}{}: {}=\"{}\", invalid", RoutineName, zoneEqModuleObject, cAlphaFields(1), AlphArray(1)));
        ShowContinueError(state, format("{} must be present.", cAlphaFields(5 + spaceFieldShift)));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    } else {
        bool UniqueNodeError = false;
        NodeInputManager::CheckUniqueNodeNames(
            state, cAlphaFields(5 + spaceFieldShift), UniqueNodeError, AlphArray(5 + spaceFieldShift), AlphArray(1));
        if (UniqueNodeError) {
            // ShowContinueError(state, format("Occurs for {} = {}", trim( cAlphaFields( 1 ) ), trim( AlphArray( 1 ) )));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        }
    }

    std::string ReturnNodeListName = AlphArray(6 + spaceFieldShift);
    if (lAlphaBlanks(7)) {
        thisEquipConfig.returnFlowFracSched = Sched::GetScheduleAlwaysOn(state); // Not an availability sched, but defaults to constant-1.0
    } else if ((thisEquipConfig.returnFlowFracSched = Sched::GetSchedule(state, AlphArray(7 + spaceFieldShift))) == nullptr) {
        ShowSevereItemNotFound(state, eoh, cAlphaFields(7), AlphArray(7));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }
    std::string ReturnFlowBasisNodeListName = AlphArray(8 + spaceFieldShift);

    // Read in the equipment type, name and sequence information
    // for each equipment list

    if (!isSpace) {
        std::string CurrentModuleObject = "ZoneHVAC:EquipmentList";
        auto &ip = state.dataInputProcessing->inputProcessor;

        int ZoneEquipListNum = ip->getObjectItemNum(state, CurrentModuleObject, thisEquipConfig.EquipListName);

        if (ZoneEquipListNum <= 0) {
            ShowSevereError(state, format("{}{} not found = {}", RoutineName, CurrentModuleObject, thisEquipConfig.EquipListName));
            ShowContinueError(state, format("In ZoneHVAC:EquipmentConnections object, for Zone = {}", thisEquipConfig.ZoneName));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        } else {
            auto const &epListFields = ip->getJSONObjectItem(state, CurrentModuleObject, thisEquipConfig.EquipListName);

            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, CurrentModuleObject);

            EquipList &thisZoneEquipList = state.dataZoneEquip->ZoneEquipList(zoneOrSpaceNum);

            thisZoneEquipList.Name = thisEquipConfig.EquipListName;

            std::string loadDistName = ip->getAlphaFieldValue(epListFields, objectSchemaProps, "load_distribution_scheme");
            thisZoneEquipList.LoadDistScheme =
                static_cast<DataZoneEquipment::LoadDist>(getEnumValue(DataZoneEquipment::LoadDistNamesUC, Util::makeUPPER(loadDistName)));
            if (thisZoneEquipList.LoadDistScheme == DataZoneEquipment::LoadDist::Invalid) {
                ShowSevereError(state, format("{}{} = \"{}, Invalid choice\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                ShowContinueError(state, format("...load_distribution_scheme=\"{}\".", loadDistName));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }

            auto extensibles = epListFields.find("equipment");
            if (extensibles != epListFields.end()) {
                auto &extensiblesArray = extensibles.value();
                thisZoneEquipList.NumOfEquipTypes = static_cast<int>(extensiblesArray.size());

                // Increment overall count of equipment
                overallEquipCount += thisZoneEquipList.NumOfEquipTypes;

                thisZoneEquipList.EquipTypeName.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.EquipType.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.compPointer.resize(thisZoneEquipList.NumOfEquipTypes + 1);
                thisZoneEquipList.EquipName.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.EquipIndex.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.zoneEquipSplitterIndex.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.EquipData.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.CoolingPriority.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.HeatingPriority.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.CoolingCapacity.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.HeatingCapacity.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.sequentialCoolingFractionScheds.allocate(thisZoneEquipList.NumOfEquipTypes);
                thisZoneEquipList.sequentialHeatingFractionScheds.allocate(thisZoneEquipList.NumOfEquipTypes);
                for (int eqNum = 1; eqNum <= thisZoneEquipList.NumOfEquipTypes; ++eqNum) {
                    thisZoneEquipList.EquipTypeName(eqNum) = "";
                    thisZoneEquipList.EquipType(eqNum) = DataZoneEquipment::ZoneEquipType::Invalid;
                    thisZoneEquipList.EquipName(eqNum) = "";
                    thisZoneEquipList.EquipIndex(eqNum) = 0;
                    thisZoneEquipList.zoneEquipSplitterIndex(eqNum) = -1;
                    thisZoneEquipList.compPointer[eqNum] = nullptr;
                    thisZoneEquipList.CoolingPriority(eqNum) = 0;
                    thisZoneEquipList.HeatingPriority(eqNum) = 0;
                    thisZoneEquipList.CoolingCapacity(eqNum) = 0;
                    thisZoneEquipList.HeatingCapacity(eqNum) = 0;
                    thisZoneEquipList.sequentialCoolingFractionScheds(eqNum) = nullptr;
                    thisZoneEquipList.sequentialHeatingFractionScheds(eqNum) = nullptr;
                }

                auto const &extensionSchemaProps = objectSchemaProps["equipment"]["items"]["properties"];

                int ZoneEquipTypeNum = 0;
                for (auto &extensibleInstance : extensiblesArray) {
                    ++ZoneEquipTypeNum;

                    thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum) =
                        ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_object_type");
                    thisZoneEquipList.EquipName(ZoneEquipTypeNum) =
                        ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_name");

                    ValidateComponent(state,
                                      thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum),
                                      thisZoneEquipList.EquipName(ZoneEquipTypeNum),
                                      IsNotOK,
                                      CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("In {}={}", CurrentModuleObject, thisZoneEquipList.Name));
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }

                    // If not present, this return 0
                    thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) =
                        ip->getIntFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_cooling_sequence");

                    if ((thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) < 0) ||
                        (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > thisZoneEquipList.NumOfEquipTypes)) {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(
                            state, format("invalid zone_equipment_cooling_sequence=[{}].", thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum)));
                        ShowContinueError(state, "equipment sequence must be > 0 and <= number of equipments in the list.");
                        if (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > 0) {
                            ShowContinueError(state, format("only {} in the list.", thisZoneEquipList.NumOfEquipTypes));
                        }
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }

                    thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) =
                        ip->getIntFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_heating_or_no_load_sequence");
                    if ((thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) < 0) ||
                        (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > thisZoneEquipList.NumOfEquipTypes)) {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(
                            state, format("invalid zone_equipment_heating_sequence=[{}].", thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum)));
                        ShowContinueError(state, "equipment sequence must be > 0 and <= number of equipments in the list.");
                        if (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > 0) {
                            ShowContinueError(state, format("only {} in the list.", thisZoneEquipList.NumOfEquipTypes));
                        }
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }

                    std::string coolingSchName =
                        ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_sequential_cooling_fraction_schedule_name");
                    if (coolingSchName.empty()) {
                        thisZoneEquipList.sequentialCoolingFractionScheds(ZoneEquipTypeNum) =
                            Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
                    } else {
                        thisZoneEquipList.sequentialCoolingFractionScheds(ZoneEquipTypeNum) = Sched::GetSchedule(state, coolingSchName);
                        if (thisZoneEquipList.sequentialCoolingFractionScheds(ZoneEquipTypeNum) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, "zone_equipment_sequential_cooling_fraction_schedule_name", coolingSchName);
                            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                        }
                    }

                    std::string heatingSchName =
                        ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_sequential_heating_fraction_schedule_name");
                    if (heatingSchName.empty()) {
                        thisZoneEquipList.sequentialHeatingFractionScheds(ZoneEquipTypeNum) =
                            Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
                    } else {
                        thisZoneEquipList.sequentialHeatingFractionScheds(ZoneEquipTypeNum) = Sched::GetSchedule(state, heatingSchName);
                        if (thisZoneEquipList.sequentialHeatingFractionScheds(ZoneEquipTypeNum) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, "zone_equipment_sequential_heating_fraction_schedule_name", coolingSchName);
                            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                        }
                    }

                    // do this here for initial prototype, but later will call all the equipment in a separate function to see who is on - maybe
                    if (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > 0) {
                        ++thisZoneEquipList.NumAvailHeatEquip;
                    }
                    if (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > 0) {
                        ++thisZoneEquipList.NumAvailCoolEquip;
                    }

                    thisZoneEquipList.EquipType(ZoneEquipTypeNum) = static_cast<ZoneEquipType>(
                        getEnumValue(zoneEquipTypeNamesUC, Util::makeUPPER(thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum))));

                    if (thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::UnitarySystem ||
                        thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalAirConditioner ||
                        thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalHeatPump ||
                        thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalHeatPumpWaterToAir) {
                        // loop index accesses correct pointer to equipment on this equipment list
                        // EquipIndex is used to access specific equipment for a single class of equipment (e.g., PTAC 1, 2 and 3)
                        thisZoneEquipList.compPointer[ZoneEquipTypeNum] = UnitarySystems::UnitarySys::factory(
                            state, HVAC::UnitarySysType::Unitary_AnyCoilType, thisZoneEquipList.EquipName(ZoneEquipTypeNum), true, 0);
                        thisZoneEquipList.EquipIndex(ZoneEquipTypeNum) = thisZoneEquipList.compPointer[ZoneEquipTypeNum]->getEquipIndex();
                    }

                    if (thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::Invalid) {
                        ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(state, format("..Invalid Equipment Type = {}", thisZoneEquipList.EquipType(ZoneEquipTypeNum)));
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }
                }
            } // End parsing all extensible Zone Equipment info

            // Check for multiple assignments
            for (int ZoneEquipTypeNum = 1; ZoneEquipTypeNum <= thisZoneEquipList.NumOfEquipTypes; ++ZoneEquipTypeNum) {
                if (count_eq(thisZoneEquipList.CoolingPriority, ZoneEquipTypeNum) > 1) {
                    ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("...multiple assignments for Zone Equipment Cooling Sequence={}, must be 1-1 correspondence between "
                                             "sequence assignments and number of equipments.",
                                             ZoneEquipTypeNum));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                } else if (count_eq(thisZoneEquipList.CoolingPriority, ZoneEquipTypeNum) == 0) {
                    ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("...zero assigned to Zone Equipment Cooling Sequence={}, apparent gap in sequence assignments in "
                                             "this equipment list.",
                                             ZoneEquipTypeNum));
                }
                if (count_eq(thisZoneEquipList.HeatingPriority, ZoneEquipTypeNum) > 1) {
                    ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("...multiple assignments for Zone Equipment Heating or No-Load Sequence={}, must be 1-1 "
                                             "correspondence between sequence assignments and number of equipments.",
                                             ZoneEquipTypeNum));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                } else if (count_eq(thisZoneEquipList.HeatingPriority, ZoneEquipTypeNum) == 0) {
                    ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("...zero assigned to Zone Equipment Heating or No-Load Sequence={}, apparent gap in sequence "
                                             "assignments in this equipment list.",
                                             ZoneEquipTypeNum));
                }
            }
        } // End ZoneHVAC:EquipmentList
    }
    bool NodeListError = false;
    int NumNodes = 0;
    GetNodeNums(state,
                InletNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                (isSpace ? DataLoopNode::ConnectionObjectType::SpaceHVACEquipmentConnections
                         : DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections),
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneInlet,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumInletNodes = NumNodes;

        thisEquipConfig.InletNode.allocate(NumNodes);
        thisEquipConfig.InletNodeAirLoopNum.allocate(NumNodes);
        thisEquipConfig.InletNodeADUNum.allocate(NumNodes);
        thisEquipConfig.AirDistUnitCool.allocate(NumNodes);
        thisEquipConfig.AirDistUnitHeat.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.InletNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Air Inlet Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
            thisEquipConfig.InletNodeAirLoopNum(NodeNum) = 0;
            thisEquipConfig.InletNodeADUNum(NodeNum) = 0;
            thisEquipConfig.AirDistUnitCool(NodeNum).InNode = 0;
            thisEquipConfig.AirDistUnitHeat(NodeNum).InNode = 0;
            thisEquipConfig.AirDistUnitCool(NodeNum).OutNode = 0;
            thisEquipConfig.AirDistUnitHeat(NodeNum).OutNode = 0;
            if (!isSpace) {
                ++locTermUnitSizingCounter;
                thisEquipConfig.AirDistUnitCool(NodeNum).TermUnitSizingIndex = locTermUnitSizingCounter;
                thisEquipConfig.AirDistUnitHeat(NodeNum).TermUnitSizingIndex = locTermUnitSizingCounter;
            }
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Air Inlet Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone = {}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ExhaustNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                (isSpace ? DataLoopNode::ConnectionObjectType::SpaceHVACEquipmentConnections
                         : DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections),
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneExhaust,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumExhaustNodes = NumNodes;

        thisEquipConfig.ExhaustNode.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ExhaustNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Air Exhaust Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                // ShowContinueError(state, format("Occurs for Zone = {}", trim( AlphArray( 1 ) )));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Air Exhaust Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ReturnNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                (isSpace ? DataLoopNode::ConnectionObjectType::SpaceHVACEquipmentConnections
                         : DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections),
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneReturn,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumReturnNodes = NumNodes;

        thisEquipConfig.ReturnNode.allocate(NumNodes);
        thisEquipConfig.returnNodeSpaceMixerIndex.allocate(NumNodes);
        for (int &mixIndex : thisEquipConfig.returnNodeSpaceMixerIndex) {
            mixIndex = -1;
        }
        thisEquipConfig.ReturnNodeAirLoopNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeRetPathNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeRetPathCompNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeInletNum.allocate(NumNodes);
        thisEquipConfig.FixedReturnFlow.allocate(NumNodes);
        thisEquipConfig.ReturnNodePlenumNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeExhaustNodeNum.allocate(NumNodes);
        thisEquipConfig.SharedExhaustNode.allocate(NumNodes);
        thisEquipConfig.ReturnNode = 0;           // initialize to zero here
        thisEquipConfig.ReturnNodeAirLoopNum = 0; // initialize to zero here
        thisEquipConfig.ReturnNodeInletNum = 0;   // initialize to zero here
        thisEquipConfig.ReturnNodeRetPathNum = 0;
        thisEquipConfig.ReturnNodeRetPathCompNum = 0;
        thisEquipConfig.FixedReturnFlow = false;                                // initialize to false here
        thisEquipConfig.ReturnNodePlenumNum = 0;                                // initialize to zero here
        thisEquipConfig.ReturnNodeExhaustNodeNum = 0;                           // initialize to zero here
        thisEquipConfig.SharedExhaustNode = LightReturnExhaustConfig::NoExhast; // initialize to zero here

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ReturnNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Return Air Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                // ShowContinueError(state, format("Occurs for Zone = {}", trim( AlphArray( 1 ) )));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Return Air Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ReturnFlowBasisNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                (isSpace ? DataLoopNode::ConnectionObjectType::SpaceHVACEquipmentConnections
                         : DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections),
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::Sensor,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumReturnFlowBasisNodes = NumNodes;

        thisEquipConfig.ReturnFlowBasisNode.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ReturnFlowBasisNode(NodeNum) = NodeNums(NodeNum);
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Return Air Node 1 Flow Rate Basis Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}",
                   thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }
}

void processZoneEquipSplitterInput(EnergyPlusData &state,
                                   std::string_view zeqSplitterModuleObject,
                                   int const zeqSplitterNum,
                                   int const zoneNum,
                                   InputProcessor::json const objectSchemaProps,
                                   InputProcessor::json const objectFields,
                                   DataZoneEquipment::ZoneEquipmentSplitter &thisZeqSplitter)

{
    static constexpr std::string_view RoutineName("processZoneEquipSplitterInput: "); // include trailing blank space
    auto &ip = state.dataInputProcessing->inputProcessor;
    std::string const zeqTypeName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_equipment_object_type");
    thisZeqSplitter.zoneEquipType = DataZoneEquipment::ZoneEquipType(getEnumValue(zoneEquipTypeNamesUC, zeqTypeName));
    if (thisZeqSplitter.zoneEquipType == ZoneEquipType::Invalid) {
        ShowSevereError(state, format("{}{} = {}", RoutineName, zeqSplitterModuleObject, thisZeqSplitter.Name));
        ShowContinueError(state, format("..Invalid Equipment Type = {}", zeqTypeName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    thisZeqSplitter.zoneEquipName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_equipment_name");

    // Search zone equipment list for matching equip type and name
    bool found = false;
    auto &thisZoneEqList = state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex);
    for (int eqCount = 1; eqCount <= thisZoneEqList.NumOfEquipTypes; ++eqCount) {
        if (thisZeqSplitter.zoneEquipType == thisZoneEqList.EquipType(eqCount)) {
            if (thisZeqSplitter.zoneEquipName == thisZoneEqList.EquipName(eqCount)) {
                found = true;
                // Set index in zone equipment list pointing to this SpaceHVAC:ZoneEquipmentSplitter
                thisZoneEqList.zoneEquipSplitterIndex = zeqSplitterNum;
                // SpaceHVAC TODO: Outletnodes aren't know yet - need to set this in a later init
                // thisZeqSplitter.zoneEquipOutletNodeNum = thisZoneEqList.EquipData(eqCount).OutletNodeNums(1);
                break;
            }
        }
    }
    if (!found) {
        ShowSevereError(state, format("{}{} = {}", RoutineName, zeqSplitterModuleObject, thisZeqSplitter.Name));
        ShowContinueError(
            state, format(".. Zone Equipment Object Type={} and Zone Equipment Name={} not found", zeqTypeName, thisZeqSplitter.zoneEquipName));
        ShowContinueError(state, format(".. in ZoneHVAC:EquipmentList={}", thisZoneEqList.Name));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        return;
    }

    bool objectIsParent = true;
    thisZeqSplitter.zoneEquipOutletNodeNum =
        GetOnlySingleNode(state,
                          ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_equipment_outlet_node_name"),
                          state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                          thisZeqSplitter.spaceEquipType,
                          thisZeqSplitter.Name,
                          DataLoopNode::NodeFluidType::Air,
                          DataLoopNode::ConnectionType::Inlet,
                          NodeInputManager::CompFluidStream::Primary,
                          objectIsParent);

    thisZeqSplitter.tstatControl = DataZoneEquipment::ZoneEquipTstatControl(
        getEnumValue(zoneEquipTstatControlNamesUC, ip->getAlphaFieldValue(objectFields, objectSchemaProps, "thermostat_control_method")));
    if (thisZeqSplitter.tstatControl == DataZoneEquipment::ZoneEquipTstatControl::SingleSpace) {
        std::string spaceName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "control_space_name");
        thisZeqSplitter.controlSpaceIndex = Util::FindItemInList(spaceName, state.dataHeatBal->space);
        if (thisZeqSplitter.controlSpaceIndex == 0) {
            ShowSevereError(state, format("{}{}={}", RoutineName, zeqSplitterModuleObject, thisZeqSplitter.Name));
            ShowContinueError(state, format("Space Name={} not found.", spaceName));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        }
    }
    thisZeqSplitter.spaceSizingBasis = DataZoneEquipment::SpaceEquipSizingBasis(
        getEnumValue(spaceEquipSizingBasisNamesUC, ip->getAlphaFieldValue(objectFields, objectSchemaProps, "space_fraction_method")));

    auto extensibles = objectFields.find("spaces");
    auto const &extensionSchemaProps = objectSchemaProps["spaces"]["items"]["properties"];
    if (extensibles != objectFields.end()) {
        auto &extensiblesArray = extensibles.value();
        int const numSpaces = extensiblesArray.size();
        thisZeqSplitter.spaces.resize(numSpaces);
        int spaceCount = -1;
        for (auto &extensibleInstance : extensiblesArray) {
            ++spaceCount;
            auto &thisZeqSpace = thisZeqSplitter.spaces[spaceCount];
            std::string const spaceName = ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_name");
            thisZeqSpace.spaceIndex = Util::FindItemInList(spaceName, state.dataHeatBal->space);
            if (thisZeqSpace.spaceIndex == 0) {
                ShowSevereError(state, format("{}{}={}", RoutineName, zeqSplitterModuleObject, thisZeqSplitter.Name));
                ShowContinueError(state, format("Space Name={} not found.", spaceName));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            } else {
                thisZeqSpace.fraction = ip->getRealFieldValue(extensibleInstance, extensionSchemaProps, "space_fraction");
                thisZeqSpace.spaceNodeNum =
                    GetOnlySingleNode(state,
                                      ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_supply_node_name"),
                                      state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                      thisZeqSplitter.spaceEquipType,
                                      thisZeqSplitter.Name,
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Outlet,
                                      NodeInputManager::CompFluidStream::Primary,
                                      objectIsParent);
                if (thisZeqSplitter.controlSpaceIndex == thisZeqSpace.spaceIndex) {
                    thisZeqSplitter.controlSpaceNumber = spaceCount;
                }
            }
        }
    }
}

void processZoneEquipMixerInput(EnergyPlusData &state,
                                std::string_view zeqMixerModuleObject,
                                int const zoneNum,
                                InputProcessor::json const objectSchemaProps,
                                InputProcessor::json const objectFields,
                                DataZoneEquipment::ZoneEquipmentMixer &thisZeqMixer)

{
    static constexpr std::string_view RoutineName("processZoneEquipMixerInput: "); // include trailing blank space
    auto &ip = state.dataInputProcessing->inputProcessor;
    bool objectIsParent = true;
    thisZeqMixer.outletNodeNum = GetOnlySingleNode(state,
                                                   ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_equipment_inlet_node_name"),
                                                   state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                   thisZeqMixer.spaceEquipType,
                                                   thisZeqMixer.Name,
                                                   DataLoopNode::NodeFluidType::Air,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   objectIsParent);
    // Check zone exhaust nodes
    bool found = false;
    auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
    for (int exhNodeNum : thisZoneEquipConfig.ExhaustNode) {
        if (thisZeqMixer.outletNodeNum == exhNodeNum) {
            found = true;
            break;
        }
    }
    if (!found) {
        ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZeqMixer.Name));
        ShowContinueError(state,
                          format("Zone Equipment Inlet Node Name={} is not an exhaust node for ZoneHVAC:EquipmentConnections={}.",
                                 state.dataLoopNodes->NodeID(thisZeqMixer.outletNodeNum),
                                 thisZoneEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    thisZeqMixer.spaceSizingBasis = DataZoneEquipment::SpaceEquipSizingBasis(
        getEnumValue(spaceEquipSizingBasisNamesUC, ip->getAlphaFieldValue(objectFields, objectSchemaProps, "space_fraction_method")));

    auto extensibles = objectFields.find("spaces");
    auto const &extensionSchemaProps = objectSchemaProps["spaces"]["items"]["properties"];
    if (extensibles != objectFields.end()) {
        auto &extensiblesArray = extensibles.value();
        int const numSpaces = extensiblesArray.size();
        thisZeqMixer.spaces.resize(numSpaces);
        int spaceCount = -1;
        for (auto &extensibleInstance : extensiblesArray) {
            ++spaceCount;
            auto &thisZeqSpace = thisZeqMixer.spaces[spaceCount];
            std::string const spaceName = ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_name");
            thisZeqSpace.spaceIndex = Util::FindItemInList(spaceName, state.dataHeatBal->space);
            if (thisZeqSpace.spaceIndex == 0) {
                ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZeqMixer.Name));
                ShowContinueError(state, format("Space Name={} not found.", spaceName));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            } else {
                thisZeqSpace.fraction = ip->getRealFieldValue(extensibleInstance, extensionSchemaProps, "space_fraction");
                thisZeqSpace.spaceNodeNum = GetOnlySingleNode(state,
                                                              ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_node_name"),
                                                              state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                              thisZeqMixer.spaceEquipType,
                                                              thisZeqMixer.Name,
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Inlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              objectIsParent);
                // Check space exhaust nodes
                found = false;
                auto &thisSpaceEquipConfig = state.dataZoneEquip->spaceEquipConfig(thisZeqSpace.spaceIndex);
                for (int exhNodeNum : thisSpaceEquipConfig.ExhaustNode) {
                    if (thisZeqSpace.spaceNodeNum == exhNodeNum) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZeqMixer.Name));
                    ShowContinueError(state,
                                      format("Space Node Name={} is not an exhaust node for SpaceHVAC:EquipmentConnections={}.",
                                             state.dataLoopNodes->NodeID(thisZeqSpace.spaceNodeNum),
                                             thisSpaceEquipConfig.ZoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }
            }
        }
    }
}

void processZoneReturnMixerInput(EnergyPlusData &state,
                                 std::string_view zeqMixerModuleObject,
                                 int const zoneNum,
                                 InputProcessor::json const objectSchemaProps,
                                 InputProcessor::json const objectFields,
                                 int mixerIndex)

{
    static constexpr std::string_view RoutineName("processZoneReturnMixerInput: "); // include trailing blank space
    auto &ip = state.dataInputProcessing->inputProcessor;
    bool objectIsParent = true;
    auto &thisZretMixer = state.dataZoneEquip->zoneReturnMixer[mixerIndex];
    thisZretMixer.outletNodeNum = GetOnlySingleNode(state,
                                                    ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_return_air_node_name"),
                                                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                    thisZretMixer.spaceEquipType,
                                                    thisZretMixer.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    objectIsParent);
    // Check zone return nodes
    bool found = false;
    auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
    int nodeCounter = 0;
    for (int retNodeNum : thisZoneEquipConfig.ReturnNode) {
        ++nodeCounter;
        if (thisZretMixer.outletNodeNum == retNodeNum) {
            found = true;
            // Zone return node is fed by a space return mixer
            thisZoneEquipConfig.returnNodeSpaceMixerIndex(nodeCounter) = mixerIndex;
            break;
        }
    }
    if (!found) {
        ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZretMixer.Name));
        ShowContinueError(state,
                          format("Zone Equipment Return Air Node Name={} is not a return air node for ZoneHVAC:EquipmentConnections={}.",
                                 state.dataLoopNodes->NodeID(thisZretMixer.outletNodeNum),
                                 thisZoneEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    auto extensibles = objectFields.find("spaces");
    auto const &extensionSchemaProps = objectSchemaProps["spaces"]["items"]["properties"];
    if (extensibles != objectFields.end()) {
        auto &extensiblesArray = extensibles.value();
        int const numSpaces = extensiblesArray.size();
        thisZretMixer.spaces.resize(numSpaces);
        int spaceCount = -1;
        for (auto &extensibleInstance : extensiblesArray) {
            ++spaceCount;
            auto &thisZeqSpace = thisZretMixer.spaces[spaceCount];
            std::string const spaceName = ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_name");
            thisZeqSpace.spaceIndex = Util::FindItemInList(spaceName, state.dataHeatBal->space);
            if (thisZeqSpace.spaceIndex == 0) {
                ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZretMixer.Name));
                ShowContinueError(state, format("Space Name={} not found.", spaceName));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            } else {
                thisZeqSpace.spaceNodeNum =
                    GetOnlySingleNode(state,
                                      ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_return_air_node_name"),
                                      state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                      thisZretMixer.spaceEquipType,
                                      thisZretMixer.Name,
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Inlet,
                                      NodeInputManager::CompFluidStream::Primary,
                                      objectIsParent);
                // Check space return nodes
                found = false;
                auto &thisSpaceEquipConfig = state.dataZoneEquip->spaceEquipConfig(thisZeqSpace.spaceIndex);
                for (int retNodeNum : thisSpaceEquipConfig.ReturnNode) {
                    if (thisZeqSpace.spaceNodeNum == retNodeNum) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(state, format("{}{}={}", RoutineName, zeqMixerModuleObject, thisZretMixer.Name));
                    ShowContinueError(state,
                                      format("Space Return Air Node Name={} is not a return air node for SpaceHVAC:EquipmentConnections={}.",
                                             state.dataLoopNodes->NodeID(thisZeqSpace.spaceNodeNum),
                                             thisSpaceEquipConfig.ZoneName));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }
            }
        }
    }
}

bool CheckZoneEquipmentList(EnergyPlusData &state,
                            std::string_view const ComponentType, // Type of component
                            std::string_view const ComponentName, // Name of component
                            ObjexxFCL::Optional_int CtrlZoneNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Provides a way to check if a component name is listed on a zone equipment list.

    // Return value
    bool IsOnList; // True if item is on a list, false if not.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int Loop;
    int ListLoop;
    int CtrlZoneNumLocal;

    CtrlZoneNumLocal = 0;
    IsOnList = false;
    for (Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) { // NumOfZoneEquipLists
        if (state.dataZoneEquip->ZoneEquipList(Loop).Name.empty()) {
            continue; // dimensioned by NumOfZones.  Only valid ones have names.
        }
        for (ListLoop = 1; ListLoop <= state.dataZoneEquip->ZoneEquipList(Loop).NumOfEquipTypes; ++ListLoop) {

            if (!Util::SameString(state.dataZoneEquip->ZoneEquipList(Loop).EquipTypeName(ListLoop), ComponentType)) {
                continue;
            }
            if (ComponentName == "*") {
                IsOnList = true;
                CtrlZoneNumLocal = Loop;
                goto EquipList_exit;
            }
            if (!Util::SameString(state.dataZoneEquip->ZoneEquipList(Loop).EquipName(ListLoop), ComponentName)) {
                continue;
            }
            IsOnList = true;
            CtrlZoneNumLocal = Loop;
            goto EquipList_exit;
        }
    }
EquipList_exit:;
    if (present(CtrlZoneNum)) {
        CtrlZoneNum = CtrlZoneNumLocal;
    }
    return IsOnList;
}

int GetControlledZoneIndex(EnergyPlusData &state, std::string const &ZoneName) // Zone name to match into Controlled Zone structure
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008

    // PURPOSE OF THIS FUNCTION:
    // This function returns the index into the Controlled Zone Equipment structure
    // of the indicated zone.

    return Util::FindItemInList(ZoneName, state.dataZoneEquip->ZoneEquipConfig, &EquipConfiguration::ZoneName);
}

int FindControlledZoneIndexFromSystemNodeNumberForZone(EnergyPlusData &state,
                                                       int const TrialZoneNodeNum) // Node number to match into Controlled Zone structure
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2013

    // PURPOSE OF THIS FUNCTION:
    // This function returns the zone number for the indicated
    // zone node num.  Returns 0 if did not find zone node in any Zone

    int ControlledZoneIndex = 0; // Index into Controlled Zone structure

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).IsControlled) {
            if (TrialZoneNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode) {
                // found it.
                ControlledZoneIndex = ZoneNum;
                break;
            }
        }
    }

    return ControlledZoneIndex;
}

int GetSystemNodeNumberForZone(EnergyPlusData &state, int const zoneNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008

    // PURPOSE OF THIS FUNCTION:
    // This function returns the system node number for the indicated
    // zone.  Returns 0 if the Zone is not a controlled zone.

    int SystemZoneNodeNumber = 0; // System node number for controlled zone

    if (zoneNum > 0) {
        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            SystemZoneNodeNumber = state.dataZoneEquip->ZoneEquipConfig(zoneNum).ZoneNode;
        }
    }

    return SystemZoneNodeNumber;
}

int GetReturnAirNodeForZone(EnergyPlusData &state,
                            int const zoneNum,
                            std::string const &NodeName,             // Return air node name to match (may be blank)
                            std::string const &calledFromDescription // String identifying the calling function and object
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008
    //       MODIFIED       Feb 2017 expanded for multiple return nodes in a zone

    // PURPOSE OF THIS FUNCTION:
    // This function returns the return air node number for the indicated
    // zone and node name.  If NodeName is blank, return the first return node number,
    // otherwise return the node number of the matching return node name.
    // Returns 0 if the Zone is not a controlled zone or the node name does not match.

    // Return value
    int ReturnAirNodeNumber = 0; // Return Air node number for controlled zone

    ReturnAirNodeNumber = 0; // default is not found
    if (zoneNum > 0) {
        {
            auto const &thisZoneEquip(state.dataZoneEquip->ZoneEquipConfig(zoneNum));
            if (thisZoneEquip.IsControlled) {
                if (NodeName.empty()) {
                    // If NodeName is blank, return first return node number, but warn if there are multiple return nodes for this zone
                    ReturnAirNodeNumber = thisZoneEquip.ReturnNode(1);
                    if (thisZoneEquip.NumReturnNodes > 1) {
                        ShowWarningError(state,
                                         format("GetReturnAirNodeForZone: {}, request for zone return node is ambiguous.", calledFromDescription));
                        ShowContinueError(state,
                                          format("Zone={} has {} return nodes. First return node will be used.",
                                                 thisZoneEquip.ZoneName,
                                                 thisZoneEquip.NumReturnNodes));
                    }
                } else {
                    for (int nodeCount = 1; nodeCount <= thisZoneEquip.NumReturnNodes; ++nodeCount) {
                        int curNodeNum = thisZoneEquip.ReturnNode(nodeCount);
                        if (NodeName == state.dataLoopNodes->NodeID(curNodeNum)) {
                            ReturnAirNodeNumber = curNodeNum;
                        }
                    }
                }
            }
        }
    }

    return ReturnAirNodeNumber;
}

int GetReturnNumForZone(EnergyPlusData &state,
                        int const zoneNum,
                        std::string const &NodeName // Return air node name to match (may be blank)
)
{

    // PURPOSE OF THIS FUNCTION:
    // This function returns the zone return number (not the node number) for the indicated
    // zone and node name.  If NodeName is blank, return 1 (the first return node)
    // otherwise return the index of the matching return node name.
    // Returns 0 if the Zone is not a controlled zone or the node name does not match.

    // Return value
    int ReturnIndex = 0; // Return number for the given zone (not the node number)

    if (zoneNum > 0) {
        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            if (NodeName.empty()) {
                // If NodeName is blank, return first return node number
                ReturnIndex = 1;
            } else {
                for (int nodeCount = 1; nodeCount <= state.dataZoneEquip->ZoneEquipConfig(zoneNum).NumReturnNodes; ++nodeCount) {
                    int curNodeNum = state.dataZoneEquip->ZoneEquipConfig(zoneNum).ReturnNode(nodeCount);
                    if (NodeName == state.dataLoopNodes->NodeID(curNodeNum)) {
                        ReturnIndex = nodeCount;
                    }
                }
            }
        }
    }

    return ReturnIndex;
}

bool VerifyLightsExhaustNodeForZone(EnergyPlusData &state, int const ZoneNum, int const ZoneExhaustNodeNum)
{
    bool exhaustNodeError = true;

    for (int ExhaustNum = 1; ExhaustNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumExhaustNodes; ++ExhaustNum) {
        if (ZoneExhaustNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ExhaustNode(ExhaustNum)) {
            exhaustNodeError = false;
            break;
        }
    }

    return exhaustNodeError;
}

void EquipList::getPrioritiesForInletNode(EnergyPlusData &state,
                                          int const inletNodeNum, // Zone inlet node number to match
                                          int &coolingPriority,   // Cooling priority num for matching equipment
                                          int &heatingPriority    // Heating priority num for matching equipment
)
{
    bool equipFound = false;
    for (int equipNum = 1; equipNum <= this->NumOfEquipTypes; ++equipNum) {
        if (this->EquipType(equipNum) == DataZoneEquipment::ZoneEquipType::AirDistributionUnit) {
            if (inletNodeNum == state.dataDefineEquipment->AirDistUnit(this->EquipIndex(equipNum)).OutletNodeNum) {
                equipFound = true;
            }
        }
        if (equipFound) {
            coolingPriority = this->CoolingPriority(equipNum);
            heatingPriority = this->HeatingPriority(equipNum);
            break;
        }
    }
    // Set MinAirLoopIterationsAfterFirst for equipment that uses sequenced loads, based on zone equip load distribution scheme
    int minIterations = state.dataHVACGlobal->MinAirLoopIterationsAfterFirst;
    if (this->LoadDistScheme == DataZoneEquipment::LoadDist::Sequential) {
        // Sequential needs one extra iterations up to the highest airterminal unit equipment number
        minIterations = max(coolingPriority, heatingPriority, minIterations);
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::Uniform) {
        // Uniform needs one extra iteration which is the default
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::UniformPLR) {
        // UniformPLR needs two extra iterations, regardless of unit equipment number
        minIterations = max(2, minIterations);
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::SequentialUniformPLR) {
        // SequentialUniformPLR needs one extra iterations up to the highest airterminal unit equipment number plus one more
        minIterations = max((coolingPriority + 1), (heatingPriority + 1), minIterations);
    }
    state.dataHVACGlobal->MinAirLoopIterationsAfterFirst = minIterations;
}

Real64 EquipList::SequentialHeatingFraction([[maybe_unused]] EnergyPlusData &state, const int equipNum)
{
    return sequentialHeatingFractionScheds(equipNum)->getCurrentVal();
}

Real64 EquipList::SequentialCoolingFraction([[maybe_unused]] EnergyPlusData &state, const int equipNum)
{
    return sequentialCoolingFractionScheds(equipNum)->getCurrentVal();
}

int GetZoneEquipControlledZoneNum(EnergyPlusData &state, DataZoneEquipment::ZoneEquipType const zoneEquipType, std::string const &EquipmentName)
{
    static constexpr std::string_view RoutineName("GetZoneEquipControlledZoneNum: ");
    int ControlZoneNum = 0;

    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
            continue;
        }
        for (int Num = 1; Num <= state.dataZoneEquip->ZoneEquipList(CtrlZone).NumOfEquipTypes; ++Num) {
            if (zoneEquipType == state.dataZoneEquip->ZoneEquipList(CtrlZone).EquipType(Num) &&
                Util::SameString(EquipmentName, state.dataZoneEquip->ZoneEquipList(CtrlZone).EquipName(Num))) {
                return ControlZoneNum = CtrlZone;
            }
        }
    }
    ShowSevereError(state,
                    fmt::format("{}{}=\"{}\" is not on any ZoneHVAC:Equipmentlist. It will not be simulated.",
                                RoutineName,
                                zoneEquipTypeNamesUC[(int)zoneEquipType],
                                EquipmentName));
    return ControlZoneNum;
}

void CheckSharedExhaust(EnergyPlusData &state)
{
    int ExhastNodeNum = 0;
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes < 2) {
            continue;
        }
        for (int nodeCount = 1; nodeCount <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++nodeCount) {
            if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) == LightReturnExhaustConfig::Shared) {
                continue;
            }
            ExhastNodeNum = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeExhaustNodeNum(nodeCount);
            if (ExhastNodeNum > 0) {
                state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) = LightReturnExhaustConfig::Single;
                for (int nodeCount1 = nodeCount + 1; nodeCount1 <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++nodeCount1) {
                    if (ExhastNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeExhaustNodeNum(nodeCount1)) {
                        state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) = LightReturnExhaustConfig::Multi;
                        state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount1) = LightReturnExhaustConfig::Shared;
                    }
                }
            }
        }
    }
}

void EquipConfiguration::setTotalInletFlows(EnergyPlusData &state)
{
    this->TotInletAirMassFlowRate = 0.0;
    Real64 TotInletAirMassFlowRateMax = 0.0;
    Real64 TotInletAirMassFlowRateMaxAvail = 0.0;
    Real64 TotInletAirMassFlowRateMin = 0.0;
    Real64 TotInletAirMassFlowRateMinAvail = 0.0;
    for (int NodeNum = 1; NodeNum <= this->NumInletNodes; ++NodeNum) {
        {
            auto const &thisNode(state.dataLoopNodes->Node(this->InletNode(NodeNum)));
            this->TotInletAirMassFlowRate += thisNode.MassFlowRate;
            TotInletAirMassFlowRateMax += thisNode.MassFlowRateMax;
            TotInletAirMassFlowRateMaxAvail += thisNode.MassFlowRateMaxAvail;
            TotInletAirMassFlowRateMin += thisNode.MassFlowRateMin;
            TotInletAirMassFlowRateMinAvail += thisNode.MassFlowRateMinAvail;
        }
    }
    auto &zoneSpaceNode = state.dataLoopNodes->Node(this->ZoneNode);
    zoneSpaceNode.MassFlowRate = this->TotInletAirMassFlowRate;
    zoneSpaceNode.MassFlowRateMax = TotInletAirMassFlowRateMax;
    zoneSpaceNode.MassFlowRateMaxAvail = TotInletAirMassFlowRateMaxAvail;
    zoneSpaceNode.MassFlowRateMin = TotInletAirMassFlowRateMin;
    zoneSpaceNode.MassFlowRateMinAvail = TotInletAirMassFlowRateMinAvail;
}

void scaleInletFlows(EnergyPlusData &state, int const zoneNodeNum, int const spaceNodeNum, Real64 const frac)
{
    assert(zoneNodeNum > 0);
    assert(spaceNodeNum > 0);
    auto const &zoneNode = state.dataLoopNodes->Node(zoneNodeNum);
    auto &spaceNode = state.dataLoopNodes->Node(spaceNodeNum);
    spaceNode.MassFlowRate = zoneNode.MassFlowRate * frac;
    spaceNode.MassFlowRateMax = zoneNode.MassFlowRateMax * frac;
    spaceNode.MassFlowRateMaxAvail = zoneNode.MassFlowRateMaxAvail * frac;
    spaceNode.MassFlowRateMin = zoneNode.MassFlowRateMin * frac;
    spaceNode.MassFlowRateMinAvail = zoneNode.MassFlowRateMinAvail * frac;
}

void ZoneEquipmentSplitterMixer::size(EnergyPlusData &state)
{
    bool anyAutoSize =
        std::any_of(spaces.begin(), spaces.end(), [](ZoneEquipSplitterMixerSpace const &s) { return s.fraction == DataSizing::AutoSize; });
    if (!anyAutoSize) {
        return;
    }

    if (!state.dataHeatBal->doSpaceHeatBalanceSizing && (this->spaceSizingBasis == DataZoneEquipment::SpaceEquipSizingBasis::DesignCoolingLoad ||
                                                         (this->spaceSizingBasis == DataZoneEquipment::SpaceEquipSizingBasis::DesignHeatingLoad))) {
        ShowSevereError(state,
                        format("ZoneEquipmentSplitterMixer::size: {} is unknown for {}={}. Unable to autosize Space Fractions.",
                               DataZoneEquipment::spaceEquipSizingBasisNamesUC[(int)this->spaceSizingBasis],
                               BranchNodeConnections::ConnectionObjectTypeNames[(int)this->spaceEquipType],
                               this->Name));
        ShowFatalError(state,
                       "Set \"Do Space Heat Balance for Sizing\" to Yes in ZoneAirHeatBalanceAlgorithm or choose a different Space Fraction Method.");
        return;
    }

    // Calculate total of space fraction basis value across all spaces for this splitter or mixer
    // including spaces which are not autosized here.
    Real64 spacesTotal = 0.0;
    switch (this->spaceSizingBasis) {
    case DataZoneEquipment::SpaceEquipSizingBasis::DesignCoolingLoad:
        for (auto &thisSpace : this->spaces) {
            spacesTotal += state.dataSize->FinalSpaceSizing(thisSpace.spaceIndex).DesCoolLoad;
        }
        break;
    case DataZoneEquipment::SpaceEquipSizingBasis::DesignHeatingLoad:
        for (auto &thisSpace : this->spaces) {
            spacesTotal += state.dataSize->FinalSpaceSizing(thisSpace.spaceIndex).DesHeatLoad;
        }
        break;
    case DataZoneEquipment::SpaceEquipSizingBasis::FloorArea:
        for (auto &thisSpace : this->spaces) {
            spacesTotal += state.dataHeatBal->space(thisSpace.spaceIndex).FloorArea;
        }
        break;
    case DataZoneEquipment::SpaceEquipSizingBasis::Volume:
        for (auto &thisSpace : this->spaces) {
            spacesTotal += state.dataHeatBal->space(thisSpace.spaceIndex).Volume;
        }
        break;
    case DataZoneEquipment::SpaceEquipSizingBasis::PerimeterLength:
        for (auto &thisSpace : this->spaces) {
            spacesTotal += state.dataHeatBal->space(thisSpace.spaceIndex).extPerimeter;
        }
        break;
    default:
        // If method is not set, then return
        return;
        break;
    }

    if (spacesTotal < 0.00001) {
        ShowSevereError(state,
                        format("ZoneEquipmentSplitterMixer::size: Total {} is zero for {}={}. Unable to autosize Space Fractions.",
                               DataZoneEquipment::spaceEquipSizingBasisNamesUC[(int)this->spaceSizingBasis],
                               BranchNodeConnections::ConnectionObjectTypeNames[(int)this->spaceEquipType],
                               this->Name));
        Real64 spaceFrac = 1.0 / (int)this->spaces.size();
        ShowContinueError(state, format("Setting space fractions to 1/number of spaces = {}.", spaceFrac));
        for (auto &thisSpace : this->spaces) {
            thisSpace.fraction = spaceFrac;
        }
    } else {
        // Calculate space fractions
        for (auto &thisSpace : this->spaces) {
            if (thisSpace.fraction == DataSizing::AutoSize) {
                switch (this->spaceSizingBasis) {
                case DataZoneEquipment::SpaceEquipSizingBasis::DesignCoolingLoad:
                    thisSpace.fraction = state.dataSize->FinalSpaceSizing(thisSpace.spaceIndex).DesCoolLoad / spacesTotal;
                    break;
                case DataZoneEquipment::SpaceEquipSizingBasis::DesignHeatingLoad:
                    thisSpace.fraction = state.dataSize->FinalSpaceSizing(thisSpace.spaceIndex).DesHeatLoad / spacesTotal;
                    break;
                case DataZoneEquipment::SpaceEquipSizingBasis::FloorArea:
                    thisSpace.fraction = state.dataHeatBal->space(thisSpace.spaceIndex).FloorArea / spacesTotal;
                    break;
                case DataZoneEquipment::SpaceEquipSizingBasis::Volume:
                    thisSpace.fraction = state.dataHeatBal->space(thisSpace.spaceIndex).Volume / spacesTotal;
                    break;
                case DataZoneEquipment::SpaceEquipSizingBasis::PerimeterLength:
                    thisSpace.fraction = state.dataHeatBal->space(thisSpace.spaceIndex).extPerimeter / spacesTotal;
                    break;
                default:
                    break;
                }
            }
        }
    }
    // Report sizing results
    int spaceCounter = 0;
    for (auto &thisSpace : this->spaces) {
        ++spaceCounter;
        BaseSizer::reportSizerOutput(state,
                                     BranchNodeConnections::ConnectionObjectTypeNames[(int)this->spaceEquipType],
                                     this->Name,
                                     format("Space {} Fraction", spaceCounter),
                                     thisSpace.fraction);
    }
}

void ZoneMixer::setOutletConditions(EnergyPlusData &state)
{
    if (this->outletNodeNum == 0) {
        return;
    }

    Real64 sumEnthalpy = 0.0;
    Real64 sumHumRat = 0.0;
    Real64 sumCO2 = 0.0;
    Real64 sumGenContam = 0.0;
    Real64 sumPressure = 0.0;
    Real64 sumFractions = 0.0;
    auto &outletNode = state.dataLoopNodes->Node(this->outletNodeNum);
    for (auto &mixerSpace : this->spaces) {
        auto const &spaceOutletNode = state.dataLoopNodes->Node(mixerSpace.spaceNodeNum);
        sumEnthalpy += spaceOutletNode.Enthalpy * mixerSpace.fraction;
        sumHumRat += spaceOutletNode.HumRat * mixerSpace.fraction;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            sumCO2 += spaceOutletNode.CO2 * mixerSpace.fraction;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            sumGenContam += spaceOutletNode.GenContam * mixerSpace.fraction;
        }
        sumPressure += spaceOutletNode.Press * mixerSpace.fraction;
        sumFractions += mixerSpace.fraction;
    }

    // For SpaceHVAC:ZoneReturnMixer, the fractions are dynamic and could be zero if there is no flow
    if (sumFractions > 0) {
        outletNode.Enthalpy = sumEnthalpy / sumFractions;
        outletNode.HumRat = sumHumRat / sumFractions;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            outletNode.CO2 = sumCO2 / sumFractions;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            outletNode.GenContam = sumGenContam / sumFractions;
        }
        outletNode.Press = sumPressure / sumFractions;

        // Use Enthalpy and humidity ratio to get outlet temperature from psych chart
        outletNode.Temp = Psychrometrics::PsyTdbFnHW(outletNode.Enthalpy, outletNode.HumRat);
    }
}

void ZoneReturnMixer::setInletConditions(EnergyPlusData &state)
{
    for (auto &mixerSpace : this->spaces) {
        auto &spaceOutletNode = state.dataLoopNodes->Node(mixerSpace.spaceNodeNum);
        int spaceZoneNodeNum = state.dataZoneEquip->spaceEquipConfig(mixerSpace.spaceIndex).ZoneNode;
        auto const &spaceNode = state.dataLoopNodes->Node(spaceZoneNodeNum);
        spaceOutletNode.Temp = spaceNode.Temp;
        spaceOutletNode.HumRat = spaceNode.HumRat;
        spaceOutletNode.Enthalpy = spaceNode.Enthalpy;
        spaceOutletNode.Press = spaceNode.Press;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            spaceOutletNode.CO2 = spaceNode.CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            spaceOutletNode.GenContam = spaceNode.GenContam;
        }
    }
}
void ZoneEquipmentMixer::setInletFlows(EnergyPlusData &state)
{
    if (this->outletNodeNum == 0) {
        return;
    }

    auto &equipInletNode = state.dataLoopNodes->Node(this->outletNodeNum);
    for (auto &mixerSpace : this->spaces) {
        auto &spaceOutletNode = state.dataLoopNodes->Node(mixerSpace.spaceNodeNum);
        spaceOutletNode.MassFlowRate = equipInletNode.MassFlowRate * mixerSpace.fraction;
        spaceOutletNode.MassFlowRateMaxAvail = equipInletNode.MassFlowRateMaxAvail * mixerSpace.fraction;
        spaceOutletNode.MassFlowRateMinAvail = equipInletNode.MassFlowRateMinAvail * mixerSpace.fraction;
    }
}

void ZoneReturnMixer::setInletFlows(EnergyPlusData &state)
{
    if (this->outletNodeNum == 0) {
        return;
    }
    auto &outletNode = state.dataLoopNodes->Node(this->outletNodeNum);

    Real64 sumMixerInletMassFlow = 0;
    for (auto const &mixerSpace : this->spaces) {
        // calc return flows for spaces feeding this mixer
        auto &spaceEquipConfig = state.dataZoneEquip->spaceEquipConfig(mixerSpace.spaceIndex);
        Real64 outletMassFlowRate = outletNode.MassFlowRate; // calcReturnFlows might adjust this parameter value, so make a copy here
        Real64 spaceReturnFlow = 0.0;
        spaceEquipConfig.calcReturnFlows(state, outletMassFlowRate, spaceReturnFlow);
        sumMixerInletMassFlow += spaceReturnFlow;
    }

    for (auto &mixerSpace : this->spaces) {
        auto &spaceOutletNode = state.dataLoopNodes->Node(mixerSpace.spaceNodeNum);
        // For return mixer, fraction is calculated every time step, not a user input
        if (sumMixerInletMassFlow > 0.0) {
            mixerSpace.fraction = spaceOutletNode.MassFlowRate / sumMixerInletMassFlow;
        } else {
            mixerSpace.fraction = 0.0;
        }
        spaceOutletNode.MassFlowRate = outletNode.MassFlowRate * mixerSpace.fraction;
        spaceOutletNode.MassFlowRateMaxAvail = outletNode.MassFlowRateMaxAvail * mixerSpace.fraction;
        spaceOutletNode.MassFlowRateMinAvail = outletNode.MassFlowRateMinAvail * mixerSpace.fraction;
    }
}

void ZoneEquipmentSplitter::adjustLoads(EnergyPlusData &state, int zoneNum, int equipTypeNum)
{
    auto &thisZoneEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum);
    auto &thisZoneMoistureDemand = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum);

    auto &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(zoneNum);

    Real64 sensibleRatio = 1.0;
    Real64 latentRatio = 1.0;
    switch (this->tstatControl) {
    case DataZoneEquipment::ZoneEquipTstatControl::Ideal: {
        return;
    } break; // Do nothing

    case DataZoneEquipment::ZoneEquipTstatControl::SingleSpace: {
        Real64 controlSpaceFrac = this->spaces[this->controlSpaceNumber].fraction;
        if (controlSpaceFrac > 0.0) {
            if (thisZoneEnergyDemand.RemainingOutputRequired != 0.0) {
                sensibleRatio = (state.dataZoneEnergyDemand->spaceSysEnergyDemand(this->controlSpaceIndex).RemainingOutputRequired /
                                 thisZoneEnergyDemand.RemainingOutputRequired) /
                                controlSpaceFrac;
            }
            if (thisZoneMoistureDemand.RemainingOutputRequired != 0.0) {
                latentRatio = (state.dataZoneEnergyDemand->spaceSysMoistureDemand(this->controlSpaceIndex).RemainingOutputRequired /
                               thisZoneMoistureDemand.RemainingOutputRequired) /
                              controlSpaceFrac;
            }
        }
    } break;

    case DataZoneEquipment::ZoneEquipTstatControl::Maximum: {
        int maxSpaceIndex = 0;
        Real64 maxDeltaTemp = 0.0; // Only positive deltaTemps are relevant
        Real64 maxSpaceFrac = 1.0;
        for (auto &splitterSpace : this->spaces) {
            Real64 spaceTemp =
                state.dataZoneTempPredictorCorrector->spaceHeatBalance(splitterSpace.spaceIndex).T1; // Based on calcPredictedSystemLoad usage
            Real64 spaceDeltaTemp = max((zoneTstatSetpt.setptLo - spaceTemp), (spaceTemp - zoneTstatSetpt.setptHi));
            if (spaceDeltaTemp > maxDeltaTemp) {
                maxSpaceIndex = splitterSpace.spaceIndex;
                maxSpaceFrac = splitterSpace.fraction;
                maxDeltaTemp = spaceDeltaTemp;
            }
        }
        if ((maxSpaceIndex > 0) && (maxSpaceFrac > 0.0)) {
            if (thisZoneEnergyDemand.RemainingOutputRequired != 0.0) {
                sensibleRatio = (state.dataZoneEnergyDemand->spaceSysEnergyDemand(maxSpaceIndex).RemainingOutputRequired /
                                 thisZoneEnergyDemand.RemainingOutputRequired) /
                                maxSpaceFrac;
            }
            if (thisZoneMoistureDemand.RemainingOutputRequired != 0.0) {
                latentRatio = (state.dataZoneEnergyDemand->spaceSysMoistureDemand(maxSpaceIndex).RemainingOutputRequired /
                               thisZoneMoistureDemand.RemainingOutputRequired) /
                              maxSpaceFrac;
            }
        }
    } break;
    default:
        break;
    }
    // Save unadjusted zone loads to restore later
    this->saveZoneSysSensibleDemand = thisZoneEnergyDemand;
    this->saveZoneSysMoistureDemand = thisZoneMoistureDemand;
    // Apply zone load adjustment
    ZoneEquipmentManager::adjustSystemOutputRequired(sensibleRatio,
                                                     latentRatio,
                                                     state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum),
                                                     state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum),
                                                     equipTypeNum);
}

void ZoneEquipmentSplitter::distributeOutput(EnergyPlusData &state,
                                             int const zoneNum,
                                             Real64 const sysOutputProvided,
                                             Real64 const latOutputProvided,
                                             Real64 const nonAirSysOutput,
                                             int const equipTypeNum)
{
    for (auto &splitterSpace : this->spaces) {
        if (this->tstatControl != DataZoneEquipment::ZoneEquipTstatControl::Ideal) {
            // Restore zone loads to unadjusted values (for all ZoneEquipTstatControl types except Ideal)
            state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum) = this->saveZoneSysSensibleDemand;
            state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum) = this->saveZoneSysMoistureDemand;
        }

        Real64 spaceFraction = splitterSpace.fraction;
        if (this->tstatControl == DataZoneEquipment::ZoneEquipTstatControl::Ideal) {
            // Proportion output by sensible space load / zone load (varies every timestep, overrides outputFraction)
            auto const &thisZoneSysEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum);
            if (thisZoneSysEnergyDemand.RemainingOutputRequired != 0.0) {
                spaceFraction = state.dataZoneEnergyDemand->spaceSysEnergyDemand(splitterSpace.spaceIndex).RemainingOutputRequired /
                                thisZoneSysEnergyDemand.RemainingOutputRequired;
            }
        }

        Real64 spaceSysOutputProvided = sysOutputProvided * spaceFraction;
        Real64 spaceLatOutputProvided = latOutputProvided * spaceFraction;
        state.dataZoneTempPredictorCorrector->spaceHeatBalance(splitterSpace.spaceIndex).NonAirSystemResponse += nonAirSysOutput * spaceFraction;
        if (this->zoneEquipOutletNodeNum > 0 && splitterSpace.spaceNodeNum > 0) {
            auto const &equipOutletNode = state.dataLoopNodes->Node(this->zoneEquipOutletNodeNum);
            auto &spaceInletNode = state.dataLoopNodes->Node(splitterSpace.spaceNodeNum);
            spaceInletNode.MassFlowRate = equipOutletNode.MassFlowRate * spaceFraction;
            spaceInletNode.MassFlowRateMaxAvail = equipOutletNode.MassFlowRateMaxAvail * spaceFraction;
            spaceInletNode.MassFlowRateMinAvail = equipOutletNode.MassFlowRateMinAvail * spaceFraction;
            spaceInletNode.Temp = equipOutletNode.Temp;
            spaceInletNode.HumRat = equipOutletNode.HumRat;
            spaceInletNode.CO2 = equipOutletNode.CO2;
        }
        ZoneEquipmentManager::updateSystemOutputRequired(state,
                                                         zoneNum,
                                                         spaceSysOutputProvided,
                                                         spaceLatOutputProvided,
                                                         state.dataZoneEnergyDemand->spaceSysEnergyDemand(splitterSpace.spaceIndex),
                                                         state.dataZoneEnergyDemand->spaceSysMoistureDemand(splitterSpace.spaceIndex),
                                                         equipTypeNum);
    }
}

void EquipConfiguration::beginEnvirnInit(EnergyPlusData &state)
{
    auto &zoneNode = state.dataLoopNodes->Node(this->ZoneNode);
    zoneNode.Temp = 20.0;
    zoneNode.MassFlowRate = 0.0;
    zoneNode.Quality = 1.0;
    zoneNode.Press = state.dataEnvrn->OutBaroPress;
    zoneNode.HumRat = state.dataEnvrn->OutHumRat;
    zoneNode.Enthalpy = Psychrometrics::PsyHFnTdbW(zoneNode.Temp, zoneNode.HumRat);
    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        zoneNode.CO2 = state.dataContaminantBalance->OutdoorCO2;
    }
    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        zoneNode.GenContam = state.dataContaminantBalance->OutdoorGC;
    }

    for (int const nodeNum : this->InletNode) {
        auto &inNode = state.dataLoopNodes->Node(nodeNum);
        inNode.Temp = 20.0;
        inNode.MassFlowRate = 0.0;
        inNode.Quality = 1.0;
        inNode.Press = state.dataEnvrn->OutBaroPress;
        inNode.HumRat = state.dataEnvrn->OutHumRat;
        inNode.Enthalpy = Psychrometrics::PsyHFnTdbW(inNode.Temp, inNode.HumRat);
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            inNode.CO2 = state.dataContaminantBalance->OutdoorCO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            inNode.GenContam = state.dataContaminantBalance->OutdoorGC;
        }
    }

    for (int const nodeNum : this->ExhaustNode) {
        auto &exhNode = state.dataLoopNodes->Node(nodeNum);
        exhNode.Temp = 20.0;
        exhNode.MassFlowRate = 0.0;
        exhNode.Quality = 1.0;
        exhNode.Press = state.dataEnvrn->OutBaroPress;
        exhNode.HumRat = state.dataEnvrn->OutHumRat;
        exhNode.Enthalpy = Psychrometrics::PsyHFnTdbW(exhNode.Temp, exhNode.HumRat);
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            exhNode.CO2 = state.dataContaminantBalance->OutdoorCO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            exhNode.GenContam = state.dataContaminantBalance->OutdoorGC;
        }
    }

    // BG CR 7122 following resets return air node.
    int NumRetNodes = this->NumReturnNodes;
    if (NumRetNodes > 0) {
        for (int const nodeNum : this->ReturnNode) {
            auto &returnNode = state.dataLoopNodes->Node(nodeNum);
            returnNode.Temp = 20.0;
            returnNode.MassFlowRate = 0.0;
            returnNode.Quality = 1.0;
            returnNode.Press = state.dataEnvrn->OutBaroPress;
            returnNode.HumRat = state.dataEnvrn->OutHumRat;
            returnNode.Enthalpy = Psychrometrics::PsyHFnTdbW(returnNode.Temp, returnNode.HumRat);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                returnNode.CO2 = state.dataContaminantBalance->OutdoorCO2;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                returnNode.GenContam = state.dataContaminantBalance->OutdoorGC;
            }
        }
    }
}

void EquipConfiguration::hvacTimeStepInit(EnergyPlusData &state, bool FirstHVACIteration)
{
    auto &zoneNode = state.dataLoopNodes->Node(this->ZoneNode);
    this->ExcessZoneExh = 0.0;

    if (FirstHVACIteration) {
        for (int const nodeNum : this->ExhaustNode) {
            auto &exhNode = state.dataLoopNodes->Node(nodeNum);
            exhNode.Temp = zoneNode.Temp;
            exhNode.HumRat = zoneNode.HumRat;
            exhNode.Enthalpy = zoneNode.Enthalpy;
            exhNode.Press = zoneNode.Press;
            exhNode.Quality = zoneNode.Quality;
            exhNode.MassFlowRate = 0.0;
            exhNode.MassFlowRateMaxAvail = 0.0;
            exhNode.MassFlowRateMinAvail = 0.0;
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                exhNode.CO2 = zoneNode.CO2;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                exhNode.GenContam = zoneNode.GenContam;
            }
        }
    }
}

void EquipConfiguration::calcReturnFlows(EnergyPlusData &state,
                                         Real64 &ExpTotalReturnMassFlow,  // Expected total return air mass flow rate
                                         Real64 &FinalTotalReturnMassFlow // Final total return air mass flow rate
)
{
    int numRetNodes = this->NumReturnNodes;
    Real64 totReturnFlow = 0.0; // Total flow to all return nodes in the zone (kg/s)
    Real64 totVarReturnFlow =
        0.0; // Total variable return flow, for return nodes connected to an airloop with an OA system or not with specified flow (kg/s)
    Real64 returnSchedFrac = this->returnFlowFracSched->getCurrentVal();
    this->FixedReturnFlow = false;
    FinalTotalReturnMassFlow = 0.0;
    this->TotAvailAirLoopOA = 0.0;

    // Set initial flow rate for each return node
    for (int returnNum = 1; returnNum <= numRetNodes; ++returnNum) {
        int retNode = this->ReturnNode(returnNum);

        if (retNode > 0) {
            Real64 returnNodeMassFlow = 0.0;
            auto &retNodeData(state.dataLoopNodes->Node(retNode));

            int inletNum = this->ReturnNodeInletNum(returnNum); // which inlet node matches this return node (same airloop)
            int ADUNum = 0;
            if (inletNum > 0) {
                ADUNum = this->InletNodeADUNum(inletNum);
            }
            int airLoop = this->ReturnNodeAirLoopNum(returnNum);
            Real64 airLoopReturnFrac = 1.0;
            if (airLoop > 0) {
                // Establish corresponding airloop inlet(s) mass flow rate and set return node max/min/maxavail
                Real64 inletMassFlow = 0.0;
                int maxMinNodeNum = 0;
                auto const &thisAirLoopFlow(state.dataAirLoop->AirLoopFlow(airLoop));
                if (ADUNum > 0) {
                    // Zone return node could carry supply flow to zone without leaks plus any induced flow from plenum (but don't include other
                    // secondary flows from exhaust nodes)
                    inletMassFlow = state.dataDefineEquipment->AirDistUnit(ADUNum).MassFlowRateZSup +
                                    state.dataDefineEquipment->AirDistUnit(ADUNum).MassFlowRatePlenInd;
                    maxMinNodeNum = state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum;
                } else if (inletNum > 0) {
                    // If not connected to an ADU, then use the inlet node flow
                    inletMassFlow = state.dataLoopNodes->Node(this->InletNode(inletNum)).MassFlowRate;
                    maxMinNodeNum = this->InletNode(inletNum);
                }
                if (maxMinNodeNum > 0) {
                    auto const &maxMinNodeData(state.dataLoopNodes->Node(maxMinNodeNum));
                    retNodeData.MassFlowRateMax = maxMinNodeData.MassFlowRateMax;
                    retNodeData.MassFlowRateMin = maxMinNodeData.MassFlowRateMin;
                    retNodeData.MassFlowRateMaxAvail = maxMinNodeData.MassFlowRateMaxAvail;
                } else {
                    auto const &zoneNodeData(state.dataLoopNodes->Node(this->ZoneNode));
                    retNodeData.MassFlowRateMax = zoneNodeData.MassFlowRateMax;
                    retNodeData.MassFlowRateMin = zoneNodeData.MassFlowRateMin;
                    retNodeData.MassFlowRateMaxAvail = zoneNodeData.MassFlowRateMaxAvail;
                }

                airLoopReturnFrac = thisAirLoopFlow.DesReturnFrac;
                if (state.dataAirSystemsData->PrimaryAirSystems(airLoop).OASysExists && (thisAirLoopFlow.MaxOutAir > 0.0)) {
                    // Set return flow as fraction of matching inlet node flow if there is an OA system and available OA flow > 0.0
                    returnNodeMassFlow = airLoopReturnFrac * inletMassFlow;
                    this->TotAvailAirLoopOA += thisAirLoopFlow.MaxOutAir;
                } else {
                    // Set return flow to matching inlet node flow
                    returnNodeMassFlow = inletMassFlow;
                    this->FixedReturnFlow(returnNum) = true;
                }
            } else {
                returnNodeMassFlow = 0.0;
            }

            // Return node 1 is special
            if (returnNum == 1) {
                // Make no return air flow adjustments during sizing
                if ((state.dataGlobal->DoingSizing) && numRetNodes == 1) {
                    returnNodeMassFlow = ExpTotalReturnMassFlow;
                    if (airLoop > 0) {
                        if (!state.dataAirSystemsData->PrimaryAirSystems(airLoop).OASysExists ||
                            (state.dataAirLoop->AirLoopFlow(airLoop).MaxOutAir == 0.0)) {
                            ExpTotalReturnMassFlow = max(0.0, ExpTotalReturnMassFlow - this->ZoneExhBalanced + this->ZoneExh);
                            returnNodeMassFlow = ExpTotalReturnMassFlow;
                        }
                    }
                } else if (!state.dataGlobal->DoingSizing) {
                    if (this->NumReturnFlowBasisNodes > 0) {
                        // Set base return air flow rate for node 1 using basis node flow rates
                        Real64 basisNodesMassFlow = 0.0;
                        for (int nodeNum = 1; nodeNum <= this->NumReturnFlowBasisNodes; ++nodeNum) {
                            basisNodesMassFlow += state.dataLoopNodes->Node(this->ReturnFlowBasisNode(nodeNum)).MassFlowRate;
                        }
                        returnNodeMassFlow = max(0.0, (basisNodesMassFlow * returnSchedFrac));
                        this->FixedReturnFlow(returnNum) = true;
                    } else {
                        // If only 1 return node, use the standard return mass flow
                        if ((numRetNodes == 1) && !this->FixedReturnFlow(returnNum)) {
                            returnNodeMassFlow = max(0.0, (ExpTotalReturnMassFlow * returnSchedFrac * airLoopReturnFrac));
                        }
                    }
                }
            }
            totReturnFlow += returnNodeMassFlow;
            retNodeData.MassFlowRate = returnNodeMassFlow;
            retNodeData.MassFlowRateMinAvail = 0.0;
            if (!this->FixedReturnFlow(returnNum)) {
                totVarReturnFlow += returnNodeMassFlow;
            }
        }
    }

    // if zone mass balance true, set to expected return flow
    if (state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) {
        // applies zone return flow schedule multiplier
        ExpTotalReturnMassFlow = returnSchedFrac * ExpTotalReturnMassFlow;
        // set air flow rate for each return node
        Real64 zoneTotReturnFlow = 0.0;
        Real64 returnNodeMassFlow = 0.0;
        for (int returnNum = 1; returnNum <= numRetNodes; ++returnNum) {
            int retNode = this->ReturnNode(returnNum);
            if (retNode > 0) {
                if (numRetNodes == 1) {
                    returnNodeMassFlow = ExpTotalReturnMassFlow;
                } else { // multiple return nodes
                    if (ExpTotalReturnMassFlow > 0.0) {
                        Real64 returnAdjFactor = state.dataLoopNodes->Node(retNode).MassFlowRate / ExpTotalReturnMassFlow;
                        returnNodeMassFlow = returnAdjFactor * ExpTotalReturnMassFlow;
                    } else {
                        returnNodeMassFlow = 0.0;
                    }
                }
            }
            zoneTotReturnFlow += returnNodeMassFlow;
        }
        // Adjust return node flows if zone total return flow is > 0
        if (zoneTotReturnFlow > 0.0) {
            for (int returnNum = 1; returnNum <= numRetNodes; ++returnNum) {
                int retNode = this->ReturnNode(returnNum);
                if (retNode > 0) {
                    if (numRetNodes == 1) {
                        // set it to expected return flows
                        state.dataLoopNodes->Node(retNode).MassFlowRate = ExpTotalReturnMassFlow;
                        FinalTotalReturnMassFlow = ExpTotalReturnMassFlow;
                    } else { // multiple return nodes, adjust nodes flow
                        Real64 newReturnFlow = 0.0;
                        Real64 returnAdjFactor = ExpTotalReturnMassFlow / zoneTotReturnFlow;
                        Real64 curReturnFlow = state.dataLoopNodes->Node(retNode).MassFlowRate;
                        newReturnFlow = curReturnFlow * returnAdjFactor;
                        state.dataLoopNodes->Node(retNode).MassFlowRate = newReturnFlow;
                        FinalTotalReturnMassFlow += newReturnFlow;
                    }
                }
            }
        } else {
            FinalTotalReturnMassFlow = ExpTotalReturnMassFlow;
        }
    } else {
        // Adjust return flows if greater than expected (i.e. there is exhaust or mixing flow reducing the total available for return)
        if ((totReturnFlow > ExpTotalReturnMassFlow) && (totVarReturnFlow > 0.0)) {
            Real64 newReturnFlow = 0.0;
            Real64 returnAdjFactor = (1 - ((totReturnFlow - ExpTotalReturnMassFlow) / totVarReturnFlow)); // Return flow adjustment factor
            for (int returnNum = 1; returnNum <= numRetNodes; ++returnNum) {
                int retNode = this->ReturnNode(returnNum);
                Real64 curReturnFlow = state.dataLoopNodes->Node(retNode).MassFlowRate;
                if (retNode > 0) {
                    if (!this->FixedReturnFlow(returnNum)) {
                        newReturnFlow = curReturnFlow * returnAdjFactor;
                        FinalTotalReturnMassFlow += newReturnFlow;
                        state.dataLoopNodes->Node(retNode).MassFlowRate = newReturnFlow;
                    } else {
                        FinalTotalReturnMassFlow += curReturnFlow;
                    }
                }
            }
        } else {
            FinalTotalReturnMassFlow = totReturnFlow;
        }
    }
}

} // namespace EnergyPlus::DataZoneEquipment
