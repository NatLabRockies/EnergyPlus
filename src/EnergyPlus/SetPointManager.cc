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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::SetPointManager {

// Module containing the SetPoint Manager routines

// MODULE INFORMATION:
//       AUTHOR         Fred Buhl
//       DATE WRITTEN   July 1998
//       MODIFIED       Shirey/Raustad (FSEC), Jan 2004
//                      Nov 2004 - Jan 2005 M. J. Witte, GARD Analytics, Inc.
//                        Add new setpoint managers:
//                          SET POINT MANAGER:SINGLE ZONE HEATING and
//                          SET POINT MANAGER:SINGLE ZONE COOLING
//                          SET POINT MANAGER:OUTSIDE AIR PRETREAT
//                        Work supported by ASHRAE research project 1254-RP
//                      Phil Haves Oct 2004
//                      B. Griffith Aug. 2006.
//                      R. Raustad - FSEC: added AllSetPtMgr used for node conflict checks
//                      July 2010 B.A. Nigusse, FSEC/UCF
//                        Added new setpoint managers:
//                          SetpointManager:MultiZone:Heating:Average
//                          SetpointManager:MultiZone:Cooling:Average
//                          SetpointManager:MultiZone:MinimumHumidity:Average
//                          SetpointManager:MultiZone:MaximumHumidity:Average
//                       22Aug2010 Craig Wray - added Fan:ComponentModel
//                      Aug 2010 B.A. Nigusse, FSEC/UCF
//                        Added new setpoint managers:
//                          SetpointManager:MultiZone:Humidity:Minimum
//                          SetpointManager:MultiZone:Humidity:Maximum
//                      July 2011 Chandan Sharma, FSEC/UCF
//                        Added new setpoint managers:
//                          SetpointManager:FollowOutdoorAirTemperature
//                          SetpointManager:FollowSystemNodeTemperature
//                          SetpointManager:FollowGroundTemperature
//                      March 2012, Atefe Makhmalbaf and Heejin Cho, PNNL
//                        Added new setpoint manager:
//                          SetpointManager:CondenserEnteringReset
//                      Jan 2022 Wooyoung Jung, Jeremy Lerond and Jian Zhang, PNNL
//                        Added new setpoint managers:
//                          SetpointManager:SystemNodeReset:Temperature
//                          SetpointManager:SystemNodeReset:Humidity

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// determine all the controller setpoints in the problem.

// METHODOLOGY EMPLOYED:
// Previous time step node data will be used, in a set of fixed, precoded algorithms,
// to determine the current time step's controller setpoints.

using namespace DataLoopNode;
using namespace DataAirLoop;
using namespace Curve;
using Psychrometrics::PsyCpAirFnW;
using Psychrometrics::PsyHFnTdbW;

constexpr std::array<std::string_view, (int)HVAC::CtrlVarType::Num> ctrlVarTypeNames = {"Temperature",
                                                                                        "MaximumTemperature",
                                                                                        "MinimumTemperature",
                                                                                        "HumidityRatio",
                                                                                        "MaximumHumidityRatio",
                                                                                        "MinimumHumidityRatio",
                                                                                        "MassFlowRate",
                                                                                        "MaximumMassFlowRate",
                                                                                        "MinimumMassFlowRate"};

constexpr std::array<std::string_view, (int)HVAC::CtrlVarType::Num> ctrlVarTypeNamesUC = {"TEMPERATURE",
                                                                                          "MAXIMUMTEMPERATURE",
                                                                                          "MINIMUMTEMPERATURE",
                                                                                          "HUMIDITYRATIO",
                                                                                          "MAXIMUMHUMIDITYRATIO",
                                                                                          "MINIMUMHUMIDITYRATIO",
                                                                                          "MASSFLOWRATE",
                                                                                          "MAXIMUMMASSFLOWRATE",
                                                                                          "MINIMUMMASSFLOWRATE"};

constexpr std::array<std::string_view, (int)ControlStrategy::Num> strategyNamesUC = {
    "TEMPERATUREFIRST",
    "FLOWFIRST",
};

constexpr std::array<std::string_view, (int)SPMType::Num> spmTypeNames = {"SetpointManager:Scheduled",
                                                                          "SetpointManager:Scheduled:DualSetpoint",
                                                                          "SetpointManager:OutdoorAirReset",
                                                                          "SetpointManager:SingleZone:Reheat",
                                                                          "SetpointManager:SingleZone:Heating",
                                                                          "SetpointManager:SingleZone:Cooling",
                                                                          "SetpointManager:SingleZone:Humidity:Minimum",
                                                                          "SetpointManager:SingleZone:Humidity:Maximum",
                                                                          "SetpointManager:MixedAir",
                                                                          "SetpointManager:OutdoorAirPretreat",
                                                                          "SetpointManager:Warmest",
                                                                          "SetpointManager:Coldest",
                                                                          "SetpointManager:WarmestTemperatureFlow",
                                                                          "SetpointManager:ReturnAirBypassFlow",
                                                                          "SetpointManager:MultiZone:Cooling:Average",
                                                                          "SetpointManager:MultiZone:Heating:Average",
                                                                          "SetpointManager:MultiZone:MinimumHumidity:Average",
                                                                          "SetpointManager:MultiZone:MaximumHumidity:Average",
                                                                          "SetpointManager:MultiZone:Humidity:Minimum",
                                                                          "SetpointManager:MultiZone:Humidity:Maximum",
                                                                          "SetpointManager:FollowOutdoorAirTemperature",
                                                                          "SetpointManager:FollowSystemNodeTemperature",
                                                                          "SetpointManager:FollowGroundTemperature",
                                                                          "SetpointManager:CondenserEnteringReset",
                                                                          "SetpointManager:CondenserEnteringReset:Ideal",
                                                                          "SetpointManager:SingleZone:OneStageCooling",
                                                                          "SetpointManager:SingleZone:OneStageHeating",
                                                                          "SetpointManager:ReturnTemperature:ChilledWater",
                                                                          "SetpointManager:ReturnTemperature:HotWater",
                                                                          "SetpointManager:ScheduledTES",
                                                                          "SetpointManager:SystemNodeReset:Temperature",
                                                                          "SetpointManager:SystemNodeReset:Humidity"};

constexpr std::array<DataLoopNode::ConnectionObjectType, (int)SPMType::Num> spmNodeObjectTypes = {
    DataLoopNode::ConnectionObjectType::SetpointManagerScheduled,
    DataLoopNode::ConnectionObjectType::SetpointManagerScheduledDualSetpoint,
    DataLoopNode::ConnectionObjectType::SetpointManagerOutdoorAirReset,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneReheat,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneHeating,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneCooling,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneHumidityMinimum,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneHumidityMaximum,
    DataLoopNode::ConnectionObjectType::SetpointManagerMixedAir,
    DataLoopNode::ConnectionObjectType::SetpointManagerOutdoorAirPretreat,
    DataLoopNode::ConnectionObjectType::SetpointManagerWarmest,
    DataLoopNode::ConnectionObjectType::SetpointManagerColdest,
    DataLoopNode::ConnectionObjectType::SetpointManagerWarmestTemperatureFlow,
    DataLoopNode::ConnectionObjectType::Invalid, // SPMType::ReturnAirBypass
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneCoolingAverage,
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneHeatingAverage,
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneMinimumHumidityAverage,
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneMaximumHumidityAverage,
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneHumidityMinimum,
    DataLoopNode::ConnectionObjectType::SetpointManagerMultiZoneHumidityMaximum,
    DataLoopNode::ConnectionObjectType::SetpointManagerFollowOutdoorAirTemperature,
    DataLoopNode::ConnectionObjectType::SetpointManagerFollowSystemNodeTemperature,
    DataLoopNode::ConnectionObjectType::SetpointManagerFollowGroundTemperature,
    DataLoopNode::ConnectionObjectType::SetpointManagerCondenserEnteringReset,
    DataLoopNode::ConnectionObjectType::SetpointManagerCondenserEnteringResetIdeal,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneOneStageCooling,
    DataLoopNode::ConnectionObjectType::SetpointManagerSingleZoneOneStageHeating,
    DataLoopNode::ConnectionObjectType::SetpointManagerReturnTemperatureChilledWater,
    DataLoopNode::ConnectionObjectType::SetpointManagerReturnTemperatureHotWater,
    DataLoopNode::ConnectionObjectType::Invalid, // SPMType::TESScheduled
    DataLoopNode::ConnectionObjectType::SetpointManagerSystemNodeResetTemperature,
    DataLoopNode::ConnectionObjectType::SetpointManagerSystemNodeResetHumidity};

constexpr std::array<std::string_view, (int)SupplyFlowTempStrategy::Num> supplyFlowTempStrategyNamesUC = {"MAXIMUMTEMPERATURE", "MINIMUMTEMPERATURE"};

// Why?
constexpr std::array<std::string_view, (int)AirTempType::Num> oaTempTypeNamesUC = {"OUTDOORAIRWETBULB", "OUTDOORAIRDRYBULB"};

// No really, why?
constexpr std::array<std::string_view, (int)AirTempType::Num> nodeTempTypeNamesUC = {"NODEWETBULB", "NODEDRYBULB"};

constexpr std::array<std::string_view, (int)DataEnvironment::GroundTempType::Num> groundTempObjectTypeNamesUC = {
    "SITE:GROUNDTEMPERATURE:BUILDINGSURFACE",
    "SITE:GROUNDTEMPERATURE:SHALLOW",
    "SITE:GROUNDTEMPERATURE:DEEP",
    "SITE:GROUNDTEMPERATURE:FCFACTORMETHOD"};

constexpr std::array<std::string_view, (int)ReturnTempType::Num> returnTempTypeNames = {"Scheduled", "Constant", "ReturnTemperatureSetpoint"};
constexpr std::array<std::string_view, (int)ReturnTempType::Num> returnTempTypeNamesUC = {"SCHEDULED", "CONSTANT", "RETURNTEMPERATURESETPOINT"};

void ManageSetPoints(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor, Rick Strand
    //       DATE WRITTEN   May 1998
    //       MODIFIED       Fred Buhl May 2000

    // PURPOSE OF THIS SUBROUTINE:

    // METHODOLOGY EMPLOYED:
    // Each flag is checked and the appropriate manager is then called.

    // First time ManageSetPoints is called, get the input for all the setpoint managers
    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    InitSetPointManagers(state);

    if (state.dataSetPointManager->ManagerOn) {
        SimSetPointManagers(state);
        UpdateSetPointManagers(state);
        // The Mixed Air Setpoint Managers (since they depend on other setpoints, they must be calculated
        // and updated next to last).
        for (auto *spm : state.dataSetPointManager->spms) {
            if (spm->type == SPMType::MixedAir) {
                spm->calculate(state);
            }
        }
        UpdateMixedAirSetPoints(state);
        // The Outside Air Pretreat Setpoint Managers (since they depend on other setpoints, they must be calculated
        // and updated last).
        for (auto *spm : state.dataSetPointManager->spms) {
            if (spm->type == SPMType::OutsideAirPretreat) {
                spm->calculate(state);
            }
        }

        UpdateOAPretreatSetPoints(state);
    }
} // ManageSetPoints()

int GetSetPointManagerIndex(EnergyPlusData const &state, std::string const &Name)
{
    auto found = state.dataSetPointManager->spmMap.find(Name);
    return (found != state.dataSetPointManager->spmMap.end()) ? found->second : 0;
} // GetSetPointManagerIndex()

void GetSetPointManagerInputs(EnergyPlusData &state)
{
    // wrapper for GetInput to allow unit testing when fatal inputs are detected
    constexpr std::string_view routineName = "GetSetPointManagerInputs"; // include trailing blank space

    if (state.dataSetPointManager->GetInputFlag) {
        bool ErrorsFound(false);
        GetSetPointManagerInputData(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input.  Program terminates.", routineName));
        }
        state.dataSetPointManager->GetInputFlag = false;
    }
} // GetSetPointManagerInputs()

void GetSetPointManagerInputData(EnergyPlusData &state, bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   July 1998
    //       MODIFIED       Shirey/Raustad (FSEC), Jan 2004
    //                      Nov 2004 - Jan 2005 M. J. Witte, GARD Analytics, Inc.
    //                        Add new setpoint managers:
    //                          SET POINT MANAGER:SINGLE ZONE HEATING and
    //                          SET POINT MANAGER:SINGLE ZONE COOLING
    //                          SET POINT MANAGER:OUTSIDE AIR PRETREAT
    //                        Work supported by ASHRAE research project 1254-RP
    //                      Haves October 2004
    //                      Witte (GARD), Sep 2006
    //                      July 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Heating:Average
    //                          SetpointManager:MultiZone:Cooling:Average
    //                          SetpointManager:MultiZone:MinimumHumidity:Average
    //                          SetpointManager:MultiZone:MaximumHumidity:Average
    //                      Aug 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Humidity:Minimum
    //                          SetpointManager:MultiZone:Humidity:Maximum
    //                      Jan 2022 Wooyoung Jung, Jeremy Lerond, and Jian Zhang, PNNL
    //                        Added new setpoint managers:
    //                          SetpointManager:SystemNodeReset:Temperature
    //                          SetpointManager:SystemNodeReset:Humidity

    // PURPOSE OF THIS SUBROUTINE
    // Input the SetPointManager data and store it in the SetPtMgrIn array.
    // Examine the Controllers in the input data and determine which ones
    // will have their setpoints set by a particular Setpoint Manager.

    // METHODOLOGY EMPLOYED:
    // Use the Get routines from the InputProcessor module.

    // Using/Aliasing
    using DataZoneEquipment::GetSystemNodeNumberForZone;
    using General::FindNumberInList;

    using NodeInputManager::GetNodeNums;
    using NodeInputManager::GetOnlySingleNode;

    // Locals
    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view routineName = "GetSetPointManagerInputs";

    std::string cCurrentModuleObject;

    int NumNodes;
    Array1D_int NodeNums;
    bool NodeListError(false);
    bool ErrInList;

    auto &ip = state.dataInputProcessing->inputProcessor;

    int NumNums = 0;
    int NumAlphas = 0;
    int NumParams = 0;

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNums);
    NodeNums.dimension(NumParams, 0);

    // Input the data for each Setpoint Manager

    for (int iSPM = 0; iSPM < (int)SPMType::Num; ++iSPM) {
        SPMType type = static_cast<SPMType>(iSPM);
        cCurrentModuleObject = spmTypeNames[iSPM];

        auto const instances = ip->epJSON.find(cCurrentModuleObject);
        if (instances == ip->epJSON.end()) {
            continue; // No SetPointManagers of this type
        }

        auto const &props = ip->getObjectSchemaProps(state, cCurrentModuleObject);

#define PRESERVE_IDF_ORDER
#ifdef PRESERVE_IDF_ORDER

        // Try to read these in IDF order (for now, can revert to simpler begin(), ++, end() iterator later)

        // Begin by grabbing the keys and IDF numbers of the objects.
        // Have to grab the keys because once you are no longer
        // dealing with the map iterator, you lose access to the key,
        // which you also need because it is the object name.

        std::vector<int> idfNums;
        std::vector<std::string> idfKeys;
        int idfFakeNum = 0;
        for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
            // If the input comes from epJson then there are no idf_order fields.
            if (state.dataGlobal->isEpJSON || !state.dataGlobal->preserveIDFOrder) {
                idfNums.push_back(++idfFakeNum);
            } else {
                idfNums.push_back(instance.value().at("idf_order").get<int>());
            }
            idfKeys.push_back(instance.key());
        }

        // Now sort the IDF numbers
        std::vector<int> idfSortedNums = idfNums;
        std::sort(idfSortedNums.begin(), idfSortedNums.end());

        // Iterate through the sorted IDF numbers
        for (int idfSortedNum : idfSortedNums) {
            // Find that number's position in the epJSON order
            int epJsonNum = std::find(idfNums.begin(), idfNums.end(), idfSortedNum) - idfNums.begin();
            // Grab the corresponding name
            std::string const &key = idfKeys[epJsonNum];
            auto const &fields = instances.value().at(key);

#else // !PRESERVE_IDF_ORDER

        // epJson order
        for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
            std::string const &key = instance.key();
            auto const &fields = instance.value();

#endif // PRESERVE_IDF_ORDER

            ip->markObjectAsUsed(cCurrentModuleObject, key);
            std::string name = Util::makeUPPER(key);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, name};

            if (state.dataSetPointManager->spmMap.find(name) != state.dataSetPointManager->spmMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            SPMBase *spm = nullptr;

            // Create a SetPointManagerObject of the right child type
            switch (type) {
            case SPMType::Scheduled: {
                spm = new SPMScheduled;
            } break;
            case SPMType::ScheduledDual: {
                spm = new SPMScheduledDual;
            } break;
            case SPMType::OutsideAir: {
                spm = new SPMOutsideAir;
            } break;
            case SPMType::SZReheat: {
                spm = new SPMSingleZoneReheat;
            } break;
            case SPMType::SZHeating:
            case SPMType::SZCooling: {
                spm = new SPMSingleZoneTemp;
            } break;
            case SPMType::SZMinHum:
            case SPMType::SZMaxHum: {
                spm = new SPMSingleZoneHum;
            } break;
            case SPMType::MixedAir: {
                spm = new SPMMixedAir;
            } break;
            case SPMType::OutsideAirPretreat: {
                spm = new SPMOutsideAirPretreat;
            } break;
            case SPMType::Warmest:
            case SPMType::Coldest: {
                spm = new SPMTempest;
            } break;
            case SPMType::WarmestTempFlow: {
                spm = new SPMWarmestTempFlow;
            } break;
            case SPMType::MZCoolingAverage:
            case SPMType::MZHeatingAverage: {
                spm = new SPMMultiZoneTemp;
            } break;
            case SPMType::MZMinHumAverage:
            case SPMType::MZMaxHumAverage:
            case SPMType::MZMinHum:
            case SPMType::MZMaxHum: {
                spm = new SPMMultiZoneHum;
            } break;
            case SPMType::ReturnAirBypass: {
                spm = new SPMReturnAirBypassFlow;
            } break;
            case SPMType::FollowOutsideAirTemp: {
                spm = new SPMFollowOutsideAirTemp;
            } break;
            case SPMType::FollowSystemNodeTemp: {
                spm = new SPMFollowSysNodeTemp;
            } break;
            case SPMType::FollowGroundTemp: {
                spm = new SPMFollowGroundTemp;
            } break;
            case SPMType::CondenserEnteringTemp: {
                spm = new SPMCondenserEnteringTemp;
            } break;
            case SPMType::IdealCondenserEnteringTemp: {
                spm = new SPMIdealCondenserEnteringTemp;
            } break;
            case SPMType::SZOneStageCooling: {
                spm = new SPMSingleZoneOneStageCooling;
            } break;
            case SPMType::SZOneStageHeating: {
                spm = new SPMSingleZoneOneStageHeating;
            } break;
            case SPMType::ChilledWaterReturnTemp:
            case SPMType::HotWaterReturnTemp: {
                spm = new SPMReturnWaterTemp;
            } break;
            case SPMType::TESScheduled: {
                spm = new SPMTESScheduled;
            } break;
            case SPMType::SystemNodeTemp:
            case SPMType::SystemNodeHum: {
                spm = new SPMSystemNode;
            } break;
            default: {
                assert(false);
            } break;
            } // switch (type)

            // Set name and append to array
            spm->Name = name;
            spm->type = type;
            state.dataSetPointManager->spms.push_back(spm);
            state.dataSetPointManager->spmMap.insert_or_assign(spm->Name, state.dataSetPointManager->spms.size());

            // control variable type
            std::string ctrlVarName;
            switch (spm->type) {
            case SPMType::SZMinHum: {
                spm->ctrlVar = HVAC::CtrlVarType::MinHumRat;
            } break;
            case SPMType::SZMaxHum: {
                spm->ctrlVar = HVAC::CtrlVarType::MaxHumRat;
            } break;
            case SPMType::MZHeatingAverage:
            case SPMType::MZCoolingAverage: {
                spm->ctrlVar = HVAC::CtrlVarType::Temp;
            } break;
            case SPMType::MZMinHumAverage:
            case SPMType::MZMinHum: {
                spm->ctrlVar = HVAC::CtrlVarType::MinHumRat;
            } break;
            case SPMType::MZMaxHumAverage:
            case SPMType::MZMaxHum: {
                spm->ctrlVar = HVAC::CtrlVarType::MaxHumRat;
            } break;
            case SPMType::SZOneStageCooling:
            case SPMType::SZOneStageHeating: {
                spm->ctrlVar = HVAC::CtrlVarType::Temp;
            } break;
            case SPMType::ChilledWaterReturnTemp:
            case SPMType::HotWaterReturnTemp: {
                spm->ctrlVar = HVAC::CtrlVarType::Temp;
            } break;
            case SPMType::ReturnAirBypass: {
                spm->ctrlVar = HVAC::CtrlVarType::MassFlowRate;
            } break;

            default: {
                ctrlVarName = ip->getAlphaFieldValue(fields, props, "control_variable");
                spm->ctrlVar = static_cast<HVAC::CtrlVarType>(getEnumValue(ctrlVarTypeNamesUC, ctrlVarName));
            } break;
            } // switch (spm->type)

            // Load Min and Max Temp setpoints for some SPMs
            switch (spm->type) {
            case SPMType::OutsideAirPretreat:
            case SPMType::Warmest:
            case SPMType::Coldest:
            case SPMType::WarmestTempFlow:
            case SPMType::MZCoolingAverage:
            case SPMType::MZHeatingAverage:
            case SPMType::FollowOutsideAirTemp:
            case SPMType::FollowGroundTemp: {
                spm->minSetTemp = ip->getRealFieldValue(fields, props, "minimum_setpoint_temperature");
                spm->maxSetTemp = ip->getRealFieldValue(fields, props, "maximum_setpoint_temperature");
                if (spm->maxSetTemp < spm->minSetTemp) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state,
                                      format("...maximum_supply_air_temperature=[{:.1R}] is less than minimum_supply_air_temperature=[{:.1R}].",
                                             spm->maxSetTemp,
                                             spm->minSetTemp));
                }
            } break;

            case SPMType::SZReheat:
            case SPMType::SZHeating:
            case SPMType::SZCooling: {
                spm->minSetTemp = ip->getRealFieldValue(fields, props, "minimum_supply_air_temperature");
                spm->maxSetTemp = ip->getRealFieldValue(fields, props, "maximum_supply_air_temperature");
                if (spm->maxSetTemp < spm->minSetTemp) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state,
                                      format("...maximum_supply_air_temperature=[{:.1R}] is less than minimum_supply_air_temperature=[{:.1R}].",
                                             spm->maxSetTemp,
                                             spm->minSetTemp));
                }
            } break;

            case SPMType::FollowSystemNodeTemp: {
                spm->minSetTemp = ip->getRealFieldValue(fields, props, "minimum_limit_setpoint_temperature");
                spm->maxSetTemp = ip->getRealFieldValue(fields, props, "maximum_limit_setpoint_temperature");
                if (spm->maxSetTemp < spm->minSetTemp) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state,
                                      format("...maximum_supply_air_temperature=[{:.1R}] is less than minimum_supply_air_temperature=[{:.1R}].",
                                             spm->maxSetTemp,
                                             spm->minSetTemp));
                }
            } break;

            default:
                break;
            } // switch (spm->type)

            // Read Min and Max HumRat for some SPMs
            switch (spm->type) {

            case SPMType::OutsideAirPretreat:
            case SPMType::MZMinHumAverage:
            case SPMType::MZMaxHumAverage:
            case SPMType::MZMinHum:
            case SPMType::MZMaxHum: {
                spm->minSetHum = ip->getRealFieldValue(fields, props, "minimum_setpoint_humidity_ratio");
                spm->maxSetHum = ip->getRealFieldValue(fields, props, "maximum_setpoint_humidity_ratio");
                if (spm->maxSetHum < spm->minSetHum) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state,
                                      format("...maximum_setpoint_humidity_ratio=[{:.1R}] is less than minimum_setpoint_humidity_ratio=[{:.1R}].",
                                             spm->maxSetHum,
                                             spm->minSetHum));
                }

                // Because a zero humidity ratio setpoint is a special value indicating "off" or "no load"
                // must not allow MinSetHumRat or MaxSetHumRat to be <=0.0
                if (spm->minSetHum <= 0.0) {
                    ShowWarningError(state, format("{}: {}=\"{}\", invalid value.", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state, "Minimum setpoint humidity ratio <=0.0, resetting to 0.00001");
                    spm->minSetHum = 0.00001;
                }
                if (spm->maxSetHum <= 0.0) {
                    ShowWarningError(state, format("{}: {}=\"{}\", invalid value.", routineName, cCurrentModuleObject, spm->Name));
                    ShowContinueError(state, "Maximum setpoint humidity ratio <=0.0, resetting to 0.00001");
                    spm->maxSetHum = 0.00001;
                }

            } break;
            default:
                break;
            } // switch (spm->type)

            // Read HVAC Air Loop name
            switch (spm->type) {
            case SPMType::Warmest:
            case SPMType::Coldest:
            case SPMType::WarmestTempFlow:
            case SPMType::ReturnAirBypass:
            case SPMType::MZCoolingAverage:
            case SPMType::MZHeatingAverage:
            case SPMType::MZMinHumAverage:
            case SPMType::MZMaxHumAverage:
            case SPMType::MZMinHum:
            case SPMType::MZMaxHum: {
                spm->airLoopName = ip->getAlphaFieldValue(fields, props, "hvac_air_loop_name");
                spm->airLoopNum = 0;
            } break;

            default:
                break;
            } // switch (spm->type)

            // Read SPM-specific fields
            switch (spm->type) {

            // SetpointManager:Scheduled
            case SPMType::Scheduled: {
                auto *spmS = dynamic_cast<SPMScheduled *>(spm);
                assert(spmS != nullptr);

                std::string schedName = ip->getAlphaFieldValue(fields, props, "schedule_name");
                if ((spmS->sched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "schedule_name", schedName);
                    ErrorsFound = true;
                }
                spmS->setPt = 0.0;
            } break;

            // SetpointManager:Scheduled:DualSetpoint
            case SPMType::ScheduledDual: {
                auto *spmSD = dynamic_cast<SPMScheduledDual *>(spm);
                assert(spmSD != nullptr);

                if (spmSD->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                std::string schedHiName = ip->getAlphaFieldValue(fields, props, "high_setpoint_schedule_name");
                if ((spmSD->hiSched = Sched::GetSchedule(state, Util::makeUPPER(schedHiName))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "high_setpoint_schedule_name", schedHiName);
                    ErrorsFound = true;
                }

                std::string schedLoName = ip->getAlphaFieldValue(fields, props, "low_setpoint_schedule_name");
                if ((spmSD->loSched = Sched::GetSchedule(state, Util::makeUPPER(schedLoName))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "low_setpoint_schedule_name", schedLoName);
                    ErrorsFound = true;
                }
                spmSD->setPtHi = 0.0;
                spmSD->setPtLo = 0.0;

            } break;

            // SetpointManager:OutdoorAirReset
            case SPMType::OutsideAir: {
                auto *spmOA = dynamic_cast<SPMOutsideAir *>(spm);
                assert(spmOA != nullptr);
                if (spmOA->ctrlVar != HVAC::CtrlVarType::Temp && spmOA->ctrlVar != HVAC::CtrlVarType::MaxTemp &&
                    spmOA->ctrlVar != HVAC::CtrlVarType::MinTemp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmOA->lowSetPt1 = ip->getRealFieldValue(fields, props, "setpoint_at_outdoor_low_temperature");
                spmOA->low1 = ip->getRealFieldValue(fields, props, "outdoor_low_temperature");
                spmOA->highSetPt1 = ip->getRealFieldValue(fields, props, "setpoint_at_outdoor_high_temperature");
                spmOA->high1 = ip->getRealFieldValue(fields, props, "outdoor_high_temperature");

                // Get optional input: schedule and 2nd reset rule
                if (auto foundSched = fields.find("schedule_name"); foundSched != fields.end()) {
                    std::string schedName = Util::makeUPPER(foundSched.value().get<std::string>());
                    if ((spmOA->sched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, "schedule_name", schedName);
                        ErrorsFound = true;
                    } else if (!spmOA->sched->checkMinMaxVals(state, Clusive::In, 1.0, Clusive::In, 2.0)) {
                        Sched::ShowSevereBadMinMax(state, eoh, "schedule_name", schedName, Clusive::In, 1.0, Clusive::In, 2.0);
                        ErrorsFound = true;
                    }

                    if (auto found = fields.find("setpoint_at_outdoor_low_temperature_2"); found != fields.end()) {
                        spmOA->lowSetPt2 = found.value().get<Real64>();
                    }
                    if (auto found = fields.find("outdoor_low_temperature_2"); found != fields.end()) {
                        spmOA->low2 = found.value().get<Real64>();
                    }
                    if (auto found = fields.find("setpoint_at_outdoor_high_temperature_2"); found != fields.end()) {
                        spmOA->highSetPt2 = found.value().get<Real64>();
                    }
                    if (auto found = fields.find("outdoor_high_temperature_2"); found != fields.end()) {
                        spmOA->high2 = found.value().get<Real64>();
                    }
                    if (spmOA->high2 < spmOA->low2) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format("...{}=[{:.1R}] is less than {}=[{:.1R}].",
                                                 "outdoor_high_temperature_2",
                                                 spmOA->high2,
                                                 "outdoor_low_temperature_2",
                                                 spmOA->low2));
                    }
                } else { // !foundSched
                    spmOA->sched = nullptr;
                    spmOA->lowSetPt2 = 0.0;
                    spmOA->low2 = 0.0;
                    spmOA->highSetPt2 = 0.0;
                    spmOA->high2 = 0.0;
                }
            } break;

            //  SetpointManager:SingleZone:Reheat
            case SPMType::SZReheat: {
                auto *spmSZR = dynamic_cast<SPMSingleZoneReheat *>(spm);
                assert(spmSZR != nullptr);

                if (spmSZR->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                std::string ctrlZoneName = ip->getAlphaFieldValue(fields, props, "control_zone_name");
                // get the actual zone number of the control zone
                spmSZR->ctrlZoneNum = Util::FindItemInList(ctrlZoneName, state.dataHeatBal->Zone);
                if (spmSZR->ctrlZoneNum == 0) {
                    ShowSevereItemNotFound(state, eoh, "control_zone_name", ctrlZoneName);
                    ErrorsFound = true;
                }
                spmSZR->setPt = 0.0;

                spmSZR->zoneNodeNum = GetOnlySingleNode(state,
                                                        ip->getAlphaFieldValue(fields, props, "zone_node_name"),
                                                        ErrorsFound,
                                                        spmNodeObjectTypes[(int)spm->type],
                                                        spmSZR->Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Sensor,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsNotParent);
                spmSZR->zoneInletNodeNum = GetOnlySingleNode(state,
                                                             ip->getAlphaFieldValue(fields, props, "zone_inlet_node_name"),
                                                             ErrorsFound,
                                                             spmNodeObjectTypes[(int)spm->type],
                                                             spmSZR->Name,
                                                             DataLoopNode::NodeFluidType::Air,
                                                             DataLoopNode::ConnectionType::Sensor,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             ObjectIsNotParent);
            } break;

            // SetpointManager:SingleZone:Heating
            // SetpointManager:SingleZone:Cooling
            case SPMType::SZHeating:
            case SPMType::SZCooling: {
                auto *spmSZTemp = dynamic_cast<SPMSingleZoneTemp *>(spm);
                assert(spmSZTemp != nullptr);

                if (spmSZTemp->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                std::string ctrlZoneName = ip->getAlphaFieldValue(fields, props, "control_zone_name");
                spmSZTemp->ctrlZoneNum = Util::FindItemInList(ctrlZoneName, state.dataHeatBal->Zone);
                if (spmSZTemp->ctrlZoneNum == 0) {
                    ShowSevereItemNotFound(state, eoh, "control_zone_name", ctrlZoneName);
                    ErrorsFound = true;
                }
                spmSZTemp->setPt = 0.0;

                spmSZTemp->zoneNodeNum = GetOnlySingleNode(state,
                                                           ip->getAlphaFieldValue(fields, props, "zone_node_name"),
                                                           ErrorsFound,
                                                           spmNodeObjectTypes[(int)spm->type],
                                                           spmSZTemp->Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Sensor,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent);
                spmSZTemp->zoneInletNodeNum = GetOnlySingleNode(state,
                                                                ip->getAlphaFieldValue(fields, props, "zone_inlet_node_name"),
                                                                ErrorsFound,
                                                                spmNodeObjectTypes[(int)spm->type],
                                                                spmSZTemp->Name,
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Sensor,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                ObjectIsNotParent);

            } break;

            // SetpointManager:SingleZone:Humidity:Minimum
            // SetpointManager:SingleZone:Humidity:Maximum
            case SPMType::SZMinHum:
            case SPMType::SZMaxHum: {
                auto *spmSZHum = dynamic_cast<SPMSingleZoneHum *>(spm);
                assert(spmSZHum != nullptr);

                ErrInList = false;
                std::string ctrlZoneNodeName = ip->getAlphaFieldValue(fields, props, "control_zone_air_node_name");
                GetNodeNums(state,
                            ctrlZoneNodeName,
                            NumNodes,
                            NodeNums,
                            ErrInList,
                            DataLoopNode::NodeFluidType::Air,
                            spmNodeObjectTypes[(int)spm->type],
                            spmSZHum->Name,
                            DataLoopNode::ConnectionType::Sensor,
                            NodeInputManager::CompFluidStream::Primary,
                            ObjectIsNotParent,
                            false,
                            "control_zone_air_node_name"); // nodes of zones whose humidity is being controlled

                if (ErrInList) {
                    ErrorsFound = true;
                }

                // only allow one control zone for now
                if (NumNodes > 1) {
                    ShowSevereError(state, format("{}: {}=\"{}\", entered nodelist.", routineName, cCurrentModuleObject, spmSZHum->Name));
                    ShowContinueError(state, format("..invalid ctrl_zone_node_name=\"{}\".", ctrlZoneNodeName));
                    ShowContinueError(state, "..only one control zone is allowed.");
                    ErrorsFound = true;
                }

                spmSZHum->zoneNodeNum = NodeNums(1);
                spmSZHum->ctrlZoneNum = 0;
            } break;

            // SetpointManager:MixedAir
            case SPMType::MixedAir: {
                auto *spmMA = dynamic_cast<SPMMixedAir *>(spm);
                assert(spmMA != nullptr);

                if (spmMA->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmMA->refNodeNum = GetOnlySingleNode(state,
                                                      ip->getAlphaFieldValue(fields, props, "reference_setpoint_node_name"),
                                                      ErrorsFound,
                                                      spmNodeObjectTypes[(int)spm->type],
                                                      spmMA->Name,
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Sensor,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      ObjectIsNotParent);
                spmMA->fanInNodeNum = GetOnlySingleNode(state,
                                                        ip->getAlphaFieldValue(fields, props, "fan_inlet_node_name"),
                                                        ErrorsFound,
                                                        spmNodeObjectTypes[(int)spm->type],
                                                        spmMA->Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Sensor,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsNotParent);
                spmMA->fanOutNodeNum = GetOnlySingleNode(state,
                                                         ip->getAlphaFieldValue(fields, props, "fan_outlet_node_name"),
                                                         ErrorsFound,
                                                         spmNodeObjectTypes[(int)spm->type],
                                                         spmMA->Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Sensor,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         ObjectIsNotParent);
            } break;

            // SetpointManager:OutdoorAirPretreat
            case SPMType::OutsideAirPretreat: {
                auto *spmOAP = dynamic_cast<SPMOutsideAirPretreat *>(spm);
                assert(spmOAP != nullptr);

                spmOAP->refNodeNum = GetOnlySingleNode(state,
                                                       ip->getAlphaFieldValue(fields, props, "reference_setpoint_node_name"),
                                                       ErrorsFound,
                                                       spmNodeObjectTypes[(int)spm->type],
                                                       spmOAP->Name,
                                                       DataLoopNode::NodeFluidType::Air,
                                                       DataLoopNode::ConnectionType::Sensor,
                                                       NodeInputManager::CompFluidStream::Primary,
                                                       ObjectIsNotParent);
                spmOAP->mixedOutNodeNum = GetOnlySingleNode(state,
                                                            ip->getAlphaFieldValue(fields, props, "mixed_air_stream_node_name"),
                                                            ErrorsFound,
                                                            spmNodeObjectTypes[(int)spm->type],
                                                            spmOAP->Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Sensor,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            ObjectIsNotParent);
                spmOAP->oaInNodeNum = GetOnlySingleNode(state,
                                                        ip->getAlphaFieldValue(fields, props, "outdoor_air_stream_node_name"),
                                                        ErrorsFound,
                                                        spmNodeObjectTypes[(int)spm->type],
                                                        spmOAP->Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Sensor,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsNotParent);
                spmOAP->returnInNodeNum = GetOnlySingleNode(state,
                                                            ip->getAlphaFieldValue(fields, props, "return_air_stream_node_name"),
                                                            ErrorsFound,
                                                            spmNodeObjectTypes[(int)spm->type],
                                                            spmOAP->Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Sensor,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            ObjectIsNotParent);

                if (std::find(spmOAP->ctrlNodeNums.begin(), spmOAP->ctrlNodeNums.end(), spmOAP->refNodeNum) != spmOAP->ctrlNodeNums.end()) {
                    ShowSevereError(state, format("{}: {}=\"{}\", reference node.", routineName, cCurrentModuleObject, spmOAP->Name));
                    if (spmOAP->ctrlNodeNums.size() > 1) {
                        ShowContinueError(state, "..Reference Node is the same as one of the nodes in SetPoint NodeList");
                    } else {
                        ShowContinueError(state, "..Reference Node is the same as the SetPoint Node");
                    }
                    ShowContinueError(state, format("Reference Node Name=\"{}\".", state.dataLoopNodes->NodeID(spmOAP->refNodeNum)));
                    ErrorsFound = true;
                }
            } break;

            // SetpointManager:Warmest
            // SetpointManager:Coldest
            case SPMType::Warmest:
            case SPMType::Coldest: {
                auto *spmEst = dynamic_cast<SPMTempest *>(spm);
                assert(spmEst != nullptr);

                if (spmEst->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                std::string strategyName = ip->getAlphaFieldValue(fields, props, "strategy");
                spmEst->strategy = static_cast<SupplyFlowTempStrategy>(getEnumValue(supplyFlowTempStrategyNamesUC, strategyName));

                if ((spmEst->type == SPMType::Warmest && spmEst->strategy != SupplyFlowTempStrategy::MaxTemp) ||
                    (spmEst->type == SPMType::Coldest && spmEst->strategy != SupplyFlowTempStrategy::MinTemp)) {
                    ShowSevereInvalidKey(state, eoh, "strategy", strategyName);
                    ErrorsFound = true;
                }
            } break;

            // SetpointManager:WarmestTemperatureFlow
            case SPMType::WarmestTempFlow: {
                auto *spmWTF = dynamic_cast<SPMWarmestTempFlow *>(spm);
                assert(spmWTF != nullptr);

                if (spmWTF->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmWTF->minTurndown = ip->getRealFieldValue(fields, props, "minimum_turndown_ratio");
                if (spmWTF->minTurndown >= 0.8) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spmWTF->Name));
                    ShowContinueError(state, format("...minimum_turndown_ratio=[{:.2R}] is greater than 0.8;", spmWTF->minTurndown));
                    ShowContinueError(state, "...typical values for minimum_turndown_ratio are less than 0.8.");
                }

                spmWTF->strategy = static_cast<ControlStrategy>(getEnumValue(strategyNamesUC, ip->getAlphaFieldValue(fields, props, "strategy")));

                SetupOutputVariable(state,
                                    "Setpoint Manager Warmest Temperature Critical Zone Number",
                                    Constant::Units::None,
                                    spmWTF->critZoneNum,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    spmWTF->Name);
                SetupOutputVariable(state,
                                    "Setpoint Manager Warmest Temperature Turndown Flow Fraction",
                                    Constant::Units::None,
                                    spmWTF->turndown,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    spmWTF->Name);
            } break;

            // SetpointManager:ReturnAirBypassFlow
            case SPMType::ReturnAirBypass: {
                auto *spmRAB = dynamic_cast<SPMReturnAirBypassFlow *>(spm);
                assert(spmRAB != nullptr);

                std::string schedName = ip->getAlphaFieldValue(fields, props, "temperature_setpoint_schedule_name");
                if ((spmRAB->sched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "temperature_setpoint_schedule_name", schedName);
                    ErrorsFound = true;
                }
            } break;

            // SetpointManager:FollowOutdoorAirTemperature
            case SPMType::FollowOutsideAirTemp: {
                auto *spmFOAT = dynamic_cast<SPMFollowOutsideAirTemp *>(spm);
                assert(spmFOAT != nullptr);

                if (spmFOAT->ctrlVar != HVAC::CtrlVarType::Temp && spmFOAT->ctrlVar != HVAC::CtrlVarType::MaxTemp &&
                    spmFOAT->ctrlVar != HVAC::CtrlVarType::MinTemp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmFOAT->refTempType =
                    static_cast<AirTempType>(getEnumValue(oaTempTypeNamesUC, ip->getAlphaFieldValue(fields, props, "reference_temperature_type")));

                spmFOAT->offset = ip->getRealFieldValue(fields, props, "offset_temperature_difference");
            } break;

            // SetpointManager:FollowSystemNodeTemperature
            case SPMType::FollowSystemNodeTemp: {
                auto *spmFNT = dynamic_cast<SPMFollowSysNodeTemp *>(spm);
                assert(spmFNT != nullptr);

                if (spmFNT->ctrlVar != HVAC::CtrlVarType::Temp && spmFNT->ctrlVar != HVAC::CtrlVarType::MaxTemp &&
                    spmFNT->ctrlVar != HVAC::CtrlVarType::MinTemp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmFNT->refNodeNum = GetOnlySingleNode(state,
                                                       ip->getAlphaFieldValue(fields, props, "reference_node_name"),
                                                       ErrorsFound,
                                                       spmNodeObjectTypes[(int)spm->type],
                                                       spmFNT->Name,
                                                       DataLoopNode::NodeFluidType::Blank,
                                                       DataLoopNode::ConnectionType::Sensor,
                                                       NodeInputManager::CompFluidStream::Primary,
                                                       ObjectIsNotParent);

                spmFNT->refTempType =
                    static_cast<AirTempType>(getEnumValue(nodeTempTypeNamesUC, ip->getAlphaFieldValue(fields, props, "reference_temperature_type")));

                spmFNT->offset = ip->getRealFieldValue(fields, props, "offset_temperature_difference");
            } break;

            // SetpointManager:FollowGroundTemperature
            case SPMType::FollowGroundTemp: {
                auto *spmFGT = dynamic_cast<SPMFollowGroundTemp *>(spm);
                assert(spmFGT != nullptr);

                if (spmFGT->ctrlVar != HVAC::CtrlVarType::Temp && spmFGT->ctrlVar != HVAC::CtrlVarType::MaxTemp &&
                    spmFGT->ctrlVar != HVAC::CtrlVarType::MinTemp) {
                    // should not come here if idd type choice and key list is working
                    ShowSevereItemNotFound(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmFGT->refTempType = static_cast<DataEnvironment::GroundTempType>(
                    getEnumValue(groundTempObjectTypeNamesUC, ip->getAlphaFieldValue(fields, props, "reference_ground_temperature_object_type")));

                if (state.dataSetPointManager->NoGroundTempObjWarning[(int)spmFGT->refTempType]) {
                    if (!state.dataEnvrn->GroundTempInputs[(int)spmFGT->refTempType]) {
                        ShowWarningError(state,
                                         format("{}: {}=\"{}\" requires \"Site:GroundTemperature:BuildingSurface\" in the input..",
                                                routineName,
                                                cCurrentModuleObject,
                                                spmFGT->Name));
                        ShowContinueError(state,
                                          format("Defaults, constant throughout the year of ({:.1R}) will be used.",
                                                 state.dataEnvrn->GroundTemp[(int)spmFGT->refTempType]));
                    }
                    state.dataSetPointManager->NoGroundTempObjWarning[(int)spmFGT->refTempType] = false;
                }

                spmFGT->offset = ip->getRealFieldValue(fields, props, "offset_temperature_difference");
            } break;

            // SetpointManager:CondenserEnteringReset
            case SPMType::CondenserEnteringTemp: {
                auto *spmCET = dynamic_cast<SPMCondenserEnteringTemp *>(spm);
                assert(spmCET != nullptr);

                if (spmCET->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                std::string condenserEnteringTempSchedName =
                    ip->getAlphaFieldValue(fields, props, "default_condenser_entering_water_temperature_schedule_name");
                if ((spmCET->condenserEnteringTempSched = Sched::GetSchedule(state, Util::makeUPPER(condenserEnteringTempSchedName))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "default_condenser_entering_water_temperature_schedule_name", condenserEnteringTempSchedName);
                    ErrorsFound = true;
                }

                std::string minDesignWetBulbCurveName = ip->getAlphaFieldValue(fields, props, "minimum_design_wetbulb_temperature_curve_name");
                spmCET->minTowerDesignWetBulbCurveNum = GetCurveIndex(state, minDesignWetBulbCurveName);

                std::string minOAWetBulbCurveName = ip->getAlphaFieldValue(fields, props, "minimum_outside_air_wetbulb_temperature_curve_name");
                spmCET->minOAWetBulbCurveNum = GetCurveIndex(state, minOAWetBulbCurveName);

                std::string optCondenserEnteringTempCurveName =
                    ip->getAlphaFieldValue(fields, props, "optimized_cond_entering_water_temperature_curve_name");

                spmCET->optCondenserEnteringTempCurveNum = GetCurveIndex(state, optCondenserEnteringTempCurveName);
                spmCET->minLift = ip->getRealFieldValue(fields, props, "minimum_lift");

                spmCET->maxCondenserEnteringTemp = ip->getRealFieldValue(fields, props, "maximum_condenser_entering_water_temperature");
                spmCET->towerDesignInletAirWetBulbTemp = ip->getRealFieldValue(fields, props, "cooling_tower_design_inlet_air_wet_bulb_temperature");

                if (spmCET->maxCondenserEnteringTemp < spmCET->towerDesignInletAirWetBulbTemp) {
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spmCET->Name));
                    ShowContinueError(state,
                                      format("...maximum_condenser_entering_water_temperature=[{:.1R}] is less than "
                                             "cooling_tower_design_inlet_air_wet-bulb_temperature=[{:.1R}].",
                                             spmCET->maxCondenserEnteringTemp,
                                             spmCET->towerDesignInletAirWetBulbTemp));
                }

            } break;

            // SetpointManager:CondenserEnteringReset:Ideal
            case SPMType::IdealCondenserEnteringTemp: {
                auto *spmIdealCET = dynamic_cast<SPMIdealCondenserEnteringTemp *>(spm);
                assert(spmIdealCET != nullptr);

                if (spmIdealCET->ctrlVar != HVAC::CtrlVarType::Temp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmIdealCET->minLift = ip->getRealFieldValue(fields, props, "minimum_lift");
                spmIdealCET->maxCondenserEnteringTemp = ip->getRealFieldValue(fields, props, "maximum_condenser_entering_water_temperature");

            } break;

            // SetpointManager:SingleZone:OneStageCooling
            case SPMType::SZOneStageCooling: {
                auto *spmSZOSC = dynamic_cast<SPMSingleZoneOneStageCooling *>(spm);
                assert(spmSZOSC != nullptr);

                spmSZOSC->coolingOnSetPt = ip->getRealFieldValue(fields, props, "cooling_stage_on_supply_air_setpoint_temperature");
                spmSZOSC->coolingOffSetPt = ip->getRealFieldValue(fields, props, "cooling_stage_off_supply_air_setpoint_temperature");

                if (spmSZOSC->coolingOffSetPt < spmSZOSC->coolingOnSetPt) {
                    // throw warning, off must be warmer than on
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spmSZOSC->Name));
                    ShowContinueError(state,
                                      format("...cooling_stage_off_supply_air_setpoint_temperature=[{:.1R}] is less than "
                                             "cooling_stage_on_supply_air_setpoint_temperature=[{:.1R}].",
                                             spmSZOSC->coolingOffSetPt,
                                             spmSZOSC->coolingOnSetPt));
                }

                std::string ctrlZoneName = ip->getAlphaFieldValue(fields, props, "control_zone_name");
                // get the actual zone number of the control zone
                spmSZOSC->ctrlZoneNum = Util::FindItemInList(ctrlZoneName, state.dataHeatBal->Zone);
                if (spmSZOSC->ctrlZoneNum == 0) {
                    ShowSevereItemNotFound(state, eoh, "control_zone_name", ctrlZoneName);
                    ErrorsFound = true;
                } else {
                    spmSZOSC->zoneNodeNum = GetSystemNodeNumberForZone(state, spmSZOSC->ctrlZoneNum);
                    if (allocated(state.dataZoneCtrls->StageZoneLogic)) {
                        if (!state.dataZoneCtrls->StageZoneLogic(spmSZOSC->ctrlZoneNum)) {
                            ShowSevereError(state, format("{}: {}=\"{}\", invalid field.", routineName, cCurrentModuleObject, spmSZOSC->Name));
                            ShowContinueError(state, format("..invalid control_zone_name=\"{}\".", ctrlZoneName));
                            ShowContinueError(state, "Zone thermostat must use ZoneControl:Thermostat:StagedDualSetpoint.");
                            ErrorsFound = true;
                        }
                    }
                }
            } break;

            // SetpointManager:SingleZone:OneStageHeating
            case SPMType::SZOneStageHeating: {
                auto *spmSZOSH = dynamic_cast<SPMSingleZoneOneStageHeating *>(spm);
                assert(spmSZOSH != nullptr);

                spmSZOSH->heatingOnSetPt = ip->getRealFieldValue(fields, props, "heating_stage_on_supply_air_setpoint_temperature");
                spmSZOSH->heatingOffSetPt = ip->getRealFieldValue(fields, props, "heating_stage_off_supply_air_setpoint_temperature");

                if (spmSZOSH->heatingOffSetPt > spmSZOSH->heatingOnSetPt) {
                    // throw warning, off must be warmer than on
                    ShowWarningError(state, format("{}: {}=\"{}\",", routineName, cCurrentModuleObject, spmSZOSH->Name));
                    ShowContinueError(state,
                                      format("...heating_stage_off_supply_air_setpoint_temperature=[{:.1R}] is less than "
                                             "heating_stage_on_supply_air_setpoint_temperature=[{:.1R}].",
                                             spmSZOSH->heatingOffSetPt,
                                             spmSZOSH->heatingOnSetPt));
                }

                std::string ctrlZoneName = ip->getAlphaFieldValue(fields, props, "control_zone_name");
                // get the actual zone number of the control zone
                spmSZOSH->ctrlZoneNum = Util::FindItemInList(ctrlZoneName, state.dataHeatBal->Zone);
                if (spmSZOSH->ctrlZoneNum == 0) {
                    ShowSevereItemNotFound(state, eoh, "control_zone_name", ctrlZoneName);
                    ErrorsFound = true;
                } else {
                    spmSZOSH->zoneNodeNum = GetSystemNodeNumberForZone(state, spmSZOSH->ctrlZoneNum);
                    if (allocated(state.dataZoneCtrls->StageZoneLogic)) {
                        if (!state.dataZoneCtrls->StageZoneLogic(spmSZOSH->ctrlZoneNum)) {
                            ShowSevereError(state, format("{}: {}=\"{}\", invalid field.", routineName, cCurrentModuleObject, spmSZOSH->Name));
                            ShowContinueError(state, format("..invalid control_zone_name=\"{}\".", ctrlZoneName));
                            ShowContinueError(state, "Zone thermostat must use ZoneControl:Thermostat:StagedDualSetpoint.");
                            ErrorsFound = true;
                        }
                    }
                }
            } break;

            // SetpointManager:ReturnTemperature:ChilledWater
            // SetpointManager:ReturnTemperature:HotWater
            case SPMType::ChilledWaterReturnTemp:
            case SPMType::HotWaterReturnTemp: {
                auto *spmRWT = dynamic_cast<SPMReturnWaterTemp *>(spm);
                assert(spmRWT != nullptr);

                bool errFlag = false;
                spmRWT->supplyNodeNum = GetOnlySingleNode(state,
                                                          ip->getAlphaFieldValue(fields, props, "plant_loop_supply_outlet_node"),
                                                          errFlag,
                                                          spmNodeObjectTypes[(int)spm->type],
                                                          spmRWT->Name,
                                                          DataLoopNode::NodeFluidType::Blank,
                                                          DataLoopNode::ConnectionType::SetPoint,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent,
                                                          "plant_loop_supply_outlet_node"); // setpoint nodes
                spmRWT->returnNodeNum = GetOnlySingleNode(state,
                                                          ip->getAlphaFieldValue(fields, props, "plant_loop_supply_inlet_node"),
                                                          errFlag,
                                                          spmNodeObjectTypes[(int)spm->type],
                                                          spmRWT->Name,
                                                          DataLoopNode::NodeFluidType::Blank,
                                                          DataLoopNode::ConnectionType::Sensor,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent,
                                                          "plant_loop_supply_inlet_node"); // setpoint nodes

                // process the setpoint inputs
                spmRWT->minSetTemp = ip->getRealFieldValue(fields, props, "minimum_supply_temperature_setpoint");
                spmRWT->maxSetTemp = ip->getRealFieldValue(fields, props, "maximum_supply_temperature_setpoint");

                spmRWT->returnTempType = static_cast<ReturnTempType>(
                    getEnumValue(returnTempTypeNamesUC, ip->getAlphaFieldValue(fields, props, "return_temperature_setpoint_input_type")));

                if (spmRWT->returnTempType == ReturnTempType::Scheduled) {
                    std::string schedName = ip->getAlphaFieldValue(fields, props, "return_temperature_setpoint_scheduled_name");
                    if ((spmRWT->returnTempSched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, "return_temperature_setpoint_scheduled_name", schedName);
                        ErrorsFound = true;
                    }
                } else if (spmRWT->returnTempType == ReturnTempType::Constant) {
                    spmRWT->returnTempConstantTarget = ip->getRealFieldValue(fields, props, "return_temperature_setpoint_constant_value");
                }
            } break;

            // SetpointManager:SystemNodeReset:Temperature
            case SPMType::SystemNodeTemp: {
                auto *spmSNRTemp = dynamic_cast<SPMSystemNode *>(spm);
                assert(spmSNRTemp != nullptr);

                if (spmSNRTemp->ctrlVar != HVAC::CtrlVarType::Temp && spmSNRTemp->ctrlVar != HVAC::CtrlVarType::MaxTemp &&
                    spmSNRTemp->ctrlVar != HVAC::CtrlVarType::MinTemp) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmSNRTemp->lowRefSetPt = ip->getRealFieldValue(fields, props, "setpoint_at_low_reference_temperature");
                spmSNRTemp->highRefSetPt = ip->getRealFieldValue(fields, props, "setpoint_at_high_reference_temperature");
                spmSNRTemp->lowRef = ip->getRealFieldValue(fields, props, "low_reference_temperature");
                spmSNRTemp->highRef = ip->getRealFieldValue(fields, props, "high_reference_temperature");

                spmSNRTemp->refNodeNum = GetOnlySingleNode(state,
                                                           ip->getAlphaFieldValue(fields, props, "reference_node_name"),
                                                           ErrorsFound,
                                                           spmNodeObjectTypes[(int)spm->type],
                                                           spmSNRTemp->Name,
                                                           DataLoopNode::NodeFluidType::Blank,
                                                           DataLoopNode::ConnectionType::Sensor,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent);
            } break;

            // SetpointManager:SystemNodeReset:Humidity
            case SPMType::SystemNodeHum: {
                auto *spmSNRHum = dynamic_cast<SPMSystemNode *>(spm);
                assert(spmSNRHum != nullptr);

                if (spmSNRHum->ctrlVar != HVAC::CtrlVarType::HumRat && spmSNRHum->ctrlVar != HVAC::CtrlVarType::MaxHumRat &&
                    spmSNRHum->ctrlVar != HVAC::CtrlVarType::MinHumRat) {
                    ShowSevereInvalidKey(state, eoh, "control_variable", ctrlVarName);
                    ErrorsFound = true;
                }

                spmSNRHum->lowRefSetPt = ip->getRealFieldValue(fields, props, "setpoint_at_low_reference_humidity_ratio");
                spmSNRHum->highRefSetPt = ip->getRealFieldValue(fields, props, "setpoint_at_high_reference_humidity_ratio");
                spmSNRHum->lowRef = ip->getRealFieldValue(fields, props, "low_reference_humidity_ratio");
                spmSNRHum->highRef = ip->getRealFieldValue(fields, props, "high_reference_humidity_ratio");

                spmSNRHum->refNodeNum = GetOnlySingleNode(state,
                                                          ip->getAlphaFieldValue(fields, props, "reference_node_name"),
                                                          ErrorsFound,
                                                          spmNodeObjectTypes[(int)spm->type],
                                                          spmSNRHum->Name,
                                                          DataLoopNode::NodeFluidType::Blank,
                                                          DataLoopNode::ConnectionType::Sensor,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent);
            } break;

                // SetpointManager:MultiZone:Cooling:Average
                // SetpointManager:MultiZone:Heating:Average
                // SetpointManager:MultiZone:MinimumHumidity:Average
                // SetpointManager:MultiZone:MaximumHumidity:Average
                // SetpointManager:MultiZone:Humidity:Minimum
                // SetpointManager:MultiZone:Humidity:Maximum

            default:
                break;
            } // switch (spm->type)

            // Load control node list
            // Do this at the end to preserve node order
            if (spm->type != SPMType::ReturnAirBypass && spm->type != SPMType::ChilledWaterReturnTemp && spm->type != SPMType::HotWaterReturnTemp) {
                std::string ctrlNodeListName = ip->getAlphaFieldValue(fields, props, "setpoint_node_or_nodelist_name");
                NodeListError = false;
                GetNodeNums(state,
                            ctrlNodeListName,
                            NumNodes,
                            NodeNums,
                            NodeListError,
                            DataLoopNode::NodeFluidType::Blank,
                            spmNodeObjectTypes[iSPM],
                            name,
                            DataLoopNode::ConnectionType::SetPoint,
                            NodeInputManager::CompFluidStream::Primary,
                            ObjectIsNotParent,
                            false,
                            "setpoint_node_or_nodelist_name");

                if (!NodeListError) {
                    for (int iNode = 1; iNode <= NumNodes; ++iNode) {
                        spm->ctrlNodeNums.push_back(NodeNums(iNode));
                    }
                } else {
                    ErrorsFound = true;
                }
            }

            // Now load all of the optional fields
            switch (spm->type) {
            case SPMType::MixedAir: {
                auto *spmMA = dynamic_cast<SPMMixedAir *>(spm);
                assert(spmMA != nullptr);
                if (auto found = fields.find("cooling_coil_inlet_node_name"); found != fields.end()) {
                    spmMA->coolCoilInNodeNum = GetOnlySingleNode(state,
                                                                 Util::makeUPPER(found.value().get<std::string>()),
                                                                 ErrorsFound,
                                                                 spmNodeObjectTypes[(int)spm->type],
                                                                 spmMA->Name,
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Sensor,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 ObjectIsNotParent);
                }

                if (auto found = fields.find("cooling_coil_outlet_node_name"); found != fields.end()) {
                    spmMA->coolCoilOutNodeNum = GetOnlySingleNode(state,
                                                                  Util::makeUPPER(found.value().get<std::string>()),
                                                                  ErrorsFound,
                                                                  spmNodeObjectTypes[(int)spm->type],
                                                                  spmMA->Name,
                                                                  DataLoopNode::NodeFluidType::Air,
                                                                  DataLoopNode::ConnectionType::Sensor,
                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                  ObjectIsNotParent);
                }

                if (auto found = fields.find("minimum_temperature_at_cooling_coil_outlet_node"); found != fields.end()) {
                    spmMA->minCoolCoilOutTemp = found.value().get<Real64>();
                }

                // Also, do this check now that we have both RefNodeNum and ctrlNodeNums
                if (std::find(spmMA->ctrlNodeNums.begin(), spmMA->ctrlNodeNums.end(), spmMA->refNodeNum) != spmMA->ctrlNodeNums.end()) {
                    ShowSevereError(state, format("{}: {}=\"{}\", reference node.", routineName, cCurrentModuleObject, spmMA->Name));
                    if (spmMA->ctrlNodeNums.size() > 1) {
                        ShowContinueError(state, "..Reference Node is the same as one of the nodes in SetPoint NodeList");
                    } else {
                        ShowContinueError(state, "..Reference Node is the same as the SetPoint Node");
                    }
                    ShowContinueError(state, format("Reference Node Name=\"{}\".", state.dataLoopNodes->NodeID(spmMA->refNodeNum)));
                    ErrorsFound = true;
                }
            } break;

            default:
                break;
            } // switch (spm->type)
        } // for (instance)
    } // for (iSPM)

} // GetSetPointManagerInputData()

void FillPredefinedTablesForSetPointManagers(EnergyPlusData &state)
{
    static constexpr std::string_view routineName = "FillPredefinedTablesForSetPointManagers";
    auto &orp = state.dataOutRptPredefined;
    for (auto *spm : state.dataSetPointManager->spms) {
        switch (spm->type) {
            // SetpointManager:OutdoorAirReset
        case SPMType::OutsideAir: {
            auto *spmOA = dynamic_cast<SPMOutsideAir *>(spm);
            assert(spmOA != nullptr);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArType, spmOA->Name, ctrlVarTypeNames[(int)spm->ctrlVar]);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArStLo1, spmOA->Name, spmOA->lowSetPt1);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArStHi1, spmOA->Name, spmOA->highSetPt1);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArOutLo1, spmOA->Name, spmOA->low1);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArOutHi1, spmOA->Name, spmOA->high1);
            if (spmOA->sched != nullptr) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArSchNm, spmOA->Name, spmOA->sched->Name);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArStLo2, spmOA->Name, spmOA->lowSetPt2);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArStHi2, spmOA->Name, spmOA->highSetPt2);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArOutLo2, spmOA->Name, spmOA->low2);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArOutHi2, spmOA->Name, spmOA->high2);
            }
            std::vector<std::string> namesOfNodes;
            std::vector<std::string> namesOfLoops;
            PlantLocation plantLoc;
            for (std::size_t i = 0; i < spm->ctrlNodeNums.size(); ++i) {
                namesOfNodes.push_back(state.dataLoopNodes->NodeID(spm->ctrlNodeNums[i]));
                int dummy = 0;
                PlantUtilities::ScanPlantLoopsForNodeNum(state, routineName, spm->ctrlNodeNums[i], plantLoc, dummy, false);
                if (plantLoc.loopNum > 0) {
                    namesOfLoops.push_back(plantLoc.loop->Name);
                }
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchSPMOArStPtNd, spmOA->Name, OutputReportTabular::stringJoinDelimiter(namesOfNodes, "; "));
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchSPMOArStPtLp, spmOA->Name, OutputReportTabular::stringJoinDelimiter(namesOfLoops, "; "));
            }
        } break;
        // SetpointManager:ReturnTemperature:ChilledWater
        // SetpointManager:ReturnTemperature:HotWater
        case SPMType::ChilledWaterReturnTemp:
        case SPMType::HotWaterReturnTemp: {
            auto *spmRWT = dynamic_cast<SPMReturnWaterTemp *>(spm);
            assert(spmRWT != nullptr);
            if (spm->type == SPMType::ChilledWaterReturnTemp) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetType, spmRWT->Name, "Chilled Water");
            } else {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetType, spmRWT->Name, "Hot Water");
            }
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetMinT, spmRWT->Name, spmRWT->minSetTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetMaxT, spmRWT->Name, spmRWT->maxSetTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetRetT, spmRWT->Name, spmRWT->returnTempConstantTarget);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetRetType, spmRWT->Name, returnTempTypeNames[(int)spmRWT->returnTempType]);
            if (spmRWT->returnTempSched != nullptr) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMOArSchNm, spmRWT->Name, spmRWT->returnTempSched->Name);
            }
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetOutNd, spmRWT->Name, state.dataLoopNodes->NodeID(spmRWT->supplyNodeNum));
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchSPMRetInNd, spmRWT->Name, state.dataLoopNodes->NodeID(spmRWT->returnNodeNum));
            if (spmRWT->plantLoopNum != 0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchSPMRetPltLp, spmRWT->Name, state.dataPlnt->PlantLoop(spmRWT->plantLoopNum).Name);
            }
        } break;
        default:
            break;
        }
    }
}

