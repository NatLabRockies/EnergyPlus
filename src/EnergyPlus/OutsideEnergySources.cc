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
#include <iostream>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutsideEnergySources.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::OutsideEnergySources {

// MODULE INFORMATION:
//       AUTHOR         Dan Fisher
//       DATE WRITTEN   Unknown
//       MODIFIED       na
//       RE-ENGINEERED  Brent Griffith, Sept 2010, revised plant interactions.

// PURPOSE OF THIS MODULE:
// Module containing the routines dealing with the OutsideEnergySources

PlantComponent *OutsideEnergySourceSpecs::factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string_view objectName)
{
    // Process the input data for outside energy sources if it hasn't been done already
    if (state.dataOutsideEnergySrcs->SimOutsideEnergyGetInputFlag) {
        GetOutsideEnergySourcesInput(state);
        state.dataOutsideEnergySrcs->SimOutsideEnergyGetInputFlag = false;
    }
    // Now look for this particular pipe in the list
    for (auto &source : state.dataOutsideEnergySrcs->EnergySource) {
        if (source.EnergyType == objectType && source.Name == objectName) {
            return &source;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("OutsideEnergySourceSpecsFactory: Error getting inputs for source named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void OutsideEnergySourceSpecs::simulate(EnergyPlusData &state,
                                        [[maybe_unused]] const PlantLocation &calledFromLocation,
                                        [[maybe_unused]] bool FirstHVACIteration,
                                        Real64 &CurLoad,
                                        bool RunFlag)
{
    this->initialize(state, CurLoad);
    this->calculate(state, RunFlag, CurLoad);
}

void OutsideEnergySourceSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &)
{
    this->initialize(state, 0.0);
    this->size(state);
}

void OutsideEnergySourceSpecs::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                                   [[maybe_unused]] const PlantLocation &calledFromLocation,
                                                   Real64 &MaxLoad,
                                                   Real64 &MinLoad,
                                                   Real64 &OptLoad)
{
    MinLoad = 0.0;
    MaxLoad = this->NomCap;
    OptLoad = this->NomCap;
}

void GetOutsideEnergySourcesInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   April 1998
    //       MODIFIED       May 2010; Edwin Lee; Linda Lawrie (consolidation)
    //                      June 2022, Dareum Nam, Add DistrictHeatingSteam
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine obtains the input data puts it into the
    // component arrays. Data items in the component arrays
    // are initialized. Output variables are set up.

    static constexpr std::string_view routineName = "GetOutsideEnergySourcesInput";

    // GET NUMBER OF ALL EQUIPMENT TYPES
    int const NumDistrictUnitsHeatWater = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "DistrictHeating:Water");
    int const NumDistrictUnitsCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "DistrictCooling");
    int const NumDistrictUnitsHeatSteam = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "DistrictHeating:Steam");
    state.dataOutsideEnergySrcs->NumDistrictUnits = NumDistrictUnitsHeatWater + NumDistrictUnitsCool + NumDistrictUnitsHeatSteam;

    if (allocated(state.dataOutsideEnergySrcs->EnergySource)) {
        return;
    }

    state.dataOutsideEnergySrcs->EnergySource.allocate(state.dataOutsideEnergySrcs->NumDistrictUnits);
    state.dataOutsideEnergySrcs->EnergySourceUniqueNames.reserve(static_cast<unsigned>(state.dataOutsideEnergySrcs->NumDistrictUnits));

    bool ErrorsFound(false); // If errors detected in input
    int heatWaterIndex = 0;
    int coolIndex = 0;
    int heatSteamIndex = 0;

    for (int EnergySourceNum = 1; EnergySourceNum <= state.dataOutsideEnergySrcs->NumDistrictUnits; ++EnergySourceNum) {

        std::string nodeNames;
        DataPlant::PlantEquipmentType EnergyType;
        DataLoopNode::ConnectionObjectType objType;
        int thisIndex;
        if (EnergySourceNum <= NumDistrictUnitsHeatWater) {
            state.dataIPShortCut->cCurrentModuleObject = "DistrictHeating:Water";
            objType = DataLoopNode::ConnectionObjectType::DistrictHeatingWater;
            nodeNames = "Hot Water Nodes";
            EnergyType = DataPlant::PlantEquipmentType::PurchHotWater;
            heatWaterIndex++;
            thisIndex = heatWaterIndex;
        } else if (EnergySourceNum <= NumDistrictUnitsHeatWater + NumDistrictUnitsCool) {
            state.dataIPShortCut->cCurrentModuleObject = "DistrictCooling";
            objType = DataLoopNode::ConnectionObjectType::DistrictCooling;
            nodeNames = "Chilled Water Nodes";
            EnergyType = DataPlant::PlantEquipmentType::PurchChilledWater;
            coolIndex++;
            thisIndex = coolIndex;
        } else { // EnergySourceNum > NumDistrictUnitsHeatWater + NumDistrictUnitsCool
            state.dataIPShortCut->cCurrentModuleObject = "DistrictHeating:Steam";
            objType = DataLoopNode::ConnectionObjectType::DistrictHeatingSteam;
            nodeNames = "Steam Nodes";
            EnergyType = DataPlant::PlantEquipmentType::PurchSteam;
            heatSteamIndex++;
            thisIndex = heatSteamIndex;
        }

        int NumAlphas = 0, NumNums = 0, IOStat = 0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 state.dataIPShortCut->cCurrentModuleObject,
                                                                 thisIndex,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames);

        ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        if (EnergySourceNum > 1) {
            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataOutsideEnergySrcs->EnergySourceUniqueNames,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);
        }
        state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).Name = state.dataIPShortCut->cAlphaArgs(1);
        if (EnergySourceNum <= NumDistrictUnitsHeatWater + NumDistrictUnitsCool) {
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).InletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(2),
                                                    ErrorsFound,
                                                    objType,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).OutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(3),
                                                    ErrorsFound,
                                                    objType,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
        } else {
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).InletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(2),
                                                    ErrorsFound,
                                                    objType,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Steam,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).OutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(3),
                                                    ErrorsFound,
                                                    objType,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Steam,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
        }
        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(2),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           nodeNames);
        state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).NomCap = state.dataIPShortCut->rNumericArgs(1);
        if (state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).NomCap == DataSizing::AutoSize) {
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).NomCapWasAutoSized = true;
        }
        state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).EnergyTransfer = 0.0;
        state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).EnergyRate = 0.0;
        state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).EnergyType = EnergyType;

        if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
            state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).capFractionSched =
                Sched::GetScheduleAlwaysOn(state); // Defaults to constant-1.0
        } else if ((state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).capFractionSched =
                        Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
            ErrorsFound = true;
        } else if (!state.dataOutsideEnergySrcs->EnergySource(EnergySourceNum).capFractionSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowWarningBadMin(state,
                                     eoh,
                                     state.dataIPShortCut->cAlphaFieldNames(4),
                                     state.dataIPShortCut->cAlphaArgs(4),
                                     Clusive::In,
                                     0.0,
                                     "Negative values will be treated as zero, and the simulation continues.");
        }
    }

    if (ErrorsFound) {
        ShowFatalError(
            state,
            format("Errors found in processing input for {}, Preceding condition caused termination.", state.dataIPShortCut->cCurrentModuleObject));
    }
}

