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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerAbsorption.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::ChillerAbsorption {

// MODULE INFORMATION:
//       AUTHOR         Dan Fisher
//       DATE WRITTEN   Nov. 2000

// PURPOSE OF THIS MODULE:
// This module simulates the performance of the BLAST absorbers.

// METHODOLOGY EMPLOYED:
// Once the PlantLoopManager determines that the BLAST absorber
// is available to meet a loop cooling demand, it calls SimBLAST
// absorber which in turn calls the appropriate Absorption Chiller model.
// All Absorption Chiller models are based on a polynomial fit of Absorber
// performance data.

// REFERENCES:
// 1. BLAST Users Manual

// OTHER NOTES:
// The Absorber program from the BLAST family of software can be used
// to generate the coefficients for the model.

const char *calcChillerAbsorption("CALC Chiller:Absorption ");
const char *moduleObjectType("Chiller:Absorption");

BLASTAbsorberSpecs *BLASTAbsorberSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data
    if (state.dataChillerAbsorber->getInput) {
        GetBLASTAbsorberInput(state);
        state.dataChillerAbsorber->getInput = false;
    }
    // Now look for this particular object
    auto thisAbs = std::find_if(state.dataChillerAbsorber->absorptionChillers.begin(),
                                state.dataChillerAbsorber->absorptionChillers.end(),
                                [&objectName](const BLASTAbsorberSpecs &myAbs) { return myAbs.Name == objectName; });
    if (thisAbs != state.dataChillerAbsorber->absorptionChillers.end()) {
        return thisAbs;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalBlastAbsorberFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void BLASTAbsorberSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{

    this->EquipFlowCtrl = state.dataPlnt->PlantLoop(calledFromLocation.loopNum)
                              .LoopSide(calledFromLocation.loopSideNum)
                              .Branch(calledFromLocation.branchNum)
                              .Comp(calledFromLocation.compNum)
                              .FlowCtrl;

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        // called from dominant chilled water connection loop side

        // Calculate Load
        this->initialize(state, RunFlag, CurLoad);
        this->calculate(state, CurLoad, RunFlag);
        this->updateRecords(state, CurLoad, RunFlag);

    } else if (calledFromLocation.loopNum == this->CDPlantLoc.loopNum) {
        // Called from non-dominant condenser water connection loop side
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            calledFromLocation.loopSideNum,
                                                            DataPlant::PlantEquipmentType::Chiller_Absorption,
                                                            this->CondInletNodeNum,
                                                            this->CondOutletNodeNum,
                                                            this->Report.QCond,
                                                            this->Report.CondInletTemp,
                                                            this->Report.CondOutletTemp,
                                                            this->Report.Condmdot,
                                                            FirstHVACIteration);

    } else if (calledFromLocation.loopNum == this->GenPlantLoc.loopNum) {
        // Called from non-dominant generator hot water or steam connection loop side
        PlantUtilities::UpdateAbsorberChillerComponentGeneratorSide(state,
                                                                    calledFromLocation.loopNum,
                                                                    calledFromLocation.loopSideNum,
                                                                    DataPlant::PlantEquipmentType::Chiller_Absorption,
                                                                    this->GeneratorInletNodeNum,
                                                                    this->GeneratorOutletNodeNum,
                                                                    this->GenHeatSourceType,
                                                                    this->Report.QGenerator,
                                                                    this->Report.SteamMdot,
                                                                    FirstHVACIteration);

    } else {
        ShowFatalError(state,
                       format("SimBLASTAbsorber: Invalid LoopNum passed={}, Unit name={}, stored chilled water loop={}, stored condenser water "
                              "loop={}, stored generator loop={}",
                              calledFromLocation.loopNum,
                              this->Name,
                              this->CWPlantLoc.loopNum,
                              this->CDPlantLoc.loopNum,
                              this->GenPlantLoc.loopNum));
    }
}

void BLASTAbsorberSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
{
    bool runFlag = true;
    Real64 myLoad = 0.0;

    this->initialize(state, runFlag, myLoad);

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->sizeChiller(state);
    }
}

void BLASTAbsorberSpecs::getDesignCapacities(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->sizeChiller(state);
        MinLoad = this->NomCap * this->MinPartLoadRat;
        MaxLoad = this->NomCap * this->MaxPartLoadRat;
        OptLoad = this->NomCap * this->OptPartLoadRat;
    } else {
        MinLoad = 0.0;
        MaxLoad = 0.0;
        OptLoad = 0.0;
    }
}

void BLASTAbsorberSpecs::getSizingFactor(Real64 &sizFac)
{
    sizFac = this->SizFac;
}

void BLASTAbsorberSpecs::getDesignTemperatures(Real64 &tempDesCondIn, [[maybe_unused]] Real64 &TempDesEvapOut)
{
    tempDesCondIn = this->TempDesCondIn;
}

void GetBLASTAbsorberInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    April 1998
    //       MODIFIED:        R. Raustad May 2008 - added generator nodes

    // PURPOSE OF THIS SUBROUTINE:
    // This routine will get the input
    // required by the BLAST Absorption chiller models as shown below:

    // METHODOLOGY EMPLOYED:
    // EnergyPlus input processor

    constexpr const char *RoutineName("GetBLASTAbsorberInput: "); // include trailing blank space

    int NumAlphas = 0; // Number of elements in the alpha array
    int NumNums = 0;   // Number of elements in the numeric array
    int IOStat = 0;    // IO Status when calling get input subroutine
    bool ErrorsFound(false);

    state.dataIPShortCut->cCurrentModuleObject = moduleObjectType;

    int numAbsorbers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

    if (numAbsorbers <= 0) {
        ShowSevereError(state, format("No {} equipment specified in input file", state.dataIPShortCut->cCurrentModuleObject));
        // See if load distribution manager has already gotten the input
        ErrorsFound = true;
    }

    if (allocated(state.dataChillerAbsorber->absorptionChillers)) {
        return;
    }

    state.dataChillerAbsorber->absorptionChillers.allocate(numAbsorbers);

    // LOAD ARRAYS WITH BLAST CURVE FIT Absorber DATA
    for (int AbsorberNum = 1; AbsorberNum <= numAbsorbers; ++AbsorberNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 state.dataIPShortCut->cCurrentModuleObject,
                                                                 AbsorberNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueChillerName(state,
                                             state.dataIPShortCut->cCurrentModuleObject,
                                             state.dataIPShortCut->cAlphaArgs(1),
                                             ErrorsFound,
                                             state.dataIPShortCut->cCurrentModuleObject + " Name");

        auto &thisChiller = state.dataChillerAbsorber->absorptionChillers(AbsorberNum);
        thisChiller.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisChiller.NomCap = state.dataIPShortCut->rNumericArgs(1);
        if (thisChiller.NomCap == DataSizing::AutoSize) {
            thisChiller.NomCapWasAutoSized = true;
        }
        thisChiller.NomPumpPower = state.dataIPShortCut->rNumericArgs(2);
        if (thisChiller.NomPumpPower == DataSizing::AutoSize) {
            thisChiller.NomPumpPowerWasAutoSized = true;
        }
        if (state.dataIPShortCut->rNumericArgs(1) == 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->rNumericArgs(1)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ErrorsFound = true;
        }
        // Assign Node Numbers to specified nodes
        thisChiller.EvapInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           state.dataIPShortCut->cAlphaArgs(2),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                           state.dataIPShortCut->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.EvapOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            state.dataIPShortCut->cAlphaArgs(3),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                            state.dataIPShortCut->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(2),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           "Chilled Water Nodes");

        thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           state.dataIPShortCut->cAlphaArgs(4),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                           state.dataIPShortCut->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Secondary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            state.dataIPShortCut->cAlphaArgs(5),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                            state.dataIPShortCut->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Secondary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(4),
                                           state.dataIPShortCut->cAlphaArgs(5),
                                           "Condenser (not tested) Nodes");

        if (NumAlphas > 8) {
            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(9), "HotWater") ||
                Util::SameString(state.dataIPShortCut->cAlphaArgs(9), "HotWater")) {
                thisChiller.GenHeatSourceType = DataLoopNode::NodeFluidType::Water;
            } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(9), "STEAM") || state.dataIPShortCut->cAlphaArgs(9).empty()) {
                thisChiller.GenHeatSourceType = DataLoopNode::NodeFluidType::Steam;
            } else {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(9), state.dataIPShortCut->cAlphaArgs(9)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "...Generator heat source type must be Steam or Hot Water.");
                ErrorsFound = true;
            }
        } else {
            thisChiller.GenHeatSourceType = DataLoopNode::NodeFluidType::Steam;
        }

        if (!state.dataIPShortCut->lAlphaFieldBlanks(6) && !state.dataIPShortCut->lAlphaFieldBlanks(7)) {
            thisChiller.GenInputOutputNodesUsed = true;
            if (thisChiller.GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
                thisChiller.GeneratorInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                        state.dataIPShortCut->cAlphaArgs(6),
                                                                                        ErrorsFound,
                                                                                        DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                                                        DataLoopNode::NodeFluidType::Water,
                                                                                        DataLoopNode::ConnectionType::Inlet,
                                                                                        NodeInputManager::CompFluidStream::Tertiary,
                                                                                        DataLoopNode::ObjectIsNotParent);
                thisChiller.GeneratorOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                         state.dataIPShortCut->cAlphaArgs(7),
                                                                                         ErrorsFound,
                                                                                         DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                                         state.dataIPShortCut->cAlphaArgs(1),
                                                                                         DataLoopNode::NodeFluidType::Water,
                                                                                         DataLoopNode::ConnectionType::Outlet,
                                                                                         NodeInputManager::CompFluidStream::Tertiary,
                                                                                         DataLoopNode::ObjectIsNotParent);
                BranchNodeConnections::TestCompSet(state,
                                                   state.dataIPShortCut->cCurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(6),
                                                   state.dataIPShortCut->cAlphaArgs(7),
                                                   "Hot Water Nodes");
            } else {
                thisChiller.steam = Fluid::GetSteam(state);
                thisChiller.GeneratorInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                        state.dataIPShortCut->cAlphaArgs(6),
                                                                                        ErrorsFound,
                                                                                        DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                                                        DataLoopNode::NodeFluidType::Steam,
                                                                                        DataLoopNode::ConnectionType::Inlet,
                                                                                        NodeInputManager::CompFluidStream::Tertiary,
                                                                                        DataLoopNode::ObjectIsNotParent);
                thisChiller.GeneratorOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                         state.dataIPShortCut->cAlphaArgs(7),
                                                                                         ErrorsFound,
                                                                                         DataLoopNode::ConnectionObjectType::ChillerAbsorption,
                                                                                         state.dataIPShortCut->cAlphaArgs(1),
                                                                                         DataLoopNode::NodeFluidType::Steam,
                                                                                         DataLoopNode::ConnectionType::Outlet,
                                                                                         NodeInputManager::CompFluidStream::Tertiary,
                                                                                         DataLoopNode::ObjectIsNotParent);
                BranchNodeConnections::TestCompSet(state,
                                                   state.dataIPShortCut->cCurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(6),
                                                   state.dataIPShortCut->cAlphaArgs(7),
                                                   "Steam Nodes");
            }
        } else if ((state.dataIPShortCut->lAlphaFieldBlanks(6) && !state.dataIPShortCut->lAlphaFieldBlanks(7)) ||
                   (!state.dataIPShortCut->lAlphaFieldBlanks(6) && state.dataIPShortCut->lAlphaFieldBlanks(7))) {
            ShowSevereError(state, format("{}, Name={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, "...Generator fluid nodes must both be entered (or both left blank).");
            ShowContinueError(state, format("...{} = {}", state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6)));
            ShowContinueError(state, format("...{} = {}", state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7)));
            ErrorsFound = true;
        } else {
            if (thisChiller.GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
                ShowWarningError(state, format("{}, Name={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "...Generator fluid type must be Steam if generator inlet/outlet nodes are blank.");
                ShowContinueError(state, "...Generator fluid type is set to Steam and the simulation continues.");
                thisChiller.GenHeatSourceType = DataLoopNode::NodeFluidType::Steam;
            }
        }

        // Get remaining data
        thisChiller.MinPartLoadRat = state.dataIPShortCut->rNumericArgs(3);
        thisChiller.MaxPartLoadRat = state.dataIPShortCut->rNumericArgs(4);
        thisChiller.OptPartLoadRat = state.dataIPShortCut->rNumericArgs(5);
        thisChiller.TempDesCondIn = state.dataIPShortCut->rNumericArgs(6);
        thisChiller.EvapVolFlowRate = state.dataIPShortCut->rNumericArgs(7);
        if (thisChiller.EvapVolFlowRate == DataSizing::AutoSize) {
            thisChiller.EvapVolFlowRateWasAutoSized = true;
        }
        thisChiller.CondVolFlowRate = state.dataIPShortCut->rNumericArgs(8);
        if (thisChiller.CondVolFlowRate == DataSizing::AutoSize) {
            thisChiller.CondVolFlowRateWasAutoSized = true;
        }
        thisChiller.SteamLoadCoef[0] = state.dataIPShortCut->rNumericArgs(9);
        thisChiller.SteamLoadCoef[1] = state.dataIPShortCut->rNumericArgs(10);
        thisChiller.SteamLoadCoef[2] = state.dataIPShortCut->rNumericArgs(11);
        thisChiller.PumpPowerCoef[0] = state.dataIPShortCut->rNumericArgs(12);
        thisChiller.PumpPowerCoef[1] = state.dataIPShortCut->rNumericArgs(13);
        thisChiller.PumpPowerCoef[2] = state.dataIPShortCut->rNumericArgs(14);
        thisChiller.TempLowLimitEvapOut = state.dataIPShortCut->rNumericArgs(15);

        thisChiller.FlowMode = static_cast<DataPlant::FlowMode>(getEnumValue(DataPlant::FlowModeNamesUC, state.dataIPShortCut->cAlphaArgs(8)));
        if (thisChiller.FlowMode == DataPlant::FlowMode::Invalid) {
            ShowSevereError(state,
                            format("{}{}=\"{}\",", RoutineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(8), state.dataIPShortCut->cAlphaArgs(8)));
            ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
            ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
            thisChiller.FlowMode = DataPlant::FlowMode::NotModulated;
        };

        if (NumNums > 15) {
            thisChiller.GeneratorVolFlowRate = state.dataIPShortCut->rNumericArgs(16);
            if (thisChiller.GeneratorVolFlowRate == DataSizing::AutoSize) {
                thisChiller.GeneratorVolFlowRateWasAutoSized = true;
            }
        }

        if (thisChiller.GeneratorVolFlowRate == 0.0 && thisChiller.GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(16), state.dataIPShortCut->rNumericArgs(16)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, "...Generator water flow rate must be greater than 0 when absorber generator fluid type is hot water.");
            ErrorsFound = true;
        }

        if (NumNums > 16) {
            thisChiller.GeneratorSubcool = state.dataIPShortCut->rNumericArgs(17);
        } else {
            thisChiller.GeneratorSubcool = 1.0;
        }

        if (NumNums > 17) {
            thisChiller.SizFac = state.dataIPShortCut->rNumericArgs(18);
        } else {
            thisChiller.SizFac = 1.0;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
    }
}

