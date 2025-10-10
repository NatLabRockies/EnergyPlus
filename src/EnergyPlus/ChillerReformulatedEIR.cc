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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerReformulatedEIR.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::ChillerReformulatedEIR {

// The Electric EIR and Reformulated EIR chiller models are similar.
// They only differ in the independent variable used to evaluate the performance curves.
// Since the Reformulated EIR chiller uses outlet condenser water temperature as an
// independent variable, iteration is required to converge on a solution.

// MODULE INFORMATION:
//       AUTHOR         Lixing Gu
//       DATE WRITTEN   August 2006

//       MODIFIED
//       Aug.  2014, Rongpeng Zhang, added An additional part-load performance curve type

// PURPOSE OF THIS MODULE:
//  This module simulates the performance of the electric vapor compression
//  chiller using a reformulated model based on the DOE-2 EIR chiller.

// METHODOLOGY EMPLOYED:
//  Once the PlantLoopManager determines that the Reformulated EIR chiller
//  is available to meet a loop cooling demand, it calls SimReformulatedEIRChiller
//  which in turn calls the reformulated EIR chiller model.
//  The ReformulatedEIR chiller model is based on polynomial fits of chiller
//  performance data.

// REFERENCES:
// 1. Hydeman, M., P. Sreedharan, N. Webb, and S. Blanc. 2002. "Development and Testing of a Reformulated
//    Regression-Based Electric Chiller Model". ASHRAE Transactions, HI-02-18-2, Vol 108, Part 2, pp. 1118-1127.

ReformulatedEIRChillerSpecs *ReformulatedEIRChillerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data if it hasn't been done already
    if (state.dataChillerReformulatedEIR->GetInputREIR) {
        GetElecReformEIRChillerInput(state);
        state.dataChillerReformulatedEIR->GetInputREIR = false;
    }
    // Now look for this particular object in the list
    auto thisObj = std::find_if(state.dataChillerReformulatedEIR->ElecReformEIRChiller.begin(),
                                state.dataChillerReformulatedEIR->ElecReformEIRChiller.end(),
                                [&objectName](const ReformulatedEIRChillerSpecs &myObj) { return myObj.Name == objectName; });
    if (thisObj != state.dataChillerReformulatedEIR->ElecReformEIRChiller.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalReformulatedElectEIRChillerFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void ReformulatedEIRChillerSpecs::getDesignCapacities(
    [[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        MinLoad = this->RefCap * this->MinPartLoadRat;
        MaxLoad = this->RefCap * this->MaxPartLoadRat;
        OptLoad = this->RefCap * this->OptPartLoadRat;
    } else {
        MinLoad = 0.0;
        MaxLoad = 0.0;
        OptLoad = 0.0;
    }
}

void ReformulatedEIRChillerSpecs::getDesignTemperatures(Real64 &TempDesCondIn, Real64 &TempDesEvapOut)
{
    TempDesEvapOut = this->TempRefEvapOut;
    TempDesCondIn = this->TempRefCondIn;
}

void ReformulatedEIRChillerSpecs::getSizingFactor(Real64 &sizFac)
{
    sizFac = this->SizFac;
}

void ReformulatedEIRChillerSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
{
    bool runFlag = true;
    Real64 myLoad = 0.0;
    this->initialize(state, runFlag, myLoad);

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->size(state);
    }
}

void ReformulatedEIRChillerSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  This is the reformulated EIR chiller model driver. It gets the input for the
    //  models, initializes simulation variables, calls the appropriate model and sets
    //  up reporting variables.

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->initialize(state, RunFlag, CurLoad);
        this->control(state, CurLoad, RunFlag, FirstHVACIteration);
        this->update(state, CurLoad, RunFlag);
    } else if (calledFromLocation.loopNum == this->CDPlantLoc.loopNum) {
        DataPlant::LoopSideLocation LoopSide = this->CDPlantLoc.loopSideNum;
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            LoopSide,
                                                            DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                                            this->CondInletNodeNum,
                                                            this->CondOutletNodeNum,
                                                            this->QCondenser,
                                                            this->CondInletTemp,
                                                            this->CondOutletTemp,
                                                            this->CondMassFlowRate,
                                                            FirstHVACIteration);
    } else if (calledFromLocation.loopNum == this->HRPlantLoc.loopNum) {
        PlantUtilities::UpdateComponentHeatRecoverySide(state,
                                                        this->HRPlantLoc.loopNum,
                                                        this->HRPlantLoc.loopSideNum,
                                                        DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                                        this->HeatRecInletNodeNum,
                                                        this->HeatRecOutletNodeNum,
                                                        this->QHeatRecovery,
                                                        this->HeatRecInletTemp,
                                                        this->HeatRecOutletTemp,
                                                        this->HeatRecMassFlow,
                                                        FirstHVACIteration);
    }
}

void GetElecReformEIRChillerInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Lixing Gu, FSEC
    //       DATE WRITTEN:    July 2006
    //       MODIFIED
    //       Aug.  2014, Rongpeng Zhang, added an additional part-load performance curve type

    // PURPOSE OF THIS SUBROUTINE:
    //  This routine will get the input required by the Reformulated Electric EIR Chiller model

    static constexpr std::string_view RoutineName("GetElecReformEIRChillerInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetElecReformEIRChillerInput";

    bool ErrorsFound(false); // True when input errors found

    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "Chiller:Electric:ReformulatedEIR";
    int NumElecReformEIRChillers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (NumElecReformEIRChillers <= 0) {
        ShowSevereError(state, format("No {} equipment specified in input file", s_ipsc->cCurrentModuleObject));
        ErrorsFound = true;
    }

    // ALLOCATE ARRAYS
    state.dataChillerReformulatedEIR->ElecReformEIRChiller.allocate(NumElecReformEIRChillers);

    // Load arrays with reformulated electric EIR chiller data
    for (int EIRChillerNum = 1; EIRChillerNum <= NumElecReformEIRChillers; ++EIRChillerNum) {
        int NumAlphas = 0; // Number of elements in the alpha array
        int NumNums = 0;   // Number of elements in the numeric array
        int IOStat = 0;    // IO Status when calling get input subroutine
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 EIRChillerNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueChillerName(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject + " Name");

        auto &thisChiller = state.dataChillerReformulatedEIR->ElecReformEIRChiller(EIRChillerNum);
        thisChiller.Name = s_ipsc->cAlphaArgs(1);
        // Performance curves
        thisChiller.ChillerCapFTIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(2));
        thisChiller.CAPFTName = s_ipsc->cAlphaArgs(2);
        if (thisChiller.ChillerCapFTIndex == 0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ErrorsFound = true;
        }

        thisChiller.ChillerEIRFTIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(3));
        thisChiller.EIRFTName = s_ipsc->cAlphaArgs(3);
        if (thisChiller.ChillerEIRFTIndex == 0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            ErrorsFound = true;
        }

        // The default type of part-load curve is: LeavingCondenserWaterTemperature
        std::string PartLoadCurveType; // Part load curve type
        if (s_ipsc->lAlphaFieldBlanks(4)) {
            PartLoadCurveType = "LeavingCondenserWaterTemperature";
        } else {
            PartLoadCurveType = s_ipsc->cAlphaArgs(4);
        }

        thisChiller.EIRFPLRName = s_ipsc->cAlphaArgs(5);
        thisChiller.ChillerEIRFPLRIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(5));
        if (thisChiller.ChillerEIRFPLRIndex == 0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
            ErrorsFound = true;
        }

        // Check the type of part-load curves implemented: 1_LeavingCondenserWaterTemperature, 2_Lift
        if (Util::SameString(PartLoadCurveType, "LeavingCondenserWaterTemperature") &&
            state.dataCurveManager->curves(thisChiller.ChillerEIRFPLRIndex)->numDims == 2) {
            thisChiller.PartLoadCurveType = PLR::LeavingCondenserWaterTemperature;
        } else if (Util::SameString(PartLoadCurveType, "Lift") && state.dataCurveManager->curves(thisChiller.ChillerEIRFPLRIndex)->numDims == 3) {
            thisChiller.PartLoadCurveType = PLR::Lift;
        } else {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state,
                              format("Invalid {}={} for {}={}",
                                     s_ipsc->cAlphaFieldNames(5),
                                     s_ipsc->cAlphaArgs(5),
                                     s_ipsc->cAlphaFieldNames(4),
                                     s_ipsc->cAlphaArgs(4)));
            ErrorsFound = true;
        }

        // Chilled water inlet/outlet node names are necessary
        if (s_ipsc->lAlphaFieldBlanks(6)) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} is blank.", s_ipsc->cAlphaFieldNames(6)));
            ErrorsFound = true;
        }
        if (s_ipsc->lAlphaFieldBlanks(7)) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} is blank.", s_ipsc->cAlphaFieldNames(7)));
            ErrorsFound = true;
        }

        thisChiller.EvapInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           s_ipsc->cAlphaArgs(6),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                           s_ipsc->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.EvapOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(7),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(6), s_ipsc->cAlphaArgs(7), "Chilled Water Nodes");

        thisChiller.CondenserType = DataPlant::CondenserType::WaterCooled;

        // Condenser inlet/outlet node names are necessary
        if (s_ipsc->lAlphaFieldBlanks(8)) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} is blank.", s_ipsc->cAlphaFieldNames(8)));
            ErrorsFound = true;
        }
        if (s_ipsc->lAlphaFieldBlanks(9)) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} is blank.", s_ipsc->cAlphaFieldNames(9)));
            ErrorsFound = true;
        }

        thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           s_ipsc->cAlphaArgs(8),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                           s_ipsc->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Secondary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(9),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Secondary,
                                                                            DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(8), s_ipsc->cAlphaArgs(9), "Condenser Water Nodes");

        {
            thisChiller.FlowMode = static_cast<DataPlant::FlowMode>(getEnumValue(DataPlant::FlowModeNamesUC, s_ipsc->cAlphaArgs(10)));
            if (thisChiller.FlowMode == DataPlant::FlowMode::Invalid) {
                ShowSevereError(state, format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10)));
                ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
                ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
                thisChiller.FlowMode = DataPlant::FlowMode::NotModulated;
            };
        }

        //   Chiller rated performance data
        thisChiller.RefCap = s_ipsc->rNumericArgs(1);
        if (thisChiller.RefCap == DataSizing::AutoSize) {
            thisChiller.RefCapWasAutoSized = true;
        }
        if (s_ipsc->rNumericArgs(1) == 0.0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={:.2R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
            ErrorsFound = true;
        }

        thisChiller.RefCOP = s_ipsc->rNumericArgs(2);
        if (s_ipsc->rNumericArgs(2) == 0.0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={:.2R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
            ErrorsFound = true;
        }

        thisChiller.TempRefEvapOut = s_ipsc->rNumericArgs(3);
        thisChiller.TempRefCondOut = s_ipsc->rNumericArgs(4);
        if (thisChiller.TempRefEvapOut >= thisChiller.TempRefCondOut) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state,
                              format("{} [{:.2R}] >= {} [{:.2R}]",
                                     s_ipsc->cNumericFieldNames(3),
                                     s_ipsc->rNumericArgs(3),
                                     s_ipsc->cNumericFieldNames(4),
                                     s_ipsc->rNumericArgs(4)));
            ShowContinueError(state, "Reference Leaving Chilled Water Temperature must be less than Reference Leaving Condenser Water Temperature ");
            ErrorsFound = true;
        }

        thisChiller.EvapVolFlowRate = s_ipsc->rNumericArgs(5);
        if (thisChiller.EvapVolFlowRate == DataSizing::AutoSize) {
            thisChiller.EvapVolFlowRateWasAutoSized = true;
        }
        thisChiller.CondVolFlowRate = s_ipsc->rNumericArgs(6);
        if (thisChiller.CondVolFlowRate == DataSizing::AutoSize) {
            thisChiller.CondVolFlowRateWasAutoSized = true;
        }
        thisChiller.MinPartLoadRat = s_ipsc->rNumericArgs(7);
        thisChiller.MaxPartLoadRat = s_ipsc->rNumericArgs(8);
        thisChiller.OptPartLoadRat = s_ipsc->rNumericArgs(9);
        thisChiller.MinUnloadRat = s_ipsc->rNumericArgs(10);
        thisChiller.SizFac = s_ipsc->rNumericArgs(14);
        if (thisChiller.SizFac <= 0.0) {
            thisChiller.SizFac = 1.0;
        }

        if (thisChiller.MinPartLoadRat > thisChiller.MaxPartLoadRat) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state,
                              format("{} [{:.3R}] > {} [{:.3R}]",
                                     s_ipsc->cNumericFieldNames(7),
                                     s_ipsc->rNumericArgs(7),
                                     s_ipsc->cNumericFieldNames(8),
                                     s_ipsc->rNumericArgs(8)));
            ShowContinueError(state, "Minimum part load ratio must be less than or equal to the maximum part load ratio ");
            ErrorsFound = true;
        }

        if (thisChiller.MinUnloadRat < thisChiller.MinPartLoadRat || thisChiller.MinUnloadRat > thisChiller.MaxPartLoadRat) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} = {:.3R}", s_ipsc->cNumericFieldNames(10), s_ipsc->rNumericArgs(10)));
            ShowContinueError(state,
                              format("{} must be greater than or equal to the {}", s_ipsc->cNumericFieldNames(10), s_ipsc->cNumericFieldNames(7)));
            ShowContinueError(state,
                              format("{} must be less than or equal to the {}", s_ipsc->cNumericFieldNames(10), s_ipsc->cNumericFieldNames(8)));
            ErrorsFound = true;
        }

        if (thisChiller.OptPartLoadRat < thisChiller.MinPartLoadRat || thisChiller.OptPartLoadRat > thisChiller.MaxPartLoadRat) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} = {:.3R}", s_ipsc->cNumericFieldNames(9), s_ipsc->rNumericArgs(9)));
            ShowContinueError(state,
                              format("{} must be greater than or equal to the {}", s_ipsc->cNumericFieldNames(9), s_ipsc->cNumericFieldNames(7)));
            ShowContinueError(state, format("{} must be less than or equal to the {}", s_ipsc->cNumericFieldNames(9), s_ipsc->cNumericFieldNames(8)));
            ErrorsFound = true;
        }

        thisChiller.CompPowerToCondenserFrac = s_ipsc->rNumericArgs(11);

        if (thisChiller.CompPowerToCondenserFrac < 0.0 || thisChiller.CompPowerToCondenserFrac > 1.0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} = {:.3R}", s_ipsc->cNumericFieldNames(11), s_ipsc->rNumericArgs(11)));
            ShowContinueError(state, format("{} must be greater than or equal to zero", s_ipsc->cNumericFieldNames(11)));
            ShowContinueError(state, format("{} must be less than or equal to one", s_ipsc->cNumericFieldNames(11)));
            ErrorsFound = true;
        }

        thisChiller.TempLowLimitEvapOut = s_ipsc->rNumericArgs(12);

        // These are the optional heat recovery inputs
        thisChiller.DesignHeatRecVolFlowRate = s_ipsc->rNumericArgs(13);
        if (thisChiller.DesignHeatRecVolFlowRate == DataSizing::AutoSize) {
            thisChiller.DesignHeatRecVolFlowRateWasAutoSized = true;
        }
        if ((thisChiller.DesignHeatRecVolFlowRate > 0.0) || (thisChiller.DesignHeatRecVolFlowRate == DataSizing::AutoSize)) {
            thisChiller.HeatRecActive = true;
            thisChiller.HeatRecInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                  s_ipsc->cAlphaArgs(11),
                                                                                  ErrorsFound,
                                                                                  DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                                  s_ipsc->cAlphaArgs(1),
                                                                                  DataLoopNode::NodeFluidType::Water,
                                                                                  DataLoopNode::ConnectionType::Inlet,
                                                                                  NodeInputManager::CompFluidStream::Tertiary,
                                                                                  DataLoopNode::ObjectIsNotParent);
            if (thisChiller.HeatRecInletNodeNum == 0) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(11), s_ipsc->cAlphaArgs(11)));
                ErrorsFound = true;
            }
            thisChiller.HeatRecOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                   s_ipsc->cAlphaArgs(12),
                                                                                   ErrorsFound,
                                                                                   DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                                                   s_ipsc->cAlphaArgs(1),
                                                                                   DataLoopNode::NodeFluidType::Water,
                                                                                   DataLoopNode::ConnectionType::Outlet,
                                                                                   NodeInputManager::CompFluidStream::Tertiary,
                                                                                   DataLoopNode::ObjectIsNotParent);
            if (thisChiller.HeatRecOutletNodeNum == 0) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(12), s_ipsc->cAlphaArgs(12)));
                ErrorsFound = true;
            }

            BranchNodeConnections::TestCompSet(
                state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(11), s_ipsc->cAlphaArgs(12), "Heat Recovery Nodes");

            if (thisChiller.DesignHeatRecVolFlowRate > 0.0) {
                PlantUtilities::RegisterPlantCompDesignFlow(state, thisChiller.HeatRecInletNodeNum, thisChiller.DesignHeatRecVolFlowRate);
            }
            if (NumNums > 14) {
                if (!s_ipsc->lNumericFieldBlanks(15)) {
                    thisChiller.HeatRecCapacityFraction = s_ipsc->rNumericArgs(15);
                } else {
                    thisChiller.HeatRecCapacityFraction = 1.0;
                }
            } else {
                thisChiller.HeatRecCapacityFraction = 1.0;
            }

            if (NumAlphas <= 12 || s_ipsc->lAlphaFieldBlanks(13)) {
                thisChiller.heatRecInletLimitSched = nullptr; // Ok for this to be nullptr
            } else if ((thisChiller.heatRecInletLimitSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(13))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(13), s_ipsc->cAlphaArgs(13));
                ErrorsFound = true;
            }

            if (NumAlphas > 13) {
                if (!s_ipsc->lAlphaFieldBlanks(14)) {
                    thisChiller.HeatRecSetPointNodeNum =
                        NodeInputManager::GetOnlySingleNode(state,
                                                            s_ipsc->cAlphaArgs(14),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::ChillerElectricReformulatedEIR,
                                                            s_ipsc->cAlphaArgs(1),
                                                            DataLoopNode::NodeFluidType::Water,
                                                            DataLoopNode::ConnectionType::Sensor,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            DataLoopNode::ObjectIsNotParent);
                } else {
                    thisChiller.HeatRecSetPointNodeNum = 0;
                }
            } else {
                thisChiller.HeatRecSetPointNodeNum = 0;
            }

        } else {
            thisChiller.HeatRecActive = false;
            thisChiller.DesignHeatRecMassFlowRate = 0.0;
            thisChiller.HeatRecInletNodeNum = 0;
            thisChiller.HeatRecOutletNodeNum = 0;
            if ((!s_ipsc->lAlphaFieldBlanks(11)) || (!s_ipsc->lAlphaFieldBlanks(12))) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowWarningError(state, "Since Reference Heat Reclaim Volume Flow Rate = 0.0, heat recovery is inactive.");
                ShowContinueError(state, "However, node names were specified for heat recovery inlet or outlet nodes.");
            }
        }

        if (NumAlphas > 14) {
            thisChiller.EndUseSubcategory = s_ipsc->cAlphaArgs(15);
        } else {
            thisChiller.EndUseSubcategory = "General";
        }

        if (NumAlphas > 15) {
            thisChiller.CondenserFlowControl =
                static_cast<DataPlant::CondenserFlowControl>(getEnumValue(DataPlant::CondenserFlowControlNamesUC, s_ipsc->cAlphaArgs(16)));
        } else {
            thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ConstantFlow;
        }

        if (thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::Invalid) {
            ShowSevereError(state, format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(16), s_ipsc->cAlphaArgs(16)));
            ShowContinueError(state, "Available choices are ConstantFlow, ModulatedChillerPLR, ModulatedLoopPLR, or ModulatedDeltaTemperature");
            thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ConstantFlow;
            ErrorsFound = true;
        };

        if (NumAlphas > 16) {
            thisChiller.ChillerCondLoopFlowFLoopPLRIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(17));
        }
        if ((thisChiller.ChillerCondLoopFlowFLoopPLRIndex == 0) &&
            (thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::ModulatedLoopPLR)) {
            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(17), s_ipsc->cAlphaArgs(17)));
            ErrorsFound = true;
        }

        if (NumAlphas < 17 || s_ipsc->lAlphaFieldBlanks(18)) {
            thisChiller.condDTSched = nullptr; // ok for this to be nullptr
        } else if (((thisChiller.condDTSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(18))) == nullptr) &&
                   thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::ModulatedDeltaTemperature) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(18), s_ipsc->cAlphaArgs(18));
            ErrorsFound = true;
        }

        if (NumNums > 15) {
            thisChiller.MinCondFlowRatio = s_ipsc->rNumericArgs(16);
        }
        if (!s_ipsc->lAlphaFieldBlanks(19)) {
            thisChiller.thermosiphonTempCurveIndex = Curve::GetCurveIndex(state, Util::makeUPPER(s_ipsc->cAlphaArgs(19)));
            if (thisChiller.thermosiphonTempCurveIndex == 0) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, thisChiller.Name));
                ShowContinueError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(19), s_ipsc->cAlphaArgs(19)));
                ErrorsFound = true;
            }
        }
        thisChiller.thermosiphonMinTempDiff = s_ipsc->rNumericArgs(17);
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", s_ipsc->cCurrentModuleObject));
    }
}

void ReformulatedEIRChillerSpecs::setupOutputVars(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Chiller Part Load Ratio",
                        Constant::Units::None,
                        this->ChillerPartLoadRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Cycling Ratio",
                        Constant::Units::None,
                        this->ChillerCyclingRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Rate",
                        Constant::Units::W,
                        this->Power,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Energy",
                        Constant::Units::J,
                        this->Energy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling,
                        this->EndUseSubcategory);

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Rate",
                        Constant::Units::W,
                        this->QEvaporator,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Energy",
                        Constant::Units::J,
                        this->EvapEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Chillers);

    SetupOutputVariable(state,
                        "Chiller False Load Heat Transfer Rate",
                        Constant::Units::W,
                        this->ChillerFalseLoadRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller False Load Heat Transfer Energy",
                        Constant::Units::J,
                        this->ChillerFalseLoad,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Inlet Temperature",
                        Constant::Units::C,
                        this->EvapInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Outlet Temperature",
                        Constant::Units::C,
                        this->EvapOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->EvapMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Rate",
                        Constant::Units::W,
                        this->QCondenser,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Energy",
                        Constant::Units::J,
                        this->CondEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::HeatRejection);

    SetupOutputVariable(state,
                        "Chiller COP",
                        Constant::Units::W_W,
                        this->ActualCOP,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Capacity Temperature Modifier Multiplier",
                        Constant::Units::None,
                        this->ChillerCapFT,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller EIR Temperature Modifier Multiplier",
                        Constant::Units::None,
                        this->ChillerEIRFT,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller EIR Part Load Modifier Multiplier",
                        Constant::Units::None,
                        this->ChillerEIRFPLR,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Inlet Temperature",
                        Constant::Units::C,
                        this->CondInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Outlet Temperature",
                        Constant::Units::C,
                        this->CondOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->CondMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Thermosiphon Status",
                        Constant::Units::None,
                        this->thermosiphonStatus,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    // If heat recovery is active then setup report variables
    if (this->HeatRecActive) {
        SetupOutputVariable(state,
                            "Chiller Total Recovered Heat Rate",
                            Constant::Units::W,
                            this->QHeatRecovery,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Chiller Total Recovered Heat Energy",
                            Constant::Units::J,
                            this->EnergyHeatRecovery,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::HeatRejection);

        SetupOutputVariable(state,
                            "Chiller Heat Recovery Inlet Temperature",
                            Constant::Units::C,
                            this->HeatRecInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Chiller Heat Recovery Outlet Temperature",
                            Constant::Units::C,
                            this->HeatRecOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Chiller Heat Recovery Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->HeatRecMassFlow,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Chiller Effective Heat Rejection Temperature",
                            Constant::Units::C,
                            this->ChillerCondAvgTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }

    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Chiller Nominal Capacity", this->Name, "[W]", this->RefCap);
    }
}