void OutsideEnergySourceSpecs::initialize(EnergyPlusData &state, Real64 MyLoad)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    October 1998
    //       MODIFIED       May 2010; Edwin Lee; Linda Lawrie (consolidation)
    //       RE-ENGINEERED  Sept 2010, Brent Griffith, plant rewrite

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does one-time inits and sets the operating mass flow rate of this machine

    // METHODOLOGY EMPLOYED:
    // One time inits include validating source type (should happen in getinput?) and locating this
    //  component on the PlantLoop topology.
    // The mass flow rate is determined based on component load, and making use of
    //  the SetComponentFlowRate routine.
    // The mass flow rate could be an inter-connected-loop side trigger. This is not really the type of
    //  interconnect that that routine was written for, but it is the clearest example of using it.

    auto &loop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);

    // begin environment inits
    if (state.dataGlobal->BeginEnvrnFlag && this->BeginEnvrnInitFlag) {
        // component model has not design flow rates, using data for overall plant loop
        PlantUtilities::InitComponentNodes(state, loop.MinMassFlowRate, loop.MaxMassFlowRate, this->InletNodeNum, this->OutletNodeNum);
        this->BeginEnvrnInitFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->BeginEnvrnInitFlag = true;
    }

    Real64 TempPlantMassFlow(0.0);
    if (std::abs(MyLoad) > 0.0) {
        TempPlantMassFlow = loop.MaxMassFlowRate;
    }

    // get actual mass flow to use, hold in MassFlowRate variable
    PlantUtilities::SetComponentFlowRate(state, TempPlantMassFlow, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);

    this->InletTemp = state.dataLoopNodes->Node(this->InletNodeNum).Temp;
    this->MassFlowRate = TempPlantMassFlow;
}