void BLASTAbsorberSpecs::setupOutputVars(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Chiller Electricity Rate",
                        Constant::Units::W,
                        this->Report.PumpingPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Electricity Energy",
                        Constant::Units::J,
                        this->Report.PumpingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling);
    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Rate",
                        Constant::Units::W,
                        this->Report.QEvap,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Energy",
                        Constant::Units::J,
                        this->Report.EvapEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Chillers);
    SetupOutputVariable(state,
                        "Chiller Evaporator Inlet Temperature",
                        Constant::Units::C,
                        this->Report.EvapInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Evaporator Outlet Temperature",
                        Constant::Units::C,
                        this->Report.EvapOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Evaporator Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->Report.Evapmdot,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Rate",
                        Constant::Units::W,
                        this->Report.QCond,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Energy",
                        Constant::Units::J,
                        this->Report.CondEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::HeatRejection);
    SetupOutputVariable(state,
                        "Chiller Condenser Inlet Temperature",
                        Constant::Units::C,
                        this->Report.CondInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Condenser Outlet Temperature",
                        Constant::Units::C,
                        this->Report.CondOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Condenser Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->Report.Condmdot,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
        SetupOutputVariable(state,
                            "Chiller Hot Water Consumption Rate",
                            Constant::Units::W,
                            this->Report.QGenerator,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Chiller Source Hot Water Energy",
                            Constant::Units::J,
                            this->Report.GeneratorEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::PlantLoopHeatingDemand,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Chillers);
    } else {
        if (this->GenInputOutputNodesUsed) {
            SetupOutputVariable(state,
                                "Chiller Source Steam Rate",
                                Constant::Units::W,
                                this->Report.QGenerator,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Chiller Source Steam Energy",
                                Constant::Units::J,
                                this->Report.GeneratorEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Chillers);
        } else {
            SetupOutputVariable(state,
                                "Chiller Source Steam Rate",
                                Constant::Units::W,
                                this->Report.QGenerator,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Chiller Source Steam Energy",
                                Constant::Units::J,
                                this->Report.GeneratorEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::DistrictHeatingSteam,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Cooling);
        }
    }

    SetupOutputVariable(state,
                        "Chiller COP",
                        Constant::Units::W_W,
                        this->Report.ActualCOP,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Chiller Nominal Capacity", this->Name, "[W]", this->NomCap);
    }
}

void BLASTAbsorberSpecs::oneTimeInit(EnergyPlusData &state)
{

    this->setupOutputVars(state);

    // Locate the chillers on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state,
                                            this->Name,
                                            DataPlant::PlantEquipmentType::Chiller_Absorption,
                                            this->CWPlantLoc,
                                            errFlag,
                                            this->TempLowLimitEvapOut,
                                            _,
                                            _,
                                            this->EvapInletNodeNum,
                                            _);
    if (this->CondInletNodeNum > 0) {
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Chiller_Absorption, this->CDPlantLoc, errFlag, _, _, _, this->CondInletNodeNum, _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->CDPlantLoc, DataPlant::PlantEquipmentType::Chiller_Absorption, true);
    }
    if (this->GeneratorInletNodeNum > 0) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_Absorption,
                                                this->GenPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->GeneratorInletNodeNum,
                                                _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->GenPlantLoc, DataPlant::PlantEquipmentType::Chiller_Absorption, true);
    }

    // Fill in connection data
    if ((this->CondInletNodeNum > 0) && (this->GeneratorInletNodeNum > 0)) {
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CDPlantLoc, this->GenPlantLoc, DataPlant::PlantEquipmentType::Chiller_Absorption, false);
    }
    if (errFlag) {
        ShowFatalError(state, "InitBLASTAbsorberModel: Program terminated due to previous condition(s).");
    }

    if (this->FlowMode == DataPlant::FlowMode::Constant) {
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
    }

    if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;

        if ((state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi == DataLoopNode::SensedNodeFlagValue)) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->ModulatedFlowErrDone) {
                    ShowWarningError(state, format("Missing temperature setpoint for LeavingSetpointModulated mode chiller named {}", this->Name));
                    ShowContinueError(
                        state, "  A temperature setpoint is needed at the outlet node of a chiller in variable flow mode, use a SetpointManager");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                    this->ModulatedFlowErrDone = true;
                }
            } else {
                // need call to EMS to check node
                bool FatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->EvapOutletNodeNum, HVAC::CtrlVarType::Temp, FatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->EvapOutletNodeNum).needsSetpointChecking = false;
                if (FatalError) {
                    if (!this->ModulatedFlowErrDone) {
                        ShowWarningError(state,
                                         format("Missing temperature setpoint for LeavingSetpointModulated mode chiller named {}", this->Name));
                        ShowContinueError(state,
                                          "  A temperature setpoint is needed at the outlet node of a chiller evaporator in variable flow mode");
                        ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the chiller evaporator outlet node ");
                        ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the outlet node ");
                        ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                        this->ModulatedFlowErrDone = true;
                    }
                }
            }

            this->ModulatedFlowSetToLoop = true;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPointHi;
        }
    }
}

void BLASTAbsorberSpecs::initEachEnvironment(EnergyPlusData &state)
{
    constexpr std::string_view RoutineName("BLASTAbsorberSpecs::initEachEnvironment");

    Real64 rho = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

    this->EvapMassFlowRateMax = this->EvapVolFlowRate * rho;

    PlantUtilities::InitComponentNodes(state, 0.0, this->EvapMassFlowRateMax, this->EvapInletNodeNum, this->EvapOutletNodeNum);

    rho = state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

    this->CondMassFlowRateMax = rho * this->CondVolFlowRate;

    PlantUtilities::InitComponentNodes(state, 0.0, this->CondMassFlowRateMax, this->CondInletNodeNum, this->CondOutletNodeNum);
    state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempDesCondIn;

    if (this->GeneratorInletNodeNum > 0) {

        if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
            rho = state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

            this->GenMassFlowRateMax = rho * this->GeneratorVolFlowRate;
        } else if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Steam) {

            this->QGenerator = (this->SteamLoadCoef[0] + this->SteamLoadCoef[1] + this->SteamLoadCoef[2]) * this->NomCap;

            // dry enthalpy of steam (quality = 1)
            Real64 EnthSteamOutDry = this->steam->getSatEnthalpy(
                state, state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp, 1.0, calcChillerAbsorption + this->Name);

            // wet enthalpy of steam (quality = 0)
            Real64 EnthSteamOutWet = this->steam->getSatEnthalpy(
                state, state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp, 0.0, calcChillerAbsorption + this->Name);
            Real64 SteamDeltaT = this->GeneratorSubcool;
            Real64 SteamOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp - SteamDeltaT;
            Real64 HfgSteam = EnthSteamOutDry - EnthSteamOutWet;
            Real64 CpWater = this->water->getDensity(state, SteamOutletTemp, calcChillerAbsorption + this->Name);
            this->GenMassFlowRateMax = this->QGenerator / (HfgSteam + CpWater * SteamDeltaT);
        }

        PlantUtilities::InitComponentNodes(state, 0.0, this->GenMassFlowRateMax, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum);
    }
}