void ReformulatedEIRChillerSpecs::oneTimeInit(EnergyPlusData &state)
{
    // Locate the chillers on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state,
                                            this->Name,
                                            DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                            this->CWPlantLoc,
                                            errFlag,
                                            this->TempLowLimitEvapOut,
                                            _,
                                            _,
                                            this->EvapInletNodeNum,
                                            _);
    if (this->CondenserType != DataPlant::CondenserType::AirCooled) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                                this->CDPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->CondInletNodeNum,
                                                _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->CDPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR, true);
    }
    if (this->HeatRecActive) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                                this->HRPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->HeatRecInletNodeNum,
                                                _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR, true);
    }

    if ((this->CondenserType != DataPlant::CondenserType::AirCooled) && (this->HeatRecActive)) {
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CDPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR, false);
    }

    if (errFlag) {
        ShowFatalError(state, "InitElecReformEIRChiller: Program terminated due to previous condition(s).");
    }

    if (this->FlowMode == DataPlant::FlowMode::Constant) {
        // reset flow priority
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
    }

    if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
        // reset flow priority
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
        // check if setpoint on outlet node
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
                bool fatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->EvapOutletNodeNum, HVAC::CtrlVarType::Temp, fatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->EvapOutletNodeNum).needsSetpointChecking = false;
                if (fatalError) {
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
                state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
                state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
        }
    }
}