void VerifySetPointManagers(EnergyPlusData &state, [[maybe_unused]] bool &ErrorsFound) // flag to denote node conflicts in input. !unused1208
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   July 2008
    //       MODIFIED       Rick Strand, Aug 2014 (removed deallocation of AllSetPtMgrs so ScheduledTES could also verify control nodes)

    // PURPOSE OF THIS SUBROUTINE
    // Check the SetPointManager data to eliminate conflicts.

    // METHODOLOGY EMPLOYED:
    // 1) Check for duplicate names in individual setpoint managers.
    // Control nodes = A B C D
    // Check A with B, C, and D
    // Check B with C and D
    // Check C with D
    // 2) Check for duplicate names in all other setpoint managers
    //    Verify setpoint managers use same control type (e.g. TEMP) and then check for duplicate nodes
    // SPM 1 - Control nodes A - D, SPM 2 - Control nodes E - H, SPM 3 - Control nodes I - L
    // If SPM 1 has same control type as SPM 2 and SPM 3 (e.g. all use SPM%CtrlTypeMode = HVAC::CtrlVarType::Temp) then:
    // Check A with E-H and I-L
    // Check B with E-H and I-L
    // Check C with E-H and I-L
    // Check D with E-H and I-L
    // Then check SPM 2 nodes with SPM 3. Check E with I-L, F with I-L, etc.
    // 3) For SET POINT MANAGER:RETURN AIR BYPASS FLOW
    //    check for duplicate air loop names.

    for (int iSPM = 1; iSPM <= state.dataSetPointManager->spms.isize(); ++iSPM) {
        auto const *spm = state.dataSetPointManager->spms(iSPM);

        // check for duplicate nodes in each setpoint managers control node list (node lists of size 1 do not need verification)
        // issue warning only since duplicate node names within a setpoint manager does not cause a conflict (i.e., same
        // value written to node) but may indicate an error in the node name.
        for (int iNode = 0; iNode < (int)spm->ctrlNodeNums.size() - 1; ++iNode) {
            for (int jNode = iNode + 1; jNode < (int)spm->ctrlNodeNums.size(); ++jNode) {
                if (spm->ctrlNodeNums[iNode] != spm->ctrlNodeNums[jNode]) {
                    continue;
                }
                ShowWarningError(state, format("{} =\"{}\"", spmTypeNames[(int)spm->type], spm->Name));
                ShowContinueError(state, format("...duplicate node specified = {}", state.dataLoopNodes->NodeID(spm->ctrlNodeNums[iNode])));
                ShowContinueError(state, format("...control type variable    = {}", ctrlVarTypeNamesUC[(int)spm->ctrlVar]));
            }
        }

        // check for node conflicts in all other setpoint managers
        for (int jSPM = iSPM + 1; jSPM <= (int)state.dataSetPointManager->spms.size(); ++jSPM) {
            auto const *spm2 = state.dataSetPointManager->spms(jSPM);

            if (spm == spm2) {
                continue;
            }

            if (spm->type == SPMType::ReturnAirBypass && spm2->type == SPMType::ReturnAirBypass) {

                //     check the air loop name for duplicates in this SP manager type
                if (spm->airLoopNum == spm2->airLoopNum) {
                    ShowWarningError(state, format("{}=\"{}\"", spmTypeNames[(int)spm->type], spm->Name));
                    ShowContinueError(state, "...air loop name conflicts with another setpoint manager.");
                    ShowContinueError(state, format("...conflicting setpoint manager = {} \"{}\"", spmTypeNames[(int)spm2->type], spm2->Name));
                    ShowContinueError(state, format("...conflicting air loop name = {}", spm->airLoopName));
                    //        ErrorsFound=.TRUE.
                }

                //     check for duplicate control nodes
                if (spm->ctrlVar != spm2->ctrlVar) {
                    continue;
                }

                for (int iNode = 0; iNode < (int)spm->ctrlNodeNums.size(); ++iNode) {
                    for (int jNode = 0; jNode < (int)spm2->ctrlNodeNums.size(); ++jNode) {
                        if ((spm->ctrlNodeNums[iNode] == spm2->ctrlNodeNums[jNode]) && spm->ctrlNodeNums[iNode] != 0) {
                            ShowWarningError(state, format("{}=\"{}\"", spmTypeNames[(int)spm->type], spm->Name));
                            ShowContinueError(state, "...setpoint node conflicts with another setpoint manager.");
                            ShowContinueError(state,
                                              format("...conflicting setpoint manager = {} \"{}\"", spmTypeNames[(int)spm2->type], spm2->Name));
                            ShowContinueError(state, format("...conflicting node name = {}", state.dataLoopNodes->NodeID(spm->ctrlNodeNums[iNode])));
                            ShowContinueError(state, format("...control type variable = {}", ctrlVarTypeNames[(int)spm->ctrlVar]));
                            //            ErrorsFound=.TRUE.
                        }
                    }
                }

            } else { // not a RAB setpoint manager

                //     check just the control nodes for other types of SP managers
                if (spm->ctrlVar != spm2->ctrlVar) {
                    continue;
                }

                for (int iNode = 0; iNode < (int)spm->ctrlNodeNums.size(); ++iNode) {
                    for (int jNode = 0; jNode < (int)spm2->ctrlNodeNums.size(); ++jNode) {

                        if (spm->ctrlNodeNums[iNode] != spm2->ctrlNodeNums[jNode]) {
                            continue;
                        }

                        //         only warn if scheduled setpoint manager is setting mass flow rate on the same node used by RAB
                        if (spm->type == SPMType::ReturnAirBypass || spm2->type == SPMType::ReturnAirBypass) {
                            ShowWarningError(state, format("{}=\"{}\"", spmTypeNames[(int)spm->type], spm->Name));
                            ShowContinueError(state, "...setpoint node conflicts with another setpoint manager.");
                            ShowContinueError(state, format("...conflicting setpoint manager ={}:\"{}\"", spmTypeNames[(int)spm2->type], spm2->Name));
                            ShowContinueError(state, format("...conflicting node name = {}", state.dataLoopNodes->NodeID(spm->ctrlNodeNums[iNode])));
                            ShowContinueError(state, format("...control type variable = {}", ctrlVarTypeNames[(int)spm->ctrlVar]));
                            ShowContinueError(state,
                                              "...return air bypass flow setpoint manager will have priority setting mass flow rate on this node.");
                        } else { // severe error for other SP manager types
                            ShowWarningError(state, format("{}=\"{}\"", spmTypeNames[(int)spm->type], spm->Name));
                            ShowContinueError(state, "...setpoint node conflicts with another setpoint manager.");
                            ShowContinueError(state,
                                              format("...conflicting setpoint manager = {}:\"{}\"", spmTypeNames[(int)spm2->type], spm2->Name));
                            ShowContinueError(state, format("...conflicting node name = {}", state.dataLoopNodes->NodeID(spm->ctrlNodeNums[iNode])));
                            ShowContinueError(state, format("...control type variable = {}", ctrlVarTypeNames[(int)spm->ctrlVar]));
                            //            ErrorsFound=.TRUE.
                        }
                    }
                }
            }

        } // for (jSPM)

    } // for (iSPM)

    // Removed the following line for ScheduledTES control implementation
    // if ( allocated( AllSetPtMgr ) ) AllSetPtMgr.deallocate();
} // VerifySetPointManager()

