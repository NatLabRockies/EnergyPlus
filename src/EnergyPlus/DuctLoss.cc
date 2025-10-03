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
#include <string>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DuctLoss.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace DuctLoss {
    // Module containing the duct loss simulation routines

    constexpr std::string_view cCMO_DuctLossConduction = "Duct:Loss:Conduction";
    constexpr std::string_view cCMO_DuctLossLeakage = "Duct:Loss:Leakage";
    constexpr std::string_view cCMO_DuctLossMakeupAir = "Duct:Loss:MakeupAir";

    void SimulateDuctLoss(EnergyPlusData &state, AirPath AirPathWay, int PathNum)
    {

        if (state.dataDuctLoss->GetDuctLossInputFlag) { // First time subroutine has been entered
            GetDuctLossInput(state);
            state.dataDuctLoss->GetDuctLossInputFlag = false;
        }

        if (PathNum == 0) {
            return;
        }

        if (state.dataAirLoop->AirLoopInputsFilled) {
            InitDuctLoss(state);
            if (state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate == 0.0) {
                state.dataDuctLoss->ZoneSen = 0.0;
                state.dataDuctLoss->ZoneLat = 0.0;
                state.dataDuctLoss->ZoneSen = 0.0;
                state.dataDuctLoss->ZoneLat = 0.0;
                state.dataDuctLoss->SysSen = 0.0;
                state.dataDuctLoss->SysLat = 0.0;
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    state.dataDuctLoss->ductloss(DuctLossNum).Qsen = 0.0;
                    state.dataDuctLoss->ductloss(DuctLossNum).Qlat = 0.0;
                    state.dataDuctLoss->ductloss(DuctLossNum).QsenSL = 0.0;
                    state.dataDuctLoss->ductloss(DuctLossNum).QlatSL = 0.0;
                }
                return;
            }
        }

        if (!state.dataDuctLoss->AirLoopConnectionFlag && state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate > 0.0) {
            if (AirPathWay == AirPath::Supply) {
                // Supply trunk
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupplyTrunk) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Supply trunk leak
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupLeakTrunk) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Supply branch
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupplyBranch) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Supply branch leak
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupLeakBranch) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
            }
            // Return branch leak
            if (AirPathWay == AirPath::Return) {
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::RetLeakBranch) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Return branch
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::ReturnBranch) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Return trunk leak
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::RetLeakTrunk) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Return trunk
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::ReturnTrunk) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
                // Makeup air
                for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                    if (state.dataDuctLoss->ductloss(DuctLossNum).LossType == DuctLossType::MakeupAir) {
                        state.dataDuctLoss->ductloss(DuctLossNum).CalcDuctLoss(state, DuctLossNum);
                    }
                }
            }
            if (AirPathWay == AirPath::Return) {
                ReturnPathUpdate(state, PathNum);
            }
            if (AirPathWay == AirPath::Supply) {
                SupplyPathUpdate(state, PathNum);
                ReportDuctLoss(state);
            }
        }
    }

    void GetDuctLossInput(EnergyPlusData &state)
    {

        // PURPOSE OF THIS SUBROUTINE:
        // This function is to get inputs of ductloss objects

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetDuctLossInput: "); // include trailing bla

        std::string CurrentModuleObject; // for ease in getting objects
        std::string LinkageName;         // Name of the Duct linkage
        bool errorsFound(false);

        int NumDuctLossConduction = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DuctLossConduction);
        int NumDuctLossLeakage = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DuctLossLeakage);
        int NumDuctLossMakeupAir = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DuctLossMakeupAir);
        if (NumDuctLossConduction + NumDuctLossLeakage + NumDuctLossMakeupAir == 0) {
            return;
        }
        int NumOfAirloops = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC");
        if (NumOfAirloops == 0) {
            ShowSevereError(state, "The simple duct model allows a single AirLoop. No AirLoopHVAC object is found");
            errorsFound = true;
        }
        int NumOfSplitters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ZoneSplitter");
        if (NumOfSplitters == 0) {
            ShowSevereError(state, "The simple duct model allows a single AirLoopHVAC:ZoneSplitter. No AirLoopHVAC:ZoneSplitter object is found");
            errorsFound = true;
        } else if (NumOfSplitters > 1) {
            ShowSevereError(state,
                            "The simple duct model allows a single AirLoopHVAC:ZoneSplitter. Multiple objects of AirLoopHVAC:ZoneSplitter are found");
            errorsFound = true;
        }
        int NumOfMixers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ZoneMixer");
        if (NumOfMixers == 0) {
            ShowSevereError(state, "The simple duct model allows a single AirLoopHVAC:ZoneMixers. No AirLoopHVAC:ZoneNumOfMixer object is found");
            errorsFound = true;
        } else if (NumOfMixers > 1) {
            ShowSevereError(state,
                            "The simple duct model allows a single AirLoopHVAC:ZonefMixer. Multiple objects of AirLoopHVAC:ZoneMixer are found");
            errorsFound = true;
        }
        if (errorsFound) {
            ShowFatalError(state, "GetDuctLossLeakageInput: Previous errors cause termination.");
        }

        CurrentModuleObject = "Duct:Loss:Conduction";

        auto instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            std::string cFieldName;
            int DuctLossCondNum = 0;
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {

                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, thisObjectName);
                ++DuctLossCondNum;
                DuctLossComp thisDuctLoss;

                thisDuctLoss.Name = Util::makeUPPER(thisObjectName);
                thisDuctLoss.AirLoopName = Util::makeUPPER(fields.at("airloophvac_name").get<std::string>());
                LinkageName = Util::makeUPPER(fields.at("airflownetwork_distribution_linkage_name").get<std::string>());
                thisDuctLoss.LinkageNum = Util::FindItemInList(LinkageName, state.afn->AirflowNetworkLinkageData);
                if (thisDuctLoss.LinkageNum == 0) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" {} not found: {}",
                                           CurrentModuleObject,
                                           thisDuctLoss.Name,
                                           "Airflownetwork:Distribution:Linkage = ",
                                           LinkageName));
                    errorsFound = true;
                }
                std::string EnvType = Util::makeUPPER(fields.at("environment_type").get<std::string>());
                if (Util::SameString(EnvType, "SCHEDULE")) {
                    thisDuctLoss.EnvType = EnvironmentType::Schedule;
                } else if (Util::SameString(EnvType, "ZONE")) {
                    thisDuctLoss.EnvType = EnvironmentType::Zone;
                } else {
                    ShowSevereError(state,
                                    format("{}, \"{}\" {} not found: {}", CurrentModuleObject, thisDuctLoss.Name, "Environment Type = ", EnvType));
                    errorsFound = true;
                }
                if (thisDuctLoss.EnvType == EnvironmentType::Schedule) {
                    thisDuctLoss.ScheduleNameT = Util::makeUPPER(fields.at("ambient_temperature_schedule_name").get<std::string>());
                    if ((thisDuctLoss.tambSched = Sched::GetSchedule(state, thisDuctLoss.ScheduleNameT)) == nullptr) {
                        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, thisDuctLoss.ScheduleNameT};
                        ShowSevereItemNotFound(state, eoh, "ambient_temperature_schedule_name", thisDuctLoss.ScheduleNameT);
                        errorsFound = true;
                    }
                    thisDuctLoss.ScheduleNameW = Util::makeUPPER(fields.at("ambient_humidity_ratio_schedule_name").get<std::string>());
                    if ((thisDuctLoss.wambSched = Sched::GetSchedule(state, thisDuctLoss.ScheduleNameW)) == nullptr) {
                        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, thisDuctLoss.ScheduleNameW};
                        ShowSevereItemNotFound(state, eoh, "ambient_humidity_ratio_schedule_name", thisDuctLoss.ScheduleNameW);
                        errorsFound = true;
                    }
                }
                if (thisDuctLoss.EnvType == EnvironmentType::Zone) {
                    thisDuctLoss.ZoneName = Util::makeUPPER(fields.at("ambient_zone_name").get<std::string>());
                    thisDuctLoss.ZoneNum = Util::FindItemInList(thisDuctLoss.ZoneName, state.dataHeatBal->Zone);
                }
                thisDuctLoss.LossType = DuctLossType::Conduction;
                state.dataDuctLoss->ductloss.push_back(thisDuctLoss);
            }
            if (errorsFound) {
                ShowFatalError(state, "GetDuctLossConductionInput: Previous errors cause termination.");
            }
            state.dataDuctLoss->NumOfDuctLosses = DuctLossCondNum;
        }

        CurrentModuleObject = "Duct:Loss:Leakage";

        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            std::string cFieldName;
            int DuctLossLeakNum = 0;
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {

                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, thisObjectName);
                ++DuctLossLeakNum;
                DuctLossComp thisDuctLoss;

                thisDuctLoss.Name = Util::makeUPPER(thisObjectName);
                thisDuctLoss.AirLoopName = Util::makeUPPER(fields.at("airloophvac_name").get<std::string>());
                LinkageName = Util::makeUPPER(fields.at("airflownetwork_distribution_linkage_name").get<std::string>());
                thisDuctLoss.LinkageNum = Util::FindItemInList(LinkageName, state.afn->AirflowNetworkLinkageData);
                if (thisDuctLoss.LinkageNum == 0) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" {} not found: {}",
                                           CurrentModuleObject,
                                           thisDuctLoss.Name,
                                           "Airflownetwork:Distribution:Linkage = ",
                                           LinkageName));
                    errorsFound = true;
                }
                thisDuctLoss.LossType = DuctLossType::Leakage;
                state.dataDuctLoss->ductloss.push_back(thisDuctLoss);
            }
            state.dataDuctLoss->NumOfDuctLosses = state.dataDuctLoss->NumOfDuctLosses + DuctLossLeakNum;
            if (errorsFound) {
                ShowFatalError(state, "GetDuctLossLeakageInput: Previous errors cause termination.");
            }
        }

        CurrentModuleObject = "Duct:Loss:MakeupAir";

        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            std::string cFieldName;
            int DuctLossMakeNum = 0;
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {

                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, thisObjectName);
                ++DuctLossMakeNum;
                DuctLossComp thisDuctLoss;

                thisDuctLoss.Name = Util::makeUPPER(thisObjectName);
                thisDuctLoss.AirLoopName = Util::makeUPPER(fields.at("airloophvac_name").get<std::string>());
                LinkageName = Util::makeUPPER(fields.at("airflownetwork_distribution_linkage_name").get<std::string>());
                thisDuctLoss.LinkageNum = Util::FindItemInList(LinkageName, state.afn->AirflowNetworkLinkageData);
                if (thisDuctLoss.LinkageNum == 0) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" {} not found: {}",
                                           CurrentModuleObject,
                                           thisDuctLoss.Name,
                                           "Airflownetwork:Distribution:Linkage = ",
                                           LinkageName));
                    errorsFound = true;
                }
                thisDuctLoss.LossType = DuctLossType::MakeupAir;
                state.dataDuctLoss->ductloss.push_back(thisDuctLoss);
            }
            state.dataDuctLoss->NumOfDuctLosses = state.dataDuctLoss->NumOfDuctLosses + DuctLossMakeNum;
            if (errorsFound) {
                ShowFatalError(state, "GetDuctLossMakeupAirInput: Previous errors cause termination.");
            }
        }

        // May need to remove ????
        if (state.afn->AirflowNetworkGetInputFlag) {
            state.afn->get_input();
            state.afn->AirflowNetworkGetInputFlag = false;
            // return;
        }
        // Validation of AirLoopHVAC
        bool airLoopFound = true;
        for (int DuctLossNum = 2; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
            if (!Util::SameString(state.dataDuctLoss->ductloss(1).AirLoopName, state.dataDuctLoss->ductloss(DuctLossNum).AirLoopName)) {
                airLoopFound = false;
            }
        }
        if (!airLoopFound) {
            ShowSevereError(state, "Multiple AirLoopHVAC names are found. A single AirLoopHVAC is required");
            errorsFound = true;
        }
        state.dataDuctLoss->SplitterNum = 1;
        state.dataDuctLoss->MixerNum = 1;
        // Allocate
        state.dataDuctLoss->ZoneSen.allocate(state.dataGlobal->NumOfZones);
        state.dataDuctLoss->ZoneLat.allocate(state.dataGlobal->NumOfZones);
        state.dataDuctLoss->SubTypeSimuFlag.dimension(8, false);
        // outputs
        state.afn->AirflowNetworkNodeSimu.allocate(state.afn->AirflowNetworkNumOfNodes); // Node simulation variable in air distribution system
        state.afn->AirflowNetworkLinkSimu.allocate(state.afn->AirflowNetworkNumOfLinks); // Link simulation variable in air distribution system

        for (int NodeNum = 1; NodeNum <= state.afn->AirflowNetworkNumOfNodes; ++NodeNum) {
            SetupOutputVariable(state,
                                "Duct Loss Node Temperature",
                                Constant::Units::C,
                                state.afn->AirflowNetworkNodeSimu(NodeNum).TZ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.afn->AirflowNetworkNodeData(NodeNum).Name);
            SetupOutputVariable(state,
                                "Duct Loss Node Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                state.afn->AirflowNetworkNodeSimu(NodeNum).WZ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.afn->AirflowNetworkNodeData(NodeNum).Name);
        }

        for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
            SetupOutputVariable(state,
                                "Duct Loss Sensible Loss Rate",
                                Constant::Units::W,
                                state.dataDuctLoss->ductloss(DuctLossNum).Qsen,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataDuctLoss->ductloss(DuctLossNum).Name);
            SetupOutputVariable(state,
                                "Duct Loss Latent Loss Rate",
                                Constant::Units::kgWater_s,
                                state.dataDuctLoss->ductloss(DuctLossNum).Qlat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataDuctLoss->ductloss(DuctLossNum).Name);
        }

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ZoneNum++) {
            SetupOutputVariable(state,
                                "Zone Added Sensible Rate Due to Duct Loss",
                                Constant::Units::W,
                                state.dataDuctLoss->ZoneSen(ZoneNum),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Added Latent Rate Due to Duct Loss",
                                Constant::Units::kgWater_s,
                                state.dataDuctLoss->ZoneLat(ZoneNum),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
        }

        if (state.dataDuctLoss->NumOfDuctLosses > 0) {
            state.dataDuctLoss->DuctLossSimu = true;
        }
    }

    void DuctLossComp::CalcDuctLoss(EnergyPlusData &state, int Index)
    {
        auto &thisDuctComp = state.dataDuctLoss->ductloss(Index);

        switch (thisDuctComp.LossType) {
        case DuctLossType::Conduction: {
            CalcConduction(state);
        } break;
        case DuctLossType::Leakage: {
            CalcLeakage(state);
        } break;
        case DuctLossType::MakeupAir: {
            CalcMakeupAir(state);
        } break;
        default:
            break;
        }
    }

    void DuctLossComp::CalcConduction(EnergyPlusData &state)
    {
        Real64 MassFlowRate;
        Real64 Tamb;
        Real64 Wamb;
        Real64 Tin;
        Real64 Tout;
        Real64 Win;
        Real64 Wout;
        Real64 CpAir;
        Real64 enthalpy;
        int NodeNum1;
        int NodeNum2;
        int NodeNum;
        int TypeNum = state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(this->LinkageNum).CompNum).TypeNum;
        Real64 DuctSurfArea = state.afn->DisSysCompDuctData(TypeNum).L * state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter * Constant::Pi;
        Real64 UThermal = state.afn->DisSysCompDuctData(TypeNum).UThermConduct;
        Real64 UMoisture = state.afn->DisSysCompDuctData(TypeNum).UMoisture;

        this->Qsen = 0.0;
        this->Qlat = 0.0;

        if (this->EnvType == EnvironmentType::Zone && this->ZoneNum > 0) {
            Tamb = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->ZoneNum).MAT;
            Wamb = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->ZoneNum).airHumRat;
        } else {
            Tamb = this->tambSched->getCurrentVal();
            Wamb = this->wambSched->getCurrentVal();
        }
        NodeNum1 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[0];
        NodeNum2 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[1];

        switch (this->LossSubType) {
        case DuctLossSubType::SupplyTrunk: {
            NodeNum = state.afn->DisSysNodeData(NodeNum1).EPlusNodeNum;
            MassFlowRate = state.dataLoopNodes->Node(NodeNum).MassFlowRate;
            Tin = state.dataLoopNodes->Node(NodeNum).Temp;
            Win = state.dataLoopNodes->Node(NodeNum).HumRat;
            state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ = Tin;
            state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ = Win;
            CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(NodeNum).HumRat);
            Tout = Tamb + (Tin - Tamb) * General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir));
            Wout = Wamb + (Win - Wamb) * General::epexp(-UMoisture * DuctSurfArea, MassFlowRate);
            this->Qsen = -MassFlowRate * CpAir * (Tamb - Tin) * (1.0 - General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir)));
            this->Qlat = -MassFlowRate * (Wamb - Win) * (1.0 - General::epexp(-UMoisture * DuctSurfArea, MassFlowRate));
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
            if (!state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyBranch) + 1)) {
                enthalpy = Psychrometrics::PsyHFnTdbW(Tout, Wout);
                for (int OutNodeNum = 1; OutNodeNum <= state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).NumOutletNodes;
                     OutNodeNum++) {
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).OutletNode(OutNodeNum))
                        .Temp = Tout;
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).OutletNode(OutNodeNum))
                        .HumRat = Wout;
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).OutletNode(OutNodeNum))
                        .Enthalpy = enthalpy;
                    state.dataLoopNodes->Node(state.dataDuctLoss->ZoneEquipInletNodes(OutNodeNum)).Temp = Tout;
                    state.dataLoopNodes->Node(state.dataDuctLoss->ZoneEquipInletNodes(OutNodeNum)).HumRat = Wout;
                    state.dataLoopNodes->Node(state.dataDuctLoss->ZoneEquipInletNodes(OutNodeNum)).Enthalpy = enthalpy;
                }
            }
        } break;
        case DuctLossSubType::SupplyBranch: {
            NodeNum = state.afn->DisSysNodeData(NodeNum2).EPlusNodeNum;
            MassFlowRate = state.dataLoopNodes->Node(NodeNum).MassFlowRate;
            if (state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyTrunk) + 1)) {
                Tin = state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ;
                Win = state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ;
            } else {
                Tin = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).Temp;
                Win = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).HumRat;
            }
            CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(NodeNum).HumRat);
            Tout = Tamb + (Tin - Tamb) * General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir));
            Wout = Wamb + (Win - Wamb) * General::epexp(-UMoisture * DuctSurfArea, MassFlowRate);
            this->Qsen = -MassFlowRate * CpAir * (Tamb - Tin) * (1.0 - General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir)));
            this->Qlat = -MassFlowRate * (Wamb - Win) * (1.0 - General::epexp(-UMoisture * DuctSurfArea, MassFlowRate));
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
        } break;
        case DuctLossSubType::ReturnTrunk: {
            NodeNum = state.afn->DisSysNodeData(NodeNum2).EPlusNodeNum;
            MassFlowRate = state.dataLoopNodes->Node(NodeNum).MassFlowRate;
            if (state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::ReturnBranch) + 1)) {
                Tin = state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ;
                Win = state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ;
            } else {
                Tin = state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).OutletTemp;
                Win = state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).OutletHumRat;
            }
            CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(NodeNum).HumRat);
            Tout = Tamb + (Tin - Tamb) * General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir));
            Wout = Wamb + (Win - Wamb) * General::epexp(-UMoisture * DuctSurfArea, (MassFlowRate * CpAir));
            this->Qsen = -MassFlowRate * CpAir * (Tamb - Tin) * (1.0 - General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir)));
            this->Qlat = -MassFlowRate * (Wamb - Win) * (1.0 - General::epexp(-UMoisture * DuctSurfArea, MassFlowRate));
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
        } break;
        case DuctLossSubType::ReturnBranch: {
            NodeNum = state.afn->DisSysNodeData(NodeNum1).EPlusNodeNum;
            MassFlowRate = state.dataLoopNodes->Node(NodeNum).MassFlowRate;
            Tin = state.dataLoopNodes->Node(NodeNum).Temp;
            state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ = Tin;
            Win = state.dataLoopNodes->Node(NodeNum).HumRat;
            state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ = Win;
            CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(NodeNum).HumRat);
            Tout = Tamb + (Tin - Tamb) * General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir));
            Wout = Wamb + (Win - Wamb) * General::epexp(-UMoisture * DuctSurfArea, MassFlowRate);
            this->Qsen = -MassFlowRate * CpAir * (Tamb - Tin) * (1.0 - General::epexp(-UThermal * DuctSurfArea, (MassFlowRate * CpAir)));
            this->Qlat = -MassFlowRate * (Wamb - Win) * (1.0 - General::epexp(-UMoisture * DuctSurfArea, MassFlowRate));
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
            if (!state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::ReturnTrunk) + 1)) {
                state.dataLoopNodes->Node(state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).OutletNode).Temp = Tout;
                state.dataLoopNodes->Node(state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).OutletNode).HumRat = Wout;
            }
        } break;

        default:
            break;
        }
    }

    void DuctLossComp::CalcLeakage(EnergyPlusData &state)
    {
        Real64 MassFlowRate;
        Real64 Tin;
        Real64 Tout;
        Real64 Win;
        Real64 Wout;
        Real64 CpAir;
        int NodeNum1;
        int NodeNum2;
        int NodeNum;
        NodeNum1 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[0];
        NodeNum2 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[1];

        int TypeNum = state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(this->LinkageNum).CompNum).TypeNum;
        Real64 LeakRatio = state.afn->DisSysCompELRData(TypeNum).ELR;

        this->Qsen = 0.0;
        this->Qlat = 0.0;
        this->QsenSL = 0.0;
        this->QlatSL = 0.0;

        switch (this->LossSubType) {
        case DuctLossSubType::SupLeakTrunk: {
            // No need to calculate equivalent T and W. Keep the same value
            Tin = state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ;
            Win = state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ;
            CpAir = Psychrometrics::PsyCpAirFnW(Win);
            MassFlowRate = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate;
            // Zone only
            this->Qsen = MassFlowRate * CpAir * LeakRatio *
                         (Tin - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).MAT);
            this->Qlat =
                MassFlowRate * LeakRatio *
                (Win - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).airHumRat);
            this->QsenSL =
                MassFlowRate * CpAir * LeakRatio * (Tin - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDuctLoss->CtrlZoneNum).MAT);
            this->QlatSL =
                MassFlowRate * LeakRatio * (Win - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDuctLoss->CtrlZoneNum).airHumRat);
        } break;
        case DuctLossSubType::SupLeakBranch: {
            Tin = state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ;
            Win = state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ;
            CpAir = Psychrometrics::PsyCpAirFnW(Win);
            MassFlowRate = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate;
            this->Qsen = MassFlowRate * CpAir * LeakRatio *
                         (Tin - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).MAT);
            this->Qlat =
                MassFlowRate * LeakRatio *
                (Win - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).airHumRat);
            this->QsenSL =
                MassFlowRate * CpAir * LeakRatio * (Tin - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDuctLoss->CtrlZoneNum).MAT);
            this->QlatSL =
                MassFlowRate * LeakRatio * (Win - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDuctLoss->CtrlZoneNum).airHumRat);
        } break;
        case DuctLossSubType::RetLeakTrunk: {
            MassFlowRate = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate;
            Tout = state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ * (1.0 - LeakRatio) +
                   state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).MAT * LeakRatio;
            Wout =
                state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ * (1.0 - LeakRatio) +
                state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).airHumRat * LeakRatio;
            CpAir = Psychrometrics::PsyCpAirFnW(Wout);
            this->Qsen = MassFlowRate * CpAir * LeakRatio *
                         (state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).MAT -
                          state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ);
            this->Qlat = MassFlowRate * LeakRatio *
                         (state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).airHumRat -
                          state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ);
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
        } break;
        case DuctLossSubType::RetLeakBranch: {
            NodeNum = state.afn->DisSysNodeData(NodeNum2).EPlusNodeNum;
            MassFlowRate = state.dataLoopNodes->Node(NodeNum).MassFlowRate;
            Tout = state.dataLoopNodes->Node(this->RetLeakZoneNum).Temp * (1.0 - LeakRatio) +
                   state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).MAT * LeakRatio;
            Wout =
                state.dataLoopNodes->Node(this->RetLeakZoneNum).HumRat * (1.0 - LeakRatio) +
                state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).airHumRat * LeakRatio;
            CpAir = Psychrometrics::PsyCpAirFnW((Wout + state.dataLoopNodes->Node(NodeNum).HumRat) / 2.0);
            this->Qsen = MassFlowRate * CpAir * LeakRatio *
                         (state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).MAT - Tout);
            this->Qlat =
                MassFlowRate * LeakRatio *
                (state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).airHumRat - Wout);
            state.afn->AirflowNetworkNodeSimu(NodeNum2).TZ = Tout;
            state.afn->AirflowNetworkNodeSimu(NodeNum2).WZ = Wout;
            state.dataLoopNodes->Node(NodeNum).Temp = Tout;
            state.dataLoopNodes->Node(NodeNum).HumRat = Wout;
        } break;
        default:
            break;
        }
    }

    void DuctLossComp::CalcMakeupAir(EnergyPlusData &state)
    {
        int NodeNum1;
        int NodeNum2;
        Real64 Tin;
        Real64 Win;
        Real64 CpAir;

        NodeNum1 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[0];
        NodeNum2 = state.afn->AirflowNetworkLinkageData(this->LinkageNum).NodeNums[1];

        int TypeNum = state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(this->LinkageNum).CompNum).TypeNum;
        Real64 LeakRatio = state.afn->DisSysCompELRData(TypeNum).ELR;

        this->Qsen = 0.0;
        this->Qlat = 0.0;

        Tin = state.afn->AirflowNetworkNodeSimu(NodeNum1).TZ;
        Win = state.afn->AirflowNetworkNodeSimu(NodeNum1).WZ;
        CpAir = Psychrometrics::PsyCpAirFnW(Win);
        // Zone only
        this->Qsen = state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate * CpAir * LeakRatio *
                     (Tin - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).MAT);
        this->Qlat =
            state.dataLoopNodes->Node(state.dataDuctLoss->AirLoopInNodeNum).MassFlowRate * LeakRatio *
            (Win - state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).airHumRat);
        ZoneNum = state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum;
        if (ZoneNum > 0) {
            state.dataDuctLoss->ZoneSen(ZoneNum) += this->Qsen;
            state.dataDuctLoss->ZoneLat(ZoneNum) += this->Qlat;
        }
    }

    void InitDuctLoss(EnergyPlusData &state)
    {
        bool errorsFound(false);
        std::string CurrentModuleObject;

        if (state.dataDuctLoss->AirLoopConnectionFlag) {

            for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                auto &thisDuctLoss(state.dataDuctLoss->ductloss(DuctLossNum));

                thisDuctLoss.AirLoopNum = Util::FindItemInList(thisDuctLoss.AirLoopName, state.dataAirSystemsData->PrimaryAirSystems);
                if (thisDuctLoss.LossType == DuctLossType::Conduction) {
                    CurrentModuleObject = cCMO_DuctLossConduction;
                } else if (thisDuctLoss.LossType == DuctLossType::Leakage) {
                    CurrentModuleObject = cCMO_DuctLossLeakage;
                } else {
                    CurrentModuleObject = cCMO_DuctLossMakeupAir;
                }
                if (thisDuctLoss.AirLoopNum == 0) {
                    ShowSevereError(
                        state,
                        format("{}, \"{}\" {} not found: {}", CurrentModuleObject, thisDuctLoss.Name, "AirLoopHVAC = ", thisDuctLoss.AirLoopName));
                    errorsFound = true;
                }
            }

            // Setup duct subtype
            int AFNNodeNum1;
            int NodeNum1;
            int AFNNodeNum2;
            int NodeNum2;
            for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
                auto &thisDuctLoss(state.dataDuctLoss->ductloss(DuctLossNum));
                if (thisDuctLoss.LossType == DuctLossType::Conduction) {
                    AFNNodeNum1 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[0];
                    AFNNodeNum2 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[1];
                    if (state.afn->DisSysNodeData(AFNNodeNum1).EPlusName != "") {
                        NodeNum1 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum1).EPlusName, state.dataLoopNodes->NodeID);
                        state.afn->DisSysNodeData(AFNNodeNum1).EPlusNodeNum = NodeNum1;
                    } else {
                        NodeNum1 = 0;
                    }
                    if (NodeNum1 > 0) {
                        if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "AirLoopHVAC:ZoneSplitter")) {
                            if (NodeNum1 == state.dataAirLoop->AirToZoneNodeInfo(thisDuctLoss.AirLoopNum).ZoneEquipSupplyNodeNum(1)) {
                                thisDuctLoss.LossSubType = DuctLossSubType::SupplyTrunk;
                                state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyTrunk) + 1) = true;
                                state.dataDuctLoss->AirLoopInNodeNum = NodeNum1;
                            }
                        } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "AirLoopHVAC:ZoneMixer")) {
                            for (int InNodeNum = 1; InNodeNum <= state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).NumInletNodes;
                                 InNodeNum++) {
                                if (NodeNum1 == state.dataMixerComponent->MixerCond(state.dataDuctLoss->MixerNum).InletNode(InNodeNum) + 1) {
                                    thisDuctLoss.LossSubType = DuctLossSubType::ReturnBranch;
                                    state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::ReturnBranch)) = true;
                                    break;
                                }
                            }
                        }
                    } else {
                        if (state.afn->DisSysNodeData(AFNNodeNum2).EPlusName != "") {
                            NodeNum2 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum2).EPlusName, state.dataLoopNodes->NodeID);
                            state.afn->DisSysNodeData(AFNNodeNum2).EPlusNodeNum = NodeNum2;
                        } else {
                            NodeNum2 = 0;
                        }
                        if (NodeNum2 > 0) {
                            if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "AirLoopHVAC:ZoneSplitter")) {
                                for (int OutNodeNum = 1;
                                     OutNodeNum <= state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).NumOutletNodes;
                                     OutNodeNum++) {
                                    if (NodeNum2 ==
                                        state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).OutletNode(OutNodeNum)) {
                                        thisDuctLoss.LossSubType = DuctLossSubType::SupplyBranch;
                                        state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyBranch) + 1) = true;
                                        // find zone inlet node
                                        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ZoneNum++) {
                                            for (int inletNum = 1; inletNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes;
                                                 ++inletNum) {
                                                if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum) > 0) {
                                                    if (state.dataDefineEquipment
                                                            ->AirDistUnit(state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum))
                                                            .InletNodeNum == NodeNum2) {
                                                        state.afn->DisSysNodeData(AFNNodeNum2).EPlusZoneInletNodeNum =
                                                            state.dataDefineEquipment
                                                                ->AirDistUnit(state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum))
                                                                .OutletNodeNum;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                            } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "AirLoopHVAC:ZoneMixer")) {
                                thisDuctLoss.LossSubType = DuctLossSubType::ReturnTrunk;
                                state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::ReturnTrunk) + 1) = true;
                            }
                        }
                    }
                }
                if (thisDuctLoss.LossType == DuctLossType::Leakage) {
                    AFNNodeNum1 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[0];
                    AFNNodeNum2 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[1];
                    if (state.afn->DisSysNodeData(AFNNodeNum1).EPlusName != "" && state.afn->DisSysNodeData(AFNNodeNum1).EPlusType != "ZONE") {
                        NodeNum1 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum1).EPlusName, state.dataLoopNodes->NodeID);
                        // Zone inlet
                        state.afn->DisSysNodeData(AFNNodeNum1).EPlusNodeNum = NodeNum1;
                        if (NodeNum1 > 0) {
                            if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "Zone") ||
                                Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OutdoorAir:NodeList") ||
                                Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OutdoorAir:Node")) {
                                thisDuctLoss.LossSubType = DuctLossSubType::SupLeakBranch;
                                state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupLeakBranch) + 1) = true;
                            }
                        }
                    } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "AirLoopHVAC:ZoneSplitter")) {
                        if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "Zone") ||
                            Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OutdoorAir:NodeList") ||
                            Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OutdoorAir:Node")) {
                            thisDuctLoss.LossSubType = DuctLossSubType::SupLeakTrunk;
                            state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupLeakTrunk) + 1) = true;
                        }
                    } else {
                        // DO we need nodenum=0?
                        NodeNum1 = 0;
                    }
                    if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "Zone") ||
                        Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "OutdoorAir:NodeList") ||
                        Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "OutdoorAir:Node")) {
                        if (state.afn->DisSysNodeData(AFNNodeNum2).EPlusName != "") {
                            // Zone outlet
                            NodeNum2 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum2).EPlusName, state.dataLoopNodes->NodeID);
                            if (NodeNum2 > 0) {
                                for (int NumEquip = 1; NumEquip <= state.dataZoneEquip->NumOfZoneEquipLists; NumEquip++) {
                                    for (int NumReturn = 1; NumReturn <= state.dataZoneEquip->ZoneEquipConfig(NumEquip).NumReturnNodes; NumReturn++) {
                                        if (state.dataZoneEquip->ZoneEquipConfig(NumEquip).ReturnNode(NumReturn) == NodeNum2) {
                                            thisDuctLoss.LossSubType = DuctLossSubType::RetLeakBranch;
                                            state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::RetLeakBranch) + 1) = true;
                                            thisDuctLoss.RetLeakZoneNum = state.dataZoneEquip->ZoneEquipConfig(NumEquip).ZoneNode;
                                        }
                                    }
                                }
                            }
                        } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "AirLoopHVAC:ZoneMixer")) {
                            thisDuctLoss.LossSubType = DuctLossSubType::RetLeakTrunk;
                            state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::RetLeakTrunk) + 1) = true;
                        }
                    }
                }

                if (thisDuctLoss.LossType == DuctLossType::MakeupAir) {
                    AFNNodeNum1 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[0];
                    AFNNodeNum2 = state.afn->AirflowNetworkLinkageData(thisDuctLoss.LinkageNum).NodeNums[1];
                    if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "ZONE")) {
                        state.afn->AirflowNetworkNodeData(AFNNodeNum1).EPlusZoneNum =
                            Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum1).EPlusName, state.dataHeatBal->Zone);
                    } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "OUTDOORAIR:NODELIST") ||
                               Util::SameString(state.afn->DisSysNodeData(AFNNodeNum1).EPlusType, "OUTDOORAIR:NODE")) {
                        NodeNum1 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum1).EPlusName, state.dataLoopNodes->NodeID);
                        if (NodeNum1 > 0) {
                            state.afn->DisSysNodeData(AFNNodeNum1).EPlusNodeNum = NodeNum1;
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}, \"{}\" {} not found: {}",
                                               "Duct:Loss:MakeupAir",
                                               thisDuctLoss.Name,
                                               "Incorrect input, not Zone, OUTDOORAIR:NODELIST, and OUTDOORAIR:NODE = ",
                                               state.afn->DisSysNodeData(AFNNodeNum1).Name));
                        errorsFound = true;
                    }
                    if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "ZONE")) {
                        state.afn->AirflowNetworkNodeData(AFNNodeNum2).EPlusZoneNum =
                            Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum2).EPlusName, state.dataHeatBal->Zone);
                    } else if (Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OUTDOORAIR:NODELIST") ||
                               Util::SameString(state.afn->DisSysNodeData(AFNNodeNum2).EPlusType, "OUTDOORAIR:NODE")) {
                        NodeNum2 = Util::FindItemInList(state.afn->DisSysNodeData(AFNNodeNum2).EPlusName, state.dataLoopNodes->NodeID);
                        if (NodeNum2 > 0) {
                            state.afn->DisSysNodeData(AFNNodeNum1).EPlusNodeNum = NodeNum2;
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}, \"{}\" {} not found: {}",
                                               "Duct:Loss:MakeupAir",
                                               thisDuctLoss.Name,
                                               "Incorrect input, not Zone, OUTDOORAIR:NODELIST, and OUTDOORAIR:NODE = ",
                                               state.afn->DisSysNodeData(AFNNodeNum2).Name));
                        errorsFound = true;
                    }
                }
                if (errorsFound) {
                    ShowFatalError(state, "GetDuctLossMakeupAirInput: Previous errors cause termination.");
                }
            }

            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ZoneNum++) {
                if (state.dataHeatBal->Zone(ZoneNum).IsControlled) {
                    state.dataDuctLoss->CtrlZoneNum = ZoneNum;
                    break;
                }
            }

            SetupOutputVariable(state,
                                "System Added Sensible Rate Due to Duct Loss",
                                Constant::Units::W,
                                state.dataDuctLoss->SysSen,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataAirSystemsData->PrimaryAirSystems(1).Name);
            SetupOutputVariable(state,
                                "System Added Latent Rate Due to Duct Loss",
                                Constant::Units::kgWater_s,
                                state.dataDuctLoss->SysLat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataAirSystemsData->PrimaryAirSystems(1).Name);

            state.dataDuctLoss->AirLoopConnectionFlag = false;

            if (!state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyTrunk) + 1)) {
                state.dataDuctLoss->AirLoopInNodeNum = state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).InletNode;
            }

            if (!state.dataDuctLoss->SubTypeSimuFlag(int(DuctLossSubType::SupplyBranch) + 1)) {
                state.dataDuctLoss->ZoneEquipInletNodes.allocate(
                    state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).NumOutletNodes);
                for (int OutNodeNum = 1; OutNodeNum <= state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).NumOutletNodes;
                     OutNodeNum++) {
                    // find zone inlet node
                    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ZoneNum++) {
                        for (int inletNum = 1; inletNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes; ++inletNum) {
                            if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum) > 0) {
                                if (state.dataDefineEquipment->AirDistUnit(state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum))
                                        .InletNodeNum ==
                                    state.dataSplitterComponent->SplitterCond(state.dataDuctLoss->SplitterNum).OutletNode(OutNodeNum)) {
                                    state.dataDuctLoss->ZoneEquipInletNodes(OutNodeNum) =
                                        state.dataDefineEquipment
                                            ->AirDistUnit(state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeADUNum(inletNum))
                                            .OutletNodeNum;
                                }
                            }
                        }
                    }
                }
            }
        }

        // assign node T and W
        for (int NodeNum = 1; NodeNum <= state.afn->AirflowNetworkNumOfNodes; ++NodeNum) {
            if (state.afn->AirflowNetworkNodeData(NodeNum).EPlusZoneNum > 0) {
                state.afn->AirflowNetworkNodeSimu(NodeNum).TZ =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum).EPlusZoneNum).MAT;
                state.afn->AirflowNetworkNodeSimu(NodeNum).WZ =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum).EPlusZoneNum).airHumRat;
            }
            if (state.afn->AirflowNetworkNodeData(NodeNum).NodeTypeNum == 1) {
                state.afn->AirflowNetworkNodeData(NodeNum).EPlusNodeNum = state.afn->DisSysNodeData(NodeNum).EPlusNodeNum;
                state.afn->AirflowNetworkNodeSimu(NodeNum).TZ =
                    state.dataLoopNodes->Node(state.afn->AirflowNetworkNodeData(NodeNum).EPlusNodeNum).Temp;
                state.afn->AirflowNetworkNodeSimu(NodeNum).WZ =
                    state.dataLoopNodes->Node(state.afn->AirflowNetworkNodeData(NodeNum).EPlusNodeNum).HumRat;
            }
        }
    }

    void ReportDuctLoss(EnergyPlusData &state)
    {
        int ZoneNum;

        state.dataDuctLoss->ZoneSen = 0.0;
        state.dataDuctLoss->ZoneLat = 0.0;
        state.dataDuctLoss->SysSen = 0.0;
        state.dataDuctLoss->SysLat = 0.0;

        for (int DuctLossNum = 1; DuctLossNum <= state.dataDuctLoss->NumOfDuctLosses; DuctLossNum++) {
            if (state.dataDuctLoss->ductloss(DuctLossNum).LossType == DuctLossType::Conduction) {
                ZoneNum = state.dataDuctLoss->ductloss(DuctLossNum).ZoneNum;
                if (ZoneNum > 0) {
                    state.dataDuctLoss->ZoneSen(ZoneNum) += state.dataDuctLoss->ductloss(DuctLossNum).Qsen;
                    state.dataDuctLoss->ZoneLat(ZoneNum) += state.dataDuctLoss->ductloss(DuctLossNum).Qlat;
                }
                if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupplyBranch ||
                    state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupplyTrunk) {
                    state.dataDuctLoss->SysSen += state.dataDuctLoss->ductloss(DuctLossNum).Qsen;
                    state.dataDuctLoss->SysLat += state.dataDuctLoss->ductloss(DuctLossNum).Qlat;
                }
            }
            if (state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupLeakBranch ||
                state.dataDuctLoss->ductloss(DuctLossNum).LossSubType == DuctLossSubType::SupLeakTrunk) {
                ZoneNum = state.afn
                              ->AirflowNetworkNodeData(
                                  state.afn->AirflowNetworkLinkageData(state.dataDuctLoss->ductloss(DuctLossNum).LinkageNum).NodeNums[1])
                              .EPlusZoneNum;
                if (ZoneNum > 0) {
                    state.dataDuctLoss->ZoneSen(ZoneNum) += state.dataDuctLoss->ductloss(DuctLossNum).Qsen;
                    state.dataDuctLoss->ZoneLat(ZoneNum) += state.dataDuctLoss->ductloss(DuctLossNum).Qlat;
                }
                state.dataDuctLoss->SysSen += state.dataDuctLoss->ductloss(DuctLossNum).QsenSL;
                state.dataDuctLoss->SysLat += state.dataDuctLoss->ductloss(DuctLossNum).QlatSL;
            }
        }
        state.dataDuctLoss->ZoneSen(state.dataDuctLoss->CtrlZoneNum) -= state.dataDuctLoss->SysSen;
        state.dataDuctLoss->ZoneLat(state.dataDuctLoss->CtrlZoneNum) -= state.dataDuctLoss->SysLat;
    }

    void ReturnPathUpdate(EnergyPlusData &state, int MixerNum)
    {
        int OutletNode;

        OutletNode = state.dataMixerComponent->MixerCond(MixerNum).OutletNode;

        for (int NodeNum = 1; NodeNum <= state.afn->AirflowNetworkNumOfNodes; ++NodeNum) {
            if (state.afn->DisSysNodeData(NodeNum).EPlusNodeNum == OutletNode) {
                state.dataLoopNodes->Node(OutletNode).Temp = state.afn->AirflowNetworkNodeSimu(NodeNum).TZ;
                state.dataLoopNodes->Node(OutletNode).HumRat = state.afn->AirflowNetworkNodeSimu(NodeNum).WZ;
                state.dataLoopNodes->Node(OutletNode).Enthalpy =
                    Psychrometrics::PsyHFnTdbW(state.afn->AirflowNetworkNodeSimu(NodeNum).TZ, state.afn->AirflowNetworkNodeSimu(NodeNum).WZ);
                break;
            }
        }
        // Set the outlet air nodes of the Mixer
        state.dataMixerComponent->MixerCond(MixerNum).OutletTemp = state.dataLoopNodes->Node(OutletNode).Temp;
        state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat = state.dataLoopNodes->Node(OutletNode).HumRat;
        state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy = state.dataLoopNodes->Node(OutletNode).Enthalpy;
    }

    void SupplyPathUpdate(EnergyPlusData &state, int SplitterNum)
    {
        int OutletNodeNum;

        for (int NodeNum = 1; NodeNum <= state.afn->AirflowNetworkNumOfNodes; ++NodeNum) {
            for (OutletNodeNum = 1; OutletNodeNum <= state.dataSplitterComponent->SplitterCond(SplitterNum).NumOutletNodes; ++OutletNodeNum) {
                if (state.afn->DisSysNodeData(NodeNum).EPlusNodeNum ==
                    state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode(OutletNodeNum)) {
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode(OutletNodeNum)).Temp =
                        state.afn->AirflowNetworkNodeSimu(NodeNum).TZ;
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode(OutletNodeNum)).HumRat =
                        state.afn->AirflowNetworkNodeSimu(NodeNum).WZ;
                    state.dataLoopNodes->Node(state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode(OutletNodeNum)).Enthalpy =
                        Psychrometrics::PsyHFnTdbW(state.afn->AirflowNetworkNodeSimu(NodeNum).TZ, state.afn->AirflowNetworkNodeSimu(NodeNum).WZ);
                    // Extend to air terminal outlet ???? if terminal unit has its coil
                    if (state.afn->DisSysNodeData(NodeNum).EPlusZoneInletNodeNum != state.afn->DisSysNodeData(NodeNum).EPlusNodeNum) {
                        state.dataLoopNodes->Node(state.afn->DisSysNodeData(NodeNum).EPlusZoneInletNodeNum).Temp =
                            state.afn->AirflowNetworkNodeSimu(NodeNum).TZ;
                        state.dataLoopNodes->Node(state.afn->DisSysNodeData(NodeNum).EPlusZoneInletNodeNum).HumRat =
                            state.afn->AirflowNetworkNodeSimu(NodeNum).WZ;
                        state.dataLoopNodes->Node(state.afn->DisSysNodeData(NodeNum).EPlusZoneInletNodeNum).Enthalpy =
                            Psychrometrics::PsyHFnTdbW(state.afn->AirflowNetworkNodeSimu(NodeNum).TZ, state.afn->AirflowNetworkNodeSimu(NodeNum).WZ);
                    }
                    break;
                }
            }
        }
    }

} // namespace DuctLoss

} // namespace EnergyPlus
