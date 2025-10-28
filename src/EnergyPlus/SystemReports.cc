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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <unordered_map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DualDuct.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/HVACCooledBeam.hh>
#include <EnergyPlus/HVACFourPipeBeam.hh>
#include <EnergyPlus/HVACSingleDuctInduc.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HybridUnitaryAirConditioners.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/SystemReports.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowAC.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::SystemReports {

// Module containing the routines dealing with Mechanical Ventilation Loads and Energy Reporting (Outside Air)

// MODULE INFORMATION:
//       AUTHOR         Mike Witte, Linda Lawrie, Dan Fisher
//       DATE WRITTEN   Apr-Jul 2005
//       MODIFIED       22Aug2010 Craig Wray - added Fan:ComponentModel

// PURPOSE OF THIS MODULE:
// This module embodies the scheme(s) for reporting ventilation loads and energy use.

// Using/Aliasing
using namespace DataLoopNode;
using namespace DataAirLoop;
using namespace DataPlant;
using namespace DataZoneEquipment;
using namespace DataAirSystems;

// Functions

void InitEnergyReports(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   April 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Initializes the energy components of the data structures

    // METHODOLOGY EMPLOYED:
    // Once all compsets have been established (second iteration) find all components
    // subcomponents, etc.

    int constexpr EnergyTransfer(1);

    if (!state.dataSysRpts->VentReportStructureCreated) {
        return;
    }

    if (state.dataSysRpts->OneTimeFlag_InitEnergyReports) {

        // ***I think we need to preprocess the main components on the branch to get them in order***
        // This needs to be done before we start in on the component loop
        // GetChildrenData will put all of the subcomponents in order for us

        for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum);
            if (!thisZoneEquipConfig.IsControlled) {
                continue;
            }
            thisZoneEquipConfig.EquipListIndex = Util::FindItemInList(thisZoneEquipConfig.EquipListName, state.dataZoneEquip->ZoneEquipList);
            auto &thisZoneEquipList = state.dataZoneEquip->ZoneEquipList(thisZoneEquipConfig.EquipListIndex);
            for (int ZoneInletNodeNum = 1; ZoneInletNodeNum <= thisZoneEquipConfig.NumInletNodes; ++ZoneInletNodeNum) {
                int AirLoopNum = thisZoneEquipConfig.InletNodeAirLoopNum(ZoneInletNodeNum);
                for (int CompNum = 1; CompNum <= thisZoneEquipList.NumOfEquipTypes; ++CompNum) {
                    for (int NodeCount = 1; NodeCount <= thisZoneEquipList.EquipData(CompNum).NumOutlets; ++NodeCount) {
                        if (thisZoneEquipList.EquipData(CompNum).OutletNodeNums(NodeCount) ==
                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).OutNode) {
                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).AirDistUnitIndex = CompNum;
                            if (thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathExists) {
                                for (int SAPNum = 1; SAPNum <= state.dataZoneEquip->NumSupplyAirPaths; ++SAPNum) {
                                    for (int SAPOutNode = 1; SAPOutNode <= state.dataZoneEquip->SupplyAirPath(SAPNum).NumOutletNodes; ++SAPOutNode) {
                                        if (thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).InNode ==
                                            state.dataZoneEquip->SupplyAirPath(SAPNum).OutletNode(SAPOutNode)) {
                                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathIndex = SAPNum;
                                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathOutNodeIndex = SAPOutNode;
                                            for (int OutNum = 1; OutNum <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes;
                                                 ++OutNum) {
                                                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum) ==
                                                    state.dataZoneEquip->SupplyAirPath(SAPNum).InletNodeNum) {
                                                    thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).AirLoopNum = AirLoopNum;
                                                    thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex =
                                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OutletBranchNum[OutNum - 1];
                                                    if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.Exists) {
                                                        for (int MainBranchNum = 1;
                                                             MainBranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                             ++MainBranchNum) {
                                                            if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                    .Branch(MainBranchNum)
                                                                    .NodeNumOut ==
                                                                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.NodeNumIn) {
                                                                thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex = MainBranchNum;
                                                            }
                                                        }
                                                    } else { // no splitter
                                                        thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            } else { // no supply air path
                                if (AirLoopNum > 0) {
                                    for (int NodeIndex = 1; NodeIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes;
                                         ++NodeIndex) {
                                        if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(NodeIndex) ==
                                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).InNode) {
                                            for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                 ++BranchNum) {
                                                if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).NodeNumOut ==
                                                    state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(NodeIndex)) {
                                                    thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).AirLoopNum = AirLoopNum;
                                                    thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex = BranchNum;
                                                    if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.Exists) {
                                                        for (int MainBranchNum = 1;
                                                             MainBranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                             ++MainBranchNum) {
                                                            if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                    .Branch(MainBranchNum)
                                                                    .NodeNumOut ==
                                                                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.NodeNumIn) {
                                                                thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex = MainBranchNum;
                                                            }
                                                        }
                                                    } else { // no splitter
                                                        thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                            thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathIndex;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (thisZoneEquipList.EquipData(CompNum).OutletNodeNums(NodeCount) ==
                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).OutNode) {
                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).AirDistUnitIndex = CompNum;
                            if (thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathExists) {
                                for (int SAPNum = 1; SAPNum <= state.dataZoneEquip->NumSupplyAirPaths; ++SAPNum) {
                                    for (int SAPOutNode = 1; SAPOutNode <= state.dataZoneEquip->SupplyAirPath(SAPNum).NumOutletNodes; ++SAPOutNode) {
                                        if (thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).InNode ==
                                            state.dataZoneEquip->SupplyAirPath(SAPNum).OutletNode(SAPOutNode)) {
                                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex = SAPNum;
                                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathOutNodeIndex = SAPOutNode;
                                            for (int OutNum = 1; OutNum <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes;
                                                 ++OutNum) {
                                                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum) ==
                                                    state.dataZoneEquip->SupplyAirPath(SAPNum).InletNodeNum) {
                                                    thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).AirLoopNum = AirLoopNum;
                                                    thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex =
                                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OutletBranchNum[OutNum - 1];
                                                    if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.Exists) {
                                                        for (int MainBranchNum = 1;
                                                             MainBranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                             ++MainBranchNum) {
                                                            if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                    .Branch(MainBranchNum)
                                                                    .NodeNumOut ==
                                                                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.NodeNumIn) {
                                                                thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex = MainBranchNum;
                                                            }
                                                        }
                                                    } else { // no splitter
                                                        thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            } else { // no supply air path
                                if (AirLoopNum > 0) {
                                    for (int NodeIndex = 1; NodeIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes;
                                         ++NodeIndex) {
                                        if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(NodeIndex) ==
                                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).InNode) {
                                            for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                 ++BranchNum) {
                                                if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).NodeNumOut ==
                                                    state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(NodeIndex)) {
                                                    thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).AirLoopNum = AirLoopNum;
                                                    thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex = BranchNum;
                                                    if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.Exists) {
                                                        for (int MainBranchNum = 1;
                                                             MainBranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches;
                                                             ++MainBranchNum) {
                                                            if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                    .Branch(MainBranchNum)
                                                                    .NodeNumOut ==
                                                                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Splitter.NodeNumIn) {
                                                                thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex = MainBranchNum;
                                                            }
                                                        }
                                                    } else { // no splitter
                                                        thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                            thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum);
            if (!thisZoneEquipConfig.IsControlled) {
                continue;
            }
            thisZoneEquipConfig.EquipListIndex = Util::FindItemInList(thisZoneEquipConfig.EquipListName, state.dataZoneEquip->ZoneEquipList);
            int ListNum = thisZoneEquipConfig.EquipListIndex;
            // loop over the zone supply air path inlet nodes
            for (int ZoneInletNodeNum = 1; ZoneInletNodeNum <= thisZoneEquipConfig.NumInletNodes; ++ZoneInletNodeNum) {
                int AirLoopNum = thisZoneEquipConfig.InletNodeAirLoopNum(ZoneInletNodeNum);

                // 1. Find HVAC component plant loop connections
                int MainBranchNum = thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex;
                MainBranchNum = max(MainBranchNum, thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex);
                if (MainBranchNum > 0) {
                    MatchPlantSys(state, AirLoopNum, MainBranchNum);
                }
                int SupplyCoolBranchNum = thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex;
                if (SupplyCoolBranchNum > 0 && (SupplyCoolBranchNum != MainBranchNum)) {
                    MatchPlantSys(state, AirLoopNum, SupplyCoolBranchNum);
                }
                int SupplyHeatBranchNum = thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex;
                if (SupplyHeatBranchNum > 0 && (SupplyHeatBranchNum != MainBranchNum)) {
                    MatchPlantSys(state, AirLoopNum, SupplyHeatBranchNum);
                }

                int AirDistUnitNum = thisZoneEquipConfig.AirDistUnitCool(ZoneInletNodeNum).AirDistUnitIndex;
                AirDistUnitNum = max(AirDistUnitNum, thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).AirDistUnitIndex);
                if (ListNum > 0 && AirDistUnitNum > 0) {
                    auto &thisZoneEquipList = state.dataZoneEquip->ZoneEquipList(ListNum);
                    for (int VarNum = 1; VarNum <= thisZoneEquipList.EquipData(AirDistUnitNum).NumMeteredVars; ++VarNum) {
                        if (thisZoneEquipList.EquipData(AirDistUnitNum).MeteredVar(VarNum).resource == Constant::eResource::EnergyTransfer) {
                            thisZoneEquipList.EquipData(AirDistUnitNum).EnergyTransComp = EnergyTransfer;
                            const std::string &CompType = thisZoneEquipList.EquipData(AirDistUnitNum).TypeOf;
                            const std::string &CompName = thisZoneEquipList.EquipData(AirDistUnitNum).Name;
                            int Idx = 0;
                            int MatchLoop = 0;
                            int MatchLoopType = 0;
                            int MatchBranch = 0;
                            int MatchComp = 0;
                            bool MatchFound = false;
                            FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                            if (MatchFound) {
                                UpdateZoneCompPtrArray(state, Idx, ListNum, AirDistUnitNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                            }
                            thisZoneEquipList.EquipData(AirDistUnitNum).ZoneEqToPlantPtr = Idx;
                            break;
                        }
                    }
                    for (int SubEquipNum = 1; SubEquipNum <= thisZoneEquipList.EquipData(AirDistUnitNum).NumSubEquip; ++SubEquipNum) {
                        for (int VarNum = 1; VarNum <= thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).NumMeteredVars;
                             ++VarNum) {
                            if (thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).MeteredVar(VarNum).resource ==
                                Constant::eResource::EnergyTransfer) {
                                thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).EnergyTransComp = EnergyTransfer;
                                const std::string &CompType = thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).TypeOf;
                                const std::string &CompName = thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).Name;
                                int Idx = 0;
                                int MatchLoop = 0;
                                int MatchLoopType = 0;
                                int MatchBranch = 0;
                                int MatchComp = 0;
                                bool MatchFound = false;
                                FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                if (MatchFound) {
                                    UpdateZoneSubCompPtrArray(
                                        state, Idx, ListNum, AirDistUnitNum, SubEquipNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                }
                                thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).ZoneEqToPlantPtr = Idx;
                                break;
                            }
                        }
                        for (int SubSubEquipNum = 1;
                             SubSubEquipNum <= thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).NumSubSubEquip;
                             ++SubSubEquipNum) {
                            for (int VarNum = 1;
                                 VarNum <=
                                 thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).SubSubEquipData(SubSubEquipNum).NumMeteredVars;
                                 ++VarNum) {
                                if (thisZoneEquipList.EquipData(AirDistUnitNum)
                                        .SubEquipData(SubEquipNum)
                                        .SubSubEquipData(SubSubEquipNum)
                                        .MeteredVar(VarNum)
                                        .resource == Constant::eResource::EnergyTransfer) {
                                    thisZoneEquipList.EquipData(AirDistUnitNum)
                                        .SubEquipData(SubEquipNum)
                                        .SubSubEquipData(SubSubEquipNum)
                                        .EnergyTransComp = EnergyTransfer;
                                    const std::string &CompType =
                                        thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).SubSubEquipData(SubSubEquipNum).TypeOf;
                                    const std::string &CompName =
                                        thisZoneEquipList.EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).SubSubEquipData(SubSubEquipNum).Name;
                                    int Idx = 0;
                                    int MatchLoop = 0;
                                    int MatchLoopType = 0;
                                    int MatchBranch = 0;
                                    int MatchComp = 0;
                                    bool MatchFound = false;
                                    FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    if (MatchFound) {
                                        UpdateZoneSubSubCompPtrArray(state,
                                                                     Idx,
                                                                     ListNum,
                                                                     AirDistUnitNum,
                                                                     SubEquipNum,
                                                                     SubSubEquipNum,
                                                                     MatchLoopType,
                                                                     MatchLoop,
                                                                     MatchBranch,
                                                                     MatchComp);
                                    }
                                    thisZoneEquipList.EquipData(AirDistUnitNum)
                                        .SubEquipData(SubEquipNum)
                                        .SubSubEquipData(SubSubEquipNum)
                                        .ZoneEqToPlantPtr = Idx;
                                    break;
                                }
                            }
                        }
                    }
                }

                int EquipNum = 0;
                int SubEquipNum = 0;
                int SubSubEquipNum = 0;
                int CompNum = 0;
                int SubCompNum = 0;
                int SubSubCompNum = 0;
                // Eliminate duplicates in the connection arrays
                if (allocated(state.dataAirSystemsData->ZoneCompToPlant)) {
                    EquipNum = isize(state.dataAirSystemsData->ZoneCompToPlant);
                }
                if (allocated(state.dataAirSystemsData->ZoneSubCompToPlant)) {
                    SubEquipNum = isize(state.dataAirSystemsData->ZoneSubCompToPlant);
                }
                if (allocated(state.dataAirSystemsData->ZoneSubSubCompToPlant)) {
                    SubSubEquipNum = isize(state.dataAirSystemsData->ZoneSubSubCompToPlant);
                }
                if (allocated(state.dataAirSystemsData->AirSysCompToPlant)) {
                    CompNum = isize(state.dataAirSystemsData->AirSysCompToPlant);
                }
                if (allocated(state.dataAirSystemsData->AirSysSubCompToPlant)) {
                    SubCompNum = isize(state.dataAirSystemsData->AirSysSubCompToPlant);
                }
                if (allocated(state.dataAirSystemsData->AirSysSubSubCompToPlant)) {
                    SubSubCompNum = isize(state.dataAirSystemsData->AirSysSubSubCompToPlant);
                }

                if (EquipNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= EquipNum; ++i) {
                        auto const &zi = state.dataAirSystemsData->ZoneCompToPlant(i);
                        bool duplicate(false);
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &zj = state.dataAirSystemsData->ZoneCompToPlant(j);
                            if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &za = state.dataAirSystemsData->ZoneCompToPlant(ArrayCount);
                                za.ZoneEqListNum = zi.ZoneEqListNum;
                                za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                za.PlantLoopType = zi.PlantLoopType;
                                za.PlantLoopNum = zi.PlantLoopNum;
                                za.PlantLoopBranch = zi.PlantLoopBranch;
                                za.PlantLoopComp = zi.PlantLoopComp;
                                za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                za.LastDemandSidePtr = zi.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= EquipNum; ++i) { // Zero the now-unused entries
                        auto &zi = state.dataAirSystemsData->ZoneCompToPlant(i);
                        zi.ZoneEqListNum = 0;
                        zi.ZoneEqCompNum = 0;
                        zi.PlantLoopType = 0;
                        zi.PlantLoopNum = 0;
                        zi.PlantLoopBranch = 0;
                        zi.PlantLoopComp = 0;
                        zi.FirstDemandSidePtr = 0;
                        zi.LastDemandSidePtr = 0;
                    }
                }

                if (SubEquipNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= SubEquipNum; ++i) {
                        auto const &zi = state.dataAirSystemsData->ZoneSubCompToPlant(i);
                        bool duplicate(false);
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &zj = state.dataAirSystemsData->ZoneSubCompToPlant(j);
                            if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum) &&
                                (zi.ZoneEqSubCompNum == zj.ZoneEqSubCompNum)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &za = state.dataAirSystemsData->ZoneSubCompToPlant(ArrayCount);
                                za.ZoneEqListNum = zi.ZoneEqListNum;
                                za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                za.ZoneEqSubCompNum = zi.ZoneEqSubCompNum;
                                za.PlantLoopType = zi.PlantLoopType;
                                za.PlantLoopNum = zi.PlantLoopNum;
                                za.PlantLoopBranch = zi.PlantLoopBranch;
                                za.PlantLoopComp = zi.PlantLoopComp;
                                za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                za.LastDemandSidePtr = zi.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= SubEquipNum; ++i) { // Zero the now-unused entries
                        auto &zi = state.dataAirSystemsData->ZoneSubCompToPlant(i);
                        zi.ZoneEqListNum = 0;
                        zi.ZoneEqCompNum = 0;
                        zi.ZoneEqSubCompNum = 0;
                        zi.PlantLoopType = 0;
                        zi.PlantLoopNum = 0;
                        zi.PlantLoopBranch = 0;
                        zi.PlantLoopComp = 0;
                        zi.FirstDemandSidePtr = 0;
                        zi.LastDemandSidePtr = 0;
                    }
                }

                if (SubSubEquipNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= SubSubEquipNum; ++i) {
                        auto const &zi = state.dataAirSystemsData->ZoneSubSubCompToPlant(i);
                        bool duplicate(false);
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &zj = state.dataAirSystemsData->ZoneSubSubCompToPlant(j);
                            if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum) &&
                                (zi.ZoneEqSubCompNum == zj.ZoneEqSubCompNum) && (zi.ZoneEqSubSubCompNum == zj.ZoneEqSubSubCompNum)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &za = state.dataAirSystemsData->ZoneSubSubCompToPlant(ArrayCount);
                                za.ZoneEqListNum = zi.ZoneEqListNum;
                                za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                za.ZoneEqSubCompNum = zi.ZoneEqSubCompNum;
                                za.ZoneEqSubSubCompNum = zi.ZoneEqSubSubCompNum;
                                za.PlantLoopType = zi.PlantLoopType;
                                za.PlantLoopNum = zi.PlantLoopNum;
                                za.PlantLoopBranch = zi.PlantLoopBranch;
                                za.PlantLoopComp = zi.PlantLoopComp;
                                za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                za.LastDemandSidePtr = zi.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= SubSubEquipNum; ++i) { // Zero the now-unused entries
                        auto &zi = state.dataAirSystemsData->ZoneSubSubCompToPlant(i);
                        zi.ZoneEqListNum = 0;
                        zi.ZoneEqCompNum = 0;
                        zi.ZoneEqSubCompNum = 0;
                        zi.ZoneEqSubSubCompNum = 0;
                        zi.PlantLoopType = 0;
                        zi.PlantLoopNum = 0;
                        zi.PlantLoopBranch = 0;
                        zi.PlantLoopComp = 0;
                        zi.FirstDemandSidePtr = 0;
                        zi.LastDemandSidePtr = 0;
                    }
                }

                if (CompNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= CompNum; ++i) {
                        auto const &ai = state.dataAirSystemsData->AirSysCompToPlant(i);
                        bool duplicate(false);
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &aj = state.dataAirSystemsData->AirSysCompToPlant(j);
                            if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) &&
                                (ai.AirLoopComp == aj.AirLoopComp)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &aa = state.dataAirSystemsData->AirSysCompToPlant(ArrayCount);
                                aa.AirLoopNum = ai.AirLoopNum;
                                aa.AirLoopBranch = ai.AirLoopBranch;
                                aa.AirLoopComp = ai.AirLoopComp;
                                aa.PlantLoopType = ai.PlantLoopType;
                                aa.PlantLoopNum = ai.PlantLoopNum;
                                aa.PlantLoopBranch = ai.PlantLoopBranch;
                                aa.PlantLoopComp = ai.PlantLoopComp;
                                aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= CompNum; ++i) { // Zero the now-unused entries
                        auto &ai = state.dataAirSystemsData->AirSysCompToPlant(i);
                        ai.AirLoopNum = 0;
                        ai.AirLoopBranch = 0;
                        ai.AirLoopComp = 0;
                        ai.PlantLoopType = 0;
                        ai.PlantLoopNum = 0;
                        ai.PlantLoopBranch = 0;
                        ai.PlantLoopComp = 0;
                        ai.FirstDemandSidePtr = 0;
                        ai.LastDemandSidePtr = 0;
                    }
                }

                if (SubCompNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= SubCompNum; ++i) {
                        auto const &ai = state.dataAirSystemsData->AirSysSubCompToPlant(i);
                        bool duplicate(false);
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &aj = state.dataAirSystemsData->AirSysSubCompToPlant(j);
                            if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) && (ai.AirLoopComp == aj.AirLoopComp) &&
                                (ai.AirLoopSubComp == aj.AirLoopSubComp)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &aa = state.dataAirSystemsData->AirSysSubCompToPlant(ArrayCount);
                                aa.AirLoopNum = ai.AirLoopNum;
                                aa.AirLoopBranch = ai.AirLoopBranch;
                                aa.AirLoopComp = ai.AirLoopComp;
                                aa.AirLoopSubComp = ai.AirLoopSubComp;
                                aa.PlantLoopType = ai.PlantLoopType;
                                aa.PlantLoopNum = ai.PlantLoopNum;
                                aa.PlantLoopBranch = ai.PlantLoopBranch;
                                aa.PlantLoopComp = ai.PlantLoopComp;
                                aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= SubCompNum; ++i) { // Zero the now-unused entries
                        auto &ai = state.dataAirSystemsData->AirSysSubCompToPlant(i);
                        ai.AirLoopNum = 0;
                        ai.AirLoopBranch = 0;
                        ai.AirLoopComp = 0;
                        ai.AirLoopSubComp = 0;
                        ai.PlantLoopType = 0;
                        ai.PlantLoopNum = 0;
                        ai.PlantLoopBranch = 0;
                        ai.PlantLoopComp = 0;
                        ai.FirstDemandSidePtr = 0;
                        ai.LastDemandSidePtr = 0;
                    }
                }

                if (SubSubCompNum > 0) {
                    int ArrayCount = 0;
                    for (int i = 1; i <= SubCompNum; ++i) {
                        auto const &ai = state.dataAirSystemsData->AirSysSubSubCompToPlant(i);
                        bool duplicate = false;
                        for (int j = 1; j <= ArrayCount; ++j) {
                            auto const &aj = state.dataAirSystemsData->AirSysSubSubCompToPlant(j);
                            if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) && (ai.AirLoopComp == aj.AirLoopComp) &&
                                (ai.AirLoopSubComp == aj.AirLoopSubComp) && (ai.AirLoopSubSubComp == aj.AirLoopSubSubComp)) { // Duplicate
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate) {
                            ++ArrayCount;
                            if (i > ArrayCount) { // Copy to lower position
                                auto &aa = state.dataAirSystemsData->AirSysSubSubCompToPlant(ArrayCount);
                                aa.AirLoopNum = ai.AirLoopNum;
                                aa.AirLoopBranch = ai.AirLoopBranch;
                                aa.AirLoopComp = ai.AirLoopComp;
                                aa.AirLoopSubComp = ai.AirLoopSubComp;
                                aa.AirLoopSubSubComp = ai.AirLoopSubSubComp;
                                aa.PlantLoopType = ai.PlantLoopType;
                                aa.PlantLoopNum = ai.PlantLoopNum;
                                aa.PlantLoopBranch = ai.PlantLoopBranch;
                                aa.PlantLoopComp = ai.PlantLoopComp;
                                aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                            }
                        }
                    }
                    for (int i = ArrayCount + 1; i <= SubCompNum; ++i) { // Zero the now-unused entries
                        auto &ai = state.dataAirSystemsData->AirSysSubSubCompToPlant(i);
                        ai.AirLoopNum = 0;
                        ai.AirLoopBranch = 0;
                        ai.AirLoopComp = 0;
                        ai.AirLoopSubComp = 0;
                        ai.AirLoopSubSubComp = 0;
                        ai.PlantLoopType = 0;
                        ai.PlantLoopNum = 0;
                        ai.PlantLoopBranch = 0;
                        ai.PlantLoopComp = 0;
                        ai.FirstDemandSidePtr = 0;
                        ai.LastDemandSidePtr = 0;
                    }
                }

                // 2. Find Supply Side loop for every demand side component
                // The demand side components only need to know what supply side loop
                // they are connected to.  The input and plant data structure will
                // force the loop numbers to be the same.

                // 3. Find Demand Side Component Corresponding to Supply Side Component
                for (int PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops; ++PlantLoopNum) {
                    for (int BranchNum = 1;
                         BranchNum <= state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum).TotalBranches;
                         ++BranchNum) {
                        for (CompNum = 1;
                             CompNum <=
                             state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum).Branch(BranchNum).TotalComponents;
                             ++CompNum) {
                            {
                                auto &thisVentRepComp = state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum)
                                                            .Branch(BranchNum)
                                                            .Comp(CompNum);
                                const std::string &CompType = thisVentRepComp.TypeOf;
                                const std::string &CompName = thisVentRepComp.Name;
                                int MatchLoop = 0;
                                int MatchLoopType = 0;
                                int MatchBranch = 0;
                                int MatchComp = 0;
                                bool MatchFound = false;
                                FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                thisVentRepComp.ConnectPlant.LoopType = MatchLoopType;
                                thisVentRepComp.ConnectPlant.LoopNum = MatchLoop;
                                thisVentRepComp.ConnectPlant.BranchNum = MatchBranch;
                                thisVentRepComp.ConnectPlant.CompNum = MatchComp;
                            }
                        }
                    }
                }

                for (int PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumCondLoops; ++PlantLoopNum) {
                    for (int BranchNum = 1;
                         BranchNum <= state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum).TotalBranches;
                         ++BranchNum) {
                        for (CompNum = 1;
                             CompNum <=
                             state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum).Branch(BranchNum).TotalComponents;
                             ++CompNum) {
                            {
                                auto &thisVentRepComp = state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](PlantLoopNum)
                                                            .Branch(BranchNum)
                                                            .Comp(CompNum);
                                const std::string &CompType = thisVentRepComp.TypeOf;
                                const std::string &CompName = thisVentRepComp.Name;
                                int MatchLoop = 0;
                                int MatchLoopType = 0;
                                int MatchBranch = 0;
                                int MatchComp = 0;
                                bool MatchFound = false;
                                FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                thisVentRepComp.ConnectPlant.LoopType = MatchLoopType;
                                thisVentRepComp.ConnectPlant.LoopNum = MatchLoop;
                                thisVentRepComp.ConnectPlant.BranchNum = MatchBranch;
                                thisVentRepComp.ConnectPlant.CompNum = MatchComp;
                            }
                        }
                    }
                }
            }
        } // Controlled Zone Loop

        // 4.  Now Load all of the plant supply/demand side connections in a single array with pointers from the
        //    connection arrays (ZoneCompToPlant, ZoneSubCompToPlant, ZoneSubSubCompToPlant, AirSysCompToPlant, etc.)
        int NumZoneConnectComps = 0;
        int NumZoneConnectSubComps = 0;
        int NumZoneConnectSubSubComps = 0;
        int NumAirSysConnectComps = 0;
        int NumAirSysConnectSubComps = 0;
        int NumAirSysConnectSubSubComps = 0;
        if (allocated(state.dataAirSystemsData->ZoneCompToPlant)) {
            NumZoneConnectComps = isize(state.dataAirSystemsData->ZoneCompToPlant);
        }
        if (allocated(state.dataAirSystemsData->ZoneSubCompToPlant)) {
            NumZoneConnectSubComps = isize(state.dataAirSystemsData->ZoneSubCompToPlant);
        }
        if (allocated(state.dataAirSystemsData->ZoneSubSubCompToPlant)) {
            NumZoneConnectSubSubComps = isize(state.dataAirSystemsData->ZoneSubSubCompToPlant);
        }
        if (allocated(state.dataAirSystemsData->AirSysCompToPlant)) {
            NumAirSysConnectComps = isize(state.dataAirSystemsData->AirSysCompToPlant);
        }
        if (allocated(state.dataAirSystemsData->AirSysSubCompToPlant)) {
            NumAirSysConnectSubComps = isize(state.dataAirSystemsData->AirSysSubCompToPlant);
        }
        if (allocated(state.dataAirSystemsData->AirSysSubSubCompToPlant)) {
            NumAirSysConnectSubSubComps = isize(state.dataAirSystemsData->AirSysSubSubCompToPlant);
        }
        state.dataSysRpts->OneTimeFlag_InitEnergyReports = false;

        int ArrayCount = 0;
        for (int CompNum = 1; CompNum <= NumZoneConnectComps; ++CompNum) {
            int LoopType = state.dataAirSystemsData->ZoneCompToPlant(CompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->ZoneCompToPlant(CompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;
            bool ConnectionFlag = false;

            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->ZoneCompToPlant(CompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->ZoneCompToPlant(CompNum).LastDemandSidePtr = LastIndex;
            }
        }

        for (int SubCompNum = 1; SubCompNum <= NumZoneConnectSubComps; ++SubCompNum) {
            int LoopType = state.dataAirSystemsData->ZoneSubCompToPlant(SubCompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->ZoneSubCompToPlant(SubCompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;

            bool ConnectionFlag = false;
            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->ZoneSubCompToPlant(SubCompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->ZoneSubCompToPlant(SubCompNum).LastDemandSidePtr = LastIndex;
            }
        }

        for (int SubSubCompNum = 1; SubSubCompNum <= NumZoneConnectSubSubComps; ++SubSubCompNum) {
            int LoopType = state.dataAirSystemsData->ZoneSubSubCompToPlant(SubSubCompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->ZoneSubSubCompToPlant(SubSubCompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;
            bool ConnectionFlag = false;

            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->ZoneSubSubCompToPlant(SubSubCompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->ZoneSubSubCompToPlant(SubSubCompNum).LastDemandSidePtr = LastIndex;
            }
        }
        for (int CompNum = 1; CompNum <= NumAirSysConnectComps; ++CompNum) {
            int LoopType = state.dataAirSystemsData->AirSysCompToPlant(CompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->AirSysCompToPlant(CompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;
            bool ConnectionFlag = false;

            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->AirSysCompToPlant(CompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->AirSysCompToPlant(CompNum).LastDemandSidePtr = LastIndex;
            }
        }

        for (int SubCompNum = 1; SubCompNum <= NumAirSysConnectSubComps; ++SubCompNum) {
            int LoopType = state.dataAirSystemsData->AirSysSubCompToPlant(SubCompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->AirSysSubCompToPlant(SubCompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;
            bool ConnectionFlag = false;

            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->AirSysSubCompToPlant(SubCompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->AirSysSubCompToPlant(SubCompNum).LastDemandSidePtr = LastIndex;
            }
        }

        for (int SubSubCompNum = 1; SubSubCompNum <= NumAirSysConnectSubSubComps; ++SubSubCompNum) {
            int LoopType = state.dataAirSystemsData->AirSysSubSubCompToPlant(SubSubCompNum).PlantLoopType;
            int LoopNum = state.dataAirSystemsData->AirSysSubSubCompToPlant(SubSubCompNum).PlantLoopNum;
            int FirstIndex = ArrayCount + 1;
            int LoopCount = 1;
            bool ConnectionFlag = false;

            if (LoopType > 0 && LoopNum > 0) {
                FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
            }

            int LastIndex = ArrayCount;
            if (FirstIndex > LastIndex) {
                FirstIndex = LastIndex;
            }
            if (ConnectionFlag) {
                state.dataAirSystemsData->AirSysSubSubCompToPlant(SubSubCompNum).FirstDemandSidePtr = FirstIndex;
                state.dataAirSystemsData->AirSysSubSubCompToPlant(SubSubCompNum).LastDemandSidePtr = LastIndex;
            }
        }

        reportAirLoopToplogy(state);

        reportZoneEquipmentToplogy(state);

        reportAirDistributionUnits(state);

        state.dataSysRpts->OneTimeFlag_InitEnergyReports = false;
    }

    // On every iteration, load the air loop energy data
    for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
        auto &pas = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
        for (int BranchNum = 1; BranchNum <= pas.NumBranches; ++BranchNum) {
            auto &pasBranch = pas.Branch(BranchNum);
            for (int CompNum = 1; CompNum <= pasBranch.TotalComponents; ++CompNum) {
                auto &pasBranchComp = pasBranch.Comp(CompNum);
                for (int VarNum = 1; VarNum <= pasBranchComp.NumMeteredVars; ++VarNum) {
                    auto &pasBranchCompMeter = pasBranchComp.MeteredVar(VarNum);
                    OutputProcessor::VariableType VarType = pasBranchCompMeter.varType;
                    int VarIndex = pasBranchCompMeter.num;
                    pasBranchCompMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                }
                for (int SubCompNum = 1; SubCompNum <= pasBranchComp.NumSubComps; ++SubCompNum) {
                    auto &pasBranchSubComp = pasBranchComp.SubComp(SubCompNum);
                    for (int VarNum = 1; VarNum <= pasBranchSubComp.NumMeteredVars; ++VarNum) {
                        auto &pasBranchSubCompMeter = pasBranchSubComp.MeteredVar(VarNum);
                        OutputProcessor::VariableType VarType = pasBranchSubCompMeter.varType;
                        int VarIndex = pasBranchSubCompMeter.num;
                        pasBranchSubCompMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                    }
                    for (int SubSubCompNum = 1; SubSubCompNum <= pasBranchSubComp.NumSubSubComps; ++SubSubCompNum) {
                        auto &pasBranchSubSubComp = pasBranchSubComp.SubSubComp(SubSubCompNum);
                        for (int VarNum = 1; VarNum <= pasBranchSubSubComp.NumMeteredVars; ++VarNum) {
                            auto &pasBranchSubSubCompMeter = pasBranchSubSubComp.MeteredVar(VarNum);
                            OutputProcessor::VariableType VarType = pasBranchSubSubCompMeter.varType;
                            int VarIndex = pasBranchSubSubCompMeter.num;
                            pasBranchSubSubCompMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                        }
                    }
                }
            }
        }
    }

    // On every iteration, load the zone equipment energy data
    for (int ListNum = 1; ListNum <= state.dataGlobal->NumOfZones; ++ListNum) {
        if (!state.dataZoneEquip->ZoneEquipConfig(ListNum).IsControlled) {
            continue;
        }
        auto &zel = state.dataZoneEquip->ZoneEquipList(ListNum);
        for (int CompNum = 1; CompNum <= zel.NumOfEquipTypes; ++CompNum) {
            auto &zelEquipData = zel.EquipData(CompNum);
            for (int VarNum = 1; VarNum <= zelEquipData.NumMeteredVars; ++VarNum) {
                auto &zelEquipDataMeter = zelEquipData.MeteredVar(VarNum);
                OutputProcessor::VariableType VarType = zelEquipDataMeter.varType;
                int VarIndex = zelEquipDataMeter.num;
                zelEquipDataMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
            }
            for (int SubCompNum = 1; SubCompNum <= zelEquipData.NumSubEquip; ++SubCompNum) {
                auto &zelSubEquipData = zelEquipData.SubEquipData(SubCompNum);
                for (int VarNum = 1; VarNum <= zelSubEquipData.NumMeteredVars; ++VarNum) {
                    auto &zelSubEquipDataMeter = zelSubEquipData.MeteredVar(VarNum);
                    OutputProcessor::VariableType VarType = zelSubEquipDataMeter.varType;
                    int VarIndex = zelSubEquipDataMeter.num;
                    zelSubEquipDataMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                }
                for (int SubSubCompNum = 1; SubSubCompNum <= zelSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                    auto &zelSubSubEquipData = zelSubEquipData.SubSubEquipData(SubSubCompNum);
                    for (int VarNum = 1; VarNum <= zelSubSubEquipData.NumMeteredVars; ++VarNum) {
                        auto &zelSubSubEquipDataMeter = zelSubSubEquipData.MeteredVar(VarNum);
                        OutputProcessor::VariableType VarType = zelSubSubEquipDataMeter.varType;
                        int VarIndex = zelSubSubEquipDataMeter.num;
                        zelSubSubEquipDataMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex); // Sankar Corrected zone array
                    }
                }
            }
        }
    }

    // On every iteration, load the Plant Supply Side Data and load the Plant Demand Side Data
    for (LoopSideLocation LoopSide : DataPlant::LoopSideKeys) {
        for (int PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops; ++PlantLoopNum) {
            auto &vrp = state.dataPlnt->VentRepPlant[static_cast<int>(LoopSide)](PlantLoopNum);
            for (int BranchNum = 1; BranchNum <= vrp.TotalBranches; ++BranchNum) {
                auto &vrpBranch = vrp.Branch(BranchNum);
                for (int CompNum = 1; CompNum <= vrpBranch.TotalComponents; ++CompNum) {
                    auto &vrpBranchComp = vrpBranch.Comp(CompNum);
                    for (int VarNum = 1; VarNum <= vrpBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrpBranchCompMeter = vrpBranchComp.MeteredVar(VarNum);
                        OutputProcessor::VariableType VarType = vrpBranchCompMeter.varType;
                        int VarIndex = vrpBranchCompMeter.num;
                        vrpBranchCompMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                    }
                }
            }
        }

        // On every iteration, load the Condenser Supply Side Data and load the Condenser Demand Side Data
        for (int PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumCondLoops; ++PlantLoopNum) {
            auto &vrc = state.dataPlnt->VentRepCond[static_cast<int>(LoopSide)](PlantLoopNum);
            for (int BranchNum = 1; BranchNum <= vrc.TotalBranches; ++BranchNum) {
                auto &vrcBranch = vrc.Branch(BranchNum);
                for (int CompNum = 1; CompNum <= vrcBranch.TotalComponents; ++CompNum) {
                    auto &vrcBranchComp = vrcBranch.Comp(CompNum);
                    for (int VarNum = 1; VarNum <= vrcBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrcBranchCompMeter = vrcBranchComp.MeteredVar(VarNum);
                        OutputProcessor::VariableType VarType = vrcBranchCompMeter.varType;
                        int VarIndex = vrcBranchCompMeter.num;
                        vrcBranchCompMeter.curMeterReading = GetInternalVariableValue(state, VarType, VarIndex);
                    }
                }
            }
        }
    }
    // initialize energy report variables
}

void FindFirstLastPtr(EnergyPlusData &state, int &LoopType, int &LoopNum, int &ArrayCount, int &LoopCount, bool &ConnectionFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   July 2005
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Initializes the energy components of the data structures

    // METHODOLOGY EMPLOYED:
    // Once all compsets have been established (second iteration) find all components
    // subcomponents, etc.

    int DemandSideLoopNum;
    int DemandSideBranchNum;
    int DemandSideCompNum;
    int SupplySideCompNum;
    int DemandSideLoopType;
    bool found;

    // Object Data
    auto &LoopStack = state.dataSysRpts->LoopStack;

    return; // Autodesk:? Is this routine now an intentional NOOP?

    if (state.dataSysRpts->OneTimeFlag_FindFirstLastPtr) {
        LoopStack.allocate(state.dataSysRpts->MaxLoopArraySize);
        state.dataAirSystemsData->DemandSideConnect.allocate(state.dataSysRpts->MaxCompArraySize);

        state.dataSysRpts->OneTimeFlag_FindFirstLastPtr = false;
    }
    for (auto &e : LoopStack) {
        e.LoopNum = 0;
        e.LoopType = 0;
    }

    ConnectionFlag = false;
    //    countloop=0
    //    write(outputfiledebug,*) '1228=lt,lc,lnum,cflag,arrcnt',looptype,loopcount,LoopNum,connectionflag,arraycount

    while (LoopCount > 0) {
        //        write(outputfiledebug,*) '1231==lt,lc,lnum,cflag,arrcnt',looptype,loopcount,LoopNum,connectionflag,arraycount
        //        write(outputfiledebug,*) 'loop=plname',TRIM(plantloop(LoopNum)%name)
        --LoopCount;
        //        countloop=countloop+1
        //        if (countloop > 100) exit
        if (LoopType == 1) {
            for (int BranchNum = 1; BranchNum <= state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](LoopNum).TotalBranches;
                 ++BranchNum) {
                for (int SupplySideCompNum = 1;
                     SupplySideCompNum <=
                     state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](LoopNum).Branch(BranchNum).TotalComponents;
                     ++SupplySideCompNum) {
                    {
                        auto const &thisVentRepComp =
                            state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Supply)](LoopNum).Branch(BranchNum).Comp(
                                SupplySideCompNum);
                        DemandSideLoopType = thisVentRepComp.ConnectPlant.LoopType;
                        DemandSideLoopNum = thisVentRepComp.ConnectPlant.LoopNum;
                        DemandSideBranchNum = thisVentRepComp.ConnectPlant.BranchNum;
                        DemandSideCompNum = thisVentRepComp.ConnectPlant.CompNum;
                    }
                    // If the connection is valid load the connection array
                    if (DemandSideLoopType == 1 || DemandSideLoopType == 2) {
                        ConnectionFlag = true;
                        ++ArrayCount;
                        if (ArrayCount > state.dataSysRpts->MaxCompArraySize) {
                            state.dataAirSystemsData->DemandSideConnect.redimension(state.dataSysRpts->MaxCompArraySize += 100);
                        }
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).LoopType = DemandSideLoopType;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).LoopNum = DemandSideLoopNum;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).BranchNum = DemandSideBranchNum;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).CompNum = DemandSideCompNum;

                        found = false;
                        print(state.files.debug, "1271=lstacksize {}\n", size(LoopStack));
                        for (int Idx = 1; Idx <= isize(LoopStack); ++Idx) {
                            if (DemandSideLoopNum == LoopStack(Idx).LoopNum && DemandSideLoopType == LoopStack(Idx).LoopType) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            ++LoopCount;
                            //       write(outputfiledebug,*) '1280=lc,mxsize',loopcount,maxlooparraysize
                            //       write(outputfiledebug,*) '1281=dsloopnum,dslooptype',DemandSideLoopNum,DemandSideLoopType
                            if (LoopCount > state.dataSysRpts->MaxLoopArraySize) {
                                LoopStack.redimension(state.dataSysRpts->MaxLoopArraySize += 100);
                            }
                            //               write(outputfiledebug,*)
                            //               '1294=lcnt,dsloopnum,dslooptype',loopcount,DemandSideLoopNum,DemandSideLoopType
                            LoopStack(LoopCount).LoopNum = DemandSideLoopNum;
                            LoopStack(LoopCount).LoopType = DemandSideLoopType;
                        }
                    }
                }
            }
        } else if (LoopType == 2) {
            for (int BranchNum = 1; BranchNum <= state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](LoopNum).TotalBranches;
                 ++BranchNum) {
                for (SupplySideCompNum = 1;
                     SupplySideCompNum <=
                     state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](LoopNum).Branch(BranchNum).TotalComponents;
                     ++SupplySideCompNum) {
                    {
                        auto const &thisVentRepComp =
                            state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Supply)](LoopNum).Branch(BranchNum).Comp(
                                SupplySideCompNum);
                        DemandSideLoopType = thisVentRepComp.ConnectPlant.LoopType;
                        DemandSideLoopNum = thisVentRepComp.ConnectPlant.LoopNum;
                        DemandSideBranchNum = thisVentRepComp.ConnectPlant.BranchNum;
                        DemandSideCompNum = thisVentRepComp.ConnectPlant.CompNum;
                    }
                    // If the connection is valid load the connection array
                    if (DemandSideLoopType == 1 || DemandSideLoopType == 2) {
                        ConnectionFlag = true;
                        ++ArrayCount;
                        if (ArrayCount > state.dataSysRpts->MaxCompArraySize) {
                            state.dataAirSystemsData->DemandSideConnect.redimension(state.dataSysRpts->MaxCompArraySize += 100);
                        }
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).LoopType = DemandSideLoopType;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).LoopNum = DemandSideLoopNum;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).BranchNum = DemandSideBranchNum;
                        state.dataAirSystemsData->DemandSideConnect(ArrayCount).CompNum = DemandSideCompNum;

                        found = false;
                        for (int Idx = 1; Idx <= isize(LoopStack); ++Idx) {
                            if (DemandSideLoopNum == LoopStack(Idx).LoopNum && DemandSideLoopType == LoopStack(Idx).LoopType) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            ++LoopCount;
                            //       write(outputfiledebug,*) '1341=lcnt,arrsize',loopcount,maxlooparraysize
                            //       write(outputfiledebug,*) '1342=lsloopnum,dslooptype',DemandSideLoopNum,DemandSideLoopType
                            if (LoopCount > state.dataSysRpts->MaxLoopArraySize) {
                                LoopStack.redimension(state.dataSysRpts->MaxLoopArraySize += 100);
                            }
                            LoopStack(LoopCount).LoopNum = DemandSideLoopNum;
                            LoopStack(LoopCount).LoopType = DemandSideLoopType;
                        }
                    }
                }
            }
        } else {
            print(state.files.debug, "{}\n", "1361=error");
            // error
        }

        // now unload the LoopNum and LoopType arrays
        if (LoopCount > 0) {
            LoopType = LoopStack(LoopCount).LoopType;
            LoopNum = LoopStack(LoopCount).LoopNum;
        }

    } // While loop
}