void ReformulatedEIRChillerSpecs::initialize(EnergyPlusData &state, bool const RunFlag, Real64 const MyLoad)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu, FSEC
    //       DATE WRITTEN   July 2006

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for initializations of the Reformulated Electric EIR Chiller variables

    // METHODOLOGY EMPLOYED:
    //  Uses the status flags to trigger initializations.

    static constexpr std::string_view RoutineName("InitElecReformEIRChiller");

    // Init more variables
    if (this->MyInitFlag) {
        this->oneTimeInit(state);
        this->setupOutputVars(state);
        this->MyInitFlag = false;
    }

    this->EquipFlowCtrl = DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowCtrl;

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {

        Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

        this->EvapMassFlowRateMax = this->EvapVolFlowRate * rho;

        PlantUtilities::InitComponentNodes(state, 0.0, this->EvapMassFlowRateMax, this->EvapInletNodeNum, this->EvapOutletNodeNum);

        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {

            rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
            this->CondMassFlowRateMax = rho * this->CondVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->CondMassFlowRateMax, this->CondInletNodeNum, this->CondOutletNodeNum);
            state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempRefCondIn;
            // get minimum condenser plant loop pump mass flow rate
            this->VSBranchPumpMinLimitMassFlowCond =
                PlantUtilities::MinFlowIfBranchHasVSPump(state, this->CDPlantLoc, this->VSBranchPumpFoundCond, this->VSLoopPumpFoundCond, false);
        } else { // air or evap air condenser
            // Initialize maximum available condenser flow rate
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate =
                this->CondVolFlowRate *
                Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, this->TempRefCondIn, 0.0, RoutineName);
            state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRateMax = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRateMax = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempRefCondIn;
        }

        if (this->HeatRecActive) {
            rho = this->HRPlantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
            this->DesignHeatRecMassFlowRate = rho * this->DesignHeatRecVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesignHeatRecMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);
            // overall capacity limit
            this->HeatRecMaxCapacityLimit = this->HeatRecCapacityFraction * (this->RefCap + this->RefCap / this->RefCOP);
        }

        this->MyEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) && this->ModulatedFlowSetToLoop) {
        // fix for clumsy old input that worked because loop setpoint was spread.
        //  could be removed with transition, testing , model change, period of being obsolete.
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
    }

    Real64 mdot = 0.0;
    Real64 mdotCond = 0.0;
    if ((std::abs(MyLoad) > 0.0) && RunFlag) {
        mdot = this->EvapMassFlowRateMax;
        mdotCond = this->CondMassFlowRateMax;
    }

    PlantUtilities::SetComponentFlowRate(state, mdot, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        PlantUtilities::SetComponentFlowRate(state, mdotCond, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
    }
    // Initialize heat recovery flow rates at node
    if (this->HeatRecActive) {

        // check if inlet limit active and if exceeded.
        bool HeatRecRunFlag = RunFlag;
        if (this->heatRecInletLimitSched != nullptr) {
            Real64 HeatRecHighInletLimit = this->heatRecInletLimitSched->getCurrentVal();
            if (state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp > HeatRecHighInletLimit) { // shut down heat recovery
                HeatRecRunFlag = false;
            } else {
                HeatRecRunFlag = RunFlag;
            }
        }

        mdot = HeatRecRunFlag ? this->DesignHeatRecMassFlowRate : 0.0;

        PlantUtilities::SetComponentFlowRate(state, mdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
    }
}

void ReformulatedEIRChillerSpecs::size(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2004
    //       MODIFIED       July 2006, L. Gu, modified for reformulated EIR chiller
    //                      November 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for sizing Reformulated Electric EIR Chiller Components for which capacities and flow rates
    //  have not been specified in the input.

    // METHODOLOGY EMPLOYED:
    //  Obtains evaporator flow rate from the plant sizing array. Calculates reference capacity from
    //  the evaporator flow rate and the chilled water loop design delta T. The condenser flow rate
    //  is calculated from the reference capacity, the COP, and the condenser loop design delta T.

    static constexpr std::string_view RoutineName("SizeElecReformEIRChiller");

    bool ErrorsFound(false); // If errors detected in input

    Real64 tmpNomCap = this->RefCap;
    Real64 tmpEvapVolFlowRate = this->EvapVolFlowRate;
    Real64 tmpCondVolFlowRate = this->CondVolFlowRate;

    int PltSizCondNum(0); // Plant Sizing index for condenser loop
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        PltSizCondNum = this->CDPlantLoc.loop->PlantSizNum;
    }

    // find the appropriate Plant Sizing object
    int PltSizNum = this->CWPlantLoc.loop->PlantSizNum;

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpEvapVolFlowRate = state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * this->SizFac;
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->EvapVolFlowRateWasAutoSized) {
                this->EvapVolFlowRate = tmpEvapVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Initial Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
            } else { // Hard-size with sizing data
                if (this->EvapVolFlowRate > 0.0 && tmpEvapVolFlowRate > 0.0) {
                    Real64 EvapVolFlowRateUser = this->EvapVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:ReformulatedEIR",
                                                     this->Name,
                                                     "Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                     tmpEvapVolFlowRate,
                                                     "User-Specified Reference Chilled Water Flow Rate [m3/s]",
                                                     EvapVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpEvapVolFlowRate - EvapVolFlowRateUser) / EvapVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeChillerElectricReformulatedEIR: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Reference Chilled Water Flow Rate of {:.5R} [m3/s]", EvapVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Reference Chilled Water Flow Rate of {:.5R} [m3/s]", tmpEvapVolFlowRate));
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
            ShowSevereError(state, "Autosizing of Reformulated Electric Chiller evap flow rate requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Reformulated Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport &&
            (this->EvapVolFlowRate > 0.0)) { // Hard-size with sizing data
            BaseSizer::reportSizerOutput(state,
                                         "Chiller:Electric:ReformulatedEIR",
                                         this->Name,
                                         "User-Specified Reference Chilled Water Flow Rate [m3/s]",
                                         this->EvapVolFlowRate);
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->EvapInletNodeNum, tmpEvapVolFlowRate);

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            Real64 SizingEvapOutletTemp = this->TempRefEvapOut; // Plant Sizing outlet temperature for CurLoopNum [C]
            Real64 SizingCondOutletTemp = this->TempRefCondOut; // Plant Sizing outlet temperature for condenser loop [C]
            if (PltSizCondNum > 0 && PltSizNum > 0) {
                SizingEvapOutletTemp = state.dataSize->PlantSizData(PltSizNum).ExitTemp;
                SizingCondOutletTemp = state.dataSize->PlantSizData(PltSizCondNum).ExitTemp + state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
            }
            Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);
            Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            Real64 RefCapFT = Curve::CurveValue(state, this->ChillerCapFTIndex, SizingEvapOutletTemp, SizingCondOutletTemp);
            tmpNomCap = (Cp * rho * state.dataSize->PlantSizData(PltSizNum).DeltaT * tmpEvapVolFlowRate) / RefCapFT;
        } else {
            if (this->RefCapWasAutoSized) {
                tmpNomCap = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->RefCapWasAutoSized) {
                this->RefCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Chiller:Electric:ReformulatedEIR", this->Name, "Design Size Reference Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Chiller:Electric:ReformulatedEIR", this->Name, "Initial Design Size Reference Capacity [W]", tmpNomCap);
                }
            } else {
                if (this->RefCap > 0.0 && tmpNomCap > 0.0) {
                    Real64 RefCapUser = this->RefCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:ReformulatedEIR",
                                                     this->Name,
                                                     "Design Size Reference Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Reference Capacity [W]",
                                                     RefCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - RefCapUser) / RefCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("Size:ChillerElectricReformulatedEIR: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Reference Capacity of {:.2R} [W]", RefCapUser));
                                ShowContinueError(state, format("differs from Design Size Reference Capacity of {:.2R} [W]", tmpNomCap));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpNomCap = RefCapUser;
                }
            }
        }
    } else {
        if (this->RefCapWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Reformulated Electric Chiller reference capacity requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Reformulated Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->RefCapWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->RefCap > 0.0)) {
            BaseSizer::reportSizerOutput(
                state, "Chiller:Electric:ReformulatedEIR", this->Name, "User-Specified Reference Capacity [W]", this->RefCap);
        }
    }

    if (PltSizCondNum > 0 && PltSizNum > 0 && this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {
            Real64 rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
            Real64 Cp = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, this->TempRefCondIn, RoutineName);
            tmpCondVolFlowRate = tmpNomCap * (1.0 + (1.0 / this->RefCOP) * this->CompPowerToCondenserFrac) /
                                 (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);
            // IF (DataPlant::PlantFirstSizesOkayToFinalize) ElecReformEIRChiller(EIRChillNum)%CondVolFlowRate = tmpCondVolFlowRate
        } else {
            if (this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = 0.0;
            }
            // IF (DataPlant::PlantFirstSizesOkayToFinalize) ElecReformEIRChiller(EIRChillNum)%CondVolFlowRate = tmpCondVolFlowRate
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->CondVolFlowRateWasAutoSized) {
                this->CondVolFlowRate = tmpCondVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Design Size Reference Condenser Water Flow Rate [m3/s]",
                                                 tmpCondVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Initial Design Size Reference Condenser Water Flow Rate [m3/s]",
                                                 tmpCondVolFlowRate);
                }
            } else {
                if (this->CondVolFlowRate > 0.0 && tmpCondVolFlowRate > 0.0) {
                    Real64 CondVolFlowRateUser = this->CondVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:ReformulatedEIR",
                                                     this->Name,
                                                     "Design Size Reference Condenser Water Flow Rate [m3/s]",
                                                     tmpCondVolFlowRate,
                                                     "User-Specified Reference Condenser Water Flow Rate [m3/s]",
                                                     CondVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpCondVolFlowRate - CondVolFlowRateUser) / CondVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("Size:ChillerElectricReformulatedEIR: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Reference Condenser Water Flow Rate of {:.5R} [m3/s]", CondVolFlowRateUser));
                                ShowContinueError(
                                    state,
                                    format("differs from Design Size Reference Condenser Water Flow Rate of {:.5R} [m3/s]", tmpCondVolFlowRate));
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
        if (this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize &&
            this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            ShowSevereError(state, "Autosizing of Reformulated Electric EIR Chiller condenser flow rate requires a condenser");
            ShowContinueError(state, "loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Reformulated Electric EIR Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        tmpCondVolFlowRate = this->CondVolFlowRateWasAutoSized ? this->RefCap * 0.000114 : this->CondVolFlowRate;
        if (this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (tmpCondVolFlowRate > 0.0) &&
            this->CondenserType != DataPlant::CondenserType::WaterCooled) {
            this->CondVolFlowRate = tmpCondVolFlowRate;
            BaseSizer::reportSizerOutput(
                state, "Chiller:Electric:ReformulatedEIR", this->Name, "Design Size Reference Condenser Air Flow Rate [m3/s]", this->CondVolFlowRate);
        } else if (!this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->CondVolFlowRate > 0.0) &&
                   this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            BaseSizer::reportSizerOutput(state,
                                         "Chiller:Electric:ReformulatedEIR",
                                         this->Name,
                                         "User-Specified Reference Condenser Water Flow Rate [m3/s]",
                                         this->CondVolFlowRate);
        }
    }

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        // save the reference condenser water volumetric flow rate for use by the condenser water loop sizing algorithms
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondInletNodeNum, tmpCondVolFlowRate);
    }

    if (this->HeatRecActive) {
        Real64 tmpHeatRecVolFlowRate;
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            tmpHeatRecVolFlowRate = tmpCondVolFlowRate * this->HeatRecCapacityFraction;
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tmpHeatRecVolFlowRate = tmpEvapVolFlowRate;
            } else {
                tmpHeatRecVolFlowRate = this->EvapVolFlowRate;
            }
            tmpHeatRecVolFlowRate *= (1.0 + (1.0 / this->RefCOP)) * this->CompPowerToCondenserFrac * this->HeatRecCapacityFraction;
        }
        if (!this->DesignHeatRecVolFlowRateWasAutoSized) {
            tmpHeatRecVolFlowRate = this->DesignHeatRecVolFlowRate;
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->DesignHeatRecVolFlowRateWasAutoSized) {
                this->DesignHeatRecVolFlowRate = tmpHeatRecVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Design Size Design Heat Recovery Fluid Flow Rate [m3/s]",
                                                 tmpHeatRecVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:ReformulatedEIR",
                                                 this->Name,
                                                 "Initial Design Size Design Heat Recovery Fluid Flow Rate [m3/s]",
                                                 tmpHeatRecVolFlowRate);
                }
            } else {
                if (this->DesignHeatRecVolFlowRate > 0.0 && tmpHeatRecVolFlowRate > 0.0) {
                    Real64 DesignHeatRecVolFlowRateUser = this->DesignHeatRecVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:ReformulatedEIR",
                                                     this->Name,
                                                     "Design Size Design Heat Recovery Fluid Flow Rate [m3/s]",
                                                     tmpHeatRecVolFlowRate,
                                                     "User-Specified Design Heat Recovery Fluid Flow Rate [m3/s]",
                                                     DesignHeatRecVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpHeatRecVolFlowRate - DesignHeatRecVolFlowRateUser) / DesignHeatRecVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("Size:ChillerElectricReformulatedEIR: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(
                                    state,
                                    format("User-Specified Design Heat Recovery Fluid Flow Rate of {:.5R} [m3/s]", DesignHeatRecVolFlowRateUser));
                                ShowContinueError(
                                    state,
                                    format("differs from Design Size Design Heat Recovery Fluid Flow Rate of {:.5R} [m3/s]", tmpHeatRecVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpHeatRecVolFlowRate = DesignHeatRecVolFlowRateUser;
                }
            }
        }
        // save the reference heat recovery fluid volumetric flow rate
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->HeatRecInletNodeNum, tmpHeatRecVolFlowRate);
    }

    std::string equipName; // Name of chiller
    if (state.dataPlnt->PlantFinalSizesOkayToReport) {

        Real64 IPLVSI_rpt_std229 = 0.0;
        Real64 IPLVIP_rpt_std229 = 0.0;

        if (this->MySizeFlag) {
            Real64 IPLVSI = 0.0;
            Real64 IPLVIP = 0.0;
            StandardRatings::CalcChillerIPLV(state,
                                             this->Name,
                                             DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                             this->RefCap,
                                             this->RefCOP,
                                             this->CondenserType,
                                             this->ChillerCapFTIndex,
                                             this->ChillerEIRFTIndex,
                                             this->ChillerEIRFPLRIndex,
                                             this->MinUnloadRat,
                                             IPLVSI,
                                             IPLVIP,
                                             this->CondVolFlowRate,
                                             this->CDPlantLoc.loopNum,
                                             this->CompPowerToCondenserFrac);

            IPLVSI_rpt_std229 = IPLVSI;
            IPLVIP_rpt_std229 = IPLVIP;

            this->MySizeFlag = false;
        }
        // create predefined report
        equipName = this->Name;
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, equipName, "Chiller:Electric:ReformulatedEIR");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, equipName, this->RefCOP);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, equipName, this->RefCap);

        // std 229 new Chiller table
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerType, this->Name, "Chiller:Electric:ReformulatedEIR");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefCap, this->Name, this->RefCap);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEff, this->Name, this->RefCOP); // Eff == COP?
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedCap, this->Name, this->RefCap); // did not find rated cap
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedEff, this->Name, this->RefCOP); // did not find rated eff or cop ; also Eff == COP?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinSI, this->Name, IPLVSI_rpt_std229);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinIP, this->Name, IPLVIP_rpt_std229);
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerPlantloopName,
                                                 this->Name,
                                                 (this->CWPlantLoc.loop != nullptr) ? this->CWPlantLoc.loop->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerPlantloopBranchName,
                                                 this->Name,
                                                 (this->CWPlantLoc.branch != nullptr) ? this->CWPlantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerCondLoopName,
                                                 this->Name,
                                                 (this->CDPlantLoc.loop != nullptr) ? this->CDPlantLoc.loop->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerCondLoopBranchName,
                                                 this->Name,
                                                 (this->CDPlantLoc.branch != nullptr) ? this->CDPlantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerMinPLR, this->Name, this->MinPartLoadRat);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerFuelType, this->Name, "Electricity");
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedEntCondTemp, this->Name, this->TempRefCondIn); // Rated==Ref?
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedLevEvapTemp, this->Name, this->TempRefEvapOut); // Rated==Ref?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEntCondTemp, this->Name, this->TempRefCondIn);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefLevEvapTemp, this->Name, this->TempRefEvapOut);

        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerDesSizeRefCHWFlowRate,
                                                 this->Name,
                                                 this->EvapMassFlowRateMax); // flowrate Max==DesignSizeRef flowrate?
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerDesSizeRefCondFluidFlowRate,
                                                 this->Name,
                                                 this->CondMassFlowRateMax); // Cond flowrate Max==DesignSizeRef Cond flowrate?
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerHeatRecPlantloopName,
                                                 this->Name,
                                                 (this->HRPlantLoc.loop != nullptr) ? this->HRPlantLoc.loop->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerHeatRecPlantloopBranchName,
                                                 this->Name,
                                                 (this->HRPlantLoc.branch != nullptr) ? this->HRPlantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRecRelCapFrac, this->Name, this->HeatRecCapacityFraction);
    }

    // Only check performance curves if Capacity and volumetric flow rate are greater than 0
    if (this->RefCap > 0.0 && this->CondVolFlowRate > 0.0) {
        //   Check the CAP-FT, EIR-FT, and PLR curves at reference conditions and warn user if different from 1.0 by more than +-10%
        if (this->ChillerCapFTIndex > 0) {
            Real64 CurveVal = Curve::CurveValue(state, this->ChillerCapFTIndex, this->TempRefEvapOut, this->TempRefCondOut);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, "Capacity ratio as a function of temperature curve output is not equal to 1.0");
                ShowContinueError(state, format("(+ or - 10%) at reference conditions for Chiller:Electric:ReformulatedEIR = {}", equipName));
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }
            Curve::GetCurveMinMaxValues(state,
                                        this->ChillerCapFTIndex,
                                        this->ChillerCAPFTXTempMin,
                                        this->ChillerCAPFTXTempMax,
                                        this->ChillerCAPFTYTempMin,
                                        this->ChillerCAPFTYTempMax);
        }

        if (this->ChillerEIRFTIndex > 0) {
            Real64 CurveVal = Curve::CurveValue(state, this->ChillerEIRFTIndex, this->TempRefEvapOut, this->TempRefCondOut);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, "Energy input ratio as a function of temperature curve output is not equal to 1.0");
                ShowContinueError(state, format("(+ or - 10%) at reference conditions for Chiller:Electric:ReformulatedEIR = {}", equipName));
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }
            Curve::GetCurveMinMaxValues(state,
                                        this->ChillerEIRFTIndex,
                                        this->ChillerEIRFTXTempMin,
                                        this->ChillerEIRFTXTempMax,
                                        this->ChillerEIRFTYTempMin,
                                        this->ChillerEIRFTYTempMax);
        }

        if (this->ChillerEIRFPLRIndex > 0) {
            Real64 CurveVal(0.0); // Used to verify EIR-FT/CAP-FT curves = 1 at reference conditions
            if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
                CurveVal = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, this->TempRefCondOut, 1.0);
            } else if (this->PartLoadCurveType == PLR::Lift) {
                CurveVal = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, 1.0, 1.0, 0.0);
            }
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, "Energy input ratio as a function of part-load ratio curve output is not equal to 1.0");
                ShowContinueError(state, format("(+ or - 10%) at reference conditions for Chiller:Electric:ReformulatedEIR = {}", equipName));
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }

            if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
                Curve::GetCurveMinMaxValues(state,
                                            this->ChillerEIRFPLRIndex,
                                            this->ChillerEIRFPLRTempMin,
                                            this->ChillerEIRFPLRTempMax,
                                            this->ChillerEIRFPLRPLRMin,
                                            this->ChillerEIRFPLRPLRMax);
            } else if (this->PartLoadCurveType == PLR::Lift) {
                Curve::GetCurveMinMaxValues(state,
                                            this->ChillerEIRFPLRIndex,
                                            this->ChillerLiftNomMin,
                                            this->ChillerLiftNomMax,
                                            this->ChillerEIRFPLRPLRMin,
                                            this->ChillerEIRFPLRPLRMax,
                                            this->ChillerTdevNomMin,
                                            this->ChillerTdevNomMax);
            }

            if (this->ChillerEIRFPLRPLRMin < 0 || this->ChillerEIRFPLRPLRMin >= this->ChillerEIRFPLRPLRMax || this->ChillerEIRFPLRPLRMin > 1) {
                ShowSevereError(state,
                                format("Invalid minimum value of PLR = {:.3T} in bicubic curve = {} which is used",
                                       this->ChillerEIRFPLRPLRMin,
                                       this->EIRFPLRName));
                ShowContinueError(state, format("by Chiller:Electric:ReformulatedEIR = {}.", equipName));
                ShowContinueError(state, "The minimum value of PLR [y] must be from zero to 1, and less than the maximum value of PLR.");
                ErrorsFound = true;
            }
            if (this->ChillerEIRFPLRPLRMax > 1.1 || this->ChillerEIRFPLRPLRMax <= this->ChillerEIRFPLRPLRMin || this->ChillerEIRFPLRPLRMax < 0) {
                ShowSevereError(state,
                                format("Invalid maximum value of PLR = {:.3T} in bicubic curve = {} which is used",
                                       this->ChillerEIRFPLRPLRMax,
                                       this->EIRFPLRName));
                ShowContinueError(state, format("by Chiller:Electric:ReformulatedEIR = {}.", equipName));
                ShowContinueError(state, "The maximum value of PLR [y] must be from zero to 1.1, and greater than the minimum value of PLR.");
                ErrorsFound = true;
            }
            //     calculate the condenser outlet temp proportional to PLR and test the EIRFPLR curve output for negative numbers.
        }

        //  Initialize condenser reference inlet temperature (not a user input)
        Real64 Density = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondOut, RoutineName);

        Real64 SpecificHeat = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, this->TempRefCondOut, RoutineName);
        Real64 CondenserCapacity = this->RefCap * (1.0 + (1.0 / this->RefCOP) * this->CompPowerToCondenserFrac);
        Real64 DeltaTCond = (CondenserCapacity) / (this->CondVolFlowRate * Density * SpecificHeat);
        this->TempRefCondIn = this->TempRefCondOut - DeltaTCond;

        if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
            //     Check EIRFPLR curve output. Calculate condenser inlet temp based on reference condenser outlet temp,
            //     chiller capacity, and mass flow rate. Starting with the calculated condenser inlet temp and PLR = 0,
            //     calculate the condenser outlet temp proportional to PLR and test the EIRFPLR curve output for negative numbers.
            bool FoundNegValue = false;
            Array1D<Real64> CurveValArray(11, 0.0); // Used to evaluate EIRFPLR curve objects
            Array1D<Real64> CondTempArray(11, 0.0); // Used to evaluate EIRFPLR curve objects

            if (this->ChillerEIRFPLRIndex > 0) {
                CondTempArray = 0.0;
                for (int CurveCheck = 0; CurveCheck <= 10; ++CurveCheck) {
                    Real64 PLRTemp = CurveCheck / 10.0;
                    Real64 CondTemp = this->TempRefCondIn + (DeltaTCond * PLRTemp);
                    CondTemp = min(CondTemp, this->ChillerEIRFPLRTempMax);
                    CondTemp = max(CondTemp, this->ChillerEIRFPLRTempMin);
                    Real64 CurveValTmp; // Used to evaluate EIRFPLR curve objects
                    if (PLRTemp < this->ChillerEIRFPLRPLRMin) {
                        CurveValTmp = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, CondTemp, this->ChillerEIRFPLRPLRMin);
                    } else {
                        CurveValTmp = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, CondTemp, PLRTemp);
                    }
                    if (CurveValTmp < 0.0) {
                        FoundNegValue = true;
                    }
                    CurveValArray(CurveCheck + 1) = int(CurveValTmp * 100.0) / 100.0;
                    CondTempArray(CurveCheck + 1) = int(CondTemp * 100.0) / 100.0;
                }
            }

            //     Output warning message if negative values are found in the EIRFPLR curve output. Results in Fatal error.
            if (FoundNegValue) {
                ShowWarningError(state, "Energy input to cooling output ratio function of part-load ratio curve shows negative values ");
                ShowContinueError(state, format("for  Chiller:Electric:ReformulatedEIR = {}.", equipName));
                ShowContinueError(state,
                                  "EIR as a function of PLR curve output at various part-load ratios and condenser water temperatures shown below:");
                ShowContinueError(state, "PLR           =    0.00   0.10   0.20   0.30   0.40   0.50   0.60   0.70   0.80   0.90   1.00");
                ShowContinueError(state, fmt::format("Cond Temp(C) = {:7.2F}", fmt::join(CondTempArray, " ")));
                ShowContinueError(state, fmt::format("Curve Output = {:7.2F}", fmt::join(CurveValArray, " ")));
                ErrorsFound = true;
            }
        }
    } else { // just get curve min/max values if capacity or cond volume flow rate = 0
        Curve::GetCurveMinMaxValues(state,
                                    this->ChillerCapFTIndex,
                                    this->ChillerCAPFTXTempMin,
                                    this->ChillerCAPFTXTempMax,
                                    this->ChillerCAPFTYTempMin,
                                    this->ChillerCAPFTYTempMax);
        Curve::GetCurveMinMaxValues(state,
                                    this->ChillerEIRFTIndex,
                                    this->ChillerEIRFTXTempMin,
                                    this->ChillerEIRFTXTempMax,
                                    this->ChillerEIRFTYTempMin,
                                    this->ChillerEIRFTYTempMax);
        if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
            Curve::GetCurveMinMaxValues(state,
                                        this->ChillerEIRFPLRIndex,
                                        this->ChillerEIRFPLRTempMin,
                                        this->ChillerEIRFPLRTempMax,
                                        this->ChillerEIRFPLRPLRMin,
                                        this->ChillerEIRFPLRPLRMax);
        } else if (this->PartLoadCurveType == PLR::Lift) {
            Curve::GetCurveMinMaxValues(state,
                                        this->ChillerEIRFPLRIndex,
                                        this->ChillerLiftNomMin,
                                        this->ChillerLiftNomMax,
                                        this->ChillerEIRFPLRPLRMin,
                                        this->ChillerEIRFPLRPLRMax,
                                        this->ChillerTdevNomMin,
                                        this->ChillerTdevNomMax);
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void ReformulatedEIRChillerSpecs::control(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag, bool const FirstIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu, FSEC
    //       DATE WRITTEN   July 2006

    // PURPOSE OF THIS SUBROUTINE:
    // Simulate a vapor compression chiller using the reformulated model developed by Mark Hydeman

    // METHODOLOGY EMPLOYED:
    // Use empirical curve fits to model performance at off-design conditions. This subroutine
    // calls Subroutines CalcReformEIRChillerModel and General::SolveRoot to obtain solution.
    // The actual chiller performance calculations are in Subroutine CalcReformEIRChillerModel.

    // REFERENCES:
    // 1. Hydeman, M., P. Sreedharan, N. Webb, and S. Blanc. 2002. "Development and Testing of a Reformulated
    //    Regression-Based Electric Chiller Model". ASHRAE Transactions, HI-02-18-2, Vol 108, Part 2, pp. 1118-1127.

    Real64 constexpr Acc(0.0001); // Accuracy control for General::SolveRoot
    int constexpr MaxIter(500);   // Iteration control for General::SolveRoot

    if (MyLoad >= 0.0 || !RunFlag) {
        this->calculate(state, MyLoad, RunFlag, state.dataLoopNodes->Node(this->CondInletNodeNum).Temp);
    } else {

        //  Find min/max condenser outlet temperature used by curve objects

        // Minimum condenser leaving temperature allowed by CAPFT curve [C]
        Real64 CAPFTYTmin = this->ChillerCAPFTYTempMin;

        // Minimum condenser leaving temperature allowed by EIRFT curve [C]
        Real64 Tmin(-99); // Minimum condenser leaving temperature allowed by curve objects [C]

        Real64 EIRFTYTmin = this->ChillerEIRFTYTempMin;
        if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
            // Minimum condenser leaving temperature allowed by EIRFPLR curve [C]
            Real64 EIRFPLRTmin = this->ChillerEIRFPLRTempMin;
            Tmin = min(CAPFTYTmin, EIRFTYTmin, EIRFPLRTmin);
        } else if (this->PartLoadCurveType == PLR::Lift) {
            Tmin = min(CAPFTYTmin, EIRFTYTmin);
        }

        // Maximum condenser leaving temperature allowed by CAPFT curve [C]
        Real64 CAPFTYTmax = this->ChillerCAPFTYTempMax;

        Real64 Tmax(-99); // Maximum condenser leaving temperature allowed by curve objects [C]

        // Maximum condenser leaving temperature allowed by EIRFT curve [C]
        Real64 EIRFTYTmax = this->ChillerEIRFTYTempMax;
        if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
            // Maximum condenser leaving temperature allowed by EIRFPLR curve [C]
            Real64 EIRFPLRTmax = this->ChillerEIRFPLRTempMax;
            Tmax = max(CAPFTYTmax, EIRFTYTmax, EIRFPLRTmax);
        } else if (this->PartLoadCurveType == PLR::Lift) {
            Tmax = max(CAPFTYTmax, EIRFTYTmax);
        }

        //  Check that condenser outlet temperature is within curve object limits prior to calling RegulaFalsi
        this->calculate(state, MyLoad, RunFlag, Tmin);

        // Condenser outlet temperature when using Tmin as input to calculate [C]
        Real64 CondTempMin = this->CondOutletTemp;

        //  Check that condenser outlet temperature is within curve object limits prior to calling RegulaFalsi
        this->calculate(state, MyLoad, RunFlag, Tmax);

        // Condenser outlet temperature when using Tmax as input to CalcReformEIRChillerModel [C]
        Real64 CondTempMax = this->CondOutletTemp;

        if (CondTempMin > Tmin && CondTempMax < Tmax) {

            int SolFla;                    // Feedback flag from General::SolveRoot
            Real64 FalsiCondOutTemp = 0.0; // RegulaFalsi condenser outlet temperature result [C]
            Real64 thisLoad =
                MyLoad; // using a temporary because a reference is necessary in the lambda, but we don't want to overwrite the outer value
            auto f = [&state, this, &thisLoad, RunFlag](Real64 FalsiCondOutTemp) {
                this->calculate(state, thisLoad, RunFlag, FalsiCondOutTemp);
                return FalsiCondOutTemp - this->CondOutletTemp; // CondOutletTemp is module level variable, final value used for reporting
            };
            General::SolveRoot(state, Acc, MaxIter, SolFla, FalsiCondOutTemp, f, Tmin, Tmax);

            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    ++this->IterLimitExceededNum;
                    if (this->IterLimitExceededNum == 1) {
                        ShowWarningError(
                            state,
                            format("{}: Iteration limit exceeded calculating condenser outlet temperature and non-converged temperature is used",
                                   this->Name));
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       this->Name + ": Iteration limit exceeded calculating condenser outlet temperature.",
                                                       this->IterLimitErrIndex,
                                                       this->CondOutletTemp,
                                                       this->CondOutletTemp);
                    }
                }
            } else if (SolFla == -2) {
                if (!state.dataGlobal->WarmupFlag) {
                    ++this->IterFailed;
                    if (this->IterFailed == 1) {
                        ShowWarningError(state,
                                         format("{}: Solution found when calculating condenser outlet temperature. The inlet temperature will used "
                                                "and the simulation continues...",
                                                this->Name));
                        ShowContinueError(state, format("Please check minimum and maximum values of x in EIRFPLR Curve {}", this->EIRFPLRName));
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       this->Name + ": Solution is not found in calculating condenser outlet temperature.",
                                                       this->IterFailedIndex,
                                                       this->CondOutletTemp,
                                                       this->CondOutletTemp);
                    }
                }
                this->calculate(state, MyLoad, RunFlag, state.dataLoopNodes->Node(this->CondInletNodeNum).Temp);
            }
        } else {
            //    If iteration is not possible, average the min/max condenser outlet temperature and manually determine solution
            this->calculate(state, MyLoad, RunFlag, (CondTempMin + CondTempMax) / 2.0);
            this->calculate(state, MyLoad, RunFlag, this->CondOutletTemp);
        }

        //  Call subroutine to evaluate all performance curve min/max values against evaporator/condenser outlet temps and PLR
        this->checkMinMaxCurveBoundaries(state, FirstIteration);
    }
}