void InitSetPointManagers(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   October 2000
    //       MODIFIED       Shirey/Raustad (FSEC), Jan 2004
    //                      Nov 2004 - Jan 2005 M. J. Witte, GARD Analytics, Inc.
    //                        Add new setpoint managers:
    //                          SET POINT MANAGER:SINGLE ZONE HEATING and
    //                          SET POINT MANAGER:SINGLE ZONE COOLING
    //                          SET POINT MANAGER:OUTSIDE AIR PRETREAT
    //                        Work supported by ASHRAE research project 1254-RP
    //                      Haves Oct 2004
    //                      July 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Heating:Average
    //                          SetpointManager:MultiZone:Cooling:Average
    //                          SetpointManager:MultiZone:MinimumHumidity:Average
    //                          SetpointManager:MultiZone:MaximumHumidity:Average
    //                      Aug 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Humidity:Minimum
    //                          SetpointManager:MultiZone:Humidity:Maximum
    //                      Sep 2010 B.A. Nigusse, FSEC/UCF
    //                         Added control variables for SetpointManage:Scheduled
    //                      Jan 2022 Wooyoung Jung, Jeremy Lerond and Jian Zhang, PNNL
    //                         Added new setpoint managers:
    //                          SetpointManager:SystemNodeReset:Temperature
    //                          SetpointManager:SystemNodeReset:Humidity

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the Setpoint Manager objects.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // Using/Aliasing
    using namespace DataPlant;
    using OutAirNodeManager::CheckOutAirNodeNumber;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    constexpr std::string_view routineName = "InitSetPointManagers";

    bool ErrorsFound(false);

    state.dataSetPointManager->ManagerOn = true;

    // One time initializations

    if (state.dataZoneEquip->ZoneEquipInputsFilled &&
        state.dataAirLoop->AirLoopInputsFilled) { // check that the zone equipment and air loop data has been read in

        if (state.dataSetPointManager->InitSetPointManagersOneTimeFlag) {

            for (auto *spm : state.dataSetPointManager->spms) {
                std::string_view spmName = spm->Name;
                std::string_view spmTypeName = spmTypeNames[(int)spm->type];

                ErrorObjectHeader eoh{routineName, spmTypeName, spmName};

                switch (spm->type) {

                case SPMType::SZHeating:
                case SPMType::SZCooling: {
                    auto *spmSZT = dynamic_cast<SPMSingleZoneTemp *>(spm);
                    assert(spmSZT != nullptr);
                    // find the index in the ZoneEquipConfig array of the control zone (the one with the main or only thermostat)
                    int ConZoneNum = 0;
                    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode == spmSZT->zoneNodeNum) {
                            ConZoneNum = ZoneNum;
                        }
                    }
                    if (ConZoneNum == 0) {
                        ShowSevereError(state, format("{}=\"{}\", Zone Node not found:", spmTypeName, spmName));
                        ShowContinueError(state,
                                          format("Node=\"{}\", not found in any controlled Zone", state.dataLoopNodes->NodeID(spmSZT->zoneNodeNum)));
                        ErrorsFound = true;
                    } else {
                        auto &zoneEquip = state.dataZoneEquip->ZoneEquipConfig(ConZoneNum);
                        bool found = false;
                        for (int zoneInNode = 1; zoneInNode <= zoneEquip.NumInletNodes; ++zoneInNode) {
                            if (spmSZT->zoneInletNodeNum == zoneEquip.InletNode(zoneInNode)) {
                                found = true;
                            }
                        }
                        if (!found) {
                            ShowSevereError(state,
                                            format("{}=\"{}\", The zone inlet node of {}",
                                                   spmTypeName,
                                                   spmName,
                                                   state.dataLoopNodes->NodeID(spmSZT->zoneInletNodeNum)));
                            ShowContinueError(state, format("is not found in Zone = {}. Please check inputs.", zoneEquip.ZoneName));
                            ErrorsFound = true;
                        }
                    }
                } break;

                case SPMType::SZMinHum:
                case SPMType::SZMaxHum: {
                    auto *spmSZH = dynamic_cast<SPMSingleZoneHum *>(spm);
                    assert(spmSZH != nullptr);

                    // set the actual and controlled zone numbers
                    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode == spmSZH->zoneNodeNum) {
                            spmSZH->ctrlZoneNum = ZoneNum;
                            break;
                        }
                    }
                    // still need to validate...
                    if (spmSZH->ctrlZoneNum == 0) { // didn't find
                        ShowSevereCustom(state, eoh, format("could not find Controlled Zone={}", state.dataHeatBal->Zone(spmSZH->ctrlZoneNum).Name));
                        ErrorsFound = true;
                    } else {
                        // make sure humidity controlled zone
                        bool HstatZoneFound = false;
                        for (int iZone = 1; iZone <= state.dataZoneCtrls->NumHumidityControlZones; ++iZone) {
                            if (state.dataZoneCtrls->HumidityControlZone(iZone).ActualZoneNum == spmSZH->ctrlZoneNum) {
                                HstatZoneFound = true;
                                break;
                            }
                        }
                        if (!HstatZoneFound) {
                            ShowSevereError(state, format("{}=\"{}\", invalid humidistat specification", spmTypeName, spmName));
                            ShowContinueError(state,
                                              format("could not locate Humidistat in Zone={}", state.dataHeatBal->Zone(spmSZH->ctrlZoneNum).Name));
                            ErrorsFound = true;
                        }
                    }
                } break;

                case SPMType::SZReheat: {
                    auto *spmSZR = dynamic_cast<SPMSingleZoneReheat *>(spm);
                    assert(spmSZR != nullptr);

                    int FanNodeIn = 0;
                    int FanNodeOut = 0;
                    int MixedAirNode = 0;
                    int InletBranchNum = 0;
                    int LoopInNode = 0;

                    // find the index in the ZoneEquipConfig array of the control zone (the one with the main or only thermostat)
                    int ConZoneNum = 0;
                    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode == spmSZR->zoneNodeNum) {
                            ConZoneNum = ZoneNum;
                        }
                    }

                    if (ConZoneNum == 0) {
                        ShowSevereError(state, format("{}=\"{}\", Zone Node not found:", spmTypeName, spmName));
                        ShowContinueError(state,
                                          format("Node=\"{}\", not found in any controlled Zone", state.dataLoopNodes->NodeID(spmSZR->zoneNodeNum)));
                        ErrorsFound = true;
                    } else {
                        bool found = false;
                        auto const &zoneEquip = state.dataZoneEquip->ZoneEquipConfig(ConZoneNum);
                        for (int zoneInNode = 1; zoneInNode <= zoneEquip.NumInletNodes; ++zoneInNode) {
                            if (spmSZR->zoneInletNodeNum == zoneEquip.InletNode(zoneInNode)) {
                                spmSZR->airLoopNum = zoneEquip.InletNodeAirLoopNum(zoneInNode);
                                found = true;
                            }
                        }
                        if (!found) {
                            ShowSevereError(state,
                                            format("{}=\"{}\", The zone inlet node of {}",
                                                   spmTypeName,
                                                   spmName,
                                                   state.dataLoopNodes->NodeID(spmSZR->zoneInletNodeNum)));
                            ShowContinueError(state, format("is not found in Zone = {}. Please check inputs.", zoneEquip.ZoneName));
                            ErrorsFound = true;
                        }
                        if (spmSZR->airLoopNum == 0) {
                            ShowSevereError(state, format("{}=\"{}\", The zone inlet node is not connected to an air loop.", spmTypeName, spmName));
                            ErrorsFound = true;
                            continue;
                        }
                    }

                    auto const &primaryAirSystem = state.dataAirSystemsData->PrimaryAirSystems(spmSZR->airLoopNum);
                    MixedAirNode = primaryAirSystem.OASysOutletNodeNum;
                    InletBranchNum = primaryAirSystem.InletBranchNum[0];
                    LoopInNode = primaryAirSystem.Branch(InletBranchNum).NodeNumIn;
                    // get the supply fan inlet and outlet nodes
                    if (MixedAirNode > 0) {
                        bool LookForFan = false;
                        for (auto const &branch : primaryAirSystem.Branch) {
                            for (auto const &comp : branch.Comp) {
                                if (MixedAirNode == comp.NodeNumIn) {
                                    LookForFan = true;
                                }
                                if (LookForFan) {
                                    if (Util::SameString(comp.TypeOf, "Fan:ConstantVolume") || Util::SameString(comp.TypeOf, "Fan:VariableVolume") ||
                                        Util::SameString(comp.TypeOf, "Fan:OnOff") || Util::SameString(comp.TypeOf, "Fan:ComponentModel")) {
                                        FanNodeIn = comp.NodeNumIn;
                                        FanNodeOut = comp.NodeNumOut;
                                        break;
                                    }
                                }
                            }
                        } // for (BranchNum)
                    } else {
                        for (auto const &branch : primaryAirSystem.Branch) {
                            for (auto const &comp : branch.Comp) {
                                if (Util::SameString(comp.TypeOf, "Fan:ConstantVolume") || Util::SameString(comp.TypeOf, "Fan:VariableVolume") ||
                                    Util::SameString(comp.TypeOf, "Fan:OnOff") || Util::SameString(comp.TypeOf, "Fan:ComponentModel")) {
                                    FanNodeIn = comp.NodeNumIn;
                                    FanNodeOut = comp.NodeNumOut;
                                }
                            }
                        }
                    }

                    spmSZR->fanInNodeNum = FanNodeIn;
                    spmSZR->fanOutNodeNum = FanNodeOut;
                    spmSZR->mixedAirNodeNum = MixedAirNode;
                    spmSZR->oaInNodeNum = primaryAirSystem.OAMixOAInNodeNum;
                    // this next line assumes that OA system is the first thing on the branch, what if there is a relief fan or heat recovery coil
                    // or other component in there first? does it matter?
                    spmSZR->retNodeNum = primaryAirSystem.OASysInletNodeNum;
                    spmSZR->loopInNodeNum = LoopInNode;

                } break;

                case SPMType::Warmest:
                case SPMType::Coldest: {
                    auto *spmT = dynamic_cast<SPMTempest *>(spm);
                    assert(spmT != nullptr);
                    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
                        spmT->airLoopNum =
                            Util::FindItemInList(spmT->airLoopName, state.dataAirLoop->AirToZoneNodeInfo, &AirLoopZoneEquipConnectData::AirLoopName);
                        if (spmT->airLoopNum == 0) {
                            ShowSevereItemNotFound(state, eoh, "hvac_air_loop_name", spmT->airLoopName);
                            ErrorsFound = true;
                        } else if (state.dataAirLoop->AirToZoneNodeInfo(spmT->airLoopNum).NumZonesCooled == 0) {
                            ShowSevereError(state, format("{}=\"{}\", no zones with cooling found:", spmTypeName, spmName));
                            ShowContinueError(state, format("Air Loop provides no cooling, Air Loop=\"{}\".", spmT->airLoopName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{}=\"{}\", no AirLoopHVAC objects found:", spmTypeName, spmName));
                        ShowContinueError(state, "Setpoint Manager needs an AirLoopHVAC to operate.");
                        ErrorsFound = true;
                    }
                } break;

                case SPMType::WarmestTempFlow: {
                    auto *spmWTF = dynamic_cast<SPMWarmestTempFlow *>(spm);
                    assert(spmWTF != nullptr);

                    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
                        spmWTF->airLoopNum = Util::FindItemInList(
                            spmWTF->airLoopName, state.dataAirLoop->AirToZoneNodeInfo, &AirLoopZoneEquipConnectData::AirLoopName);
                        if (spmWTF->airLoopNum == 0) {
                            ShowSevereItemNotFound(state, eoh, "hvac_air_loop_name", spmWTF->airLoopName);
                            ErrorsFound = true;
                        } else {
                            spmWTF->simReady = true;
                        }
                        if (state.dataAirLoop->AirToZoneNodeInfo(spmWTF->airLoopNum).NumZonesCooled == 0) {
                            ShowSevereError(state, format("{}=\"{}\", no zones with cooling found:", spmTypeName, spmName));
                            ShowContinueError(state, format("Air Loop provides no cooling, Air Loop=\"{}\".", spmWTF->airLoopName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{}=\"{}\", no AirLoopHVAC objects found:", spmTypeName, spmName));
                        ShowContinueError(state, "Setpoint Manager needs an AirLoopHVAC to operate.");
                        ErrorsFound = true;
                    }
                } break;

                case SPMType::ReturnAirBypass: {
                    auto *spmRAB = dynamic_cast<SPMReturnAirBypassFlow *>(spm);
                    assert(spmRAB != nullptr);

                    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
                        spmRAB->airLoopNum = Util::FindItemInList(
                            spmRAB->airLoopName, state.dataAirLoop->AirToZoneNodeInfo, &AirLoopZoneEquipConnectData::AirLoopName);
                        if (spmRAB->airLoopNum == 0) {
                            ShowSevereItemNotFound(state, eoh, "hvac_air_loop_name", spmRAB->airLoopName);
                            ErrorsFound = true;
                        }

                        auto const &primaryAirSystem = state.dataAirSystemsData->PrimaryAirSystems(spmRAB->airLoopNum);
                        if (primaryAirSystem.RABExists) {
                            spmRAB->rabMixInNodeNum = primaryAirSystem.RABMixInNode;
                            spmRAB->supMixInNodeNum = primaryAirSystem.SupMixInNode;
                            spmRAB->mixOutNodeNum = primaryAirSystem.MixOutNode;
                            spmRAB->rabSplitOutNodeNum = primaryAirSystem.RABSplitOutNode;
                            spmRAB->sysOutNodeNum = state.dataAirLoop->AirToZoneNodeInfo(spmRAB->airLoopNum).AirLoopSupplyNodeNum(1);
                            spmRAB->ctrlNodeNums.push_back(spmRAB->rabSplitOutNodeNum);
                        } else {
                            ShowSevereError(state, format("{}=\"{}\", no RAB in air loop found:", spmTypeName, spmName));
                            ShowContinueError(state, format("Air Loop=\"{}\".", spmRAB->airLoopName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{}=\"{}\", no AirLoopHVAC objects found:", spmTypeName, spmName));
                        ShowContinueError(state, "Setpoint Manager needs an AirLoopHVAC to operate.");
                        ErrorsFound = true;
                    }
                } break;

                case SPMType::MZCoolingAverage:
                case SPMType::MZHeatingAverage: {
                    auto *spmMZTemp = dynamic_cast<SPMMultiZoneTemp *>(spm);
                    assert(spmMZTemp != nullptr);

                    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
                        spmMZTemp->airLoopNum = Util::FindItemInList(
                            spmMZTemp->airLoopName, state.dataAirLoop->AirToZoneNodeInfo, &AirLoopZoneEquipConnectData::AirLoopName);
                        if (spmMZTemp->airLoopNum == 0) {
                            ShowSevereItemNotFound(state, eoh, "hvac_air_loop_name", spmMZTemp->airLoopName);
                            ErrorsFound = true;
                        }

                        if (state.dataAirLoop->AirToZoneNodeInfo(spmMZTemp->airLoopNum).NumZonesCooled == 0) {
                            ShowSevereError(state, format("{}=\"{}\", no zones with cooling found:", spmTypeName, spmName));
                            ShowContinueError(state, format("Air Loop provides no cooling, Air Loop=\"{}\".", spmMZTemp->airLoopName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{}=\"{}\", no AirLoopHVAC objects found:", spmTypeName, spmName));
                        ShowContinueError(state, "Setpoint Manager needs an AirLoopHVAC to operate.");
                        ErrorsFound = true;
                    }
                } break;

                case SPMType::MZMinHumAverage:
                case SPMType::MZMaxHumAverage:
                case SPMType::MZMinHum:
                case SPMType::MZMaxHum: {
                    auto *spmMZHum = dynamic_cast<SPMMultiZoneHum *>(spm);
                    assert(spmMZHum != nullptr);

                    if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
                        spmMZHum->airLoopNum = Util::FindItemInList(
                            spmMZHum->airLoopName, state.dataAirLoop->AirToZoneNodeInfo, &AirLoopZoneEquipConnectData::AirLoopName);
                        if (spmMZHum->airLoopNum == 0) {
                            ShowSevereItemNotFound(state, eoh, "hvac_air_loop_name", spmMZHum->airLoopName);
                            ErrorsFound = true;
                        } else {
                            // make sure humidity controlled zone
                            auto const &primaryAirSystem = state.dataAirSystemsData->PrimaryAirSystems(spmMZHum->airLoopNum);
                            auto const &airToZoneNode = state.dataAirLoop->AirToZoneNodeInfo(spmMZHum->airLoopNum);
                            bool HstatZoneFound = false;
                            for (int iZone = 1; iZone <= state.dataZoneCtrls->NumHumidityControlZones; ++iZone) {
                                for (int jZone = 1; jZone <= airToZoneNode.NumZonesCooled; ++jZone) {
                                    if (state.dataZoneCtrls->HumidityControlZone(iZone).ActualZoneNum == airToZoneNode.CoolCtrlZoneNums(jZone)) {
                                        HstatZoneFound = true;
                                        break;
                                    }
                                }
                            }

                            if (!HstatZoneFound) {
                                ShowSevereError(state, format("{}=\"{}\", invalid humidistat specification", spmTypeName, spmName));
                                ShowContinueError(
                                    state,
                                    format("could not locate Humidistat in any of the zones served by the Air loop={}", primaryAirSystem.Name));
                                ErrorsFound = true;
                            }
                        }
                    } else {
                        ShowSevereError(state, format("{}=\"{}\", no AirLoopHVAC objects found:", spmTypeName, spmName));
                        ShowContinueError(state, "Setpoint Manager needs an AirLoopHVAC to operate.");
                        ErrorsFound = true;
                    }
                } break;

                case SPMType::CondenserEnteringTemp: {
                    auto *spmCET = dynamic_cast<SPMCondenserEnteringTemp *>(spm);
                    assert(spmCET != nullptr);

                    // Scan loops and find the loop index that includes the condenser cooling tower node used as setpoint
                    // Begin demand side loops ... When condenser is added becomes NumLoops
                    for (int LoopNum = 1; LoopNum <= state.dataHVACGlobal->NumCondLoops + state.dataHVACGlobal->NumPlantLoops; ++LoopNum) {
                        auto &plantLoop = state.dataPlnt->PlantLoop(LoopNum);
                        for (int ctrlNodeNum : spmCET->ctrlNodeNums) {
                            if (plantLoop.TempSetPointNodeNum != ctrlNodeNum) {
                                continue;
                            }

                            for (auto const &branch : plantLoop.LoopSide(LoopSideLocation::Supply).Branch) {
                                for (auto const &comp : branch.Comp) {
                                    if (comp.Type == PlantEquipmentType::CoolingTower_SingleSpd) {
                                        ShowSevereError(state, format("{}=\"{}\", invalid tower found", spmTypeName, spmName));
                                        ShowContinueError(state, format("Found SingleSpeed Cooling Tower, Cooling Tower={}", comp.Name));
                                        ShowContinueError(state, "SingleSpeed cooling towers cannot be used with this setpoint manager.");
                                        ErrorsFound = true;
                                    }
                                }
                            }

                            // Scan all attached chillers in the condenser loop index found to find the chiller index
                            for (int BranchNum = 1; BranchNum <= plantLoop.LoopSide(LoopSideLocation::Demand).TotalBranches; ++BranchNum) {
                                auto &branch = plantLoop.LoopSide(LoopSideLocation::Demand).Branch(BranchNum);

                                for (int CompNum = 1; CompNum <= branch.TotalComponents; ++CompNum) {
                                    auto const &comp = branch.Comp(CompNum);
                                    switch (comp.Type) {

                                    case PlantEquipmentType::Chiller_Absorption:
                                    case PlantEquipmentType::Chiller_Indirect_Absorption:
                                    case PlantEquipmentType::Chiller_CombTurbine:
                                    case PlantEquipmentType::Chiller_ConstCOP:
                                    case PlantEquipmentType::Chiller_Electric:
                                    case PlantEquipmentType::Chiller_ElectricEIR:
                                    case PlantEquipmentType::Chiller_DFAbsorption:
                                    case PlantEquipmentType::Chiller_ElectricReformEIR:
                                    case PlantEquipmentType::Chiller_EngineDriven: {
                                        // Scan the supply side to find the chiller index and branch index on plantloop
                                        DataPlant::PlantEquipmentType ChillerType = comp.Type;
                                        for (int LoopNum2 = 1; LoopNum2 <= state.dataHVACGlobal->NumCondLoops + state.dataHVACGlobal->NumPlantLoops;
                                             ++LoopNum2) {
                                            auto &plantLoop2 = state.dataPlnt->PlantLoop(LoopNum2);
                                            auto &loopSide2 = plantLoop2.LoopSide(LoopSideLocation::Supply);
                                            for (int BranchNum2 = 1; BranchNum2 <= loopSide2.TotalBranches; ++BranchNum2) {
                                                auto const &branch2 = loopSide2.Branch(BranchNum2);

                                                for (int CompNum2 = 1; CompNum2 <= branch2.TotalComponents; ++CompNum2) {
                                                    auto const &comp2 = branch2.Comp(CompNum2);
                                                    if (comp2.Type == ChillerType) {
                                                        spmCET->plantPloc = {LoopNum2, LoopSideLocation::Supply, BranchNum2, CompNum2};
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        spmCET->chillerType = ChillerType;
                                        spmCET->demandPloc = {LoopNum, LoopSideLocation::Demand, BranchNum, CompNum};
                                    } break;

                                    default:
                                        break;
                                    }
                                } // for (comp)
                            } // for (branch)
                        } // if (
                    } // for (LoopNum)
                } break;

                case SPMType::IdealCondenserEnteringTemp: {
                    auto *spmIdealCET = dynamic_cast<SPMIdealCondenserEnteringTemp *>(spm);
                    assert(spmIdealCET != nullptr);

                    PlantEquipmentType InitType = PlantEquipmentType::Invalid;
                    int NumChiller = 0;

                    // Scan loops and find the loop index that includes the condenser cooling tower node used as setpoint
                    // Begin demand side loops ... When condenser is added becomes NumLoops
                    for (int LoopNum = 1; LoopNum <= state.dataHVACGlobal->NumCondLoops + state.dataHVACGlobal->NumPlantLoops; ++LoopNum) {
                        auto &plantLoop = state.dataPlnt->PlantLoop(LoopNum);
                        auto &supplySide = plantLoop.LoopSide(LoopSideLocation::Supply);
                        for (int ctrlNodeNum : spmIdealCET->ctrlNodeNums) {
                            if (plantLoop.TempSetPointNodeNum != ctrlNodeNum) {
                                continue;
                            }

                            for (int BranchNum = 1; BranchNum <= supplySide.TotalBranches; ++BranchNum) {
                                auto &branch = supplySide.Branch(BranchNum);
                                for (int CompNum = 1; CompNum <= branch.TotalComponents; ++CompNum) {
                                    auto &comp = branch.Comp(CompNum);
                                    // Check if cooling tower is single speed and generate and error
                                    InitType = comp.Type;
                                    if (InitType == PlantEquipmentType::CoolingTower_SingleSpd) {
                                        ShowSevereError(state, format("{}=\"{}\", invalid cooling tower found", spmTypeName, spmName));
                                        ShowContinueError(state, format("Found Single Speed Cooling Tower, Cooling Tower={}", comp.Name));
                                        ShowContinueError(state, "SingleSpeed cooling towers cannot be used with this setpoint manager on each loop");
                                        ErrorsFound = true;
                                    } else if (InitType == PlantEquipmentType::CoolingTower_TwoSpd ||
                                               InitType == PlantEquipmentType::CoolingTower_VarSpd) {
                                        spmIdealCET->towerPlocs.push_back(PlantLocation(LoopNum, LoopSideLocation::Supply, BranchNum, CompNum));
                                        spmIdealCET->numTowers++;
                                    }
                                    // Scan the pump on the condenser water loop
                                    if (InitType == PlantEquipmentType::PumpVariableSpeed || InitType == PlantEquipmentType::PumpConstantSpeed) {
                                        spmIdealCET->condenserPumpPloc = {LoopNum, LoopSideLocation::Supply, BranchNum, CompNum};
                                    }
                                }
                            }

                            auto &demandSide = plantLoop.LoopSide(LoopSideLocation::Demand);
                            // Scan all attached chillers in the condenser loop index found to find the chiller index
                            for (int BranchNum = 1; BranchNum <= demandSide.TotalBranches; ++BranchNum) {
                                auto &branch = demandSide.Branch(BranchNum);
                                for (int CompNum = 1; CompNum <= branch.TotalComponents; ++CompNum) {
                                    auto &comp = branch.Comp(CompNum);
                                    InitType = comp.Type;

                                    switch (InitType) {
                                    case PlantEquipmentType::Chiller_Absorption:
                                    case PlantEquipmentType::Chiller_Indirect_Absorption:
                                    case PlantEquipmentType::Chiller_CombTurbine:
                                    case PlantEquipmentType::Chiller_ConstCOP:
                                    case PlantEquipmentType::Chiller_Electric:
                                    case PlantEquipmentType::Chiller_ElectricEIR:
                                    case PlantEquipmentType::Chiller_DFAbsorption:
                                    case PlantEquipmentType::Chiller_ElectricReformEIR:
                                    case PlantEquipmentType::Chiller_EngineDriven: {
                                        // Scan the supply side to find the chiller index and branch index on plantloop
                                        DataPlant::PlantEquipmentType ChillerType = comp.Type;
                                        for (int LoopNum2 = 1; LoopNum2 <= state.dataHVACGlobal->NumCondLoops + state.dataHVACGlobal->NumPlantLoops;
                                             ++LoopNum2) {
                                            auto &plantLoop2 = state.dataPlnt->PlantLoop(LoopNum2);
                                            auto &supplySide2 = plantLoop2.LoopSide(LoopSideLocation::Supply);
                                            for (int BranchNum2 = 1; BranchNum2 <= supplySide2.TotalBranches; ++BranchNum2) {
                                                auto &branch2 = supplySide2.Branch(BranchNum2);
                                                for (int CompNum2 = 1; CompNum2 <= branch2.TotalComponents; ++CompNum2) {
                                                    auto const &comp2 = branch2.Comp(CompNum2);
                                                    InitType = comp2.Type;
                                                    if (InitType == ChillerType) {
                                                        ++NumChiller;
                                                        spmIdealCET->chillerPloc = {LoopNum2, LoopSideLocation::Supply, BranchNum2, CompNum2};
                                                        // Scan the pump on the chilled water loop
                                                        for (int BranchNum3 = 1; BranchNum3 <= supplySide2.TotalBranches; ++BranchNum3) {
                                                            auto &branch3 = supplySide2.Branch(BranchNum3);
                                                            for (int CompNum3 = 1; CompNum3 <= branch3.TotalComponents; ++CompNum3) {
                                                                auto const &comp3 = branch3.Comp(CompNum3);
                                                                InitType = comp3.Type;
                                                                if (InitType == PlantEquipmentType::PumpVariableSpeed ||
                                                                    InitType == PlantEquipmentType::PumpConstantSpeed) {
                                                                    spmIdealCET->chilledWaterPumpPloc = {
                                                                        LoopNum2, LoopSideLocation::Supply, BranchNum3, CompNum3};
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (NumChiller > 1) {
                                            ShowSevereError(state, format("{}=\"{}\", too many chillers found", spmTypeName, spmName));
                                            ShowContinueError(state, "only one chiller can be used with this setpoint manager on each loop");
                                            ShowContinueError(state, format("Found more than one chiller, chiller ={}", comp.Name));
                                            ErrorsFound = true;
                                        }
                                        spmIdealCET->chillerType = ChillerType;
                                        spmIdealCET->condenserPumpPloc.loopNum = LoopNum;
                                    } break;

                                    default:
                                        break;
                                    } // switch (InitType)
                                } // for (CompNum)
                            } // for (BranchNum)
                            NumChiller = 0;
                        } // for (iNode)
                    } // for (LoopNum)
                } break;

                default:
                    break;
                } // switch (spm->type)
            } // for (spm)

            VerifySetPointManagers(state, ErrorsFound);

            state.dataSetPointManager->InitSetPointManagersOneTimeFlag = false;
        } // if (InitSetPointManagersOneTimeFlag)

        if (ErrorsFound) {
            ShowFatalError(state, "InitSetPointManagers: Errors found in getting SetPointManager input.");
        }
    } // if (AirLoopInputsFilled)

    if ((state.dataGlobal->BeginEnvrnFlag && state.dataSetPointManager->InitSetPointManagersMyEnvrnFlag) ||
        state.dataSetPointManager->InitSetPointManagersOneTimeFlag2) {

        state.dataSetPointManager->ManagerOn = false;

        for (auto *spm : state.dataSetPointManager->spms) {

            switch (spm->type) {
            case SPMType::Scheduled: {
                auto *spmS = dynamic_cast<SPMScheduled *>(spm);
                assert(spmS != nullptr);

                for (int ctrlNodeNum : spmS->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    Real64 SchedValue = spmS->sched->getCurrentVal();
                    // Initialize scheduled setpoints
                    switch (spmS->ctrlVar) {
                    case HVAC::CtrlVarType::Temp: {
                        node.TempSetPoint = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MaxTemp: {
                        node.TempSetPointHi = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MinTemp: {
                        node.TempSetPointLo = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::HumRat: {
                        node.HumRatSetPoint = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MaxHumRat: {
                        node.HumRatMax = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MinHumRat: {
                        node.HumRatMin = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MassFlowRate: {
                        node.MassFlowRateSetPoint = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MaxMassFlowRate: {
                        node.MassFlowRateMax = SchedValue;
                    } break;
                    case HVAC::CtrlVarType::MinMassFlowRate: {
                        node.MassFlowRateMin = SchedValue;
                    } break;
                    default:
                        break;
                    }
                }
            } break;

            case SPMType::ScheduledDual: {
                auto *spmSD = dynamic_cast<SPMScheduledDual *>(spm);
                assert(spmSD != nullptr);
                for (int ctrlNodeNum : spmSD->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    if (spmSD->ctrlVar == HVAC::CtrlVarType::Temp) {
                        node.TempSetPointHi = spmSD->hiSched->getCurrentVal();
                        node.TempSetPointLo = spmSD->loSched->getCurrentVal();
                        node.TempSetPoint = (node.TempSetPointHi + node.TempSetPointLo) / 2.0;
                    }
                }
            } break;

            case SPMType::OutsideAir: {
                auto *spmOA = dynamic_cast<SPMOutsideAir *>(spm);
                assert(spmOA != nullptr);

                for (int NodeNum : spmOA->ctrlNodeNums) {
                    spmOA->calculate(state); // Why is this calculated for every node?

                    auto &node = state.dataLoopNodes->Node(NodeNum);
                    if (spmOA->ctrlVar == HVAC::CtrlVarType::Temp) {
                        node.TempSetPoint = spmOA->setPt;
                    } else if (spmOA->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                        node.TempSetPointHi = spmOA->setPt;
                    } else if (spmOA->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                        node.TempSetPointLo = spmOA->setPt;
                    }
                }
            } break;

            case SPMType::SZMinHum: {
                auto *spmSZH = dynamic_cast<SPMSingleZoneHum *>(spm);
                assert(spmSZH != nullptr);

                state.dataLoopNodes->Node(spmSZH->zoneNodeNum).MassFlowRate = 0.0;
                for (int ctrlNodeNum : spmSZH->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMin = 0.007;
                }
            } break;

            case SPMType::SZMaxHum: {
                auto *spmSZH = dynamic_cast<SPMSingleZoneHum *>(spm);
                assert(spmSZH != nullptr);

                state.dataLoopNodes->Node(spmSZH->zoneNodeNum).MassFlowRate = 0.0;
                for (int ctrlNodeNum : spmSZH->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMax = 0.011;
                }
            } break;

            case SPMType::SZReheat: {
                auto *spmSZR = dynamic_cast<SPMSingleZoneReheat *>(spm);
                assert(spmSZR != nullptr);

                state.dataLoopNodes->Node(spmSZR->zoneInletNodeNum).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(spmSZR->zoneNodeNum).MassFlowRate = 0.0;
                if (spmSZR->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmSZR->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the setpoint
                    }
                }

            } break;

            case SPMType::SZHeating:
            case SPMType::SZCooling: {
                auto *spmSZT = dynamic_cast<SPMSingleZoneTemp *>(spm);
                assert(spmSZT != nullptr);

                state.dataLoopNodes->Node(spmSZT->zoneInletNodeNum).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(spmSZT->zoneNodeNum).MassFlowRate = 0.0;

                if (spmSZT->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmSZT->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the setpoint
                    }
                }
            } break;

            case SPMType::MixedAir: {
                auto *spmMA = dynamic_cast<SPMMixedAir *>(spm);
                assert(spmMA != nullptr);

                auto &refNode = state.dataLoopNodes->Node(spmMA->refNodeNum);
                auto &fanInNode = state.dataLoopNodes->Node(spmMA->fanInNodeNum);
                auto &fanOutNode = state.dataLoopNodes->Node(spmMA->fanOutNodeNum);

                refNode.MassFlowRate = fanInNode.MassFlowRate = fanOutNode.MassFlowRate = 0.0;
                refNode.Temp = fanInNode.Temp = fanOutNode.Temp = 20.0;
                refNode.HumRat = fanInNode.HumRat = fanOutNode.HumRat = state.dataEnvrn->OutHumRat;
                refNode.Quality = fanInNode.Quality = fanOutNode.Quality = 1.0;
                refNode.Press = fanInNode.Press = fanOutNode.Press = state.dataEnvrn->OutBaroPress;
                refNode.Enthalpy = fanInNode.Enthalpy = fanOutNode.Enthalpy = PsyHFnTdbW(20.0, state.dataEnvrn->OutHumRat);

                if (spmMA->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmMA->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the setpoint
                    }
                }

            } break;

            case SPMType::OutsideAirPretreat: {
                auto *spmOAP = dynamic_cast<SPMOutsideAirPretreat *>(spm);
                assert(spmOAP != nullptr);

                auto &refNode = state.dataLoopNodes->Node(spmOAP->refNodeNum);
                auto &mixedOutNode = state.dataLoopNodes->Node(spmOAP->mixedOutNodeNum);
                auto &oaInNode = state.dataLoopNodes->Node(spmOAP->oaInNodeNum);
                auto &returnInNode = state.dataLoopNodes->Node(spmOAP->returnInNodeNum);

                refNode.MassFlowRate = mixedOutNode.MassFlowRate = oaInNode.MassFlowRate = returnInNode.MassFlowRate = 0.0;
                refNode.Temp = mixedOutNode.Temp = oaInNode.Temp = returnInNode.Temp = 20.0;
                refNode.HumRat = mixedOutNode.HumRat = oaInNode.HumRat = returnInNode.HumRat = state.dataEnvrn->OutHumRat;
                refNode.Quality = mixedOutNode.Quality = oaInNode.Quality = returnInNode.Quality = 1.0;
                refNode.Press = mixedOutNode.Press = oaInNode.Press = returnInNode.Press = state.dataEnvrn->OutBaroPress;
                refNode.Enthalpy = mixedOutNode.Enthalpy = oaInNode.Enthalpy = returnInNode.Enthalpy = PsyHFnTdbW(20.0, state.dataEnvrn->OutHumRat);

                for (int ctrlNodeNum : spmOAP->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    if (spmOAP->ctrlVar == HVAC::CtrlVarType::Temp) {
                        node.TempSetPoint = 20.0; // Set the setpoint
                    } else if (spmOAP->ctrlVar == HVAC::CtrlVarType::MaxHumRat) {
                        node.HumRatMax = state.dataEnvrn->OutHumRat; // Set the setpoint
                    } else if (spmOAP->ctrlVar == HVAC::CtrlVarType::MinHumRat) {
                        node.HumRatMin = state.dataEnvrn->OutHumRat; // Set the setpoint
                    } else if (spmOAP->ctrlVar == HVAC::CtrlVarType::HumRat) {
                        node.HumRatSetPoint = state.dataEnvrn->OutHumRat; // Set the setpoint
                    }
                }
            } break;

            case SPMType::Warmest:
            case SPMType::Coldest: {

                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    if (spm->ctrlVar == HVAC::CtrlVarType::Temp) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the setpoint
                    }
                }

            } break;

            case SPMType::WarmestTempFlow: {
                auto *spmWTF = dynamic_cast<SPMWarmestTempFlow *>(spm);
                assert(spmWTF != nullptr);

                if (spmWTF->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmWTF->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the temperature setpoint
                        if (spmWTF->airLoopNum != 0) {
                            state.dataAirLoop->AirLoopFlow(spmWTF->airLoopNum).ReqSupplyFrac = 1.0;           // PH 10/09/04 Set the flow
                            state.dataAirLoop->AirLoopControlInfo(spmWTF->airLoopNum).LoopFlowRateSet = true; // PH 10/09/04 Set the flag
                        }
                    }
                }
            } break;

            case SPMType::ReturnAirBypass: {
                if (state.dataZoneEquip->ZoneEquipInputsFilled && state.dataAirLoop->AirLoopInputsFilled) {

                    auto *spmRAB = dynamic_cast<SPMReturnAirBypassFlow *>(spm);
                    assert(spmRAB != nullptr);

                    if (spmRAB->ctrlVar == HVAC::CtrlVarType::MassFlowRate) {
                        state.dataLoopNodes->Node(spmRAB->rabSplitOutNodeNum).MassFlowRateSetPoint = 0.0;
                    }
                }
            } break;

            case SPMType::MZCoolingAverage:
            case SPMType::MZHeatingAverage: {
                if (spm->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spm->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = 20.0; // Set the setpoint
                    }
                }
            } break;

            case SPMType::MZMinHumAverage:
            case SPMType::MZMinHum: {
                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMin = 0.007; // Set the setpoint
                }
            } break;

            case SPMType::MZMaxHumAverage:
            case SPMType::MZMaxHum: {
                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMax = 0.011; // Set the setpoint
                }
            } break;

            case SPMType::FollowOutsideAirTemp: {
                auto *spmFOAT = dynamic_cast<SPMFollowOutsideAirTemp *>(spm);
                assert(spmFOAT != nullptr);
                bool isWetBulb = spmFOAT->refTempType == AirTempType::WetBulb;

                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    if (spmFOAT->ctrlVar == HVAC::CtrlVarType::Temp) {
                        node.TempSetPoint = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                    } else if (spmFOAT->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                        node.TempSetPointHi = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                    } else if (spmFOAT->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                        node.TempSetPointLo = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                    }
                }
            } break;

            case SPMType::FollowSystemNodeTemp: {
                auto *spmFSNT = dynamic_cast<SPMFollowSysNodeTemp *>(spm);
                assert(spmFSNT != nullptr);

                bool isWetBulb = spmFSNT->refTempType == AirTempType::WetBulb;
                auto &refNode = state.dataLoopNodes->Node(spmFSNT->refNodeNum);
                for (int ctrlNodeNum : spmFSNT->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    if (CheckOutAirNodeNumber(state, spmFSNT->refNodeNum)) {
                        refNode.SPMNodeWetBulbRepReq = isWetBulb;
                        if (spmFSNT->ctrlVar == HVAC::CtrlVarType::Temp) {
                            node.TempSetPoint = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                        } else if (spmFSNT->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                            node.TempSetPointHi = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                        } else if (spmFSNT->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                            node.TempSetPointLo = isWetBulb ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp;
                        }
                    } else { // If reference node is a water node, then set RefTypeMode to NodeDryBulb

                        if (refNode.FluidType == DataLoopNode::NodeFluidType::Water) {
                            spmFSNT->refTempType = AirTempType::DryBulb;
                        } else if (refNode.FluidType == DataLoopNode::NodeFluidType::Air) {
                            if (spmFSNT->refTempType == AirTempType::WetBulb) {
                                refNode.SPMNodeWetBulbRepReq = true;
                            }
                        }
                        if (spmFSNT->ctrlVar == HVAC::CtrlVarType::Temp) {
                            node.TempSetPoint = 20.0; // Set the setpoint
                        } else if (spmFSNT->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                            node.TempSetPointHi = 20.0; // Set the setpoint
                        } else if (spmFSNT->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                            node.TempSetPointLo = 20.0; // Set the setpoint
                        }
                    }
                }
            } break;

            case SPMType::FollowGroundTemp: {
                auto *spmFGT = dynamic_cast<SPMFollowGroundTemp *>(spm);
                assert(spmFGT != nullptr);

                Real64 GroundTemp = state.dataEnvrn->GroundTemp[(int)spmFGT->refTempType];

                for (int ctrlNodeNum : spmFGT->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    if (spmFGT->ctrlVar == HVAC::CtrlVarType::Temp) {
                        node.TempSetPoint = GroundTemp;
                    } else if (spmFGT->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                        node.TempSetPointHi = GroundTemp;
                    } else if (spmFGT->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                        node.TempSetPointLo = GroundTemp;
                    }
                }
            } break;

            case SPMType::CondenserEnteringTemp: {
                auto *spmCER = dynamic_cast<SPMCondenserEnteringTemp *>(spm);
                assert(spmCER != nullptr);
                Real64 SchedValue = spmCER->condenserEnteringTempSched->getCurrentVal();
                for (int ctrlNodeNum : spmCER->ctrlNodeNums) {
                    if (spmCER->ctrlVar == HVAC::CtrlVarType::Temp) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = SchedValue;
                    }
                }
            } break;

            case SPMType::IdealCondenserEnteringTemp: {
                auto const *spmICER = dynamic_cast<SPMIdealCondenserEnteringTemp *>(spm);
                assert(spmICER != nullptr);

                if (spmICER->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmICER->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spmICER->maxCondenserEnteringTemp;
                    }
                }
            } break;

            case SPMType::SZOneStageCooling: {
                auto const *spmSZOSC = dynamic_cast<SPMSingleZoneOneStageCooling *>(spm);
                assert(spmSZOSC != nullptr);

                if (spmSZOSC->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmSZOSC->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spmSZOSC->coolingOffSetPt;
                    }
                }
            } break;

            case SPMType::SZOneStageHeating: {
                auto const *spmSZOSH = dynamic_cast<SPMSingleZoneOneStageHeating *>(spm);
                assert(spmSZOSH != nullptr);

                if (spmSZOSH->ctrlVar == HVAC::CtrlVarType::Temp) {
                    for (int ctrlNodeNum : spmSZOSH->ctrlNodeNums) {
                        state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spmSZOSH->heatingOffSetPt;
                    }
                }

            } break;

            case SPMType::ChilledWaterReturnTemp: {
                auto *spmRWT = dynamic_cast<SPMReturnWaterTemp *>(spm);
                assert(spmRWT != nullptr);

                state.dataLoopNodes->Node(spmRWT->supplyNodeNum).TempSetPoint = spmRWT->minSetTemp;

            } break;

            case SPMType::HotWaterReturnTemp: {
                auto *spmRWT = dynamic_cast<SPMReturnWaterTemp *>(spm);
                assert(spmRWT != nullptr);

                state.dataLoopNodes->Node(spmRWT->supplyNodeNum).TempSetPoint = spmRWT->maxSetTemp;
            } break;

            case SPMType::SystemNodeTemp:
            case SPMType::SystemNodeHum: {
                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                    spm->calculate(state);
                    switch (spm->ctrlVar) {
                    case HVAC::CtrlVarType::Temp: {
                        node.TempSetPoint = spm->setPt;
                    } break;
                    case HVAC::CtrlVarType::MaxTemp: {
                        node.TempSetPointHi = spm->setPt;
                    } break;
                    case HVAC::CtrlVarType::MinTemp: {
                        node.TempSetPointLo = spm->setPt;
                    } break;
                    case HVAC::CtrlVarType::HumRat: {
                        node.HumRatSetPoint = spm->setPt;
                    } break;
                    case HVAC::CtrlVarType::MaxHumRat: {
                        node.HumRatMax = spm->setPt;
                    } break;
                    case HVAC::CtrlVarType::MinHumRat: {
                        node.HumRatMin = spm->setPt;
                    } break;
                    default:
                        break;
                    }
                }
            } break;

            default:
                break;
            } // switch (spm->type)
        } // for (spm)

        state.dataSetPointManager->InitSetPointManagersMyEnvrnFlag = false;
        if (!state.dataSetPointManager->InitSetPointManagersOneTimeFlag) {
            state.dataSetPointManager->InitSetPointManagersOneTimeFlag2 = false;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "InitSetPointManagers: Errors found. Program Terminates.");
        }

    } // end begin environment inits

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataSetPointManager->InitSetPointManagersMyEnvrnFlag = true;
    }
} // InitSetPointManagers()