void UpdateZoneCompPtrArray(EnergyPlusData &state,
                            int &Idx,
                            int const ListNum,
                            int const AirDistUnitNum,
                            int const PlantLoopType,
                            int const PlantLoop,
                            int const PlantBranch,
                            int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Zone Component pointers

    if (state.dataSysRpts->OneTimeFlag_UpdateZoneCompPtrArray) {
        state.dataAirSystemsData->ZoneCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateZoneCompPtrArray);
        for (auto &e : state.dataAirSystemsData->ZoneCompToPlant) {
            e.ZoneEqListNum = 0;
            e.ZoneEqCompNum = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateZoneCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateZoneCompPtrArray >= state.dataSysRpts->ArrayLimit_UpdateZoneCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateZoneCompPtrArray);
        state.dataAirSystemsData->ZoneCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateZoneCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateZoneCompPtrArray; ++i) {
            auto &zctp = state.dataAirSystemsData->ZoneCompToPlant(i);
            zctp.ZoneEqListNum = 0;
            zctp.ZoneEqCompNum = 0;
            zctp.PlantLoopType = 0;
            zctp.PlantLoopNum = 0;
            zctp.PlantLoopBranch = 0;
            zctp.PlantLoopComp = 0;
            zctp.FirstDemandSidePtr = 0;
            zctp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateZoneCompPtrArray;
    auto &zctp = state.dataAirSystemsData->ZoneCompToPlant(Idx);
    zctp.ZoneEqListNum = ListNum;
    zctp.ZoneEqCompNum = AirDistUnitNum;
    zctp.PlantLoopType = PlantLoopType;
    zctp.PlantLoopNum = PlantLoop;
    zctp.PlantLoopBranch = PlantBranch;
    zctp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateZoneCompPtrArray;
}

void UpdateZoneSubCompPtrArray(EnergyPlusData &state,
                               int &Idx,
                               int const ListNum,
                               int const AirDistUnitNum,
                               int const SubCompNum,
                               int const PlantLoopType,
                               int const PlantLoop,
                               int const PlantBranch,
                               int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Zone Sub Component Pointer Array

    if (state.dataSysRpts->OneTimeFlag_UpdateZoneSubCompPtrArray) {
        state.dataAirSystemsData->ZoneSubCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateZoneSubCompPtrArray);
        for (auto &e : state.dataAirSystemsData->ZoneSubCompToPlant) {
            e.ZoneEqListNum = 0;
            e.ZoneEqCompNum = 0;
            e.ZoneEqSubCompNum = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateZoneSubCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateZoneSubCompPtrArray >= state.dataSysRpts->ArrayLimit_UpdateZoneSubCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateZoneSubCompPtrArray);
        state.dataAirSystemsData->ZoneSubCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateZoneSubCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateZoneSubCompPtrArray; ++i) {
            auto &zctp = state.dataAirSystemsData->ZoneSubCompToPlant(i);
            zctp.ZoneEqListNum = 0;
            zctp.ZoneEqCompNum = 0;
            zctp.ZoneEqSubCompNum = 0;
            zctp.PlantLoopType = 0;
            zctp.PlantLoopNum = 0;
            zctp.PlantLoopBranch = 0;
            zctp.PlantLoopComp = 0;
            zctp.FirstDemandSidePtr = 0;
            zctp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateZoneSubCompPtrArray;
    auto &zctp = state.dataAirSystemsData->ZoneSubCompToPlant(Idx);
    zctp.ZoneEqListNum = ListNum;
    zctp.ZoneEqCompNum = AirDistUnitNum;
    zctp.ZoneEqSubCompNum = SubCompNum;
    zctp.PlantLoopType = PlantLoopType;
    zctp.PlantLoopNum = PlantLoop;
    zctp.PlantLoopBranch = PlantBranch;
    zctp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateZoneSubCompPtrArray;
}

void UpdateZoneSubSubCompPtrArray(EnergyPlusData &state,
                                  int &Idx,
                                  int const ListNum,
                                  int const AirDistUnitNum,
                                  int const SubCompNum,
                                  int const SubSubCompNum,
                                  int const PlantLoopType,
                                  int const PlantLoop,
                                  int const PlantBranch,
                                  int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Zone Sub Component Pointer Array

    if (state.dataSysRpts->OneTimeFlag_UpdateZoneSubSubCompPtrArray) {
        state.dataAirSystemsData->ZoneSubSubCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateZoneSubSubCompPtrArray);
        for (auto &e : state.dataAirSystemsData->ZoneSubSubCompToPlant) {
            e.ZoneEqListNum = 0;
            e.ZoneEqCompNum = 0;
            e.ZoneEqSubCompNum = 0;
            e.ZoneEqSubSubCompNum = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateZoneSubSubCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateZoneSubSubCompPtrArray >=
        state.dataSysRpts->ArrayLimit_UpdateZoneSubSubCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateZoneSubSubCompPtrArray);
        state.dataAirSystemsData->ZoneSubSubCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateZoneSubSubCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateZoneSubSubCompPtrArray; ++i) {
            auto &zctp = state.dataAirSystemsData->ZoneSubSubCompToPlant(i);
            zctp.ZoneEqListNum = 0;
            zctp.ZoneEqCompNum = 0;
            zctp.ZoneEqSubCompNum = 0;
            zctp.ZoneEqSubSubCompNum = 0;
            zctp.PlantLoopType = 0;
            zctp.PlantLoopNum = 0;
            zctp.PlantLoopBranch = 0;
            zctp.PlantLoopComp = 0;
            zctp.FirstDemandSidePtr = 0;
            zctp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateZoneSubSubCompPtrArray;
    auto &zctp = state.dataAirSystemsData->ZoneSubSubCompToPlant(Idx);
    zctp.ZoneEqListNum = ListNum;
    zctp.ZoneEqCompNum = AirDistUnitNum;
    zctp.ZoneEqSubCompNum = SubCompNum;
    zctp.ZoneEqSubSubCompNum = SubSubCompNum;
    zctp.PlantLoopType = PlantLoopType;
    zctp.PlantLoopNum = PlantLoop;
    zctp.PlantLoopBranch = PlantBranch;
    zctp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateZoneSubSubCompPtrArray;
}