void ReformulatedEIRChillerSpecs::calcHeatRecovery(EnergyPlusData &state,
                                                   Real64 &QCond,              // Current condenser load [W]
                                                   Real64 const CondMassFlow,  // Current condenser mass flow [kg/s]
                                                   Real64 const condInletTemp, // Current condenser inlet temp [C]
                                                   Real64 &QHeatRec            // Amount of heat recovered [W]
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Lixing Gu, FSEC
    //       DATE WRITTEN:    July 2006

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculate the heat recovered from the chiller condenser

    static constexpr std::string_view RoutineName("EIRChillerHeatRecovery");

    // inlet node to the heat recovery heat exchanger
    Real64 heatRecInletTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
    Real64 HeatRecMassFlowRate = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;

    Real64 CpHeatRec = this->HRPlantLoc.loop->glycol->getSpecificHeat(state, heatRecInletTemp, RoutineName);
    Real64 CpCond;
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        CpCond = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, condInletTemp, RoutineName);
    } else {
        CpCond = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->HeatRecInletNodeNum).HumRat);
    }

    // Before we modify the QCondenser, the total or original value is transferred to QTot
    Real64 QTotal = QCond;

    if (this->HeatRecSetPointNodeNum == 0) { // use original algorithm that blends temps
        Real64 TAvgIn = (HeatRecMassFlowRate * CpHeatRec * heatRecInletTemp + CondMassFlow * CpCond * condInletTemp) /
                        (HeatRecMassFlowRate * CpHeatRec + CondMassFlow * CpCond);

        Real64 TAvgOut = QTotal / (HeatRecMassFlowRate * CpHeatRec + CondMassFlow * CpCond) + TAvgIn;

        QHeatRec = HeatRecMassFlowRate * CpHeatRec * (TAvgOut - heatRecInletTemp);
        QHeatRec = max(QHeatRec, 0.0); // ensure non negative
        // check if heat flow too large for physical size of bundle
        QHeatRec = min(QHeatRec, this->HeatRecMaxCapacityLimit);
    } else { // use new algorithm to meet setpoint
        Real64 THeatRecSetPoint(0.0);

        switch (this->HRPlantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            THeatRecSetPoint = state.dataLoopNodes->Node(this->HeatRecSetPointNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            THeatRecSetPoint = state.dataLoopNodes->Node(this->HeatRecSetPointNodeNum).TempSetPointHi;
        } break;
        default: {
            assert(false);
        } break;
        }

        Real64 QHeatRecToSetPoint = HeatRecMassFlowRate * CpHeatRec * (THeatRecSetPoint - heatRecInletTemp);
        QHeatRecToSetPoint = max(QHeatRecToSetPoint, 0.0);
        QHeatRec = min(QTotal, QHeatRecToSetPoint);
        // check if heat flow too large for physical size of bundle
        QHeatRec = min(QHeatRec, this->HeatRecMaxCapacityLimit);
    }

    // check if limit on inlet is present and exceeded.
    if (this->heatRecInletLimitSched != nullptr) {
        Real64 HeatRecHighInletLimit = this->heatRecInletLimitSched->getCurrentVal();
        if (heatRecInletTemp > HeatRecHighInletLimit) { // shut down heat recovery
            QHeatRec = 0.0;
        }
    }

    QCond = QTotal - QHeatRec;

    // Calculate a new Heat Recovery Coil Outlet Temp
    if (HeatRecMassFlowRate > 0.0) {
        this->HeatRecOutletTemp = QHeatRec / (HeatRecMassFlowRate * CpHeatRec) + heatRecInletTemp;
    } else {
        this->HeatRecOutletTemp = heatRecInletTemp;
    }
}