void SimSetPointManagers(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   July 1998
    //       MODIFIED       Shirey/Raustad (FSEC), Jan 2004
    //                      Nov 2004 M. J. Witte, GARD Analytics, Inc.
    //                        Add new setpoint managers:
    //                          SET POINT MANAGER:SINGLE ZONE HEATING and
    //                          SET POINT MANAGER:SINGLE ZONE COOLING
    //                        Work supported by ASHRAE research project 1254-RP
    //                      Haves Oct 2004
    //                      July 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers
    //                          SetpointManager:MultiZone:Heating:Average
    //                          SetpointManager:MultiZone:Cooling:Average
    //                          SetpointManager:MultiZone:MinimumHumidity:Average
    //                          SetpointManager:MultiZone:MaximumHumidity:Average
    //                      Aug 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Humidity:Minimum
    //                          SetpointManager:MultiZone:Humidity:Maximum
    //                      Aug 2014 Rick Strand, UIUC
    //                         SetpointManager:ScheduleTES (internally defined)
    //                      Jan 2022 Wooyoung Jung, Jeremy Lerond and Jian Zhang, PNNL
    //                        Added new setpoint managers:
    //                          SetpointManager:SystemNodeReset:Temperature
    //                          SetpointManager:SystemNodeReset:Humidity

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the Setpoint Managers and invoke the correct
    // Setpoint Manager algorithm.

    for (auto *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::MixedAir && spm->type != SPMType::OutsideAirPretreat) {
            spm->calculate(state);
        }
    }
} // SimSetPointManagers()