void UpdateAirSysCompPtrArray(EnergyPlusData &state,
                              int &Idx,
                              int const AirLoopNum,
                              int const BranchNum,
                              int const CompNum,
                              int const PlantLoopType,
                              int const PlantLoop,
                              int const PlantBranch,
                              int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Air System Component Pointer Array

    if (state.dataSysRpts->OneTimeFlag_UpdateAirSysCompPtrArray) {
        state.dataAirSystemsData->AirSysCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateAirSysCompPtrArray);
        for (auto &e : state.dataAirSystemsData->AirSysCompToPlant) {
            e.AirLoopNum = 0;
            e.AirLoopBranch = 0;
            e.AirLoopComp = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateAirSysCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateAirSysCompPtrArray >= state.dataSysRpts->ArrayLimit_UpdateAirSysCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateAirSysCompPtrArray);
        state.dataAirSystemsData->AirSysCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateAirSysCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateAirSysCompPtrArray; ++i) {
            auto &actp = state.dataAirSystemsData->AirSysCompToPlant(i);
            actp.AirLoopNum = 0;
            actp.AirLoopBranch = 0;
            actp.AirLoopComp = 0;
            actp.PlantLoopType = 0;
            actp.PlantLoopNum = 0;
            actp.PlantLoopBranch = 0;
            actp.PlantLoopComp = 0;
            actp.FirstDemandSidePtr = 0;
            actp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateAirSysCompPtrArray;
    auto &actp = state.dataAirSystemsData->AirSysCompToPlant(Idx);
    actp.AirLoopNum = AirLoopNum;
    actp.AirLoopBranch = BranchNum;
    actp.AirLoopComp = CompNum;
    actp.PlantLoopType = PlantLoopType;
    actp.PlantLoopNum = PlantLoop;
    actp.PlantLoopBranch = PlantBranch;
    actp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateAirSysCompPtrArray;
}

void UpdateAirSysSubCompPtrArray(EnergyPlusData &state,
                                 int &Idx,
                                 int const AirLoopNum,
                                 int const BranchNum,
                                 int const CompNum,
                                 int const SubCompNum,
                                 int const PlantLoopType,
                                 int const PlantLoop,
                                 int const PlantBranch,
                                 int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Air System Sub Component Pointer Array

    if (state.dataSysRpts->OneTimeFlag_UpdateAirSysSubCompPtrArray) {
        state.dataAirSystemsData->AirSysSubCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateAirSysSubCompPtrArray);
        for (auto &e : state.dataAirSystemsData->AirSysSubCompToPlant) {
            e.AirLoopNum = 0;
            e.AirLoopBranch = 0;
            e.AirLoopComp = 0;
            e.AirLoopSubComp = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateAirSysSubCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateAirSysSubCompPtrArray >=
        state.dataSysRpts->ArrayLimit_UpdateAirSysSubCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateAirSysSubCompPtrArray);
        state.dataAirSystemsData->AirSysSubCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateAirSysSubCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateAirSysSubCompPtrArray; ++i) {
            auto &actp = state.dataAirSystemsData->AirSysSubCompToPlant(i);
            actp.AirLoopNum = 0;
            actp.AirLoopBranch = 0;
            actp.AirLoopComp = 0;
            actp.AirLoopSubComp = 0;
            actp.PlantLoopType = 0;
            actp.PlantLoopNum = 0;
            actp.PlantLoopBranch = 0;
            actp.PlantLoopComp = 0;
            actp.FirstDemandSidePtr = 0;
            actp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateAirSysSubCompPtrArray;
    auto &actp = state.dataAirSystemsData->AirSysSubCompToPlant(Idx);
    actp.AirLoopNum = AirLoopNum;
    actp.AirLoopBranch = BranchNum;
    actp.AirLoopComp = CompNum;
    actp.AirLoopSubComp = SubCompNum;
    actp.PlantLoopType = PlantLoopType;
    actp.PlantLoopNum = PlantLoop;
    actp.PlantLoopBranch = PlantBranch;
    actp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateAirSysSubCompPtrArray;
}

void UpdateAirSysSubSubCompPtrArray(EnergyPlusData &state,
                                    int &Idx,
                                    int const AirLoopNum,
                                    int const BranchNum,
                                    int const CompNum,
                                    int const SubCompNum,
                                    int const SubSubCompNum,
                                    int const PlantLoopType,
                                    int const PlantLoop,
                                    int const PlantBranch,
                                    int const PlantComp)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   June 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update Air System Sub Sub Component Pointer Array

    if (state.dataSysRpts->OneTimeFlag_UpdateAirSysSubSubCompPtrArray) {
        state.dataAirSystemsData->AirSysSubSubCompToPlant.allocate(state.dataSysRpts->ArrayLimit_UpdateAirSysSubSubCompPtrArray);
        for (auto &e : state.dataAirSystemsData->AirSysSubSubCompToPlant) {
            e.AirLoopNum = 0;
            e.AirLoopBranch = 0;
            e.AirLoopComp = 0;
            e.AirLoopSubComp = 0;
            e.AirLoopSubSubComp = 0;
            e.PlantLoopType = 0;
            e.PlantLoopNum = 0;
            e.PlantLoopBranch = 0;
            e.PlantLoopComp = 0;
            e.FirstDemandSidePtr = 0;
            e.LastDemandSidePtr = 0;
        }

        state.dataSysRpts->OneTimeFlag_UpdateAirSysSubSubCompPtrArray = false;
    }

    if (state.dataSysRpts->ArrayCounter_UpdateAirSysSubSubCompPtrArray >=
        state.dataSysRpts->ArrayLimit_UpdateAirSysSubSubCompPtrArray) { // Redimension larger
        int const OldArrayLimit(state.dataSysRpts->ArrayLimit_UpdateAirSysSubSubCompPtrArray);
        state.dataAirSystemsData->AirSysSubSubCompToPlant.redimension(state.dataSysRpts->ArrayLimit_UpdateAirSysSubSubCompPtrArray *= 2);
        for (int i = OldArrayLimit + 1; i <= state.dataSysRpts->ArrayLimit_UpdateAirSysSubSubCompPtrArray; ++i) {
            auto &actp = state.dataAirSystemsData->AirSysSubSubCompToPlant(i);
            actp.AirLoopNum = 0;
            actp.AirLoopBranch = 0;
            actp.AirLoopComp = 0;
            actp.AirLoopSubComp = 0;
            actp.AirLoopSubSubComp = 0;
            actp.PlantLoopType = 0;
            actp.PlantLoopNum = 0;
            actp.PlantLoopBranch = 0;
            actp.PlantLoopComp = 0;
            actp.FirstDemandSidePtr = 0;
            actp.LastDemandSidePtr = 0;
        }
    }

    Idx = state.dataSysRpts->ArrayCounter_UpdateAirSysSubSubCompPtrArray;
    auto &actp = state.dataAirSystemsData->AirSysSubSubCompToPlant(Idx);
    actp.AirLoopNum = AirLoopNum;
    actp.AirLoopBranch = BranchNum;
    actp.AirLoopComp = CompNum;
    actp.AirLoopSubComp = SubCompNum;
    actp.AirLoopSubSubComp = SubSubCompNum;
    actp.PlantLoopType = PlantLoopType;
    actp.PlantLoopNum = PlantLoop;
    actp.PlantLoopBranch = PlantBranch;
    actp.PlantLoopComp = PlantComp;
    ++state.dataSysRpts->ArrayCounter_UpdateAirSysSubSubCompPtrArray;
}

void AllocateAndSetUpVentReports(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Sankaranarayanan K P
    //       DATE WRITTEN   July 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    int NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;

    // PURPOSE OF THIS SUBROUTINE:
    // Allocates Arrays and setup output variables related to Ventilation reports.
    state.dataSysRpts->ZoneVentRepVars.allocate(state.dataGlobal->NumOfZones);
    state.dataSysRpts->SysLoadRepVars.allocate(NumPrimaryAirSys);
    state.dataSysRpts->SysVentRepVars.allocate(NumPrimaryAirSys);
    state.dataSysRpts->SysPreDefRep.allocate(NumPrimaryAirSys);

    for (int sysIndex = 1; sysIndex <= NumPrimaryAirSys; ++sysIndex) {
        auto &thisSysVentRepVars = state.dataSysRpts->SysVentRepVars(sysIndex);
        thisSysVentRepVars.MechVentFlow = 0.0;
        thisSysVentRepVars.NatVentFlow = 0.0;
        thisSysVentRepVars.TargetVentilationFlowVoz = 0.0;
        thisSysVentRepVars.TimeBelowVozDyn = 0.0;
        thisSysVentRepVars.TimeAtVozDyn = 0.0;
        thisSysVentRepVars.TimeAboveVozDyn = 0.0;
        thisSysVentRepVars.TimeVentUnocc = 0.0;
        thisSysVentRepVars.AnyZoneOccupied = false;
    }

    if (state.dataSysRpts->AirLoopLoadsReportEnabled) {
        for (int sysIndex = 1; sysIndex <= NumPrimaryAirSys; ++sysIndex) {
            auto &thisSysLoadRepVars = state.dataSysRpts->SysLoadRepVars(sysIndex);
            auto &thisSysVentRepVars = state.dataSysRpts->SysVentRepVars(sysIndex);
            std::string const primaryAirSysName = state.dataAirSystemsData->PrimaryAirSystems(sysIndex).Name;

            // CurrentModuleObject='AirloopHVAC'
            // SYSTEM LOADS REPORT
            SetupOutputVariable(state,
                                "Air System Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotHTNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotCLNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            // SYSTEM ENERGY USE REPORT
            SetupOutputVariable(state,
                                "Air System Hot Water Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotH2OHOT,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Steam Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotSteam,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Chilled Water Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotH2OCOLD,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System NaturalGas Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotNaturalGas,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Propane Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.TotPropane,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Water Volume",
                                Constant::Units::m3,
                                thisSysLoadRepVars.DomesticH2O,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            // SYSTEM COMPONENT LOAD REPORT
            SetupOutputVariable(state,
                                "Air System Fan Air Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.FANCompHTNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Cooling Coil Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.CCCompCLNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompHTNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heat Exchanger Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HeatExHTNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heat Exchanger Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HeatExCLNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Solar Collector Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.SolarCollectHeating,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Solar Collector Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.SolarCollectCooling,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System User Defined Air Terminal Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.UserDefinedTerminalHeating,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System User Defined Air Terminal Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.UserDefinedTerminalCooling,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Humidifier Total Heating Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HumidHTNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Evaporative Cooler Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.EvapCLNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Desiccant Dehumidifier Total Cooling Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.DesDehumidCLNG,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            // SYSTEM COMPONENT ENERGY REPORT
            SetupOutputVariable(state,
                                "Air System Fan Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.FANCompElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil Hot Water Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompH2OHOT,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Cooling Coil Chilled Water Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.CCCompH2OCOLD,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System DX Heating Coil Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System DX Cooling Coil Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.CCCompElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompElecRes,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil NaturalGas Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompNaturalGas,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil Propane Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompPropane,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Heating Coil Steam Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HCCompSteam,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Humidifier Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HumidElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Humidifier NaturalGas Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HumidNaturalGas,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Humidifier Propane Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.HumidPropane,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Evaporative Cooler Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.EvapElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Desiccant Dehumidifier Electricity Energy",
                                Constant::Units::J,
                                thisSysLoadRepVars.DesDehumidElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Mechanical Ventilation Flow Rate",
                                Constant::Units::m3_s,
                                thisSysVentRepVars.MechVentFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Natural Ventilation Flow Rate",
                                Constant::Units::m3_s,
                                thisSysVentRepVars.NatVentFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Target Voz Ventilation Flow Rate",
                                Constant::Units::m3_s,
                                thisSysVentRepVars.TargetVentilationFlowVoz,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Ventilation Below Target Voz Time",
                                Constant::Units::hr,
                                thisSysVentRepVars.TimeBelowVozDyn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Ventilation At Target Voz Time",
                                Constant::Units::hr,
                                thisSysVentRepVars.TimeAtVozDyn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Ventilation Above Target Voz Time",
                                Constant::Units::hr,
                                thisSysVentRepVars.TimeAboveVozDyn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);

            SetupOutputVariable(state,
                                "Air System Ventilation When Unoccupied Time",
                                Constant::Units::hr,
                                thisSysVentRepVars.TimeVentUnocc,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                primaryAirSysName);
        }
    }
    for (int ZoneIndex = 1; ZoneIndex <= state.dataGlobal->NumOfZones; ++ZoneIndex) {
        if (!state.dataZoneEquip->ZoneEquipConfig(ZoneIndex).IsControlled) {
            continue;
        }
        auto &thisZoneVentRepVars = state.dataSysRpts->ZoneVentRepVars(ZoneIndex);
        auto &thisZoneName = state.dataZoneEquip->ZoneEquipConfig(ZoneIndex).ZoneName;
        // CurrentModuleObject='Zones(Controlled)'
        if (state.dataSysRpts->VentLoadsReportEnabled) {
            // Cooling Loads
            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation No Load Heat Removal Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.NoLoadCoolingByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Cooling Load Increase Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.CoolingLoadAddedByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Cooling Load Increase Due to Overheating Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.OverheatingByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Cooling Load Decrease Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.CoolingLoadMetByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);
            // Heating Loads
            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation No Load Heat Addition Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.NoLoadHeatingByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Heating Load Increase Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.HeatingLoadAddedByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Heating Load Increase Due to Overcooling Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.OvercoolingByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);

            SetupOutputVariable(state,
                                "Zone Mechanical Ventilation Heating Load Decrease Energy",
                                Constant::Units::J,
                                thisZoneVentRepVars.HeatingLoadMetByVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisZoneName);
        }

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisZoneVentRepVars.OAMassFlow,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Mass",
                            Constant::Units::kg,
                            thisZoneVentRepVars.OAMass,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            thisZoneVentRepVars.OAVolFlowStdRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Standard Density Volume",
                            Constant::Units::m3,
                            thisZoneVentRepVars.OAVolStdRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Current Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            thisZoneVentRepVars.OAVolFlowCrntRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Current Density Volume",
                            Constant::Units::m3,
                            thisZoneVentRepVars.OAVolCrntRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Mechanical Ventilation Air Changes per Hour",
                            Constant::Units::ach,
                            thisZoneVentRepVars.MechACH,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Target Voz Ventilation Flow Rate",
                            Constant::Units::m3_s,
                            thisZoneVentRepVars.TargetVentilationFlowVoz,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Ventilation Below Target Voz Time",
                            Constant::Units::hr,
                            thisZoneVentRepVars.TimeBelowVozDyn,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Ventilation At Target Voz Time",
                            Constant::Units::hr,
                            thisZoneVentRepVars.TimeAtVozDyn,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Ventilation Above Target Voz Time",
                            Constant::Units::hr,
                            thisZoneVentRepVars.TimeAboveVozDyn,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);

        SetupOutputVariable(state,
                            "Zone Ventilation When Unoccupied Time",
                            Constant::Units::hr,
                            thisZoneVentRepVars.TimeVentUnocc,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneName);
    }

    // Facility outputs
    SetupOutputVariable(state,
                        "Facility Any Zone Ventilation Below Target Voz Time",
                        Constant::Units::hr,
                        state.dataSysRpts->AnyZoneTimeBelowVozDyn,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Facility");

    SetupOutputVariable(state,
                        "Facility All Zones Ventilation At Target Voz Time",
                        Constant::Units::hr,
                        state.dataSysRpts->AllZonesTimeAtVozDyn,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Facility");

    SetupOutputVariable(state,
                        "Facility Any Zone Ventilation Above Target Voz Time",
                        Constant::Units::hr,
                        state.dataSysRpts->AnyZoneTimeAboveVozDyn,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Facility");

    SetupOutputVariable(state,
                        "Facility Any Zone Ventilation When Unoccupied Time",
                        Constant::Units::hr,
                        state.dataSysRpts->AnyZoneTimeVentUnocc,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Facility");
}

void CreateEnergyReportStructure(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher/Linda Lawrie
    //       DATE WRITTEN   June 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Creates the Energy Reporting Structure.  This routine is only called once --
    // so string compares have been left in.

    // METHODOLOGY EMPLOYED:
    // Once all compsets/nodes/connections have been established find all components
    // subcomponents, etc.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int AirLoopNum;
    int BranchNum;
    int CompNum;
    int SubCompNum;
    int SubSubCompNum;
    int VarNum;
    int VarNum1;
    int CtrlZoneNum;
    bool ErrorsFound;
    bool ModeFlagOn;
    int NumInlets;
    int NumOutlets;
    int PlantLoopNum;

    // Dimension GetChildrenData arrays
    EPVector<DataLoopNode::ConnectionObjectType> SubCompTypes;
    Array1D_string SubCompNames;
    Array1D_string InletNodeNames;
    Array1D_int InletNodeNumbers;
    Array1D<NodeInputManager::CompFluidStream> InletFluidStreams;
    Array1D_string OutletNodeNames;
    Array1D_int OutletNodeNumbers;
    Array1D<NodeInputManager::CompFluidStream> OutletFluidStreams;
    int NumChildren;
    int NumGrandChildren;
    bool IsParent;

    // Dimension GetMeteredVariables arrays
    Array1D<OutputProcessor::MeteredVar> meteredVars;
    int NumVariables;
    int NumLeft; // Counter for deeper components

    // some variables for setting up the plant data structures

    state.dataSysRpts->VentReportStructureCreated = true;
    for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                DataLoopNode::ConnectionObjectType TypeOfComp = static_cast<DataLoopNode::ConnectionObjectType>(
                    EnergyPlus::getEnumValue(BranchNodeConnections::ConnectionObjectTypeNamesUC,
                                             state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf));
                std::string &NameOfComp = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name;
                // Get complete list of components for complex branches
                if (BranchNodeConnections::IsParentObject(state, TypeOfComp, NameOfComp)) {

                    state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Parent = true;
                    NumChildren = BranchNodeConnections::GetNumChildren(state, TypeOfComp, NameOfComp);

                    SubCompTypes.allocate(NumChildren);
                    SubCompNames.allocate(NumChildren);
                    InletNodeNames.allocate(NumChildren);
                    InletNodeNumbers.allocate(NumChildren);
                    OutletNodeNames.allocate(NumChildren);
                    OutletNodeNumbers.allocate(NumChildren);
                    state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp.allocate(NumChildren);

                    BranchNodeConnections::GetChildrenData(state,
                                                           TypeOfComp,
                                                           NameOfComp,
                                                           NumChildren,
                                                           SubCompTypes,
                                                           SubCompNames,
                                                           InletNodeNames,
                                                           InletNodeNumbers,
                                                           OutletNodeNames,
                                                           OutletNodeNumbers,
                                                           ErrorsFound);

                    for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                        {
                            auto &thisSubComponent =
                                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum);
                            thisSubComponent.TypeOf = BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubCompNum))];
                            thisSubComponent.Name = SubCompNames(SubCompNum);
                            thisSubComponent.NodeNameIn = InletNodeNames(SubCompNum);
                            thisSubComponent.NodeNameOut = OutletNodeNames(SubCompNum);
                            thisSubComponent.NodeNumIn = InletNodeNumbers(SubCompNum);
                            thisSubComponent.NodeNumOut = OutletNodeNumbers(SubCompNum);
                        }
                    }

                    SubCompTypes.deallocate();
                    SubCompNames.deallocate();
                    InletNodeNames.deallocate();
                    InletNodeNumbers.deallocate();
                    OutletNodeNames.deallocate();
                    OutletNodeNumbers.deallocate();

                } else {
                    NumChildren = 0;
                    state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Parent = false;
                }
                state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).NumSubComps = NumChildren;

                // check for 'grandchildren'
                for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                    DataLoopNode::ConnectionObjectType TypeOfSubComp = static_cast<DataLoopNode::ConnectionObjectType>(EnergyPlus::getEnumValue(
                        BranchNodeConnections::ConnectionObjectTypeNamesUC,
                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).TypeOf));
                    std::string &NameOfSubComp =
                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).Name;
                    if (BranchNodeConnections::IsParentObject(state, TypeOfSubComp, NameOfSubComp)) {
                        NumGrandChildren = BranchNodeConnections::GetNumChildren(state, TypeOfSubComp, NameOfSubComp);
                        SubCompTypes.allocate(NumGrandChildren);
                        SubCompNames.allocate(NumGrandChildren);
                        InletNodeNames.allocate(NumGrandChildren);
                        InletNodeNumbers.allocate(NumGrandChildren);
                        OutletNodeNames.allocate(NumGrandChildren);
                        OutletNodeNumbers.allocate(NumGrandChildren);
                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                            .Branch(BranchNum)
                            .Comp(CompNum)
                            .SubComp(SubCompNum)
                            .SubSubComp.allocate(NumGrandChildren);

                        BranchNodeConnections::GetChildrenData(state,
                                                               TypeOfSubComp,
                                                               NameOfSubComp,
                                                               NumGrandChildren,
                                                               SubCompTypes,
                                                               SubCompNames,
                                                               InletNodeNames,
                                                               InletNodeNumbers,
                                                               OutletNodeNames,
                                                               OutletNodeNumbers,
                                                               ErrorsFound);

                        for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                            {
                                auto &thisSubSubComponent = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                .Branch(BranchNum)
                                                                .Comp(CompNum)
                                                                .SubComp(SubCompNum)
                                                                .SubSubComp(SubSubCompNum);
                                thisSubSubComponent.TypeOf = static_cast<std::string>(
                                    BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubSubCompNum))]);
                                thisSubSubComponent.Name = SubCompNames(SubSubCompNum);
                                thisSubSubComponent.NodeNameIn = InletNodeNames(SubSubCompNum);
                                thisSubSubComponent.NodeNameOut = OutletNodeNames(SubSubCompNum);
                                thisSubSubComponent.NodeNumIn = InletNodeNumbers(SubSubCompNum);
                                thisSubSubComponent.NodeNumOut = OutletNodeNumbers(SubSubCompNum);
                                NumLeft = BranchNodeConnections::GetNumChildren(state, SubCompTypes(SubSubCompNum), SubCompNames(SubSubCompNum));
                                if (NumLeft > 0) {
                                    ShowSevereError(
                                        state,
                                        format("Hanging Children for component={}:{}", thisSubSubComponent.TypeOf, SubCompNames(SubSubCompNum)));
                                }
                            }
                        }

                        SubCompTypes.deallocate();
                        SubCompNames.deallocate();
                        InletNodeNames.deallocate();
                        InletNodeNumbers.deallocate();
                        OutletNodeNames.deallocate();
                        OutletNodeNumbers.deallocate();
                    } else {
                        NumGrandChildren = 0;
                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).Parent = false;
                    }

                    state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).NumSubSubComps =
                        NumGrandChildren;
                }
            }
        }
    }

    for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                // Get complete list of components for complex branches
                {
                    auto &thisComp = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum);
                    std::string &TypeOfComp = thisComp.TypeOf;
                    std::string &NameOfComp = thisComp.Name;
                    NumVariables = GetNumMeteredVariables(state, TypeOfComp, NameOfComp);
                    if (NumVariables > 0) {
                        meteredVars.allocate(NumVariables);
                        thisComp.MeteredVar.allocate(NumVariables);

                        thisComp.NumMeteredVars = NumVariables;
                        GetMeteredVariables(state, NameOfComp, meteredVars);
                        ModeFlagOn = true;
                        for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                            {
                                thisComp.MeteredVar(VarNum) = meteredVars(VarNum); // Copy
                                auto &thisVar = thisComp.MeteredVar(VarNum);
                                if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisComp.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisComp.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (ModeFlagOn) {
                                    thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                                }
                            }
                        }

                        meteredVars.deallocate();
                    }
                    for (SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                        // Get complete list of components for complex branches
                        std::string &TypeOfSubComp = thisComp.SubComp(SubCompNum).TypeOf;
                        std::string &NameOfSubComp = thisComp.SubComp(SubCompNum).Name;
                        NumVariables = GetNumMeteredVariables(state, TypeOfSubComp, NameOfSubComp);
                        if (NumVariables > 0) {
                            meteredVars.allocate(NumVariables);
                            thisComp.SubComp(SubCompNum).MeteredVar.allocate(NumVariables);

                            GetMeteredVariables(state, NameOfSubComp, meteredVars);
                            ModeFlagOn = true;
                            for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                {
                                    thisComp.SubComp(SubCompNum).MeteredVar(VarNum) = meteredVars(VarNum);
                                    auto &thisVar = thisComp.SubComp(SubCompNum).MeteredVar(VarNum);
                                    if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (ModeFlagOn) {
                                        thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                                    }
                                }
                            }

                            meteredVars.deallocate();
                        }

                        thisComp.SubComp(SubCompNum).NumMeteredVars = NumVariables;

                        for (SubSubCompNum = 1; SubSubCompNum <= thisComp.SubComp(SubCompNum).NumSubSubComps; ++SubSubCompNum) {
                            // Get complete list of components for complex branches
                            std::string &TypeOfSubSubComp = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).TypeOf;
                            std::string &NameOfSubSubComp = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).Name;
                            NumVariables = GetNumMeteredVariables(state, TypeOfSubSubComp, NameOfSubSubComp);
                            if (NumVariables > 0) {
                                meteredVars.allocate(NumVariables);
                                thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar.allocate(NumVariables);

                                GetMeteredVariables(state, NameOfSubSubComp, meteredVars);
                                ModeFlagOn = true;
                                for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                    {
                                        thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum) = meteredVars(VarNum);
                                        auto &thisVar = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum);
                                        if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum1).heatOrCool =
                                                    Constant::HeatOrCool::HeatingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum1).heatOrCool =
                                                    Constant::HeatOrCool::CoolingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (ModeFlagOn) {
                                            thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                                        }
                                    }
                                }

                                meteredVars.deallocate();
                            }
                            thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).NumMeteredVars = NumVariables;
                        }
                    }
                }
            }
        }
    }

    // Allocate the system serving zone equipment component arrays
    for (CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
            continue;
        }
        // Set index of air loop serving zone
        for (CompNum = 1; CompNum <= state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).NumOfEquipTypes; ++CompNum) {
            std::string &TypeOfComp = state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).EquipTypeName(CompNum);
            std::string &NameOfComp = state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).EquipName(CompNum);
            DataLoopNode::ConnectionObjectType TypeOfCompNum = static_cast<DataLoopNode::ConnectionObjectType>(
                EnergyPlus::getEnumValue(BranchNodeConnections::ConnectionObjectTypeNamesUC, TypeOfComp));
            BranchNodeConnections::GetComponentData(state,
                                                    TypeOfCompNum,
                                                    NameOfComp,
                                                    IsParent,
                                                    NumInlets,
                                                    InletNodeNames,
                                                    InletNodeNumbers,
                                                    InletFluidStreams,
                                                    NumOutlets,
                                                    OutletNodeNames,
                                                    OutletNodeNumbers,
                                                    OutletFluidStreams);
            {
                auto &thisEquipData = state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).EquipData(CompNum);
                thisEquipData.TypeOf = TypeOfComp;
                thisEquipData.Name = NameOfComp;
                thisEquipData.OutletNodeNums.allocate(NumOutlets);
                thisEquipData.NumOutlets = NumOutlets;
                thisEquipData.OutletNodeNums = OutletNodeNumbers;
                thisEquipData.InletNodeNums.allocate(NumInlets);
                thisEquipData.NumInlets = NumInlets;
                thisEquipData.InletNodeNums = InletNodeNumbers;
                thisEquipData.Parent = IsParent;
                NumVariables = GetNumMeteredVariables(state, TypeOfComp, NameOfComp);
                thisEquipData.NumMeteredVars = NumVariables;
                if (NumVariables > 0) {
                    InletNodeNames.deallocate();
                    InletNodeNumbers.deallocate();
                    InletFluidStreams.deallocate();
                    OutletNodeNames.deallocate();
                    OutletNodeNumbers.deallocate();
                    OutletFluidStreams.deallocate();

                    meteredVars.allocate(NumVariables);
                    thisEquipData.MeteredVar.allocate(NumVariables);

                    GetMeteredVariables(state, NameOfComp, meteredVars);

                    ModeFlagOn = true;
                    for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                        {
                            thisEquipData.MeteredVar(VarNum) = meteredVars(VarNum);
                            auto &thisVar = thisEquipData.MeteredVar(VarNum);
                            if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisEquipData.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisEquipData.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (ModeFlagOn) {
                                thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                            }
                        }
                    }

                    meteredVars.deallocate();
                }

                if (BranchNodeConnections::IsParentObject(state, TypeOfCompNum, NameOfComp)) {
                    NumChildren = BranchNodeConnections::GetNumChildren(state, TypeOfCompNum, NameOfComp);
                    thisEquipData.NumSubEquip = NumChildren;

                    SubCompTypes.allocate(NumChildren);
                    SubCompNames.allocate(NumChildren);
                    InletNodeNames.allocate(NumChildren);
                    InletNodeNumbers.allocate(NumChildren);
                    OutletNodeNames.allocate(NumChildren);
                    OutletNodeNumbers.allocate(NumChildren);
                    thisEquipData.SubEquipData.allocate(NumChildren);

                    BranchNodeConnections::GetChildrenData(state,
                                                           TypeOfCompNum,
                                                           NameOfComp,
                                                           NumChildren,
                                                           SubCompTypes,
                                                           SubCompNames,
                                                           InletNodeNames,
                                                           InletNodeNumbers,
                                                           OutletNodeNames,
                                                           OutletNodeNumbers,
                                                           ErrorsFound);

                    for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                        thisEquipData.SubEquipData(SubCompNum).TypeOf =
                            BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubCompNum))];
                        thisEquipData.SubEquipData(SubCompNum).Name = SubCompNames(SubCompNum);
                        thisEquipData.SubEquipData(SubCompNum).OutletNodeNum = OutletNodeNumbers(SubCompNum);
                        thisEquipData.SubEquipData(SubCompNum).InletNodeNum = InletNodeNumbers(SubCompNum);
                    }

                    SubCompTypes.deallocate();
                    SubCompNames.deallocate();
                    InletNodeNames.deallocate();
                    InletNodeNumbers.deallocate();
                    OutletNodeNames.deallocate();
                    OutletNodeNumbers.deallocate();
                } else {
                    NumChildren = 0;
                }

                for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                    std::string &TypeOfSubComp = thisEquipData.SubEquipData(SubCompNum).TypeOf;
                    std::string &NameOfSubComp = thisEquipData.SubEquipData(SubCompNum).Name;
                    DataLoopNode::ConnectionObjectType TypeOfSubCompNum = static_cast<DataLoopNode::ConnectionObjectType>(
                        EnergyPlus::getEnumValue(BranchNodeConnections::ConnectionObjectTypeNamesUC, TypeOfSubComp));
                    if (BranchNodeConnections::IsParentObject(state, TypeOfSubCompNum, NameOfSubComp)) {
                        NumGrandChildren = BranchNodeConnections::GetNumChildren(state, TypeOfSubCompNum, NameOfSubComp);
                        thisEquipData.SubEquipData(SubCompNum).NumSubSubEquip = NumGrandChildren;
                        SubCompTypes.allocate(NumGrandChildren);
                        SubCompNames.allocate(NumGrandChildren);
                        InletNodeNames.allocate(NumGrandChildren);
                        InletNodeNumbers.allocate(NumGrandChildren);
                        OutletNodeNames.allocate(NumGrandChildren);
                        OutletNodeNumbers.allocate(NumGrandChildren);
                        thisEquipData.SubEquipData(SubCompNum).SubSubEquipData.allocate(NumGrandChildren);
                        // Sankar added the array number for EquipData
                        BranchNodeConnections::GetChildrenData(state,
                                                               TypeOfSubCompNum,
                                                               NameOfSubComp,
                                                               NumGrandChildren,
                                                               SubCompTypes,
                                                               SubCompNames,
                                                               InletNodeNames,
                                                               InletNodeNumbers,
                                                               OutletNodeNames,
                                                               OutletNodeNumbers,
                                                               ErrorsFound);

                        for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                            thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).TypeOf =
                                BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubSubCompNum))];
                            thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).Name = SubCompNames(SubSubCompNum);
                            thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).OutletNodeNum = OutletNodeNumbers(SubSubCompNum);
                            thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).InletNodeNum = InletNodeNumbers(SubSubCompNum);
                        }
                        SubCompTypes.deallocate();
                        SubCompNames.deallocate();
                        InletNodeNames.deallocate();
                        InletNodeNumbers.deallocate();
                        OutletNodeNames.deallocate();
                        OutletNodeNumbers.deallocate();
                    } else {
                        NumGrandChildren = 0;
                    }
                } // for (SubCompNum)
            }
        } // for (CompNum)
    } // for (CtrlZoneNum)

    for (CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
            continue;
        }
        for (CompNum = 1; CompNum <= state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).NumOfEquipTypes; ++CompNum) {
            for (SubCompNum = 1; SubCompNum <= state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).EquipData(CompNum).NumSubEquip; ++SubCompNum) {
                auto &thisSubEquipData = state.dataZoneEquip->ZoneEquipList(CtrlZoneNum).EquipData(CompNum).SubEquipData(SubCompNum);
                std::string &TypeOfSubComp = thisSubEquipData.TypeOf;
                std::string &NameOfSubComp = thisSubEquipData.Name;

                NumVariables = GetNumMeteredVariables(state, TypeOfSubComp, NameOfSubComp);
                thisSubEquipData.NumMeteredVars = NumVariables; // Sankar added this line
                if (NumVariables > 0) {
                    meteredVars.allocate(NumVariables);
                    thisSubEquipData.MeteredVar.allocate(NumVariables);

                    GetMeteredVariables(state, NameOfSubComp, meteredVars);

                    ModeFlagOn = true;
                    for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                        thisSubEquipData.MeteredVar(VarNum) = meteredVars(VarNum);
                        auto &thisVar = thisSubEquipData.MeteredVar(VarNum);
                        if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                thisSubEquipData.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                            }
                            ModeFlagOn = false;
                        } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                thisSubEquipData.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                            }
                            ModeFlagOn = false;
                        } else if (ModeFlagOn) {
                            thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                        }
                    }

                    meteredVars.deallocate();
                } // if (NumVariables > 0)

                for (SubSubCompNum = 1; SubSubCompNum <= thisSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                    std::string &TypeOfSubSubComp = thisSubEquipData.SubSubEquipData(SubSubCompNum).TypeOf;
                    std::string &NameOfSubSubComp = thisSubEquipData.SubSubEquipData(SubSubCompNum).Name;

                    NumVariables = GetNumMeteredVariables(state, TypeOfSubSubComp, NameOfSubSubComp);
                    thisSubEquipData.SubSubEquipData(SubSubCompNum).NumMeteredVars = NumVariables; // Sankar added this line
                    if (NumVariables > 0) {
                        meteredVars.allocate(NumVariables);
                        thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar.allocate(NumVariables);

                        GetMeteredVariables(state, NameOfSubSubComp, meteredVars);
                        ModeFlagOn = true;
                        for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                            thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum) = meteredVars(VarNum);
                            auto &thisVar = thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum);
                            if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum1).heatOrCool =
                                        Constant::HeatOrCool::HeatingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum1).heatOrCool =
                                        Constant::HeatOrCool::CoolingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (ModeFlagOn) {
                                thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                            }
                        }

                        meteredVars.deallocate();
                    } // if (NumVariables > 0)
                } // for (SubSubCompNum)
            } // for (SubCompNum)
        } // for (CompNum)
    } // for (CtrlZoneNum)

    //***Plant Loops

    // previously, four separate huge DO loops all looking very very similar were used here
    // each individual block would operate on a single type of loop-side (plant demand, cond supply, etc.)
    // now, a bigger DO loop is applied iterating over all loops
    // a pointer (ThisReportData) is then directed to a particular item in the appropriate array
    // by operating on the pointer directly, we are actually operating on the item in the TARGET array item
    // in making this change, over 700 lines of code were dropped down to a single block

    for (PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops + state.dataHVACGlobal->NumCondLoops; ++PlantLoopNum) {
        for (DataPlant::LoopSideLocation LoopSideNum : DataPlant::LoopSideKeys) {

            // Report selection
            ReportLoopData *select_ThisReportData(nullptr);

            assert(LoopSideNum == LoopSideLocation::Demand || LoopSideNum == LoopSideLocation::Supply);
            if (PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops) {
                select_ThisReportData = &state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideNum)](PlantLoopNum);
            } else { // CondLoop
                select_ThisReportData =
                    &state.dataPlnt->VentRepCond[static_cast<int>(LoopSideNum)](PlantLoopNum - state.dataHVACGlobal->NumPlantLoops);
            }

            // Object Data
            ReportLoopData &ThisReportData(*select_ThisReportData);

            for (BranchNum = 1; BranchNum <= ThisReportData.TotalBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= ThisReportData.Branch(BranchNum).TotalComponents; ++CompNum) {
                    {
                        auto &thisComp = ThisReportData.Branch(BranchNum).Comp(CompNum);
                        std::string &TypeOfComp = thisComp.TypeOf;
                        std::string &NameOfComp = thisComp.Name;
                        DataLoopNode::ConnectionObjectType TypeOfCompNum = static_cast<DataLoopNode::ConnectionObjectType>(
                            EnergyPlus::getEnumValue(BranchNodeConnections::ConnectionObjectTypeNamesUC, TypeOfComp));
                        // Get complete list of components for complex branches
                        if (BranchNodeConnections::IsParentObject(state, TypeOfCompNum, NameOfComp)) {

                            NumChildren = BranchNodeConnections::GetNumChildren(state, TypeOfCompNum, NameOfComp);

                            SubCompTypes.allocate(NumChildren);
                            SubCompNames.allocate(NumChildren);
                            InletNodeNames.allocate(NumChildren);
                            InletNodeNumbers.allocate(NumChildren);
                            OutletNodeNames.allocate(NumChildren);
                            OutletNodeNumbers.allocate(NumChildren);
                            thisComp.SubComp.allocate(NumChildren);

                            BranchNodeConnections::GetChildrenData(state,
                                                                   TypeOfCompNum,
                                                                   NameOfComp,
                                                                   NumChildren,
                                                                   SubCompTypes,
                                                                   SubCompNames,
                                                                   InletNodeNames,
                                                                   InletNodeNumbers,
                                                                   OutletNodeNames,
                                                                   OutletNodeNumbers,
                                                                   ErrorsFound);

                            for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                                thisComp.SubComp(SubCompNum).TypeOf =
                                    BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubCompNum))];
                                thisComp.SubComp(SubCompNum).Name = SubCompNames(SubCompNum);
                                thisComp.SubComp(SubCompNum).NodeNameIn = InletNodeNames(SubCompNum);
                                thisComp.SubComp(SubCompNum).NodeNameOut = OutletNodeNames(SubCompNum);
                                thisComp.SubComp(SubCompNum).NodeNumIn = InletNodeNumbers(SubCompNum);
                                thisComp.SubComp(SubCompNum).NodeNumOut = OutletNodeNumbers(SubCompNum);
                            }

                            SubCompTypes.deallocate();
                            SubCompNames.deallocate();
                            InletNodeNames.deallocate();
                            InletNodeNumbers.deallocate();
                            OutletNodeNames.deallocate();
                            OutletNodeNumbers.deallocate();

                        } else {
                            NumChildren = 0;
                        }
                        thisComp.NumSubComps = NumChildren;

                        // check for 'grandchildren'
                        for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                            std::string &TypeOfSubComp = thisComp.SubComp(SubCompNum).TypeOf;
                            std::string NameOfSubComp = thisComp.SubComp(SubCompNum).Name;
                            DataLoopNode::ConnectionObjectType TypeOfSubCompNum = static_cast<DataLoopNode::ConnectionObjectType>(
                                EnergyPlus::getEnumValue(BranchNodeConnections::ConnectionObjectTypeNamesUC, TypeOfSubComp));
                            if (BranchNodeConnections::IsParentObject(state, TypeOfSubCompNum, NameOfSubComp)) {
                                NumGrandChildren = BranchNodeConnections::GetNumChildren(state, TypeOfSubCompNum, NameOfSubComp);
                                SubCompTypes.allocate(NumGrandChildren);
                                SubCompNames.allocate(NumGrandChildren);
                                InletNodeNames.allocate(NumGrandChildren);
                                InletNodeNumbers.allocate(NumGrandChildren);
                                OutletNodeNames.allocate(NumGrandChildren);
                                OutletNodeNumbers.allocate(NumGrandChildren);
                                thisComp.SubComp(SubCompNum).SubSubComp.allocate(NumGrandChildren);

                                BranchNodeConnections::GetChildrenData(state,
                                                                       TypeOfSubCompNum,
                                                                       NameOfSubComp,
                                                                       NumGrandChildren,
                                                                       SubCompTypes,
                                                                       SubCompNames,
                                                                       InletNodeNames,
                                                                       InletNodeNumbers,
                                                                       OutletNodeNames,
                                                                       OutletNodeNumbers,
                                                                       ErrorsFound);

                                for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                    {
                                        auto &thisSubSubComp = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum);
                                        thisSubSubComp.TypeOf =
                                            BranchNodeConnections::ConnectionObjectTypeNamesUC[static_cast<int>(SubCompTypes(SubSubCompNum))];
                                        thisSubSubComp.Name = SubCompNames(SubSubCompNum);
                                        thisSubSubComp.NodeNameIn = InletNodeNames(SubSubCompNum);
                                        thisSubSubComp.NodeNameOut = OutletNodeNames(SubSubCompNum);
                                        thisSubSubComp.NodeNumIn = InletNodeNumbers(SubSubCompNum);
                                        thisSubSubComp.NodeNumOut = OutletNodeNumbers(SubSubCompNum);
                                    }
                                }

                                SubCompTypes.deallocate();
                                SubCompNames.deallocate();
                                InletNodeNames.deallocate();
                                InletNodeNumbers.deallocate();
                                OutletNodeNames.deallocate();
                                OutletNodeNumbers.deallocate();
                            } else {
                                NumGrandChildren = 0;
                                thisComp.SubComp(SubCompNum).Parent = false;
                            }

                            thisComp.SubComp(SubCompNum).NumSubSubComps = NumGrandChildren;
                        }
                    }
                }
            }
        }
    }

    for (PlantLoopNum = 1; PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops + state.dataHVACGlobal->NumCondLoops; ++PlantLoopNum) {

        for (DataPlant::LoopSideLocation LoopSideNum : DataPlant::LoopSideKeys) {

            // Report selection
            ReportLoopData *select_ThisReportData(nullptr);

            assert(LoopSideNum == LoopSideLocation::Demand || LoopSideNum == LoopSideLocation::Supply);
            if (PlantLoopNum <= state.dataHVACGlobal->NumPlantLoops) {
                select_ThisReportData = &state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideNum)](PlantLoopNum);
            } else { // CondLoop
                select_ThisReportData =
                    &state.dataPlnt->VentRepCond[static_cast<int>(LoopSideNum)](PlantLoopNum - state.dataHVACGlobal->NumPlantLoops);
            }

            // Object Data
            ReportLoopData &ThisReportData(*select_ThisReportData);

            for (BranchNum = 1; BranchNum <= ThisReportData.TotalBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= ThisReportData.Branch(BranchNum).TotalComponents; ++CompNum) {
                    // Get complete list of components for complex branches
                    auto &thisComp = ThisReportData.Branch(BranchNum).Comp(CompNum);
                    std::string &TypeOfComp = thisComp.TypeOf;
                    std::string &NameOfComp = thisComp.Name;
                    NumVariables = GetNumMeteredVariables(state, TypeOfComp, NameOfComp);
                    if (NumVariables > 0) {
                        meteredVars.allocate(NumVariables);
                        thisComp.MeteredVar.allocate(NumVariables);

                        thisComp.NumMeteredVars = NumVariables;
                        GetMeteredVariables(state, NameOfComp, meteredVars);
                        ModeFlagOn = true;

                        for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                            thisComp.MeteredVar(VarNum) = meteredVars(VarNum);
                            auto &thisVar = thisComp.MeteredVar(VarNum);
                            if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisComp.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                    thisComp.MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                                }
                                ModeFlagOn = false;
                            } else if (ModeFlagOn) {
                                thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                            }
                        }

                        meteredVars.deallocate();
                    } // if (NumVariables > 0)

                    for (SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                        // Get complete list of components for complex branches
                        std::string &TypeOfSubComp = thisComp.SubComp(SubCompNum).TypeOf;
                        std::string &NameOfSubComp = thisComp.SubComp(SubCompNum).Name;
                        NumVariables = GetNumMeteredVariables(state, TypeOfSubComp, NameOfSubComp);
                        if (NumVariables > 0) {
                            meteredVars.allocate(NumVariables);
                            thisComp.SubComp(SubCompNum).MeteredVar.allocate(NumVariables);

                            GetMeteredVariables(state, NameOfSubComp, meteredVars);
                            ModeFlagOn = true;

                            for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                thisComp.SubComp(SubCompNum).MeteredVar(VarNum) = meteredVars(VarNum);
                                auto &thisVar = thisComp.SubComp(SubCompNum).MeteredVar(VarNum);
                                if (thisVar.endUseCat == OutputProcessor::EndUseCat::HeatingCoils && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::HeatingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (thisVar.endUseCat == OutputProcessor::EndUseCat::CoolingCoils && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).heatOrCool = Constant::HeatOrCool::CoolingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (ModeFlagOn) {
                                    thisVar.heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
                                }
                            }

                            meteredVars.deallocate();
                        } // if (NumVariables > 0)

                        thisComp.SubComp(SubCompNum).NumMeteredVars = NumVariables;
                    } // for (SubCompNum)
                } // for (CompNum)
            } // for (BranchNum)
        } // for (LoopSide)
    } // for (PlantLoopNum)
} // CreateEnergyReportStructure()

