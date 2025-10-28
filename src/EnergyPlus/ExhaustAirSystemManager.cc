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
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ExhaustAirSystemManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace ExhaustAirSystemManager {
    // Module containing the routines dealing with the AirLoopHVAC:ExhaustSystem

    static constexpr std::array<std::string_view, static_cast<int>(ZoneExhaustControl::FlowControlType::Num)> flowControlTypeNamesUC = {
        "SCHEDULED", "FOLLOWSUPPLY"};

    void SimExhaustAirSystem(EnergyPlusData &state, bool FirstHVACIteration)
    {
        // Obtains and Allocates Mixer related parameters from input file
        if (state.dataExhAirSystemMrg->GetInputFlag) { // First time subroutine has been entered
            GetExhaustAirSystemInput(state);
            state.dataExhAirSystemMrg->GetInputFlag = false;
        }

        for (int ExhaustAirSystemNum = 1; ExhaustAirSystemNum <= state.dataZoneEquip->NumExhaustAirSystems; ++ExhaustAirSystemNum) {
            CalcExhaustAirSystem(state, ExhaustAirSystemNum, FirstHVACIteration);
        }

        // After this, update the exhaust flows according to zone grouping:
        UpdateZoneExhaustControl(state);
    }

    void GetExhaustAirSystemInput(EnergyPlusData &state)
    {
        if (!state.dataExhAirSystemMrg->GetInputFlag) {
            return;
        }
        // state.dataExhAirSystemMrg->GetInputFlag = false;

        // Locals
        bool ErrorsFound = false;

        constexpr std::string_view RoutineName("GetExhaustAirSystemInput: ");
        constexpr std::string_view routineName = "GetExhaustAirSystemInput";
        std::string const cCurrentModuleObject = "AirLoopHVAC:ExhaustSystem";

        auto &ip = state.dataInputProcessing->inputProcessor;
        auto const instances = ip->epJSON.find(cCurrentModuleObject);
        if (instances != ip->epJSON.end()) {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
            auto &instancesValue = instances.value();
            int numExhaustSystems = instancesValue.size();
            int exhSysNum = 0;

            if (numExhaustSystems > 0) {
                state.dataZoneEquip->ExhaustAirSystem.allocate(numExhaustSystems);
            }

            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++exhSysNum;
                auto const &objectFields = instance.value();
                auto &thisExhSys = state.dataZoneEquip->ExhaustAirSystem(exhSysNum);
                thisExhSys.Name = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());

                std::string zoneMixerName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_mixer_name");
                int zoneMixerIndex = 0;
                bool zoneMixerErrFound = false;
                MixerComponent::GetZoneMixerIndex(state, zoneMixerName, zoneMixerIndex, zoneMixerErrFound, thisExhSys.Name);

                if (!zoneMixerErrFound) {
                    // With the correct MixerNum Initialize
                    MixerComponent::InitAirMixer(state, zoneMixerIndex); // Initialize all Mixer related parameters

                    // See if need to do the zone mixer's CheckEquipName() function
                    bool IsNotOK = false; // Flag to verify name
                    ValidateComponent(state, "AirLoopHVAC:ZoneMixer", zoneMixerName, IsNotOK, "AirLoopHVAC:ExhaustSystem");
                    if (IsNotOK) {
                        ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhSys.Name));
                        ShowContinueError(state, format("ZoneMixer Name ={} mismatch or not found.", zoneMixerName));
                        ErrorsFound = true;
                    } else {
                        // normal conditions
                    }
                } else {
                    ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhSys.Name));
                    ShowContinueError(state, format("Zone Mixer Name ={} not found.", zoneMixerName));
                    ErrorsFound = true;
                }
                thisExhSys.ZoneMixerName = zoneMixerName;
                thisExhSys.ZoneMixerIndex = zoneMixerIndex;

                thisExhSys.centralFanType = static_cast<HVAC::FanType>(
                    getEnumValue(HVAC::fanTypeNamesUC, Util::makeUPPER(ip->getAlphaFieldValue(objectFields, objectSchemaProps, "fan_object_type"))));
                if (thisExhSys.centralFanType != HVAC::FanType::SystemModel && thisExhSys.centralFanType != HVAC::FanType::ComponentModel) {
                    ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhSys.Name));
                    ShowContinueError(state, format("Fan Type ={} is not supported.", HVAC::fanTypeNames[(int)thisExhSys.centralFanType]));
                    ShowContinueError(state, "It needs to be either a Fan:SystemModel or a Fan:ComponentModel type.");
                    ErrorsFound = true;
                }

                std::string centralFanName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "fan_name");

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisExhSys.Name};
                int centralFanIndex = Fans::GetFanIndex(state, centralFanName);
                if (centralFanIndex == 0) {
                    ShowSevereItemNotFound(state, eoh, "fan_name", centralFanName);
                    ErrorsFound = true;
                } else {
                    auto *fan = state.dataFans->fans(centralFanIndex);

                    thisExhSys.availSched = fan->availSched;

                    BranchNodeConnections::SetUpCompSets(state,
                                                         cCurrentModuleObject,
                                                         thisExhSys.Name,
                                                         HVAC::fanTypeNames[(int)thisExhSys.centralFanType],
                                                         centralFanName,
                                                         state.dataLoopNodes->NodeID(fan->inletNodeNum),
                                                         state.dataLoopNodes->NodeID(fan->outletNodeNum));

                    SetupOutputVariable(state,
                                        "Central Exhaust Fan Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        thisExhSys.centralFan_MassFlowRate,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        thisExhSys.Name);

                    SetupOutputVariable(state,
                                        "Central Exhaust Fan Volumetric Flow Rate Standard",
                                        Constant::Units::m3_s,
                                        thisExhSys.centralFan_VolumeFlowRate_Std,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        thisExhSys.Name);

                    SetupOutputVariable(state,
                                        "Central Exhaust Fan Volumetric Flow Rate Current",
                                        Constant::Units::m3_s,
                                        thisExhSys.centralFan_VolumeFlowRate_Cur,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        thisExhSys.Name);

                    SetupOutputVariable(state,
                                        "Central Exhaust Fan Power",
                                        Constant::Units::W,
                                        thisExhSys.centralFan_Power,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        thisExhSys.Name);

                    SetupOutputVariable(state,
                                        "Central Exhaust Fan Energy",
                                        Constant::Units::J,
                                        thisExhSys.centralFan_Energy,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        thisExhSys.Name);
                }

                thisExhSys.CentralFanName = centralFanName;
                thisExhSys.CentralFanIndex = centralFanIndex;

                // sizing
                if (thisExhSys.SizingFlag) {
                    SizeExhaustSystem(state, exhSysNum);
                }
            }
            state.dataZoneEquip->NumExhaustAirSystems = numExhaustSystems;
        } else {
            // If no exhaust systems are defined, then do something <or nothing>:
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found getting AirLoopHVAC:ExhaustSystem.  Preceding condition(s) causes termination.");
        }
    }

    void CalcExhaustAirSystem(EnergyPlusData &state, int const ExhaustAirSystemNum, bool FirstHVACIteration)
    {
        auto &thisExhSys = state.dataZoneEquip->ExhaustAirSystem(ExhaustAirSystemNum);
        constexpr std::string_view RoutineName = "CalExhaustAirSystem: ";
        constexpr std::string_view cCurrentModuleObject = "AirloopHVAC:ExhaustSystem";
        bool ErrorsFound = false;
        if (!(state.afn->AirflowNetworkFanActivated && state.afn->distribution_simulated)) {
            MixerComponent::SimAirMixer(state, thisExhSys.ZoneMixerName, thisExhSys.ZoneMixerIndex);
        } else {
            // Give a warning that the current model does not work with AirflowNetwork for now
            ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhSys.Name));
            ShowContinueError(state, "AirloopHVAC:ExhaustSystem currently does not work with AirflowNetwork.");
            ErrorsFound = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found conducting CalcExhasutAirSystem(). Preceding condition(s) causes termination.");
        }

        Real64 mixerFlow_Prior = 0.0;
        int outletNode_index = state.dataMixerComponent->MixerCond(thisExhSys.ZoneMixerIndex).OutletNode;
        mixerFlow_Prior = state.dataLoopNodes->Node(outletNode_index).MassFlowRate;
        if (mixerFlow_Prior == 0.0) {
            // No flow coming out from the exhaust controls;
            // fan should be cut off now;
        }

        int outletNode_Num = 0;
        Real64 RhoAirCurrent = state.dataEnvrn->StdRhoAir;

        if (thisExhSys.centralFanType == HVAC::FanType::SystemModel) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
            state.dataFans->fans(thisExhSys.CentralFanIndex)->simulate(state, false, _, _);

            // Update report variables
            outletNode_Num = state.dataFans->fans(thisExhSys.CentralFanIndex)->outletNodeNum;

            thisExhSys.centralFan_MassFlowRate = state.dataLoopNodes->Node(outletNode_Num).MassFlowRate;

            thisExhSys.centralFan_VolumeFlowRate_Std = state.dataLoopNodes->Node(outletNode_Num).MassFlowRate / state.dataEnvrn->StdRhoAir;

            RhoAirCurrent = EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                          state.dataEnvrn->OutBaroPress,
                                                                          state.dataLoopNodes->Node(outletNode_Num).Temp,
                                                                          state.dataLoopNodes->Node(outletNode_Num).HumRat);
            if (RhoAirCurrent <= 0.0) {
                RhoAirCurrent = state.dataEnvrn->StdRhoAir;
            }
            thisExhSys.centralFan_VolumeFlowRate_Cur = state.dataLoopNodes->Node(outletNode_Num).MassFlowRate / RhoAirCurrent;

            thisExhSys.centralFan_Power = state.dataFans->fans(thisExhSys.CentralFanIndex)->totalPower;

            thisExhSys.centralFan_Energy = thisExhSys.centralFan_Power * state.dataHVACGlobal->TimeStepSysSec;

        } else if (thisExhSys.centralFanType == HVAC::FanType::ComponentModel) {
            auto *fan = state.dataFans->fans(thisExhSys.CentralFanIndex);
            fan->simulate(state, FirstHVACIteration);

            // Update output variables

            outletNode_Num = fan->outletNodeNum;

            thisExhSys.centralFan_MassFlowRate = fan->outletAirMassFlowRate;

            thisExhSys.centralFan_VolumeFlowRate_Std = fan->outletAirMassFlowRate / state.dataEnvrn->StdRhoAir;

            RhoAirCurrent = EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                          state.dataEnvrn->OutBaroPress,
                                                                          state.dataLoopNodes->Node(outletNode_Num).Temp,
                                                                          state.dataLoopNodes->Node(outletNode_Num).HumRat);
            if (RhoAirCurrent <= 0.0) {
                RhoAirCurrent = state.dataEnvrn->StdRhoAir;
            }
            thisExhSys.centralFan_VolumeFlowRate_Cur = fan->outletAirMassFlowRate / RhoAirCurrent;

            thisExhSys.centralFan_Power = fan->totalPower * 1000.0;

            thisExhSys.centralFan_Energy = fan->totalEnergy * 1000.0;
        }
        thisExhSys.exhTotalHVACReliefHeatLoss = state.dataLoopNodes->Node(outletNode_Num).MassFlowRate *
                                                (state.dataLoopNodes->Node(outletNode_Num).Enthalpy - state.dataEnvrn->OutEnthalpy);

        Real64 mixerFlow_Posterior = 0.0;
        mixerFlow_Posterior = state.dataLoopNodes->Node(outletNode_index).MassFlowRate;
        if (mixerFlow_Posterior < HVAC::SmallMassFlow) {
            // fan flow is nearly zero and should be considered off
            // but this still can use the ratio
        }
        if (mixerFlow_Prior < HVAC::SmallMassFlow) {
            // this is the case where the fan flow should be resetted to zeros and not run the ratio
        }
        if ((mixerFlow_Prior - mixerFlow_Posterior > HVAC::SmallMassFlow) || (mixerFlow_Prior - mixerFlow_Posterior < -HVAC::SmallMassFlow)) {
            // calculate a ratio
            Real64 flowRatio = mixerFlow_Posterior / mixerFlow_Prior;
            if (flowRatio > 1.0) {
                ShowWarningError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhSys.Name));
                ShowContinueError(state, "Requested flow rate is lower than the exhasut fan flow rate.");
                ShowContinueError(state, "Will scale up the requested flow rate to meet fan flow rate.");
            }

            // get the mixer inlet node index
            int zoneMixerIndex = thisExhSys.ZoneMixerIndex;
            for (int i = 1; i <= state.dataMixerComponent->MixerCond(zoneMixerIndex).NumInletNodes; ++i) {
                int exhLegIndex = state.dataExhAirSystemMrg->mixerIndexMap[state.dataMixerComponent->MixerCond(zoneMixerIndex).InletNode(i)];
                CalcZoneHVACExhaustControl(state, exhLegIndex, flowRatio);
            }

            // Simulate the mixer again to update the flow
            MixerComponent::SimAirMixer(state, thisExhSys.ZoneMixerName, thisExhSys.ZoneMixerIndex);

            // if the adjustment matches, then no need to run fan simulation again.
        }
    }

    void GetZoneExhaustControlInput(EnergyPlusData &state)
    {
        // Process ZoneExhaust Control inputs

        // Locals
        bool ErrorsFound = false;

        // Use the json helper to process input
        constexpr std::string_view RoutineName("GetZoneExhaustControlInput: ");
        constexpr std::string_view routineName = "GetZoneExhaustControlInput";

        std::string const cCurrentModuleObject = "ZoneHVAC:ExhaustControl";
        auto &ip = state.dataInputProcessing->inputProcessor;
        auto const instances = ip->epJSON.find(cCurrentModuleObject);
        if (instances != ip->epJSON.end()) {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
            auto &instancesValue = instances.value();
            int numZoneExhaustControls = instancesValue.size();
            int exhCtrlNum = 0;
            int NumAlphas;
            int NumNums;

            if (numZoneExhaustControls > 0) {
                state.dataZoneEquip->ZoneExhaustControlSystem.allocate(numZoneExhaustControls);
            }

            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++exhCtrlNum;
                auto const &objectFields = instance.value();
                auto &thisExhCtrl = state.dataZoneEquip->ZoneExhaustControlSystem(exhCtrlNum);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, instance.key()};

                thisExhCtrl.Name = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());

                std::string availSchName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "availability_schedule_name");
                if (availSchName.empty()) {
                    // blank
                    thisExhCtrl.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisExhCtrl.availSched = Sched::GetSchedule(state, availSchName)) == nullptr) {
                    // mismatch, reset to always on
                    thisExhCtrl.availSched = Sched::GetScheduleAlwaysOn(state);
                    ShowWarningItemNotFound(state, eoh, "Avaiability Schedule Name", availSchName, "Availability Schedule is reset to Always ON.");
                }

                std::string zoneName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_name");
                thisExhCtrl.ZoneName = zoneName;
                int zoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
                thisExhCtrl.ZoneNum = zoneNum;

                thisExhCtrl.ControlledZoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);

                // These two nodes are required inputs:
                std::string inletNodeName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "inlet_node_name");
                int inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                       inletNodeName,
                                                                       ErrorsFound,
                                                                       DataLoopNode::ConnectionObjectType::ZoneHVACExhaustControl,
                                                                       thisExhCtrl.Name,
                                                                       DataLoopNode::NodeFluidType::Air,
                                                                       DataLoopNode::ConnectionType::Inlet,
                                                                       NodeInputManager::CompFluidStream::Primary,
                                                                       DataLoopNode::ObjectIsParent);
                thisExhCtrl.InletNodeNum = inletNodeNum;

                std::string outletNodeName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "outlet_node_name");

                int outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                        outletNodeName,
                                                                        ErrorsFound,
                                                                        DataLoopNode::ConnectionObjectType::ZoneHVACExhaustControl,
                                                                        thisExhCtrl.Name,
                                                                        DataLoopNode::NodeFluidType::Air,
                                                                        DataLoopNode::ConnectionType::Outlet,
                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                        DataLoopNode::ObjectIsParent);
                thisExhCtrl.OutletNodeNum = outletNodeNum;

                if (!state.dataExhAirSystemMrg->mappingDone) {
                    state.dataExhAirSystemMrg->mixerIndexMap.emplace(outletNodeNum, exhCtrlNum);
                }

                Real64 designExhaustFlowRate = ip->getRealFieldValue(objectFields, objectSchemaProps, "design_exhaust_flow_rate");
                thisExhCtrl.DesignExhaustFlowRate = designExhaustFlowRate;

                std::string flowControlTypeName = Util::makeUPPER(ip->getAlphaFieldValue(objectFields, objectSchemaProps, "flow_control_type"));
                // std::string flowControlTypeName = Util::makeUPPER(fields.at("flow_control_type").get<std::string>());
                thisExhCtrl.FlowControlOption =
                    static_cast<ZoneExhaustControl::FlowControlType>(getEnumValue(flowControlTypeNamesUC, flowControlTypeName));

                std::string exhaustFlowFractionSchedName =
                    ip->getAlphaFieldValue(objectFields, objectSchemaProps, "exhaust_flow_fraction_schedule_name");

                if (exhaustFlowFractionSchedName.empty()) {
                    thisExhCtrl.exhaustFlowFractionSched =
                        Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
                } else if ((thisExhCtrl.exhaustFlowFractionSched = Sched::GetSchedule(state, exhaustFlowFractionSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Exhaust Flow Fraction Schedule Name", exhaustFlowFractionSchedName);
                }

                thisExhCtrl.SupplyNodeOrNodelistName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "supply_node_or_nodelist_name");

                bool NodeListError = false;
                int NumParams = 0;
                int NumNodes = 0;

                ip->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNums);
                thisExhCtrl.SuppNodeNums.dimension(NumParams, 0);
                NodeInputManager::GetNodeNums(state,
                                              thisExhCtrl.SupplyNodeOrNodelistName,
                                              NumNodes,
                                              thisExhCtrl.SuppNodeNums,
                                              NodeListError,
                                              DataLoopNode::NodeFluidType::Air,
                                              DataLoopNode::ConnectionObjectType::ZoneHVACExhaustControl, // maybe zone inlets?
                                              thisExhCtrl.Name,
                                              DataLoopNode::ConnectionType::Sensor,
                                              NodeInputManager::CompFluidStream::Primary,
                                              DataLoopNode::ObjectIsNotParent); // , // _, // supplyNodeOrNodelistName);

                // Verify these nodes are indeed supply nodes:
                if (thisExhCtrl.FlowControlOption == ZoneExhaustControl::FlowControlType::FollowSupply) { // FollowSupply
                    bool nodeNotFound = false;
                    for (size_t i = 1; i <= thisExhCtrl.SuppNodeNums.size(); ++i) {
                        CheckForSupplyNode(state, exhCtrlNum, nodeNotFound);
                        if (nodeNotFound) {
                            ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisExhCtrl.Name));
                            ShowContinueError(state,
                                              format("Node or NodeList Name ={}. Must all be supply nodes.", thisExhCtrl.SupplyNodeOrNodelistName));
                            ErrorsFound = true;
                        }
                    }
                }

                // Deal with design exhaust autosize here;
                if (thisExhCtrl.DesignExhaustFlowRate == DataSizing::AutoSize) {
                    SizeExhaustControlFlow(state, exhCtrlNum, thisExhCtrl.SuppNodeNums);
                }

                std::string minZoneTempLimitSchedName =
                    ip->getAlphaFieldValue(objectFields, objectSchemaProps, "minimum_zone_temperature_limit_schedule_name");
                if (minZoneTempLimitSchedName.empty()) {
                } else if ((thisExhCtrl.minZoneTempLimitSched = Sched::GetSchedule(state, minZoneTempLimitSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Minimum Zone Temperature Limit Schedule Name", minZoneTempLimitSchedName);
                }

                std::string minExhFlowFracSchedName =
                    ip->getAlphaFieldValue(objectFields, objectSchemaProps, "minimum_exhaust_flow_fraction_schedule_name");
                // to do so schedule matching
                if (minExhFlowFracSchedName.empty()) {
                } else if ((thisExhCtrl.minExhFlowFracSched = Sched::GetSchedule(state, minExhFlowFracSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Minimum Exhaust Flow Fraction Schedule Name", minExhFlowFracSchedName);
                }

                std::string balancedExhFracSchedName =
                    ip->getAlphaFieldValue(objectFields, objectSchemaProps, "balanced_exhaust_fraction_schedule_name");
                // to do so schedule matching
                if (balancedExhFracSchedName.empty()) {
                } else if ((thisExhCtrl.balancedExhFracSched = Sched::GetSchedule(state, balancedExhFracSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Balanced Exhaust Fraction Schedule Name", balancedExhFracSchedName);
                }

                // Maybe an additional check per IORef:
                // This input field must be blank when the zone air flow balance is enforced. If user specifies a schedule and zone air flow balance
                // is enforced, then EnergyPlus throws a warning error message, ignores the schedule and simulation continues.
            }

            state.dataZoneEquip->NumZoneExhaustControls = numZoneExhaustControls; // or exhCtrlNum

            // Done with creating a map that contains a table of for each zone to exhasut controls
            state.dataExhAirSystemMrg->mappingDone = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found getting ZoneHVAC:ExhaustControl.  Preceding condition(s) causes termination.");
        }
    }

    void SimZoneHVACExhaustControls(EnergyPlusData &state)
    {
        if (state.dataExhCtrlSystemMrg->GetInputFlag) { // First time subroutine has been entered
            GetZoneExhaustControlInput(state);
            state.dataExhCtrlSystemMrg->GetInputFlag = false;
        }

        for (int ExhaustControlNum = 1; ExhaustControlNum <= state.dataZoneEquip->NumZoneExhaustControls; ++ExhaustControlNum) {
            CalcZoneHVACExhaustControl(state, ExhaustControlNum);
        }

        // report results if needed
    }

    void CalcZoneHVACExhaustControl(EnergyPlusData &state, int const ZoneHVACExhaustControlNum, Real64 const FlowRatio)
    {
        // Calculate a zonehvac exhaust control system

        auto &thisExhCtrl = state.dataZoneEquip->ZoneExhaustControlSystem(ZoneHVACExhaustControlNum);

        int InletNode = thisExhCtrl.InletNodeNum;
        int OutletNode = thisExhCtrl.OutletNodeNum;
        auto &thisExhInlet = state.dataLoopNodes->Node(InletNode);
        auto &thisExhOutlet = state.dataLoopNodes->Node(OutletNode);
        Real64 MassFlow;
        Real64 Tin = state.dataZoneTempPredictorCorrector->zoneHeatBalance(thisExhCtrl.ZoneNum).ZT;
        Real64 thisExhCtrlAvailScheVal = thisExhCtrl.availSched->getCurrentVal();

        if (FlowRatio >= 0.0) {
            thisExhCtrl.BalancedFlow *= FlowRatio;
            thisExhCtrl.UnbalancedFlow *= FlowRatio;

            thisExhInlet.MassFlowRate *= FlowRatio;
        } else {
            // Availability schedule:
            if (thisExhCtrlAvailScheVal <= 0.0) {
                MassFlow = 0.0;
                thisExhInlet.MassFlowRate = 0.0;
            } else {
                //
            }

            Real64 DesignFlowRate = thisExhCtrl.DesignExhaustFlowRate;
            Real64 FlowFrac = 1.0;
            if (thisExhCtrl.exhaustFlowFractionSched != nullptr) {
                FlowFrac = thisExhCtrl.exhaustFlowFractionSched->getCurrentVal();
                if (FlowFrac < 0.0) {
                    ShowWarningError(
                        state, format("Exhaust Flow Fraction Schedule value is negative for Zone Exhaust Control Named: {};", thisExhCtrl.Name));
                    ShowContinueError(state, "Reset value to zero and continue the simulation.");
                    FlowFrac = 0.0;
                }
            }

            Real64 MinFlowFrac = 0.0;
            if (thisExhCtrl.minExhFlowFracSched != nullptr) {
                MinFlowFrac = thisExhCtrl.minExhFlowFracSched->getCurrentVal();
                if (MinFlowFrac < 0.0) {
                    ShowWarningError(
                        state,
                        format("Minimum Exhaust Flow Fraction Schedule value is negative for Zone Exhaust Control Named: {};", thisExhCtrl.Name));
                    ShowContinueError(state, "Reset value to zero and continue the simulation.");
                    MinFlowFrac = 0.0;
                }
            }

            if (FlowFrac < MinFlowFrac) {
                FlowFrac = MinFlowFrac;
            }

            if (thisExhCtrlAvailScheVal > 0.0) { // available
                if (thisExhCtrl.minZoneTempLimitSched != nullptr) {
                    if (Tin >= thisExhCtrl.minZoneTempLimitSched->getCurrentVal()) {
                    } else {
                        FlowFrac = MinFlowFrac;
                    }
                } else {
                    // flow not changed
                }
            } else {
                FlowFrac = 0.0; // directly set flow rate to zero.
            }

            if (thisExhCtrl.FlowControlOption == ZoneExhaustControl::FlowControlType::FollowSupply) { // follow-supply
                Real64 supplyFlowRate = 0.0;
                int numOfSuppNodes = thisExhCtrl.SuppNodeNums.size();
                for (int i = 1; i <= numOfSuppNodes; ++i) {
                    supplyFlowRate += state.dataLoopNodes->Node(thisExhCtrl.SuppNodeNums(i)).MassFlowRate;
                }
                MassFlow = supplyFlowRate * FlowFrac;
            } else { // Scheduled or Invalid
                MassFlow = DesignFlowRate * FlowFrac;
            }

            if (thisExhCtrl.balancedExhFracSched != nullptr) {
                thisExhCtrl.BalancedFlow = // state.dataHVACGlobal->BalancedExhMassFlow =
                    MassFlow *             // state.dataHVACGlobal->UnbalExhMassFlow *
                    thisExhCtrl.balancedExhFracSched->getCurrentVal();
                thisExhCtrl.UnbalancedFlow =  // state.dataHVACGlobal->UnbalExhMassFlow =
                    MassFlow -                // = state.dataHVACGlobal->UnbalExhMassFlow -
                    thisExhCtrl.BalancedFlow; // state.dataHVACGlobal->BalancedExhMassFlow;
            } else {
                thisExhCtrl.BalancedFlow = 0.0;
                thisExhCtrl.UnbalancedFlow = MassFlow;
            }

            thisExhInlet.MassFlowRate = MassFlow;
        }

        thisExhOutlet.MassFlowRate = thisExhInlet.MassFlowRate;

        thisExhOutlet.Temp = thisExhInlet.Temp;
        thisExhOutlet.HumRat = thisExhInlet.HumRat;
        thisExhOutlet.Enthalpy = thisExhInlet.Enthalpy;
        // Set the outlet nodes for properties that just pass through & not used
        thisExhOutlet.Quality = thisExhInlet.Quality;
        thisExhOutlet.Press = thisExhInlet.Press;

        // Set the Node Flow Control Variables from the Fan Control Variables
        thisExhOutlet.MassFlowRateMax = thisExhInlet.MassFlowRateMax;
        thisExhOutlet.MassFlowRateMaxAvail = thisExhInlet.MassFlowRateMaxAvail;
        thisExhOutlet.MassFlowRateMinAvail = thisExhInlet.MassFlowRateMinAvail;

        // these might also be useful to pass through
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            thisExhOutlet.CO2 = thisExhInlet.CO2;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            thisExhOutlet.GenContam = thisExhInlet.GenContam;
        }
    }

    void SizeExhaustSystem(EnergyPlusData &state, int const exhSysNum)
    {
        auto &thisExhSys = state.dataZoneEquip->ExhaustAirSystem(exhSysNum);

        if (!thisExhSys.SizingFlag) {
            return;
        }

        // mixer outlet sizing:
        Real64 outletFlowMaxAvail = 0.0;
        for (int i = 1; i <= state.dataMixerComponent->MixerCond(thisExhSys.ZoneMixerIndex).NumInletNodes; ++i) {
            int inletNode_index = state.dataMixerComponent->MixerCond(thisExhSys.ZoneMixerIndex).InletNode(i);
            outletFlowMaxAvail += state.dataLoopNodes->Node(inletNode_index).MassFlowRateMaxAvail;
        }

        // mixer outlet considered OutletMassFlowRateMaxAvail?
        int outletNode_index = state.dataMixerComponent->MixerCond(thisExhSys.ZoneMixerIndex).OutletNode;
        state.dataLoopNodes->Node(outletNode_index).MassFlowRateMaxAvail = outletFlowMaxAvail;

        auto *fan = state.dataFans->fans(thisExhSys.CentralFanIndex);
        // then central exhasut fan sizing here:
        if (thisExhSys.centralFanType == HVAC::FanType::SystemModel) {
            if (fan->maxAirFlowRate == DataSizing::AutoSize) {
                fan->maxAirFlowRate = outletFlowMaxAvail / state.dataEnvrn->StdRhoAir;
            }
            BaseSizer::reportSizerOutput(state, "FAN:SYSTEMMODEL", fan->Name, "Design Fan Airflow [m3/s]", fan->maxAirFlowRate);
        } else if (thisExhSys.centralFanType == HVAC::FanType::ComponentModel) {
            if (fan->maxAirMassFlowRate == DataSizing::AutoSize) {
                fan->maxAirMassFlowRate = outletFlowMaxAvail * dynamic_cast<Fans::FanComponent *>(fan)->sizingFactor;
            }
            BaseSizer::reportSizerOutput(state,
                                         HVAC::fanTypeNames[(int)fan->type],
                                         fan->Name,
                                         "Design Fan Airflow [m3/s]",
                                         fan->maxAirMassFlowRate / state.dataEnvrn->StdRhoAir);
        } else {
            //
        }

        // after evertyhing sized, set the sizing flag to be false
        thisExhSys.SizingFlag = false;
    }

    void SizeExhaustControlFlow(EnergyPlusData &state, int const zoneExhCtrlNum, Array1D_int &NodeNums)
    {
        auto &thisExhCtrl = state.dataZoneEquip->ZoneExhaustControlSystem(zoneExhCtrlNum);

        Real64 designFlow = 0.0;

        if (thisExhCtrl.FlowControlOption == ZoneExhaustControl::FlowControlType::FollowSupply) { // FollowSupply
            // size based on supply nodelist flow
            for (size_t i = 1; i <= NodeNums.size(); ++i) {
                designFlow += state.dataLoopNodes->Node(NodeNums(i)).MassFlowRateMax;
            }
        } else { // scheduled etc.
            // based on zone OA.
            designFlow = state.dataSize->FinalZoneSizing(thisExhCtrl.ZoneNum).MinOA;
        }

        thisExhCtrl.DesignExhaustFlowRate = designFlow;
    }

    void UpdateZoneExhaustControl(EnergyPlusData &state)
    {
        for (int i = 1; i <= state.dataZoneEquip->NumZoneExhaustControls; ++i) {
            int controlledZoneNum = state.dataZoneEquip->ZoneExhaustControlSystem(i).ControlledZoneNum;
            state.dataZoneEquip->ZoneEquipConfig(controlledZoneNum).ZoneExh +=
                state.dataZoneEquip->ZoneExhaustControlSystem(i).BalancedFlow + state.dataZoneEquip->ZoneExhaustControlSystem(i).UnbalancedFlow;
            state.dataZoneEquip->ZoneEquipConfig(controlledZoneNum).ZoneExhBalanced += state.dataZoneEquip->ZoneExhaustControlSystem(i).BalancedFlow;
        }
    }

    void CheckForSupplyNode(EnergyPlusData &state, int const ExhCtrlNum, bool &NodeNotFound)
    {
        // Trying to check a node to see if it is truely a supply node
        // for a nodelist, need a call loop to check each node in the list

        auto &thisExhCtrl = state.dataZoneEquip->ZoneExhaustControlSystem(ExhCtrlNum);

        // check a node is a zone inlet node.
        std::string_view constexpr RoutineName = "GetExhaustControlInput: ";
        std::string_view constexpr CurrentModuleObject = "ZoneHVAC:ExhaustControl";

        bool ZoneNodeNotFound = true;
        bool ErrorsFound = false;
        for (size_t i = 1; i <= thisExhCtrl.SuppNodeNums.size(); ++i) {
            int supplyNodeNum = thisExhCtrl.SuppNodeNums(i);
            for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(thisExhCtrl.ZoneNum).NumInletNodes; ++NodeNum) {
                if (supplyNodeNum == state.dataZoneEquip->ZoneEquipConfig(thisExhCtrl.ZoneNum).InletNode(NodeNum)) {
                    ZoneNodeNotFound = false;
                    break;
                }
            }
            if (ZoneNodeNotFound) {
                ShowSevereError(state, format("{}{}={}", RoutineName, CurrentModuleObject, thisExhCtrl.Name));
                ShowContinueError(
                    state,
                    format("Supply or supply list = \"{}\" contains at least one node that is not a zone inlet node for Zone Name = \"{}\"",
                           thisExhCtrl.SupplyNodeOrNodelistName,
                           thisExhCtrl.ZoneName));
                ShowContinueError(state, "..Nodes in the supply node or nodelist must be a zone inlet node.");
                ErrorsFound = true;
            }
        }

        NodeNotFound = ErrorsFound;
    }

    bool ExhaustSystemHasMixer(EnergyPlusData &state, std::string_view CompName) // component (mixer) name
    {
        // Given a mixer name, this routine determines if that mixer is found on Exhaust Systems.

        if (state.dataExhAirSystemMrg->GetInputFlag) {
            GetExhaustAirSystemInput(state);
            state.dataExhAirSystemMrg->GetInputFlag = false;
        }

        return // ( state.dataZoneEquip->NumExhaustAirSystems > 0) &&
            (Util::FindItemInList(CompName, state.dataZoneEquip->ExhaustAirSystem, &ExhaustAir::ZoneMixerName) > 0);
    }

} // namespace ExhaustAirSystemManager

} // namespace EnergyPlus