void SPMScheduled::calculate([[maybe_unused]] EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   July 1998

    // PURPOSE OF THIS SUBROUTINE:
    // Set the setpoint using a simple schedule.
    this->setPt = this->sched->getCurrentVal();
} // SPMScheduled::calculate()

void SPMTESScheduled::calculate([[maybe_unused]] EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Aug 2014

    // PURPOSE OF THIS SUBROUTINE:
    // Set the setpoint using a simple schedule, then modify the value based on TES simple controls logic

    // METHODOLOGY EMPLOYED:
    // Modified schedule setpoint manager logic

    // Locals
    Real64 constexpr OnVal(0.5);

    Real64 CurSchValOnPeak = this->sched->getCurrentVal();
    Real64 CurSchValCharge = this->chargeSched->getCurrentVal();

    // CtrlType bug
    //        if (this->CompOpType == DataPlant::CtrlType::CoolingOp) { // this is some sort of chiller
    if (this->compOpType == DataPlant::CtrlType::HeatingOp) { // this is some sort of chiller
        if (CurSchValOnPeak >= OnVal) {
            this->setPt = this->nonChargeCHWTemp;
        } else if (CurSchValCharge < OnVal) {
            this->setPt = this->nonChargeCHWTemp;
        } else {
            this->setPt = this->chargeCHWTemp;
        }
        // CtrlType Bug
        //        } else if (this->CompOpType == DataPlant::CtrlType::DualOp) { // this is some sort of ice storage system
    } else if (this->compOpType == DataPlant::CtrlType::CoolingOp) { // this is some sort of ice storage system
        this->setPt = this->nonChargeCHWTemp;
    }
} // SPMTESSScheduled::calculate()

void SPMScheduledDual::calculate([[maybe_unused]] EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   May 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Set the both setpoint using a simple schedule.
    this->setPtHi = this->hiSched->getCurrentVal();
    this->setPtLo = this->loSched->getCurrentVal();
} // SPMScheduledDual::calculate()

void SPMOutsideAir::calculate(EnergyPlusData &state)
{
    Real64 SchedVal = (this->sched != nullptr) ? this->sched->getCurrentVal() : 0.0;

    if (SchedVal == 2.0) {
        this->setPt = interpSetPoint(this->low2, this->high2, state.dataEnvrn->OutDryBulbTemp, this->lowSetPt2, this->highSetPt2);
    } else {
        if ((this->sched != nullptr) && (SchedVal != 1.0)) { // Since schedule is optional, only check this if the user entered a schedule
            ++this->setPtErrorCount;
            if (this->setPtErrorCount <= 10) {
                ShowSevereError(state,
                                format("Schedule Values for the Outside Air Setpoint Manager = {} are something other than 1 or 2.", this->Name));
                ShowContinueError(state, format("...the value for the schedule currently is {}", SchedVal));
                ShowContinueError(state, "...the value is being interpreted as 1 for this run but should be fixed.");
            } else {
                ShowRecurringSevereErrorAtEnd(
                    state,
                    format("Schedule Values for the Outside Air Setpoint Manager = {} are something other than 1 or 2.", this->Name),
                    this->invalidSchedValErrorIndex);
            }
        }
        this->setPt = interpSetPoint(this->low1, this->high1, state.dataEnvrn->OutDryBulbTemp, this->lowSetPt1, this->highSetPt1);
    }

} // SPMOutsideAir::calculate()

void SPMSingleZoneReheat::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2000

    // PURPOSE OF THIS SUBROUTINE:
    // From the heating or cooling load of the control zone, calculate the supply air setpoint
    // needed to meet that zone load

    // Using/Aliasing
    using namespace DataZoneEnergyDemands;
    using Psychrometrics::PsyTdbFnHW;

    Real64 TSetPt;

    auto const &zoneInletNode = state.dataLoopNodes->Node(this->zoneInletNodeNum);

    // changed from MinOAFrac, now updates to current oa fraction for improve deadband control
    Real64 OAFrac = state.dataAirLoop->AirLoopFlow(this->airLoopNum).OAFrac;
    Real64 ZoneMassFlow = zoneInletNode.MassFlowRate;

    auto const &zoneSysEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->ctrlZoneNum);
    Real64 ZoneLoad = zoneSysEnergyDemand.TotalOutputRequired;
    Real64 ZoneLoadToCoolSetPt = zoneSysEnergyDemand.OutputRequiredToCoolingSP;
    Real64 ZoneLoadToHeatSetPt = zoneSysEnergyDemand.OutputRequiredToHeatingSP;
    bool DeadBand = state.dataZoneEnergyDemand->DeadBandOrSetback(this->ctrlZoneNum);
    Real64 ZoneTemp = state.dataLoopNodes->Node(this->zoneNodeNum).Temp;

    Real64 TMixAtMinOA;
    if (this->oaInNodeNum > 0) {
        auto const &oaInNode = state.dataLoopNodes->Node(this->oaInNodeNum);
        auto const &retNode = state.dataLoopNodes->Node(this->retNodeNum);
        Real64 HumRatMixAtMinOA = (1.0 - OAFrac) * retNode.HumRat + OAFrac * oaInNode.HumRat;
        Real64 EnthMixAtMinOA = (1.0 - OAFrac) * retNode.Enthalpy + OAFrac * oaInNode.Enthalpy;
        TMixAtMinOA = PsyTdbFnHW(EnthMixAtMinOA, HumRatMixAtMinOA);
    } else {
        TMixAtMinOA = state.dataLoopNodes->Node(this->loopInNodeNum).Temp;
    }

    Real64 FanDeltaT;
    if (this->fanOutNodeNum > 0 && this->fanInNodeNum > 0) {
        FanDeltaT = state.dataLoopNodes->Node(this->fanOutNodeNum).Temp - state.dataLoopNodes->Node(this->fanInNodeNum).Temp;
    } else {
        FanDeltaT = 0.0;
    }

    Real64 TSupNoHC = TMixAtMinOA + FanDeltaT;
    Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
    Real64 ExtrRateNoHC = CpAir * ZoneMassFlow * (TSupNoHC - ZoneTemp);
    if (ZoneMassFlow <= HVAC::SmallMassFlow) {
        TSetPt = TSupNoHC;

    } else if (DeadBand || std::abs(ZoneLoad) < HVAC::SmallLoad) {
        // if air with no active heating or cooling provides cooling
        if (ExtrRateNoHC < 0.0) {
            // if still in deadband, do no active heating or cooling;
            // if below heating setpoint, set a supply temp that will cool to the heating setpoint
            TSetPt = (ExtrRateNoHC >= ZoneLoadToHeatSetPt) ? TSupNoHC : (ZoneTemp + ZoneLoadToHeatSetPt / (CpAir * ZoneMassFlow));

            // if air with no active heating or cooling provides heating
        } else if (ExtrRateNoHC > 0.0) {
            // if still in deadband, do no active heating or cooling;
            // if above cooling setpoint, set a supply temp that will heat to the cooling setpoint
            TSetPt = (ExtrRateNoHC <= ZoneLoadToCoolSetPt) ? TSupNoHC : (ZoneTemp + ZoneLoadToCoolSetPt / (CpAir * ZoneMassFlow));

        } else {
            TSetPt = TSupNoHC;
        }

    } else if (ZoneLoad < (-1.0 * HVAC::SmallLoad)) {
        Real64 TSetPt1 = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlow);
        Real64 TSetPt2 = ZoneTemp + ZoneLoadToHeatSetPt / (CpAir * ZoneMassFlow);
        TSetPt = (TSetPt1 <= TSupNoHC) ? TSetPt1 : ((TSetPt2 > TSupNoHC) ? TSetPt2 : TSupNoHC);

    } else if (ZoneLoad > HVAC::SmallLoad) {
        Real64 TSetPt1 = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlow);
        Real64 TSetPt2 = ZoneTemp + ZoneLoadToCoolSetPt / (CpAir * ZoneMassFlow);
        TSetPt = (TSetPt1 >= TSupNoHC) ? TSetPt1 : ((TSetPt2 < TSupNoHC) ? TSetPt2 : TSupNoHC);

    } else {
        TSetPt = TSupNoHC;
    }

    this->setPt = std::clamp(TSetPt, this->minSetTemp, this->maxSetTemp);
} // SPMSZReheat::calculate()

void SPMSingleZoneTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. J. Witte based on CalcSingZoneRhSetPoint by Fred Buhl,
    //                        Work supported by ASHRAE research project 1254-RP
    //       DATE WRITTEN   November 2004

    // PURPOSE OF THIS SUBROUTINE:
    // From the heating load of the control zone, calculate the supply air setpoint
    // needed to meet that zone load (based on CalcSingZoneRhSetPoint)
    auto const &zoneInletNode = state.dataLoopNodes->Node(this->zoneInletNodeNum);

    // This function handles both heating and cooling
    auto const &zoneEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->ctrlZoneNum);
    Real64 ZoneLoadToSP =
        (this->type == SPMType::SZHeating) ? zoneEnergyDemand.OutputRequiredToHeatingSP : zoneEnergyDemand.OutputRequiredToCoolingSP;

    Real64 ZoneTemp = state.dataLoopNodes->Node(this->zoneNodeNum).Temp;
    if (zoneInletNode.MassFlowRate <= HVAC::SmallMassFlow) {
        this->setPt = (this->type == SPMType::SZHeating) ? this->minSetTemp : this->maxSetTemp;
    } else {
        Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
        this->setPt = ZoneTemp + ZoneLoadToSP / (CpAir * zoneInletNode.MassFlowRate);
        this->setPt = std::clamp(this->setPt, this->minSetTemp, this->maxSetTemp);
    }
} // SPMSZTemp::calculate()

void SPMSingleZoneOneStageCooling::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   August 2013

    // PURPOSE OF THIS SUBROUTINE:
    // calculate the setpoint for staged on/off cooling

    // METHODOLOGY EMPLOYED:
    // Evaluate stage in zone energy demand structure and choose setpoint accordingly

    this->setPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->ctrlZoneNum).StageNum >= 0) ? this->coolingOffSetPt : this->coolingOnSetPt;
    // negative so a cooling stage is set
} // SPMSingleZoneOneStageCooling::calculate()

void SPMSingleZoneOneStageHeating::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   August 2013

    // PURPOSE OF THIS SUBROUTINE:
    // calculate the setpoint for staged on/off control

    // METHODOLOGY EMPLOYED:
    // Evaluate stage in zone energy demand structure and choose setpoint accordingly

    this->setPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->ctrlZoneNum).StageNum <= 0) ? this->heatingOffSetPt : this->heatingOnSetPt;
} // SPMSingleZoneOneStageHeating::calculate()