void ReformulatedEIRChillerSpecs::update(EnergyPlusData &state, Real64 const MyLoad, bool const RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Lixing Gu, FSEC
    //       DATE WRITTEN:    July 2006

    // PURPOSE OF THIS SUBROUTINE:
    //  Reporting

    if (MyLoad >= 0.0 || !RunFlag) { // Chiller not running so pass inlet states to outlet states
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;

        this->ChillerPartLoadRatio = 0.0;
        this->ChillerCyclingRatio = 0.0;
        this->ChillerFalseLoadRate = 0.0;
        this->ChillerFalseLoad = 0.0;
        this->Power = 0.0;
        this->QEvaporator = 0.0;
        this->QCondenser = 0.0;
        this->Energy = 0.0;
        this->EvapEnergy = 0.0;
        this->CondEnergy = 0.0;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->ActualCOP = 0.0;

        if (this->HeatRecActive) {

            PlantUtilities::SafeCopyPlantNode(state, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);
            this->QHeatRecovery = 0.0;
            this->EnergyHeatRecovery = 0.0;
            this->HeatRecInletTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
            this->HeatRecOutletTemp = state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp;
            this->HeatRecMassFlow = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;
        }

    } else { // Chiller is running, so pass calculated values
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = this->EvapOutletTemp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = this->CondOutletTemp;
        // Set node flow rates;  for these load based models
        // assume that sufficient evaporator flow rate is available
        this->ChillerFalseLoad = this->ChillerFalseLoadRate * state.dataHVACGlobal->TimeStepSysSec;
        this->Energy = this->Power * state.dataHVACGlobal->TimeStepSysSec;
        this->EvapEnergy = this->QEvaporator * state.dataHVACGlobal->TimeStepSysSec;
        this->CondEnergy = this->QCondenser * state.dataHVACGlobal->TimeStepSysSec;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        if (this->Power != 0.0) {
            this->ActualCOP = (this->QEvaporator + this->ChillerFalseLoadRate) / this->Power;
        } else {
            this->ActualCOP = 0.0;
        }

        if (this->HeatRecActive) {

            PlantUtilities::SafeCopyPlantNode(state, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);
            this->EnergyHeatRecovery = this->QHeatRecovery * state.dataHVACGlobal->TimeStepSysSec;
            state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp = this->HeatRecOutletTemp;
            this->HeatRecInletTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
            this->HeatRecOutletTemp = state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp;
            this->HeatRecMassFlow = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;
        }
    }
}

void ReformulatedEIRChillerSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag, Real64 const FalsiCondOutTemp)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu, FSEC
    //       DATE WRITTEN   July 2006

    //       MODIFIED
    //       Aug. 2014, Rongpeng Zhang, added an additional part-load performance curve type
    //       Jun. 2016, Rongpeng Zhang, applied the chiller supply water temperature sensor fault model
    //       Nov. 2016, Rongpeng Zhang, added fouling chiller fault

    // PURPOSE OF THIS SUBROUTINE:
    //  Simulate a vapor compression chiller using the reformulated model developed by Mark Hydeman

    // METHODOLOGY EMPLOYED:
    //  Use empirical curve fits to model performance at off-design conditions

    // REFERENCES:
    // 1. Hydeman, M., P. Sreedharan, N. Webb, and S. Blanc. 2002. "Development and Testing of a Reformulated
    //    Regression-Based Electric Chiller Model". ASHRAE Transactions, HI-02-18-2, Vol 108, Part 2, pp. 1118-1127.

    static constexpr std::string_view RoutineName("CalcElecReformEIRChillerModel");

    this->ChillerPartLoadRatio = 0.0;
    this->ChillerCyclingRatio = 0.0;
    this->ChillerFalseLoadRate = 0.0;
    this->EvapMassFlowRate = 0.0;
    this->CondMassFlowRate = 0.0;
    this->Power = 0.0;
    this->QCondenser = 0.0;
    this->QEvaporator = 0.0;
    this->QHeatRecovery = 0.0;

    // Set performance curve outputs to 0.0 when chiller is off
    this->ChillerCapFT = 0.0;
    this->ChillerEIRFT = 0.0;
    this->ChillerEIRFPLR = 0.0;
    this->thermosiphonStatus = 0;

    // Set module-level chiller evap and condenser inlet temperature variables
    Real64 condInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
    this->CondInletTemp = condInletTemp; // needed for thermosiphon model

    // If no loop demand or chiller OFF, return
    // If chiller load is 0 or chiller is not running then leave the subroutine. Before leaving
    //  if the component control is SERIESACTIVE we set the component flow to inlet flow so that
    //  flow resolver will not shut down the branch
    if (MyLoad >= 0 || !RunFlag) {
        if (this->EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive ||
            this->CWPlantLoc.side->FlowLock == DataPlant::FlowLock::Locked) {
            this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        }
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            if (DataPlant::CompData::getPlantComponent(state, this->CDPlantLoc).FlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                this->CondMassFlowRate = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            }
        }

        return;
    }

    // Chiller reference capacity [W]
    Real64 ChillerRefCap = this->RefCap;

    // Reference coefficient of performance, from user input
    Real64 ReferenceCOP = this->RefCOP;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;

    // Evaporator low temp. limit cut off [C]
    Real64 TempLowLimitEout = this->TempLowLimitEvapOut;

    // If there is a fault of chiller fouling
    if (this->FaultyChillerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
        (!state.dataGlobal->KickOffSimulation)) {
        int FaultIndex = this->FaultyChillerFoulingIndex;
        Real64 NomCap_ff = ChillerRefCap;
        Real64 ReferenceCOP_ff = ReferenceCOP;

        // calculate the Faulty Chiller Fouling Factor using fault information
        this->FaultyChillerFoulingFactor = state.dataFaultsMgr->FaultsChillerFouling(FaultIndex).CalFoulingFactor(state);

        // update the Chiller nominal capacity and COP at faulty cases
        ChillerRefCap = NomCap_ff * this->FaultyChillerFoulingFactor;
        ReferenceCOP = ReferenceCOP_ff * this->FaultyChillerFoulingFactor;
    }

    // Set initial mass flow rates
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        this->CondMassFlowRate = this->CondMassFlowRateMax;
        PlantUtilities::SetComponentFlowRate(state, this->CondMassFlowRate, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
        PlantUtilities::PullCompInterconnectTrigger(
            state, this->CWPlantLoc, this->CondMassFlowIndex, this->CDPlantLoc, DataPlant::CriteriaType::MassFlowRate, this->CondMassFlowRate);

        if (this->CondMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
            MyLoad = 0.0;
            this->EvapMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            return;
        }
    }
    Real64 FRAC = 1.0;
    Real64 EvapOutletTempSetPoint(0.0); // Evaporator outlet temperature setpoint [C]
    switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        }
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
        }
    } break;
    default: {
        assert(false);
    } break;
    }

    // If there is a fault of Chiller SWT Sensor
    if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation)) {
        int FaultIndex = this->FaultyChillerSWTIndex;
        Real64 EvapOutletTempSetPoint_ff = EvapOutletTempSetPoint;

        // calculate the sensor offset using fault information
        this->FaultyChillerSWTOffset = state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex).CalFaultOffsetAct(state);
        // update the EvapOutletTempSetPoint
        EvapOutletTempSetPoint =
            max(this->TempLowLimitEvapOut,
                min(state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, EvapOutletTempSetPoint_ff - this->FaultyChillerSWTOffset));
        this->FaultyChillerSWTOffset = EvapOutletTempSetPoint_ff - EvapOutletTempSetPoint;
    }

    // correct temperature if using heat recovery
    // use report values for latest valid calculation, lagged somewhat
    if (this->HeatRecActive) {
        if ((this->QHeatRecovery + this->QCondenser) > 0.0) { // protect div by zero
            this->ChillerCondAvgTemp =
                (this->QHeatRecovery * this->HeatRecOutletTemp + this->QCondenser * this->CondOutletTemp) / (this->QHeatRecovery + this->QCondenser);
        } else {
            this->ChillerCondAvgTemp = FalsiCondOutTemp;
        }
    } else {
        this->ChillerCondAvgTemp = FalsiCondOutTemp;
    }

    // Get capacity curve info with respect to CW setpoint and leaving condenser water temps
    this->ChillerCapFT = max(0.0, Curve::CurveValue(state, this->ChillerCapFTIndex, EvapOutletTempSetPoint, this->ChillerCondAvgTemp));

    // Available chiller capacity as a function of temperature
    Real64 AvailChillerCap = ChillerRefCap * this->ChillerCapFT;

    this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
    //   Some other component set the flow to 0. No reason to continue with calculations.
    if (this->EvapMassFlowRate == 0.0) {
        MyLoad = 0.0;
        return;
    }

    // This chiller is currently has only a water-cooled condenser

    // Calculate water side load
    Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, RoutineName);

    // actual load to be met by chiller. This value is compared to MyLoad
    // and reset when necessary since this chiller can cycle, the load passed
    // should be the actual load.  Instead the minimum PLR * RefCap is
    // passed in.
    Real64 TempLoad = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRateMaxAvail * Cp *
                      (state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapOutletTempSetPoint);

    TempLoad = max(0.0, TempLoad);

    // MyLoad is capped at minimum PLR * RefCap, adjust load to actual water side load because this chiller can cycle
    if (std::abs(MyLoad) > TempLoad) {
        MyLoad = sign(TempLoad, MyLoad);
    }

    // Part load ratio based on load and available chiller capacity, cap at max part load ratio
    Real64 PartLoadRat = 0.0; // Operating part load ratio
    if (AvailChillerCap > 0) {
        PartLoadRat = max(0.0, min(std::abs(MyLoad) / AvailChillerCap, this->MaxPartLoadRat));
    }

    // Set evaporator heat transfer rate
    this->QEvaporator = AvailChillerCap * PartLoadRat;
    this->ChillerPartLoadRatio = PartLoadRat;
    // If FlowLock is False (0), the chiller sets the plant loop mdot
    // If FlowLock is True (1),  the new resolved plant loop mdot is used
    if (this->CWPlantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
        this->PossibleSubcooling = !(this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased);

        Real64 EvapDeltaTemp(0.0); // Evaporator temperature difference [C]

        // Either set the flow to the Constant value or calculate the flow for the variable volume case
        if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
            // Set the evaporator mass flow rate to design
            // Start by assuming max (design) flow
            this->EvapMassFlowRate = this->EvapMassFlowRateMax;
            // Use PlantUtilities::SetComponentFlowRate to decide actual flow
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            if (this->EvapMassFlowRate != 0.0) {
                EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
            } else {
                EvapDeltaTemp = 0.0;
            }
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
        } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
            switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                // Calculate the Delta Temp from the inlet temp to the chiller outlet setpoint
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
                this->EvapMassFlowRate = max(0.0, (this->QEvaporator / Cp / EvapDeltaTemp));
                if ((this->EvapMassFlowRate - this->EvapMassFlowRateMax) > DataBranchAirLoopPlant::MassFlowTolerance) {
                    this->PossibleSubcooling = true;
                }
                // Check to see if the Maximum is exceeded, if so set to maximum
                this->EvapMassFlowRate = min(this->EvapMassFlowRateMax, this->EvapMassFlowRate);
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // Should we recalculate this with the corrected setpoint?
                switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
                } break;
                default:
                    break;
                }
                this->QEvaporator = max(0.0, (this->EvapMassFlowRate * Cp * EvapDeltaTemp));
            } else {
                // Try to request zero flow
                this->EvapMassFlowRate = 0.0;
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // No deltaT since component is not running
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                this->QEvaporator = 0.0;
                PartLoadRat = 0.0;
                this->ChillerPartLoadRatio = PartLoadRat;

                if (this->DeltaTErrCount < 1 && !state.dataGlobal->WarmupFlag) {
                    ++this->DeltaTErrCount;
                    ShowWarningError(state, "Evaporator DeltaTemp = 0 in mass flow calculation (Tevapin = Tevapout setpoint temp).");
                    ShowContinueErrorTimeStamp(state, "");
                } else if (!state.dataGlobal->WarmupFlag) {
                    ++this->ChillerCapFTError;
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                                       "\": Evaporator DeltaTemp = 0 in mass flow calculation warning continues...",
                                                   this->DeltaTErrCountIndex,
                                                   EvapDeltaTemp,
                                                   EvapDeltaTemp);
                }
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
                                    Cp,
                                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                    this->EvapOutletTemp,
                                    this->EvapMassFlowRate,
                                    this->QEvaporator);
            // update corresponding variables at faulty case
            PartLoadRat = (AvailChillerCap > 0.0) ? (this->QEvaporator / AvailChillerCap) : 0.0;
            PartLoadRat = max(0.0, min(PartLoadRat, this->MaxPartLoadRat));
            this->ChillerPartLoadRatio = PartLoadRat;
        }

    } else { // If FlowLock is True
        this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
        //       Some other component set the flow to 0. No reason to continue with calculations.
        if (this->EvapMassFlowRate == 0.0) {
            MyLoad = 0.0;
            return;
        }

        Real64 EvapDeltaTemp;
        if (this->PossibleSubcooling) {
            this->QEvaporator = std::abs(MyLoad);
            EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
        } else {
            EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapOutletTempSetPoint;
            this->QEvaporator = max(0.0, (this->EvapMassFlowRate * Cp * EvapDeltaTemp));
            this->EvapOutletTemp = EvapOutletTempSetPoint;
        }
        if (this->EvapOutletTemp < TempLowLimitEout) {
            if ((state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - TempLowLimitEout) > DataPlant::DeltaTempTol) {
                this->EvapOutletTemp = TempLowLimitEout;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * Cp * EvapDeltaTemp;
            } else {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * Cp * EvapDeltaTemp;
            }
        }
        if (this->EvapOutletTemp < state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin) {
            if ((state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin) >
                DataPlant::DeltaTempTol) {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempMin;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * Cp * EvapDeltaTemp;
            } else {
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - this->EvapOutletTemp;
                this->QEvaporator = this->EvapMassFlowRate * Cp * EvapDeltaTemp;
            }
        }
        // If load exceeds the distributed load set to the distributed load
        if (this->QEvaporator > std::abs(MyLoad)) {
            if (this->EvapMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                this->QEvaporator = std::abs(MyLoad);
                EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
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
                                    Cp,
                                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                    this->EvapOutletTemp,
                                    this->EvapMassFlowRate,
                                    this->QEvaporator);
            // update corresponding variables at faulty case
        }

        // Checks QEvaporator on the basis of the machine limits.
        if (this->QEvaporator > (AvailChillerCap * this->MaxPartLoadRat)) {
            if (this->EvapMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                this->QEvaporator = AvailChillerCap * this->MaxPartLoadRat;
                EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
                // evaporator outlet temperature is allowed to float upwards (recalculate AvailChillerCap? iterate?)
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
            } else {
                this->QEvaporator = 0.0;
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
            }
        }

        if (AvailChillerCap > 0.0) {
            PartLoadRat = max(0.0, min((this->QEvaporator / AvailChillerCap), this->MaxPartLoadRat));
        } else {
            PartLoadRat = 0.0;
        }

        // Chiller cycles below minimum part load ratio, FRAC = amount of time chiller is ON during this time step
        if (PartLoadRat < this->MinPartLoadRat) {
            FRAC = min(1.0, (PartLoadRat / this->MinPartLoadRat));
        }

        // Chiller is false loading below PLR = minimum unloading ratio, find PLR used for energy calculation
        if (AvailChillerCap > 0.0) {
            PartLoadRat = max(PartLoadRat, this->MinUnloadRat);
        } else {
            PartLoadRat = 0.0;
        }

        // set the module level variable used for reporting PLR
        this->ChillerPartLoadRatio = PartLoadRat;

        // calculate the load due to false loading on chiller over and above water side load
        this->ChillerFalseLoadRate = (AvailChillerCap * PartLoadRat * FRAC) - this->QEvaporator;
        if (this->ChillerFalseLoadRate < HVAC::SmallLoad) {
            this->ChillerFalseLoadRate = 0.0;
        }

    } // This is the end of the FlowLock Block

    // set the module level variable used for reporting FRAC
    this->ChillerCyclingRatio = FRAC;

    this->ChillerEIRFT = max(0.0, Curve::CurveValue(state, this->ChillerEIRFTIndex, this->EvapOutletTemp, this->ChillerCondAvgTemp));

    // Part Load Ratio Curve Type: 1_LeavingCondenserWaterTemperature; 2_Lift
    if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
        this->ChillerEIRFPLR = max(0.0, Curve::CurveValue(state, this->ChillerEIRFPLRIndex, this->ChillerCondAvgTemp, PartLoadRat));
    } else if (this->PartLoadCurveType == PLR::Lift) {

        // Chiller lift
        Real64 ChillerLift = this->ChillerCondAvgTemp - this->EvapOutletTemp;

        // Deviation of leaving chilled water temperature from the reference condition
        Real64 ChillerTdev = std::abs(this->EvapOutletTemp - this->TempRefEvapOut);

        // Chiller lift under the reference condition
        Real64 ChillerLiftRef = this->TempRefCondOut - this->TempRefEvapOut;

        if (ChillerLiftRef <= 0) {
            ChillerLiftRef = 35 - 6.67;
        }

        // Normalized chiller lift
        Real64 ChillerLiftNom = ChillerLift / ChillerLiftRef;

        // Normalized ChillerTdev
        Real64 ChillerTdevNom = ChillerTdev / ChillerLiftRef;

        this->ChillerEIRFPLR = max(0.0, Curve::CurveValue(state, this->ChillerEIRFPLRIndex, ChillerLiftNom, PartLoadRat, ChillerTdevNom));
    }

    if (ReferenceCOP <= 0) {
        ReferenceCOP = 5.5;
    }
    if (this->thermosiphonDisabled(state)) {
        this->Power = (AvailChillerCap / ReferenceCOP) * this->ChillerEIRFPLR * this->ChillerEIRFT * FRAC;
    }

    this->QCondenser = this->Power * this->CompPowerToCondenserFrac + this->QEvaporator + this->ChillerFalseLoadRate;

    // set condenser mass flow rate
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        switch (this->CondenserFlowControl) {
        case DataPlant::CondenserFlowControl::ConstantFlow: {
            this->CondMassFlowRate = this->CondMassFlowRateMax;
        } break;
        case DataPlant::CondenserFlowControl::ModulatedChillerPLR: {
            this->CondMassFlowRate = this->CondMassFlowRateMax * PartLoadRat;
        } break;
        case DataPlant::CondenserFlowControl::ModulatedLoopPLR: {
            int PltSizNum = this->CWPlantLoc.loop->PlantSizNum;
            int CondPltSizNum = this->CDPlantLoc.loop->PlantSizNum;
            if (PltSizNum > 0 && CondPltSizNum > 0) {
                Real64 chwLoopCap = state.dataSize->PlantSizData(PltSizNum).DesCapacity;
                Real64 chwLoopDemand = std::abs(this->CWPlantLoc.side->UpdatedDemandToLoopSetPoint);
                Real64 cwhLoopPLR = 0.0;
                if (chwLoopDemand > 0) {
                    cwhLoopPLR = chwLoopDemand / chwLoopCap;
                }
                Real64 condWaterFlowFrac = Curve::CurveValue(state, this->ChillerCondLoopFlowFLoopPLRIndex, cwhLoopPLR);
                Real64 cwLoopDesVolFlowRate = state.dataSize->PlantSizData(CondPltSizNum).DesVolFlowRate;
                Real64 cwLoopVolFlowRate = condWaterFlowFrac * cwLoopDesVolFlowRate;
                Real64 rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
                if (chwLoopDemand > 0) {
                    this->CondMassFlowRate = cwLoopVolFlowRate * rho * this->QEvaporator / chwLoopDemand;
                } else {
                    this->CondMassFlowRate = 0.0;
                }
            } else {
                ShowFatalError(state,
                               format("{}: The ModulatedLoopPLR condenser flow control requires a Sizing:Plant object for "
                                      "both loops connected to the condenser and evaporator of the chiller.",
                                      RoutineName));
            }
        } break;
        case DataPlant::CondenserFlowControl::ModulatedDeltaTemperature: {
            Real64 CpCond = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, this->CondInletTemp, RoutineName);
            Real64 condDT = (this->condDTSched != nullptr) ? this->condDTSched->getCurrentVal() : 0.0;
            this->CondMassFlowRate = this->QCondenser / (CpCond * condDT);
        } break;

        default: {
            this->CondMassFlowRate = this->CondMassFlowRateMax;
        } break;
        }
        Real64 minCondMassFlowRate = this->MinCondFlowRatio * this->CondMassFlowRateMax;
        Real64 minPumpMassFlowRate = this->VSBranchPumpMinLimitMassFlowCond;
        Real64 maxCondMassFlowRate = min(this->CondMassFlowRate, this->CondMassFlowRateMax);
        this->CondMassFlowRate = max(maxCondMassFlowRate, minCondMassFlowRate, minPumpMassFlowRate);
        PlantUtilities::SetComponentFlowRate(state, this->CondMassFlowRate, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
        PlantUtilities::PullCompInterconnectTrigger(
            state, this->CWPlantLoc, this->CondMassFlowIndex, this->CDPlantLoc, DataPlant::CriteriaType::MassFlowRate, this->CondMassFlowRate);
    }

    //  Currently only water cooled chillers are allowed for the reformulated EIR chiller model
    if (this->CondMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
        // If Heat Recovery specified for this vapor compression chiller, then Qcondenser will be adjusted by this subroutine
        if (this->HeatRecActive) {
            this->calcHeatRecovery(state, this->QCondenser, this->CondMassFlowRate, condInletTemp, this->QHeatRecovery);
        }
        Real64 CpCond = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, condInletTemp, RoutineName);
        this->CondOutletTemp = this->QCondenser / this->CondMassFlowRate / CpCond + condInletTemp;
    } else {
        ShowSevereError(state, format("ControlReformEIRChillerModel: Condenser flow = 0, for ElecReformEIRChiller={}", this->Name));
        ShowContinueErrorTimeStamp(state, "");
    }
}