void BLASTAbsorberSpecs::initialize(EnergyPlusData &state,
                                    bool RunFlag, // TRUE when chiller operating
                                    Real64 MyLoad)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   September 2009

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the Electric Chiller components

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // Init more variables
    if (this->MyOneTimeFlag) {
        this->oneTimeInit(state);
        this->MyOneTimeFlag = false;
    }

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        this->initEachEnvironment(state);
        this->MyEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    // every time inits

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) && this->ModulatedFlowSetToLoop) {
        // fix for clumsy old input that worked because loop setpoint was spread.
        //  could be removed with transition, testing , model change, period of being obsolete.
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPointHi;
    }

    Real64 mdotEvap = 0.0; // local fluid mass flow rate thru evaporator
    Real64 mdotCond = 0.0; // local fluid mass flow rate thru condenser
    Real64 mdotGen = 0.0;  // local fluid mass flow rate thru generator

    if ((MyLoad < 0.0) && RunFlag) {
        mdotEvap = this->EvapMassFlowRateMax;
        mdotCond = this->CondMassFlowRateMax;
        mdotGen = this->GenMassFlowRateMax;
    }

    PlantUtilities::SetComponentFlowRate(state, mdotEvap, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);

    PlantUtilities::SetComponentFlowRate(state, mdotCond, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);

    if (this->GeneratorInletNodeNum > 0) {

        PlantUtilities::SetComponentFlowRate(state, mdotGen, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum, this->GenPlantLoc);
    }
}