void OutsideEnergySourceSpecs::size(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   April 2014
    //       MODIFIED       June 2021, Dareum Nam, Add DistrictHeatingSteam
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for sizing capacities of district cooling and heating objects.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false); // If errors detected in input

    // Type name string variable to collapse the sizing for cooling and heating into one block
    std::string_view typeName = DataPlant::PlantEquipTypeNames[static_cast<int>(this->EnergyType)];

    auto &loop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);
    int const PltSizNum = loop.PlantSizNum;
    if (PltSizNum > 0) {
        Real64 NomCapDes;
        if (this->EnergyType == DataPlant::PlantEquipmentType::PurchChilledWater ||
            this->EnergyType == DataPlant::PlantEquipmentType::PurchHotWater) {
            Real64 const rho = loop.glycol->getDensity(state, Constant::InitConvTemp, format("Size {}", typeName));
            Real64 const Cp = loop.glycol->getSpecificHeat(state, Constant::InitConvTemp, format("Size {}", typeName));
            NomCapDes = Cp * rho * state.dataSize->PlantSizData(PltSizNum).DeltaT * state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate;
        } else { // this->EnergyType == DataPlant::TypeOf_PurchSteam
            Real64 const tempSteam = loop.steam->getSatTemperature(state, state.dataEnvrn->StdBaroPress, format("Size {}", typeName));
            Real64 const rhoSteam = loop.steam->getSatDensity(state, tempSteam, 1.0, format("Size {}", typeName));
            Real64 const EnthSteamDry = loop.steam->getSatEnthalpy(state, tempSteam, 1.0, format("Size {}", typeName));
            Real64 const EnthSteamWet = loop.steam->getSatEnthalpy(state, tempSteam, 0.0, format("Size {}", typeName));
            Real64 const LatentHeatSteam = EnthSteamDry - EnthSteamWet;
            NomCapDes = rhoSteam * state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * LatentHeatSteam;
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->NomCapWasAutoSized) {
                this->NomCap = NomCapDes;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, typeName, this->Name, "Design Size Nominal Capacity [W]", NomCapDes);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, typeName, this->Name, "Initial Design Size Nominal Capacity [W]", NomCapDes);
                }
            } else { // Hard-size with sizing data
                if (this->NomCap > 0.0 && NomCapDes > 0.0) {
                    Real64 const NomCapUser = this->NomCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     typeName,
                                                     this->Name,
                                                     "Design Size Nominal Capacity [W]",
                                                     NomCapDes,
                                                     "User-Specified Nominal Capacity [W]",
                                                     NomCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(NomCapDes - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("Size {}: Potential issue with equipment sizing for {}", typeName, this->Name));
                                ShowContinueError(state, format("User-Specified Nominal Capacity of {:.2R} [W]", NomCapUser));
                                ShowContinueError(state, format("differs from Design Size Nominal Capacity of {:.2R} [W]", NomCapDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }
    } else {
        if (this->NomCapWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, format("Autosizing of {} nominal capacity requires a loop Sizing:Plant object", typeName));
            ShowContinueError(state, format("Occurs in {} object={}", typeName, this->Name));
            ErrorsFound = true;
        }
        if (!this->NomCapWasAutoSized && this->NomCap > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, typeName, this->Name, "User-Specified Nominal Capacity [W]", this->NomCap);
        }
    }
    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void OutsideEnergySourceSpecs::calculate(EnergyPlusData &state, bool runFlag, Real64 MyLoad)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   July 1998
    //       MODIFIED       May 2010; Edwin Lee; Linda Lawrie (consolidation)
    //                      June 2021, Dareum Nam, Add DistrictHeatingSteam
    //       RE-ENGINEERED  Sept 2010, Brent Griffith, plant rewrite

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SimDistrictEnergy");

    auto &loop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);

    // set inlet and outlet nodes
    int const LoopNum = this->plantLoc.loopNum;
    Real64 const LoopMinTemp = state.dataPlnt->PlantLoop(LoopNum).MinTemp;
    Real64 const LoopMaxTemp = state.dataPlnt->PlantLoop(LoopNum).MaxTemp;
    Real64 const LoopMinMdot = state.dataPlnt->PlantLoop(LoopNum).MinMassFlowRate;
    Real64 const LoopMaxMdot = state.dataPlnt->PlantLoop(LoopNum).MaxMassFlowRate;

    //  apply power limit from input
    Real64 CapFraction = this->capFractionSched->getCurrentVal();
    CapFraction = max(0.0, CapFraction); // ensure non negative
    Real64 const CurrentCap = this->NomCap * CapFraction;
    if (std::abs(MyLoad) > CurrentCap) {
        MyLoad = sign(CurrentCap, MyLoad);
    }

    if (this->EnergyType == DataPlant::PlantEquipmentType::PurchChilledWater) {
        if (MyLoad > 0.0) {
            MyLoad = 0.0;
        }
    } else if (this->EnergyType == DataPlant::PlantEquipmentType::PurchHotWater || this->EnergyType == DataPlant::PlantEquipmentType::PurchSteam) {
        if (MyLoad < 0.0) {
            MyLoad = 0.0;
        }
    }

    // determine outlet temp based on inlet temp, cp, and MyLoad
    if ((this->MassFlowRate > 0.0) && runFlag) {
        if (this->EnergyType == DataPlant::PlantEquipmentType::PurchChilledWater ||
            this->EnergyType == DataPlant::PlantEquipmentType::PurchHotWater) {
            Real64 const Cp = state.dataPlnt->PlantLoop(LoopNum).glycol->getSpecificHeat(state, this->InletTemp, RoutineName);
            this->OutletTemp = (MyLoad + this->MassFlowRate * Cp * this->InletTemp) / (this->MassFlowRate * Cp);
            // apply loop limits on temperature result to keep in check
            if (this->OutletTemp < LoopMinTemp) {
                this->OutletTemp = max(this->OutletTemp, LoopMinTemp);
                MyLoad = this->MassFlowRate * Cp * (this->OutletTemp - this->InletTemp);
            }
            if (this->OutletTemp > LoopMaxTemp) {
                this->OutletTemp = min(this->OutletTemp, LoopMaxTemp);
                MyLoad = this->MassFlowRate * Cp * (this->OutletTemp - this->InletTemp);
            }
        } else if (this->EnergyType == DataPlant::PlantEquipmentType::PurchSteam) { // determine mass flow rate based on inlet temp, saturate temp at
                                                                                    // atmospheric pressure, Cp of inlet condensate, and MyLoad
            Real64 SatTempAtmPress = loop.steam->getSatTemperature(state, DataEnvironment::StdPressureSeaLevel, RoutineName);
            Real64 CpCondensate = loop.glycol->getSpecificHeat(state, this->InletTemp, RoutineName);
            Real64 deltaTsensible = SatTempAtmPress - this->InletTemp;
            Real64 EnthSteamInDry = loop.steam->getSatEnthalpy(state, this->InletTemp, 1.0, RoutineName);
            Real64 EnthSteamOutWet = loop.steam->getSatEnthalpy(state, this->InletTemp, 0.0, RoutineName);
            Real64 LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
            this->MassFlowRate = MyLoad / (LatentHeatSteam + (CpCondensate * deltaTsensible));
            PlantUtilities::SetComponentFlowRate(state, this->MassFlowRate, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);
            // Like the assumption in Boiler:Steam, assume that it can meet the steam loop setpoint
            this->OutletTemp = state.dataLoopNodes->Node(loop.TempSetPointNodeNum).TempSetPoint;
            this->OutletSteamQuality = 0.0;
            // apply loop limits on mass flow rate result to keep in check
            if (this->MassFlowRate < LoopMinMdot) {
                this->MassFlowRate = max(this->MassFlowRate, LoopMinMdot);
                PlantUtilities::SetComponentFlowRate(state, this->MassFlowRate, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);
                MyLoad = this->MassFlowRate * LatentHeatSteam;
            }
            if (this->MassFlowRate > LoopMaxMdot) {
                this->MassFlowRate = min(this->MassFlowRate, LoopMaxMdot);
                PlantUtilities::SetComponentFlowRate(state, this->MassFlowRate, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);
                MyLoad = this->MassFlowRate * LatentHeatSteam;
            }
            // Like the assumption in Boiler:Steam, assume that saturated steam is leaving the district heating steam plant
            state.dataLoopNodes->Node(this->OutletNodeNum).Quality = 1.0;
        }
    } else {
        this->OutletTemp = this->InletTemp;
        MyLoad = 0.0;
    }
    int const OutletNode = this->OutletNodeNum;
    state.dataLoopNodes->Node(OutletNode).Temp = this->OutletTemp;
    this->EnergyRate = std::abs(MyLoad);
    this->EnergyTransfer = this->EnergyRate * state.dataHVACGlobal->TimeStepSysSec;
}