void ReformulatedEIRChillerSpecs::checkMinMaxCurveBoundaries(EnergyPlusData &state, bool const FirstIteration)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          R Raustad, FSEC
    //       DATE WRITTEN:    August 2006

    // PURPOSE OF THIS SUBROUTINE:
    //  To compare the evaporator/condenser outlet temperatures to curve object min/max values

    // Do not print out warnings if chiller not operating or FirstIteration/WarmupFlag/FlowLock
    if (FirstIteration || state.dataGlobal->WarmupFlag || this->CWPlantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
        return;
    }

    // Minimum evaporator leaving temperature allowed by CAPFT curve [C]
    Real64 CAPFTXTmin = this->ChillerCAPFTXTempMin;

    // Maximum evaporator leaving temperature allowed by CAPFT curve [C]
    Real64 CAPFTXTmax = this->ChillerCAPFTXTempMax;

    // Minimum evaporator leaving temperature allowed by EIRFT curve [C]
    Real64 EIRFTXTmin = this->ChillerEIRFTXTempMin;

    // Maximum evaporator leaving temperature allowed by EIRFT curve [C]
    Real64 EIRFTXTmax = this->ChillerEIRFTXTempMax;

    // Check bounds for curves, lump min/max into same check since min/max values are reported in recurring warning messages
    if (this->EvapOutletTemp < CAPFTXTmin || this->EvapOutletTemp > CAPFTXTmax) {
        ++this->CAPFTXIter;
        if (this->CAPFTXIter == 1) {
            ShowWarningError(
                state,
                format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The evaporator outlet temperature ({:.2T} C) is outside the range of evaporator "
                       "outlet temperatures (X var) given in Cooling Capacity Function of Temperature biquadratic curve = {}",
                       this->Name,
                       this->EvapOutletTemp,
                       this->CAPFTName));
            ShowContinueErrorTimeStamp(state, format("The range specified = {:.2T} C to {:.2T} C.", CAPFTXTmin, CAPFTXTmax));
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": The evap outlet temp range in Cooling Capacity Function of Temp curve error continues.",
                                           this->CAPFTXIterIndex,
                                           this->EvapOutletTemp,
                                           this->EvapOutletTemp);
        } else {
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": The evap outlet temp range in Cooling Capacity Function of Temp curve error continues.",
                                           this->CAPFTXIterIndex,
                                           this->EvapOutletTemp,
                                           this->EvapOutletTemp);
        }
    }

    if (this->EvapOutletTemp < EIRFTXTmin || this->EvapOutletTemp > EIRFTXTmax) {
        ++this->EIRFTXIter;
        if (this->EIRFTXIter == 1) {
            ShowWarningError(
                state,
                format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The evaporator outlet temperature ({:.2T} C) is outside the range of evaporator "
                       "outlet temperatures (X var) given in Electric Input to Cooling Output Ratio Function of Temperature biquadratic curve = {}",
                       this->Name,
                       this->EvapOutletTemp,
                       this->EIRFTName));
            ShowContinueErrorTimeStamp(state, format("The range specified = {:.2T} C to {:.2T} C.", EIRFTXTmin, EIRFTXTmax));
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The evap outlet temp range in Electric Input to Cooling Output Ratio Function of Temp curve error continues.",
                this->EIRFTXIterIndex,
                this->EvapOutletTemp,
                this->EvapOutletTemp);
        } else {
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The evap outlet temp range in Electric Input to Cooling Output Ratio Function of Temp curve error continues.",
                this->EIRFTXIterIndex,
                this->EvapOutletTemp,
                this->EvapOutletTemp);
        }
    }

    // Move CAPFT, EIRFT, and EIRFPLR min/max condenser outlet temperature values to local variables

    // Minimum condenser  leaving temperature allowed by CAPFT curve [C]
    Real64 CAPFTYTmin = this->ChillerCAPFTYTempMin;

    // Maximum condenser  leaving temperature allowed by CAPFT curve [C]
    Real64 CAPFTYTmax = this->ChillerCAPFTYTempMax;

    // Minimum condenser  leaving temperature allowed by EIRFT curve [C]
    Real64 EIRFTYTmin = this->ChillerEIRFTYTempMin;

    // Maximum condenser  leaving temperature allowed by EIRFT curve [C]
    Real64 EIRFTYTmax = this->ChillerEIRFTYTempMax;

    Real64 EIRFPLRTmin(0.0); // Minimum condenser  leaving temperature allowed by EIRFPLR curve [C]
    Real64 EIRFPLRTmax(0.0); // Maximum condenser  leaving temperature allowed by EIRFPLR curve [C]

    if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
        EIRFPLRTmin = this->ChillerEIRFPLRTempMin;
        EIRFPLRTmax = this->ChillerEIRFPLRTempMax;
    }

    // Move EIRFPLR min/max part-load ratio values to local variables

    // Minimum PLR allowed by EIRFPLR curve
    Real64 EIRFPLRPLRmin = this->ChillerEIRFPLRPLRMin;

    // Maximum PLR allowed by EIRFPLR curve
    Real64 EIRFPLRPLRmax = this->ChillerEIRFPLRPLRMax;

    // Check bounds for curves, lump min/max into same check since min/max values are reported in recurring warning messages
    if (this->CondOutletTemp < CAPFTYTmin || this->CondOutletTemp > CAPFTYTmax) {
        ++this->CAPFTYIter;
        if (this->CAPFTYIter == 1) {
            ShowWarningError(
                state,
                format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The condenser outlet temperature ({:.2T} C) is outside the range of condenser "
                       "outlet temperatures (Y var) given in Cooling Capacity Function of Temperature biquadratic curve = {}",
                       this->Name,
                       this->CondOutletTemp,
                       this->CAPFTName));
            ShowContinueErrorTimeStamp(state, format("The range specified = {:.2T} C to {:.2T} C.", CAPFTYTmin, CAPFTYTmax));
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": The cond outlet temp range in Cooling Capacity Function of Temp curve error continues.",
                                           this->CAPFTYIterIndex,
                                           this->CondOutletTemp,
                                           this->CondOutletTemp);
        } else {
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": The cond outlet temp range in Cooling Capacity Function of Temp curve error continues.",
                                           this->CAPFTYIterIndex,
                                           this->CondOutletTemp,
                                           this->CondOutletTemp);
        }
    }

    if (this->CondOutletTemp < EIRFTYTmin || this->CondOutletTemp > EIRFTYTmax) {
        ++this->EIRFTYIter;
        if (this->EIRFTYIter == 1) {
            ShowWarningError(
                state,
                format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The condenser outlet temperature ({:.2T} C) is outside the range of condenser "
                       "outlet temperatures (Y var) given in Electric Input to Cooling Output Ratio Function of Temperature biquadratic curve = {}",
                       this->Name,
                       this->CondOutletTemp,
                       this->EIRFTName));
            ShowContinueErrorTimeStamp(state, format("The range specified = {:.2T} C to {:.2T} C.", EIRFTYTmin, EIRFTYTmax));
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The cond outlet temp range in Electric Input to Cooling Output Ratio as a Function of Temp curve error continues.",
                this->EIRFTYIterIndex,
                this->CondOutletTemp,
                this->CondOutletTemp);
        } else {
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The cond outlet temp range in Electric Input to Cooling Output Ratio as a Function of Temp curve error continues.",
                this->EIRFTYIterIndex,
                this->CondOutletTemp,
                this->CondOutletTemp);
        }
    }

    if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
        if (this->CondOutletTemp < EIRFPLRTmin || this->CondOutletTemp > EIRFPLRTmax) {
            ++this->EIRFPLRTIter;
            if (this->EIRFPLRTIter == 1) {
                ShowWarningError(state,
                                 format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The condenser outlet temperature ({:.2T} C) is outside the "
                                        "range of condenser outlet temperatures (X var) given in Electric Input to Cooling Output Ratio Function "
                                        "of Part-load Ratio bicubic curve = {}",
                                        this->Name,
                                        this->CondOutletTemp,
                                        this->EIRFPLRName));
                ShowContinueErrorTimeStamp(state, format("The range specified = {:.2T} C to {:.2T} C.", EIRFPLRTmin, EIRFPLRTmax));
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                        "\": The cond outlet temp range in Electric Input to Cooling Output Ratio Function of PLR curve error continues.",
                    this->EIRFPLRTIterIndex,
                    this->CondOutletTemp,
                    this->CondOutletTemp);
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                        "\": The cond outlet temp range in Electric Input to Cooling Output Ratio Function of PLR curve error continues.",
                    this->EIRFPLRTIterIndex,
                    this->CondOutletTemp,
                    this->CondOutletTemp);
            }
        }
    }

    if (this->ChillerPartLoadRatio < EIRFPLRPLRmin || this->ChillerPartLoadRatio > EIRFPLRPLRmax) {
        ++this->EIRFPLRPLRIter;
        if (this->EIRFPLRPLRIter == 1) {
            ShowWarningError(
                state,
                format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\": The part-load ratio ({:.3T}) is outside the range of part-load ratios (Y var) "
                       "given in Electric Input to Cooling Output Ratio Function of Part-load Ratio bicubic curve = {}",
                       this->Name,
                       this->ChillerPartLoadRatio,
                       this->EIRFPLRName));
            ShowContinueErrorTimeStamp(state, format("The range specified = {:.3T} to {:.3T}.", EIRFPLRPLRmin, EIRFPLRPLRmax));
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The part-load ratio range in Electric Input to Cooling Output Ratio Function of PLRatio curve error continues.",
                this->EIRFPLRPLRIterIndex,
                this->ChillerPartLoadRatio,
                this->ChillerPartLoadRatio);
        } else {
            ShowRecurringWarningErrorAtEnd(
                state,
                "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                    "\": The part-load ratio range in Electric Input to Cooling Output Ratio Function of PLRatio curve error continues.",
                this->EIRFPLRPLRIterIndex,
                this->ChillerPartLoadRatio,
                this->ChillerPartLoadRatio);
        }
    }

    Real64 EvapOutletTempSetPoint(0.0); // Evaporator outlet temperature setpoint [C]

    switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        }
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
        }
    } break;
    default: {
        assert(false);
    } break;
    }

    this->ChillerCapFT = Curve::CurveValue(state, this->ChillerCapFTIndex, EvapOutletTempSetPoint, this->CondOutletTemp);

    if (this->ChillerCapFT < 0) {
        if (this->ChillerCapFTError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerCapFTError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\":", this->Name));
            ShowContinueError(state, format(" Chiller Capacity as a Function of Temperature curve output is negative ({:.3R}).", this->ChillerCapFT));
            ShowContinueError(state,
                              format(" Negative value occurs using an Evaporator Leaving Temp of {:.1R} and a Condenser Leaving Temp of {:.1R}.",
                                     EvapOutletTempSetPoint,
                                     this->CondOutletTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerCapFTError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": Chiller Capacity as a Function of Temperature curve output is negative warning continues...",
                                           this->ChillerCapFTErrorIndex,
                                           this->ChillerCapFT,
                                           this->ChillerCapFT);
        }
    }

    this->ChillerEIRFT = Curve::CurveValue(state, this->ChillerEIRFTIndex, this->EvapOutletTemp, this->CondOutletTemp);

    if (this->ChillerEIRFT < 0.0) {
        if (this->ChillerEIRFTError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFTError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\":", this->Name));
            ShowContinueError(
                state, format(" Reformulated Chiller EIR as a Function of Temperature curve output is negative ({:.3R}).", this->ChillerEIRFT));
            ShowContinueError(state,
                              format(" Negative value occurs using an Evaporator Leaving Temp of {:.1R} and a Condenser Leaving Temp of {:.1R}.",
                                     this->EvapOutletTemp,
                                     this->CondOutletTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFTError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": Chiller EIR as a Function of Temperature curve output is negative warning continues...",
                                           this->ChillerEIRFTErrorIndex,
                                           this->ChillerEIRFT,
                                           this->ChillerEIRFT);
        }
    }

    if (this->PartLoadCurveType == PLR::LeavingCondenserWaterTemperature) {
        this->ChillerEIRFPLR = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, this->CondOutletTemp, this->ChillerPartLoadRatio);
    } else if (this->PartLoadCurveType == PLR::Lift) {

        // Chiller lift  [C]
        Real64 ChillerLift = this->CondOutletTemp - this->EvapOutletTemp;

        // Deviation of leaving chilled water temperature from the reference condition
        Real64 ChillerTdev = std::abs(this->EvapOutletTemp - this->TempRefEvapOut);

        // Chiller lift under the reference condition  [C]
        Real64 ChillerLiftRef = this->TempRefCondOut - this->TempRefEvapOut;

        if (ChillerLiftRef <= 0) {
            ChillerLiftRef = 35 - 6.67;
        }

        // Normalized chiller lift
        Real64 ChillerLiftNom = ChillerLift / ChillerLiftRef;

        // Normalized ChillerTdev
        Real64 ChillerTdevNom = ChillerTdev / ChillerLiftRef;

        this->ChillerEIRFPLR = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, ChillerLiftNom, this->ChillerPartLoadRatio, ChillerTdevNom);
    }

    if (this->ChillerEIRFPLR < 0.0) {
        if (this->ChillerEIRFPLRError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFPLRError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:REFORMULATEDEIR \"{}\":", this->Name));
            ShowContinueError(
                state,
                format(" Chiller EIR as a function of PLR and condenser water temperature curve output is negative ({:.3R}).", this->ChillerEIRFPLR));
            ShowContinueError(state,
                              format(" Negative value occurs using a part-load ratio of {:.3R} and a Condenser Leaving Temp of {:.1R} C.",
                                     this->ChillerPartLoadRatio,
                                     this->CondOutletTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFPLRError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:REFORMULATEDEIR \"" + this->Name +
                                               "\": Chiller EIR as a function of PLR curve output is negative warning continues...",
                                           this->ChillerEIRFPLRErrorIndex,
                                           this->ChillerEIRFPLR,
                                           this->ChillerEIRFPLR);
        }
    }
}

bool ReformulatedEIRChillerSpecs::thermosiphonDisabled(EnergyPlusData &state)
{
    if (this->thermosiphonTempCurveIndex > 0) {
        this->thermosiphonStatus = 0;
        Real64 dT = this->EvapOutletTemp - this->CondInletTemp;
        if (dT < this->thermosiphonMinTempDiff) {
            return true;
        }
        Real64 thermosiphonCapFrac = Curve::CurveValue(state, this->thermosiphonTempCurveIndex, dT);
        Real64 capFrac = this->ChillerPartLoadRatio * this->ChillerCyclingRatio;
        if (thermosiphonCapFrac >= capFrac) {
            this->thermosiphonStatus = 1;
            this->Power = 0.0;
            return false;
        }
        return true;
    } else {
        return true;
    }
}

} // namespace EnergyPlus::ChillerReformulatedEIR