void BLASTAbsorberSpecs::sizeChiller(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   March 2008
    //       MODIFIED:      R. Raustad May 2008 - added generator node sizing
    //                      November 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Constant COP Chiller Components for which capacities and flow rates
    // have not been specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains evaporator flow rate from the plant sizing array. Calculates nominal capacity from
    // the evaporator flow rate and the chilled water loop design delta T. The condenser flow rate
    // is calculated from the nominal capacity, the COP, and the condenser loop design delta T.

    //        Real64 SteamMassFlowRate; // steam mass flow rate through generator

    constexpr const char *RoutineName("SizeAbsorpChiller");

    int PltSizSteamNum(0);   // Plant Sizing index for steam heating loop
    int PltSizHeatingNum(0); // Plant Sizing index for how water heating loop
    bool ErrorsFound(false); // If errors detected in input
    bool LoopErrorsFound = false;

    // nominal energy input ratio (steam or hot water)
    Real64 SteamInputRatNom = this->SteamLoadCoef[0] + this->SteamLoadCoef[1] + this->SteamLoadCoef[2];
    // init local temporary version in case of partial/mixed autosizing

    // local nominal capacity cooling power
    Real64 tmpNomCap = this->NomCap;

    // local evaporator design volume flow rate
    Real64 tmpEvapVolFlowRate = this->EvapVolFlowRate;

    // local condenser design volume flow rate
    Real64 tmpCondVolFlowRate = this->CondVolFlowRate;

    // local generator design volume flow rate
    Real64 tmpGeneratorVolFlowRate = this->GeneratorVolFlowRate;

    // find the appropriate Plant Sizing object
    int PltSizNum = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).PlantSizNum;
    int PltSizCondNum = state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).PlantSizNum;

    if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Steam) {
        if (this->GeneratorInletNodeNum > 0 && this->GeneratorOutletNodeNum > 0) {
            PltSizSteamNum = PlantUtilities::MyPlantSizingIndex(
                state, moduleObjectType, this->Name, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum, LoopErrorsFound);
        } else {
            for (int PltSizIndex = 1; PltSizIndex <= state.dataSize->NumPltSizInput; ++PltSizIndex) {
                if (state.dataSize->PlantSizData(PltSizIndex).LoopType == DataSizing::TypeOfPlantLoop::Steam) {
                    PltSizSteamNum = PltSizIndex;
                }
            }
        }
    } else {
        if (this->GeneratorInletNodeNum > 0 && this->GeneratorOutletNodeNum > 0) {
            PltSizHeatingNum = PlantUtilities::MyPlantSizingIndex(
                state, moduleObjectType, this->Name, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum, LoopErrorsFound);
        } else {
            for (int PltSizIndex = 1; PltSizIndex <= state.dataSize->NumPltSizInput; ++PltSizIndex) {
                if (state.dataSize->PlantSizData(PltSizIndex).LoopType == DataSizing::TypeOfPlantLoop::Heating) {
                    PltSizHeatingNum = PltSizIndex;
                }
            }
        }
    }

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {

            Real64 Cp = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);

            Real64 rho = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            tmpNomCap =
                Cp * rho * state.dataSize->PlantSizData(PltSizNum).DeltaT * state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * this->SizFac;
            if (!this->NomCapWasAutoSized) {
                tmpNomCap = this->NomCap;
            }
        } else {
            if (this->NomCapWasAutoSized) {
                tmpNomCap = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->NomCapWasAutoSized) {
                this->NomCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, moduleObjectType, this->Name, "Design Size Nominal Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, moduleObjectType, this->Name, "Initial Design Size Nominal Capacity [W]", tmpNomCap);
                }
            } else {
                if (this->NomCap > 0.0 && tmpNomCap > 0.0) {
                    Real64 NomCapUser = this->NomCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     moduleObjectType,
                                                     this->Name,
                                                     "Design Size Nominal Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Nominal Capacity [W]",
                                                     NomCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Nominal Capacity of {:.2R} [W]", NomCapUser));
                                ShowContinueError(state, format("differs from Design Size Nominal Capacity of {:.2R} [W]", tmpNomCap));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpNomCap = NomCapUser;
                }
            }
        }
    } else {
        if (this->NomCapWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Absorption Chiller nominal capacity requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Chiller:Absorption object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->NomCapWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && this->NomCap > 0.0) {
            BaseSizer::reportSizerOutput(state, moduleObjectType, this->Name, "User-Specified Nominal Capacity [W]", this->NomCap);
        }
    }

    // local nominal pump power
    Real64 tmpNomPumpPower = 0.0045 * this->NomCap;

    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
        // the DOE-2 EIR for single stage absorption chiller
        if (this->NomPumpPowerWasAutoSized) {
            this->NomPumpPower = tmpNomPumpPower;
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state, moduleObjectType, this->Name, "Design Size Nominal Pumping Power [W]", tmpNomPumpPower);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state, moduleObjectType, this->Name, "Initial Design Size Nominal Pumping Power [W]", tmpNomPumpPower);
            }
        } else {
            if (this->NomPumpPower > 0.0 && tmpNomPumpPower > 0.0) {
                // Hardsized nominal pump power for reporting
                Real64 NomPumpPowerUser = this->NomPumpPower;

                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 moduleObjectType,
                                                 this->Name,
                                                 "Design Size Nominal Pumping Power [W]",
                                                 tmpNomPumpPower,
                                                 "User-Specified Nominal Pumping Power [W]",
                                                 NomPumpPowerUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(tmpNomPumpPower - NomPumpPowerUser) / NomPumpPowerUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                            ShowContinueError(state, format("User-Specified Nominal Pumping Power of {:.2R} [W]", NomPumpPowerUser));
                            ShowContinueError(state, format("differs from Design Size Nominal Pumping Power of {:.2R} [W]", tmpNomPumpPower));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
                tmpNomPumpPower = NomPumpPowerUser;
            }
        }
    }

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpEvapVolFlowRate = state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * this->SizFac;
            if (!this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = this->EvapVolFlowRate;
            }
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->EvapVolFlowRateWasAutoSized) {
                this->EvapVolFlowRate = tmpEvapVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, moduleObjectType, this->Name, "Design Size Design Chilled Water Flow Rate [m3/s]", tmpEvapVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, moduleObjectType, this->Name, "Initial Design Size Design Chilled Water Flow Rate [m3/s]", tmpEvapVolFlowRate);
                }
            } else {
                if (this->EvapVolFlowRate > 0.0 && tmpEvapVolFlowRate > 0.0) {
                    // Hardsized evaporator volume flow rate for reporting
                    Real64 EvapVolFlowRateUser = this->EvapVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     moduleObjectType,
                                                     this->Name,
                                                     "Design Size Design Chilled Water Flow Rate [m3/s]",
                                                     tmpEvapVolFlowRate,
                                                     "User-Specified Design Chilled Water Flow Rate [m3/s]",
                                                     EvapVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpEvapVolFlowRate - EvapVolFlowRateUser) / EvapVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Design Chilled Water Flow Rate of {:.5R} [m3/s]", EvapVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Design Chilled Water Flow Rate of {:.5R} [m3/s]", tmpEvapVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpEvapVolFlowRate = EvapVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Absorption Chiller evap flow rate requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in CHILLER:ABSORPTION object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && this->EvapVolFlowRate > 0.0) {
            BaseSizer::reportSizerOutput(
                state, moduleObjectType, this->Name, "User-Specified Design Chilled Water Flow Rate [m3/s]", this->EvapVolFlowRate);
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->EvapInletNodeNum, tmpEvapVolFlowRate);

    if (PltSizCondNum > 0 && PltSizNum > 0) {
        if (this->EvapVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {
            //       QCondenser = QEvaporator + QGenerator + PumpingPower

            Real64 Cp = state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).glycol->getSpecificHeat(state, this->TempDesCondIn, RoutineName);

            Real64 rho = state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            tmpCondVolFlowRate =
                tmpNomCap * (1.0 + SteamInputRatNom + tmpNomPumpPower / tmpNomCap) / (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);
            if (!this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = this->CondVolFlowRate;
            }

        } else {
            if (this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->CondVolFlowRateWasAutoSized) {
                this->CondVolFlowRate = tmpCondVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, moduleObjectType, this->Name, "Design Size Design Condenser Water Flow Rate [m3/s]", tmpCondVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, moduleObjectType, this->Name, "Initial Design Size Design Condenser Water Flow Rate [m3/s]", tmpCondVolFlowRate);
                }
            } else {
                if (this->CondVolFlowRate > 0.0 && tmpCondVolFlowRate > 0.0) {
                    // Hardsized condenser flow rate for reporting
                    Real64 CondVolFlowRateUser = this->CondVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     moduleObjectType,
                                                     this->Name,
                                                     "Design Size Design Condenser Water Flow Rate [m3/s]",
                                                     tmpCondVolFlowRate,
                                                     "User-Specified Design Condenser Water Flow Rate [m3/s]",
                                                     CondVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpCondVolFlowRate - CondVolFlowRateUser) / CondVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Design Condenser Water Flow Rate of {:.5R} [m3/s]", CondVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Design Condenser Water Flow Rate of {:.5R} [m3/s]", tmpCondVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpCondVolFlowRate = CondVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Absorption Chiller condenser flow rate requires a condenser");
            ShowContinueError(state, "loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in CHILLER:ABSORPTION object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize && (this->CondVolFlowRate > 0.0)) {
            BaseSizer::reportSizerOutput(
                state, moduleObjectType, this->Name, "User-Specified Design Condenser Water Flow Rate [m3/s]", this->CondVolFlowRate);
        }
    }

    // save the design condenser water volumetric flow rate for use by the condenser water loop sizing algorithms
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondInletNodeNum, tmpCondVolFlowRate);

    if ((PltSizSteamNum > 0 && this->GenHeatSourceType == DataLoopNode::NodeFluidType::Steam) ||
        (PltSizHeatingNum > 0 && this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water)) {
        if (this->EvapVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {
            if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
                Real64 CpWater = state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum)
                                     .glycol->getSpecificHeat(state, state.dataSize->PlantSizData(PltSizHeatingNum).ExitTemp, RoutineName);
                Real64 SteamDeltaT = max(0.5, state.dataSize->PlantSizData(PltSizHeatingNum).DeltaT);
                Real64 RhoWater =
                    state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum)
                        .glycol->getDensity(state, (state.dataSize->PlantSizData(PltSizHeatingNum).ExitTemp - SteamDeltaT), RoutineName);
                tmpGeneratorVolFlowRate = (this->NomCap * SteamInputRatNom) / (CpWater * SteamDeltaT * RhoWater);
                if (!this->GeneratorVolFlowRateWasAutoSized) {
                    tmpGeneratorVolFlowRate = this->GeneratorVolFlowRate;
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    if (this->GeneratorVolFlowRateWasAutoSized) {
                        this->GeneratorVolFlowRate = tmpGeneratorVolFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(
                                state, moduleObjectType, this->Name, "Design Size Design Generator Fluid Flow Rate [m3/s]", tmpGeneratorVolFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         moduleObjectType,
                                                         this->Name,
                                                         "Initial Design Size Design Generator Fluid Flow Rate [m3/s]",
                                                         tmpGeneratorVolFlowRate);
                        }
                    } else {
                        if (this->GeneratorVolFlowRate > 0.0 && tmpGeneratorVolFlowRate > 0.0) {
                            // Hardsized generator flow rate for reporting
                            Real64 GeneratorVolFlowRateUser = this->GeneratorVolFlowRate;
                            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             moduleObjectType,
                                                             this->Name,
                                                             "Design Size Design Generator Fluid Flow Rate [m3/s]",
                                                             tmpGeneratorVolFlowRate,
                                                             "User-Specified Design Generator Fluid Flow Rate [m3/s]",
                                                             GeneratorVolFlowRateUser);
                                if (state.dataGlobal->DisplayExtraWarnings) {
                                    if ((std::abs(tmpGeneratorVolFlowRate - GeneratorVolFlowRateUser) / GeneratorVolFlowRateUser) >
                                        state.dataSize->AutoVsHardSizingThreshold) {
                                        ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                                        ShowContinueError(
                                            state,
                                            format("User-Specified Design Generator Fluid Flow Rate of {:.5R} [m3/s]", GeneratorVolFlowRateUser));
                                        ShowContinueError(state,
                                                          format("differs from Design Size Design Generator Fluid Flow Rate of {:.5R} [m3/s]",
                                                                 tmpGeneratorVolFlowRate));
                                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                        ShowContinueError(state,
                                                          "Verify that the value entered is intended and is consistent with other components.");
                                    }
                                }
                            }
                            tmpGeneratorVolFlowRate = GeneratorVolFlowRateUser;
                        }
                    }
                }
            } else {
                constexpr std::string_view RoutineNameLong("SizeAbsorptionChiller");
                Real64 SteamDensity = this->steam->getSatDensity(state, state.dataSize->PlantSizData(PltSizSteamNum).ExitTemp, 1.0, RoutineNameLong);
                Real64 SteamDeltaT = state.dataSize->PlantSizData(PltSizSteamNum).DeltaT;
                Real64 GeneratorOutletTemp = state.dataSize->PlantSizData(PltSizSteamNum).ExitTemp - SteamDeltaT;

                Real64 EnthSteamOutDry =
                    this->steam->getSatEnthalpy(state, state.dataSize->PlantSizData(PltSizSteamNum).ExitTemp, 1.0, moduleObjectType + this->Name);
                Real64 EnthSteamOutWet =
                    this->steam->getSatEnthalpy(state, state.dataSize->PlantSizData(PltSizSteamNum).ExitTemp, 0.0, moduleObjectType + this->Name);
                Real64 CpWater = this->water->getSpecificHeat(state, GeneratorOutletTemp, RoutineName);
                Real64 HfgSteam = EnthSteamOutDry - EnthSteamOutWet;
                this->SteamMassFlowRate = (this->NomCap * SteamInputRatNom) / ((HfgSteam) + (SteamDeltaT * CpWater));
                tmpGeneratorVolFlowRate = this->SteamMassFlowRate / SteamDensity;

                if (!this->GeneratorVolFlowRateWasAutoSized) {
                    tmpGeneratorVolFlowRate = this->GeneratorVolFlowRate;
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {

                    if (this->GeneratorVolFlowRateWasAutoSized) {
                        this->GeneratorVolFlowRate = tmpGeneratorVolFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(
                                state, moduleObjectType, this->Name, "Design Size Design Generator Fluid Flow Rate [m3/s]", tmpGeneratorVolFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         moduleObjectType,
                                                         this->Name,
                                                         "Initial Design Size Design Generator Fluid Flow Rate [m3/s]",
                                                         tmpGeneratorVolFlowRate);
                        }
                    } else {
                        if (this->GeneratorVolFlowRate > 0.0 && tmpGeneratorVolFlowRate > 0.0) {
                            // Hardsized generator flow rate for reporting
                            Real64 GeneratorVolFlowRateUser = this->GeneratorVolFlowRate;
                            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             moduleObjectType,
                                                             this->Name,
                                                             "Design Size Design Generator Fluid Flow Rate [m3/s]",
                                                             tmpGeneratorVolFlowRate,
                                                             "User-Specified Design Generator Fluid Flow Rate [m3/s]",
                                                             GeneratorVolFlowRateUser);
                                if (state.dataGlobal->DisplayExtraWarnings) {
                                    if ((std::abs(tmpGeneratorVolFlowRate - GeneratorVolFlowRateUser) / GeneratorVolFlowRateUser) >
                                        state.dataSize->AutoVsHardSizingThreshold) {
                                        ShowMessage(state, format("SizeChillerAbsorption: Potential issue with equipment sizing for {}", this->Name));
                                        ShowContinueError(
                                            state,
                                            format("User-Specified Design Generator Fluid Flow Rate of {:.5R} [m3/s]", GeneratorVolFlowRateUser));
                                        ShowContinueError(state,
                                                          format("differs from Design Size Design Generator Fluid Flow Rate of {:.5R} [m3/s]",
                                                                 tmpGeneratorVolFlowRate));
                                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                        ShowContinueError(state,
                                                          "Verify that the value entered is intended and is consistent with other components.");
                                    }
                                }
                            }
                            tmpGeneratorVolFlowRate = GeneratorVolFlowRateUser;
                        }
                    }
                }
            }
        } else {
            if (this->GeneratorVolFlowRateWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->GeneratorVolFlowRate = 0.0;
                } else {
                    tmpGeneratorVolFlowRate = 0.0;
                }
            }
        }
    } else {
        if (this->GeneratorVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Absorption Chiller generator flow rate requires a loop Sizing:Plant object.");
            ShowContinueError(state, " For steam loops, use a steam Sizing:Plant object.");
            ShowContinueError(state, " For hot water loops, use a heating Sizing:Plant object.");
            ShowContinueError(state, format("Occurs in Chiller:Absorption object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->GeneratorVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->GeneratorVolFlowRate > 0.0)) {
            BaseSizer::reportSizerOutput(
                state, moduleObjectType, this->Name, "User-Specified Design Generator Fluid Flow Rate [m3/s]", this->GeneratorVolFlowRate);
        }
    }

    // save the design steam or hot water volumetric flow rate for use by the steam or hot water loop sizing algorithms
    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->GeneratorInletNodeNum, this->GeneratorVolFlowRate);
    } else {
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->GeneratorInletNodeNum, tmpGeneratorVolFlowRate);
    }

    if (this->GeneratorDeltaTempWasAutoSized) {
        if (PltSizHeatingNum > 0 && this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
            this->GeneratorDeltaTemp = max(0.5, state.dataSize->PlantSizData(PltSizHeatingNum).DeltaT);
        } else if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                Real64 Cp =
                    state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum).glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                Real64 rho = state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                this->GeneratorDeltaTemp = (SteamInputRatNom * this->NomCap) / (Cp * rho * this->GeneratorVolFlowRate);
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        // create predefined report
        std::string equipName = this->Name;
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, equipName, moduleObjectType);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, equipName, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, equipName, this->NomCap);

        // std 229 new Chiller table
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerType, this->Name, moduleObjectType);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefCap, this->Name, this->NomCap);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEff, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedCap, this->Name, this->NomCap); // did not find rated cap, using Nominal
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRatedEff, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinSI, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinIP, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerPlantloopName,
                                                 this->Name,
                                                 this->CWPlantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchChillerPlantloopBranchName,
            this->Name,
            this->CWPlantLoc.loopNum > 0
                ? state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).LoopSide(this->CWPlantLoc.loopSideNum).Branch(this->CWPlantLoc.branchNum).Name
                : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerCondLoopName,
                                                 this->Name,
                                                 this->CDPlantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchChillerCondLoopBranchName,
            this->Name,
            this->CDPlantLoc.loopNum > 0
                ? state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).LoopSide(this->CDPlantLoc.loopSideNum).Branch(this->CDPlantLoc.branchNum).Name
                : "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerMinPLR, this->Name, this->MinPartLoadRat);
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerFuelType,
                                                 this->Name,
                                                 DataLoopNode::NodeFluidTypeNames[static_cast<int>(this->GenHeatSourceType)]);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedEntCondTemp, this->Name, this->TempDesCondIn); // Rated==Ref?
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedLevEvapTemp, this->Name, this->TempLowLimitEvapOut); // Rated==Ref?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEntCondTemp, this->Name, this->TempDesCondIn);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefLevEvapTemp, this->Name, this->TempLowLimitEvapOut);

        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerDesSizeRefCHWFlowRate,
                                                 this->Name,
                                                 this->EvapMassFlowRateMax); // flowrate Max==DesignSizeRef flowrate?
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerDesSizeRefCondFluidFlowRate,
                                                 this->Name,
                                                 this->CondMassFlowRateMax); // Cond flowrate Max==DesignSizeRef Cond flowrate?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerHeatRecPlantloopName, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerHeatRecPlantloopBranchName, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRecRelCapFrac, this->Name, "N/A");
    }
}

void BLASTAbsorberSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Sept. 1998
    //       MODIFIED       Apr. 1999, May 2000- Taecheol Kim
    //                      May. 2008, R. Raustad, Added generator nodes
    //                      Jun. 2016, Rongpeng Zhang, Applied the chiller supply water temperature sensor fault model

    // PURPOSE OF THIS SUBROUTINE:
    // simulate a vapor compression Absorber using the BLAST model

    // METHODOLOGY EMPLOYED:
    // curve fit of performance data:

    // REFERENCES:
    // 1.  BLAST User Manual
    // 2.  Absorber User Manual

    constexpr const char *RoutineName("CalcBLASTAbsorberModel");

    Real64 EvapDeltaTemp(0.0); // C - evaporator temperature difference, water side

    // If no loop demand or Absorber OFF, return
    if (MyLoad >= 0.0 || !RunFlag) { // off or heating
        if (this->EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
            this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        }
        return;
    }

    // Set the condenser mass flow rates
    this->CondMassFlowRate = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;

    Real64 TempEvapOut = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;

    Real64 CpFluid = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum)
                         .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, RoutineName);

    // If there is a fault of Chiller SWT Sensor
    if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation)) {
        int FaultIndex = this->FaultyChillerSWTIndex;
        Real64 EvapOutletTemp_ff = TempEvapOut;

        // calculate the sensor offset using fault information
        this->FaultyChillerSWTOffset = state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex).CalFaultOffsetAct(state);
        // update the TempEvapOut
        TempEvapOut = max(this->TempLowLimitEvapOut,
                          min(state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, EvapOutletTemp_ff - this->FaultyChillerSWTOffset));
        this->FaultyChillerSWTOffset = EvapOutletTemp_ff - TempEvapOut;
    }

    // If FlowLock is True, the new resolved mdot is used to update Power, QEvap, Qcond, and
    // condenser side outlet temperature.
    if (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).LoopSide(this->CWPlantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
        this->PossibleSubcooling = false;
        this->QEvaporator = std::abs(MyLoad);
        // limit by max capacity
        this->QEvaporator = min(this->QEvaporator, (this->MaxPartLoadRat * this->NomCap));

        // Either set the flow to the Constant value or calculate the flow for the variable volume
        if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
            this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;

            if (this->EvapMassFlowRate != 0.0) {
                EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / CpFluid;
            } else {
                EvapDeltaTemp = 0.0;
            }
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;

        } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
            // Calculate the Delta Temp from the inlet temp to the chiller outlet setpoint
            switch (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                EvapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                EvapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
            } break;
            default: {
                assert(false);
            } break;
            }
            if (EvapDeltaTemp != 0) {

                this->EvapMassFlowRate = std::abs(this->QEvaporator / CpFluid / EvapDeltaTemp);
                if ((this->EvapMassFlowRate - this->EvapMassFlowRateMax) > DataBranchAirLoopPlant::MassFlowTolerance) {
                    this->PossibleSubcooling = true;
                }
                // Check to see if the Maximum is exceeded, if so set to maximum
                this->EvapMassFlowRate = min(this->EvapMassFlowRateMax, this->EvapMassFlowRate);
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                switch (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
                } break;
                default:
                    break;
                }
            } else {
                this->EvapMassFlowRate = 0.0;

                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;

                ShowRecurringWarningErrorAtEnd(state,
                                               "CalcBLASTAbsorberModel: Name=\"" + this->Name +
                                                   "\" Evaporative Condenser Delta Temperature = 0 in mass flow calculation.",
                                               this->ErrCount2);
            }
        } // End of Constant Variable Flow If Block

        // If there is a fault of Chiller SWT Sensor
        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation) && (this->EvapMassFlowRate > 0)) {
            // calculate directly affected variables at faulty case: EvapOutletTemp, EvapMassFlowRate, QEvaporator
            int FaultIndex = this->FaultyChillerSWTIndex;
            bool VarFlowFlag = (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated);
            state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex)
                .CalFaultChillerSWT(VarFlowFlag,
                                    this->FaultyChillerSWTOffset,
                                    CpFluid,
                                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                    this->EvapOutletTemp,
                                    this->EvapMassFlowRate,
                                    this->QEvaporator);
            // update corresponding variables at faulty case
            // PartLoadRat = ( AvailChillerCap > 0.0 ) ? ( QEvaporator / AvailChillerCap ) : 0.0;
            // PartLoadRat = max( 0.0, min( PartLoadRat, MaxPartLoadRat ));
            // ChillerPartLoadRatio = PartLoadRat;
        }

    } else { // If FlowLock is True

        this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        if (this->PossibleSubcooling) {
            this->QEvaporator = std::abs(MyLoad);
            EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / CpFluid;
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
        } else {
            Real64 TempEvapOutSetPoint{0}; // C - evaporator outlet temperature setpoint
            switch (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
                    (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
                    (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
                    TempEvapOutSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
                } else {
                    TempEvapOutSetPoint =
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
                }
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
                    (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
                    (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi != DataLoopNode::SensedNodeFlagValue)) {
                    TempEvapOutSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
                } else {
                    TempEvapOutSetPoint =
                        state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPointHi;
                }
            } break;
            default: {
                assert(false);
            } break;
            }
            EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - TempEvapOutSetPoint;
            this->QEvaporator = std::abs(this->EvapMassFlowRate * CpFluid * EvapDeltaTemp);
            this->EvapOutletTemp = TempEvapOutSetPoint;
        }
        // Check that the Evap outlet temp honors both plant loop temp low limit and also the chiller low limit
        if (this->EvapOutletTemp < this->TempLowLimitEvapOut) {
            if ((state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->TempLowLimitEvapOut) > DataPlant::DeltaTempTol) {
                this->EvapOutletTemp = this->TempLowLimitEvapOut;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * CpFluid * EvapDeltaTemp;
            } else {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * CpFluid * EvapDeltaTemp;
            }
        }
        if (this->EvapOutletTemp < state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin) {
            if ((state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin) >
                DataPlant::DeltaTempTol) {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * CpFluid * EvapDeltaTemp;
            } else {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * CpFluid * EvapDeltaTemp;
            }
        }

        // Checks QEvaporator on the basis of the machine limits.
        if (this->QEvaporator > std::abs(MyLoad)) {
            if (this->EvapMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                this->QEvaporator = std::abs(MyLoad);
                EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / CpFluid;
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
            } else {
                this->QEvaporator = 0.0;
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
            }
        }

        // If there is a fault of Chiller SWT Sensor
        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation) && (this->EvapMassFlowRate > 0)) {
            // calculate directly affected variables at faulty case: EvapOutletTemp, EvapMassFlowRate, QEvaporator
            int FaultIndex = this->FaultyChillerSWTIndex;
            bool VarFlowFlag = false;
            state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex)
                .CalFaultChillerSWT(VarFlowFlag,
                                    this->FaultyChillerSWTOffset,
                                    CpFluid,
                                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                    this->EvapOutletTemp,
                                    this->EvapMassFlowRate,
                                    this->QEvaporator);
            // update corresponding variables at faulty case
        }

    } // This is the end of the FlowLock Block

    // Calculate part load ratio for efficiency calcs. If this part load ratio is greater than
    // Min PLR it will be used for calculations too.
    Real64 PartLoadRat = max(this->MinPartLoadRat, min(this->QEvaporator / this->NomCap, this->MaxPartLoadRat));

    // In case MyLoad is less than the Min PLR load, the power and steam input should be adjusted
    // for cycling. The ratios used however are based on MinPLR.
    Real64 OperPartLoadRat = this->QEvaporator / this->NomCap;

    Real64 FRAC = 1.0;
    if (OperPartLoadRat < PartLoadRat) {
        FRAC = min(1.0, OperPartLoadRat / this->MinPartLoadRat);
    }

    // Calculate steam input ratio
    Real64 SteamInputRat = this->SteamLoadCoef[0] / PartLoadRat + this->SteamLoadCoef[1] + this->SteamLoadCoef[2] * PartLoadRat;

    // Calculate electric input ratio
    Real64 ElectricInputRat = this->PumpPowerCoef[0] + this->PumpPowerCoef[1] * PartLoadRat + this->PumpPowerCoef[2] * pow_2(PartLoadRat);

    // Calculate electric energy input
    this->PumpingPower = ElectricInputRat * this->NomPumpPower * FRAC;

    // Calculate steam load
    this->QGenerator = SteamInputRat * this->QEvaporator * FRAC;

    if (this->EvapMassFlowRate == 0.0) {
        this->QGenerator = 0.0;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->PumpingPower = 0.0;
    }

    this->QCondenser = this->QEvaporator + this->QGenerator + this->PumpingPower;

    CpFluid = state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum)
                  .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->CondInletNodeNum).Temp, RoutineName);

    if (this->CondMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
        this->CondOutletTemp = this->QCondenser / this->CondMassFlowRate / CpFluid + state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
    } else {

        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondMassFlowRate = 0.0;
        this->QCondenser = 0.0;
        MyLoad = 0.0;
        this->EvapMassFlowRate = 0.0;
        PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
        return;
        // V7 plant upgrade, no longer fatal here anymore, set some things and return
    }

    if (this->GeneratorInletNodeNum > 0) {
        if (this->GenHeatSourceType == DataLoopNode::NodeFluidType::Water) {
            Real64 GenMassFlowRate = 0.0;
            //  Hot water plant is used for the generator
            CpFluid = state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum)
                          .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp, RoutineName);
            if (state.dataPlnt->PlantLoop(this->GenPlantLoc.loopNum).LoopSide(this->GenPlantLoc.loopSideNum).FlowLock ==
                DataPlant::FlowLock::Unlocked) {
                if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
                    GenMassFlowRate = this->GenMassFlowRateMax;
                } else { // LeavingSetpointModulated
                    // since the .FlowMode applies to the chiller evaporator, the generator mass flow rate will be proportional to the evaporator
                    // mass flow rate
                    Real64 GenFlowRatio = this->EvapMassFlowRate / this->EvapMassFlowRateMax;
                    GenMassFlowRate = min(this->GenMassFlowRateMax, GenFlowRatio * this->GenMassFlowRateMax);
                }
            } else { // If FlowLock is True
                GenMassFlowRate = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).MassFlowRate;
            }
            PlantUtilities::SetComponentFlowRate(
                state, GenMassFlowRate, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum, this->GenPlantLoc);

            if (GenMassFlowRate <= 0.0) {
                this->GenOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp;
                this->SteamOutletEnthalpy = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Enthalpy;
            } else {
                this->GenOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp - this->QGenerator / (CpFluid * GenMassFlowRate);
                this->SteamOutletEnthalpy = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Enthalpy - this->QGenerator / GenMassFlowRate;
            }
            state.dataLoopNodes->Node(this->GeneratorOutletNodeNum).Temp = this->GenOutletTemp;
            state.dataLoopNodes->Node(this->GeneratorOutletNodeNum).Enthalpy = this->SteamOutletEnthalpy;
            state.dataLoopNodes->Node(this->GeneratorOutletNodeNum).MassFlowRate = GenMassFlowRate;

        } else { // using a steam plant for the generator

            // enthalpy of dry steam at generator inlet
            Real64 EnthSteamOutDry = this->steam->getSatEnthalpy(
                state, state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp, 1.0, calcChillerAbsorption + this->Name);
            // enthalpy of wet steam at generator inlet
            Real64 EnthSteamOutWet = this->steam->getSatEnthalpy(
                state, state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp, 0.0, calcChillerAbsorption + this->Name);
            Real64 SteamDeltaT = this->GeneratorSubcool;
            Real64 SteamOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp - SteamDeltaT;
            Real64 HfgSteam = EnthSteamOutDry - EnthSteamOutWet;
            CpFluid = this->water->getSpecificHeat(state, SteamOutletTemp, calcChillerAbsorption + this->Name);
            this->SteamMassFlowRate = this->QGenerator / (HfgSteam + CpFluid * SteamDeltaT);
            PlantUtilities::SetComponentFlowRate(
                state, this->SteamMassFlowRate, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum, this->GenPlantLoc);

            if (this->SteamMassFlowRate <= 0.0) {
                this->GenOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp;
                this->SteamOutletEnthalpy = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Enthalpy;
            } else {
                this->GenOutletTemp = state.dataLoopNodes->Node(this->GeneratorInletNodeNum).Temp - SteamDeltaT;
                this->SteamOutletEnthalpy = this->steam->getSatEnthalpy(state, this->GenOutletTemp, 0.0, moduleObjectType + this->Name);
                this->SteamOutletEnthalpy -= CpFluid * SteamDeltaT;
            }
        }
    } // IF(GeneratorInletNode .GT. 0)THEN

    // convert power to energy
    this->GeneratorEnergy = this->QGenerator * state.dataHVACGlobal->TimeStepSysSec;
    this->EvaporatorEnergy = this->QEvaporator * state.dataHVACGlobal->TimeStepSysSec;
    this->CondenserEnergy = this->QCondenser * state.dataHVACGlobal->TimeStepSysSec;
    this->PumpingEnergy = this->PumpingPower * state.dataHVACGlobal->TimeStepSysSec;
}