// End Initialization Section of the Module
//******************************************************************************

// Beginning of Reporting subroutines for the SimAir Module
// *****************************************************************************

void ReportSystemEnergyUse(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   November 2005

    // PURPOSE OF THIS SUBROUTINE:
    // calculate and report system loads and energy

    // METHODOLOGY EMPLOYED:
    // Accumulate meter data to appropriate report variables

    int Idx;          // loop counter
    int nodes;        // loop counter
    int BranchNum;    // counter for zone air distribution inlets
    int EquipListNum; // counter for zone air distribution inlets
    int VarNum;       // counter for zone air distribution inlets
    int CompNum;
    int SubCompNum;
    int SubSubCompNum;
    Constant::HeatOrCool CompMode;
    int InletNodeNum;
    int OutletNodeNum;
    int ADUNum;
    int ADUCoolNum;
    int ADUHeatNum;
    int AirDistCoolInletNodeNum;
    int AirDistHeatInletNodeNum;
    Constant::eResource EnergyType;
    Real64 CompEnergyUse;
    Real64 ZoneLoad;
    Real64 CompLoad;
    Real64 ADUCoolFlowrate;
    Real64 ADUHeatFlowrate;
    bool CompLoadFlag;

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    if (!state.dataSysRpts->AirLoopLoadsReportEnabled) {
        return;
    }

    for (int airLoopNum = 1; airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++airLoopNum) {
        auto &thisSysLoadRepVars = state.dataSysRpts->SysLoadRepVars(airLoopNum);
        // SYSTEM LOADS REPORT
        thisSysLoadRepVars.TotHTNG = 0.0;
        thisSysLoadRepVars.TotCLNG = 0.0;

        // SYSTEM ENERGY USE REPORT
        thisSysLoadRepVars.TotElec = 0.0;
        thisSysLoadRepVars.TotNaturalGas = 0.0;
        thisSysLoadRepVars.TotPropane = 0.0;
        thisSysLoadRepVars.TotSteam = 0.0;
        thisSysLoadRepVars.TotH2OCOLD = 0.0;
        thisSysLoadRepVars.TotH2OHOT = 0.0;

        // SYSTEM COMPONENT LOADS REPORT
        thisSysLoadRepVars.FANCompHTNG = 0.0;
        thisSysLoadRepVars.CCCompCLNG = 0.0;
        thisSysLoadRepVars.HCCompHTNG = 0.0;
        thisSysLoadRepVars.HeatExHTNG = 0.0;
        thisSysLoadRepVars.HeatExCLNG = 0.0;
        thisSysLoadRepVars.SolarCollectHeating = 0.0;
        thisSysLoadRepVars.SolarCollectCooling = 0.0;
        thisSysLoadRepVars.UserDefinedTerminalHeating = 0.0;
        thisSysLoadRepVars.UserDefinedTerminalCooling = 0.0;
        thisSysLoadRepVars.HumidHTNG = 0.0;
        thisSysLoadRepVars.EvapCLNG = 0.0;
        thisSysLoadRepVars.DesDehumidCLNG = 0.0;
        thisSysLoadRepVars.DomesticH2O = 0.0;

        // SYSTEM COMPONENT ENERGY REPORT
        thisSysLoadRepVars.FANCompElec = 0.0;
        thisSysLoadRepVars.HCCompH2OHOT = 0.0;
        thisSysLoadRepVars.CCCompH2OCOLD = 0.0;
        thisSysLoadRepVars.HCCompElec = 0.0;
        thisSysLoadRepVars.CCCompElec = 0.0;
        thisSysLoadRepVars.HCCompElecRes = 0.0;
        thisSysLoadRepVars.HCCompNaturalGas = 0.0;
        thisSysLoadRepVars.HCCompPropane = 0.0;
        thisSysLoadRepVars.HCCompSteam = 0.0;
        thisSysLoadRepVars.HumidElec = 0.0;
        thisSysLoadRepVars.HumidNaturalGas = 0.0;
        thisSysLoadRepVars.HumidPropane = 0.0;
        thisSysLoadRepVars.DesDehumidElec = 0.0;
        thisSysLoadRepVars.EvapElec = 0.0;
    }

    auto &Node = state.dataLoopNodes->Node;

    for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
        auto const &pas = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum);
        for (BranchNum = 1; BranchNum <= pas.NumBranches; ++BranchNum) {
            auto const &pasBranch = pas.Branch(BranchNum);
            if (Node(pasBranch.NodeNumOut).MassFlowRate <= 0.0) {
                continue;
            }
            for (CompNum = 1; CompNum <= pasBranch.TotalComponents; ++CompNum) {
                auto const &pasBranchComp = pasBranch.Comp(CompNum);
                InletNodeNum = pasBranchComp.NodeNumIn;
                OutletNodeNum = pasBranchComp.NodeNumOut;
                if (InletNodeNum <= 0 || OutletNodeNum <= 0) {
                    continue;
                }
                CompLoad = Node(OutletNodeNum).MassFlowRate * (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                               Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                CompLoad *= TimeStepSysSec;
                CompEnergyUse = 0.0;
                EnergyType = Constant::eResource::Invalid;
                CompLoadFlag = true;
                CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                CompLoadFlag = false;
                for (VarNum = 1; VarNum <= pasBranchComp.NumMeteredVars; ++VarNum) {
                    auto const &pasBranchCompMeter = pasBranchComp.MeteredVar(VarNum);
                    CompMode = pasBranchCompMeter.heatOrCool;
                    CompEnergyUse = pasBranchCompMeter.curMeterReading;
                    EnergyType = pasBranchCompMeter.resource;
                    CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                }

                for (SubCompNum = 1; SubCompNum <= pasBranchComp.NumSubComps; ++SubCompNum) {
                    auto const &pasBranchSubComp = pasBranchComp.SubComp(SubCompNum);
                    InletNodeNum = pasBranchSubComp.NodeNumIn;
                    OutletNodeNum = pasBranchSubComp.NodeNumOut;
                    if (InletNodeNum <= 0 || OutletNodeNum <= 0) {
                        continue;
                    }
                    CompLoad = Node(OutletNodeNum).MassFlowRate * (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                   Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                    CompLoad *= TimeStepSysSec;
                    CompEnergyUse = 0.0;
                    EnergyType = Constant::eResource::Invalid;
                    CompLoadFlag = true;
                    CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    CompLoadFlag = false;
                    for (VarNum = 1; VarNum <= pasBranchSubComp.NumMeteredVars; ++VarNum) {
                        auto const &pasBranchSubCompMeter = pasBranchSubComp.MeteredVar(VarNum);
                        CompMode = pasBranchSubCompMeter.heatOrCool;
                        CompEnergyUse = pasBranchSubCompMeter.curMeterReading;
                        EnergyType = pasBranchSubCompMeter.resource;
                        CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    }

                    for (SubSubCompNum = 1; SubSubCompNum <= pasBranchSubComp.NumSubSubComps; ++SubSubCompNum) {
                        auto const &pasBranchSubSubComp = pasBranchSubComp.SubSubComp(SubSubCompNum);
                        InletNodeNum = pasBranchSubSubComp.NodeNumIn;
                        OutletNodeNum = pasBranchSubSubComp.NodeNumOut;
                        if (InletNodeNum <= 0 || OutletNodeNum <= 0) {
                            continue;
                        }
                        CompLoad =
                            Node(OutletNodeNum).MassFlowRate * (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                        CompLoad *= TimeStepSysSec;
                        CompEnergyUse = 0.0;
                        EnergyType = Constant::eResource::Invalid;
                        CompLoadFlag = true;
                        CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchSubSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        CompLoadFlag = false;
                        for (VarNum = 1; VarNum <= pasBranchSubSubComp.NumMeteredVars; ++VarNum) {
                            auto const &pasBranchSubSubCompMeter = pasBranchSubSubComp.MeteredVar(VarNum);
                            CompMode = pasBranchSubSubCompMeter.heatOrCool;
                            CompEnergyUse = pasBranchSubSubCompMeter.curMeterReading;
                            EnergyType = pasBranchSubSubCompMeter.resource;
                            CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, pasBranchSubSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        }
                    } // for (SubSubCompNum)
                } // for (SubCompNum)
            } // for (CompNum)
        } // for (BranchNum)
    } // for (AirLoopNum)

    for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
        auto const &zecCtrlZone = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum);
        if (!zecCtrlZone.IsControlled) {
            continue;
        }

        ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;

        // if system operating in deadband reset zone load
        if (state.dataZoneEnergyDemand->DeadBandOrSetback(CtrlZoneNum)) {
            ZoneLoad = 0.0;
        }

        // loop over the zone supply air path inlet nodes
        for (int ZoneInNum = 1; ZoneInNum <= zecCtrlZone.NumInletNodes; ++ZoneInNum) {
            // retrieve air loop indexes
            int AirLoopNum = zecCtrlZone.InletNodeAirLoopNum(ZoneInNum);
            if (AirLoopNum == 0) {
                continue;
            }

            auto const &zecCtrlZoneCool = zecCtrlZone.AirDistUnitCool(ZoneInNum);
            auto const &zecCtrlZoneHeat = zecCtrlZone.AirDistUnitHeat(ZoneInNum);

            AirDistCoolInletNodeNum = max(zecCtrlZoneCool.InNode, 0);
            AirDistHeatInletNodeNum = max(zecCtrlZoneHeat.InNode, 0);

            // Set for cooling or heating path
            if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum == 0) {
                ADUCoolFlowrate = max(Node(zecCtrlZoneCool.InNode).MassFlowRate, 0.0);
            } else if (AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum == 0) {
                ADUHeatFlowrate = max(Node(zecCtrlZoneHeat.InNode).MassFlowRate, 0.0);
            } else {
                ADUCoolFlowrate = 0.0;
                ADUHeatFlowrate = 0.0;
            }

            EquipListNum = zecCtrlZone.EquipListIndex;
            auto const &zel = state.dataZoneEquip->ZoneEquipList(EquipListNum);

            for (Idx = 1; Idx <= 2; ++Idx) {
                if (Idx == 1) {
                    ADUCoolNum = max(zecCtrlZoneCool.AirDistUnitIndex, 0);
                    if (ADUCoolNum == 0) {
                        continue;
                    }
                    ADUNum = ADUCoolNum;
                } else { //(Idx =2)THEN
                    ADUHeatNum = max(zecCtrlZoneHeat.AirDistUnitIndex, 0);
                    if (ADUHeatNum == 0) {
                        continue;
                    }
                    ADUNum = ADUHeatNum;
                }

                auto const &zelEquipData = zel.EquipData(ADUNum);

                CompLoad = 0.0;
                if (zelEquipData.NumInlets > 0) {
                    for (nodes = 1; nodes <= zelEquipData.NumInlets; ++nodes) {
                        InletNodeNum = zelEquipData.InletNodeNums(Idx);
                        CompLoad +=
                            (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) * Node(InletNodeNum).MassFlowRate);
                    }
                    for (nodes = 1; nodes <= zelEquipData.NumOutlets; ++nodes) {
                        OutletNodeNum = zelEquipData.OutletNodeNums(Idx);
                        CompLoad -=
                            (Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat) * Node(OutletNodeNum).MassFlowRate);
                    }
                }
                CompLoad *= TimeStepSysSec;
                CompEnergyUse = 0.0;
                EnergyType = Constant::eResource::Invalid;
                CompLoadFlag = true;
                CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                CompLoadFlag = false;
                for (VarNum = 1; VarNum <= zelEquipData.NumMeteredVars; ++VarNum) {
                    CompEnergyUse = zelEquipData.MeteredVar(VarNum).curMeterReading;
                    EnergyType = zelEquipData.MeteredVar(VarNum).resource;
                    CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                }

                for (SubCompNum = 1; SubCompNum <= zelEquipData.NumSubEquip; ++SubCompNum) {
                    auto const &zelSubEquipData = zelEquipData.SubEquipData(SubCompNum);
                    InletNodeNum = zelSubEquipData.InletNodeNum;
                    OutletNodeNum = zelSubEquipData.OutletNodeNum;
                    if (InletNodeNum <= 0 || OutletNodeNum <= 0) {
                        continue;
                    }
                    CompLoad = Node(InletNodeNum).MassFlowRate * (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                  Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                    CompLoad *= TimeStepSysSec;
                    CompEnergyUse = 0.0;
                    EnergyType = Constant::eResource::Invalid;
                    CompLoadFlag = true;
                    CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    CompLoadFlag = false;
                    for (VarNum = 1; VarNum <= zelSubEquipData.NumMeteredVars; ++VarNum) {
                        CompEnergyUse = zelSubEquipData.MeteredVar(VarNum).curMeterReading;
                        EnergyType = zelSubEquipData.MeteredVar(VarNum).resource;
                        CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    }

                    for (SubSubCompNum = 1; SubSubCompNum <= zelSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                        auto const &zelSubSubEquipData = zelSubEquipData.SubSubEquipData(SubSubCompNum);
                        InletNodeNum = zelSubSubEquipData.InletNodeNum;
                        OutletNodeNum = zelSubSubEquipData.OutletNodeNum;
                        if (InletNodeNum <= 0 || OutletNodeNum <= 0) {
                            continue;
                        }
                        CompLoad =
                            Node(InletNodeNum).MassFlowRate * (Psychrometrics::PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                               Psychrometrics::PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                        CompLoad *= TimeStepSysSec;
                        CompEnergyUse = 0.0;
                        EnergyType = Constant::eResource::Invalid;
                        CompLoadFlag = true;
                        CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelSubSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        CompLoadFlag = false;
                        for (VarNum = 1; VarNum <= zelSubSubEquipData.NumMeteredVars; ++VarNum) {
                            CompEnergyUse = zelSubSubEquipData.MeteredVar(VarNum).curMeterReading;
                            EnergyType = zelSubSubEquipData.MeteredVar(VarNum).resource;
                            CalcSystemEnergyUse(state, CompLoadFlag, AirLoopNum, zelSubSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        }
                    } // SubSubCompNum
                } // SubCompNum
            } // Idx
        } // ZoneInNum
    } // Controlled Zone Loop

    for (int airLoopNum = 1; airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++airLoopNum) {
        auto &thisSysLoadRepVars = state.dataSysRpts->SysLoadRepVars(airLoopNum);
        thisSysLoadRepVars.TotHTNG = thisSysLoadRepVars.FANCompHTNG + thisSysLoadRepVars.HCCompHTNG + thisSysLoadRepVars.HeatExHTNG +
                                     thisSysLoadRepVars.HumidHTNG + thisSysLoadRepVars.SolarCollectHeating +
                                     thisSysLoadRepVars.UserDefinedTerminalHeating;
        thisSysLoadRepVars.TotCLNG = thisSysLoadRepVars.CCCompCLNG + thisSysLoadRepVars.HeatExCLNG + thisSysLoadRepVars.EvapCLNG +
                                     thisSysLoadRepVars.DesDehumidCLNG + thisSysLoadRepVars.SolarCollectCooling +
                                     thisSysLoadRepVars.UserDefinedTerminalCooling;
        thisSysLoadRepVars.TotElec = thisSysLoadRepVars.FANCompElec + thisSysLoadRepVars.HCCompElec + thisSysLoadRepVars.CCCompElec +
                                     thisSysLoadRepVars.HCCompElecRes + thisSysLoadRepVars.HumidElec + thisSysLoadRepVars.DesDehumidElec +
                                     thisSysLoadRepVars.EvapElec;
        thisSysLoadRepVars.TotNaturalGas = thisSysLoadRepVars.HCCompNaturalGas + thisSysLoadRepVars.HumidNaturalGas;
        thisSysLoadRepVars.TotPropane = thisSysLoadRepVars.HCCompPropane + thisSysLoadRepVars.HumidPropane;
        thisSysLoadRepVars.TotSteam = thisSysLoadRepVars.HCCompSteam;
        thisSysLoadRepVars.TotH2OCOLD = thisSysLoadRepVars.CCCompH2OCOLD;
        thisSysLoadRepVars.TotH2OHOT = thisSysLoadRepVars.HCCompH2OHOT;
    }
}