void SPMSingleZoneHum::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   October 2000
    //       MODIFIED       Shirey/Raustad Jan 2002
    //                      Gu, Dec 2007

    // PURPOSE OF THIS SUBROUTINE:
    // From humidity load of the control zone, calculate the supply air humidity
    // needed to meet the minimum humidity setpoint

    // METHODOLOGY EMPLOYED:
    // Zone moisture load from ZoneTempPredictorCorrector (via DataZoneEnergyDemands)
    // is used to calculate the minimum supply air humidity ratio
    // needed to meet minimum zone relative humidity requirement

    // Using/Aliasing
    using Psychrometrics::PsyWFnTdbRhPb;

    // Only use one zone for now
    auto &zoneNode = state.dataLoopNodes->Node(this->zoneNodeNum);

    Real64 ZoneMassFlow = zoneNode.MassFlowRate;
    if (ZoneMassFlow > HVAC::SmallMassFlow) {
        auto const &zoneMoistureDemand = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(this->ctrlZoneNum);
        Real64 MoistureLoad =
            (this->type == SPMType::SZMinHum) ? zoneMoistureDemand.OutputRequiredToHumidifyingSP : zoneMoistureDemand.OutputRequiredToDehumidifyingSP;

        // This function handles both SZMinHum and SZMaxHum
        // MoistureLoad (negative for dehumidification) may be so large that a negative humrat results, cap at 0.00001
        Real64 MaxHum = (this->type == SPMType::SZMinHum) ? 0.0 : 0.00001;

        // Positive Humidity Ratio MoistureLoad means a humidification load and only humidifying can raise up to a minimum
        //  IF(MoistureLoad .GT. 0.0) SZMinHumSetPtMgr(SetPtMgrNum)%SetPt = SupplyAirHumRat
        this->setPt = max(MaxHum, zoneNode.HumRat + MoistureLoad / ZoneMassFlow);

        // This hum rat is currently used in Controller:Simple, control variable "TEMPandHUMRAT" (Jan 2004)
        // Negative MoistureLoad means a dehumidification load
    } else {
        this->setPt = 0.0;
    }
} // SPMSingleZoneHum::calculate()

void SPMMixedAir::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Starting with the setpoint at the reference node, subtract the supply fan
    // temperature rise and set the resulting temperature at the mixed air node.
    // Using/Aliasing
    using EMSManager::CheckIfNodeSetPointManagedByEMS;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &fanInNode = state.dataLoopNodes->Node(this->fanInNodeNum);
    auto &fanOutNode = state.dataLoopNodes->Node(this->fanOutNodeNum);
    auto &refNode = state.dataLoopNodes->Node(this->refNodeNum);

    this->freezeCheckEnable = false;

    if (!state.dataGlobal->SysSizingCalc && this->mySetPointCheckFlag) {

        if (refNode.TempSetPoint == SensedNodeFlagValue) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                ShowSevereError(state,
                                format("CalcMixedAirSetPoint: Missing reference temperature setpoint for Mixed Air Setpoint Manager {}", this->Name));
                ShowContinueError(state, format("Node Referenced ={}", state.dataLoopNodes->NodeID(this->refNodeNum)));
                ShowContinueError(
                    state, "  use an additional Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at this node.");
                state.dataHVACGlobal->SetPointErrorFlag = true;
            } else {
                // need call to check if this is the target of an EnergyManagementSystem:Actuator object
                CheckIfNodeSetPointManagedByEMS(state, this->refNodeNum, HVAC::CtrlVarType::Temp, state.dataHVACGlobal->SetPointErrorFlag);
                if (state.dataHVACGlobal->SetPointErrorFlag) {
                    ShowSevereError(
                        state, format("CalcMixedAirSetPoint: Missing reference temperature setpoint for Mixed Air Setpoint Manager {}", this->Name));
                    ShowContinueError(state, format("Node Referenced ={}", state.dataLoopNodes->NodeID(this->refNodeNum)));
                    ShowContinueError(
                        state, "  use an additional Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at this node.");
                    ShowContinueError(state, "Or add EMS Actuator to provide temperature setpoint at this node");
                }
            }
        }

        this->mySetPointCheckFlag = false;
    }

    this->setPt = refNode.TempSetPoint - (fanOutNode.Temp - fanInNode.Temp);
    if (this->coolCoilInNodeNum > 0 && this->coolCoilOutNodeNum > 0) {
        auto const &coolCoilInNode = state.dataLoopNodes->Node(this->coolCoilInNodeNum);
        auto const &coolCoilOutNode = state.dataLoopNodes->Node(this->coolCoilOutNodeNum);
        Real64 dtFan = fanOutNode.Temp - fanInNode.Temp;
        Real64 dtCoolCoil = coolCoilInNode.Temp - coolCoilOutNode.Temp;
        if (dtCoolCoil > 0.0 && this->minCoolCoilOutTemp > state.dataEnvrn->OutDryBulbTemp) {
            this->freezeCheckEnable = true;
            if (refNode.Temp == coolCoilOutNode.Temp) { // blow through
                this->setPt = max(refNode.TempSetPoint, this->minCoolCoilOutTemp) - dtFan + dtCoolCoil;
            } else if (this->refNodeNum != this->coolCoilOutNodeNum) { // // draw through Ref node is outlet node
                this->setPt = max(refNode.TempSetPoint - dtFan, this->minCoolCoilOutTemp) + dtCoolCoil;
            } else {
                this->setPt = max(refNode.TempSetPoint, this->minCoolCoilOutTemp) + dtCoolCoil;
            }
        }
    }
} // SPMMixedAir::calculate()

void SPMOutsideAirPretreat::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. J. Witte based on CalcMixedAirSetPoint by Fred Buhl,
    //                        Work supported by ASHRAE research project 1254-RP
    //       DATE WRITTEN   January 2005
    //       MODIFIED       Witte (GARD), Sep 2006
    //                      Griffith( NREL), May 2009, added EMS setpoint checks

    // PURPOSE OF THIS SUBROUTINE:
    // Starting with the setpoint at the reference node, determine the required
    // outside air inlet conditions which when mixed with return air result in
    // the reference setpoint at the mixed air node.
    // (based on CalcMixedAirSetPoint)

    // Using/Aliasing
    using EMSManager::CheckIfNodeSetPointManagedByEMS;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 ReturnInValue = 0;   // return air inlet node mass flow rate
    Real64 RefNodeSetPoint = 0; // setpoint at reference node
    Real64 MinSetPoint = 0;     // minimum allowed setpoint
    Real64 MaxSetPoint = 0;     // maximum allowed setpoint

    auto &refNode = state.dataLoopNodes->Node(this->refNodeNum);
    auto &mixedOutNode = state.dataLoopNodes->Node(this->mixedOutNodeNum);
    auto &oaInNode = state.dataLoopNodes->Node(this->oaInNodeNum);
    auto &returnInNode = state.dataLoopNodes->Node(this->returnInNodeNum);

    bool isHumiditySetPoint = false;

    switch (this->ctrlVar) {
    case HVAC::CtrlVarType::Temp: { // 'Temperature'
        RefNodeSetPoint = refNode.TempSetPoint;
        ReturnInValue = returnInNode.Temp;
        MinSetPoint = this->minSetTemp;
        MaxSetPoint = this->maxSetTemp;
    } break;
    case HVAC::CtrlVarType::MaxHumRat: { // 'HUMRATMAX'
        RefNodeSetPoint = refNode.HumRatMax;
        ReturnInValue = returnInNode.HumRat;
        MinSetPoint = this->minSetHum;
        MaxSetPoint = this->maxSetHum;
        isHumiditySetPoint = true;
    } break;
    case HVAC::CtrlVarType::MinHumRat: { // 'HUMRATMIN'
        RefNodeSetPoint = refNode.HumRatMin;
        ReturnInValue = returnInNode.HumRat;
        MinSetPoint = this->minSetHum;
        MaxSetPoint = this->maxSetHum;
        isHumiditySetPoint = true;
    } break;
    case HVAC::CtrlVarType::HumRat: { // 'HumidityRatio'
        RefNodeSetPoint = refNode.HumRatSetPoint;
        ReturnInValue = returnInNode.HumRat;
        MinSetPoint = this->minSetHum;
        MaxSetPoint = this->maxSetHum;
        isHumiditySetPoint = true;
    } break;
    default:
        break;
    }

    if (!state.dataGlobal->SysSizingCalc && this->mySetPointCheckFlag) {
        this->mySetPointCheckFlag = false;
        if (RefNodeSetPoint == SensedNodeFlagValue) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                ShowSevereError(
                    state, format("CalcOAPretreatSetPoint: Missing reference setpoint for Outdoor Air Pretreat Setpoint Manager {}", this->Name));
                ShowContinueError(state, format("Node Referenced ={}", state.dataLoopNodes->NodeID(this->refNodeNum)));
                ShowContinueError(state, "use a Setpoint Manager to establish a setpoint at this node.");
                ShowFatalError(state, "Missing reference setpoint.");
            } else {
                bool LocalSetPointCheckFailed = false;
                switch (this->ctrlVar) {
                case HVAC::CtrlVarType::Temp:      // 'Temperature'
                case HVAC::CtrlVarType::MaxHumRat: // 'HUMRATMAX'
                case HVAC::CtrlVarType::MinHumRat: // 'HUMRATMIN'
                case HVAC::CtrlVarType::HumRat: {  // 'HumidityRatio'
                    CheckIfNodeSetPointManagedByEMS(state, this->refNodeNum, this->ctrlVar, LocalSetPointCheckFailed);
                } break;
                default:
                    break;
                }
                if (LocalSetPointCheckFailed) {
                    ShowSevereError(
                        state, format("CalcOAPretreatSetPoint: Missing reference setpoint for Outdoor Air Pretreat Setpoint Manager {}", this->Name));
                    ShowContinueError(state, format("Node Referenced ={}", state.dataLoopNodes->NodeID(this->refNodeNum)));
                    ShowContinueError(state, "use a Setpoint Manager to establish a setpoint at this node.");
                    ShowContinueError(state, "Or use an EMS actuator to control a setpoint at this node.");
                    ShowFatalError(state, "Missing reference setpoint.");
                }
            }
        }
    }
    if ((mixedOutNode.MassFlowRate <= 0.0) || (oaInNode.MassFlowRate <= 0.0)) {
        this->setPt = RefNodeSetPoint;
    } else if (isHumiditySetPoint && (RefNodeSetPoint == 0.0)) {
        // For humidity setpoints, zero is special meaning "off" or "no load"
        // so pass through zero setpoints without enforcing the max/min setpoint limits
        this->setPt = 0.0;
    } else {
        Real64 OAFraction = oaInNode.MassFlowRate / mixedOutNode.MassFlowRate;
        this->setPt = ReturnInValue + (RefNodeSetPoint - ReturnInValue) / OAFraction;
        // Apply maximum and minimum values
        this->setPt = std::clamp(this->setPt, MinSetPoint, MaxSetPoint);
    }
} // SPMOutsideAirPretreat::calculate()

void SPMTempest::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2002

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the "warmest" supply air setpoint temperature that will satisfy the cooling
    // requirements of all the zones served by a central air system.

    // METHODOLOGY EMPLOYED:
    // Zone sensible heat balance

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SetPointTemp = 0.0;

    auto &airToZoneNode = state.dataAirLoop->AirToZoneNodeInfo(this->airLoopNum);

    if (this->type == SPMType::Warmest) {

        Real64 TotCoolLoad = 0.0;
        SetPointTemp = this->maxSetTemp;

        for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesCooled; ++iZoneNum) {
            int CtrlZoneNum = airToZoneNode.CoolCtrlZoneNums(iZoneNum);
            auto &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.CoolZoneInletNodes(iZoneNum));
            auto const &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);

            Real64 ZoneMassFlowMax = zoneInletNode.MassFlowRateMax;
            Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
            Real64 ZoneTemp = zoneNode.Temp;
            Real64 ZoneSetPointTemp = this->maxSetTemp;
            if (ZoneLoad < 0.0) {
                TotCoolLoad += std::abs(ZoneLoad);
                Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
                if (ZoneMassFlowMax > HVAC::SmallMassFlow) {
                    ZoneSetPointTemp = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlowMax);
                }
            }
            SetPointTemp = min(SetPointTemp, ZoneSetPointTemp);
        }

        SetPointTemp = std::clamp(SetPointTemp, this->minSetTemp, this->maxSetTemp);
        if (TotCoolLoad < HVAC::SmallLoad) {
            SetPointTemp = this->maxSetTemp;
        }

    } else { // (spm->type == SPMType::Coldest)
        Real64 TotHeatLoad = 0.0;
        SetPointTemp = this->minSetTemp;

        if (airToZoneNode.NumZonesHeated > 0) {
            // dual-duct heated only zones
            for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesHeated; ++iZoneNum) {
                int CtrlZoneNum = airToZoneNode.HeatCtrlZoneNums(iZoneNum);
                auto &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.HeatZoneInletNodes(iZoneNum));
                auto const &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);
                Real64 ZoneMassFlowMax = zoneInletNode.MassFlowRateMax;
                Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
                Real64 ZoneTemp = zoneNode.Temp;
                Real64 ZoneSetPointTemp = this->minSetTemp;
                if (ZoneLoad > 0.0) {
                    TotHeatLoad += ZoneLoad;
                    Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
                    if (ZoneMassFlowMax > HVAC::SmallMassFlow) {
                        ZoneSetPointTemp = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlowMax);
                    }
                }
                SetPointTemp = max(SetPointTemp, ZoneSetPointTemp);
            }
        } else {
            // single-duct or central heated and cooled zones
            for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesCooled; ++iZoneNum) {
                int CtrlZoneNum = airToZoneNode.CoolCtrlZoneNums(iZoneNum);
                auto &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.CoolZoneInletNodes(iZoneNum));
                auto const &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);
                Real64 ZoneMassFlowMax = zoneInletNode.MassFlowRateMax;
                Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
                Real64 ZoneTemp = zoneNode.Temp;
                Real64 ZoneSetPointTemp = this->minSetTemp;
                if (ZoneLoad > 0.0) {
                    TotHeatLoad += ZoneLoad;
                    Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
                    if (ZoneMassFlowMax > HVAC::SmallMassFlow) {
                        ZoneSetPointTemp = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlowMax);
                    }
                }
                SetPointTemp = max(SetPointTemp, ZoneSetPointTemp);
            }
        }

        SetPointTemp = std::clamp(SetPointTemp, this->minSetTemp, this->maxSetTemp);
        if (TotHeatLoad < HVAC::SmallLoad) {
            SetPointTemp = this->minSetTemp;
        }
    }

    this->setPt = SetPointTemp;
} // SMPTempest::calculate()

void SPMWarmestTempFlow::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2002
    //       MODIFIED       Haves, Oct 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the "warmest" supply air setpoint temperature that will satisfy the cooling
    // requirements of all the zones served by a central air system.

    // METHODOLOGY EMPLOYED:
    // Zone sensible heat balance

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    if (!this->simReady) {
        return;
    }
    Real64 TotCoolLoad = 0.0;
    Real64 MaxSetPointTemp = this->maxSetTemp;
    Real64 SetPointTemp = MaxSetPointTemp;
    Real64 MinSetPointTemp = this->minSetTemp;
    Real64 MinFracFlow = this->minTurndown;
    Real64 FracFlow = MinFracFlow;
    int CritZoneNumTemp = 0;
    int CritZoneNumFlow = 0;

    auto &airToZoneNode = state.dataAirLoop->AirToZoneNodeInfo(this->airLoopNum);

    for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesCooled; ++iZoneNum) {
        int CtrlZoneNum = airToZoneNode.CoolCtrlZoneNums(iZoneNum);
        auto &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.CoolZoneInletNodes(iZoneNum));
        auto const &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);

        Real64 ZoneMassFlowMax = zoneInletNode.MassFlowRateMax;
        Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
        Real64 ZoneTemp = zoneNode.Temp;
        Real64 ZoneSetPointTemp = MaxSetPointTemp;
        Real64 ZoneFracFlow = MinFracFlow;

        if (ZoneLoad < 0.0) {
            TotCoolLoad += std::abs(ZoneLoad);
            Real64 CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
            if (ZoneMassFlowMax > HVAC::SmallMassFlow) {
                if (this->strategy == ControlStrategy::TempFirst) {
                    // First find supply air temperature required to meet the load at minimum flow. If this is
                    // below the minimum supply air temperature, calculate the fractional flow rate required to meet the
                    // load at the minimum supply air temperature.
                    ZoneSetPointTemp = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlowMax * MinFracFlow);
                    if (ZoneSetPointTemp < MinSetPointTemp) {
                        ZoneFracFlow = (ZoneLoad / (CpAir * (MinSetPointTemp - ZoneTemp))) / ZoneMassFlowMax;
                    } else {
                        ZoneFracFlow = MinFracFlow;
                    }
                } else { // ControlStrategy = FlowFirst
                    // First find supply air flow rate required to meet the load at maximum supply air temperature. If this
                    // is above the maximum supply air flow rate, calculate the supply air temperature required to meet the
                    // load at the maximum flow.
                    ZoneFracFlow = (ZoneLoad / (CpAir * (MaxSetPointTemp - ZoneTemp))) / ZoneMassFlowMax;
                    if (ZoneFracFlow > 1.0 || ZoneFracFlow < 0.0) {
                        ZoneSetPointTemp = ZoneTemp + ZoneLoad / (CpAir * ZoneMassFlowMax);
                    } else {
                        ZoneSetPointTemp = MaxSetPointTemp;
                    }
                }
            }
        }
        if (ZoneSetPointTemp < SetPointTemp) {
            SetPointTemp = ZoneSetPointTemp;
            CritZoneNumTemp = CtrlZoneNum;
        }
        if (ZoneFracFlow > FracFlow) {
            FracFlow = ZoneFracFlow;
            CritZoneNumFlow = CtrlZoneNum;
        }
    }

    SetPointTemp = std::clamp(SetPointTemp, MinSetPointTemp, MaxSetPointTemp);
    FracFlow = std::clamp(FracFlow, MinFracFlow, 1.0);
    if (TotCoolLoad < HVAC::SmallLoad) {
        SetPointTemp = MaxSetPointTemp;
        FracFlow = MinFracFlow;
    }

    this->setPt = SetPointTemp;
    this->turndown = FracFlow;
    if (this->strategy == ControlStrategy::TempFirst) {
        this->critZoneNum = (CritZoneNumFlow != 0) ? CritZoneNumFlow : CritZoneNumTemp;
    } else { // ControlStrategy = FlowFirst
        this->critZoneNum = (CritZoneNumTemp != 0) ? CritZoneNumTemp : CritZoneNumFlow;
    }
} // SPMWarmestTempFlow::calculate()

void SPMReturnAirBypassFlow::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Given the desired setpoint temperature, calculate the flow rate through the
    // return air branch that will deliver the desired temperature at the loop outlet
    // node.

    auto &mixerRABInNode = state.dataLoopNodes->Node(this->rabMixInNodeNum);
    auto &mixerSupInNode = state.dataLoopNodes->Node(this->supMixInNodeNum);
    auto &mixerOutNode = state.dataLoopNodes->Node(this->mixOutNodeNum);
    auto &loopOutNode = state.dataLoopNodes->Node(this->sysOutNodeNum);

    Real64 TempSetPt = this->sched->getCurrentVal();
    Real64 TempSetPtMod = TempSetPt - (loopOutNode.Temp - mixerOutNode.Temp);
    Real64 SupFlow = mixerSupInNode.MassFlowRate;
    Real64 TempSup = mixerSupInNode.Temp;
    Real64 TotSupFlow = mixerOutNode.MassFlowRate;
    Real64 TempRAB = mixerRABInNode.Temp;
    Real64 RABFlow = (TotSupFlow * TempSetPtMod - SupFlow * TempSup) / max(TempRAB, 1.0);
    RABFlow = std::clamp(RABFlow, 0.0, TotSupFlow);
    this->FlowSetPt = RABFlow;
}

void SPMMultiZoneTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse, FSEC
    //       DATE WRITTEN   July 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the "Average" supply air setpoint temperature that will satisfy the heating
    // requirements of multizones served by a central air system.

    // This function handles both MZAverageHeating and MZAverageCooling

    // METHODOLOGY EMPLOYED:
    // Zone sensible (heating load) heat balance around the zones served by a central air system

    // sum of the zone's predicted loads for this air loop [W]
    Real64 SumLoad = 0.0;
    // sum of the product of zone inlet node actual mass flow rate, and
    // Cp of air at zone inlet node for all heated zones in the airloop [W/C]
    Real64 SumProductMdotCp = 0.0;
    // sum of the product of zone inlet node actual mass flow rate, and
    // Cp of air at zone air node for all zones in the airloop [W/C]
    Real64 SumProductMdotCpTot = 0.0;
    // sum of the product of zone inlet node actual mass flow rate,
    // Cp of air at zone air node and zone air node temperature for
    // all zones in the air loop [W]
    Real64 SumProductMdotCpTZoneTot = 0.0;

    auto &airToZoneNode = state.dataAirLoop->AirToZoneNodeInfo(this->airLoopNum);
    for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesCooled; ++iZoneNum) {
        // DO ZonesHeatedIndex=1,AirToZoneNodeInfo(AirLoopNum)%NumZonesHeated
        // Using AirToZoneNodeInfo(AirLoopNum)%Cool* structure variables since they include heating and cooling.

        // The data for number of zones heated is included in the data structure of the variable
        // "AirToZoneNodeInfo(AirLoopNum)%NumZonesCooled" for all systems.  The data structure
        // "AirToZoneNodeInfo(AirLoopNum)%NumZonesHeated" applies to Dual Duct System only and
        // if used will limit the application of this setpoint manager to other systems.  Thus,
        // the "AirToZoneNodeInfo(AirLoopNum)%NumZonesCooled" data is used instead.

        int CtrlZoneNum = airToZoneNode.CoolCtrlZoneNums(iZoneNum);
        auto &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.CoolZoneInletNodes(iZoneNum));
        auto &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);
        Real64 ZoneMassFlowRate = zoneInletNode.MassFlowRate;
        Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
        Real64 ZoneTemp = zoneNode.Temp;
        Real64 CpAir = PsyCpAirFnW(zoneNode.HumRat);
        SumProductMdotCpTot += ZoneMassFlowRate * CpAir;
        SumProductMdotCpTZoneTot += ZoneMassFlowRate * CpAir * ZoneTemp;
        if ((this->type == SPMType::MZHeatingAverage && ZoneLoad > 0.0) || (this->type == SPMType::MZCoolingAverage && ZoneLoad < 0.0)) {
            CpAir = PsyCpAirFnW(zoneInletNode.HumRat);
            SumLoad += ZoneLoad;
            SumProductMdotCp += ZoneMassFlowRate * CpAir;
        }
    }
    Real64 ZoneAverageTemp = (SumProductMdotCpTot > 0.0) ? (SumProductMdotCpTZoneTot / SumProductMdotCpTot) : 0.0;
    Real64 SetPointTemp = (SumProductMdotCp > 0.0) ? (ZoneAverageTemp + SumLoad / SumProductMdotCp)
                                                   : ((this->type == SPMType::MZHeatingAverage) ? this->minSetTemp : this->maxSetTemp);

    SetPointTemp = std::clamp(SetPointTemp, this->minSetTemp, this->maxSetTemp);
    if (std::abs(SumLoad) < HVAC::SmallLoad) {
        SetPointTemp = (this->type == SPMType::MZHeatingAverage) ? this->minSetTemp : this->maxSetTemp;
    }
    this->setPt = SetPointTemp;
} // SPMMultiZoneTemp::calculate()

void SPMMultiZoneHum::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse, FSEC
    //       DATE WRITTEN   July 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the "Average" supply air minimum humidity setpoint that will satisfy the minimum
    // humidity ratio requirements of multiple zones served by a central air system.

    // This function handles both MZMinHumAverage and MZMaxHumAverage

    // METHODOLOGY EMPLOYED:
    // Zone latent load balance around the zones served by a central air system

    Real64 constexpr SmallMoistureLoad(0.00001); // small moisture load [kgWater/s]

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SumMdot = 0.0;         // sum of the actual mass flow rate for controlled zones in the air loop [kg/s]
    Real64 SumMdotTot = 0.0;      // sum of the actual mass flow rate for this air loop [kg/s]
    Real64 SumMoistureLoad = 0.0; // sum of the zone moisture loads for this air loop [W]
    // sum of product of actual mass flow rate at the zone inlet node,
    // and humidity ratio at zones air node for all zones in the airloop [kgWater/s]
    Real64 SumProductMdotHumTot = 0.0;

    auto &airToZoneNode = state.dataAirLoop->AirToZoneNodeInfo(this->airLoopNum);

    Real64 SetPointHum = (this->type == SPMType::MZMinHum || this->type == SPMType::MZMinHumAverage) ? this->minSetHum : this->maxSetHum;

    for (int iZoneNum = 1; iZoneNum <= airToZoneNode.NumZonesCooled; ++iZoneNum) {
        int CtrlZoneNum = airToZoneNode.CoolCtrlZoneNums(iZoneNum);
        auto const &zoneInletNode = state.dataLoopNodes->Node(airToZoneNode.CoolZoneInletNodes(iZoneNum));
        auto const &zoneNode = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ZoneNode);
        auto const &zoneMoistureDemand = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(CtrlZoneNum);
        Real64 ZoneMassFlowRate = zoneInletNode.MassFlowRate;
        Real64 MoistureLoad = (this->type == SPMType::MZMinHum || this->type == SPMType::MZMinHumAverage)
                                  ? zoneMoistureDemand.OutputRequiredToHumidifyingSP
                                  : zoneMoistureDemand.OutputRequiredToDehumidifyingSP;

        Real64 ZoneHum = zoneNode.HumRat;
        // For humidification the moisture load is positive

        switch (this->type) {
        case SPMType::MZMinHumAverage: {
            SumMdotTot += ZoneMassFlowRate;
            SumProductMdotHumTot += ZoneMassFlowRate * ZoneHum;
            if (MoistureLoad > 0.0) {
                SumMdot += ZoneMassFlowRate;
                SumMoistureLoad += MoistureLoad;
            }
        } break;

        case SPMType::MZMaxHumAverage: {
            SumMdotTot += ZoneMassFlowRate;
            SumProductMdotHumTot += ZoneMassFlowRate * ZoneHum;
            if (MoistureLoad < 0.0) {
                SumMdot += ZoneMassFlowRate;
                SumMoistureLoad += MoistureLoad;
            }
        } break;

        case SPMType::MZMinHum: {
            Real64 ZoneSetPointHum = this->minSetHum;
            if (MoistureLoad > 0.0) {
                SumMoistureLoad += MoistureLoad;
                if (ZoneMassFlowRate > HVAC::SmallMassFlow) {
                    ZoneSetPointHum = max(0.0, ZoneHum + MoistureLoad / ZoneMassFlowRate);
                }
            }
            SetPointHum = max(SetPointHum, ZoneSetPointHum);
        } break;

        case SPMType::MZMaxHum: {
            Real64 ZoneSetPointHum = this->maxSetHum;
            if (MoistureLoad < 0.0) {
                SumMoistureLoad += MoistureLoad;
                if (ZoneMassFlowRate > HVAC::SmallMassFlow) {
                    ZoneSetPointHum = max(0.0, ZoneHum + MoistureLoad / ZoneMassFlowRate);
                }
            }
            SetPointHum = min(SetPointHum, ZoneSetPointHum);
        } break;

        default:
            break;
        } // switch (this->type)
    }

    if (this->type == SPMType::MZMinHumAverage || this->type == SPMType::MZMaxHumAverage) {
        Real64 AverageZoneHum = (SumMdotTot > HVAC::SmallMassFlow) ? (SumProductMdotHumTot / SumMdotTot) : 0.0;
        if (SumMdot > HVAC::SmallMassFlow) {
            SetPointHum = max(0.0, AverageZoneHum + SumMoistureLoad / SumMdot);
        }
    } else {
        if (std::abs(SumMoistureLoad) < SmallMoistureLoad) {
            SetPointHum = (this->type == SPMType::MZMinHum) ? this->minSetHum : this->maxSetHum;
        }
    }

    this->setPt = std::clamp(SetPointHum, this->minSetHum, this->maxSetHum);
} // SPMMultiZoneHum::calculate()

void SPMFollowOutsideAirTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Chandan Sharma, FSEC
    //       DATE WRITTEN   July 2011

    // PURPOSE OF THIS SUBROUTINE:
    // Set the setpoint based on outdoor air dry-bulb/wet-bulb temperature

    // METHODOLOGY EMPLOYED:
    // Based on reference temperature type specified in the setpoint manager,
    // the setpoint is calculated as OutWetBulbTemp(Or OutDryBulbTemp) + Offset.
    // The sign convention is that a positive Offset will increase the resulting setpoint.
    // Final value of the setpoint is limited by the Max and Min limit specified in the setpoint manager.
    this->setPt = ((this->refTempType == AirTempType::WetBulb) ? state.dataEnvrn->OutWetBulbTemp : state.dataEnvrn->OutDryBulbTemp) + this->offset;

    // Apply maximum and minimum values
    this->setPt = std::clamp(this->setPt, this->minSetTemp, this->maxSetTemp);
} // SPMFollowOutsideAirTemp::calculate()

void SPMFollowSysNodeTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Chandan Sharma, FSEC
    //       DATE WRITTEN   July 2011

    // PURPOSE OF THIS SUBROUTINE:
    // Set the setpoint based on current temperatures at a separate system node.

    // METHODOLOGY EMPLOYED:
    // The current value of the temperature at a reference node are obtained and used
    // to generate setpoint on a second system node.  If the reference node is also designated
    // to be an outdoor air (intake) node, then this setpoint manager can be used to follow
    // outdoor air conditions that are adjusted for altitude.
    // Also, based on reference temperature type specified in the setpoint manager, the out door air wet-bulb
    // or dry-bulb temperature at the reference node could be used.
    // A temperature offset will be applied to the value obtained from the reference system node.
    // If this value is zero, and the limits are met, then the resulting setpoint will be exactly the same
    // as the reference system node temperature.  The sign convention is that a positive offset will increase
    // the resulting setpoint.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 RefNodeTemp = (this->refTempType == AirTempType::DryBulb)
                             ? state.dataLoopNodes->Node(this->refNodeNum).Temp
                             : (allocated(state.dataLoopNodes->MoreNodeInfo) ? state.dataLoopNodes->MoreNodeInfo(this->refNodeNum).WetBulbTemp : 0.0);

    this->setPt = RefNodeTemp + this->offset;

    // Apply maximum and minimum values
    this->setPt = std::clamp(this->setPt, this->minSetTemp, this->maxSetTemp);
} // SPMFollowSysNodeTemp::calculate()

void SPMFollowGroundTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Chandan Sharma, FSEC
    //       DATE WRITTEN   July 2011

    // PURPOSE OF THIS SUBROUTINE:
    // Set the setpoint based on current ground temperature

    // METHODOLOGY EMPLOYED:
    // Based on reference ground temperature object type specified in the setpoint manager,
    // the setpoint is calculated as GroundTemperature + Offset.
    // The sign convention is that a positive Offset will increase the resulting setpoint.
    // Final value of the setpoint is limited by the Max and Min limit specified in the setpoint manager.
    this->setPt = state.dataEnvrn->GroundTemp[(int)this->refTempType] + this->offset;

    // Apply maximum and minimum values
    this->setPt = std::clamp(this->setPt, this->minSetTemp, this->maxSetTemp);
} // SPMFollowGrounTemp::calculate()

void SPMCondenserEnteringTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Atefe Makhmalbaf and Heejin Cho, PNNL
    //       DATE WRITTEN   March 2012

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the optimal condenser water temperature set point for a chiller plant
    // with one or more chillers.  The condenser water leaving the tower should be at this temperature
    // for optimal operation of the chiller plant.

    // METHODOLOGY EMPLOYED:
    // using one curve to determine the optimum condenser entering water temperature for a given timestep
    // and two other curves to place boundary conditions on the optimal setpoint value.

    // Using/Aliasing
    using namespace DataPlant;

    auto &dspm = state.dataSetPointManager;

    // Current timestep's condenser water entering setpoint
    Real64 CondenserEnteringTempSetPoint = this->condenserEnteringTempSched->getCurrentVal();

    auto &supplyLoop = state.dataPlnt->PlantLoop(this->plantPloc.loopNum).LoopSide(LoopSideLocation::Supply);
    auto &supplyComp = supplyLoop.Branch(this->plantPloc.branchNum).Comp(this->plantPloc.compNum);

    auto &demandLoop = state.dataPlnt->PlantLoop(this->demandPloc.loopNum).LoopSide(LoopSideLocation::Demand);
    auto &demandComp = demandLoop.Branch(this->demandPloc.branchNum).Comp(this->demandPloc.compNum);

    // If chiller is on
    Real64 CurLoad = std::abs(supplyComp.MyLoad);
    if (CurLoad > 0) {

        Real64 CondInletTemp = 0.0;
        Real64 EvapOutletTemp = 0.0;

        Real64 DesignLoad = 0.0;            // array of chiller design loads
        Real64 ActualLoad = 0.0;            // array of chiller actual loads
        Real64 DesignCondenserInTemp = 0.0; // Design condenser inlet temp. C , or 25.d0
        Real64 DesignEvapOutTemp = 0.0;     // design evaporator outlet temperature, water side

        // Get from tower design values
        constexpr Real64 NormDesignCondenserFlow = 5.38e-8; // m3/s per watt (typically 3 gpm/ton)=(Volume of condenser fluid)/(ton of heat rejection)

        if (this->chillerType == PlantEquipmentType::Chiller_Absorption || this->chillerType == PlantEquipmentType::Chiller_CombTurbine ||
            this->chillerType == PlantEquipmentType::Chiller_Electric || this->chillerType == PlantEquipmentType::Chiller_ElectricReformEIR ||
            this->chillerType == PlantEquipmentType::Chiller_EngineDriven) {
            DesignCondenserInTemp = supplyComp.TempDesCondIn;
            CondInletTemp = state.dataLoopNodes->Node(demandComp.NodeNumIn).Temp;
            EvapOutletTemp = state.dataLoopNodes->Node(supplyComp.NodeNumOut).Temp;
            DesignEvapOutTemp = supplyComp.TempDesEvapOut;
            DesignLoad = supplyComp.MaxLoad;
            ActualLoad = state.dataPlnt->PlantLoop(this->plantPloc.loopNum).CoolingDemand;
        } else if (this->chillerType == PlantEquipmentType::Chiller_Indirect_Absorption ||
                   this->chillerType == PlantEquipmentType::Chiller_DFAbsorption) {
            DesignCondenserInTemp = supplyComp.TempDesCondIn;
            DesignEvapOutTemp = 6.666;
        } else {
            DesignCondenserInTemp = 25.0;
            DesignEvapOutTemp = 6.666;
        }

        // for attached chillers (that are running this timestep) find their Dsn_MinCondSetpt and Dsn_EntCondTemp
        dspm->CET_DesignMinCondenserSetPt = 999.0;
        dspm->CET_DesignEnteringCondenserTemp = 0.0;

        // Design Minimum Condenser Entering as a function of the minimum lift and TEvapLvg
        // for chillers operating on current cond loop this timestep
        Real64 DesignMinCondenserEnteringTempThisChiller = DesignEvapOutTemp + (this->minLift);
        dspm->CET_DesignMinCondenserSetPt = min(dspm->CET_DesignMinCondenserSetPt, DesignMinCondenserEnteringTempThisChiller);

        // Design entering condenser water temperature for chillers operating
        // on current cond loop this timestep
        dspm->CET_DesignEnteringCondenserTemp = max(dspm->CET_DesignEnteringCondenserTemp, DesignCondenserInTemp);

        // ***** Load Calculations *****
        // In this section the sum of the actual load (watts) and design load (watts)
        // of the chillers that are on is calculated.
        dspm->CET_ActualLoadSum += ActualLoad;
        dspm->CET_DesignLoadSum += DesignLoad;

        // Exit if the chillers are all off this hour
        if (dspm->CET_ActualLoadSum <= 0) {
            CondenserEnteringTempSetPoint = dspm->CET_DesignEnteringCondenserTemp;
            return;
        }

        // ***** Weighted Ratio Calculation *****
        // This section first calculates the actual (ALW) and design (DLW) individual
        // weights. Then the weighted actual and design loads are computed. Finally
        // the Weighted Ratio is found.
        Real64 WeightedActualLoad = 0.0; // Actual load weighting of each chiller, W
        Real64 WeightedDesignLoad = 0.0; // Design capacity of each chiller, W
        if (dspm->CET_ActualLoadSum != 0 && dspm->CET_DesignLoadSum != 0) {
            WeightedActualLoad = ((ActualLoad / dspm->CET_ActualLoadSum) * ActualLoad);
            WeightedDesignLoad = ((DesignLoad / dspm->CET_DesignLoadSum) * DesignLoad);
        }

        dspm->CET_WeightedActualLoadSum += WeightedActualLoad;
        dspm->CET_WeightedDesignLoadSum += WeightedDesignLoad;
        dspm->CET_WeightedLoadRatio = dspm->CET_WeightedActualLoadSum / dspm->CET_WeightedDesignLoadSum;

        // ***** Optimal Temperature Calculation *****
        // In this section the optimal temperature is computed along with the minimum
        // design wet bulb temp and the minimum actual wet bulb temp.
        // Min_DesignWB = ACoef1 + ACoef2*OaWb + ACoef3*WPLR + ACoef4*TwrDsnWB + ACoef5*NF
        dspm->CET_DesignMinWetBulbTemp = EnergyPlus::Curve::CurveValue(state,
                                                                       this->minTowerDesignWetBulbCurveNum,
                                                                       state.dataEnvrn->OutWetBulbTemp,
                                                                       dspm->CET_WeightedLoadRatio,
                                                                       this->towerDesignInletAirWetBulbTemp,
                                                                       NormDesignCondenserFlow);

        // Min_ActualWb = BCoef1 + BCoef2*MinDsnWB + BCoef3*WPLR + BCoef4*TwrDsnWB + BCoef5*NF
        dspm->CET_MinActualWetBulbTemp = EnergyPlus::Curve::CurveValue(state,
                                                                       this->minOAWetBulbCurveNum,
                                                                       dspm->CET_DesignMinWetBulbTemp,
                                                                       dspm->CET_WeightedLoadRatio,
                                                                       this->towerDesignInletAirWetBulbTemp,
                                                                       NormDesignCondenserFlow);

        // Opt_CondEntTemp = CCoef1 + CCoef2*OaWb + CCoef3*WPLR + CCoef4*TwrDsnWB + CCoef5*NF
        dspm->CET_OptCondenserEnteringTemp = EnergyPlus::Curve::CurveValue(state,
                                                                           this->optCondenserEnteringTempCurveNum,
                                                                           state.dataEnvrn->OutWetBulbTemp,
                                                                           dspm->CET_WeightedLoadRatio,
                                                                           this->towerDesignInletAirWetBulbTemp,
                                                                           NormDesignCondenserFlow);

        // ***** Calculate (Cond ent - Evap lvg) Section *****
        // In this section we find the worst case of (Cond ent - Evap lvg) for the
        // chillers that are running.
        dspm->CET_CurMinLift = 9999.0;
        // temp_MinLiftTD = 20.0 / 1.8;
        Real64 TempMinLift = CondInletTemp - EvapOutletTemp;
        dspm->CET_CurMinLift = min(dspm->CET_CurMinLift, TempMinLift);
    }

    Real64 SetPoint = 0.0; // Condenser entering water temperature setpoint this timestep, C

    // ***** Limit conditions Section *****
    // Check for limit conditions and control to the proper value.
    if ((dspm->CET_WeightedLoadRatio >= 0.90) && (dspm->CET_OptCondenserEnteringTemp >= (dspm->CET_DesignEnteringCondenserTemp + 1.0))) {
        // Optimized value exceeds the design condenser entering condition or chillers
        // near full load condition; reset condenser entering setpoint to its design value
        SetPoint = dspm->CET_DesignEnteringCondenserTemp + 1.0;
    } else if ((state.dataEnvrn->OutWetBulbTemp >= dspm->CET_MinActualWetBulbTemp) &&
               (this->towerDesignInletAirWetBulbTemp >= dspm->CET_DesignMinWetBulbTemp) && (dspm->CET_CurMinLift > this->minLift)) {
        // Boundaries are satisfied; use optimized condenser entering water temp
        SetPoint = dspm->CET_OptCondenserEnteringTemp;
    } else {
        // Boundaries violated; Reset to scheduled value of condenser water entering setpoint
        SetPoint = CondenserEnteringTempSetPoint;
    }

    // Do not allow new setpoint to be less than the design condenser minimum entering condition,
    // i.e., TCondWaterEnt not allowed to be less than DsnEvapWaterLvg + MinimumLiftTD
    this->setPt = max(SetPoint, dspm->CET_DesignMinCondenserSetPt);
} // SPMCondenserEneteringTemp::calculate()

void SPMIdealCondenserEnteringTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Heejin Cho, PNNL
    //       DATE WRITTEN   March 2012

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the optimal condenser water entering temperature set point for a chiller plant.

    // METHODOLOGY EMPLOYED:
    // The "ideal" chiller-tower optimization scheme uses a search algorithm to find the ideal optimal setpoint
    // at a given timestep. This requires resimulating HVAC systems at each timestep until finding
    // an "optimal" condenser water entering setpoint (OptSetpoint) which gives the minimum total chiller,
    // cooling tower, chilled water pump and condenser water pump power consumption.
    // The OptSetpoint falls between realistic minimum and maximum boundaries, which are set by the user.
    // The minimum boundary is determined based on the minimum lift (user input)
    // and evaporator leaving water temperature. The maximum boundary is specified by the user.
    // It is assumed that a single minimum point exists between these boundaries.

    // Using/Aliasing
    using namespace DataPlant;

    auto &dspm = state.dataSetPointManager;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &supplyLoop = state.dataPlnt->PlantLoop(this->chillerPloc.loopNum).LoopSide(LoopSideLocation::Supply);
    auto &supplyComp = supplyLoop.Branch(this->chillerPloc.branchNum).Comp(this->chillerPloc.compNum);

    if (state.dataGlobal->MetersHaveBeenInitialized) {
        // Setup meter vars
        if (this->setupIdealCondEntSetPtVars) {
            this->SetupMeteredVarsForSetPt(state);
            this->setupIdealCondEntSetPtVars = false;
        }
    }

    if (state.dataGlobal->MetersHaveBeenInitialized && state.dataGlobal->RunOptCondEntTemp) {

        // If chiller is on
        Real64 CurLoad = std::abs(supplyComp.MyLoad);

        if (CurLoad > 0) {

            Real64 EvapOutletTemp =
                (this->chillerType == PlantEquipmentType::Chiller_Absorption || this->chillerType == PlantEquipmentType::Chiller_CombTurbine ||
                 this->chillerType == PlantEquipmentType::Chiller_Electric || this->chillerType == PlantEquipmentType::Chiller_ElectricReformEIR ||
                 this->chillerType == PlantEquipmentType::Chiller_EngineDriven)
                    ? state.dataLoopNodes->Node(supplyComp.NodeNumOut).Temp
                    : 6.666;

            Real64 CondTempLimit = this->minLift + EvapOutletTemp;

            Real64 TotEnergy = this->calculateCurrentEnergyUsage(state);

            this->setupSetPointAndFlags(TotEnergy,
                                        dspm->ICET_TotEnergyPre,
                                        dspm->ICET_CondenserWaterSetPt,
                                        CondTempLimit,
                                        state.dataGlobal->RunOptCondEntTemp,
                                        dspm->ICET_RunSubOptCondEntTemp,
                                        dspm->ICET_RunFinalOptCondEntTemp);

        } else {
            dspm->ICET_CondenserWaterSetPt = this->maxCondenserEnteringTemp;
            dspm->ICET_TotEnergyPre = 0.0;
            state.dataGlobal->RunOptCondEntTemp = false;
            dspm->ICET_RunSubOptCondEntTemp = false;
        }
    } else {
        dspm->ICET_CondenserWaterSetPt = this->maxCondenserEnteringTemp;
        state.dataGlobal->RunOptCondEntTemp = false;
        dspm->ICET_RunSubOptCondEntTemp = false;
    }

    this->setPt = dspm->ICET_CondenserWaterSetPt;
} // SPMIdealCondenserEnteringTemp::calculate()

void SPMIdealCondenserEnteringTemp::setupSetPointAndFlags(Real64 &TotEnergy,
                                                          Real64 &TotEnergyPre,
                                                          Real64 &CondWaterSetPoint,
                                                          Real64 &CondTempLimit,
                                                          bool &RunOptCondEntTemp,
                                                          bool &RunSubOptCondEntTemp,
                                                          bool &RunFinalOptCondEntTemp) const
{
    Real64 DeltaTotEnergy;
    if (TotEnergyPre != 0.0) {
        // Calculate the total energy consumption difference
        DeltaTotEnergy = TotEnergyPre - TotEnergy;
        // Search for the minimum total energy consumption
        if ((DeltaTotEnergy > 0) && (CondWaterSetPoint >= CondTempLimit) && (!RunFinalOptCondEntTemp)) {
            if (!RunSubOptCondEntTemp) {
                --CondWaterSetPoint;
                RunOptCondEntTemp = true;
            } else {
                CondWaterSetPoint -= 0.2;
                RunOptCondEntTemp = true;
            }
            TotEnergyPre = TotEnergy;
            // Set smaller set point (0.2 degC) decrease
        } else if ((DeltaTotEnergy < 0) && (!RunSubOptCondEntTemp) && (CondWaterSetPoint > CondTempLimit) && (!RunFinalOptCondEntTemp)) {
            CondWaterSetPoint += 0.8;
            RunOptCondEntTemp = true;
            RunSubOptCondEntTemp = true;
        } else {
            if (!RunFinalOptCondEntTemp) {
                CondWaterSetPoint += 0.2;
                RunOptCondEntTemp = true;
                RunSubOptCondEntTemp = false;
                RunFinalOptCondEntTemp = true;
            } else {
                // CondWaterSetPoint = CondWaterSetPoint; // Self-assignment commented out
                TotEnergyPre = 0.0;
                RunOptCondEntTemp = false;
                RunSubOptCondEntTemp = false;
                RunFinalOptCondEntTemp = false;
            }
        }
    } else {
        CondWaterSetPoint = this->maxCondenserEnteringTemp - 1.0;
        TotEnergyPre = TotEnergy;
        RunOptCondEntTemp = true;
        RunSubOptCondEntTemp = false;
    }
} // SPMIdealCondenserEneteringTemp::()

Real64 SPMIdealCondenserEnteringTemp::calculateCurrentEnergyUsage(EnergyPlusData &state)
{
    Real64 ChillerEnergy = GetInternalVariableValue(state, this->chillerVar.Type, this->chillerVar.Num);
    Real64 ChilledPumpEnergy = GetInternalVariableValue(state, this->chilledWaterPumpVar.Type, this->chilledWaterPumpVar.Num);
    Real64 TowerFanEnergy = 0;
    for (int i = 1; i <= this->numTowers; i++) {
        TowerFanEnergy += GetInternalVariableValue(state, this->towerVars(i).Type, this->towerVars(i).Num);
    }
    Real64 CondPumpEnergy = GetInternalVariableValue(state, this->condenserPumpVar.Type, this->condenserPumpVar.Num);
    return (ChillerEnergy + ChilledPumpEnergy + TowerFanEnergy + CondPumpEnergy);
} // SPMIdealCondenserEnteringTemp::calculateCurrentEnergyUsage()

void SPMReturnWaterTemp::calculate(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Edwin Lee, NREL
    //       DATE WRITTEN   May 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the plant supply temperature reset required to achieve a target plant return temperature

    // METHODOLOGY EMPLOYED:
    // The setpoint manager follows this procedure:
    //  1. Calculate the current demand
    //    a. Sense the current return temperature
    //    b. Sense the current supply temperature
    //    c. Sense the current flow rate
    //    d. Approximate the fluid properties (rho, Cp) from the temperatures
    //    ---> Use these to calculate the demand with Q_demand = V_dot * rho * C_p * (T_return_sensed - T_supply_sensed)
    //  2. Calculate a new value of supply setpoint that will reject this much Q_demand, while providing a target return temperature
    //    * this assumes that the demand will be the same value on the next time around
    //    * at any time step, the value of target return temperature may vary if it is scheduled (or actuated with EMS)
    //    a. T_supply_setpoint = T_return_target - Q_demand / ( V_dot * rho * C_p )
    //  3. Constrain this value to limits
    //    a. T_supply_setpoint will be within: [ Design Chilled Water Supply Temperature, Maximum Supply Water Reset Temperature ]

    // NOTES:
    // The assumptions related to lagging of setpoint are most suited for smaller timesteps and/or plants that don't vary wildly from one time
    // step to another The assumptions also become affected by variable flow plants more-so than constant-flow plants

    // Using/Aliasing
    using namespace DataPlant;

    auto &supplyNode = state.dataLoopNodes->Node(this->supplyNodeNum);
    auto &returnNode = state.dataLoopNodes->Node(this->returnNodeNum);

    // we need to know the plant to get the fluid ID in case it is glycol
    // but we have to wait in case plant isn't initialized yet
    // if plant isn't initialized, assume index=1 (water)
    if (this->plantLoopNum == 0) {
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            auto &plantLoop = state.dataPlnt->PlantLoop(LoopNum);
            if (this->supplyNodeNum == plantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut) {
                this->plantLoopNum = LoopNum;
                this->plantSetPtNodeNum = plantLoop.TempSetPointNodeNum;
                // now that we've found the plant populated, let's verify that the nodes match
                if (!PlantUtilities::verifyTwoNodeNumsOnSamePlantLoop(state, this->supplyNodeNum, this->returnNodeNum)) {
                    ShowSevereError(state, "Node problem for SetpointManager:ReturnTemperature:ChilledWater.");
                    ShowContinueError(state, "Return and Supply nodes were not found on the same plant loop.  Verify node names.");
                    ShowFatalError(state, "Simulation aborts due to setpoint node problem");
                }
            }
        }
    }

    // get the operating flow rate
    Real64 const mdot = supplyNode.MassFlowRate;
    Real64 const deltaT = (this->type == SPMType::ChilledWaterReturnTemp) ? (returnNode.Temp - supplyNode.Temp) : (supplyNode.Temp - returnNode.Temp);

    // // calculate the current demand
    // fluidIndex = state.dataPlnt->PlantLoop(this->plantLoopNum).FluidIndex;
    // // we don't need fluid names since we have a real index, so just pass in the temperature and get properties
    // Real64 const avgTemp = (returnNode.Temp + supplyNode.Temp) / 2;
    // Real64 const cp = Fluid::GetSpecificHeatGlycol(state, "", avgTemp, fluidIndex, "ReturnWaterChWSetPointManager::calculate");
    // Real64 const Qdemand = mdot * cp * deltaT;

    // check for strange conditions
    if (deltaT < 0) {
        this->currentSupplySetPt = (this->type == SPMType::ChilledWaterReturnTemp) ? this->minSetTemp : this->maxSetTemp;
        return;
    }

    // Determine a return target, default is to use the constant value, but scheduled or externally
    //  set on the return node TempSetPoint will overwrite it.  Note that the schedule index is only
    //  greater than zero if the input type is scheduled, and the useReturnTempSetpoint flag is only
    //  true if the input type is specified as such
    Real64 T_return_target = this->returnTempConstantTarget;
    if (this->returnTempSched != nullptr) {
        T_return_target = this->returnTempSched->getCurrentVal();
    } else if (this->returnTempType == ReturnTempType::Setpoint) {
        if (returnNode.TempSetPoint != SensedNodeFlagValue) {
            T_return_target = returnNode.TempSetPoint;
        } else {
            ShowSevereError(state, "Return temperature reset setpoint manager encountered an error.");
            ShowContinueError(state,
                              "The manager is specified to look to the return node setpoint to find a target return temperature, but the node "
                              "setpoint was invalid");
            ShowContinueError(state,
                              format("Verify that a separate setpoint manager is specified to set the setpoint on the return node named \"{}\"",
                                     state.dataLoopNodes->NodeID(this->returnNodeNum)));
            ShowContinueError(state, "Or change the target return temperature input type to constant or scheduled");
            ShowFatalError(state, "Missing reference setpoint");
        }
    }

    // calculate the supply setpoint to use, default to the design value if flow is zero
    Real64 T_supply_setpoint = (this->type == SPMType::ChilledWaterReturnTemp) ? this->minSetTemp : this->maxSetTemp;
    if (mdot > DataConvergParams::PlantFlowRateToler) {
        T_supply_setpoint = T_return_target + ((this->type == SPMType::ChilledWaterReturnTemp) ? -deltaT : deltaT);
    }

    this->currentSupplySetPt = std::clamp(T_supply_setpoint, this->minSetTemp, this->maxSetTemp);
} // SPMReturnWaterTemp::calculate()