void BLASTAbsorberSpecs::updateRecords(EnergyPlusData &state, Real64 MyLoad, bool RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    October 1998

    // PURPOSE OF THIS SUBROUTINE:
    // reporting

    if (MyLoad >= 0 || !RunFlag) {
        // set node conditions
        PlantUtilities::SafeCopyPlantNode(state, this->EvapInletNodeNum, this->EvapOutletNodeNum);
        PlantUtilities::SafeCopyPlantNode(state, this->CondInletNodeNum, this->CondOutletNodeNum);

        this->Report.PumpingPower = 0.0;
        this->Report.QEvap = 0.0;
        this->Report.QCond = 0.0;
        this->Report.QGenerator = 0.0;
        this->Report.PumpingEnergy = 0.0;
        this->Report.EvapEnergy = 0.0;
        this->Report.CondEnergy = 0.0;
        this->Report.GeneratorEnergy = 0.0;
        this->Report.EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->Report.CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->Report.CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->Report.EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->Report.Evapmdot = 0.0;
        this->Report.Condmdot = 0.0;
        this->Report.Genmdot = 0.0;
        this->Report.ActualCOP = 0.0;

        if (this->GeneratorInletNodeNum > 0) {
            PlantUtilities::SafeCopyPlantNode(state, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum);
        }

    } else {
        // set node conditions
        PlantUtilities::SafeCopyPlantNode(state, this->EvapInletNodeNum, this->EvapOutletNodeNum);
        PlantUtilities::SafeCopyPlantNode(state, this->CondInletNodeNum, this->CondOutletNodeNum);
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = this->EvapOutletTemp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = this->CondOutletTemp;

        this->Report.PumpingPower = this->PumpingPower;
        this->Report.QEvap = this->QEvaporator;
        this->Report.QCond = this->QCondenser;
        this->Report.QGenerator = this->QGenerator;
        this->Report.PumpingEnergy = this->PumpingEnergy;
        this->Report.EvapEnergy = this->EvaporatorEnergy;
        this->Report.CondEnergy = this->CondenserEnergy;
        this->Report.GeneratorEnergy = this->GeneratorEnergy;
        this->Report.EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->Report.CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->Report.CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->Report.EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->Report.Evapmdot = this->EvapMassFlowRate;
        this->Report.Condmdot = this->CondMassFlowRate;
        this->Report.Genmdot = this->SteamMassFlowRate;
        if (this->QGenerator != 0.0) {
            this->Report.ActualCOP = this->QEvaporator / this->QGenerator;
        } else {
            this->Report.ActualCOP = 0.0;
        }

        if (this->GeneratorInletNodeNum > 0) {
            PlantUtilities::SafeCopyPlantNode(state, this->GeneratorInletNodeNum, this->GeneratorOutletNodeNum);
            state.dataLoopNodes->Node(this->GeneratorOutletNodeNum).Temp = this->GenOutletTemp;
        }
    }
}

} // namespace EnergyPlus::ChillerAbsorption