void CalcSystemEnergyUse(EnergyPlusData &state,
                         bool const CompLoadFlag,
                         int const AirLoopNum,
                         std::string const &CompType,
                         Constant::eResource const EnergyType,
                         Real64 const CompLoad,
                         Real64 const CompEnergy)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Nov. 2005

    // PURPOSE OF THIS SUBROUTINE:
    // accumulate system loads and energy to report variables

    // Tuned String comparisons were a big performance hit
    // ComponentTypes and component_strings must remain in sync
    enum ComponentTypes
    { // Using older enum style to avoid the name scoping cruft
        AIRLOOPHVAC_OUTDOORAIRSYSTEM,
        AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL,
        AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY,
        AIRLOOPHVAC_UNITARYHEATCOOL,
        AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS,
        AIRLOOPHVAC_UNITARYHEATONLY,
        AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR,
        AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED,
        AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR,
        AIRLOOPHVAC_UNITARYSYSTEM,
        AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL,
        AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT,
        AIRTERMINAL_DUALDUCT_VAV_COOL,
        AIRTERMINAL_DUALDUCT_VAV_HEAT,
        AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR,
        AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR,
        AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM,
        AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM,
        AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION,
        AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT,
        AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT,
        AIRTERMINAL_SINGLEDUCT_MIXER,
        AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT,
        AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT,
        AIRTERMINAL_SINGLEDUCT_USERDEFINED,
        AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT,
        AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT,
        AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT,
        AIRTERMINAL_SINGLEDUCT_VAV_REHEAT,
        AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN,
        COIL_COOLING_DX,
        COIL_COOLING_DX_MULTISPEED,
        COIL_COOLING_DX_SINGLESPEED,
        COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE,
        COIL_COOLING_DX_TWOSPEED,
        COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE,
        COIL_COOLING_DX_VARIABLESPEED,
        COIL_INTEGRATED_DX_VARIABLESPEED,
        COIL_COOLING_WATER,
        COIL_COOLING_WATER_DETAILEDGEOMETRY,
        COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT,
        COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION,
        COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT,
        COIL_HEATING_DESUPERHEATER,
        COIL_HEATING_DX_MULTISPEED,
        COIL_HEATING_DX_SINGLESPEED,
        COIL_HEATING_DX_VARIABLESPEED,
        COIL_HEATING_ELECTRIC,
        COIL_HEATING_ELECTRIC_MULTISTAGE,
        COIL_HEATING_GAS,
        COIL_HEATING_GAS_MULTISTAGE,
        COIL_HEATING_STEAM,
        COIL_HEATING_WATER,
        COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT,
        COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION,
        COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT,
        COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED,
        COIL_USERDEFINED,
        COILSYSTEM_COOLING_DX,
        COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED,
        COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED,
        COILSYSTEM_COOLING_WATER,
        COILSYSTEM_HEATING_DX,
        DEHUMIDIFIER_DESICCANT_NOFANS,
        DEHUMIDIFIER_DESICCANT_SYSTEM,
        DUCT,
        EVAPORATIVECOOLER_DIRECT_CELDEKPAD,
        EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL,
        EVAPORATIVECOOLER_INDIRECT_CELDEKPAD,
        EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL,
        EVAPORATIVECOOLER_INDIRECT_WETCOIL,
        FAN_COMPONENTMODEL,
        FAN_SYSTEMMODEL,
        FAN_CONSTANTVOLUME,
        FAN_ONOFF,
        FAN_VARIABLEVOLUME,
        HEATEXCHANGER_AIRTOAIR_FLATPLATE,
        HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT,
        HEATEXCHANGER_DESICCANT_BALANCEDFLOW,
        HUMIDIFIER_STEAM_ELECTRIC,
        HUMIDIFIER_STEAM_GAS,
        OUTDOORAIR_MIXER,
        SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL,
        SOLARCOLLECTOR_UNGLAZEDTRANSPIRED,
        ZONEHVAC_AIRDISTRIBUTIONUNIT,
        ZONEHVAC_TERMINALUNIT_VRF,
        COIL_COOLING_VRF,
        COIL_HEATING_VRF,
        COIL_COOLING_VRF_FTC,
        COIL_HEATING_VRF_FTC,
        n_ComponentTypes,
        Unknown_ComponentType
    };

    static std::unordered_map<std::string, ComponentTypes> const component_map = {
        {"AIRLOOPHVAC:OUTDOORAIRSYSTEM", AIRLOOPHVAC_OUTDOORAIRSYSTEM},
        {"AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL", AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL},
        {"AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY", AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY},
        {"AIRLOOPHVAC:UNITARYHEATCOOL", AIRLOOPHVAC_UNITARYHEATCOOL},
        {"AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS", AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS},
        {"AIRLOOPHVAC:UNITARYHEATONLY", AIRLOOPHVAC_UNITARYHEATONLY},
        {"AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR", AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR},
        {"AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED", AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED},
        {"AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR", AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR},
        {"AIRLOOPHVAC:UNITARYSYSTEM", AIRLOOPHVAC_UNITARYSYSTEM},
        {"AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:COOL", AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL},
        {"AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:HEAT", AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT},
        {"AIRTERMINAL:DUALDUCT:VAV:COOL", AIRTERMINAL_DUALDUCT_VAV_COOL},
        {"AIRTERMINAL:DUALDUCT:VAV:HEAT", AIRTERMINAL_DUALDUCT_VAV_HEAT},
        {"AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:OUTDOORAIR", AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR},
        {"AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:RECIRCULATEDAIR", AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR},
        {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM},
        {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEBEAM", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM},
        {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION},
        {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT},
        {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT},
        {"AIRTERMINAL:SINGLEDUCT:MIXER", AIRTERMINAL_SINGLEDUCT_MIXER},
        {"AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT", AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT},
        {"AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT", AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT},
        {"AIRTERMINAL:SINGLEDUCT:USERDEFINED", AIRTERMINAL_SINGLEDUCT_USERDEFINED},
        {"AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT", AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT},
        {"AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT", AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT},
        {"AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT", AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT},
        {"AIRTERMINAL:SINGLEDUCT:VAV:REHEAT", AIRTERMINAL_SINGLEDUCT_VAV_REHEAT},
        {"AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN", AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN},
        {"COIL:COOLING:DX", COIL_COOLING_DX},
        {"COIL:COOLING:DX:MULTISPEED", COIL_COOLING_DX_MULTISPEED},
        {"COIL:COOLING:DX:SINGLESPEED", COIL_COOLING_DX_SINGLESPEED},
        {"COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE", COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE},
        {"COIL:COOLING:DX:TWOSPEED", COIL_COOLING_DX_TWOSPEED},
        {"COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE", COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE},
        {"COIL:COOLING:DX:VARIABLESPEED", COIL_COOLING_DX_VARIABLESPEED},
        {"COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE", COIL_INTEGRATED_DX_VARIABLESPEED},
        {"COIL:COOLING:WATER", COIL_COOLING_WATER},
        {"COIL:COOLING:WATER:DETAILEDGEOMETRY", COIL_COOLING_WATER_DETAILEDGEOMETRY},
        {"COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT", COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT},
        {"COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION", COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION},
        {"COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT", COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT},
        {"COIL:HEATING:DESUPERHEATER", COIL_HEATING_DESUPERHEATER},
        {"COIL:HEATING:DX:MULTISPEED", COIL_HEATING_DX_MULTISPEED},
        {"COIL:HEATING:DX:SINGLESPEED", COIL_HEATING_DX_SINGLESPEED},
        {"COIL:HEATING:DX:VARIABLESPEED", COIL_HEATING_DX_VARIABLESPEED},
        {"COIL:HEATING:ELECTRIC", COIL_HEATING_ELECTRIC},
        {"COIL:HEATING:ELECTRIC:MULTISTAGE", COIL_HEATING_ELECTRIC_MULTISTAGE},
        {"COIL:HEATING:FUEL", COIL_HEATING_GAS},
        {"COIL:HEATING:GAS:MULTISTAGE", COIL_HEATING_GAS_MULTISTAGE},
        {"COIL:HEATING:STEAM", COIL_HEATING_STEAM},
        {"COIL:HEATING:WATER", COIL_HEATING_WATER},
        {"COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT", COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT},
        {"COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION", COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION},
        {"COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT", COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT},
        {"COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED", COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED},
        {"COIL:USERDEFINED", COIL_USERDEFINED},
        {"COILSYSTEM:COOLING:DX", COILSYSTEM_COOLING_DX},
        {"COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED", COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED},
        {"COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED", COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED},
        {"COILSYSTEM:COOLING:WATER", COILSYSTEM_COOLING_WATER},
        {"COILSYSTEM:HEATING:DX", COILSYSTEM_HEATING_DX},
        {"DEHUMIDIFIER:DESICCANT:NOFANS", DEHUMIDIFIER_DESICCANT_NOFANS},
        {"DEHUMIDIFIER:DESICCANT:SYSTEM", DEHUMIDIFIER_DESICCANT_SYSTEM},
        {"DUCT", DUCT},
        {"EVAPORATIVECOOLER:DIRECT:CELDEKPAD", EVAPORATIVECOOLER_DIRECT_CELDEKPAD},
        {"EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL", EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL},
        {"EVAPORATIVECOOLER:INDIRECT:CELDEKPAD", EVAPORATIVECOOLER_INDIRECT_CELDEKPAD},
        {"EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL", EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL},
        {"EVAPORATIVECOOLER:INDIRECT:WETCOIL", EVAPORATIVECOOLER_INDIRECT_WETCOIL},
        {"FAN:COMPONENTMODEL", FAN_COMPONENTMODEL},
        {"FAN:SYSTEMMODEL", FAN_SYSTEMMODEL},
        {"FAN:CONSTANTVOLUME", FAN_CONSTANTVOLUME},
        {"FAN:ONOFF", FAN_ONOFF},
        {"FAN:VARIABLEVOLUME", FAN_VARIABLEVOLUME},
        {"HEATEXCHANGER:AIRTOAIR:FLATPLATE", HEATEXCHANGER_AIRTOAIR_FLATPLATE},
        {"HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT", HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT},
        {"HEATEXCHANGER:DESICCANT:BALANCEDFLOW", HEATEXCHANGER_DESICCANT_BALANCEDFLOW},
        {"HUMIDIFIER:STEAM:ELECTRIC", HUMIDIFIER_STEAM_ELECTRIC},
        {"HUMIDIFIER:STEAM:GAS", HUMIDIFIER_STEAM_GAS},
        {"OUTDOORAIR:MIXER", OUTDOORAIR_MIXER},
        {"SOLARCOLLECTOR:FLATPLATE:PHOTOVOLTAICTHERMAL", SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL},
        {"SOLARCOLLECTOR:UNGLAZEDTRANSPIRED", SOLARCOLLECTOR_UNGLAZEDTRANSPIRED},
        {"ZONEHVAC:AIRDISTRIBUTIONUNIT", ZONEHVAC_AIRDISTRIBUTIONUNIT},
        {"ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW", ZONEHVAC_TERMINALUNIT_VRF},
        {"COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW", COIL_COOLING_VRF},
        {"COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW", COIL_HEATING_VRF},
        {"COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL", COIL_COOLING_VRF_FTC},
        {"COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL", COIL_HEATING_VRF_FTC}};
    assert(component_map.size() == n_ComponentTypes);

    if (!state.dataSysRpts->AirLoopLoadsReportEnabled) {
        return;
    }
    auto &thisSysLoadRepVars = state.dataSysRpts->SysLoadRepVars(AirLoopNum);

    // Find enum for the component type string
    ComponentTypes comp_type;
    auto const it = component_map.find(CompType);
    if (it != component_map.end()) {
        comp_type = it->second;
    } else {
        comp_type = Unknown_ComponentType;
    }

    switch (comp_type) {
    case AIRLOOPHVAC_OUTDOORAIRSYSTEM: // Outside Air System
        // Not reported
        break;
    case OUTDOORAIR_MIXER: // Outdoor Air Mixer
        // No energy transfers to account for
        break;
    case AIRTERMINAL_SINGLEDUCT_MIXER:
        // No energy transfers to account for
        break;
    case FAN_CONSTANTVOLUME:
    case FAN_VARIABLEVOLUME:
    case FAN_ONOFF:
    case FAN_SYSTEMMODEL:
    case FAN_COMPONENTMODEL:

        if (CompLoadFlag) {
            thisSysLoadRepVars.FANCompHTNG += std::abs(CompLoad);
        }
        thisSysLoadRepVars.FANCompElec += CompEnergy;

        // Cooling Coil Types for the air sys simulation
        break;
    case COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED:
    case COIL_COOLING_DX_SINGLESPEED:
    case COIL_COOLING_DX_TWOSPEED:
    case COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE:
    case COIL_COOLING_DX:
    case COIL_COOLING_DX_MULTISPEED:
    case COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT:
    case COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION:
    case COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT:
    case COIL_COOLING_DX_VARIABLESPEED:
    case COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED:
    case COILSYSTEM_COOLING_WATER:
    case COIL_COOLING_WATER_DETAILEDGEOMETRY:
    case COIL_COOLING_WATER:
    case COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE:
    case COIL_COOLING_VRF:
    case COIL_COOLING_VRF_FTC:
    case COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED:

        if (CompLoadFlag) {
            thisSysLoadRepVars.CCCompCLNG += std::abs(CompLoad);
        }
        if ((EnergyType == Constant::eResource::PlantLoopCoolingDemand) || (EnergyType == Constant::eResource::DistrictCooling)) {
            thisSysLoadRepVars.CCCompH2OCOLD += CompEnergy;
        } else if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.CCCompElec += CompEnergy;
        }

        // Heating Coil Types for the air sys simulation
        break;
    case COIL_HEATING_WATER:
    case COIL_HEATING_DX_SINGLESPEED:
    case COIL_HEATING_DX_MULTISPEED:
    case COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT:
    case COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION:
    case COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT:
    case COIL_HEATING_DX_VARIABLESPEED:
    case COIL_HEATING_STEAM:
    case COIL_HEATING_GAS:
    case COIL_HEATING_GAS_MULTISTAGE:
    case COIL_HEATING_DESUPERHEATER:

        if (CompLoadFlag) {
            thisSysLoadRepVars.HCCompHTNG += std::abs(CompLoad);
        }
        if ((EnergyType == Constant::eResource::PlantLoopHeatingDemand) || (EnergyType == Constant::eResource::DistrictHeatingWater)) {
            thisSysLoadRepVars.HCCompH2OHOT += CompEnergy;
        } else if (EnergyType == Constant::eResource::DistrictHeatingSteam) {
            thisSysLoadRepVars.HCCompSteam += CompEnergy;
        } else if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.HCCompElec += CompEnergy;
        } else if (EnergyType == Constant::eResource::NaturalGas) {
            thisSysLoadRepVars.HCCompNaturalGas += CompEnergy;
        } else if (EnergyType == Constant::eResource::Propane) {
            thisSysLoadRepVars.HCCompPropane += CompEnergy;
        }

        break;
    case COIL_HEATING_ELECTRIC:
    case COIL_HEATING_ELECTRIC_MULTISTAGE:

        if (CompLoadFlag) {
            thisSysLoadRepVars.HCCompHTNG += std::abs(CompLoad);
        }
        if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.HCCompElecRes += CompEnergy;
        }

        break;
    case COIL_USERDEFINED:

        if (CompLoadFlag) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.CCCompCLNG += std::abs(CompLoad);
            } else {
                thisSysLoadRepVars.HCCompHTNG += std::abs(CompLoad);
            }
        }
        if ((EnergyType == Constant::eResource::PlantLoopHeatingDemand) || (EnergyType == Constant::eResource::DistrictHeatingWater)) {
            thisSysLoadRepVars.HCCompH2OHOT += CompEnergy;
        } else if ((EnergyType == Constant::eResource::PlantLoopCoolingDemand) || (EnergyType == Constant::eResource::DistrictCooling)) {
            thisSysLoadRepVars.CCCompH2OCOLD += CompEnergy;
        } else if (EnergyType == Constant::eResource::DistrictHeatingSteam) {
            thisSysLoadRepVars.HCCompSteam += CompEnergy;
        } else if (EnergyType == Constant::eResource::Electricity) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.CCCompElec += CompEnergy;
            } else {
                thisSysLoadRepVars.HCCompElec += CompEnergy;
            }
        } else if (EnergyType == Constant::eResource::NaturalGas) {
            thisSysLoadRepVars.HCCompNaturalGas += CompEnergy;
        } else if (EnergyType == Constant::eResource::Propane) {
            thisSysLoadRepVars.HCCompPropane += CompEnergy;
        }

        // DX Systems
        break;
    case COIL_HEATING_VRF:
    case COIL_HEATING_VRF_FTC:
    case AIRLOOPHVAC_UNITARYSYSTEM:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR:
        // All energy transfers accounted for in subcomponent models
        break;
    case COILSYSTEM_COOLING_DX:
        // All energy transfers accounted for in subcomponent models
        break;
    case COILSYSTEM_HEATING_DX:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATONLY:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATCOOL:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS:
        // All energy transfers accounted for in subcomponent models
        break;
    case AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED:
        // All energy transfers accounted for in subcomponent models
        break;
    case ZONEHVAC_TERMINALUNIT_VRF:
        // All energy transfers accounted for in subcomponent models
        break;
        // Humidifier Types for the air system simulation
    case HUMIDIFIER_STEAM_GAS:
    case HUMIDIFIER_STEAM_ELECTRIC:
        if (CompLoadFlag) {
            thisSysLoadRepVars.HumidHTNG += std::abs(CompLoad);
        }
        if (EnergyType == Constant::eResource::Water) {
            thisSysLoadRepVars.DomesticH2O += std::abs(CompEnergy);
        } else if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.HumidElec += CompEnergy;
        } else if (EnergyType == Constant::eResource::NaturalGas) {
            thisSysLoadRepVars.HumidNaturalGas += CompEnergy;
        } else if (EnergyType == Constant::eResource::Propane) {
            thisSysLoadRepVars.HumidPropane += CompEnergy;
        }

        // Evap Cooler Types for the air system simulation
        break;
    case EVAPORATIVECOOLER_DIRECT_CELDEKPAD:
    case EVAPORATIVECOOLER_INDIRECT_CELDEKPAD:
    case EVAPORATIVECOOLER_INDIRECT_WETCOIL:
    case EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL:
    case EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL:
        if (CompLoadFlag) {
            thisSysLoadRepVars.EvapCLNG += std::abs(CompLoad);
        }
        if (EnergyType == Constant::eResource::Water) {
            thisSysLoadRepVars.DomesticH2O += std::abs(CompEnergy);
        } else if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.EvapElec += CompEnergy;
        }

        // Desiccant Dehumidifier Types for the air system simulation
        break;
    case DEHUMIDIFIER_DESICCANT_NOFANS:
    case DEHUMIDIFIER_DESICCANT_SYSTEM:
        if (CompLoadFlag) {
            thisSysLoadRepVars.DesDehumidCLNG += std::abs(CompLoad);
        }
        if (EnergyType == Constant::eResource::Electricity) {
            thisSysLoadRepVars.DesDehumidElec += CompEnergy;
        }

        // Heat Exchanger Types
        break;
    case HEATEXCHANGER_AIRTOAIR_FLATPLATE:
    case HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT:
    case HEATEXCHANGER_DESICCANT_BALANCEDFLOW:
        if (CompLoadFlag) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.HeatExCLNG += std::abs(CompLoad);
            } else {
                thisSysLoadRepVars.HeatExHTNG += std::abs(CompLoad);
            }
        }

        // Air Terminal Types
        break;
    case AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL:
    case AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT:
    case AIRTERMINAL_DUALDUCT_VAV_COOL:
    case AIRTERMINAL_DUALDUCT_VAV_HEAT:
    case AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR:
    case AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR:
    case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION:
    case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT:
    case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT:
    case AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT:
    case AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT:
    case AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT:
    case AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT:
    case AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT:
    case AIRTERMINAL_SINGLEDUCT_VAV_REHEAT:
    case AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN:
    case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM:
    case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM:
    case ZONEHVAC_AIRDISTRIBUTIONUNIT:
        // All energy transfers accounted for in component models

        // Duct Types
        break;
    case DUCT:
        // duct losses should be accounted for here ???
        // requires addition of a new variable to sum duct losses
        // Example:
        //      IF(CompLoad > 0.0d0)THEN
        //        SysDuctHTNG =  SysDuctHTNG + ABS(CompLoad)
        //      ELSE
        //        SysDuctCLNG =  SysDuctCLNG + ABS(CompLoad)
        //      ENDIF

        // Solar Collector Types
        break;
    case SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL:
    case SOLARCOLLECTOR_UNGLAZEDTRANSPIRED:
        if (CompLoadFlag) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.SolarCollectCooling += std::abs(CompLoad);
            } else {
                thisSysLoadRepVars.SolarCollectHeating += std::abs(CompLoad);
            }
        }

        break;
    case AIRTERMINAL_SINGLEDUCT_USERDEFINED:
        // User component model energy use should be accounted for here
        if (CompLoadFlag) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.UserDefinedTerminalCooling += std::abs(CompLoad);
            } else {
                thisSysLoadRepVars.UserDefinedTerminalHeating += std::abs(CompLoad);
            }
        }
        if ((EnergyType == Constant::eResource::PlantLoopHeatingDemand) || (EnergyType == Constant::eResource::DistrictHeatingWater)) {
            thisSysLoadRepVars.HCCompH2OHOT += CompEnergy;
        } else if ((EnergyType == Constant::eResource::PlantLoopCoolingDemand) || (EnergyType == Constant::eResource::DistrictCooling)) {
            thisSysLoadRepVars.CCCompH2OCOLD += CompEnergy;
        } else if (EnergyType == Constant::eResource::DistrictHeatingSteam) {
            thisSysLoadRepVars.HCCompSteam += CompEnergy;
        } else if (EnergyType == Constant::eResource::Electricity) {
            if (CompLoad > 0.0) {
                thisSysLoadRepVars.CCCompElec += CompEnergy;
            } else {
                thisSysLoadRepVars.HCCompElec += CompEnergy;
            }
        } else if (EnergyType == Constant::eResource::NaturalGas) {
            thisSysLoadRepVars.HCCompNaturalGas += CompEnergy;
        } else if (EnergyType == Constant::eResource::Propane) {
            thisSysLoadRepVars.HCCompPropane += CompEnergy;
        }
        // Recurring warning for unaccounted equipment types
        // (should never happen, when this does happen enter appropriate equipment CASE statement above)
        break;
    case COIL_INTEGRATED_DX_VARIABLESPEED:
        // All energy transfers accounted for in component models
        break;
    default:
        int found = 0;
        if (state.dataSysRpts->NumCompTypes > 0) {
            found = Util::FindItemInList(CompType, state.dataSysRpts->CompTypeErrors, &CompTypeError::CompType, state.dataSysRpts->NumCompTypes);
        }
        if (found == 0) {
            state.dataSysRpts->CompTypeErrors(++state.dataSysRpts->NumCompTypes).CompType = CompType;
            found = state.dataSysRpts->NumCompTypes;
        }
        ShowRecurringSevereErrorAtEnd(state,
                                      "CalcSystemEnergyUse: Component Type=" + CompType + " not logged as one of allowable Component Types.",
                                      state.dataSysRpts->CompTypeErrors(found).CompErrIndex);
        break;
    } // switch
}

