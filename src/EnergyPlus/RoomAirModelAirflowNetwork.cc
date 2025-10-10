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
#include <ObjexxFCL/Array1D.hh>
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataMoistureBalanceEMPD.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceHAMTManager.hh>
#include <EnergyPlus/HighTempRadiantSystem.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/RoomAirModelAirflowNetwork.hh>
#include <EnergyPlus/SteamBaseboardRadiator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // MODULE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   November 2009
    //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

    // PURPOSE OF THIS MODULE:
    // contains the RoomAir model portions of RoomAirflowNetwork modeling

    // METHODOLOGY EMPLOYED:
    // Interact with Surface HB, internal gain, HVAC system and Airflow Network Domains
    // Do heat and moisture balance calculations on roomair nodes.

    // Using/Aliasing
    using namespace DataHeatBalSurface;
    using namespace DataSurfaces;
    using namespace DataHeatBalance;

    void SimRoomAirModelAFN(EnergyPlusData &state, int const zoneNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   January 2004/Aug 2005
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages RoomAirflowNetwork model simulation

        // METHODOLOGY EMPLOYED:
        // calls subroutines (LOL)

        auto const &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);

        // model control volume for each roomAir:node in the zone.
        for (int roomAirNodeNum = 1; roomAirNodeNum <= afnZoneInfo.NumOfAirNodes; ++roomAirNodeNum) {
            InitRoomAirModelAFN(state, zoneNum, roomAirNodeNum);
            CalcRoomAirModelAFN(state, zoneNum, roomAirNodeNum);
        }

        UpdateRoomAirModelAFN(state, zoneNum);

    } // SimRoomAirModelAirflowNetwork

    //****************************************************

    void LoadPredictionRoomAirModelAFN(EnergyPlusData &state,
                                       int const zoneNum,
                                       int const roomAirNodeNum) // index number for the specified zone and node
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June, 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Predict zone loads at a controlled node

        InitRoomAirModelAFN(state, zoneNum, roomAirNodeNum);

    } // LoadPredictionRoomAirModelAirflowNetwork

    //****************************************************

    void InitRoomAirModelAFN(EnergyPlusData &state, int const zoneNum,
                             int const roomAirNodeNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 release

        // PURPOSE OF THIS SUBROUTINE:
        // Perform one-time checking and term calculations

        using InternalHeatGains::SumInternalLatentGainsByTypes;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        Array1D_bool NodeFound; // True if a node is found.
        Array1D_bool EquipFound;
        Array1D<Real64> SupplyFrac;
        Array1D<Real64> ReturnFrac;

        if (state.dataRoomAirflowNetModel->OneTimeFlag) { // then do one - time setup inits

            // loop over all zones with RoomAirflowNetwork model
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                if (!afnZoneInfo.IsUsed) {
                    continue;
                }
                int NumSurfs = 0; // NumSurfs isn't used anywhere?
                for (int spaceNum : state.dataHeatBal->Zone(iZone).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    NumSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
                }

                for (auto &afnNode : afnZoneInfo.Node) {
                    // calculate volume of air in node's control volume
                    afnNode.AirVolume = state.dataHeatBal->Zone(iZone).Volume * afnNode.ZoneVolumeFraction;

                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node NonAirSystemResponse",
                                        Constant::Units::W,
                                        afnNode.NonAirSystemResponse,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        afnNode.Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SysDepZoneLoadsLagged",
                                        Constant::Units::W,
                                        afnNode.SysDepZoneLoadsLagged,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        afnNode.Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SumIntSensibleGain",
                                        Constant::Units::W,
                                        afnNode.SumIntSensibleGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        afnNode.Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SumIntLatentGain",
                                        Constant::Units::W,
                                        afnNode.SumIntLatentGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        afnNode.Name);
                }
            }
            state.dataRoomAirflowNetModel->OneTimeFlag = false;
        }

        if (state.dataRoomAirflowNetModel->OneTimeFlagConf) { // then do one - time setup inits
            if (allocated(state.dataZoneEquip->ZoneEquipConfig) && allocated(state.dataZoneEquip->ZoneEquipList)) {
                int MaxNodeNum = 0;
                int MaxEquipNum = 0;
                bool ErrorsFound = false;
                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (!state.dataHeatBal->Zone(iZone).IsControlled) {
                        continue;
                    }
                    MaxEquipNum = max(MaxEquipNum, state.dataZoneEquip->ZoneEquipList(iZone).NumOfEquipTypes);
                    MaxNodeNum = max(MaxNodeNum, state.dataZoneEquip->ZoneEquipConfig(iZone).NumInletNodes);
                }
                if (MaxNodeNum > 0) {
                    NodeFound.allocate(MaxNodeNum);
                    NodeFound = false;
                }
                if (MaxEquipNum > 0) {
                    EquipFound.allocate(MaxEquipNum);
                    SupplyFrac.allocate(MaxEquipNum);
                    ReturnFrac.allocate(MaxEquipNum);
                    EquipFound = false;
                    SupplyFrac = 0.0;
                    ReturnFrac = 0.0;
                }

                // loop over all zones with RoomAirflowNetwork model
                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    auto const &zone = state.dataHeatBal->Zone(iZone);
                    if (!zone.IsControlled) {
                        continue;
                    }

                    auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                    if (!afnZoneInfo.IsUsed) {
                        continue;
                    }
                    afnZoneInfo.ActualZoneID = iZone;
                    SupplyFrac = 0.0;
                    ReturnFrac = 0.0;
                    NodeFound = false;
                    int numAirDistUnits = 0;

                    auto const &zoneEquipList = state.dataZoneEquip->ZoneEquipList(iZone);
                    auto const &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(iZone);

                    // find supply air node number
                    for (auto &afnNode : afnZoneInfo.Node) {
                        for (auto &afnHVAC : afnNode.HVAC) {
                            for (int I = 1; I <= zoneEquipList.NumOfEquipTypes; ++I) { // loop over all equip types
                                if (zoneEquipList.EquipType(I) == DataZoneEquipment::ZoneEquipType::AirDistributionUnit) {
                                    if (numAirDistUnits == 0) {
                                        numAirDistUnits =
                                            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:AirDistributionUnit");
                                    }
                                    if (state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag) {
                                        ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(state);
                                        state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
                                    }

                                    for (int AirDistUnitNum = 1; AirDistUnitNum <= numAirDistUnits; ++AirDistUnitNum) {
                                        if (zoneEquipList.EquipName(I) == state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).Name) {
                                            if (afnHVAC.Name == state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EquipName(1)) {
                                                if (afnHVAC.EquipConfigIndex == 0) {
                                                    afnHVAC.EquipConfigIndex = I;
                                                }
                                                EquipFound(I) = true;
                                                SupplyFrac(I) += afnHVAC.SupplyFraction;
                                                ReturnFrac(I) += afnHVAC.ReturnFraction;
                                            }
                                        }
                                    }
                                } else if (Util::SameString(zoneEquipList.EquipName(I), afnHVAC.Name)) {
                                    if (afnHVAC.EquipConfigIndex == 0) {
                                        afnHVAC.EquipConfigIndex = I;
                                    }
                                    EquipFound(I) = true;
                                    SupplyFrac(I) += afnHVAC.SupplyFraction;
                                    ReturnFrac(I) += afnHVAC.ReturnFraction;
                                }
                            }
                            for (int iNode = 1; iNode <= state.dataLoopNodes->NumOfNodes; ++iNode) { // loop over all nodes to find supply node ID
                                if (Util::SameString(state.dataLoopNodes->NodeID(iNode), afnHVAC.SupplyNodeName)) {
                                    afnHVAC.SupNodeNum = iNode;
                                    break;
                                }
                            }
                            // Verify inlet nodes
                            int inletNodeIndex = 0;
                            for (int iNode = 1; iNode <= zoneEquipConfig.NumInletNodes;
                                 ++iNode) { // loop over all supply inlet nodes in a single zone
                                // !Get node conditions
                                if (zoneEquipConfig.InletNode(iNode) == afnHVAC.SupNodeNum) {
                                    NodeFound(iNode) = true;
                                    inletNodeIndex = iNode;
                                    break;
                                }
                            }

                            if (afnHVAC.SupNodeNum > 0 && afnHVAC.ReturnNodeName.empty()) {
                                // Find matching return node
                                for (int retNode = 1; retNode <= zoneEquipConfig.NumReturnNodes; ++retNode) {
                                    if ((zoneEquipConfig.ReturnNodeInletNum(retNode) == inletNodeIndex) &&
                                        (zoneEquipConfig.ReturnNode(retNode) > 0)) {
                                        afnHVAC.RetNodeNum = zoneEquipConfig.ReturnNode(retNode); // Zone return node
                                        break;
                                    }
                                }
                            }

                            if (afnHVAC.RetNodeNum == 0) {
                                for (int iNode = 1; iNode <= state.dataLoopNodes->NumOfNodes; ++iNode) { // loop over all nodes to find return node ID
                                    if (Util::SameString(state.dataLoopNodes->NodeID(iNode), afnHVAC.ReturnNodeName)) {
                                        afnHVAC.RetNodeNum = iNode;
                                        break;
                                    }
                                }
                            }
                            SetupOutputVariable(state,
                                                "RoomAirflowNetwork Node HVAC Supply Fraction",
                                                Constant::Units::None,
                                                afnHVAC.SupplyFraction,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                afnHVAC.Name);
                            SetupOutputVariable(state,
                                                "RoomAirflowNetwork Node HVAC Return Fraction",
                                                Constant::Units::None,
                                                afnHVAC.ReturnFraction,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                afnHVAC.Name);
                        }
                    }
                    // Count node with.TRUE.
                    int ISum = 0;
                    for (int iNode = 1; iNode <= MaxNodeNum; ++iNode) { // loop over all supply inlet nodes in a single zone
                        if (NodeFound(iNode)) {
                            ++ISum;
                        }
                    }
                    // Provide error messages with incorrect supplu node inputs
                    if (ISum != zoneEquipConfig.NumInletNodes) {
                        if (ISum > zoneEquipConfig.NumInletNodes) {
                            ShowSevereError(
                                state, "GetRoomAirflowNetworkData: The number of equipment listed in RoomAirflowNetwork:Node:HVACEquipment objects");
                            ShowContinueError(state, format("is greater than the number of zone configuration inlet nodes in {}", zone.Name));
                            ShowContinueError(state, "Please check inputs of both objects.");
                            ErrorsFound = true;
                        } else {
                            ShowSevereError(
                                state, "GetRoomAirflowNetworkData: The number of equipment listed in RoomAirflowNetwork:Node:HVACEquipment objects");
                            ShowContinueError(state, format("is less than the number of zone configuration inlet nodes in {}", zone.Name));
                            ShowContinueError(state, "Please check inputs of both objects.");
                            ErrorsFound = true;
                        }
                    }

                    // Check equipment names to ensure they are used in RoomAirflowNetwork : Node : HVACEquipment objects
                    for (int I = 1; I <= zoneEquipList.NumOfEquipTypes; ++I) { // loop over all equip types
                        if (!EquipFound(I)) {
                            ShowSevereError(state,
                                            "GetRoomAirflowNetworkData: The equipment listed in ZoneEquipList is not found in the lsit of "
                                            "RoomAir:Node:AirflowNetwork:HVACEquipment objects =");
                            ShowContinueError(state, format("{}. Please check inputs of both objects.", zoneEquipList.EquipName(I)));
                            ErrorsFound = true;
                        }
                    }

                    // Check fraction to ensure sum = 1.0 for every equipment
                    for (int I = 1; I <= zoneEquipList.NumOfEquipTypes; ++I) { // loop over all equip types
                        if (std::abs(SupplyFrac(I) - 1.0) > 0.001) {
                            ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, zone supply fractions do not sum to 1.0");
                            ShowContinueError(
                                state, format("Entered in {} defined in RoomAir:Node:AirflowNetwork:HVACEquipment", zoneEquipList.EquipName(I)));
                            ShowContinueError(state,
                                              "The Fraction of supply fraction values across all the roomair nodes in a zone needs to sum to 1.0.");
                            ShowContinueError(state, format("The sum of fractions entered = {:.3R}", SupplyFrac(I)));
                            ErrorsFound = true;
                        }
                        if (std::abs(ReturnFrac(I) - 1.0) > 0.001) {
                            ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, zone return fractions do not sum to 1.0");
                            ShowContinueError(
                                state, format("Entered in {} defined in RoomAir:Node:AirflowNetwork:HVACEquipment", zoneEquipList.EquipName(I)));
                            ShowContinueError(state,
                                              "The Fraction of return fraction values across all the roomair nodes in a zone needs to sum to 1.0.");
                            ShowContinueError(state, format("The sum of fractions entered = {:.3R}", ReturnFrac(I)));
                            ErrorsFound = true;
                        }
                    }
                }
                state.dataRoomAirflowNetModel->OneTimeFlagConf = false;
                if (allocated(NodeFound)) {
                    NodeFound.deallocate();
                }
                if (ErrorsFound) {
                    ShowFatalError(state, "GetRoomAirflowNetworkData: Errors found getting air model input.  Program terminates.");
                }
            } // if (allocated)
        } // if (OneTimeFlagConf)

        if (state.dataGlobal->BeginEnvrnFlag && state.dataRoomAirflowNetModel->EnvrnFlag) {
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                if (!afnZoneInfo.IsUsed) {
                    continue;
                }
                for (auto &afnNode : afnZoneInfo.Node) {
                    afnNode.AirTemp = 23.0;
                    afnNode.AirTempX = {23.0, 23.0, 23.0, 23.0};
                    afnNode.AirTempDSX = {23.0, 23.0, 23.0, 23.0};
                    afnNode.AirTempT1 = 23.0;
                    afnNode.AirTempTX = 23.0;
                    afnNode.AirTempT2 = 23.0;

                    afnNode.HumRat = 0.0;
                    afnNode.HumRatX = {0.0, 0.0, 0.0, 0.0};
                    afnNode.HumRatDSX = {0.0, 0.0, 0.0, 0.0};
                    afnNode.HumRatT1 = 0.0;
                    afnNode.HumRatTX = 0.0;
                    afnNode.HumRatT2 = 0.0;

                    afnNode.SysDepZoneLoadsLagged = 0.0;
                    afnNode.SysDepZoneLoadsLaggedOld = 0.0;
                }
            }
            state.dataRoomAirflowNetModel->EnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataRoomAirflowNetModel->EnvrnFlag = true;
        }

        // reuse code in ZoneTempPredictorCorrector for sensible components.
        CalcNodeSums(state, zoneNum, roomAirNodeNum);

        SumNonAirSystemResponseForNode(state, zoneNum, roomAirNodeNum);

        // latent gains.
        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
        auto &afnNode = afnZoneInfo.Node(roomAirNodeNum);

        if (allocated(afnNode.SurfMask)) {
            CalcSurfaceMoistureSums(state, zoneNum, roomAirNodeNum, afnNode.SumHmAW, afnNode.SumHmARa, afnNode.SumHmARaW, afnNode.SurfMask);
        }

        // prepare AirflowNetwor flow rates and temperatures
        Real64 SumLinkMCp = 0.0;
        Real64 SumLinkMCpT = 0.0;
        Real64 SumLinkM = 0.0;
        Real64 SumLinkMW = 0.0;

        if (afnNode.AFNNodeID > 0) {
            for (int iLink = 1; iLink <= afnNode.NumOfAirflowLinks; ++iLink) {
                auto &afnLink = afnNode.Link(iLink);
                int linkNum = afnLink.AFNSimuID;
                if (state.afn->AirflowNetworkLinkageData(linkNum).NodeNums[0] == afnNode.AFNNodeID) { // incoming flow
                    int nodeInNum = state.afn->AirflowNetworkLinkageData(linkNum).NodeNums[1];
                    afnLink.TempIn = state.afn->AirflowNetworkNodeSimu(nodeInNum).TZ;
                    afnLink.HumRatIn = state.afn->AirflowNetworkNodeSimu(nodeInNum).WZ;
                    afnLink.MdotIn = state.afn->AirflowNetworkLinkSimu(linkNum).FLOW2;
                }
                if (state.afn->AirflowNetworkLinkageData(linkNum).NodeNums[1] == afnNode.AFNNodeID) { // outgoing flow
                    int nodeInNum = state.afn->AirflowNetworkLinkageData(linkNum).NodeNums[0];
                    afnLink.TempIn = state.afn->AirflowNetworkNodeSimu(nodeInNum).TZ;
                    afnLink.HumRatIn = state.afn->AirflowNetworkNodeSimu(nodeInNum).WZ;
                    afnLink.MdotIn = state.afn->AirflowNetworkLinkSimu(linkNum).FLOW;
                }
            }

            for (int iLink = 1; iLink <= afnNode.NumOfAirflowLinks; ++iLink) {
                auto &afnLink = afnNode.Link(iLink);
                Real64 CpAir = PsyCpAirFnW(afnLink.HumRatIn);
                SumLinkMCp += CpAir * afnLink.MdotIn;
                SumLinkMCpT += CpAir * afnLink.MdotIn * afnLink.TempIn;
                SumLinkM += afnLink.MdotIn;
                SumLinkMW += afnLink.MdotIn * afnLink.HumRatIn;
            }
        }

        afnNode.SumLinkMCp = SumLinkMCp;
        afnNode.SumLinkMCpT = SumLinkMCpT;
        afnNode.SumLinkM = SumLinkM;
        afnNode.SumLinkMW = SumLinkMW;
        afnNode.SysDepZoneLoadsLagged = afnNode.SysDepZoneLoadsLaggedOld;

        afnNode.RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, afnNode.AirTemp, afnNode.HumRat, "InitRoomAirModelAirflowNetwork");

        afnNode.CpAir = PsyCpAirFnW(afnNode.HumRat);

    } // InitRoomAirModelAirflowNetwork

    //*****************************************************************************************

    void CalcRoomAirModelAFN(EnergyPlusData &state, int const zoneNum,
                             int const roomAirNodeNum) // index number for the specified zone and node
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // calculate new values for temperature and humidity ratio for room air node

        // METHODOLOGY EMPLOYED:
        // take terms(updated in init routine) and use classic air balance equations
        // solved for state variables. Store results in structure.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using Psychrometrics::PsyHgAirFnWTdb;
        using Psychrometrics::PsyRhFnTdbWPb;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::array<Real64, 3> NodeTempX;
        std::array<Real64, 3> NodeHumRatX;
        Real64 AirTempT1;
        Real64 HumRatT1;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
        auto &afnNode = afnZoneInfo.Node(roomAirNodeNum);

        if (state.dataHVACGlobal->UseZoneTimeStepHistory) {
            NodeTempX[0] = afnNode.AirTempX[0];
            NodeTempX[1] = afnNode.AirTempX[1];
            NodeTempX[2] = afnNode.AirTempX[2];

            NodeHumRatX[0] = afnNode.HumRatX[0];
            NodeHumRatX[1] = afnNode.HumRatX[1];
            NodeHumRatX[2] = afnNode.HumRatX[2];
        } else { // use down - stepped history
            NodeTempX[0] = afnNode.AirTempDSX[0];
            NodeTempX[1] = afnNode.AirTempDSX[1];
            NodeTempX[2] = afnNode.AirTempDSX[2];

            NodeHumRatX[0] = afnNode.HumRatDSX[0];
            NodeHumRatX[1] = afnNode.HumRatDSX[1];
            NodeHumRatX[2] = afnNode.HumRatDSX[2];
        }

        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            AirTempT1 = afnNode.AirTempT1;
            HumRatT1 = afnNode.HumRatT1;
        }
        // solve for node drybulb temperature
        Real64 TempDepCoef = afnNode.SumHA + afnNode.SumLinkMCp + afnNode.SumSysMCp;
        Real64 TempIndCoef = afnNode.SumIntSensibleGain + afnNode.SumHATsurf - afnNode.SumHATref + afnNode.SumLinkMCpT + afnNode.SumSysMCpT +
                             afnNode.NonAirSystemResponse + afnNode.SysDepZoneLoadsLagged;
        Real64 AirCap = afnNode.AirVolume * state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpSens * afnNode.RhoAir * afnNode.CpAir / TimeStepSysSec;

        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (TempDepCoef == 0.0) { // B=0
                afnNode.AirTemp = AirTempT1 + TempIndCoef / AirCap;
            } else {
                afnNode.AirTemp = (AirTempT1 - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) + TempIndCoef / TempDepCoef;
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            afnNode.AirTemp = (AirCap * AirTempT1 + TempIndCoef) / (AirCap + TempDepCoef);
        } else {
            afnNode.AirTemp = (TempIndCoef + AirCap * (3.0 * NodeTempX[0] - (3.0 / 2.0) * NodeTempX[1] + (1.0 / 3.0) * NodeTempX[2])) /
                              ((11.0 / 6.0) * AirCap + TempDepCoef);
        }

        // solve for node humidity ratio using 3 algorithms
        Real64 H2OHtOfVap = PsyHgAirFnWTdb(afnNode.HumRat, afnNode.AirTemp);
        Real64 A = afnNode.SumLinkM + afnNode.SumHmARa + afnNode.SumSysM;
        Real64 B = (afnNode.SumIntLatentGain / H2OHtOfVap) + afnNode.SumSysMW + afnNode.SumLinkMW + afnNode.SumHmARaW;
        Real64 C = afnNode.RhoAir * afnNode.AirVolume * state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpMoist / TimeStepSysSec;

        // Exact solution
        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (A == 0.0) { // B=0
                afnNode.HumRat = HumRatT1 + B / C;
            } else {
                afnNode.HumRat = (HumRatT1 - B / A) * std::exp(min(700., -A / C)) + B / A;
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            afnNode.HumRat = (C * HumRatT1 + B) / (C + A);
        } else {
            afnNode.HumRat = (B + C * (3.0 * NodeHumRatX[0] - (3.0 / 2.0) * NodeHumRatX[1] + (1.0 / 3.0) * NodeHumRatX[2])) / ((11.0 / 6.0) * C + A);
        }

        afnNode.AirCap = AirCap;
        afnNode.AirHumRat = C;

        afnNode.RelHumidity =
            PsyRhFnTdbWPb(state, afnNode.AirTemp, afnNode.HumRat, state.dataEnvrn->OutBaroPress, "CalcRoomAirModelAirflowNetwork") * 100.0;

    } // CalcRoomAirModelAirflowNetwork

    void UpdateRoomAirModelAFN(EnergyPlusData &state, int const zoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // update variables
        auto const &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);

        if (!afnZoneInfo.IsUsed) {
            return;
        }

        if (!state.dataGlobal->ZoneSizingCalc) {
            SumSystemDepResponseForNode(state, zoneNum);
        }

        // Update return node conditions
        for (int I = 1; I <= state.dataZoneEquip->ZoneEquipList(zoneNum).NumOfEquipTypes; ++I) { // loop over all equip types
            Real64 SumMass = 0.0;
            Real64 SumMassT = 0.0;
            Real64 SumMassW = 0.0;
            int RetNodeNum = 0;
            for (auto const &afnNode : afnZoneInfo.Node) {
                for (auto const &afnHVAC : afnNode.HVAC) {
                    if (afnHVAC.EquipConfigIndex == I && afnHVAC.SupNodeNum > 0 && afnHVAC.RetNodeNum > 0) {
                        Real64 NodeMass = state.dataLoopNodes->Node(afnHVAC.SupNodeNum).MassFlowRate * afnHVAC.ReturnFraction;
                        SumMass += NodeMass;
                        SumMassT += NodeMass * afnNode.AirTemp;
                        SumMassW += NodeMass * afnNode.HumRat;
                        RetNodeNum = afnHVAC.RetNodeNum;
                    }
                }
            }
            if (SumMass > 0.0) {
                state.dataLoopNodes->Node(RetNodeNum).Temp = SumMassT / SumMass;
                state.dataLoopNodes->Node(RetNodeNum).HumRat = SumMassW / SumMass;
            }
        }
    } // UpdateRoomAirModelAirflowNetwork

    void CalcNodeSums(EnergyPlusData &state, int const zoneNum, int const roomAirNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   August 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE - ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE :
        // This subroutine calculates the various sums that go into the zone heat balance
        // equation.This replaces the SUMC, SUMHA, and SUMHAT calculations that were
        // previously done in various places throughout the program.
        // The SumHAT portion of the code is reproduced in RadiantSystemHighTemp and
        // RadiantSystemLowTemp and should be updated accordingly.
        //
        // A reference temperature(Tref) is specified for use with the ceiling diffuser
        // convection correlation.A bogus value of Tref = -999.9 defaults to using
        // the zone air(i.e.outlet) temperature for the reference temperature.
        // If Tref is applied to all surfaces, SumHA = 0, and SumHATref /= 0.
        // If Tref is not used at all, SumHATref = 0, and SumHA /= 0.
        //

        // USE STATEMENTS:
        using InternalHeatGains::SumInternalConvectionGainsByIndices;
        using InternalHeatGains::SumInternalLatentGainsByIndices;
        using InternalHeatGains::SumReturnAirConvectionGainsByIndices;
        using InternalHeatGains::SumReturnAirConvectionGainsByTypes;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HA;         //                     !Hc*Area
        Real64 Area;       //                   !Effective surface area
        Real64 RefAirTemp; //             !Reference air temperature for surface convection calculations
        bool Found;        //

        Real64 SumIntGain = 0.0; // node sum of convective internal gains
        Real64 SumHA = 0.0;      // Zone sum of Hc*Area
        Real64 SumHATsurf = 0.0; // Zone sum of Hc*Area*Tsurf
        Real64 SumHATref = 0.0;  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
        Real64 SumSysMCp = 0.0;  // Zone sum of air system MassFlowRate*Cp
        Real64 SumSysMCpT = 0.0; // Zone sum of air system MassFlowRate*Cp*T
        Real64 SumSysM = 0.0;    // Zone sum of air system MassFlowRate
        Real64 SumSysMW = 0.0;   // Zone sum of air system MassFlowRate*W

        auto const &zone = state.dataHeatBal->Zone(zoneNum);
        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
        auto &afnNode = afnZoneInfo.Node(roomAirNodeNum);
        // Sum all convective internal gains: SumIntGain
        afnNode.SumIntSensibleGain = SumInternalConvectionGainsByIndices(
            state, afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);

        afnNode.SumIntLatentGain = SumInternalLatentGainsByIndices(
            state, afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);
        // Add heat to return air if zonal system(no return air) or cycling system(return air frequently very low or zero)
        if (state.dataHeatBal->Zone(zoneNum).NoHeatToReturnAir) {
            // *******************************************
            SumIntGain = SumReturnAirConvectionGainsByIndices(
                state, afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);
            afnNode.SumIntSensibleGain += SumIntGain;
        }

        // Check to see if this is a controlled zone
        // Check to see if this is a plenum zone
        int zoneRetPlenumNum = 0;
        for (int iPlenum = 1; iPlenum <= state.dataZonePlenum->NumZoneReturnPlenums; ++iPlenum) {
            if (state.dataZonePlenum->ZoneRetPlenCond(iPlenum).ActualZoneNum != zoneNum) {
                continue;
            }
            zoneRetPlenumNum = iPlenum;
            break;
        }
        bool zoneSupPlenumNum = false;
        for (int iPlenum = 1; iPlenum <= state.dataZonePlenum->NumZoneSupplyPlenums; ++iPlenum) {
            if (state.dataZonePlenum->ZoneSupPlenCond(iPlenum).ActualZoneNum != zoneNum) {
                continue;
            }
            zoneSupPlenumNum = iPlenum;
            break;
        }

        // Plenum and controlled zones have a different set of inlet nodes which must be calculated.
        auto &zoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);
        if (zone.IsControlled) {
            auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
            for (int iNode = 1; iNode <= zoneEquipConfig.NumInletNodes; ++iNode) {
                // Get node conditions
                // this next block is of interest to irratic system loads... maybe nodes are not accurate at time of call ?
                // how can we tell ? predict step must be lagged ? correct step, systems have run.
                auto const &inletNode = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(iNode));
                for (auto const &afnHVAC : afnNode.HVAC) {
                    if (afnHVAC.SupNodeNum == zoneEquipConfig.InletNode(iNode)) {
                        Real64 MassFlowRate = inletNode.MassFlowRate * afnHVAC.SupplyFraction;
                        Real64 CpAir = PsyCpAirFnW(zoneHB.airHumRat);
                        SumSysMCp += MassFlowRate * CpAir;
                        SumSysMCpT += MassFlowRate * CpAir * inletNode.Temp;
                        SumSysM += MassFlowRate;
                        SumSysMW += MassFlowRate * inletNode.HumRat;
                    }
                } // EquipLoop
            } // NodeNum
        } else if (zoneRetPlenumNum != 0) {
            auto const &zoneRetPlenum = state.dataZonePlenum->ZoneRetPlenCond(zoneRetPlenumNum);
            for (int iNode = 1; iNode <= zoneRetPlenum.NumInletNodes; ++iNode) {
                // Get node conditions
                auto const &zoneRetPlenumNode = state.dataLoopNodes->Node(zoneRetPlenum.InletNode(iNode));
                Real64 CpAir = PsyCpAirFnW(zoneHB.airHumRat);
                SumSysMCp += zoneRetPlenumNode.MassFlowRate * CpAir;
                SumSysMCpT += zoneRetPlenumNode.MassFlowRate * CpAir * zoneRetPlenumNode.Temp;
            } // NodeNum
            // add in the leaks
            for (int iADU = 1; iADU <= zoneRetPlenum.NumADUs; ++iADU) {
                int ADUNum = zoneRetPlenum.ADUIndex(iADU);
                auto const &adu = state.dataDefineEquipment->AirDistUnit(ADUNum);
                if (adu.UpStreamLeak) {
                    Real64 CpAir = PsyCpAirFnW(zoneHB.airHumRat);
                    SumSysMCp += adu.MassFlowRateUpStrLk * CpAir;
                    SumSysMCpT += adu.MassFlowRateUpStrLk * CpAir * state.dataLoopNodes->Node(adu.InletNodeNum).Temp;
                }
                if (adu.DownStreamLeak) {
                    Real64 CpAir = PsyCpAirFnW(zoneHB.airHumRat);
                    SumSysMCp += adu.MassFlowRateDnStrLk * CpAir;
                    SumSysMCpT += adu.MassFlowRateDnStrLk * CpAir * state.dataLoopNodes->Node(adu.OutletNodeNum).Temp;
                }
            } // ADUListIndex
        } else if (zoneSupPlenumNum != 0) {
            // Get node conditions
            auto const &zoneSupPlenum = state.dataZonePlenum->ZoneSupPlenCond(zoneSupPlenumNum);
            auto const &inletNode = state.dataLoopNodes->Node(zoneSupPlenum.InletNode);
            Real64 CpAir = PsyCpAirFnW(zoneHB.airHumRat);
            SumSysMCp += inletNode.MassFlowRate * CpAir;
            SumSysMCpT += inletNode.MassFlowRate * CpAir * inletNode.Temp;
        }

        int ZoneMult = zone.Multiplier * zone.ListMultiplier;

        SumSysMCp /= ZoneMult;
        SumSysMCpT /= ZoneMult;
        SumSysM /= ZoneMult;
        SumSysMW /= ZoneMult;

        // Sum all surface convection : SumHA, SumHATsurf, SumHATref(and additional contributions to SumIntGain)
        // Modified by Gu to include assigned surfaces only shown in the surface lsit
        if (!afnNode.HasSurfacesAssigned) {
            return;
        }

        int surfCount = 0;
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                ++surfCount;
                if (afnZoneInfo.ControlAirNodeID == roomAirNodeNum) {
                    Found = false;
                    for (int Loop = 1; Loop <= afnZoneInfo.NumOfAirNodes; ++Loop) {
                        if (Loop != roomAirNodeNum) {
                            if (afnZoneInfo.Node(Loop).SurfMask(surfCount)) {
                                Found = true;
                                break;
                            }
                        }
                    }
                    if (Found) {
                        continue;
                    }
                } else {
                    if (!afnNode.SurfMask(surfCount)) {
                        continue;
                    }
                }

                HA = 0.0;
                Area = state.dataSurface->Surface(SurfNum).Area; // For windows, this is the glazing area

                if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {

                    // Add to the convective internal gains
                    if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        // The shade area covers the area of the glazing plus the area of the dividers.
                        Area += state.dataSurface->SurfWinDividerArea(SurfNum);
                        SumIntGain += state.dataSurface->SurfWinDividerHeatGain(SurfNum);
                    }

                    // Convective heat gain from natural convection in gap between glass and interior shade or blind
                    if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        SumIntGain += state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum);
                    }

                    // Convective heat gain from airflow window
                    if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0) {
                        SumIntGain += state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum);
                        if (zone.NoHeatToReturnAir) {
                            SumIntGain += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                            state.dataSurface->SurfWinHeatGain(SurfNum) += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                            if (state.dataSurface->SurfWinHeatGain(SurfNum) >= 0.0) {
                                state.dataSurface->SurfWinHeatGainRep(SurfNum) = state.dataSurface->SurfWinHeatGain(SurfNum);
                                state.dataSurface->SurfWinHeatGainRepEnergy(SurfNum) =
                                    state.dataSurface->SurfWinHeatGainRep(SurfNum) * state.dataGlobal->TimeStepZone * Constant::rSecsInHour;
                            } else {
                                state.dataSurface->SurfWinHeatLossRep(SurfNum) = -state.dataSurface->SurfWinHeatGain(SurfNum);
                                state.dataSurface->SurfWinHeatLossRepEnergy(SurfNum) =
                                    state.dataSurface->SurfWinHeatLossRep(SurfNum) * state.dataGlobal->TimeStepZone * Constant::rSecsInHour;
                            }
                            state.dataSurface->SurfWinHeatTransferRepEnergy(SurfNum) =
                                state.dataSurface->SurfWinHeatGain(SurfNum) * state.dataGlobal->TimeStepZone * Constant::rSecsInHour;
                        }
                    }

                    // Add to the surface convection sums
                    if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                        // Window frame contribution
                        SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                                      (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum)) * state.dataSurface->SurfWinFrameTempIn(SurfNum);
                        HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                              (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum));
                    }

                    if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0 &&
                        !ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        // Window divider contribution(only from shade or blind for window with divider and interior shade or blind)
                        SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                                      (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) *
                                      state.dataSurface->SurfWinDividerTempIn(SurfNum);
                        HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                              (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum));
                    }

                } // End of check if window

                HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area;
                SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area * state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

                if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::ZoneMeanAirTemp) {
                    // The zone air is the reference temperature(which is to be solved for in CorrectZoneAirTemp).
                    RefAirTemp = zoneHB.MAT;
                    SumHA += HA;
                } else if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::AdjacentAirTemp) {
                    RefAirTemp = state.dataHeatBal->SurfTempEffBulkAir(SurfNum);
                    SumHATref += HA * RefAirTemp;
                } else if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::ZoneSupplyAirTemp) {
                    // check whether this zone is a controlled zone or not
                    if (!zone.IsControlled) {
                        ShowFatalError(state,
                                       format("Zones must be controlled for Ceiling-Diffuser Convection model. No system serves zone {}", zone.Name));
                        return;
                    }
                    // determine supply air temperature as a weighted average of the inlet temperatures.
                    RefAirTemp = SumSysMCpT / SumSysMCp;
                    SumHATref += HA * RefAirTemp;
                } else {
                    RefAirTemp = zoneHB.MAT;
                    SumHA += HA;
                }

            } // SurfNum
        }
        // Assemble values
        afnNode.SumHA = SumHA;
        afnNode.SumHATsurf = SumHATsurf;
        afnNode.SumHATref = SumHATref;
        afnNode.SumSysMCp = SumSysMCp;
        afnNode.SumSysMCpT = SumSysMCpT;
        afnNode.SumSysM = SumSysM;
        afnNode.SumSysMW = SumSysMW;

    } // CalcNodeSums

    void CalcSurfaceMoistureSums(EnergyPlusData &state,
                                 int const zoneNum,
                                 int const roomAirNodeNum,
                                 Real64 &SumHmAW,
                                 Real64 &SumHmARa,
                                 Real64 &SumHmARaW,
                                 [[maybe_unused]] Array1D<bool> const &SurfMask)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //                      derived from P. Biddulph-- HAMT, L. Gu -- EPMD,
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // Breakout summation of surface moisture interaction terms

        // Using/Aliasing

        using HeatBalanceHAMTManager::UpdateHeatBalHAMT;
        using MoistureBalanceEMPDManager::UpdateMoistureBalanceEMPD;
        using Psychrometrics::PsyRhFnTdbRhov;
        using Psychrometrics::PsyRhFnTdbRhovLBnd0C;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyWFnTdbRhPb;

        SumHmAW = 0.0;
        SumHmARa = 0.0;
        SumHmARaW = 0.0;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);

        int surfCount = 1;
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum, ++surfCount) {
                auto const &surf = state.dataSurface->Surface(SurfNum);
                if (surf.Class == SurfaceClass::Window) {
                    continue;
                }

                if (afnZoneInfo.ControlAirNodeID == roomAirNodeNum) {
                    bool Found = false;
                    for (int Loop = 1; Loop <= afnZoneInfo.NumOfAirNodes && !Found; ++Loop) {
                        // None - assigned surfaces belong to the zone node
                        Found = (Loop != roomAirNodeNum) && afnZoneInfo.Node(Loop).SurfMask(surfCount);
                    }
                    if (Found) {
                        continue;
                    }
                } else {
                    if (!afnZoneInfo.Node(roomAirNodeNum).SurfMask(surfCount)) {
                        continue;
                    }
                }

                auto const &HMassConvInFD = state.dataMstBal->HMassConvInFD(SurfNum);
                auto &RhoVaporSurfIn = state.dataMstBal->RhoVaporSurfIn(SurfNum);
                auto &RhoVaporAirIn = state.dataMstBal->RhoVaporAirIn(SurfNum);
                if (surf.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                    UpdateHeatBalHAMT(state, SurfNum);

                    SumHmAW += HMassConvInFD * surf.Area * (RhoVaporSurfIn - RhoVaporAirIn);

                    Real64 RhoAirZone = PsyRhoAirFnPbTdbW(
                        state,
                        state.dataEnvrn->OutBaroPress,
                        state.dataZoneTempPredictorCorrector->zoneHeatBalance(surf.Zone).MAT,
                        PsyRhFnTdbRhov(state,
                                       state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(SurfNum).Zone).MAT,
                                       RhoVaporAirIn,
                                       "RhoAirZone"));

                    Real64 Wsurf = PsyWFnTdbRhPb(state,
                                                 state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                                 PsyRhFnTdbRhov(state, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), RhoVaporSurfIn, "Wsurf"),
                                                 state.dataEnvrn->OutBaroPress);

                    SumHmARa += HMassConvInFD * surf.Area * RhoAirZone;
                    SumHmARaW += HMassConvInFD * surf.Area * RhoAirZone * Wsurf;
                }

                else if (surf.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {

                    UpdateMoistureBalanceEMPD(state, SurfNum);
                    RhoVaporSurfIn = state.dataMstBalEMPD->RVSurface(SurfNum);

                    SumHmAW += HMassConvInFD * surf.Area * (RhoVaporSurfIn - RhoVaporAirIn);
                    SumHmARa +=
                        HMassConvInFD * surf.Area *
                        PsyRhoAirFnPbTdbW(state,
                                          state.dataEnvrn->OutBaroPress,
                                          state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                          PsyWFnTdbRhPb(state,
                                                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                                        PsyRhFnTdbRhovLBnd0C(state, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), RhoVaporAirIn),
                                                        state.dataEnvrn->OutBaroPress));
                    SumHmARaW += HMassConvInFD * surf.Area * RhoVaporSurfIn;
                }
            } // for (SurfNum)
        } // for (spaceNum)

    } // CalcSurfaceMoistureSums

    void SumNonAirSystemResponseForNode(EnergyPlusData &state, int const zoneNum, int const roomAirNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // Sum system response from none air systems

        // USE STATEMENTS:
        using BaseboardElectric::SimElectricBaseboard;
        using BaseboardRadiator::SimBaseboard;
        using ElectricBaseboardRadiator::SimElecBaseboard;
        using HighTempRadiantSystem::SimHighTempRadiantSystem;
        using HWBaseboardRadiator::SimHWBaseboard;
        using RefrigeratedCase::SimAirChillerSet;
        using SteamBaseboardRadiator::SimSteamBaseboard;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SysOutputProvided;
        Real64 LatOutputProvided;

        // TODO
        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
        auto &afnNode = afnZoneInfo.Node(roomAirNodeNum);

        afnNode.NonAirSystemResponse = 0.0;

        if (!allocated(state.dataZoneEquip->ZoneEquipConfig)) {
            return;
        }

        for (auto &afnHVAC : afnNode.HVAC) {
            switch (afnHVAC.zoneEquipType) {

            case DataZoneEquipment::ZoneEquipType::BaseboardWater: {
                //'ZoneHVAC:Baseboard:RadiantConvective:Water' 13
                SimHWBaseboard(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardSteam: {
                // CASE(BBSteam_Num) !'ZoneHVAC:Baseboard:RadiantConvective:Steam' 14
                SimSteamBaseboard(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, afnHVAC.CompIndex);

                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater: {
                // CASE(BBWaterConvective_Num)  !'ZoneHVAC:Baseboard:Convective:Water' 16
                SimBaseboard(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric: {
                // CASE(BBElectricConvective_Num)  !'ZoneHVAC:Baseboard:Convective:Electric' 15
                SimElectricBaseboard(state, afnHVAC.Name, zoneNum, SysOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::RefrigerationChillerSet: {
                // CASE(RefrigerationAirChillerSet_Num)  !'ZoneHVAC:RefrigerationChillerSet' 20
                SimAirChillerSet(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, LatOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardElectric: {
                // CASE(BBElectric_Num)  !'ZoneHVAC:Baseboard:RadiantConvective:Electric' 12
                SimElecBaseboard(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::HighTemperatureRadiant: {
                // CASE(BBElectric_Num)  !'ZoneHVAC:HighTemperatureRadiant' 17
                SimHighTempRadiantSystem(state, afnHVAC.Name, false, SysOutputProvided, afnHVAC.CompIndex);
                afnNode.NonAirSystemResponse += afnHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            default: {
            } break;
            } // switch

            // Zone sum of system convective gains, collected via NonAirSystemResponse
        }

    } // SumNonAirSystemResponseForNode

    //*****************************************************************************************

    void SumSystemDepResponseForNode(EnergyPlusData &state, int const zoneNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.Griffith
        //       DATE WRITTEN   aug 2005, Jan2004
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // Sum system sensible loads used at the next time step

        // USE STATEMENTS:
        using ZoneDehumidifier::SimZoneDehumidifier;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 LatOutputProvided;

        // TODO

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);

        // SysDepZoneLoads saved to be added to zone heat balance next
        Real64 SysOutputProvided = 0.0;
        for (auto &afnNode : afnZoneInfo.Node) {
            afnNode.SysDepZoneLoadsLaggedOld = 0.0;
            for (auto &afnHVAC : afnNode.HVAC) {
                if (afnHVAC.zoneEquipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                    if (SysOutputProvided == 0.0) {
                        SimZoneDehumidifier(state, afnHVAC.Name, zoneNum, false, SysOutputProvided, LatOutputProvided, afnHVAC.CompIndex);
                    }
                    if (SysOutputProvided > 0.0) {
                        break;
                    }
                }
            }
        }

        if (SysOutputProvided > 0.0) {
            for (auto &afnNode : afnZoneInfo.Node) {
                for (auto const &afnHVAC : afnNode.HVAC) {
                    if (afnHVAC.zoneEquipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                        afnNode.SysDepZoneLoadsLaggedOld += afnHVAC.SupplyFraction * SysOutputProvided;
                    }
                }
            }
        }

    } // SumSystemDepResponseForNode

    //*****************************************************************************************

} // namespace RoomAir

} // namespace EnergyPlus