void SPMIdealCondenserEnteringTemp::SetupMeteredVarsForSetPt(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Sep 2013

    // PURPOSE OF THIS SUBROUTINE:
    // For the Ideal Cond reset setpoint manager, this sets up the
    // report variables used during the calculation.

    // Using/Aliasing
    using namespace DataPlant;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D<OutputProcessor::MeteredVar> meteredVars;

    auto &plantLoop = state.dataPlnt->PlantLoop(this->chillerPloc.loopNum);
    auto &supplySide = plantLoop.LoopSide(this->chillerPloc.loopSideNum);
    auto &chillerBranch = supplySide.Branch(this->chillerPloc.branchNum);
    auto &chillerComp = chillerBranch.Comp(this->chillerPloc.compNum);

    int NumVariables = GetNumMeteredVariables(state, chillerComp.TypeOf, chillerComp.Name);
    meteredVars.allocate(NumVariables);

    GetMeteredVariables(state, chillerComp.Name, meteredVars);
    this->chillerVar.Type = meteredVars(1).varType;
    this->chillerVar.Num = meteredVars(1).num;

    auto &chilledWaterPumpBranch = supplySide.Branch(this->chilledWaterPumpPloc.branchNum);
    auto &chilledWaterPumpComp = chilledWaterPumpBranch.Comp(this->chilledWaterPumpPloc.compNum);

    NumVariables = GetNumMeteredVariables(state, chilledWaterPumpComp.TypeOf, chilledWaterPumpComp.Name);
    meteredVars.allocate(NumVariables);

    GetMeteredVariables(state, chilledWaterPumpComp.Name, meteredVars);
    this->chilledWaterPumpVar.Type = meteredVars(1).varType;
    this->chilledWaterPumpVar.Num = meteredVars(1).num;

    auto &towerLoopSide = state.dataPlnt->PlantLoop(this->towerPlocs(1).loopNum).LoopSide(this->towerPlocs(1).loopSideNum);

    for (int i = 1; i <= this->numTowers; i++) {
        auto &towerComp = towerLoopSide.Branch(this->towerPlocs(i).branchNum).Comp(this->towerPlocs(i).compNum);
        NumVariables = GetNumMeteredVariables(state, towerComp.TypeOf, towerComp.Name);
        meteredVars.allocate(NumVariables);

        GetMeteredVariables(state, towerComp.Name, meteredVars);
        this->towerVars.push_back({meteredVars(1).varType, meteredVars(1).num});
    }

    auto &condenserPumpComp = towerLoopSide.Branch(this->condenserPumpPloc.branchNum).Comp(this->condenserPumpPloc.compNum);
    NumVariables = GetNumMeteredVariables(state, condenserPumpComp.TypeOf, condenserPumpComp.Name);
    meteredVars.allocate(NumVariables);

    GetMeteredVariables(state, condenserPumpComp.Name, meteredVars);
    this->condenserPumpVar = {meteredVars(1).varType, meteredVars(1).num};
} // SPMIdealCondenserEnteringTemp::SetupMeteredVarsForSetPt()

void SPMSystemNode::calculate(EnergyPlusData &state)
{
    Real64 RefValue = 0; // Reference value from the Reference node

    auto &refNode = state.dataLoopNodes->Node(this->refNodeNum);

    switch (this->ctrlVar) {
    case HVAC::CtrlVarType::Temp:
    case HVAC::CtrlVarType::MaxTemp:
    case HVAC::CtrlVarType::MinTemp: {
        RefValue = refNode.Temp;
    } break;
    case HVAC::CtrlVarType::HumRat:
    case HVAC::CtrlVarType::MaxHumRat:
    case HVAC::CtrlVarType::MinHumRat: {
        RefValue = refNode.HumRat;
    } break;
    default:
        break;
    }

    this->setPt = interpSetPoint(this->lowRef, this->highRef, RefValue, this->lowRefSetPt, this->highRefSetPt);
} // SPMSystemNode::calculate()

Real64 interpSetPoint(Real64 const LowVal, Real64 const HighVal, Real64 const RefVal, Real64 const SetptAtLowVal, Real64 const SetptAtHighVal)
{
    if (LowVal >= HighVal) {
        return 0.5 * (SetptAtLowVal + SetptAtHighVal);
    } else if (RefVal <= LowVal) {
        return SetptAtLowVal;
    } else if (RefVal >= HighVal) {
        return SetptAtHighVal;
    } else {
        return SetptAtLowVal - ((RefVal - LowVal) / (HighVal - LowVal)) * (SetptAtLowVal - SetptAtHighVal);
    }
}

void UpdateSetPointManagers(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   July 1998
    //       MODIFIED       Shirey/Raustad (FSEC), Jan 2004
    //                      P. Haves Oct 2004
    //                        Add new setpoint managers:
    //                          SET POINT MANAGER:WARMEST TEMP FLOW and
    //                          SET POINT MANAGER:COLDEST TEMP FLOW
    //                      Nov 2004 M. J. Witte, GARD Analytics, Inc.
    //                        Add new setpoint managers:
    //                          SET POINT MANAGER:SINGLE ZONE HEATING and
    //                          SET POINT MANAGER:SINGLE ZONE COOLING
    //                        Work supported by ASHRAE research project 1254-RP
    //                      B. Griffith Aug. 2006.  Allow HUMRAT for scheduled setpoint manager
    //                      P. Haves Aug 2007
    //                        SET POINT MANAGER:WARMEST TEMP FLOW:
    //                          Set AirLoopControlInfo()%LoopFlowRateSet every call not just on
    //                          initialization (flag now reset in SUBROUTINE ResetHVACControl)
    //                        Removed SET POINT MANAGER:COLDEST TEMP FLOW
    //                      July 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers
    //                          SetpointManager:MultiZone:Heating:Average
    //                          SetpointManager:MultiZone:Cooling:Average
    //                          SetpointManager:MultiZone:MinimumHumidity:Average
    //                          SetpointManager:MultiZone:MaximumHumidity:Average
    //                      Aug 2010 B.A. Nigusse, FSEC/UCF
    //                        Added new setpoint managers:
    //                          SetpointManager:MultiZone:Humidity:Minimum
    //                          SetpointManager:MultiZone:Humidity:Maximum
    //                      Aug 2014 Rick Strand, UIUC
    //                          SetpointManager:ScheduledTES (internally defined)
    //                      Jan 2022 Wooyoung Jung, Jeremy Lerond and Jian Zhang, PNNL
    //                        Added new setpoint managers:
    //                          SetpointManager:SystemNodeReset:Temperature
    //                          SetpointManager:SystemNodeReset:Humidity

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the Setpoint Managers and use their output arrays
    // to set the node setpoints.

    // Using/Aliasing
    using EMSManager::CheckIfNodeSetPointManagedByEMS;

    // Loop over all the Scheduled Setpoint Managers
    for (auto *spm : state.dataSetPointManager->spms) {

        switch (spm->type) {

        case SPMType::Scheduled:
        case SPMType::SystemNodeTemp:
        case SPMType::SystemNodeHum: {
            for (int ctrlNodeNum : spm->ctrlNodeNums) {
                auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                switch (spm->ctrlVar) {
                // set the setpoint depending on the type of variable being controlled
                case HVAC::CtrlVarType::Temp: {
                    node.TempSetPoint = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MaxTemp: {
                    node.TempSetPointHi = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MinTemp: {
                    node.TempSetPointLo = spm->setPt;
                } break;
                case HVAC::CtrlVarType::HumRat: {
                    node.HumRatSetPoint = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MaxHumRat: {
                    node.HumRatMax = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MinHumRat: {
                    node.HumRatMin = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MassFlowRate: {
                    node.MassFlowRateSetPoint = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MaxMassFlowRate: {
                    node.MassFlowRateMax = spm->setPt;
                } break;
                case HVAC::CtrlVarType::MinMassFlowRate: {
                    node.MassFlowRateMin = spm->setPt;
                } break;
                default:
                    break;
                }
            } // for (CtrlNodeNum)
        } break;

        case SPMType::TESScheduled: {
            auto *spmTESS = dynamic_cast<SPMTESScheduled *>(spm);
            assert(spmTESS != nullptr);

            state.dataLoopNodes->Node(spmTESS->ctrlNodeNum).TempSetPoint = spm->setPt;
        } break;

        case SPMType::ScheduledDual: {
            auto *spmSD = dynamic_cast<SPMScheduledDual *>(spm);
            assert(spmSD != nullptr);

            if (spmSD->ctrlVar == HVAC::CtrlVarType::Temp) {
                for (int ctrlNodeNum : spmSD->ctrlNodeNums) {
                    auto &node = state.dataLoopNodes->Node(ctrlNodeNum);

                    node.TempSetPointHi = spmSD->setPtHi;                                  // Set the setpoint High
                    node.TempSetPointLo = spmSD->setPtLo;                                  // Set the setpoint Low
                    node.TempSetPoint = (node.TempSetPointHi + node.TempSetPointLo) / 2.0; // average of the high and low
                }
            }
        } break;

        case SPMType::OutsideAir:
        case SPMType::FollowOutsideAirTemp:
        case SPMType::FollowSystemNodeTemp:
        case SPMType::FollowGroundTemp: {
            for (int ctrlNodeNum : spm->ctrlNodeNums) {
                auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
                if (spm->ctrlVar == HVAC::CtrlVarType::Temp) {
                    node.TempSetPoint = spm->setPt;
                } else if (spm->ctrlVar == HVAC::CtrlVarType::MaxTemp) {
                    node.TempSetPointHi = spm->setPt;
                } else if (spm->ctrlVar == HVAC::CtrlVarType::MinTemp) {
                    node.TempSetPointLo = spm->setPt;
                }
            }

        } break;

        case SPMType::SZReheat:
        case SPMType::SZHeating:
        case SPMType::SZCooling:
        case SPMType::Warmest:
        case SPMType::Coldest:
        case SPMType::MZCoolingAverage:
        case SPMType::MZHeatingAverage:
        case SPMType::CondenserEnteringTemp:
        case SPMType::IdealCondenserEnteringTemp:
        case SPMType::SZOneStageCooling:
        case SPMType::SZOneStageHeating: {
            if (spm->ctrlVar == HVAC::CtrlVarType::Temp) {
                for (int ctrlNodeNum : spm->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spm->setPt; // Set the setpoint
                }
            }
        } break;

        case SPMType::SZMinHum:
        case SPMType::MZMinHumAverage:
        case SPMType::MZMinHum: {
            for (int ctrlNodeNum : spm->ctrlNodeNums) {
                if (spm->type == SPMType::SZMinHum || spm->ctrlVar == HVAC::CtrlVarType::MinHumRat) { // Why is SZMinHum not tested for this?
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMin = spm->setPt;
                }
            }
        } break;

        case SPMType::SZMaxHum:
        case SPMType::MZMaxHumAverage:
        case SPMType::MZMaxHum: {
            for (int ctrlNodeNum : spm->ctrlNodeNums) {
                if (spm->type == SPMType::SZMaxHum || spm->ctrlVar == HVAC::CtrlVarType::MaxHumRat) { // Why is SZMaxHum not tested for this?
                    state.dataLoopNodes->Node(ctrlNodeNum).HumRatMax = spm->setPt;
                }
            }
        } break;

        case SPMType::WarmestTempFlow: {
            auto *spmWTF = dynamic_cast<SPMWarmestTempFlow *>(spm);
            assert(spmWTF != nullptr);

            if (spmWTF->ctrlVar == HVAC::CtrlVarType::Temp) {
                for (int ctrlNodeNum : spmWTF->ctrlNodeNums) {
                    state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spmWTF->setPt; // Set the supply air temperature setpoint
                }

                state.dataAirLoop->AirLoopFlow(spmWTF->airLoopNum).ReqSupplyFrac = spmWTF->turndown; // Set the supply air flow rate
                state.dataAirLoop->AirLoopControlInfo(spmWTF->airLoopNum).LoopFlowRateSet = true;    // PH 8/17/07
            }
        } break;

        case SPMType::ReturnAirBypass: {
            auto *spmRAB = dynamic_cast<SPMReturnAirBypassFlow *>(spm);
            assert(spmRAB != nullptr);

            if (spmRAB->ctrlVar == HVAC::CtrlVarType::MassFlowRate) {
                state.dataLoopNodes->Node(spmRAB->rabSplitOutNodeNum).MassFlowRateSetPoint = spmRAB->FlowSetPt; // Set the flow setpoint
            }
        } break;

        case SPMType::ChilledWaterReturnTemp:
        case SPMType::HotWaterReturnTemp: {
            auto *spmRWT = dynamic_cast<SPMReturnWaterTemp *>(spm);
            assert(spmRWT != nullptr);
            if (spmRWT->plantSetPtNodeNum > 0) {
                state.dataLoopNodes->Node(spmRWT->plantSetPtNodeNum).TempSetPoint = spmRWT->currentSupplySetPt;
            }
        } break;

        // MixedAir and OutsideAirPretreat SPMs have to be handled separately because they depend on other SPMs
        case SPMType::MixedAir:
        case SPMType::OutsideAirPretreat: {
        } break;

        default:
            break;
        } // switch (sys->type)
    } // for (spm)
} // UpdateSetPointManagers()

void UpdateMixedAirSetPoints(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2001

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the Mixed Air Managers and use their output arrays
    // to set the node setpoints.

    for (auto *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::MixedAir) {
            continue;
        }
        if (spm->ctrlVar != HVAC::CtrlVarType::Temp) {
            continue;
        }
        for (int ctrlNodeNum : spm->ctrlNodeNums) {
            state.dataLoopNodes->Node(ctrlNodeNum).TempSetPoint = spm->setPt; // Set the setpoint
        }
    }
} // UpdateMixedAirSetPoints()

void UpdateOAPretreatSetPoints(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. J. Witte based on UpdateMixedAirSetPoints by Fred Buhl,
    //                        Work supported by ASHRAE research project 1254-RP
    //       DATE WRITTEN   January 2005

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the Outside Air Pretreat Managers and use their output arrays
    // to set the node setpoints.

    for (auto *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::OutsideAirPretreat) {
            continue;
        }
        for (int ctrlNodeNum : spm->ctrlNodeNums) {
            auto &node = state.dataLoopNodes->Node(ctrlNodeNum);
            switch (spm->ctrlVar) {
            case HVAC::CtrlVarType::Temp: {
                node.TempSetPoint = spm->setPt;
            } break;
            case HVAC::CtrlVarType::MaxHumRat: {
                node.HumRatMax = spm->setPt;
            } break;
            case HVAC::CtrlVarType::MinHumRat: {
                node.HumRatMin = spm->setPt;
            } break;
            case HVAC::CtrlVarType::HumRat: {
                node.HumRatSetPoint = spm->setPt;
            } break;
            default:
                break;
            }
        }
    }
} // UpdateOutsideAirSetPoints()

int GetSetPointManagerIndexByNode(EnergyPlusData &state, int const NodeNum, HVAC::CtrlVarType const ctrlVar, SPMType const spmType, bool isRefNode)
{

    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (int iSPM = 1; iSPM < (int)state.dataSetPointManager->spms.size(); ++iSPM) {
        auto *spm = state.dataSetPointManager->spms(iSPM);
        if (spm->type != spmType) {
            continue;
        }
        if (spm->ctrlVar != ctrlVar) {
            continue;
        }

        if (isRefNode) {
            if (NodeNum == spm->refNodeNum) {
                return iSPM;
            }
        } else {
            for (int ctrlNodeNum : spm->ctrlNodeNums) {
                if (NodeNum == ctrlNodeNum) {
                    return iSPM;
                }
            }
        }
    }

    return 0;
} // getSPMIndexByNode()

bool IsNodeOnSetPtManager(EnergyPlusData &state, int const NodeNum, HVAC::CtrlVarType const ctrlVar)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Sankaranarayanan K P
    //       DATE WRITTEN   January 2007

    // PURPOSE OF THIS SUBROUTINE:
    // Determines if a particular node is acted upon by a specific setpoint manager

    // METHODOLOGY EMPLOYED:
    // Cycle through all setpoint managers and find if the node passed in has a setpoint manager of passed
    // in type associated to it.
    // Return value

    // First time called, get the input for all the setpoint managers
    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (auto const *spm : state.dataSetPointManager->spms) {
        if (spm->ctrlVar != ctrlVar) {
            continue;
        }
        for (int ctrlNodeNum : spm->ctrlNodeNums) {
            if (NodeNum == ctrlNodeNum) {
                return true;
            }
        }
    }

    return false;
} // IsNodeOnSetPointManager()

bool NodeHasSPMCtrlVarType(EnergyPlusData &state, int const NodeNum, HVAC::CtrlVarType const ctrlVar)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Chandan Sharma
    //       DATE WRITTEN   March 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Determines if a particular node is acted upon by a specific setpoint manager

    // METHODOLOGY EMPLOYED:
    // Cycle through all setpoint managers and find if the node has a specific control type

    // First time called, get the input for all the setpoint managers
    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (auto const *spm : state.dataSetPointManager->spms) {
        if (spm->ctrlVar != ctrlVar) {
            continue;
        }
        for (int ctrlNodeNum : spm->ctrlNodeNums) {
            if (NodeNum == ctrlNodeNum) {
                return true;
            }
        }
    }

    return false;
} // NodeHasSPMCtrlVarType()

void ResetHumidityRatioCtrlVarType(EnergyPlusData &state, int const NodeNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   August 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Resets setpoint control variable type to "Maximum Humidity Ratio" if control variable type
    // is "Humidity Ratio".

    // METHODOLOGY EMPLOYED:
    // Cycle through all setpoint managers and find if the node has a "Humidity Ratio" control
    // variable type. This routine is called from "GetControllerInput" routine.  This reset is
    // just to stop false warning message due to control variable type mismatch.

    // First time called, get the input for all the setpoint managers
    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (auto *spm : state.dataSetPointManager->spms) {
        if (spm->ctrlVar != HVAC::CtrlVarType::HumRat) {
            continue;
        }
        for (int ctrlNodeNum : spm->ctrlNodeNums) {
            if (NodeNum != ctrlNodeNum) {
                continue;
            }

            spm->ctrlVar = HVAC::CtrlVarType::MaxHumRat;
            ShowWarningError(state, format("ResetHumidityRatioCtrlVarType: {}=\"{}\". ", spmTypeNames[(int)spm->type], spm->Name));
            ShowContinueError(state, " ..Humidity ratio control variable type specified is = HumidityRatio");
            ShowContinueError(state, " ..Humidity ratio control variable type allowed with water coils is = MaximumHumidityRatio");
            ShowContinueError(state, " ..Setpointmanager control variable type is reset to = MaximumHumidityRatio");
            ShowContinueError(state, " ..Simulation continues. ");
            return;
        }
    }
} // ResetHumidityRatioCtrlVarType()

void CheckIfAnyIdealCondEntSetPoint(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Heejin Cho, PNNL
    //       DATE WRITTEN   March 2012

    // PURPOSE OF THIS SUBROUTINE:
    // Determine if ideal condenser entering set point manager is used in model and set flag

    state.dataGlobal->AnyIdealCondEntSetPointInModel =
        (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "SetpointManager:CondenserEnteringReset:Ideal") > 0);
} // CheckIfAnyIdealCondEntSetPoint()

HVAC::CtrlVarType GetHumidityRatioVariableType(EnergyPlusData &state, int const NodeNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. A. Nigusse
    //       DATE WRITTEN   December 2013

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the humidity setpoint Managers to determine the
    // humidity ratio setpoint type

    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (auto const *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::SZMaxHum && spm->type != SPMType::MZMaxHum && spm->type != SPMType::MZMaxHumAverage) {
            continue;
        }
        if (std::find(spm->ctrlNodeNums.begin(), spm->ctrlNodeNums.end(), NodeNum) != spm->ctrlNodeNums.end()) {
            return HVAC::CtrlVarType::MaxHumRat;
        }
    }

    for (auto const *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::SZMinHum && spm->type != SPMType::MZMinHum && spm->type != SPMType::MZMinHumAverage) {
            continue;
        }
        if (std::find(spm->ctrlNodeNums.begin(), spm->ctrlNodeNums.end(), NodeNum) != spm->ctrlNodeNums.end()) {
            return HVAC::CtrlVarType::MaxHumRat;
        }
    }

    for (auto const *spm : state.dataSetPointManager->spms) {
        if (spm->type != SPMType::Scheduled) {
            continue;
        }
        if (std::find(spm->ctrlNodeNums.begin(), spm->ctrlNodeNums.end(), NodeNum) != spm->ctrlNodeNums.end()) {
            if (spm->ctrlVar == HVAC::CtrlVarType::HumRat || spm->ctrlVar == HVAC::CtrlVarType::MaxHumRat) {
                return spm->ctrlVar;
            }
        }
    }

    return HVAC::CtrlVarType::HumRat;
} // GetHumidityRatioVariableType()

void SetUpNewScheduledTESSetPtMgr(EnergyPlusData &state,
                                  Sched::Schedule *sched,
                                  Sched::Schedule *chargeSched,
                                  Real64 NonChargeCHWTemp,
                                  Real64 ChargeCHWTemp,
                                  DataPlant::CtrlType CompOpType,
                                  int const ControlNodeNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2014

    // PURPOSE OF THIS SUBROUTINE
    // Set up new scheduled TES setpoint managers based on plant control Simple TES

    // METHODOLOGY EMPLOYED:
    // Set up internally created scheduled setpoint managers to control the setpoints
    // of various ice storage equipment with the user having to do this manually.  The
    // point is to provide a simpler input description and take care of logic internally.

    auto *spm = new SPMTESScheduled;

    // Set up the scheduled TES setpoint manager information
    spm->Name = format("TES Scheduled {}", state.dataSetPointManager->spms.size());
    state.dataSetPointManager->spms.push_back(spm);
    state.dataSetPointManager->spmMap.insert_or_assign(spm->Name, state.dataSetPointManager->spms.size());

    spm->sched = sched;
    spm->chargeSched = chargeSched;
    spm->nonChargeCHWTemp = NonChargeCHWTemp;
    spm->chargeCHWTemp = ChargeCHWTemp;
    spm->compOpType = CompOpType;
    spm->ctrlNodeNum = ControlNodeNum;

    // Set up the all setpoint manager information for "verification" that no other setpoint manager controls the node that this new ones does
    spm->ctrlNodeNums.push_back(spm->ctrlNodeNum);
    spm->type = SPMType::TESScheduled;
    spm->ctrlVar = HVAC::CtrlVarType::Temp;

    // Now verify that there is no overlap (no other SPM uses the node of the new setpoint manager)
    bool ErrorsFoundinTESSchSetup = false;
    VerifySetPointManagers(state, ErrorsFoundinTESSchSetup);
    if (ErrorsFoundinTESSchSetup) {
        ShowFatalError(state, "Errors found in verification step of SetUpNewScheduledTESSetPtMgr.  Program terminates.");
    }
    // Since all of the other setpoint managers not only been read and verified but also initialized, simulated, and updated,
    // we must now also initialize, simulate, and update the current SchTESStPtMgr that was just added.  But the init and simulate
    // steps are the same so we can call the simulate first.

    spm->calculate(state);

    // Now update reusing code from Update routine specialized to only doing the current (new) setpoint manager and then we are done
    state.dataLoopNodes->Node(spm->ctrlNodeNum).TempSetPoint = spm->setPt;
} // end of SetUpNewScheduledTESSetPtMgr

bool GetCoilFreezingCheckFlag(EnergyPlusData &state, int const spmNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         L. Gu
    //       DATE WRITTEN   Nov. 2015

    // PURPOSE OF THIS SUBROUTINE
    // Get freezing check status
    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    auto const *spmMA = dynamic_cast<SPMMixedAir *>(state.dataSetPointManager->spms(spmNum));
    assert(spmMA != nullptr);
    return spmMA->freezeCheckEnable;
} // GetCoilFreezingCheckFlag()

int GetMixedAirNumWithCoilFreezingCheck(EnergyPlusData &state, int const MixedAirNode)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         L. Gu
    //       DATE WRITTEN   Nov. 2015

    // PURPOSE OF THIS SUBROUTINE
    // Loop over all the MixedAir setpoint Managers to find coil freezing check flag

    if (state.dataSetPointManager->GetInputFlag) {
        GetSetPointManagerInputs(state);
        state.dataSetPointManager->GetInputFlag = false;
    }

    for (int iSPM = 1; iSPM <= state.dataSetPointManager->spms.isize(); ++iSPM) {
        auto *const spm = state.dataSetPointManager->spms(iSPM);
        if (spm->type != SPMType::MixedAir) {
            continue;
        }

        auto *spmMA = dynamic_cast<SPMMixedAir *>(spm);
        assert(spmMA != nullptr);

        if (std::find(spmMA->ctrlNodeNums.begin(), spmMA->ctrlNodeNums.end(), MixedAirNode) != spmMA->ctrlNodeNums.end() &&
            spmMA->coolCoilInNodeNum > 0 && spmMA->coolCoilOutNodeNum > 0) {
            return iSPM; // Is this really thing we are returning? Not the number of the SPM?  Why?
        }
    }

    return 0;
} // End of GetMixedAirNumWithCoilFreezingCheck()

} // namespace EnergyPlus::SetPointManager