void ReportVentilationLoads(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher (with minor assistance from RKS)
    //       DATE WRITTEN   July 2004
    //       MODIFIED       Dec. 2006, BG. reengineered to add zone forced air units to vent rates and loads

    // PURPOSE OF THIS SUBROUTINE:
    // calculate and report zone ventilation loads

    // METHODOLOGY EMPLOYED:
    // calculate energy contribution of outside air through mixing box and pro-rate to
    // zones according to zone mass flow rates.

    Real64 constexpr SmallLoad(0.1); // (W)

    auto &Node = state.dataLoopNodes->Node;
    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    if (!state.dataSysRpts->VentReportStructureCreated) {
        return;
    }
    if (!state.dataSysRpts->VentLoadsReportEnabled) {
        return;
    }
    // following inits are array assignments across all controlled zones.
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        auto &thisZoneVentRepVars = state.dataSysRpts->ZoneVentRepVars(zoneNum);
        if (!state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            continue;
        }
        thisZoneVentRepVars.OAMassFlow = 0.0;
        thisZoneVentRepVars.OAMass = 0.0;
        thisZoneVentRepVars.OAVolFlowStdRho = 0.0;
        thisZoneVentRepVars.OAVolStdRho = 0.0;
        thisZoneVentRepVars.OAVolFlowCrntRho = 0.0;
        thisZoneVentRepVars.OAVolCrntRho = 0.0;
        thisZoneVentRepVars.MechACH = 0.0;
        thisZoneVentRepVars.TargetVentilationFlowVoz = 0.0;
        thisZoneVentRepVars.TimeBelowVozDyn = 0.0;
        thisZoneVentRepVars.TimeAtVozDyn = 0.0;
        thisZoneVentRepVars.TimeAboveVozDyn = 0.0;
        thisZoneVentRepVars.TimeVentUnocc = 0.0;
        thisZoneVentRepVars.CoolingLoadMetByVent = 0.0;
        thisZoneVentRepVars.CoolingLoadAddedByVent = 0.0;
        thisZoneVentRepVars.OvercoolingByVent = 0.0;
        thisZoneVentRepVars.HeatingLoadMetByVent = 0.0;
        thisZoneVentRepVars.HeatingLoadAddedByVent = 0.0;
        thisZoneVentRepVars.OverheatingByVent = 0.0;
        thisZoneVentRepVars.NoLoadHeatingByVent = 0.0;
        thisZoneVentRepVars.NoLoadCoolingByVent = 0.0;
    }

    state.dataSysRpts->AnyZoneTimeBelowVozDyn = 0.0;
    state.dataSysRpts->AllZonesTimeAtVozDyn = 0.0;
    state.dataSysRpts->AnyZoneTimeAboveVozDyn = 0.0;
    state.dataSysRpts->AnyZoneTimeVentUnocc = 0.0;
    state.dataSysRpts->AnyZoneTimeBelowVozDynOcc = 0.0;
    state.dataSysRpts->AllZonesTimeAtVozDynOcc = 0.0;
    state.dataSysRpts->AnyZoneTimeAboveVozDynOcc = 0.0;

    for (int sysNum = 1; sysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++sysNum) {
        auto &thisSysVentRepVars = state.dataSysRpts->SysVentRepVars(sysNum);
        thisSysVentRepVars.MechVentFlow = 0.0;
        thisSysVentRepVars.NatVentFlow = 0.0;
        thisSysVentRepVars.TargetVentilationFlowVoz = 0.0;
        thisSysVentRepVars.TimeBelowVozDyn = 0.0;
        thisSysVentRepVars.TimeAtVozDyn = 0.0;
        thisSysVentRepVars.TimeAboveVozDyn = 0.0;
        thisSysVentRepVars.TimeVentUnocc = 0.0;
        thisSysVentRepVars.AnyZoneOccupied = false;
    }

    for (int CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum);
        if (!thisZoneEquipConfig.IsControlled) {
            continue;
        }

        Real64 ZAirSysZoneVentLoad = 0.0; // ventilation load attributed to a particular zone from all primary air systems serving the zone [J]
        Real64 ZAirSysOutAirFlow = 0.0;   // outside air flow rate for zone from all primary air systems serving the zone [kg/s]
        Real64 ZFAUFlowRate = 0.0;        // Zone forced Air unit air mass flow rate [kg/s]
        Real64 ZFAUZoneVentLoad = 0.0;    // ventilation load attributed to a particular zone from zone forced air units [J]
        Real64 ZFAUOutAirFlow = 0.0;      // outside air flow rate for zone from zone forced air units. [kg/s]
        Real64 OutAirFlow = 0.0;          // Total outside air mass flow from zone equipment and air loop equipment [kg/s]
        Real64 ZoneFlowFrac = 0.0;        // fraction of mixed air flowing to a zone
        Real64 ZFAUEnthReturnAir = 0.0;   // Zone forced Air unit enthalpy of the return air [kJ/kgK]
        Real64 ZFAUTempMixedAir = 0.0;    // Zone forced Air unit dry-bulb temperature of the mixed air [C]
        Real64 ZFAUHumRatMixedAir = 0.0;  // Zone forced Air unit humidity ratio of the mixed air [kg/kg]
        Real64 ZFAUEnthMixedAir = 0.0;    // Zone forced Air unit enthalpy of the mixed air [kJ/kgK]
        Real64 ZFAUEnthOutdoorAir = 0.0;  // Zone forced Air unit enthalpy of the outdoor air [kJ/kgK]

        // retrieve the zone load for each zone
        Real64 ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(CtrlZoneNum).TotalOutputRequired;
        auto const &thisZone = state.dataHeatBal->Zone(CtrlZoneNum);
        int const zoneMult = thisZone.Multiplier * thisZone.ListMultiplier;
        Real64 const ZoneVolume = thisZone.Volume * zoneMult; // CR 7170

        bool constexpr UseOccSchFlag = true;
        bool constexpr UseMinOASchFlag = true;

        auto &thisZonePredefRep = state.dataHeatBal->ZonePreDefRep(CtrlZoneNum);
        auto &thisZoneVentRepVars = state.dataSysRpts->ZoneVentRepVars(CtrlZoneNum);
        thisZoneVentRepVars.TargetVentilationFlowVoz = DataSizing::calcDesignSpecificationOutdoorAir(
            state, thisZoneEquipConfig.ZoneDesignSpecOAIndex, CtrlZoneNum, UseOccSchFlag, UseMinOASchFlag);
        if (thisZoneEquipConfig.ZoneAirDistributionIndex > 0) {
            thisZoneVentRepVars.TargetVentilationFlowVoz =
                thisZoneVentRepVars.TargetVentilationFlowVoz /
                state.dataSize->ZoneAirDistribution(thisZoneEquipConfig.ZoneAirDistributionIndex).calculateEz(state, CtrlZoneNum);
        }

        // if system operating in deadband reset zone load
        if (state.dataZoneEnergyDemand->DeadBandOrSetback(CtrlZoneNum)) {
            ZoneLoad = 0.0;
        }

        // first deal with any (and all) Zone Forced Air Units that might have outside air.
        auto &thisZoneEquipList = state.dataZoneEquip->ZoneEquipList(thisZoneEquipConfig.EquipListIndex);
        for (int thisZoneEquipNum = 1; thisZoneEquipNum <= thisZoneEquipList.NumOfEquipTypes; ++thisZoneEquipNum) {
            auto &thisEquipIndex = thisZoneEquipList.EquipIndex(thisZoneEquipNum);

            switch (thisZoneEquipList.EquipType(thisZoneEquipNum)) {
                // case statement to cover all possible zone forced air units that could have outside air

            case DataZoneEquipment::ZoneEquipType::WindowAirConditioner: { // Window Air Conditioner
                int OutAirNode = WindowAC::GetWindowACOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = WindowAC::GetWindowACZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = WindowAC::GetWindowACMixedAirNode(state, thisEquipIndex);
                int ReturnAirNode = WindowAC::GetWindowACReturnAirNode(state, thisEquipIndex);
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::VariableRefrigerantFlowTerminal: {
                int OutAirNode = HVACVariableRefrigerantFlow::GetVRFTUOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }
                int ZoneInletAirNode = HVACVariableRefrigerantFlow::GetVRFTUZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = HVACVariableRefrigerantFlow::GetVRFTUMixedAirNode(state, thisEquipIndex);
                int ReturnAirNode = HVACVariableRefrigerantFlow::GetVRFTUReturnAirNode(state, thisEquipIndex);
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPump:
            case DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner:
            case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPumpWaterToAir: {
                // loop index accesses correct pointer to equipment on this equipment list, DataZoneEquipment::GetZoneEquipmentData
                // thisEquipIndex (EquipIndex) is used to access specific equipment for a single class of equipment (e.g., PTAC 1, 2 and 3)
                int OutAirNode = thisZoneEquipList.compPointer[thisZoneEquipNum]->getMixerOANode();
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }
                int ZoneInletAirNode = thisZoneEquipList.compPointer[thisZoneEquipNum]->getAirOutletNode();
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = thisZoneEquipList.compPointer[thisZoneEquipNum]->getMixerMixNode();
                int ReturnAirNode = thisZoneEquipList.compPointer[thisZoneEquipNum]->getMixerRetNode();
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::FourPipeFanCoil: {
                int OutAirNode = FanCoilUnits::GetFanCoilOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = FanCoilUnits::GetFanCoilZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = FanCoilUnits::GetFanCoilMixedAirNode(state, thisEquipIndex);
                int ReturnAirNode = FanCoilUnits::GetFanCoilReturnAirNode(state, thisEquipIndex);
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::UnitVentilator: {
                int OutAirNode = UnitVentilator::GetUnitVentilatorOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = UnitVentilator::GetUnitVentilatorZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = UnitVentilator::GetUnitVentilatorMixedAirNode(state, thisEquipIndex);
                int ReturnAirNode = UnitVentilator::GetUnitVentilatorReturnAirNode(state, thisEquipIndex);
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::PurchasedAir: {
                ZFAUOutAirFlow += PurchasedAirManager::GetPurchasedAirOutAirMassFlow(state, thisEquipIndex);
                int ZoneInletAirNode = PurchasedAirManager::GetPurchasedAirZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                ZFAUTempMixedAir = PurchasedAirManager::GetPurchasedAirMixedAirTemp(state, thisEquipIndex);
                ZFAUHumRatMixedAir = PurchasedAirManager::GetPurchasedAirMixedAirHumRat(state, thisEquipIndex);
                int ReturnAirNode = PurchasedAirManager::GetPurchasedAirReturnAirNode(state, thisEquipIndex);
                if ((ZFAUFlowRate > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(ZFAUTempMixedAir, ZFAUHumRatMixedAir);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::EnergyRecoveryVentilator: {
                int OutAirNode = HVACStandAloneERV::GetStandAloneERVOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = HVACStandAloneERV::GetStandAloneERVZoneInletAirNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int MixedAirNode = ZoneInletAirNode;
                int ReturnAirNode = HVACStandAloneERV::GetStandAloneERVReturnAirNode(state, thisEquipIndex);
                if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::UnitarySystem: {
                // add accounting for OA when unitary system is used as zone equipment
            } break;

            case DataZoneEquipment::ZoneEquipType::OutdoorAirUnit: {
                int OutAirNode = OutdoorAirUnit::GetOutdoorAirUnitOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = OutdoorAirUnit::GetOutdoorAirUnitZoneInletNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }
                int ReturnAirNode = OutdoorAirUnit::GetOutdoorAirUnitReturnAirNode(state, thisEquipIndex);
                if ((OutAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    ZFAUEnthOutdoorAir = Psychrometrics::PsyHFnTdbW(Node(OutAirNode).Temp, Node(OutAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthOutdoorAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::HybridEvaporativeCooler: {
                int OutAirNode = HybridUnitaryAirConditioners::GetHybridUnitaryACOutAirNode(state, thisEquipIndex);
                if (OutAirNode > 0) {
                    ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                }

                int ZoneInletAirNode = HybridUnitaryAirConditioners::GetHybridUnitaryACZoneInletNode(state, thisEquipIndex);
                if (ZoneInletAirNode > 0) {
                    ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                }

                int ReturnAirNode = HybridUnitaryAirConditioners::GetHybridUnitaryACReturnAirNode(state, thisEquipIndex);
                if ((OutAirNode > 0) && (ReturnAirNode > 0)) {
                    ZFAUEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    ZFAUEnthOutdoorAir = Psychrometrics::PsyHFnTdbW(Node(OutAirNode).Temp, Node(OutAirNode).HumRat);
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)

                    ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthOutdoorAir - ZFAUEnthReturnAir) * TimeStepSysSec; //*KJperJ
                } else {
                    ZFAUZoneVentLoad += 0.0;
                }
            } break;

            case DataZoneEquipment::ZoneEquipType::UnitHeater:
            case DataZoneEquipment::ZoneEquipType::VentilatedSlab:
                //    ZoneHVAC:EvaporativeCoolerUnit ?????
            case DataZoneEquipment::ZoneEquipType::EvaporativeCooler:
            case DataZoneEquipment::ZoneEquipType::AirDistributionUnit:
            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater:
            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric:
            case DataZoneEquipment::ZoneEquipType::HighTemperatureRadiant:
                //    not sure how HeatExchanger:* could be used as zone equipment ?????
            case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantConstFlow:
            case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantVarFlow:
            case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantElectric:
            case DataZoneEquipment::ZoneEquipType::ExhaustFan:
            case DataZoneEquipment::ZoneEquipType::HeatExchanger:
                // HPWaterHeater can be used as zone equipment
            case DataZoneEquipment::ZoneEquipType::HeatPumpWaterHeaterPumpedCondenser:
            case DataZoneEquipment::ZoneEquipType::HeatPumpWaterHeaterWrappedCondenser:
            case DataZoneEquipment::ZoneEquipType::BaseboardWater:
            case DataZoneEquipment::ZoneEquipType::DehumidifierDX:
            case DataZoneEquipment::ZoneEquipType::BaseboardSteam:
            case DataZoneEquipment::ZoneEquipType::BaseboardElectric:
            case DataZoneEquipment::ZoneEquipType::RefrigerationChillerSet:
            case DataZoneEquipment::ZoneEquipType::UserDefinedHVACForcedAir:
            case DataZoneEquipment::ZoneEquipType::CoolingPanel: {
                // do nothing, OA not included
            } break;

            default: {
                ShowFatalError(state,
                               "ReportMaxVentilationLoads: Developer must either create accounting for OA or include in final else if "
                               "to do nothing");

                break;
            }
            }
        }

        // loop over the zone supply air path inlet nodes
        for (int ZoneInNum = 1; ZoneInNum <= thisZoneEquipConfig.NumInletNodes; ++ZoneInNum) {
            Real64 AirSysEnthReturnAir = 0.0;    // enthalpy of the return air (mixing box inlet node, return side) [kJ/kgK]
            Real64 AirSysEnthMixedAir = 0.0;     // enthalpy of the mixed air (mixing box outlet node, mixed air side) [kJ/kgK]
            Real64 AirSysZoneVentLoad = 0.0;     // ventilation load attributed to a particular zone from primary air system [J]
            Real64 ADUCoolFlowrate = 0.0;        // Air distribution unit cooling air mass flow rate [kg/s]
            Real64 ADUHeatFlowrate = 0.0;        // Air distribution unit heating air mass flow rate [kg/s]
            Real64 AirSysTotalMixFlowRate = 0.0; // Mixed air mass flow rate [kg/s]
            Real64 AirSysOutAirFlow = 0.0;       // outside air flow rate for zone from primary air system [kg/s]
            // retrieve air loop index
            int AirLoopNum = thisZoneEquipConfig.InletNodeAirLoopNum(ZoneInNum);
            int MixedAirNode = 0;
            int ReturnAirNode = 0;
            int AirDistCoolInletNodeNum = 0;
            int AirDistHeatInletNodeNum = 0;
            if (AirLoopNum != 0) { // deal with primary air system
                AirDistCoolInletNodeNum = max(thisZoneEquipConfig.AirDistUnitCool(ZoneInNum).InNode, 0);
                AirDistHeatInletNodeNum = max(thisZoneEquipConfig.AirDistUnitHeat(ZoneInNum).InNode, 0);
                // Set for cooling or heating path
                if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum == 0) {
                    ADUCoolFlowrate = max(Node(thisZoneEquipConfig.AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                          0.0); // CR7244 need to accumulate flow across multiple inlets
                } else if (AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum == 0) {
                    ADUHeatFlowrate = max(Node(thisZoneEquipConfig.AirDistUnitHeat(ZoneInNum).InNode).MassFlowRate,
                                          0.0); // CR7244 need to accumulate flow across multiple inlets
                } else if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum != AirDistHeatInletNodeNum) {
                    // dual ducts! CR7244 need to accumulate flow across multiple inlets (don't count same inlet twice)
                    ADUHeatFlowrate = max(Node(thisZoneEquipConfig.AirDistUnitHeat(ZoneInNum).InNode).MassFlowRate,
                                          0.0); // CR7244 need to accumulate flow across multiple inlets
                    ADUCoolFlowrate = max(Node(thisZoneEquipConfig.AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                          0.0); // CR7244 need to accumulate flow across multiple inlets
                } else if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum > 0) {
                    // dual ducts! CR7244 need to accumulate flow across multiple inlets (don't count same inlet twice)
                    ADUCoolFlowrate = max(Node(thisZoneEquipConfig.AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                          0.0); // CR7244 need to accumulate flow across multiple inlets
                } else {
                    // do nothing (already inits)
                }
                // Find the mixed air node and return air node of the system that supplies the zone
                MixedAirNode = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OASysOutletNodeNum;
                ReturnAirNode = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OASysInletNodeNum;

                // Collect air loop Voz-dyn and natural ventilation
                int ADUNum = thisZoneEquipConfig.InletNodeADUNum(ZoneInNum);
                Real64 termUnitOAFrac = 1.0;
                if (ADUNum > 0) {
                    int termUnitSizingNum = state.dataDefineEquipment->AirDistUnit(ADUNum).TermUnitSizingNum;
                    if (termUnitSizingNum > 0) {
                        termUnitOAFrac = state.dataSize->TermUnitSizing(termUnitSizingNum).SpecMinOAFrac;
                    }
                }
                state.dataSysRpts->SysVentRepVars(AirLoopNum).TargetVentilationFlowVoz +=
                    termUnitOAFrac * thisZoneVentRepVars.TargetVentilationFlowVoz;

                Real64 naturalVentFlow = (state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeStdDensity + thisZonePredefRep.AFNVentVolStdDen) *
                                         zoneMult / TimeStepSysSec;
                state.dataSysRpts->SysVentRepVars(AirLoopNum).NatVentFlow += termUnitOAFrac * naturalVentFlow;

                if (thisZonePredefRep.isOccupied) {
                    state.dataSysRpts->SysVentRepVars(AirLoopNum).AnyZoneOccupied = true;
                }
            }

            if (MixedAirNode == 0 || ReturnAirNode == 0) {
                AirSysZoneVentLoad = 0.0;
                AirSysOutAirFlow = 0.0;
            } else {
                // Calculate return and mixed air enthalpies
                AirSysEnthReturnAir = Psychrometrics::PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                AirSysEnthMixedAir = Psychrometrics::PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);

                if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OASysExists) {
                    int OutAirNode = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OAMixOAInNodeNum;
                    AirSysOutAirFlow = Node(OutAirNode).MassFlowRate;
                } else {
                    AirSysOutAirFlow = 0.0;
                }

                AirSysTotalMixFlowRate = Node(MixedAirNode).MassFlowRate;

                if (AirSysTotalMixFlowRate != 0.0) {
                    ZoneFlowFrac = (ADUCoolFlowrate + ADUHeatFlowrate) / AirSysTotalMixFlowRate;
                    AirSysOutAirFlow *= ZoneFlowFrac;
                } else {
                    ZoneFlowFrac = 0.0;
                    AirSysOutAirFlow = 0.0;
                }
                // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                AirSysZoneVentLoad = (ADUCoolFlowrate + ADUHeatFlowrate) * (AirSysEnthMixedAir - AirSysEnthReturnAir) * TimeStepSysSec; //*KJperJ
            }
            ZAirSysZoneVentLoad += AirSysZoneVentLoad;
            ZAirSysOutAirFlow += AirSysOutAirFlow;
        } // primary air system present

        // now combine OA flow from zone forced air units with primary air system
        OutAirFlow = ZAirSysOutAirFlow + ZFAUOutAirFlow;
        // assign report variables
        thisZoneVentRepVars.OAMassFlow = OutAirFlow;
        thisZoneVentRepVars.OAMass = thisZoneVentRepVars.OAMassFlow * TimeStepSysSec;

        // determine volumetric values from mass flow using standard density (adjusted for elevation)
        thisZoneVentRepVars.OAVolFlowStdRho = thisZoneVentRepVars.OAMassFlow / state.dataEnvrn->StdRhoAir;
        thisZoneVentRepVars.OAVolStdRho = thisZoneVentRepVars.OAVolFlowStdRho * TimeStepSysSec;

        // set time mechanical+natural ventilation is below, at, or above target Voz-dyn
        Real64 totMechNatVentVolStdRho =
            thisZoneVentRepVars.OAVolStdRho +
            (state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeStdDensity + thisZonePredefRep.AFNVentVolStdDen) * zoneMult;
        Real64 targetVoz = thisZoneVentRepVars.TargetVentilationFlowVoz * TimeStepSysSec;
        // Allow 1% tolerance
        if (totMechNatVentVolStdRho < (0.99 * targetVoz)) {
            thisZoneVentRepVars.TimeBelowVozDyn = TimeStepSys;
            state.dataSysRpts->AnyZoneTimeBelowVozDyn = TimeStepSys;
        } else if (totMechNatVentVolStdRho > (1.01 * targetVoz)) {
            thisZoneVentRepVars.TimeAboveVozDyn = TimeStepSys;
            state.dataSysRpts->AnyZoneTimeAboveVozDyn = TimeStepSys;
        } else if (totMechNatVentVolStdRho > HVAC::SmallAirVolFlow) {
            thisZoneVentRepVars.TimeAtVozDyn = TimeStepSys;
            state.dataSysRpts->AllZonesTimeAtVozDyn = TimeStepSys;
        }

        // determine volumetric values from mass flow using current air density for zone (adjusted for elevation)
        Real64 currentZoneAirDensity =
            Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                              state.dataEnvrn->OutBaroPress,
                                              state.dataZoneTempPredictorCorrector->zoneHeatBalance(CtrlZoneNum).MAT,
                                              state.dataZoneTempPredictorCorrector->zoneHeatBalance(CtrlZoneNum).airHumRatAvg);
        if (currentZoneAirDensity > 0.0) {
            thisZoneVentRepVars.OAVolFlowCrntRho = thisZoneVentRepVars.OAMassFlow / currentZoneAirDensity;
        }
        thisZoneVentRepVars.OAVolCrntRho = thisZoneVentRepVars.OAVolFlowCrntRho * TimeStepSysSec;
        if (ZoneVolume > 0.0) {
            thisZoneVentRepVars.MechACH = (thisZoneVentRepVars.OAVolCrntRho / TimeStepSys) / ZoneVolume;
        }

        // store data for predefined tabular report on outside air
        if (thisZonePredefRep.isOccupied) {
            // accumulate the occupied time
            thisZonePredefRep.TotTimeOcc += TimeStepSys;
            // mechanical ventilation
            thisZonePredefRep.MechVentVolTotalOcc += thisZoneVentRepVars.OAVolCrntRho;
            if ((thisZoneVentRepVars.OAVolCrntRho / TimeStepSys) < thisZonePredefRep.MechVentVolMin) {
                thisZonePredefRep.MechVentVolMin = thisZoneVentRepVars.OAVolCrntRho / TimeStepSys;
            }
            thisZonePredefRep.MechVentVolTotalOccStdDen += thisZoneVentRepVars.OAVolStdRho;
            // infiltration
            thisZonePredefRep.InfilVolTotalOcc += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).InfilVolumeCurDensity;
            if (state.dataHeatBal->ZnAirRpt(CtrlZoneNum).InfilVolumeCurDensity < thisZonePredefRep.InfilVolMin) {
                thisZonePredefRep.InfilVolMin = state.dataHeatBal->ZnAirRpt(CtrlZoneNum).InfilVolumeCurDensity;
            }
            thisZonePredefRep.InfilVolTotalOccStdDen += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).InfilVolumeStdDensity;
            // 'simple' natural ventilation
            thisZonePredefRep.SimpVentVolTotalOcc += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeCurDensity;
            if (state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeCurDensity < thisZonePredefRep.SimpVentVolMin) {
                thisZonePredefRep.SimpVentVolMin = state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeCurDensity;
            }
            thisZonePredefRep.SimpVentVolTotalOccStdDen += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeStdDensity;
            // target ventilation Voz-dyn
            state.dataSysRpts->AnyZoneTimeBelowVozDynOcc = state.dataSysRpts->AnyZoneTimeBelowVozDyn;
            state.dataSysRpts->AllZonesTimeAtVozDynOcc = state.dataSysRpts->AllZonesTimeAtVozDyn;
            state.dataSysRpts->AnyZoneTimeAboveVozDynOcc = state.dataSysRpts->AnyZoneTimeAboveVozDyn;
            thisZonePredefRep.VozTargetTotalOcc += targetVoz;

            // time mechanical+natural ventilation is below, at, or above target Voz-dyn
            thisZonePredefRep.VozTargetTimeBelowOcc += thisZoneVentRepVars.TimeBelowVozDyn;
            thisZonePredefRep.VozTargetTimeAtOcc += thisZoneVentRepVars.TimeAtVozDyn;
            thisZonePredefRep.VozTargetTimeAboveOcc += thisZoneVentRepVars.TimeAboveVozDyn;
        } else if (totMechNatVentVolStdRho > HVAC::SmallAirVolFlow) {
            thisZoneVentRepVars.TimeVentUnocc = TimeStepSys;
            state.dataSysRpts->AnyZoneTimeVentUnocc = TimeStepSys;
            thisZonePredefRep.TotVentTimeNonZeroUnocc += thisZoneVentRepVars.TimeVentUnocc;
        }
        // accumulate during occupancy or not
        thisZonePredefRep.MechVentVolTotalStdDen += thisZoneVentRepVars.OAVolStdRho;
        thisZonePredefRep.InfilVolTotalStdDen += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).InfilVolumeStdDensity;
        thisZonePredefRep.SimpVentVolTotalStdDen += state.dataHeatBal->ZnAirRpt(CtrlZoneNum).VentilVolumeStdDensity;
        thisZonePredefRep.VozTargetTotal += targetVoz;
        thisZonePredefRep.VozTargetTimeBelow += thisZoneVentRepVars.TimeBelowVozDyn;
        thisZonePredefRep.VozTargetTimeAt += thisZoneVentRepVars.TimeAtVozDyn;
        thisZonePredefRep.VozTargetTimeAbove += thisZoneVentRepVars.TimeAboveVozDyn;

        // now combine Vent load from zone forced air units with primary air system
        Real64 ZoneVentLoad = ZAirSysZoneVentLoad + ZFAUZoneVentLoad;
        // cycle if ZoneVentLoad is small
        if (std::abs(ZoneVentLoad) < SmallLoad) {
            continue; // orig. had RETURN here, BG changed to CYCLE for next controlled zone in do loop.
        }

        // Ventilation Heating
        if (ZoneVentLoad > SmallLoad) {
            // Zone cooling load
            if (ZoneLoad < -SmallLoad) {
                thisZoneVentRepVars.CoolingLoadAddedByVent += std::abs(ZoneVentLoad);
                // Zone heating load
            } else if (ZoneLoad > SmallLoad) {
                if (ZoneVentLoad > ZoneLoad) {
                    thisZoneVentRepVars.HeatingLoadMetByVent += std::abs(ZoneLoad);
                    thisZoneVentRepVars.OverheatingByVent += (ZoneVentLoad - ZoneLoad);
                } else {
                    thisZoneVentRepVars.HeatingLoadMetByVent += std::abs(ZoneVentLoad);
                }
                // No Zone Load
            } else {
                thisZoneVentRepVars.NoLoadHeatingByVent += std::abs(ZoneVentLoad);
            }

            // Ventilation Cooling
        } else if (ZoneVentLoad < -SmallLoad) {
            // Zone cooling load
            if (ZoneLoad < -SmallLoad) {
                if (ZoneVentLoad < ZoneLoad) {
                    thisZoneVentRepVars.CoolingLoadMetByVent += std::abs(ZoneLoad);
                    thisZoneVentRepVars.OvercoolingByVent += std::abs(ZoneVentLoad - ZoneLoad);
                } else {
                    thisZoneVentRepVars.CoolingLoadMetByVent += std::abs(ZoneVentLoad);
                }
                // Zone heating load
            } else if (ZoneLoad > SmallLoad) {
                thisZoneVentRepVars.HeatingLoadAddedByVent += std::abs(ZoneVentLoad);
                // No Zone Load
            } else {
                thisZoneVentRepVars.NoLoadCoolingByVent += std::abs(ZoneVentLoad);
            }

            // Ventilation No Load
        } else {
        }
    } // loop over controlled zones

    // loop over air loops
    for (int sysNum = 1; sysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++sysNum) {
        auto &thisSysVentRepVars = state.dataSysRpts->SysVentRepVars(sysNum);
        auto &thisSysPreDefRep = state.dataSysRpts->SysPreDefRep(sysNum);
        auto &sysCtrlInfo = state.dataAirLoop->AirLoopControlInfo(sysNum);
        auto &sysFlow = state.dataAirLoop->AirLoopFlow(sysNum);
        Real64 mechVentFlow = sysFlow.OAFlow / state.dataEnvrn->StdRhoAir; // [m3/s]
        thisSysVentRepVars.MechVentFlow = mechVentFlow;
        thisSysPreDefRep.MechVentTotal += mechVentFlow * TimeStepSysSec;
        thisSysPreDefRep.NatVentTotal += thisSysVentRepVars.NatVentFlow * TimeStepSysSec;

        // set time mechanical+natural ventilation is below, at, or above target Voz-dyn
        Real64 totMechNatVentVolFlowStdRho = mechVentFlow + thisSysVentRepVars.NatVentFlow;

        Real64 targetFlowVoz = thisSysVentRepVars.TargetVentilationFlowVoz;
        thisSysPreDefRep.TargetVentTotalVoz += targetFlowVoz * TimeStepSysSec;
        // Allow 1% tolerance
        if (totMechNatVentVolFlowStdRho < (0.99 * targetFlowVoz)) {
            thisSysVentRepVars.TimeBelowVozDyn = TimeStepSys;
            thisSysPreDefRep.TimeBelowVozDynTotal += TimeStepSys;
        } else if (totMechNatVentVolFlowStdRho > (1.01 * targetFlowVoz)) {
            thisSysVentRepVars.TimeAboveVozDyn = TimeStepSys;
            thisSysPreDefRep.TimeAboveVozDynTotal += TimeStepSys;
        } else if (totMechNatVentVolFlowStdRho > HVAC::SmallAirVolFlow) {
            thisSysVentRepVars.TimeAtVozDyn = TimeStepSys;
            thisSysPreDefRep.TimeAtVozDynTotal += TimeStepSys;
        }

        if (thisSysVentRepVars.AnyZoneOccupied) {
            thisSysPreDefRep.TimeOccupiedTotal += TimeStepSys;
            thisSysPreDefRep.MechVentTotalOcc += mechVentFlow * TimeStepSysSec;
            thisSysPreDefRep.NatVentTotalOcc += thisSysVentRepVars.NatVentFlow * TimeStepSysSec;
            thisSysPreDefRep.TargetVentTotalVozOcc += targetFlowVoz * TimeStepSysSec;
            thisSysPreDefRep.TimeBelowVozDynTotalOcc += thisSysVentRepVars.TimeBelowVozDyn;
            thisSysPreDefRep.TimeAboveVozDynTotalOcc += thisSysVentRepVars.TimeAboveVozDyn;
            thisSysPreDefRep.TimeAtVozDynTotalOcc += thisSysVentRepVars.TimeAtVozDyn;
            if (sysFlow.SupFlow > HVAC::SmallMassFlow) {
                if (sysCtrlInfo.fanOp == HVAC::FanOp::Continuous) {
                    thisSysPreDefRep.TimeFanContTotalOcc += TimeStepSys;
                } else {
                    thisSysPreDefRep.TimeFanCycTotalOcc += TimeStepSys;
                }
            } else {
                thisSysPreDefRep.TimeFanOffTotalOcc += TimeStepSys;
            }
        } else {
            if (totMechNatVentVolFlowStdRho > HVAC::SmallAirVolFlow) {
                thisSysVentRepVars.TimeVentUnocc = TimeStepSys;
                thisSysPreDefRep.TimeVentUnoccTotal += TimeStepSys;
            }
            thisSysPreDefRep.TimeUnoccupiedTotal += TimeStepSys;
            if (sysFlow.SupFlow > HVAC::SmallMassFlow) {
                if (sysCtrlInfo.fanOp == HVAC::FanOp::Continuous) {
                    thisSysPreDefRep.TimeFanContTotalUnocc += TimeStepSys;
                } else {
                    thisSysPreDefRep.TimeFanCycTotalUnocc += TimeStepSys;
                }
            } else {
                thisSysPreDefRep.TimeFanOffTotalUnocc += TimeStepSys;
            }
        }

        // set time at OA limiting factors
        if (mechVentFlow > HVAC::SmallAirVolFlow) {
            int thisOAControlNum = state.dataAirLoop->AirLoopControlInfo(sysNum).OACtrlNum;
            if (thisOAControlNum > 0) {
                int limitFactorIndex = static_cast<int>(state.dataMixedAir->OAController(thisOAControlNum).OALimitingFactor);
                thisSysPreDefRep.TimeAtOALimit[limitFactorIndex] += TimeStepSys;
                if (thisSysVentRepVars.AnyZoneOccupied) {
                    thisSysPreDefRep.TimeAtOALimitOcc[limitFactorIndex] += TimeStepSys;
                    thisSysPreDefRep.MechVentTotAtLimitOcc[limitFactorIndex] += mechVentFlow * TimeStepSysSec;
                }
            }
        }
    }
    // Accumulate facility totals
    state.dataOutRptPredefined->TotalAnyZoneBelowVozDynForOA += state.dataSysRpts->AnyZoneTimeBelowVozDyn;
    state.dataOutRptPredefined->TotalAllZonesAtVozDynForOA += state.dataSysRpts->AllZonesTimeAtVozDyn;
    state.dataOutRptPredefined->TotalAnyZoneAboveVozDynForOA += state.dataSysRpts->AnyZoneTimeAboveVozDyn;
    state.dataOutRptPredefined->TotalAnyZoneVentUnoccForOA += state.dataSysRpts->AnyZoneTimeVentUnocc;
    state.dataOutRptPredefined->TotalAnyZoneBelowVozDynOccForOA += state.dataSysRpts->AnyZoneTimeBelowVozDynOcc;
    state.dataOutRptPredefined->TotalAllZonesAtVozDynOccForOA += state.dataSysRpts->AllZonesTimeAtVozDynOcc;
    state.dataOutRptPredefined->TotalAnyZoneAboveVozDynOccForOA += state.dataSysRpts->AnyZoneTimeAboveVozDynOcc;
}

void MatchPlantSys(EnergyPlusData &state,
                   int const AirLoopNum, // counter for zone air distribution inlets
                   int const BranchNum   // counter for zone air distribution inlets
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS SUBROUTINE:
    // calculate and report zone ventilation loads

    // METHODOLOGY EMPLOYED:
    // calculate energy contribution of outside air through mixing box and pro-rate to
    // zones according to zone mass flow rates.

    int constexpr EnergyTrans(1);

    for (int CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
        {
            auto &thisComp = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum);
            for (int VarNum = 1; VarNum <= thisComp.NumMeteredVars; ++VarNum) {
                if (thisComp.MeteredVar(VarNum).resource == Constant::eResource::EnergyTransfer) {
                    thisComp.EnergyTransComp = EnergyTrans;
                    const std::string &CompType = thisComp.TypeOf;
                    const std::string &CompName = thisComp.Name;
                    bool MatchFound = false; // Set to .TRUE. when a match is found
                    int MatchLoop = 0;       // Loop number of the match
                    int MatchBranch = 0;     // Branch number of the match
                    int MatchComp = 0;       // Component number of the match
                    int MatchLoopType = 0;
                    int Idx = 0;
                    FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                    if (MatchFound) {
                        UpdateAirSysCompPtrArray(state, Idx, AirLoopNum, BranchNum, CompNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                    }
                    thisComp.AirSysToPlantPtr = Idx;
                    break;
                }
            }
            for (int SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                //!!!!          IF(SysVentLoad == 0.0d0)EXIT
                {
                    auto &thisSubComp(thisComp.SubComp(SubCompNum));
                    for (int VarNum = 1; VarNum <= thisSubComp.NumMeteredVars; ++VarNum) {
                        if (thisSubComp.MeteredVar(VarNum).resource == Constant::eResource::EnergyTransfer) {
                            thisSubComp.EnergyTransComp = EnergyTrans;
                            const std::string &CompType = thisComp.TypeOf;
                            const std::string &CompName = thisComp.Name;
                            bool MatchFound = false; // Set to .TRUE. when a match is found
                            int MatchLoop = 0;       // Loop number of the match
                            int MatchBranch = 0;     // Branch number of the match
                            int MatchComp = 0;       // Component number of the match
                            int MatchLoopType = 0;
                            int Idx = 0;
                            FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                            if (MatchFound) {
                                UpdateAirSysSubCompPtrArray(
                                    state, Idx, AirLoopNum, BranchNum, CompNum, SubCompNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                            }
                            thisSubComp.AirSysToPlantPtr = Idx;
                            break;
                        }
                    }
                    for (int SubSubCompNum = 1; SubSubCompNum <= thisSubComp.NumSubSubComps; ++SubSubCompNum) {
                        //!!!!            IF(SysVentLoad == 0.0d0)EXIT
                        {
                            auto &thisSubSubComp = thisSubComp.SubSubComp(SubSubCompNum);
                            for (int VarNum = 1; VarNum <= thisSubSubComp.NumMeteredVars; ++VarNum) {
                                if (thisSubSubComp.MeteredVar(VarNum).resource == Constant::eResource::EnergyTransfer) {
                                    thisSubSubComp.EnergyTransComp = EnergyTrans;
                                    const std::string &CompType = thisComp.TypeOf;
                                    const std::string &CompName = thisComp.Name;
                                    bool MatchFound = false; // Set to .TRUE. when a match is found
                                    int MatchLoop = 0;       // Loop number of the match
                                    int MatchBranch = 0;     // Branch number of the match
                                    int MatchComp = 0;       // Component number of the match
                                    int MatchLoopType = 0;
                                    int Idx = 0;
                                    FindDemandSideMatch(state, CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    if (MatchFound) {
                                        UpdateAirSysSubSubCompPtrArray(state,
                                                                       Idx,
                                                                       AirLoopNum,
                                                                       BranchNum,
                                                                       CompNum,
                                                                       SubCompNum,
                                                                       SubSubCompNum,
                                                                       MatchLoopType,
                                                                       MatchLoop,
                                                                       MatchBranch,
                                                                       MatchComp);
                                    }
                                    thisSubSubComp.AirSysToPlantPtr = Idx;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void FindDemandSideMatch(EnergyPlusData &state,
                         std::string const &CompType, // Inlet node of the component to find the match of
                         std::string_view CompName,   // Outlet node of the component to find the match of
                         bool &MatchFound,            // Set to .TRUE. when a match is found
                         int &MatchLoopType,          // Loop number of the match
                         int &MatchLoop,              // Loop number of the match
                         int &MatchBranch,            // Branch number of the match
                         int &MatchComp               // Component number of the match
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   September 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the connections between various loops.
    // Due to the fact that this requires numerous string compares, it
    // is much more efficient to find this information once and then
    // store it in module level variables (LoopConnect derived type).

    // METHODOLOGY EMPLOYED:
    // Simply cycles through the plant and condenser demand sides until
    // a component is found that matches the component type and name

    // Initialize all of the output variables

    MatchFound = false;
    MatchLoopType = 0;
    MatchLoop = 0;
    MatchLoop = 0;
    MatchBranch = 0;
    MatchComp = 0;

    // Now cycle through all of the demand side loops to see if we can find
    // a match for the component type and name.  Once a match is found,
    // record the type of loop and the loop, branch, and component numbers.
    if (!MatchFound) { // Go through the plant demand side loops
        for (int PassLoopNum = 1; PassLoopNum <= state.dataHVACGlobal->NumPlantLoops; ++PassLoopNum) {
            for (int PassBranchNum = 1;
                 PassBranchNum <= state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum).TotalBranches;
                 ++PassBranchNum) {
                for (int PassCompNum = 1;
                     PassCompNum <=
                     state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum).Branch(PassBranchNum).TotalComponents;
                     ++PassCompNum) {
                    if (Util::SameString(CompType,
                                         state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum)
                                             .Branch(PassBranchNum)
                                             .Comp(PassCompNum)
                                             .TypeOf) &&
                        Util::SameString(CompName,
                                         state.dataPlnt->VentRepPlant[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum)
                                             .Branch(PassBranchNum)
                                             .Comp(PassCompNum)
                                             .Name)) {
                        // Found a match on the plant demand side--increment the counter
                        MatchFound = true;
                        MatchLoopType = 1;
                        MatchLoop = PassLoopNum;
                        MatchBranch = PassBranchNum;
                        MatchComp = PassCompNum;
                        break; // PassCompNum DO loop
                    }
                }
                if (MatchFound) {
                    break; // PassBranchNum DO loop
                }
            }
            if (MatchFound) {
                break; // PassLoopNum DO loop
            }
        }
    }

    if (!MatchFound) { // Go through the condenser demand side loops
        for (int PassLoopNum = 1; PassLoopNum <= state.dataHVACGlobal->NumCondLoops; ++PassLoopNum) {
            for (int PassBranchNum = 1;
                 PassBranchNum <= state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum).TotalBranches;
                 ++PassBranchNum) {
                for (int PassCompNum = 1;
                     PassCompNum <=
                     state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum).Branch(PassBranchNum).TotalComponents;
                     ++PassCompNum) {
                    if (Util::SameString(CompType,
                                         state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum)
                                             .Branch(PassBranchNum)
                                             .Comp(PassCompNum)
                                             .TypeOf) &&
                        Util::SameString(CompName,
                                         state.dataPlnt->VentRepCond[static_cast<int>(LoopSideLocation::Demand)](PassLoopNum)
                                             .Branch(PassBranchNum)
                                             .Comp(PassCompNum)
                                             .Name)) {
                        // Found a match on the plant demand side--increment the counter
                        MatchFound = true;
                        MatchLoopType = 2;
                        MatchLoop = PassLoopNum;
                        MatchBranch = PassBranchNum;
                        MatchComp = PassCompNum;
                        break; // PassCompNum DO loop
                    }
                }
                if (MatchFound) {
                    break; // PassBranchNum DO loop
                }
            }
            if (MatchFound) {
                break; // PassLoopNum DO loop
            }
        }
    }
}

void ReportAirLoopConnections(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael J. Witte, Linda K. Lawrie
    //       DATE WRITTEN   February 2004 (moved from BranchInputManager ReportLoopConnections)

    // PURPOSE OF THIS SUBROUTINE:
    // Report air loop splitter connections to the BND file.

    static constexpr std::string_view errstring("**error**");

    static constexpr std::string_view Format_706("! <#AirLoopHVACs>,<Number of AirLoopHVACs>");
    static constexpr std::string_view Format_708(
        "! <AirLoopHVAC>,<Air Loop Name>,<# Return Nodes>,<# Supply Nodes>,<# Zones Cooled>,<# Zones Heated>,<Outdoor Air Used>");
    static constexpr std::string_view Format_709(
        "! <AirLoop Return Connections>,<Connection Count>,<AirLoopHVAC Name>,<Zn Eqp Return Node #>,<Zn Eqp Return "
        "Node Name>,<AirLoop Return Node #>,<Air Loop Return Node Name>");
    static constexpr std::string_view Format_710(
        "! <AirLoop Supply Connections>,<Connection Count>,<AirLoopHVAC Name>,<Zn Eqp Supply Node #>,<Zn Eqp Supply "
        "Node Name>,<AirLoop Supply Node #>,<Air Loop Supply Node Name>");
    static constexpr std::string_view Format_711(
        "! <Cooled Zone Info>,<Cooled Zone Count>,<Cooled Zone Name>,<Cooled Zone Inlet Node #>,<Cooled Zone Inlet "
        "Node Name>,<AirLoopHVAC Name>");
    static constexpr std::string_view Format_712(
        "! <Heated Zone Info>,<Heated Zone Count>,<Heated Zone Name>,<Heated Zone Inlet Node #>,<Heated Zone Inlet "
        "Node Name>,<AirLoopHVAC Name>");
    static constexpr std::string_view Format_714(
        "! <Outdoor Air Connections>,<OA Inlet Node #>,<OA Return Air Inlet Node Name>,<OA Outlet Node #>,<OA Mixed "
        "Air Outlet Node Name>,<AirLoopHVAC Name>");

    auto const &NodeID = state.dataLoopNodes->NodeID;

    print(state.files.bnd, "{}\n", "! ===============================================================");
    print(state.files.bnd, "{}\n", Format_706);
    print(state.files.bnd, " #AirLoopHVACs,{}\n", state.dataHVACGlobal->NumPrimaryAirSys);
    print(state.files.bnd, "{}\n", Format_708);
    print(state.files.bnd, "{}\n", Format_709);
    print(state.files.bnd, "{}\n", Format_710);
    print(state.files.bnd, "{}\n", Format_711);
    print(state.files.bnd, "{}\n", Format_712);
    print(state.files.bnd, "{}\n", Format_714);
    print(state.files.bnd, "{}\n", "! <AirLoopHVAC Connector>,<Connector Type>,<Connector Name>,<Loop Name>,<Loop Type>,<Number of Inlets/Outlets>");
    print(state.files.bnd,
          "{}\n",
          "! <AirLoopHVAC Connector Branches>,<Connector Node Count>,<Connector Type>,<Connector Name>,<Inlet Branch>,<Outlet Branch>,<Loop "
          "Name>,<Loop Type>");
    print(state.files.bnd,
          "{}\n",
          "! <AirLoopHVAC Connector Nodes>,<Connector Node Count>,<Connector Type>,<Connector Name>,<Inlet Node>,<Outlet Node>,<Loop Name>,<Loop "
          "Type>");
    for (int Count = 1; Count <= state.dataHVACGlobal->NumPrimaryAirSys; ++Count) {
        const auto oaSysExists = [&]() {
            if (state.dataAirLoop->AirToOANodeInfo(Count).OASysExists) {
                return "Yes";
            } else {
                return "No";
            }
        }();

        print(state.files.bnd,
              " AirLoopHVAC,{},{},{},{},{},{}\n",
              state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName,
              state.dataAirLoop->AirToZoneNodeInfo(Count).NumReturnNodes,
              state.dataAirLoop->AirToZoneNodeInfo(Count).NumSupplyNodes,
              state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesCooled,
              state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesHeated,
              oaSysExists);
        for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumReturnNodes; ++Count1) {
            print(state.files.bnd, "   AirLoop Return Connections,{},{},", Count1, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1) > 0) {
                print(state.files.bnd,
                      "{},{},",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1)));
            } else {
                print(state.files.bnd, "{},{},", errstring, errstring);
            }
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1) > 0) {
                print(state.files.bnd,
                      "{},{}\n",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1)));
            } else {
                print(state.files.bnd, "{},{}\n", errstring, errstring);
            }
        }
        for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumSupplyNodes; ++Count1) {
            print(state.files.bnd, "   AirLoop Supply Connections,{},{},", Count1, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1) > 0) {
                print(state.files.bnd,
                      "{},{},",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1)));
            } else {
                print(state.files.bnd, "{},{},", errstring, errstring);
            }
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1) > 0) {
                print(state.files.bnd,
                      "{},{}\n",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1)));
            } else {
                print(state.files.bnd, "{},{}\n", errstring, errstring);
            }
        }

        for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesCooled; ++Count1) {
            const int CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(Count).CoolCtrlZoneNums(Count1);
            print(state.files.bnd, "   Cooled Zone Info,{},{},", Count1, state.dataHeatBal->Zone(CtrldZoneNum).Name);
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1) > 0) {
                print(state.files.bnd,
                      "{},{},{}\n",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1)),
                      state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            } else {
                print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            }
        }
        for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesHeated; ++Count1) {
            const int CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(Count).HeatCtrlZoneNums(Count1);
            print(state.files.bnd, "   Heated Zone Info,{},{},", Count1, state.dataHeatBal->Zone(CtrldZoneNum).Name);
            if (state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1) > 0) {
                print(state.files.bnd,
                      "{},{},{}\n",
                      state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1),
                      NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1)),
                      state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            } else {
                print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            }
        }
        if (state.dataAirLoop->AirToOANodeInfo(Count).OASysExists) {
            std::string ChrOut;
            std::string ChrOut2;
            if (state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum > 0) {
                ChrOut = fmt::to_string(state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum);
            } else {
                ChrOut = errstring;
            }
            if (state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum > 0) {
                ChrOut2 = fmt::to_string(state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum);
            } else {
                ChrOut2 = errstring;
            }

            print(state.files.bnd, "   Outdoor Air Connections,{},", ChrOut);
            if (ChrOut != errstring) {
                print(state.files.bnd, "{},", NodeID(state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum));
            } else {
                print(state.files.bnd, "{},", errstring);
            }
            if (ChrOut2 != errstring) {
                print(state.files.bnd,
                      "{},{},{}\n",
                      ChrOut2,
                      NodeID(state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum),
                      state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            } else {
                print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
            }
        }
        //  Report HVAC Air Loop Splitter to BND file
        if (state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.Exists) {
            print(state.files.bnd,
                  "   AirLoopHVAC Connector,Splitter,{},{},Air,{}\n",
                  state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.Name,
                  state.dataAirSystemsData->PrimaryAirSystems(Count).Name,
                  state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.TotalOutletNodes);
            for (int Count1 = 1; Count1 <= state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.TotalOutletNodes; ++Count1) {
                print(state.files.bnd,
                      "     AirLoopHVAC Connector Branches,{},Splitter,{},",
                      Count1,
                      state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.Name);

                if (state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.BranchNumIn <= 0) {
                    print(state.files.bnd, "{},", errstring);
                } else {
                    print(state.files.bnd,
                          "{},",
                          state.dataAirSystemsData->PrimaryAirSystems(Count)
                              .Branch(state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.BranchNumIn)
                              .Name);
                }

                if (state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.BranchNumOut(Count1) <= 0) {
                    print(state.files.bnd, "{},{},Air\n", errstring, state.dataAirSystemsData->PrimaryAirSystems(Count).Name);
                } else {
                    print(state.files.bnd,
                          "{},{},Air\n",
                          state.dataAirSystemsData->PrimaryAirSystems(Count)
                              .Branch(state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.BranchNumOut(Count1))
                              .Name,
                          state.dataAirSystemsData->PrimaryAirSystems(Count).Name);
                }

                print(state.files.bnd,
                      "     AirLoopHVAC Connector Nodes,   {},Splitter,{},{},{},{},Air\n",
                      Count1,
                      state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.Name,
                      state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.NodeNameIn,
                      state.dataAirSystemsData->PrimaryAirSystems(Count).Splitter.NodeNameOut(Count1),
                      state.dataAirSystemsData->PrimaryAirSystems(Count).Name);
            }
        }
    }
}

