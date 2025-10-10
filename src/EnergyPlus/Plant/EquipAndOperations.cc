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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/EquipAndOperations.hh>
#include <EnergyPlus/Plant/Loop.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace DataPlant {

    void ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme(EnergyPlusData &state)
    {
        if (this->oneTimeSetupComplete) {
            return;
        }

        SetupOutputVariable(state,
                            "Supervisory Plant Heat Pump Operation Mode",
                            Constant::Units::unknown,
                            this->Report.AirSourcePlant_OpMode,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Supervisory Plant Auxiliary Boiler Mode",
                            Constant::Units::unknown,
                            this->Report.BoilerAux_OpMode,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Supervisory Plant Operation Polled Building Heating Load",
                            Constant::Units::W,
                            this->Report.BuildingPolledHeatingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Supervisory Plant Operation Polled Building Cooling Load",
                            Constant::Units::W,
                            this->Report.BuildingPolledCoolingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Supervisory Plant Operation Primary Plant Heating Load",
                            Constant::Units::W,
                            this->Report.PrimaryPlantHeatingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Supervisory Plant Operation Primary Plant Cooling Load",
                            Constant::Units::W,
                            this->Report.PrimaryPlantCoolingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        // routine for setup of chiller heater supervisory plant operation scheme
        for (int zoneListNum = 1; zoneListNum <= state.dataHeatBal->NumOfZoneLists; ++zoneListNum) {
            if (this->ZoneListName == state.dataHeatBal->ZoneList(zoneListNum).Name) {

                this->PlantOps.NumOfZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                this->ZonePtrs.allocate(this->PlantOps.NumOfZones);
                for (int zoneNumInList = 1; zoneNumInList <= state.dataHeatBal->ZoneList(zoneListNum).NumOfZones; ++zoneNumInList) {
                    this->ZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                }
            }
        }

        if (state.dataHVACGlobal->NumPrimaryAirSys > 0) {
            this->AirLoopPtrs.allocate(state.dataHVACGlobal->NumPrimaryAirSys); // size to all, if zero then that airloop is not served
            this->PlantOps.NumOfAirLoops = state.dataHVACGlobal->NumPrimaryAirSys;
            this->AirLoopPtrs = 0;
            for (int AirLoopIndex = 1; AirLoopIndex <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopIndex) { // loop over the air systems
                auto &AirToZoneNodeInfo(state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex));
                for (int ZonesPolledIndex = 1; ZonesPolledIndex <= this->PlantOps.NumOfZones; ++ZonesPolledIndex) {
                    for (int ZonesCooledIndex = 1; ZonesCooledIndex <= AirToZoneNodeInfo.NumZonesCooled; ++ZonesCooledIndex) {
                        if (AirToZoneNodeInfo.CoolCtrlZoneNums(ZonesCooledIndex) == this->ZonePtrs(ZonesPolledIndex)) {
                            this->AirLoopPtrs(AirLoopIndex) = AirLoopIndex;
                        }
                    }
                    for (int ZonesHeatedIndex = 1; ZonesHeatedIndex <= AirToZoneNodeInfo.NumZonesHeated; ++ZonesHeatedIndex) {
                        if (AirToZoneNodeInfo.HeatCtrlZoneNums(ZonesHeatedIndex) == this->ZonePtrs(ZonesPolledIndex)) {
                            this->AirLoopPtrs(AirLoopIndex) = AirLoopIndex;
                        }
                    }
                }
            }
        }

        if (this->PlantOps.NumSimultHeatCoolHeatingEquipLists > 0 && this->PlantOps.NumSimultHeatCoolCoolingEquipLists > 0) {
            this->PlantOps.SimultHeatCoolOpAvailable = true;
        }

        this->PlantLoopIndicesBeingSupervised.allocate(state.dataPlnt->TotNumLoops);
        this->PlantLoopIndicesBeingSupervised = 0;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            auto &this_plant_loop(state.dataPlnt->PlantLoop(LoopNum));
            for (int OpNum = 1, OpNum_end = this_plant_loop.NumOpSchemes; OpNum <= OpNum_end; ++OpNum) {
                auto const &this_op_scheme(this_plant_loop.OpScheme(OpNum));
                if (this_op_scheme.Type == OpScheme::ChillerHeaterSupervisory) {
                    this->PlantLoopIndicesBeingSupervised(LoopNum) = LoopNum;
                }
            }
        }

        // find an setup any Plant Load Profile objects on the supervised loops
        int numLoadProfileOnSupervisedLoops = 0;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for any plant load profile on loop demand side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Demand));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::PlantLoadProfile) {
                            ++numLoadProfileOnSupervisedLoops;
                        }
                    }
                }
            }
        }
        this->PlantOps.numPlantLoadProfiles = numLoadProfileOnSupervisedLoops;

        this->PlantLoadProfileComps.allocate(this->PlantOps.numPlantLoadProfiles);
        int loadProfileCompNum = 1;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for any plant load profile on loop demand side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Demand));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::PlantLoadProfile) {
                            PlantLocation foundLoc;
                            foundLoc.loopNum = LoopNum;
                            foundLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
                            foundLoc.branchNum = BranchNum;
                            foundLoc.compNum = CompNum;
                            PlantLoadProfileComps(loadProfileCompNum) = foundLoc;
                            ++loadProfileCompNum;
                        }
                    }
                }
            }
        } // end load profile setup

        // find and setup any boilers on the supervised loops
        int numBoilersOnSupervisedLoops = 0;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for any Boiler on loop supply side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::Boiler_Simple) {
                            ++numBoilersOnSupervisedLoops;
                        }
                    }
                }
            }
        }
        this->PlantOps.numBoilers = numBoilersOnSupervisedLoops;
        this->PlantBoilerComps.allocate(this->PlantOps.numBoilers);
        int BoilerCompNum = 1;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for  boiler on loop supply side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::Boiler_Simple) {
                            PlantLocation foundLoc;
                            foundLoc.loopNum = LoopNum;
                            foundLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
                            foundLoc.branchNum = BranchNum;
                            foundLoc.compNum = CompNum;
                            PlantBoilerComps(BoilerCompNum) = foundLoc;
                            ++BoilerCompNum;
                        }
                    }
                }
            }
        } // end boiler setup

        // find and setup any fluid to fluid heat exchangers on the supervised loops
        this->SecondaryPlantLoopIndicesBeingSupervised.allocate(state.dataPlnt->TotNumLoops);
        this->SecondaryPlantLoopIndicesBeingSupervised = 0;
        int numHXsOnSupervisedLoops = 0;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for any Heat exchangers on loop supply side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg) {
                            ++numHXsOnSupervisedLoops;
                        }
                    }
                }
            }
        }
        this->PlantOps.numPlantHXs = numHXsOnSupervisedLoops;
        this->PlantHXComps.allocate(this->PlantOps.numPlantHXs);
        int HXCompNum = 1;
        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                // search for  boiler on loop supply side
                auto &this_plant_loopside(state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply));
                for (int BranchNum = 1; BranchNum <= this_plant_loopside.TotalBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= this_plant_loopside.Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (this_plant_loopside.Branch(BranchNum).Comp(CompNum).Type == DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg) {
                            PlantLocation foundLoc;
                            foundLoc.loopNum = LoopNum;
                            foundLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
                            foundLoc.branchNum = BranchNum;
                            foundLoc.compNum = CompNum;
                            PlantHXComps(HXCompNum) = foundLoc;
                            ++HXCompNum;

                            // store this loop as being a secondary loop
                            // assuming that since there is a heat exchanger on the supply side of this loop it is a secondary loop.
                            this->SecondaryPlantLoopIndicesBeingSupervised(LoopNum) = LoopNum;
                        }
                    }
                }
            }
        } // end HX setup

        // setup Comp.SetPointNodeNum for machines
        if (this->PlantOps.NumCoolingOnlyEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumCoolingOnlyEquipLists; ++equipListNum) {

                int NumComps = this->CoolingOnlyEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->CoolingOnlyEquipList(equipListNum).Comp(compNum));
                    PlantLocation compLoc;
                    DataPlant::PlantEquipmentType Type =
                        static_cast<DataPlant::PlantEquipmentType>(getEnumValue(PlantEquipTypeNamesUC, Util::makeUPPER(this_equip.TypeOf)));
                    bool errFlag1(false);
                    int NumSearchResults(0);
                    PlantUtilities::ScanPlantLoopsForObject(state, this_equip.Name, Type, compLoc, errFlag1, _, _, NumSearchResults);
                    if (NumSearchResults == 1) {

                        this_equip.LoopNumPtr = compLoc.loopNum;
                        this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                        this_equip.BranchNumPtr = compLoc.branchNum;
                        this_equip.CompNumPtr = compLoc.compNum;
                        this->PlantOps.PrimaryChWLoopIndex = compLoc.loopNum;
                    } else if (NumSearchResults > 1) {
                        bool foundit = false;
                        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
                            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                                int PltSizNum = state.dataPlnt->PlantLoop(LoopNum).PlantSizNum;
                                if (PltSizNum > 0) {
                                    if (state.dataSize->PlantSizData(PltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Cooling) {
                                        int innerNumSearchResults = 0;
                                        PlantUtilities::ScanPlantLoopsForObject(
                                            state, this_equip.Name, Type, compLoc, errFlag1, _, _, innerNumSearchResults, _, LoopNum);
                                        if (innerNumSearchResults == 1) {
                                            this_equip.LoopNumPtr = compLoc.loopNum;
                                            this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                                            this_equip.BranchNumPtr = compLoc.branchNum;
                                            this_equip.CompNumPtr = compLoc.compNum;
                                            foundit = true;
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                        if (!foundit) {
                            ShowSevereError(state,
                                            format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                                   "component \"{}\" was not found on a cooling plant loop.",
                                                   this->Name,
                                                   this_equip.Name));
                        }
                    } else if (NumSearchResults == 0) {
                        ShowSevereError(state,
                                        format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                               "component \"{}\" was not found on a plant loop.",
                                               this->Name,
                                               this_equip.Name));
                    }
                    int inletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                        .LoopSide(this_equip.LoopSideNumPtr)
                                        .Branch(this_equip.BranchNumPtr)
                                        .Comp(this_equip.CompNumPtr)
                                        .NodeNumIn;
                    int outletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                         .LoopSide(this_equip.LoopSideNumPtr)
                                         .Branch(this_equip.BranchNumPtr)
                                         .Comp(this_equip.CompNumPtr)
                                         .NodeNumOut;
                    this_equip.DemandNodeNum = inletNode;
                    this_equip.SetPointNodeNum = outletNode;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme(1)
                        .OpSchemePtr = 1; // TODO check
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .CurOpSchemeType = this->Type;
                }
            }
        }
        if (this->PlantOps.NumHeatingOnlyEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumHeatingOnlyEquipLists; ++equipListNum) {

                int NumComps = this->HeatingOnlyEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->HeatingOnlyEquipList(equipListNum).Comp(compNum));
                    PlantLocation compLoc;
                    DataPlant::PlantEquipmentType Type;
                    Type = static_cast<DataPlant::PlantEquipmentType>(getEnumValue(PlantEquipTypeNamesUC, Util::makeUPPER(this_equip.TypeOf)));
                    bool errFlag1(false);
                    int NumSearchResults(0);
                    PlantUtilities::ScanPlantLoopsForObject(state, this_equip.Name, Type, compLoc, errFlag1, _, _, NumSearchResults);
                    if (NumSearchResults == 1) {

                        this_equip.LoopNumPtr = compLoc.loopNum;
                        this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                        this_equip.BranchNumPtr = compLoc.branchNum;
                        this_equip.CompNumPtr = compLoc.compNum;
                        this->PlantOps.PrimaryHWLoopIndex = compLoc.loopNum;

                    } else if (NumSearchResults > 1) {

                        bool foundit = false;
                        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
                            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                                int PltSizNum = state.dataPlnt->PlantLoop(LoopNum).PlantSizNum;
                                if (PltSizNum > 0) {
                                    if (state.dataSize->PlantSizData(PltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Heating) {
                                        int innerNumSearchResults = 0;
                                        PlantUtilities::ScanPlantLoopsForObject(
                                            state, this_equip.Name, Type, compLoc, errFlag1, _, _, innerNumSearchResults, _, LoopNum);
                                        if (innerNumSearchResults == 1) {
                                            this_equip.LoopNumPtr = compLoc.loopNum;
                                            this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                                            this_equip.BranchNumPtr = compLoc.branchNum;
                                            this_equip.CompNumPtr = compLoc.compNum;
                                            foundit = true;
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                        if (!foundit) {
                            ShowSevereError(state,
                                            format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                                   "component \"{}\" was not found on a heating plant loop.",
                                                   this->Name,
                                                   this_equip.Name));
                        }
                    } else if (NumSearchResults == 0) {
                        ShowSevereError(state,
                                        format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                               "component \"{}\" was not found on a plant loop.",
                                               this->Name,
                                               this_equip.Name));
                    }
                    int inletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                        .LoopSide(this_equip.LoopSideNumPtr)
                                        .Branch(this_equip.BranchNumPtr)
                                        .Comp(this_equip.CompNumPtr)
                                        .NodeNumIn;
                    int outletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                         .LoopSide(this_equip.LoopSideNumPtr)
                                         .Branch(this_equip.BranchNumPtr)
                                         .Comp(this_equip.CompNumPtr)
                                         .NodeNumOut;
                    this_equip.DemandNodeNum = inletNode;
                    this_equip.SetPointNodeNum = outletNode;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme(1)
                        .OpSchemePtr = 1; // TODO check
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .CurOpSchemeType = this->Type;
                }
            }
        }

        if (this->PlantOps.NumSimultHeatCoolCoolingEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolCoolingEquipLists; ++equipListNum) {

                int NumComps = this->SimultHeatCoolCoolingEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->SimultHeatCoolCoolingEquipList(equipListNum).Comp(compNum));
                    PlantLocation compLoc;
                    DataPlant::PlantEquipmentType Type;
                    Type = static_cast<DataPlant::PlantEquipmentType>(getEnumValue(PlantEquipTypeNamesUC, Util::makeUPPER(this_equip.TypeOf)));
                    bool errFlag1(false);
                    int NumSearchResults(0);
                    PlantUtilities::ScanPlantLoopsForObject(state, this_equip.Name, Type, compLoc, errFlag1, _, _, NumSearchResults);
                    if (NumSearchResults == 1) {

                        this_equip.LoopNumPtr = compLoc.loopNum;
                        this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                        this_equip.BranchNumPtr = compLoc.branchNum;
                        this_equip.CompNumPtr = compLoc.compNum;

                    } else if (NumSearchResults > 1) {

                        bool foundit = false;
                        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
                            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                                int PltSizNum = state.dataPlnt->PlantLoop(LoopNum).PlantSizNum;
                                if (PltSizNum > 0) {
                                    if (state.dataSize->PlantSizData(PltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Cooling) {
                                        int innerNumSearchResults = 0;
                                        PlantUtilities::ScanPlantLoopsForObject(
                                            state, this_equip.Name, Type, compLoc, errFlag1, _, _, innerNumSearchResults, _, LoopNum);
                                        if (innerNumSearchResults == 1) {
                                            this_equip.LoopNumPtr = compLoc.loopNum;
                                            this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                                            this_equip.BranchNumPtr = compLoc.branchNum;
                                            this_equip.CompNumPtr = compLoc.compNum;
                                            foundit = true;
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                        if (!foundit) {
                            ShowSevereError(state,
                                            format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                                   "component \"{}\" was not found on a cooling plant loop.",
                                                   this->Name,
                                                   this_equip.Name));
                        }
                    } else if (NumSearchResults == 0) {
                        ShowSevereError(state,
                                        format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                               "component \"{}\" was not found on a plant loop.",
                                               this->Name,
                                               this_equip.Name));
                    }
                    int inletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                        .LoopSide(this_equip.LoopSideNumPtr)
                                        .Branch(this_equip.BranchNumPtr)
                                        .Comp(this_equip.CompNumPtr)
                                        .NodeNumIn;
                    int outletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                         .LoopSide(this_equip.LoopSideNumPtr)
                                         .Branch(this_equip.BranchNumPtr)
                                         .Comp(this_equip.CompNumPtr)
                                         .NodeNumOut;
                    this_equip.DemandNodeNum = inletNode;
                    this_equip.SetPointNodeNum = outletNode;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme(1)
                        .OpSchemePtr = 1; // TODO check
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .CurOpSchemeType = this->Type;
                }
            }
        }

        if (this->PlantOps.NumSimultHeatCoolHeatingEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolHeatingEquipLists; ++equipListNum) {

                int NumComps = this->SimultHeatCoolHeatingEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->SimultHeatCoolHeatingEquipList(equipListNum).Comp(compNum));
                    PlantLocation compLoc;
                    DataPlant::PlantEquipmentType Type;
                    Type = static_cast<DataPlant::PlantEquipmentType>(getEnumValue(PlantEquipTypeNamesUC, Util::makeUPPER(this_equip.TypeOf)));
                    bool errFlag1(false);
                    int NumSearchResults(0);
                    PlantUtilities::ScanPlantLoopsForObject(state, this_equip.Name, Type, compLoc, errFlag1, _, _, NumSearchResults);
                    if (NumSearchResults == 1) {

                        this_equip.LoopNumPtr = compLoc.loopNum;
                        this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                        this_equip.BranchNumPtr = compLoc.branchNum;
                        this_equip.CompNumPtr = compLoc.compNum;

                    } else if (NumSearchResults > 1) {

                        bool foundit = false;
                        for (int LoopNum = 1; LoopNum <= state.dataPlnt->TotNumLoops; ++LoopNum) {
                            if (this->PlantLoopIndicesBeingSupervised(LoopNum) > 0) {
                                int PltSizNum = state.dataPlnt->PlantLoop(LoopNum).PlantSizNum;
                                if (PltSizNum > 0) {
                                    if (state.dataSize->PlantSizData(PltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Heating) {
                                        int innerNumSearchResults = 0;
                                        PlantUtilities::ScanPlantLoopsForObject(
                                            state, this_equip.Name, Type, compLoc, errFlag1, _, _, innerNumSearchResults, _, LoopNum);
                                        if (innerNumSearchResults == 1) {
                                            this_equip.LoopNumPtr = compLoc.loopNum;
                                            this_equip.LoopSideNumPtr = compLoc.loopSideNum;
                                            this_equip.BranchNumPtr = compLoc.branchNum;
                                            this_equip.CompNumPtr = compLoc.compNum;
                                            foundit = true;
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                        if (!foundit) {
                            ShowSevereError(state,
                                            format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                                   "component \"{}\" was not found on a heating plant loop.",
                                                   this->Name,
                                                   this_equip.Name));
                        }
                    } else if (NumSearchResults == 0) {
                        ShowSevereError(state,
                                        format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" "
                                               "component \"{}\" was not found on a plant loop.",
                                               this->Name,
                                               this_equip.Name));
                    }
                    int inletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                        .LoopSide(this_equip.LoopSideNumPtr)
                                        .Branch(this_equip.BranchNumPtr)
                                        .Comp(this_equip.CompNumPtr)
                                        .NodeNumIn;
                    int outletNode = state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                         .LoopSide(this_equip.LoopSideNumPtr)
                                         .Branch(this_equip.BranchNumPtr)
                                         .Comp(this_equip.CompNumPtr)
                                         .NodeNumOut;
                    this_equip.DemandNodeNum = inletNode;
                    this_equip.SetPointNodeNum = outletNode;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .OpScheme(1)
                        .OpSchemePtr = 1; // TODO check
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .CurOpSchemeType = this->Type;
                }
            }
        }

        // process primary loops for supply inlet node numbers

        // examine supply inlet branch on primary chilled water loop to see if there is a tank, use outlet of tank if so

        if (this->PlantOps.PrimaryChWLoopIndex > 0) {

            this->PlantOps.PrimaryChWLoopSupInletNode =
                state.dataPlnt->PlantLoop(this->PlantOps.PrimaryChWLoopIndex).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).NodeNumIn;
            for (int numComps = 1; numComps <= state.dataPlnt->PlantLoop(this->PlantOps.PrimaryChWLoopIndex)
                                                   .LoopSide(DataPlant::LoopSideLocation::Supply)
                                                   .Branch(1)
                                                   .TotalComponents;
                 ++numComps) {
                auto const &this_Comp(state.dataPlnt->PlantLoop(this->PlantOps.PrimaryChWLoopIndex)
                                          .LoopSide(DataPlant::LoopSideLocation::Supply)
                                          .Branch(1)
                                          .Comp(numComps));
                if (this_Comp.Type == DataPlant::PlantEquipmentType::ChilledWaterTankMixed ||
                    this_Comp.Type == DataPlant::PlantEquipmentType::ChilledWaterTankStratified) {
                    // assume tank is on chilled water return for use as a buffer tank, use the outlet of the tank instead of the inlet when
                    // calculating the current load on the primary chilled water plant
                    this->PlantOps.PrimaryChWLoopSupInletNode = this_Comp.NodeNumOut;
                }
            }
        }

        if (this->PlantOps.PrimaryHWLoopIndex > 0) {
            this->PlantOps.PrimaryHWLoopSupInletNode =
                state.dataPlnt->PlantLoop(this->PlantOps.PrimaryHWLoopIndex).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).NodeNumIn;
        }
        // process dedicated heat recovery water to water heatpumps to control
        if (this->PlantOps.DedicatedHR_ChWRetControl_Input && this->PlantOps.DedicatedHR_HWRetControl_Input) {
            bool founditCooling = false;
            bool founditHeating = false;
            for (auto &thisHP : state.dataEIRPlantLoopHeatPump->heatPumps) {
                std::string const thisPLHPName = Util::makeUPPER(thisHP.name);
                // find cooling side heat pump
                std::string const targetDedHRCoolName = Util::makeUPPER(this->DedicatedHR_ChWRetControl_Name);
                if (thisPLHPName == targetDedHRCoolName) {  // found it
                    this->DedicatedHR_CoolingPLHP = thisHP; // store pointer to cooling side of heat pump
                    founditCooling = true;

                    int pltSizNum = state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum).PlantSizNum;
                    this->PlantOps.DedicatedHR_SecChW_DesignCapacity = state.dataSize->PlantSizData(pltSizNum).DesCapacity;
                    this->PlantOps.SecondaryChWLoopIndex = this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum;

                    // set up load side plant loop information for cooling side of heat pump
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                        .OpScheme(1)
                        .OpSchemePtr = 1;
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                        .CurOpSchemeType = this->Type;

                    // setup source side plant loop data structure information for cooling side of heat pump
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
                        .OpScheme(1)
                        .OpSchemePtr = 1;
                    state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
                        .CurOpSchemeType = this->Type;
                }

                // find heating side heat pump
                std::string const targetDedHRHeatName = Util::makeUPPER(this->DedicatedHR_HWRetControl_Name);
                if (thisPLHPName == targetDedHRHeatName) {  // found it
                    this->DedicatedHR_HeatingPLHP = thisHP; // store pointer to heating side of heat pump
                    founditHeating = true;

                    int pltSizNum = state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum).PlantSizNum;
                    this->PlantOps.DedicatedHR_SecHW_DesignCapacity = state.dataSize->PlantSizData(pltSizNum).DesCapacity;
                    this->PlantOps.SecondaryHWLoopIndex = this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum;

                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                        .OpScheme(1)
                        .OpSchemePtr = 1;
                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                        .CurOpSchemeType = this->Type;

                    // setup source side plant loop data structure information for heating side of heat pump
                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
                        .OpScheme.allocate(1);
                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
                        .OpScheme(1)
                        .OpSchemePtr = 1;
                    state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
                        .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
                        .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
                        .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
                        .CurOpSchemeType = this->Type;
                }
            }

            if (!founditCooling) {
                ShowSevereError(state,
                                format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" component "
                                       "\"{}\" was not found on a cooling plant loop.",
                                       this->Name,
                                       this->DedicatedHR_ChWRetControl_Name));
            }
            if (!founditHeating) {
                ShowSevereError(state,
                                format("ChillerHeaterSupervisoryOperationData::OneTimeInitChillerHeaterChangeoverOpScheme problem=\"{}\" component "
                                       "\"{}\" was not found on a heating plant loop.",
                                       this->Name,
                                       this->DedicatedHR_ChWRetControl_Name));
            }
            if (founditCooling && founditHeating) {
                this->PlantOps.DedicatedHR_Present = true;
                SetupOutputVariable(state,
                                    "Supervisory Plant Heat Recovery Operation Mode",
                                    Constant::Units::unknown,
                                    this->Report.DedicHR_OpMode,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Supervisory Plant Operation Secondary Plant Heating Load",
                                    Constant::Units::W,
                                    this->Report.SecondaryPlantHeatingLoad,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Supervisory Plant Operation Secondary Plant Cooling Load",
                                    Constant::Units::W,
                                    this->Report.SecondaryPlantCoolingLoad,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
            }
        }

        this->oneTimeSetupComplete = true;
    }

    void ChillerHeaterSupervisoryOperationData::EvaluateChillerHeaterChangeoverOpScheme(EnergyPlusData &state)
    {

        DetermineCurrentBuildingLoads(state);
        DetermineCurrentPlantLoads(state);
        ProcessSupervisoryControlLogicForAirSourcePlants(state);
        InitAirSourcePlantEquipmentOff(state);
        ProcessAndSetAirSourcePlantEquipLists(state);
        ProcessAndSetDedicatedHeatRecovWWHP(state);
        ProcessAndSetAuxilBoiler(state);
    }

    void ChillerHeaterSupervisoryOperationData::DetermineCurrentBuildingLoads(EnergyPlusData &state)
    {
        // Poll the loads on the zones to help decide how to run

        Real64 sumZonePredictedHeatingLoad(0.0);
        Real64 sumZonePredictedCoolingLoad(0.0);
        for (int zoneIndexinList = 1; zoneIndexinList <= this->PlantOps.NumOfZones; ++zoneIndexinList) {
            int thisZoneIndex = this->ZonePtrs(zoneIndexinList);
            Real64 ZoneMult = state.dataHeatBal->Zone(thisZoneIndex).Multiplier * state.dataHeatBal->Zone(thisZoneIndex).ListMultiplier;
            // aggregate required outputs to setpoint, with zone multipliers included
            sumZonePredictedCoolingLoad +=
                min(0.0,
                    state.dataZoneEnergyDemand->ZoneSysEnergyDemand(thisZoneIndex).OutputRequiredToCoolingSP * ZoneMult); // sum only negative values
            sumZonePredictedHeatingLoad +=
                max(0.0,
                    state.dataZoneEnergyDemand->ZoneSysEnergyDemand(thisZoneIndex).OutputRequiredToHeatingSP * ZoneMult); // sum only positive values
        }

        // now add in ventilation loading at the central air system level
        Real64 sumAirSysVentHeatingLoad(0.0);
        Real64 sumAirSysVentCoolingLoad(0.0);

        for (int airLoopsServedIndex = 1; airLoopsServedIndex <= this->PlantOps.NumOfAirLoops; ++airLoopsServedIndex) {
            int AirLoopNum = this->AirLoopPtrs(airLoopsServedIndex);
            Real64 outAir_H = state.dataEnvrn->OutEnthalpy;
            Real64 outAirMdot = state.dataAirLoop->AirLoopFlow(AirLoopNum).OAFlow;
            Real64 retAir_Tdb = state.dataLoopNodes->Node(state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopReturnNodeNum(1)).Temp;
            Real64 retAir_w = state.dataLoopNodes->Node(state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopReturnNodeNum(1)).HumRat;
            Real64 ventLoad = outAirMdot * (Psychrometrics::PsyHFnTdbW(retAir_Tdb, retAir_w) - outAir_H); // negative is cooling
            if (ventLoad > HVAC::SmallLoad) {                                                             // add to heating
                sumAirSysVentHeatingLoad += ventLoad;
            } else if (ventLoad < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) { // add to cooling
                sumAirSysVentCoolingLoad += ventLoad;
            }
        }

        // now add in any process loads from plant load profiles on the controlled loops.
        Real64 sumLoadProfileHeatingLoad(0.0);
        Real64 sumLoadProfileCoolingLoad(0.0);
        for (int NumProcLoad = 1; NumProcLoad <= this->PlantOps.numPlantLoadProfiles; ++NumProcLoad) {
            Real64 load = 0.0;
            DataPlant::CompData::getPlantComponent(state, PlantLoadProfileComps(NumProcLoad)).compPtr->getCurrentPower(state, load);
            if (load > 0.0) {
                sumLoadProfileHeatingLoad += load;
            } else {
                sumLoadProfileCoolingLoad += load;
            }
        }

        this->Report.BuildingPolledCoolingLoad = sumZonePredictedCoolingLoad + sumAirSysVentCoolingLoad + sumLoadProfileCoolingLoad;
        this->Report.BuildingPolledHeatingLoad = sumZonePredictedHeatingLoad + sumAirSysVentHeatingLoad + sumLoadProfileHeatingLoad;
        // end  collect loads.
    }

    void ChillerHeaterSupervisoryOperationData::DetermineCurrentPlantLoads(EnergyPlusData &state)
    {

        // Calculate load on primary chilled water loop and store in PrimaryPlantCoolingLoad

        Real64 CW_RetMdot = state.dataLoopNodes->Node(this->PlantOps.PrimaryChWLoopSupInletNode).MassFlowRate;
        Real64 const CpCW = state.dataPlnt->PlantLoop(this->PlantOps.PrimaryChWLoopIndex)
                                .glycol->getSpecificHeat(
                                    state, state.dataLoopNodes->Node(this->PlantOps.PrimaryChWLoopSupInletNode).Temp, "DetermineCurrentPlantLoads");
        Real64 CW_Qdot =
            min(0.0,
                CW_RetMdot * CpCW *
                    (this->Setpoint.PrimCW -
                     state.dataLoopNodes->Node(this->PlantOps.PrimaryChWLoopSupInletNode).Temp)); // power = Mdot Cp Delta T, cooling load is negative
        this->Report.PrimaryPlantCoolingLoad = CW_Qdot;

        // Calculate load on primary hot water loop and store in PrimaryPlantHeatingLoad
        // int HWSupInletNode = this->PlantOps.PrimaryHWLoopSupInletNode;
        //      state.dataPlnt->PlantLoop(this->PlantOps.PrimaryHWLoopIndex).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).NodeNumIn;
        Real64 HW_RetMdot = state.dataLoopNodes->Node(this->PlantOps.PrimaryHWLoopSupInletNode).MassFlowRate;
        Real64 const CpHW = state.dataPlnt->PlantLoop(this->PlantOps.PrimaryHWLoopIndex)
                                .glycol->getSpecificHeat(
                                    state, state.dataLoopNodes->Node(this->PlantOps.PrimaryHWLoopSupInletNode).Temp, "DetermineCurrentPlantLoads");

        Real64 HW_Qdot =
            max(0.0,
                HW_RetMdot * CpHW *
                    (this->DetermineHWSetpointOARest(state) -
                     state.dataLoopNodes->Node(this->PlantOps.PrimaryHWLoopSupInletNode).Temp)); // power = Mdot Cp Delta T, heating load is positive
        this->Report.PrimaryPlantHeatingLoad = HW_Qdot;
    }

    void ChillerHeaterSupervisoryOperationData::ProcessSupervisoryControlLogicForAirSourcePlants(EnergyPlusData &state)
    {
        // this routine decides which of three modes the plants should operate in,  Heating Only, Cooling Only, Simultaneous Heating and Cooling.

        // step 1, initialize control bools
        this->PlantOps.AirSourcePlantCoolingOnly = false;
        this->PlantOps.AirSourcePlantHeatingOnly = false;
        this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = false;
        this->PlantOps.SimultaneousHeatingCoolingWithCoolingDominant = false;
        this->PlantOps.SimultaneousHeatingCoolingWithHeatingDominant = false;

        // step 2, process logic based on poll results for building loads.
        if (this->Report.BuildingPolledHeatingLoad < HVAC::SmallLoad &&
            this->Report.BuildingPolledCoolingLoad < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) {
            this->PlantOps.AirSourcePlantCoolingOnly = true;
        } else if (this->Report.BuildingPolledCoolingLoad > DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad &&
                   this->Report.BuildingPolledHeatingLoad > HVAC::SmallLoad) {
            this->PlantOps.AirSourcePlantHeatingOnly = true;

            if (state.dataEnvrn->OutDryBulbTemp < this->TempReset.LowOutdoorTemp) { // too cold for airsource HPs so
                this->PlantOps.AirSourcePlantHeatingOnly = false;
            }

        } else if ((this->Report.BuildingPolledCoolingLoad < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) &&
                   (this->Report.BuildingPolledHeatingLoad > HVAC::SmallLoad)) {
            this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = true;
            if (this->Report.BuildingPolledHeatingLoad > abs(this->Report.BuildingPolledCoolingLoad)) {
                this->PlantOps.SimultaneousHeatingCoolingWithHeatingDominant = true;
                if (this->PlantOps.SimultHeatCoolOpAvailable) {
                    this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = true;
                } else {
                    this->PlantOps.AirSourcePlantHeatingOnly = true;
                    this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = false;
                }
            } else if (abs(this->Report.BuildingPolledCoolingLoad) > this->Report.BuildingPolledHeatingLoad) {
                this->PlantOps.SimultaneousHeatingCoolingWithCoolingDominant = true;
                if (this->PlantOps.SimultHeatCoolOpAvailable) {
                    this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = true;
                } else {
                    this->PlantOps.AirSourcePlantCoolingOnly = true;
                    this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = false;
                }
            }
            if (state.dataEnvrn->OutDryBulbTemp < this->TempReset.LowOutdoorTemp) { // too cold for airsource HPs
                this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = false;
                this->PlantOps.AirSourcePlantCoolingOnly = true;
            }
        }

        // step 3, revise control decision based on current loads on primary plant loops
        if (this->PlantOps.AirSourcePlantHeatingOnly &&
            this->Report.PrimaryPlantCoolingLoad < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) {
            // polled building loads indicate all heating, but cooling plant has cooling load, try to switch to simultaneous cooling and heating with
            // heating dominant
            if (this->PlantOps.SimultHeatCoolOpAvailable) {
                this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = true;
                this->PlantOps.SimultaneousHeatingCoolingWithHeatingDominant = true;
                this->PlantOps.AirSourcePlantHeatingOnly = false;
            }
        }

        if (this->PlantOps.AirSourcePlantCoolingOnly && this->Report.PrimaryPlantHeatingLoad > HVAC::SmallLoad &&
            state.dataEnvrn->OutDryBulbTemp >= this->TempReset.LowOutdoorTemp) {
            // polled building loads indicate all cooling, but heating plant has heating load, and outdoor air is warm enough for heat pump, try to
            // switch to simultaneous cooling and heating with cooling dominant
            if (this->PlantOps.SimultHeatCoolOpAvailable) {
                this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling = true;
                this->PlantOps.SimultaneousHeatingCoolingWithCoolingDominant = true;
                this->PlantOps.AirSourcePlantCoolingOnly = false;
            }
        }

        // do we need to turn on cooling-only if in off mode but PrimaryPlantCoolingLoad is loaded?
        if (!this->PlantOps.AirSourcePlantCoolingOnly && !this->PlantOps.AirSourcePlantHeatingOnly &&
            !this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) { // all off
            if (this->Report.PrimaryPlantCoolingLoad < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) {
                this->PlantOps.AirSourcePlantCoolingOnly = true;
            }
        }

        // override reset AirSourcePlantHeatingOnly to false and AirSourcePlantCoolingOnly to true if the plant cooling load is higher
        // the plant heating load and the plant heating load is small.
        if (!this->PlantOps.AirSourcePlantCoolingOnly && this->PlantOps.AirSourcePlantHeatingOnly &&
            !this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) { // all off
            if (std::abs(this->Report.PrimaryPlantCoolingLoad) > this->Report.PrimaryPlantHeatingLoad &&
                this->Report.PrimaryPlantHeatingLoad < HVAC::SmallLoad) {
                this->PlantOps.AirSourcePlantCoolingOnly = true;
                this->PlantOps.AirSourcePlantHeatingOnly = false;
            }
        }

        // override reset AirSourcePlantHeatingOnly to true and AirSourcePlantCoolingOnly to false if the plant heating load is higher
        // the plant cooling load and the plant cooling load is small.
        if (this->PlantOps.AirSourcePlantCoolingOnly && !this->PlantOps.AirSourcePlantHeatingOnly &&
            !this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) { // all off
            if (this->Report.PrimaryPlantHeatingLoad > std::abs(this->Report.PrimaryPlantCoolingLoad) &&
                this->Report.PrimaryPlantCoolingLoad > DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) {
                this->PlantOps.AirSourcePlantHeatingOnly = true;
                this->PlantOps.AirSourcePlantCoolingOnly = false;
            }
        }

        // do we need to turn on heating-only if in off mode but PrimaryPlantHeatingLoad is loaded?
        if (!this->PlantOps.AirSourcePlantCoolingOnly && !this->PlantOps.AirSourcePlantHeatingOnly &&
            !this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) { // all off
            if (this->Report.PrimaryPlantHeatingLoad > HVAC::SmallLoad && state.dataEnvrn->OutDryBulbTemp >= this->TempReset.LowOutdoorTemp) {
                this->PlantOps.AirSourcePlantHeatingOnly = true;
            }
        }

        // step 4, convert logical flags into integers for output variable reporting
        if (this->PlantOps.AirSourcePlantHeatingOnly) {
            this->Report.AirSourcePlant_OpMode = 1;
        } else if (this->PlantOps.AirSourcePlantCoolingOnly) {
            this->Report.AirSourcePlant_OpMode = 2;
        } else if (this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) {
            this->Report.AirSourcePlant_OpMode = 3;
        } else {
            this->Report.AirSourcePlant_OpMode = 0;
        }
    }

    void ChillerHeaterSupervisoryOperationData::InitAirSourcePlantEquipmentOff(EnergyPlusData &state)
    {
        //_____________________________________________________________________________
        // initialize all possible equipment to turn off machines before applying controls to turn them on.
        // set .Available and .ON to false in plant structure

        if (this->PlantOps.NumCoolingOnlyEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumCoolingOnlyEquipLists; ++equipListNum) {
                int NumComps = this->CoolingOnlyEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->CoolingOnlyEquipList(equipListNum).Comp(compNum));
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .Available = false;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .ON = false;
                }
            }
        }

        if (this->PlantOps.NumHeatingOnlyEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumHeatingOnlyEquipLists; ++equipListNum) {
                int NumComps = this->HeatingOnlyEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->HeatingOnlyEquipList(equipListNum).Comp(compNum));
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .Available = false;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .ON = false;
                }
            }
        }

        if (this->PlantOps.NumSimultHeatCoolCoolingEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolCoolingEquipLists; ++equipListNum) {

                int NumComps = this->SimultHeatCoolCoolingEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->SimultHeatCoolCoolingEquipList(equipListNum).Comp(compNum));
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .Available = false;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .ON = false;
                }
            }
        }

        if (this->PlantOps.NumSimultHeatCoolHeatingEquipLists > 0) {
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolHeatingEquipLists; ++equipListNum) {

                int NumComps = this->SimultHeatCoolHeatingEquipList(equipListNum).NumComps;
                for (int compNum = 1; compNum <= NumComps; ++compNum) {
                    auto &this_equip(this->SimultHeatCoolHeatingEquipList(equipListNum).Comp(compNum));
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .Available = false;
                    state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                        .LoopSide(this_equip.LoopSideNumPtr)
                        .Branch(this_equip.BranchNumPtr)
                        .Comp(this_equip.CompNumPtr)
                        .ON = false;
                }
            }
        }
        // end init machines off
    }

    void ChillerHeaterSupervisoryOperationData::ProcessAndSetAirSourcePlantEquipLists(EnergyPlusData &state)
    {
        // TODO this routine is currently code to compare real current plant loads, polled building loads have also been studied.
        Real64 CoolingLoadSignal = this->Report.PrimaryPlantCoolingLoad;
        Real64 HeatingLoadSignal = this->Report.PrimaryPlantHeatingLoad;

        //___________________________________________________________________________
        if (this->PlantOps.AirSourcePlantCoolingOnly) {
            // use zone loads to find range based cooling loads
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumCoolingOnlyEquipLists; ++equipListNum) {
                // zone cooling loads are negative, switch to positive for range based limiting
                if (CoolingLoadSignal * DataPrecisionGlobals::constant_minusone > this->CoolingOnlyEquipList(equipListNum).RangeLowerLimit &&
                    this->CoolingOnlyEquipList(equipListNum).RangeUpperLimit > CoolingLoadSignal * DataPrecisionGlobals::constant_minusone) {
                    // found that this equipment list load ranges match the zone predicted cooling loads

                    int NumComps = this->CoolingOnlyEquipList(equipListNum).NumComps;
                    for (int compNum = 1; compNum <= NumComps; ++compNum) {
                        auto &this_equip(this->CoolingOnlyEquipList(equipListNum).Comp(compNum));
                        // set cooling setpoint at outlet

                        // todo, oa reset ?

                        state.dataLoopNodes->Node(this_equip.SetPointNodeNum).TempSetPoint = this->Setpoint.PrimCW;
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this_equip.LoopNumPtr).TempSetPointNodeNum).TempSetPoint =
                            this->Setpoint.PrimCW;
                        if (state.dataLoopNodes->Node(this_equip.DemandNodeNum).Temp > this->Setpoint.PrimCW) {

                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .Available = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .ON = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .CurOpSchemeType = this->Type;
                        }
                        //
                    }
                }
            }
        }

        //____________________________________________________________________________
        if (this->PlantOps.AirSourcePlantHeatingOnly) { // Use Heating Only equipment operation

            Real64 HWsetpt = DetermineHWSetpointOARest(state);
            // use zone loads to find range based heating loads
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumHeatingOnlyEquipLists; ++equipListNum) {
                if (HeatingLoadSignal > this->HeatingOnlyEquipList(equipListNum).RangeLowerLimit &&
                    this->HeatingOnlyEquipList(equipListNum).RangeUpperLimit > HeatingLoadSignal) {
                    // found that this equipment list load ranges match the zone predicted heating loads

                    int NumComps = this->HeatingOnlyEquipList(equipListNum).NumComps;
                    for (int compNum = 1; compNum <= NumComps; ++compNum) {
                        auto &this_equip(this->HeatingOnlyEquipList(equipListNum).Comp(compNum));
                        // set heating setpoint at outlet

                        state.dataLoopNodes->Node(this_equip.SetPointNodeNum).TempSetPoint = HWsetpt;
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this_equip.LoopNumPtr).TempSetPointNodeNum).TempSetPoint = HWsetpt;

                        if (state.dataLoopNodes->Node(this_equip.DemandNodeNum).Temp < HWsetpt) {
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .Available = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .ON = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .CurOpSchemeType = this->Type;
                        }
                        //
                    }
                }
            }
        }

        if (this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling) {

            // use zone cooling loads to find range based equipment
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolCoolingEquipLists; ++equipListNum) {
                // zone cooling loads are negative, switch to positive for range based limiting
                if (CoolingLoadSignal * DataPrecisionGlobals::constant_minusone >
                        this->SimultHeatCoolCoolingEquipList(equipListNum).RangeLowerLimit &&
                    this->SimultHeatCoolCoolingEquipList(equipListNum).RangeUpperLimit >
                        CoolingLoadSignal * DataPrecisionGlobals::constant_minusone) {
                    // found that this equipment list load ranges match the zone predicted cooling loads

                    int NumComps = this->SimultHeatCoolCoolingEquipList(equipListNum).NumComps;
                    for (int compNum = 1; compNum <= NumComps; ++compNum) {
                        auto &this_equip(this->SimultHeatCoolCoolingEquipList(equipListNum).Comp(compNum));
                        // set cooling setpoint at outlet

                        state.dataLoopNodes->Node(this_equip.SetPointNodeNum).TempSetPoint = this->Setpoint.PrimCW;
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this_equip.LoopNumPtr).TempSetPointNodeNum).TempSetPoint =
                            this->Setpoint.PrimCW;
                        if (state.dataLoopNodes->Node(this_equip.DemandNodeNum).Temp > this->Setpoint.PrimCW) {
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .Available = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .ON = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .CurOpSchemeType = this->Type;
                        }
                        //
                    }
                }
            }

            // use zone loads to find range based heating loads
            Real64 HWsetpt = DetermineHWSetpointOARest(state);
            for (int equipListNum = 1; equipListNum <= this->PlantOps.NumSimultHeatCoolHeatingEquipLists; ++equipListNum) {
                if (HeatingLoadSignal > this->SimultHeatCoolHeatingEquipList(equipListNum).RangeLowerLimit &&
                    this->SimultHeatCoolHeatingEquipList(equipListNum).RangeUpperLimit > HeatingLoadSignal) {
                    // found that this equipment list load ranges match the zone predicted heating loads

                    int NumComps = this->SimultHeatCoolHeatingEquipList(equipListNum).NumComps;
                    for (int compNum = 1; compNum <= NumComps; ++compNum) {
                        auto &this_equip(this->SimultHeatCoolHeatingEquipList(equipListNum).Comp(compNum));
                        // set heating setpoint at outlet

                        state.dataLoopNodes->Node(this_equip.SetPointNodeNum).TempSetPoint = HWsetpt;
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this_equip.LoopNumPtr).TempSetPointNodeNum).TempSetPoint = HWsetpt;

                        if (state.dataLoopNodes->Node(this_equip.DemandNodeNum).Temp < HWsetpt) {
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .Available = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .ON = true;
                            state.dataPlnt->PlantLoop(this_equip.LoopNumPtr)
                                .LoopSide(this_equip.LoopSideNumPtr)
                                .Branch(this_equip.BranchNumPtr)
                                .Comp(this_equip.CompNumPtr)
                                .CurOpSchemeType = this->Type;
                        }
                        //
                    }
                }
            }
        }
    }

    void ChillerHeaterSupervisoryOperationData::ProcessAndSetDedicatedHeatRecovWWHP(EnergyPlusData &state)
    {
        // evaluate if and how dedicated heat recovery WWHP should run

        if (!this->PlantOps.DedicatedHR_Present) {
            return;
        }

        // initialize off
        state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
            .Available = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
            .ON = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
            .Available = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
            .ON = false;

        state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
            .Available = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
            .ON = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
            .Available = false;
        state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
            .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
            .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
            .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
            .ON = false;

        // Dedicated Heat Recovery Water To Water Heat Pump Control.
        // Assume there are two companion machines, one leads for cooling the return chilled water, the other leads for heating the return hot
        // water When one side leads, the other gets favorable heat addition/extraction it is just not controlled to meet a setpoint Assume these
        // are on the secondary loops.  Need to decide if it runs and which of cooling or heating companion coils gets to lead.
        //
        // Step 1. get the mass flow rates of the returns.  both must be non-zero for the WWHP to run
        int inletChWReturnNodeNum = state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
                                        .LoopSide(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopSideNum)
                                        .Branch(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.branchNum)
                                        .Comp(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.compNum)
                                        .NodeNumIn;
        int inletHWReturnNodeNum = state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
                                       .LoopSide(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopSideNum)
                                       .Branch(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.branchNum)
                                       .Comp(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.compNum)
                                       .NodeNumIn;
        Real64 CW_RetMdot = state.dataLoopNodes->Node(inletChWReturnNodeNum).MassFlowRate;
        Real64 HW_RetMdot = state.dataLoopNodes->Node(inletHWReturnNodeNum).MassFlowRate;

        bool flowInEach = false;
        // need flow in both returns.
        if (CW_RetMdot <= HVAC::SmallWaterVolFlow || HW_RetMdot <= HVAC::SmallWaterVolFlow) {
            flowInEach = false;
        } else {
            flowInEach = true;
        }

        // step 2. calculate the loads to adjust the
        // returns to hit the associated setpoints at their current mass flow
        Real64 const CpCW =
            state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.sourceSidePlantLoc.loopNum)
                .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(inletChWReturnNodeNum).Temp, "EvaluateChillerHeaterChangeoverOpScheme");
        Real64 CW_Qdot =
            CW_RetMdot * CpCW *
            (this->Setpoint.SecCW - state.dataLoopNodes->Node(inletChWReturnNodeNum).Temp); // power = Mdot Cp Delta T, cooling load is negative
        Real64 const CpHW =
            state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.sourceSidePlantLoc.loopNum)
                .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(inletHWReturnNodeNum).Temp, "EvaluateChillerHeaterChangeoverOpScheme");
        Real64 HW_Qdot = HW_RetMdot * CpHW * (this->Setpoint.SecHW - state.dataLoopNodes->Node(inletHWReturnNodeNum).Temp); // power = Mdot Cp Delta T

        // store for reporting
        this->Report.SecondaryPlantCoolingLoad = CW_Qdot;
        this->Report.SecondaryPlantHeatingLoad = HW_Qdot;

        // step 3 decide if Dedicated HR is on and which leads based
        bool CoolLedNeed = false;
        bool HeatLedNeed = false;

        if (this->PlantOps.AirSourcePlantHeatingOnly && (CW_Qdot < DataPrecisionGlobals::constant_minusone * HVAC::SmallLoad) && flowInEach) {
            // polled building loads are heating only, but secondary ChW plant has some cooling load and there is mass flow in each. So turn dedicated
            // HR on in cooling lead mode
            CoolLedNeed = true;
        }

        if (this->PlantOps.AirSourcePlantCoolingOnly && (HW_Qdot > HVAC::SmallLoad) && flowInEach) {
            // polled building loads are cooling only, but secondary HW plant has some heating load and there is mass flow in each. So turn dedicated
            // HR on in heating lead mode
            HeatLedNeed = true;
        }

        if (this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling && this->PlantOps.SimultaneousHeatingCoolingWithHeatingDominant &&
            flowInEach) {
            // polled building loads are simultaneous with heating dominating and there is mass flow in each, So turn
            // dedicated HR on in heating lead mode
            HeatLedNeed = true;
        }

        if (this->PlantOps.AirSourcePlantSimultaneousHeatingAndCooling && this->PlantOps.SimultaneousHeatingCoolingWithCoolingDominant &&
            flowInEach) {
            // polled building loads are simultaneous with cooling dominating and there is mass flow in each, So turn
            // dedicated HR on in cooling lead mode
            CoolLedNeed = true;
        }

        //  step 4. check that there is sufficient flow in source side for chosen leader to avoid runaway plant conditions on source side
        //  if not, see if other side could run beneficially as leader and switch to it if so
        // Real64 FlowImbalanceRatioThreshold = 10.0; // TODO, check with TRANE engineering about WWHP operating limits wrt to relative flows (real
        //                                          // systems have a pumped sided arm flow situation and do not have low flow problems)

        // if (CoolLedNeed) {
        //    if (CW_RetMdot / HW_RetMdot > FlowImbalanceRatioThreshold) { // insufficient flow in source side relative to load side
        //        CoolLedNeed = false;
        //        // if (HW_Qdot > 1.0) {
        //        //    HeatLedNeed = true;
        //        //}
        //    }
        //}
        // if (HeatLedNeed) {
        //    if (HW_RetMdot / CW_RetMdot > FlowImbalanceRatioThreshold) { // insufficient flow in source side relative to load side
        //        HeatLedNeed = false;
        //        // if (CW_Qdot < -1.0) {
        //        //    CoolLedNeed = true;
        //        //}
        //    }
        //}

        this->Report.DedicHR_OpMode = 0;
        if (CoolLedNeed) {
            this->Report.DedicHR_OpMode = 2;
            // turn ON load side of this water to water heat pump
            state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                .Available = true;
            state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                .ON = true;

            state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                .CurOpSchemeType = this->Type;

            state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                .MyLoad = CW_Qdot; // cooling load is negative

            int OutletChWReturnNodeNum = state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)
                                             .LoopSide(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopSideNum)
                                             .Branch(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.branchNum)
                                             .Comp(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.compNum)
                                             .NodeNumOut;
            state.dataLoopNodes->Node(OutletChWReturnNodeNum).TempSetPoint = this->Setpoint.SecCW;
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum).TempSetPointNodeNum)
                .TempSetPoint = this->Setpoint.SecCW;

            if (this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum ==
                SecondaryPlantLoopIndicesBeingSupervised(this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum)) {
                // search for HX on this loop and place setpoint on outlet
                for (int HXnum = 1; HXnum <= this->PlantOps.numPlantHXs; ++HXnum) {
                    if (this->PlantHXComps(HXnum).loopNum == this->DedicatedHR_CoolingPLHP.loadSidePlantLoc.loopNum) {
                        int outletnode = state.dataPlnt->PlantLoop(this->PlantHXComps(HXnum).loopNum)
                                             .LoopSide(this->PlantHXComps(HXnum).loopSideNum)
                                             .Branch(this->PlantHXComps(HXnum).branchNum)
                                             .Comp(this->PlantHXComps(HXnum).compNum)
                                             .NodeNumOut;
                        state.dataLoopNodes->Node(outletnode).TempSetPoint = this->Setpoint.SecCW;
                    }
                }
            }

        } else if (HeatLedNeed) {
            this->Report.DedicHR_OpMode = 1;
            // turn load side of this water to water heat pump
            state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                .Available = true;
            state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                .ON = true;
            state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                .CurOpSchemeType = this->Type;
            state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                .MyLoad = HW_Qdot;

            int OutletHWReturnNodeNum = state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)
                                            .LoopSide(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopSideNum)
                                            .Branch(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.branchNum)
                                            .Comp(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.compNum)
                                            .NodeNumOut;

            state.dataLoopNodes->Node(OutletHWReturnNodeNum).TempSetPoint = this->Setpoint.SecHW;
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum).TempSetPointNodeNum)
                .TempSetPoint = this->Setpoint.SecHW;

            if (this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum ==
                SecondaryPlantLoopIndicesBeingSupervised(this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum)) {
                // search for HX on this loop and place setpoint on outlet
                for (int HXnum = 1; HXnum <= this->PlantOps.numPlantHXs; ++HXnum) {
                    if (this->PlantHXComps(HXnum).loopNum == this->DedicatedHR_HeatingPLHP.loadSidePlantLoc.loopNum) {
                        int outletnode = state.dataPlnt->PlantLoop(this->PlantHXComps(HXnum).loopNum)
                                             .LoopSide(this->PlantHXComps(HXnum).loopSideNum)
                                             .Branch(this->PlantHXComps(HXnum).branchNum)
                                             .Comp(this->PlantHXComps(HXnum).compNum)
                                             .NodeNumOut;
                        state.dataLoopNodes->Node(outletnode).TempSetPoint = min(this->Setpoint.SecHW, this->DetermineHWSetpointOARest(state));
                    }
                }
            }
        }
    }

    void ChillerHeaterSupervisoryOperationData::ProcessAndSetAuxilBoiler(EnergyPlusData &state)
    {
        // Check for boiler used as auxiliary or supplemental
        // Assume boilers are in-line on supply side outlet branch, typically on secondary loop but may be on primary loop
        this->Report.BoilerAux_OpMode = 0;
        if (this->PlantOps.numBoilers <= 0) {
            return;
        }

        // first initialize them to be off
        if (this->PlantOps.numBoilers > 0) {
            for (int BoilerNum = 1; BoilerNum <= this->PlantOps.numBoilers; ++BoilerNum) {
                state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                    .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                    .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                    .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                    .Available = false;
                state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                    .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                    .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                    .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                    .ON = false;
                state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                    .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                    .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                    .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                    .MyLoad = 0.0;
            }
        }

        if (this->PlantOps.numBoilers > 0) {
            // Boilers will run if outdoor air temperature is too low and there is flow in HW return loop

            bool LowOAAuxiliaryNeeded = false;
            if (state.dataEnvrn->OutDryBulbTemp < this->TempReset.LowOutdoorTemp) {
                LowOAAuxiliaryNeeded = true;
            }
            for (int BoilerNum = 1; BoilerNum <= this->PlantOps.numBoilers; ++BoilerNum) {
                // determine if primary or secondary setpoint in use
                Real64 HWsetpt = 0.0;
                if (this->SecondaryPlantLoopIndicesBeingSupervised(this->PlantBoilerComps(BoilerNum).loopNum) >
                    0) { // appears to be on secondary loop, supplemental boiler
                    HWsetpt = min(this->Setpoint.SecHW,
                                  DetermineHWSetpointOARest(
                                      state)); // Assume if OA reset is lower than setting for secondary HW loop, then use the lower of the two
                } else {                       // primary loop, auxiliary boiler
                    HWsetpt = DetermineHWSetpointOARest(state);
                }

                HWsetpt = HWsetpt - this->TempReset.BoilerTemperatureOffset;

                // check inlet temperature
                int inletBoilerNodeNum = state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                                             .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                                             .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                                             .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                                             .NodeNumIn;
                Real64 Tin = state.dataLoopNodes->Node(inletBoilerNodeNum).Temp;
                Real64 Mdot = state.dataLoopNodes->Node(inletBoilerNodeNum).MassFlowRate;

                Real64 const CpHW = state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                                        .glycol->getSpecificHeat(state, Tin, "ChillerHeaterSupervisoryOperationData::ProcessAndSetAuxilBoiler");
                Real64 LoadToSetpoint = max(0.0, Mdot * CpHW * (HWsetpt - Tin));
                int pltSizNum = state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum).PlantSizNum;
                Real64 const thresholdPlantLoad =
                    0.001 * state.dataSize->PlantSizData(pltSizNum).DesCapacity; // model an operating threshold at 0.1% of loop capacity, only run if
                                                                                 // larger than that

                if (((LoadToSetpoint > thresholdPlantLoad) &&
                     ((this->Report.AirSourcePlant_OpMode == 0) ||
                      LowOAAuxiliaryNeeded)) || // run boiler if there is any heating load and also heatpumps are off or too cold outside
                    ((LoadToSetpoint > thresholdPlantLoad) &&
                     (this->Report.AirSourcePlant_OpMode ==
                      2))) { // run boiler if there is a somewhat significant heating load and heat pumps in cooling only mode

                    state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                        .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                        .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                        .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                        .Available = true;
                    state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                        .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                        .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                        .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                        .ON = true;
                    state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                        .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                        .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                        .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                        .CurOpSchemeType = this->Type;
                    // boilers don't really have setpoint control mode, so set value for myLoad
                    state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                        .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                        .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                        .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                        .MyLoad = LoadToSetpoint;
                    int OutletBoilerNodeNum = state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                                                  .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                                                  .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                                                  .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                                                  .NodeNumOut;
                    state.dataLoopNodes->Node(OutletBoilerNodeNum).TempSetPoint = HWsetpt;
                    state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum).TempSetPointNodeNum).TempSetPoint =
                        HWsetpt;
                    this->Report.BoilerAux_OpMode = 1;
                } else { // still apply the setpoint, but don't turn on
                    int OutletBoilerNodeNum = state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum)
                                                  .LoopSide(this->PlantBoilerComps(BoilerNum).loopSideNum)
                                                  .Branch(this->PlantBoilerComps(BoilerNum).branchNum)
                                                  .Comp(this->PlantBoilerComps(BoilerNum).compNum)
                                                  .NodeNumOut;
                    state.dataLoopNodes->Node(OutletBoilerNodeNum).TempSetPoint = HWsetpt;
                    state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->PlantBoilerComps(BoilerNum).loopNum).TempSetPointNodeNum).TempSetPoint =
                        HWsetpt;
                }
            }
        }
    }

    Real64 ChillerHeaterSupervisoryOperationData::DetermineHWSetpointOARest(EnergyPlusData &state)
    {
        // algorithm from calcSetPointLinInt
        Real64 HWSetpoint = 0.0;

        if ((this->TempReset.LowOutdoorTemp == this->TempReset.BackupLowOutdoorTemp) &&
            (this->Setpoint.PrimHW_Low == this->Setpoint.PrimHW_BackupLow)) { // no second-stage reset scheme

            if (this->TempReset.LowOutdoorTemp < this->TempReset.HighOutdoorTemp) {
                if (state.dataEnvrn->OutDryBulbTemp <= this->TempReset.LowOutdoorTemp) {
                    HWSetpoint = this->Setpoint.PrimHW_Low;
                } else if (state.dataEnvrn->OutDryBulbTemp >= this->TempReset.HighOutdoorTemp) {
                    HWSetpoint = this->Setpoint.PrimHW_High;
                } else {
                    HWSetpoint = this->Setpoint.PrimHW_Low - ((state.dataEnvrn->OutDryBulbTemp - this->TempReset.LowOutdoorTemp) /
                                                              (this->TempReset.HighOutdoorTemp - this->TempReset.LowOutdoorTemp)) *
                                                                 (this->Setpoint.PrimHW_Low - this->Setpoint.PrimHW_High);
                    HWSetpoint = min(HWSetpoint, this->Setpoint.PrimHW_High); // don't extrapolate, hold at high limit of primary HW
                }

            } else {
                HWSetpoint = 0.5 * (this->Setpoint.PrimHW_Low + this->Setpoint.PrimHW_High);
            }
        } else { // apply two stage reset scheme
            if ((this->TempReset.LowOutdoorTemp < this->TempReset.HighOutdoorTemp) &&
                (this->TempReset.BackupLowOutdoorTemp < this->TempReset.LowOutdoorTemp)) { // expected configuration

                if (state.dataEnvrn->OutDryBulbTemp <= this->TempReset.BackupLowOutdoorTemp) {
                    HWSetpoint = this->Setpoint.PrimHW_BackupLow;
                } else if (state.dataEnvrn->OutDryBulbTemp >= this->TempReset.HighOutdoorTemp) {
                    HWSetpoint = this->Setpoint.PrimHW_High;
                } else if ((state.dataEnvrn->OutDryBulbTemp >= this->TempReset.LowOutdoorTemp) &&
                           (state.dataEnvrn->OutDryBulbTemp < this->TempReset.HighOutdoorTemp)) { // first stage for Heat pump reset down
                    HWSetpoint = this->Setpoint.PrimHW_Low - ((state.dataEnvrn->OutDryBulbTemp - this->TempReset.LowOutdoorTemp) /
                                                              (this->TempReset.HighOutdoorTemp - this->TempReset.LowOutdoorTemp)) *
                                                                 (this->Setpoint.PrimHW_Low - this->Setpoint.PrimHW_High);
                    HWSetpoint = min(HWSetpoint, this->Setpoint.PrimHW_High); // don't extrapolate, hold at high limit of primary HW
                } else if ((state.dataEnvrn->OutDryBulbTemp > this->TempReset.BackupLowOutdoorTemp) &&
                           (state.dataEnvrn->OutDryBulbTemp < this->TempReset.LowOutdoorTemp)) { // second stage for backup boiler reset up
                    HWSetpoint = this->Setpoint.PrimHW_BackupLow - ((state.dataEnvrn->OutDryBulbTemp - this->TempReset.BackupLowOutdoorTemp) /
                                                                    (this->TempReset.LowOutdoorTemp - this->TempReset.BackupLowOutdoorTemp)) *
                                                                       (this->Setpoint.PrimHW_BackupLow - this->Setpoint.PrimHW_Low);
                    HWSetpoint = min(HWSetpoint, this->Setpoint.PrimHW_BackupLow); // don't extrapolate
                    HWSetpoint = max(HWSetpoint, this->Setpoint.PrimHW_Low);       // don't extrapolate
                } else {
                    // shouldn't get here, throw error?
                }
            } else { // malformed input, take average of three setpoints
                HWSetpoint = (this->Setpoint.PrimHW_Low + this->Setpoint.PrimHW_High + this->Setpoint.PrimHW_BackupLow) / 3.0;
            }
        }
        return HWSetpoint;
    }

} // namespace DataPlant
} // namespace EnergyPlus