void OutsideEnergySourceSpecs::oneTimeInit_new(EnergyPlusData &state)
{
    // Locate the unit on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->EnergyType, this->plantLoc, errFlag, _, _, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "InitSimVars: Program terminated due to previous condition(s).");
    }

    auto &loop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);
    // set limits on outlet node temps to plant loop limits
    DataPlant::CompData::getPlantComponent(state, this->plantLoc).MinOutletTemp = loop.MinTemp;
    DataPlant::CompData::getPlantComponent(state, this->plantLoc).MaxOutletTemp = loop.MaxTemp;
    // Register design flow rate for inlet node (helps to autosize comp setpoint op scheme flows
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->InletNodeNum, loop.MaxVolFlowRate);

    std::string reportVarPrefix = "District Heating Water ";
    OutputProcessor::EndUseCat heatingOrCooling = OutputProcessor::EndUseCat::Heating;
    Constant::eResource meterTypeKey = Constant::eResource::DistrictHeatingWater;

    if (this->EnergyType == DataPlant::PlantEquipmentType::PurchChilledWater) {
        reportVarPrefix = "District Cooling Water ";
        heatingOrCooling = OutputProcessor::EndUseCat::Cooling;
        meterTypeKey = Constant::eResource::DistrictCooling;
    } else if (this->EnergyType == DataPlant::PlantEquipmentType::PurchSteam) {
        reportVarPrefix = "District Heating Steam ";
        heatingOrCooling = OutputProcessor::EndUseCat::Heating;
        meterTypeKey = Constant::eResource::DistrictHeatingSteam;
    }
    SetupOutputVariable(state,
                        format("{}Energy", reportVarPrefix),
                        Constant::Units::J,
                        this->EnergyTransfer,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        meterTypeKey,
                        OutputProcessor::Group::Plant,
                        heatingOrCooling);
    SetupOutputVariable(state,
                        format("{}Rate", reportVarPrefix),
                        Constant::Units::W,
                        this->EnergyRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("{}Inlet Temperature", reportVarPrefix),
                        Constant::Units::C,
                        this->InletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("{}Outlet Temperature", reportVarPrefix),
                        Constant::Units::C,
                        this->OutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("{}Mass Flow Rate", reportVarPrefix),
                        Constant::Units::kg_s,
                        this->MassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
}

void OutsideEnergySourceSpecs::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::OutsideEnergySources