void reportAirLoopToplogy(EnergyPlusData &state)
{
    // s->pdstTopAirLoop = newPreDefSubTable(state, s->pdrTopology, "Air Loop Supply Side Component Arrangement");
    // s->pdchTopAirLoopName = newPreDefColumn(state, s->pdstTopAirLoop, "Airloop Name");
    // s->pdchTopAirSplitName = newPreDefColumn(state, s->pdstTopAirLoop, "Splitter Name");
    // s->pdchTopAirBranchName = newPreDefColumn(state, s->pdstTopAirLoop, "Supply Branch Name");
    // s->pdchTopAirSupplyBranchType = newPreDefColumn(state, s->pdstTopAirLoop, "Supply Branch Type");
    // s->pdchTopAirCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Component Type");
    // s->pdchTopAirCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Component Name");
    // s->pdchTopAirSubCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Component Type");
    // s->pdchTopAirSubCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Component Name");
    // s->pdchTopAirSubSubCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Sub-Component Type");
    // s->pdchTopAirSubSubCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Sub-Component Name");
    // s->pdchTopAirMixName = newPreDefColumn(state, s->pdstTopAirLoop, "Mixer Name");

    auto &orp = state.dataOutRptPredefined;
    int rowCounter = 1;
    for (int airLoopNum = 1; airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++airLoopNum) {
        auto &pas = state.dataAirSystemsData->PrimaryAirSystems(airLoopNum);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirLoopName, format("{}", rowCounter), pas.Name);
        ++rowCounter;
        for (int BranchNum = 1; BranchNum <= pas.NumBranches; ++BranchNum) {
            auto &pasBranch = pas.Branch(BranchNum);
            if (pas.Splitter.Exists) {
                for (int outNum : pas.Splitter.BranchNumOut) {
                    if (outNum == BranchNum) {
                        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSplitName, format("{}", rowCounter), pas.Splitter.Name);
                        break;
                    }
                }
            }
            for (int CompNum = 1; CompNum <= pasBranch.TotalComponents; ++CompNum) {
                auto &pasBranchComp = pasBranch.Comp(CompNum);
                fillAirloopToplogyComponentRow(
                    state, pas.Name, pasBranch.Name, pasBranch.DuctType, pasBranchComp.TypeOf, pasBranchComp.Name, rowCounter);
                for (int SubCompNum = 1; SubCompNum <= pasBranchComp.NumSubComps; ++SubCompNum) {
                    auto &pasBranchSubComp = pasBranchComp.SubComp(SubCompNum);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSubCompType, format("{}", rowCounter), pasBranchSubComp.TypeOf);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSubCompName, format("{}", rowCounter), pasBranchSubComp.Name);
                    fillAirloopToplogyComponentRow(
                        state, pas.Name, pasBranch.Name, pasBranch.DuctType, pasBranchComp.TypeOf, pasBranchComp.Name, rowCounter);
                    for (int SubSubCompNum = 1; SubSubCompNum <= pasBranchSubComp.NumSubSubComps; ++SubSubCompNum) {
                        auto &pasBranchSubSubComp = pasBranchSubComp.SubSubComp(SubSubCompNum);
                        OutputReportPredefined::PreDefTableEntry(
                            state, orp->pdchTopAirSubCompType, format("{}", rowCounter), pasBranchSubComp.TypeOf);
                        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSubCompName, format("{}", rowCounter), pasBranchSubComp.Name);
                        OutputReportPredefined::PreDefTableEntry(
                            state, orp->pdchTopAirSubSubCompType, format("{}", rowCounter), pasBranchSubSubComp.TypeOf);
                        OutputReportPredefined::PreDefTableEntry(
                            state, orp->pdchTopAirSubSubCompName, format("{}", rowCounter), pasBranchSubSubComp.Name);
                        fillAirloopToplogyComponentRow(
                            state, pas.Name, pasBranch.Name, pasBranch.DuctType, pasBranchComp.TypeOf, pasBranchComp.Name, rowCounter);
                    }
                }
            }
            if (pas.Mixer.Exists) {
                for (int inNum : pas.Mixer.BranchNumIn) {
                    if (inNum == BranchNum) {
                        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirMixName, format("{}", rowCounter - 1), pas.Mixer.Name);
                        break;
                    }
                }
            }
        }
    }

    // s->pdstTopAirDemand = newPreDefSubTable(state, s->pdrTopology, "Air Loop Demand Side Component Arrangement");
    // s->pdchTopAirDemandName = newPreDefColumn(state, s->pdstTopAirDemand, "Airloop Name");
    // s->pdchTopAirSupplyBranchName = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Branch Name");
    // s->pdchTopAirSupplyPCompType = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Path Component Type");
    // s->pdchTopAirSupplyPCompName = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Path Component Name");
    // s->pdchTopAirZoneName = newPreDefColumn(state, s->pdstTopAirDemand, "Zone Name");
    // s->pdchTopAirTermUnitType = newPreDefColumn(state, s->pdstTopAirDemand, "Terminal Unit Type");
    // s->pdchTopAirTermUnitName = newPreDefColumn(state, s->pdstTopAirDemand, "Terminal Unit Name");
    // s->pdchTopAirReturnPCompType = newPreDefColumn(state, s->pdstTopAirDemand, "Return Path Component Type");
    // s->pdchTopAirReturnPCompName = newPreDefColumn(state, s->pdstTopAirDemand, "Return Path Component Name");

    rowCounter = 1;
    for (int airLoopNum = 1; airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++airLoopNum) {
        auto &pas = state.dataAirSystemsData->PrimaryAirSystems(airLoopNum);
        auto &thisAtoZInfo = state.dataAirLoop->AirToZoneNodeInfo(airLoopNum);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
        ++rowCounter;
        for (int ductNum = 1; ductNum <= thisAtoZInfo.NumSupplyNodes; ++ductNum) {
            auto &thisBranch = pas.Branch(thisAtoZInfo.SupplyDuctBranchNum(ductNum));
            if (thisAtoZInfo.SupplyAirPathNum(ductNum) > 0) {
                auto &thisSupplyPath = state.dataZoneEquip->SupplyAirPath(thisAtoZInfo.SupplyAirPathNum(ductNum));
                for (int compNum = 1; compNum <= thisSupplyPath.NumOfComponents; ++compNum) {
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSupplyBranchName, format("{}", rowCounter), thisBranch.Name);
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopAirSupplyDuctType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)thisBranch.DuctType]);
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopAirSupplyPCompType, format("{}", rowCounter), thisSupplyPath.ComponentType(compNum));
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopAirSupplyPCompName, format("{}", rowCounter), thisSupplyPath.ComponentName(compNum));
                    ++rowCounter;
                }
                if (thisBranch.DuctType == HVAC::AirDuctType::Cooling || thisBranch.DuctType == HVAC::AirDuctType::Main) {
                    for (int zoneNum : thisAtoZInfo.CoolCtrlZoneNums) {
                        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
                        auto &zel = state.dataZoneEquip->ZoneEquipList(zoneNum);
                        for (auto &thisCoolADU : thisZoneEquipConfig.AirDistUnitCool) {
                            if (thisCoolADU.AirLoopNum != airLoopNum) {
                                continue;
                            }
                            if (thisCoolADU.SupplyBranchIndex != thisAtoZInfo.SupplyDuctBranchNum(ductNum)) {
                                continue;
                            }
                            if (thisCoolADU.SupplyAirPathExists) {
                                int spCompNum = thisSupplyPath.OutletNodeSupplyPathCompNum(thisCoolADU.SupplyAirPathOutNodeIndex);
                                OutputReportPredefined::PreDefTableEntry(
                                    state, orp->pdchTopAirSupplyPCompType, format("{}", rowCounter), thisSupplyPath.ComponentType(spCompNum));
                                OutputReportPredefined::PreDefTableEntry(
                                    state, orp->pdchTopAirSupplyPCompName, format("{}", rowCounter), thisSupplyPath.ComponentName(spCompNum));
                            }
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirSupplyBranchName, format("{}", rowCounter), thisBranch.Name);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirSupplyDuctType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)thisBranch.DuctType]);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirZoneName, format("{}", rowCounter), thisZoneEquipConfig.ZoneName);
                            auto &aduIndex = zel.EquipIndex(thisCoolADU.AirDistUnitIndex);
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     orp->pdchTopAirTermUnitType,
                                                                     format("{}", rowCounter),
                                                                     state.dataDefineEquipment->AirDistUnit(aduIndex).EquipType(1));
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     orp->pdchTopAirTermUnitName,
                                                                     format("{}", rowCounter),
                                                                     state.dataDefineEquipment->AirDistUnit(aduIndex).EquipName(1));
                            if (thisAtoZInfo.ReturnAirPathNum(1) > 0) {
                                auto &thisReturnPath = state.dataZoneEquip->ReturnAirPath(thisAtoZInfo.ReturnAirPathNum(1));
                                for (int retNodeNum = 1; retNodeNum <= thisZoneEquipConfig.NumReturnNodes; ++retNodeNum) {
                                    if (thisZoneEquipConfig.ReturnNodeAirLoopNum(retNodeNum) == airLoopNum) {
                                        int retPathCompNum = thisZoneEquipConfig.ReturnNodeRetPathCompNum(retNodeNum);
                                        if (retPathCompNum > 0) {
                                            OutputReportPredefined::PreDefTableEntry(state,
                                                                                     orp->pdchTopAirReturnPCompType,
                                                                                     format("{}", rowCounter),
                                                                                     thisReturnPath.ComponentType(retPathCompNum));
                                            OutputReportPredefined::PreDefTableEntry(state,
                                                                                     orp->pdchTopAirReturnPCompName,
                                                                                     format("{}", rowCounter),
                                                                                     thisReturnPath.ComponentName(retPathCompNum));
                                        }
                                        break;
                                    }
                                }
                            }
                            ++rowCounter;
                        }
                    }
                } else if (thisBranch.DuctType == HVAC::AirDuctType::Heating) {
                    for (int zoneNum : thisAtoZInfo.HeatCtrlZoneNums) {
                        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
                        auto &zel = state.dataZoneEquip->ZoneEquipList(zoneNum);
                        for (auto &thisHeatADU : thisZoneEquipConfig.AirDistUnitHeat) {
                            if (thisHeatADU.AirLoopNum != airLoopNum) {
                                continue;
                            }
                            if (thisHeatADU.SupplyBranchIndex != thisAtoZInfo.SupplyDuctBranchNum(ductNum)) {
                                continue;
                            }
                            if (thisHeatADU.SupplyAirPathExists) {
                                int spCompNum = thisSupplyPath.OutletNodeSupplyPathCompNum(thisHeatADU.SupplyAirPathOutNodeIndex);
                                OutputReportPredefined::PreDefTableEntry(
                                    state, orp->pdchTopAirSupplyPCompType, format("{}", rowCounter), thisSupplyPath.ComponentType(spCompNum));
                                OutputReportPredefined::PreDefTableEntry(
                                    state, orp->pdchTopAirSupplyPCompName, format("{}", rowCounter), thisSupplyPath.ComponentName(spCompNum));
                            }
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirSupplyBranchName, format("{}", rowCounter), thisBranch.Name);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirSupplyDuctType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)thisBranch.DuctType]);
                            OutputReportPredefined::PreDefTableEntry(
                                state, orp->pdchTopAirZoneName, format("{}", rowCounter), thisZoneEquipConfig.ZoneName);
                            auto &aduIndex = zel.EquipIndex(thisHeatADU.AirDistUnitIndex);
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     orp->pdchTopAirTermUnitType,
                                                                     format("{}", rowCounter),
                                                                     state.dataDefineEquipment->AirDistUnit(aduIndex).EquipType(1));
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     orp->pdchTopAirTermUnitName,
                                                                     format("{}", rowCounter),
                                                                     state.dataDefineEquipment->AirDistUnit(aduIndex).EquipName(1));
                            if (thisAtoZInfo.ReturnAirPathNum(1) > 0) {
                                auto &thisReturnPath = state.dataZoneEquip->ReturnAirPath(thisAtoZInfo.ReturnAirPathNum(1));
                                for (int retNodeNum = 1; retNodeNum <= thisZoneEquipConfig.NumReturnNodes; ++retNodeNum) {
                                    int retPathCompNum = thisZoneEquipConfig.ReturnNodeRetPathCompNum(retNodeNum);
                                    if (retPathCompNum > 0) {
                                        OutputReportPredefined::PreDefTableEntry(state,
                                                                                 orp->pdchTopAirReturnPCompType,
                                                                                 format("{}", rowCounter),
                                                                                 thisReturnPath.ComponentType(retPathCompNum));
                                        OutputReportPredefined::PreDefTableEntry(state,
                                                                                 orp->pdchTopAirReturnPCompName,
                                                                                 format("{}", rowCounter),
                                                                                 thisReturnPath.ComponentName(retPathCompNum));
                                    }
                                }
                            }
                            ++rowCounter;
                        }
                    }
                }

            } else {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSupplyBranchName, format("{}", rowCounter), thisBranch.Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchTopAirSupplyDuctType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)thisBranch.DuctType]);
                ++rowCounter;
            }
        }
        if (thisAtoZInfo.ReturnAirPathNum(1) > 0) {
            auto &thisReturnPath = state.dataZoneEquip->ReturnAirPath(thisAtoZInfo.ReturnAirPathNum(1));
            for (int compNum = 1; compNum <= thisReturnPath.NumOfComponents; ++compNum) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirDemandName, format("{}", rowCounter), thisAtoZInfo.AirLoopName);
                if (compNum == thisReturnPath.OutletRetPathCompNum) {
                    auto &thisBranch = pas.Branch(pas.InletBranchNum[0]);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSupplyBranchName, format("{}", rowCounter), thisBranch.Name);
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopAirSupplyDuctType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)thisBranch.DuctType]);
                }
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchTopAirReturnPCompType, format("{}", rowCounter), thisReturnPath.ComponentType(compNum));
                OutputReportPredefined::PreDefTableEntry(
                    state, orp->pdchTopAirReturnPCompName, format("{}", rowCounter), thisReturnPath.ComponentName(compNum));
                ++rowCounter;
            }
        }
    }
}

void fillAirloopToplogyComponentRow(EnergyPlusData &state,
                                    const std::string_view &loopName,
                                    const std::string_view &branchName,
                                    const HVAC::AirDuctType ductType,
                                    const std::string_view &compType,
                                    const std::string_view &compName,
                                    int &rowCounter)
{
    auto &orp = state.dataOutRptPredefined;
    // s->pdchTopAirLoopName = newPreDefColumn(state, s->pdstTopAirLoop, "Airloop Name");
    // s->pdchTopAirBranchName = newPreDefColumn(state, s->pdstTopAirLoop, "Branch Name");
    // s->pdchTopAirCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Component Type");
    // s->pdchTopAirCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Component Name");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirLoopName, format("{}", rowCounter), loopName);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirBranchName, format("{}", rowCounter), branchName);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirSupplyBranchType, format("{}", rowCounter), HVAC::airDuctTypeNames[(int)ductType]);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirCompType, format("{}", rowCounter), compType);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopAirCompName, format("{}", rowCounter), compName);
    ++rowCounter;
}

void reportZoneEquipmentToplogy(EnergyPlusData &state)
{
    // s->pdstTopZnEqp = newPreDefSubTable(state, s->pdrTopology, "Zone Equipment Component Arrangement");
    // s->pdchTopZnEqpName = newPreDefColumn(state, s->pdstTopZnEqp, "Zone Name");
    // s->pdchTopZnEqpCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Component Type");
    // s->pdchTopZnEqpCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Component Name");
    // s->pdchTopZnEqpSubCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Component Type");
    // s->pdchTopZnEqpSubCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Component Name");
    // s->pdchTopZnEqpSubSubCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Sub-Component Type");
    // s->pdchTopZnEqpSubSubCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Sub-Component Name");

    auto &orp = state.dataOutRptPredefined;
    int rowCounter = 1;
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        const std::string_view zoneName = state.dataHeatBal->Zone(zoneNum).Name;
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpName, format("{}", rowCounter), zoneName);
        ++rowCounter;
        if (!state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            continue;
        }
        auto &zel = state.dataZoneEquip->ZoneEquipList(zoneNum);
        for (int CompNum = 1; CompNum <= zel.NumOfEquipTypes; ++CompNum) {
            auto &zelEquipData = zel.EquipData(CompNum);
            fillZoneEquipToplogyComponentRow(state, zoneName, zelEquipData.TypeOf, zelEquipData.Name, rowCounter);
            for (int SubCompNum = 1; SubCompNum <= zelEquipData.NumSubEquip; ++SubCompNum) {
                auto &zelSubEquipData = zelEquipData.SubEquipData(SubCompNum);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpSubCompType, format("{}", rowCounter), zelSubEquipData.TypeOf);
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpSubCompName, format("{}", rowCounter), zelSubEquipData.Name);
                fillZoneEquipToplogyComponentRow(state, zoneName, zelEquipData.TypeOf, zelEquipData.Name, rowCounter);
                for (int SubSubCompNum = 1; SubSubCompNum <= zelSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                    auto &zelSubSubEquipData = zelSubEquipData.SubSubEquipData(SubSubCompNum);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpSubCompType, format("{}", rowCounter), zelSubEquipData.TypeOf);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpSubCompName, format("{}", rowCounter), zelSubEquipData.Name);
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopZnEqpSubSubCompType, format("{}", rowCounter), zelSubSubEquipData.TypeOf);
                    OutputReportPredefined::PreDefTableEntry(
                        state, orp->pdchTopZnEqpSubSubCompName, format("{}", rowCounter), zelSubSubEquipData.Name);
                    fillZoneEquipToplogyComponentRow(state, zoneName, zelEquipData.TypeOf, zelEquipData.Name, rowCounter);
                }
            }
        }
    }
}

void fillZoneEquipToplogyComponentRow(
    EnergyPlusData &state, const std::string_view &zoneName, const std::string_view &compType, const std::string_view &compName, int &rowCounter)
{
    auto &orp = state.dataOutRptPredefined;
    // s->pdstTopZnEqp = newPreDefSubTable(state, s->pdrTopology, "Zone Equipment Component Arrangement");
    // s->pdchTopZnEqpName = newPreDefColumn(state, s->pdstTopZnEqp, "Zone Name");
    // s->pdchTopZnEqpCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Component Type");
    // s->pdchTopZnEqpCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Component Name");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpName, format("{}", rowCounter), zoneName);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpCompType, format("{}", rowCounter), compType);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchTopZnEqpCompName, format("{}", rowCounter), compName);
    ++rowCounter;
}

void reportAirDistributionUnits(EnergyPlusData &state)
{
    // populate the predefined tabular report for Equipment Summary - Air Terminals

    auto &orp = state.dataOutRptPredefined;
    for (auto &adu : state.dataDefineEquipment->AirDistUnit) {
        constexpr int aduCompNum = 1;
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermZoneName, adu.Name, state.dataHeatBal->Zone(adu.ZoneNum).Name);
        switch (adu.EquipTypeEnum(aduCompNum)) {
        case DataDefineEquip::ZnAirLoopEquipType::DualDuctConstVolume:
        case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAV:
        case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir:
            state.dataDualDuct->dd_airterminal(adu.EquipIndex(aduCompNum)).reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolReheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolNoReheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVNoReheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheatVSFan:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVReheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVNoReheat:
            state.dataSingleDuct->sd_airterminal(adu.EquipIndex(aduCompNum)).reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat:
            state.dataPowerInductionUnits->PIU(adu.EquipIndex(aduCompNum)).reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ConstVol_4PipeInduc:
            state.dataHVACSingleDuctInduc->IndUnit(adu.EquipIndex(aduCompNum)).reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolCooledBeam:
            state.dataHVACCooledBeam->CoolBeam(adu.EquipIndex(aduCompNum)).reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolFourPipeBeam:
            adu.airTerminalPtr->reportTerminalUnit(state);
            break;
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctUserDefined:
        case DataDefineEquip::ZnAirLoopEquipType::SingleDuctATMixer:
            break;
        default:
            break;
        } // end switch
    }
}

//        End of Reporting subroutines for the SimAir Module
// *****************************************************************************

} // namespace EnergyPlus::SystemReports
