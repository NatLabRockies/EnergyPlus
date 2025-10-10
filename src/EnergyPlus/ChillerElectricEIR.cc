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
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerElectricEIR.hh>
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
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::ChillerElectricEIR {

// NOTES:
// The Electric EIR and Reformulated EIR chiller models are similar.
// They only differ in the independent variable used to evaluate the performance curves.

// MODULE INFORMATION:
//       AUTHOR         Richard Raustad
//       DATE WRITTEN   June 2004
//       MODIFIED       Chandan Sharma, FSEC, February 2010, Added basin heater
//                      Brent Griffith, NREL, Sept 2010, revised for plant changes
//                      generalized fluid properties

// PURPOSE OF THIS MODULE:
//  This module simulates the performance of the electric vapor
//  compression chiller used in DOE-2.

// METHODOLOGY EMPLOYED:
//  Once the PlantLoopManager determines that the Electric EIR chiller
//  is available to meet a loop cooling demand, it calls SimElectricEIRChiller
//  which in turn calls the electric EIR model. The EIR chiller model is based on
//  polynomial fits of chiller performance data.

// REFERENCES:
// 1. DOE-2 Engineers Manual, Version 2.1A, November 1982, LBL-11353

ElectricEIRChillerSpecs *ElectricEIRChillerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data if it hasn't been done already
    if (state.dataChillerElectricEIR->getInputFlag) {
        GetElectricEIRChillerInput(state);
        state.dataChillerElectricEIR->getInputFlag = false;
    }
    // Now look for this particular object in the list
    auto thisObj = std::find_if(state.dataChillerElectricEIR->ElectricEIRChiller.begin(),
                                state.dataChillerElectricEIR->ElectricEIRChiller.end(),
                                [&objectName](const ElectricEIRChillerSpecs &myObj) { return myObj.Name == objectName; });
    if (thisObj != state.dataChillerElectricEIR->ElectricEIRChiller.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalElectEIRChillerFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void ElectricEIRChillerSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  This is the electric EIR chiller model driver. It gets the input for the
    //  model, initializes simulation variables, calls the appropriate model and sets
    //  up reporting variables.

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->initialize(state, RunFlag, CurLoad);
        this->calculate(state, CurLoad, RunFlag);
        this->update(state, CurLoad, RunFlag);

    } else if (calledFromLocation.loopNum == this->CDPlantLoc.loopNum) {
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            this->CDPlantLoc.loopSideNum,
                                                            DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
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
                                                        DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
                                                        this->HeatRecInletNodeNum,
                                                        this->HeatRecOutletNodeNum,
                                                        this->QHeatRecovered,
                                                        this->HeatRecInletTemp,
                                                        this->HeatRecOutletTemp,
                                                        this->HeatRecMassFlow,
                                                        FirstHVACIteration);
    }
}

void ElectricEIRChillerSpecs::getDesignCapacities(
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

void ElectricEIRChillerSpecs::getDesignTemperatures(Real64 &TempDesCondIn, Real64 &TempDesEvapOut)
{
    TempDesCondIn = this->TempRefCondIn;
    TempDesEvapOut = this->TempRefEvapOut;
}

void ElectricEIRChillerSpecs::getSizingFactor(Real64 &sizFac)
{
    sizFac = this->SizFac;
}

void ElectricEIRChillerSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
{
    bool runFlag = true;
    Real64 myLoad = 0.0;

    this->initialize(state, runFlag, myLoad);

    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->size(state);
    }
}

void GetElectricEIRChillerInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Richard Raustad, FSEC
    //       DATE WRITTEN:    June 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  This routine will get the input required by the Electric EIR Chiller model.

    static constexpr std::string_view RoutineName("GetElectricEIRChillerInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetElectricEIRChillerInput";  // include trailing blank space

    bool ErrorsFound(false); // True when input errors are found

    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "Chiller:Electric:EIR";
    int NumElectricEIRChillers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (NumElectricEIRChillers <= 0) {
        ShowSevereError(state, format("No {} equipment specified in input file", s_ipsc->cCurrentModuleObject));
        ErrorsFound = true;
    }

    // ALLOCATE ARRAYS
    state.dataChillerElectricEIR->ElectricEIRChiller.allocate(NumElectricEIRChillers);

    // Load arrays with electric EIR chiller data
    for (int EIRChillerNum = 1; EIRChillerNum <= NumElectricEIRChillers; ++EIRChillerNum) {
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

        auto &thisChiller = state.dataChillerElectricEIR->ElectricEIRChiller(EIRChillerNum);
        thisChiller.Name = s_ipsc->cAlphaArgs(1);

        //   Performance curves
        thisChiller.ChillerCapFTIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(2));
        if (thisChiller.ChillerCapFTIndex == 0) {
            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ErrorsFound = true;
        }

        thisChiller.ChillerEIRFTIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(3));
        if (thisChiller.ChillerEIRFTIndex == 0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            ErrorsFound = true;
        }

        thisChiller.ChillerEIRFPLRIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(4));
        if (thisChiller.ChillerEIRFPLRIndex == 0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
            ErrorsFound = true;
        }

        thisChiller.EvapInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           s_ipsc->cAlphaArgs(5),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                           s_ipsc->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.EvapOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(6),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(5), s_ipsc->cAlphaArgs(6), "Chilled Water Nodes");

        if (Util::SameString(s_ipsc->cAlphaArgs(9), "WaterCooled")) {
            thisChiller.CondenserType = DataPlant::CondenserType::WaterCooled;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(9), "AirCooled")) {
            thisChiller.CondenserType = DataPlant::CondenserType::AirCooled;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(9), "EvaporativelyCooled")) {
            thisChiller.CondenserType = DataPlant::CondenserType::EvapCooled;
        } else {
            ShowSevereError(state, format("{}{}: {}", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(9), s_ipsc->cAlphaArgs(9)));
            ShowContinueError(state, "Valid entries are AirCooled, WaterCooled, or EvaporativelyCooled");
            ErrorsFound = true;
        }

        if (thisChiller.CondenserType == DataPlant::CondenserType::AirCooled || thisChiller.CondenserType == DataPlant::CondenserType::EvapCooled) {
            // Connection not required for air or evap cooled condenser
            // If the condenser inlet is blank for air cooled and evap cooled condensers then supply a generic name
            // since it is not used elsewhere for connection
            if (s_ipsc->lAlphaFieldBlanks(7)) {
                if (len(s_ipsc->cAlphaArgs(1)) < Constant::MaxNameLength - 25) { // protect against long name leading to > 100 chars
                    s_ipsc->cAlphaArgs(7) = s_ipsc->cAlphaArgs(1) + " INLET NODE FOR CONDENSER";
                } else {
                    s_ipsc->cAlphaArgs(7) = s_ipsc->cAlphaArgs(1).substr(0, 75) + " INLET NODE FOR CONDENSER";
                }
            }
            if (s_ipsc->lAlphaFieldBlanks(8)) {
                if (len(s_ipsc->cAlphaArgs(1)) < Constant::MaxNameLength - 26) { // protect against long name leading to > 100 chars
                    s_ipsc->cAlphaArgs(8) = s_ipsc->cAlphaArgs(1) + " OUTLET NODE FOR CONDENSER";
                } else {
                    s_ipsc->cAlphaArgs(8) = s_ipsc->cAlphaArgs(1).substr(0, 74) + " OUTLET NODE FOR CONDENSER";
                }
            }

            thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                               s_ipsc->cAlphaArgs(7),
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                               s_ipsc->cAlphaArgs(1),
                                                                               DataLoopNode::NodeFluidType::Air,
                                                                               DataLoopNode::ConnectionType::OutsideAirReference,
                                                                               NodeInputManager::CompFluidStream::Secondary,
                                                                               DataLoopNode::ObjectIsNotParent);
            bool Okay = true;
            OutAirNodeManager::CheckAndAddAirNodeNumber(state, thisChiller.CondInletNodeNum, Okay);
            if (!Okay) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Adding OutdoorAir:Node={}", s_ipsc->cAlphaArgs(7)));
            }

            thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                s_ipsc->cAlphaArgs(8),
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                                s_ipsc->cAlphaArgs(1),
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Outlet,
                                                                                NodeInputManager::CompFluidStream::Secondary,
                                                                                DataLoopNode::ObjectIsNotParent);

        } else if (thisChiller.CondenserType == DataPlant::CondenserType::WaterCooled) {
            // Condenser inlet node name is necessary for water-cooled condenser
            if (s_ipsc->lAlphaFieldBlanks(7) || s_ipsc->lAlphaFieldBlanks(8)) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Condenser Inlet or Outlet Node Name is blank.");
                ErrorsFound = true;
            }

            thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                               s_ipsc->cAlphaArgs(7),
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                               s_ipsc->cAlphaArgs(1),
                                                                               DataLoopNode::NodeFluidType::Water,
                                                                               DataLoopNode::ConnectionType::Inlet,
                                                                               NodeInputManager::CompFluidStream::Secondary,
                                                                               DataLoopNode::ObjectIsNotParent);

            thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                s_ipsc->cAlphaArgs(8),
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                                s_ipsc->cAlphaArgs(1),
                                                                                DataLoopNode::NodeFluidType::Water,
                                                                                DataLoopNode::ConnectionType::Outlet,
                                                                                NodeInputManager::CompFluidStream::Secondary,
                                                                                DataLoopNode::ObjectIsNotParent);

            BranchNodeConnections::TestCompSet(
                state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(7), s_ipsc->cAlphaArgs(8), "Condenser Water Nodes");

        } else {
            // Condenser inlet node name is necessary (never should reach this part of code)
            if (s_ipsc->lAlphaFieldBlanks(7) || s_ipsc->lAlphaFieldBlanks(8)) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Condenser Inlet or Outlet Node Name is blank.");
                ErrorsFound = true;
            }
            thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                               s_ipsc->cAlphaArgs(7),
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                               s_ipsc->cAlphaArgs(1),
                                                                               DataLoopNode::NodeFluidType::Blank,
                                                                               DataLoopNode::ConnectionType::Inlet,
                                                                               NodeInputManager::CompFluidStream::Secondary,
                                                                               DataLoopNode::ObjectIsNotParent);

            thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                s_ipsc->cAlphaArgs(8),
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
                                                                                s_ipsc->cAlphaArgs(1),
                                                                                DataLoopNode::NodeFluidType::Blank,
                                                                                DataLoopNode::ConnectionType::Outlet,
                                                                                NodeInputManager::CompFluidStream::Secondary,
                                                                                DataLoopNode::ObjectIsNotParent);

            BranchNodeConnections::TestCompSet(state,
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaArgs(7),
                                               s_ipsc->cAlphaArgs(8),
                                               "Condenser (unknown?) Nodes");
        }

        thisChiller.FlowMode = static_cast<DataPlant::FlowMode>(getEnumValue(DataPlant::FlowModeNamesUC, s_ipsc->cAlphaArgs(10)));
        if (thisChiller.FlowMode == DataPlant::FlowMode::Invalid) {
            ShowSevereError(state, format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10)));
            ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
            ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
            thisChiller.FlowMode = DataPlant::FlowMode::NotModulated;
        };

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
        thisChiller.TempRefCondIn = s_ipsc->rNumericArgs(4);
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
        thisChiller.SizFac = s_ipsc->rNumericArgs(15);
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

        thisChiller.CondenserFanPowerRatio = s_ipsc->rNumericArgs(11);
        thisChiller.CompPowerToCondenserFrac = s_ipsc->rNumericArgs(12);

        if (thisChiller.CompPowerToCondenserFrac < 0.0 || thisChiller.CompPowerToCondenserFrac > 1.0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} = {:.3R}", s_ipsc->cNumericFieldNames(12), s_ipsc->rNumericArgs(12)));
            ShowContinueError(state, format("{} must be greater than or equal to zero", s_ipsc->cNumericFieldNames(12)));
            ShowContinueError(state, format("{} must be less than or equal to one", s_ipsc->cNumericFieldNames(12)));
            ErrorsFound = true;
        }

        thisChiller.TempLowLimitEvapOut = s_ipsc->rNumericArgs(13);

        // These are the heat recovery inputs
        thisChiller.DesignHeatRecVolFlowRate = s_ipsc->rNumericArgs(14);
        if (thisChiller.DesignHeatRecVolFlowRate == DataSizing::AutoSize) {
            thisChiller.DesignHeatRecVolFlowRateWasAutoSized = true;
        }
        if ((thisChiller.DesignHeatRecVolFlowRate > 0.0) || (thisChiller.DesignHeatRecVolFlowRate == DataSizing::AutoSize)) {
            thisChiller.HeatRecActive = true;
            thisChiller.HeatRecInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                  s_ipsc->cAlphaArgs(11),
                                                                                  ErrorsFound,
                                                                                  DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
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
                                                                                   DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
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
            // store heat recovery volume flow for plant sizing
            if (thisChiller.DesignHeatRecVolFlowRate > 0.0) {
                PlantUtilities::RegisterPlantCompDesignFlow(state, thisChiller.HeatRecInletNodeNum,
                                                            thisChiller.DesignHeatRecVolFlowRate); // CR 6953
            }
            if (NumNums > 17) {
                if (!s_ipsc->lNumericFieldBlanks(18)) {
                    thisChiller.HeatRecCapacityFraction = s_ipsc->rNumericArgs(18);
                } else {
                    thisChiller.HeatRecCapacityFraction = 1.0;
                }
            } else {
                thisChiller.HeatRecCapacityFraction = 1.0;
            }

            if (NumAlphas <= 13 || s_ipsc->lAlphaFieldBlanks(14)) {
                thisChiller.heatRecInletLimitSched = nullptr; // Ok for this schedule to remain null if field is empty
            } else if ((thisChiller.heatRecInletLimitSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(14))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(14), s_ipsc->cAlphaArgs(14));
                ErrorsFound = true;
            }

            if (NumAlphas > 14) {
                if (!s_ipsc->lAlphaFieldBlanks(15)) {
                    thisChiller.HeatRecSetPointNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                             s_ipsc->cAlphaArgs(15),
                                                                                             ErrorsFound,
                                                                                             DataLoopNode::ConnectionObjectType::ChillerElectricEIR,
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
            if (!s_ipsc->lAlphaFieldBlanks(11) || !s_ipsc->lAlphaFieldBlanks(12)) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Since Reference Heat Reclaim Volume Flow Rate = 0.0, heat recovery is inactive.");
                ShowContinueError(state, "However, node names were specified for heat recovery inlet or outlet nodes.");
            }
        }

        if (NumAlphas > 16) {
            thisChiller.CondenserFlowControl =
                static_cast<DataPlant::CondenserFlowControl>(getEnumValue(DataPlant::CondenserFlowControlNamesUC, s_ipsc->cAlphaArgs(17)));
        } else {
            thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ConstantFlow;
        }

        if (thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::Invalid) {
            ShowSevereError(state, format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(17), s_ipsc->cAlphaArgs(17)));
            ShowContinueError(state, "Available choices are ConstantFlow, ModulatedChillerPLR, ModulatedLoopPLR, or ModulatedDeltaTemperature");
            ShowContinueError(state, "Flow mode ConstantFlow is assumed and the simulation continues.");
            thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ConstantFlow;
        };

        if (NumAlphas > 17) {
            thisChiller.ChillerCondLoopFlowFLoopPLRIndex = Curve::GetCurveIndex(state, s_ipsc->cAlphaArgs(18));
        }
        if ((thisChiller.ChillerCondLoopFlowFLoopPLRIndex == 0) &&
            (thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::ModulatedLoopPLR)) {
            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(18), s_ipsc->cAlphaArgs(18)));
            ErrorsFound = true;
        }

        if (NumAlphas <= 18 || s_ipsc->lAlphaFieldBlanks(19)) {
            thisChiller.condDTSched = nullptr; // Okay for this schedule to remain nullptr if field is empty
        } else if (((thisChiller.condDTSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(19))) == nullptr) &&
                   thisChiller.CondenserFlowControl == DataPlant::CondenserFlowControl::ModulatedDeltaTemperature) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(19), s_ipsc->cAlphaArgs(19));
            ErrorsFound = true;
        }

        if (NumNums > 18) {
            thisChiller.MinCondFlowRatio = s_ipsc->rNumericArgs(19);
        }

        //   Check the CAP-FT, EIR-FT, and PLR curves and warn user if different from 1.0 by more than +-10%
        if (thisChiller.ChillerCapFTIndex > 0) {
            Real64 CurveVal = Curve::CurveValue(state, thisChiller.ChillerCapFTIndex, thisChiller.TempRefEvapOut, thisChiller.TempRefCondIn);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state, "Capacity ratio as a function of temperature curve output is not equal to 1.0 (+ or - 10%) at reference conditions.");
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }
        }

        if (thisChiller.ChillerEIRFTIndex > 0) {
            Real64 CurveVal = Curve::CurveValue(state, thisChiller.ChillerEIRFTIndex, thisChiller.TempRefEvapOut, thisChiller.TempRefCondIn);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state, "Energy input ratio as a function of temperature curve output is not equal to 1.0 (+ or - 10%) at reference conditions.");
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }
        }

        if (thisChiller.ChillerEIRFPLRIndex > 0) {
            Real64 CurveVal = Curve::CurveValue(state, thisChiller.ChillerEIRFPLRIndex, 1.0);

            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state,
                    "Energy input ratio as a function of part-load ratio curve output is not equal to 1.0 (+ or - 10%) at reference conditions.");
                ShowContinueError(state, format("Curve output at reference conditions = {:.3T}", CurveVal));
            }
        }

        if (thisChiller.ChillerEIRFPLRIndex > 0) {
            bool FoundNegValue = false;
            Array1D<Real64> CurveValArray(11); // Used to evaluate PLFFPLR curve objects
            for (int CurveCheck = 0; CurveCheck <= 10; ++CurveCheck) {
                Real64 CurveValTmp = Curve::CurveValue(state, thisChiller.ChillerEIRFPLRIndex, double(CurveCheck / 10.0));
                if (CurveValTmp < 0.0) {
                    FoundNegValue = true;
                }
                CurveValArray(CurveCheck + 1) = int(CurveValTmp * 100.0) / 100.0;
            }
            if (FoundNegValue) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Energy input ratio as a function of part-load ratio curve shows negative values.");
                ShowContinueError(state, "EIR as a function of PLR curve output at various part-load ratios shown below:");
                ShowContinueError(state, "PLR          =    0.00   0.10   0.20   0.30   0.40   0.50   0.60   0.70   0.80   0.90   1.00");
                ShowContinueError(state, fmt::format("Curve Output = {:7.2F}", fmt::join(CurveValArray, ",")));
                ErrorsFound = true;
            }
        }
        //   Basin heater power as a function of temperature must be greater than or equal to 0
        thisChiller.BasinHeaterPowerFTempDiff = s_ipsc->rNumericArgs(16);
        if (s_ipsc->rNumericArgs(16) < 0.0) {
            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} must be >= 0", s_ipsc->cNumericFieldNames(16)));
            ErrorsFound = true;
        }

        thisChiller.BasinHeaterSetPointTemp = s_ipsc->rNumericArgs(17);

        if (thisChiller.BasinHeaterPowerFTempDiff > 0.0) {
            if (NumNums < 17) {
                thisChiller.BasinHeaterSetPointTemp = 2.0;
            }
            if (thisChiller.BasinHeaterSetPointTemp < 2.0) {
                ShowWarningError(state, format("{}{} \"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("{} is less than 2 deg C. Freezing could occur.", s_ipsc->cNumericFieldNames(17)));
            }
        }

        if (!s_ipsc->lAlphaFieldBlanks(13)) {
            if ((thisChiller.basinHeaterSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(13))) == nullptr) {
                ShowWarningItemNotFound(state,
                                        eoh,
                                        s_ipsc->cAlphaFieldNames(13),
                                        s_ipsc->cAlphaArgs(13),
                                        "Basin heater operation will not be modeled and the simulation continues");
            }
        }

        if (NumAlphas > 15) {
            thisChiller.EndUseSubcategory = s_ipsc->cAlphaArgs(16);
        } else {
            thisChiller.EndUseSubcategory = "General";
        }
        if (!s_ipsc->lAlphaFieldBlanks(20)) {
            thisChiller.thermosiphonTempCurveIndex = Curve::GetCurveIndex(state, Util::makeUPPER(s_ipsc->cAlphaArgs(20)));
            if (thisChiller.thermosiphonTempCurveIndex == 0) {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, thisChiller.Name));
                ShowContinueError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(20), s_ipsc->cAlphaArgs(20)));
                ErrorsFound = true;
            }
        }
        thisChiller.thermosiphonMinTempDiff = s_ipsc->rNumericArgs(20);
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", s_ipsc->cCurrentModuleObject));
    }
}

void ElectricEIRChillerSpecs::setupOutputVars(EnergyPlusData &state)
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
                        "Thermosiphon Status",
                        Constant::Units::None,
                        this->thermosiphonStatus,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    // Condenser mass flow and outlet temp are valid for water cooled
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
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

        // If heat recovery is active then setup report variables
        if (this->HeatRecActive) {
            SetupOutputVariable(state,
                                "Chiller Total Recovered Heat Rate",
                                Constant::Units::W,
                                this->QHeatRecovered,
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
                                OutputProcessor::EndUseCat::HeatRecovery);

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

    } else {
        SetupOutputVariable(state,
                            "Chiller Condenser Inlet Temperature",
                            Constant::Units::C,
                            this->CondInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        if (this->CondenserFanPowerRatio > 0) {
            SetupOutputVariable(state,
                                "Chiller Condenser Fan Electricity Rate",
                                Constant::Units::W,
                                this->CondenserFanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Chiller Condenser Fan Electricity Energy",
                                Constant::Units::J,
                                this->CondenserFanEnergyConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Cooling);
        }
        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            SetupOutputVariable(state,
                                "Chiller Evaporative Condenser Water Volume",
                                Constant::Units::m3,
                                this->EvapWaterConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);

            SetupOutputVariable(state,
                                "Chiller Evaporative Condenser Mains Supply Water Volume",
                                Constant::Units::m3,
                                this->EvapWaterConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);

            if (this->BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "Chiller Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    this->BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);

                SetupOutputVariable(state,
                                    "Chiller Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    this->BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    this->Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Plant,
                                    OutputProcessor::EndUseCat::Chillers);
            }
        }
    }
    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Chiller Nominal Capacity", this->Name, "[W]", this->RefCap);
    }
}

void ElectricEIRChillerSpecs::oneTimeInit(EnergyPlusData &state)
{
    this->setupOutputVars(state);

    // Locate the chillers on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state,
                                            this->Name,
                                            DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
                                            this->CWPlantLoc,
                                            errFlag,
                                            this->TempLowLimitEvapOut,
                                            _,
                                            _,
                                            this->EvapInletNodeNum,
                                            _);
    if (this->CondenserType != DataPlant::CondenserType::AirCooled && this->CondenserType != DataPlant::CondenserType::EvapCooled) {
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Chiller_ElectricEIR, this->CDPlantLoc, errFlag, _, _, _, this->CondInletNodeNum, _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->CDPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricEIR, true);
    }
    if (this->HeatRecActive) {
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Chiller_ElectricEIR, this->HRPlantLoc, errFlag, _, _, _, this->HeatRecInletNodeNum, _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricEIR, true);
    }

    if (this->CondenserType != DataPlant::CondenserType::AirCooled && this->CondenserType != DataPlant::CondenserType::EvapCooled &&
        this->HeatRecActive) {
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CDPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricEIR, false);
    }

    if (errFlag) {
        ShowFatalError(state, "InitElectricEIRChiller: Program terminated due to previous condition(s).");
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

void ElectricEIRChillerSpecs::initEachEnvironment(EnergyPlusData &state)
{

    static constexpr std::string_view RoutineName("ElectricEIRChillerSpecs::initEachEnvironment");

    Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

    this->EvapMassFlowRateMax = this->EvapVolFlowRate * rho;

    PlantUtilities::InitComponentNodes(state, 0.0, this->EvapMassFlowRateMax, this->EvapInletNodeNum, this->EvapOutletNodeNum);

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {

        rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
        this->CondMassFlowRateMax = rho * this->CondVolFlowRate;
        PlantUtilities::InitComponentNodes(state, 0.0, this->CondMassFlowRateMax, this->CondInletNodeNum, this->CondOutletNodeNum);
        state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempRefCondIn;
    } else { // air or evap air condenser
        // Initialize maximum available condenser flow rate
        rho = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, this->TempRefCondIn, 0.0, RoutineName);
        this->CondMassFlowRateMax = rho * this->CondVolFlowRate;

        state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate = this->CondMassFlowRateMax;
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
        rho = this->HRPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
        this->DesignHeatRecMassFlowRate = rho * this->DesignHeatRecVolFlowRate;

        PlantUtilities::InitComponentNodes(state, 0.0, this->DesignHeatRecMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);
        // overall capacity limit
        this->HeatRecMaxCapacityLimit = this->HeatRecCapacityFraction * (this->RefCap + this->RefCap / this->RefCOP);

        if (this->HeatRecSetPointNodeNum > 0) {
            Real64 THeatRecSetPoint(0.0); // tests set point node for proper set point value
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
            if (THeatRecSetPoint == DataLoopNode::SensedNodeFlagValue) {
                if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    if (!this->HRSPErrDone) {
                        ShowWarningError(state, format("Missing heat recovery temperature setpoint for chiller named {}", this->Name));
                        ShowContinueError(state,
                                          "  A temperature setpoint is needed at the heat recovery leaving temperature setpoint node "
                                          "specified, use a SetpointManager");
                        ShowContinueError(state, "  The overall loop setpoint will be assumed for heat recovery. The simulation continues ...");
                        this->HeatRecSetPointNodeNum = this->HRPlantLoc.loop->TempSetPointNodeNum;
                        this->HRSPErrDone = true;
                    }
                } else {
                    // need call to EMS to check node
                    bool fatalError = false; // but not really fatal yet, but should be.
                    EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->EvapOutletNodeNum, HVAC::CtrlVarType::Temp, fatalError);
                    state.dataLoopNodes->NodeSetpointCheck(this->EvapOutletNodeNum).needsSetpointChecking = false;
                    if (fatalError) {
                        if (!this->HRSPErrDone) {
                            ShowWarningError(state, format("Missing heat recovery temperature setpoint for chiller named {}", this->Name));
                            ShowContinueError(state,
                                              "  A temperature setpoint is needed at the heat recovery leaving temperature setpoint node "
                                              "specified, use a SetpointManager to establish a setpoint");
                            ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at this node ");
                            ShowContinueError(state, "  The overall loop setpoint will be assumed for heat recovery. The simulation continues ...");
                            this->HeatRecSetPointNodeNum = this->HRPlantLoc.loop->TempSetPointNodeNum;
                            this->HRSPErrDone = true;
                        }
                    }
                } // IF (.NOT. AnyEnergyManagementSystemInModel) THEN
            } // IF(THeatRecSetPoint == SensedNodeFlagValue)THEN
        } // IF(ElectricEIRChiller(EIRChillNum)%HeatRecSetPointNodeNum > 0)THEN
    } // IF (ElectricEIRChiller(EIRChillNum)%HeatRecActive) THEN
}

void ElectricEIRChillerSpecs::initialize(EnergyPlusData &state, bool const RunFlag, Real64 const MyLoad)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for initializations of the Electric EIR Chiller variables

    // METHODOLOGY EMPLOYED:
    //  Uses the status flags to trigger initializations.

    // Init more variables
    if (this->oneTimeFlag) {
        this->oneTimeInit(state);
        this->oneTimeFlag = false;
    }

    this->EquipFlowCtrl = DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowCtrl;

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        this->initEachEnvironment(state);
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
        // get minimum condenser plant loop pump mass flow rate
        this->VSBranchPumpMinLimitMassFlowCond =
            PlantUtilities::MinFlowIfBranchHasVSPump(state, this->CDPlantLoc, this->VSBranchPumpFoundCond, this->VSLoopPumpFoundCond, false);
    }
    // Initialize heat recovery flow rates at node
    if (this->HeatRecActive) {
        mdot = RunFlag ? this->DesignHeatRecMassFlowRate : 0.0; // if RunFlag is true, mdot = this->DesignHeatRecMassFlowRate, else mdot = 0.0
        PlantUtilities::SetComponentFlowRate(state, mdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
    }

    if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
        this->BasinHeaterPower = 0.0;
    }
}

void ElectricEIRChillerSpecs::size(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2004
    //       MODIFIED       October 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for sizing Electric EIR Chiller Components for which capacities and flow rates
    //  have not been specified in the input.

    // METHODOLOGY EMPLOYED:
    //  Obtains evaporator flow rate from the plant sizing array. Calculates reference capacity from
    //  the evaporator flow rate and the chilled water loop design delta T. The condenser flow rate
    //  is calculated from the reference capacity, the COP, and the condenser loop design delta T.

    static constexpr std::string_view RoutineName("SizeElectricEIRChiller");

    int PltSizCondNum = 0;
    bool ErrorsFound = false;
    Real64 tmpNomCap = this->RefCap;
    Real64 tmpEvapVolFlowRate = this->EvapVolFlowRate;
    Real64 tmpCondVolFlowRate = this->CondVolFlowRate;

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
                    BaseSizer::reportSizerOutput(
                        state, "Chiller:Electric:EIR", this->Name, "Design Size Reference Chilled Water Flow Rate [m3/s]", tmpEvapVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:EIR",
                                                 this->Name,
                                                 "Initial Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
            } else { // Hard-size with sizing data
                if (this->EvapVolFlowRate > 0.0 && tmpEvapVolFlowRate > 0.0) {
                    Real64 EvapVolFlowRateUser = this->EvapVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:EIR",
                                                     this->Name,
                                                     "Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                     tmpEvapVolFlowRate,
                                                     "User-Specified Reference Chilled Water Flow Rate [m3/s]",
                                                     EvapVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpEvapVolFlowRate - EvapVolFlowRateUser) / EvapVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerElectricEIR: Potential issue with equipment sizing for {}", this->Name));
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
            ShowSevereError(state, "Autosizing of Electric Chiller evap flow rate requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->EvapVolFlowRate > 0.0)) {
            BaseSizer::reportSizerOutput(
                state, "Chiller:Electric:EIR", this->Name, "User-Specified Reference Chilled Water Flow Rate [m3/s]", this->EvapVolFlowRate);
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->EvapInletNodeNum, tmpEvapVolFlowRate);

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);

            Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            tmpNomCap = Cp * rho * state.dataSize->PlantSizData(PltSizNum).DeltaT * tmpEvapVolFlowRate;
        } else {
            tmpNomCap = 0.0;
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->RefCapWasAutoSized) {
                this->RefCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, "Chiller:Electric:EIR", this->Name, "Design Size Reference Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, "Chiller:Electric:EIR", this->Name, "Initial Design Size Reference Capacity [W]", tmpNomCap);
                }
            } else { // Hard-sized with sizing data
                if (this->RefCap > 0.0 && tmpNomCap > 0.0) {
                    Real64 RefCapUser = this->RefCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:EIR",
                                                     this->Name,
                                                     "Design Size Reference Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Reference Capacity [W]",
                                                     RefCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - RefCapUser) / RefCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerElectricEIR: Potential issue with equipment sizing for {}", this->Name));
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
            ShowSevereError(state, "Autosizing of Electric Chiller reference capacity requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->RefCapWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->RefCap > 0.0)) { // Hard-sized with no sizing data
            BaseSizer::reportSizerOutput(state, "Chiller:Electric:EIR", this->Name, "User-Specified Reference Capacity [W]", this->RefCap);
        }
    }

    if (PltSizCondNum > 0 && PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {

            Real64 rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
            Real64 Cp = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, this->TempRefCondIn, RoutineName);
            tmpCondVolFlowRate = tmpNomCap * (1.0 + (1.0 / this->RefCOP) * this->CompPowerToCondenserFrac) /
                                 (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);

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
                        state, "Chiller:Electric:EIR", this->Name, "Design Size Reference Condenser Fluid Flow Rate [m3/s]", tmpCondVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "Chiller:Electric:EIR",
                                                 this->Name,
                                                 "Initial Design Size Reference Condenser Fluid Flow Rate [m3/s]",
                                                 tmpCondVolFlowRate);
                }
            } else {
                if (this->CondVolFlowRate > 0.0 && tmpCondVolFlowRate > 0.0) {
                    Real64 CondVolFlowRateUser = this->CondVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:EIR",
                                                     this->Name,
                                                     "Design Size Reference Condenser Fluid Flow Rate [m3/s]",
                                                     tmpCondVolFlowRate,
                                                     "User-Specified Reference Condenser Fluid Flow Rate [m3/s]",
                                                     CondVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpCondVolFlowRate - CondVolFlowRateUser) / CondVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeChillerElectricEIR: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Reference Condenser Fluid Flow Rate of {:.5R} [m3/s]", CondVolFlowRateUser));
                                ShowContinueError(
                                    state,
                                    format("differs from Design Size Reference Condenser Fluid Flow Rate of {:.5R} [m3/s]", tmpCondVolFlowRate));
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
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {

            if (this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "Autosizing of Electric EIR Chiller condenser fluid flow rate requires a condenser");
                ShowContinueError(state, "loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in Electric EIR Chiller object={}", this->Name));
                ErrorsFound = true;
            }
            if (!this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->CondVolFlowRate > 0.0)) {
                BaseSizer::reportSizerOutput(
                    state, "Chiller:Electric:EIR", this->Name, "User-Specified Reference Condenser Fluid Flow Rate [m3/s]", this->CondVolFlowRate);
            }

        } else {

            // Auto size condenser air flow to Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                std::string_view CompType = DataPlant::PlantEquipTypeNames[static_cast<int>(DataPlant::PlantEquipmentType::Chiller_ElectricEIR)];
                state.dataSize->DataConstantUsedForSizing = this->RefCap;
                state.dataSize->DataFractionUsedForSizing = 0.000114;
                Real64 TempSize = this->CondVolFlowRate;
                bool bPRINT = true; // TRUE if sizing is reported to output (eio)
                AutoCalculateSizer sizerCondAirFlow;
                std::string stringOverride = "Reference Condenser Fluid Flow Rate  [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "reference_condenser_fluid_flow_rate [m3/s]";
                }
                sizerCondAirFlow.overrideSizingString(stringOverride);
                sizerCondAirFlow.initializeWithinEP(state, CompType, this->Name, bPRINT, RoutineName);
                this->CondVolFlowRate = sizerCondAirFlow.size(state, TempSize, ErrorsFound);
                tmpCondVolFlowRate = this->CondVolFlowRate;
            }
        }
    }

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        // save the reference condenser water volumetric flow rate for use by the condenser water loop sizing algorithms
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondInletNodeNum, tmpCondVolFlowRate);
    }

    // now do heat recovery flow rate sizing if active
    if (this->HeatRecActive) {
        Real64 tempHeatRecVolFlowRate;
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            tempHeatRecVolFlowRate = tmpCondVolFlowRate * this->HeatRecCapacityFraction;
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tempHeatRecVolFlowRate = tmpEvapVolFlowRate;
            } else {
                tempHeatRecVolFlowRate = this->EvapVolFlowRate;
            }
            tempHeatRecVolFlowRate *= (1.0 + (1.0 / this->RefCOP)) * this->CompPowerToCondenserFrac * this->HeatRecCapacityFraction;
        }
        if (this->DesignHeatRecVolFlowRateWasAutoSized) {

            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->DesignHeatRecVolFlowRate = tempHeatRecVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Chiller:Electric:EIR", this->Name, "Design Size Heat Recovery Water Flow Rate [m3/s]", tempHeatRecVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Chiller:Electric:EIR", this->Name, "Intial Design Size Heat Recovery Water Flow Rate [m3/s]", tempHeatRecVolFlowRate);
                }
            }
        } else {
            if (this->DesignHeatRecVolFlowRate > 0.0 && tempHeatRecVolFlowRate > 0.0) {
                Real64 nomHeatRecVolFlowRateUser = this->DesignHeatRecVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    if (state.dataGlobal->DoPlantSizing) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:EIR",
                                                     this->Name,
                                                     "Design Size Heat Recovery Water Flow Rate [m3/s]",
                                                     tempHeatRecVolFlowRate,
                                                     "User-Specified Heat Recovery Water Flow Rate [m3/s]",
                                                     nomHeatRecVolFlowRateUser);
                    } else {
                        BaseSizer::reportSizerOutput(state,
                                                     "Chiller:Electric:EIR",
                                                     this->Name,
                                                     "User-Specified Heat Recovery Water Flow Rate [m3/s]",
                                                     nomHeatRecVolFlowRateUser);
                    }

                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(tempHeatRecVolFlowRate - nomHeatRecVolFlowRateUser) / nomHeatRecVolFlowRateUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizeChillerElectricEIR: Potential issue with equipment sizing for {}", this->Name));
                            ShowContinueError(state,
                                              format("User-Specified Heat Recovery Water Flow Rate of {:.5R} [m3/s]", nomHeatRecVolFlowRateUser));
                            ShowContinueError(
                                state, format("differs from Design Size Heat Recovery Water Flow Rate of {:.5R} [m3/s]", tempHeatRecVolFlowRate));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
                tempHeatRecVolFlowRate = nomHeatRecVolFlowRateUser;
            }
        }
        if (!this->DesignHeatRecVolFlowRateWasAutoSized) {
            tempHeatRecVolFlowRate = this->DesignHeatRecVolFlowRate;
        }
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->HeatRecInletNodeNum, tempHeatRecVolFlowRate);
    } // Heat recovery active

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {

        Real64 IPLVSI_rpt_std229 = 0.0;
        Real64 IPLVIP_rpt_std229 = 0.0;

        if (this->IPLVFlag) {
            Real64 IPLVSI = 0.0;
            Real64 IPLVIP = 0.0;
            StandardRatings::CalcChillerIPLV(state,
                                             this->Name,
                                             DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
                                             this->RefCap,
                                             this->RefCOP,
                                             this->CondenserType,
                                             this->ChillerCapFTIndex,
                                             this->ChillerEIRFTIndex,
                                             this->ChillerEIRFPLRIndex,
                                             this->MinUnloadRat,
                                             IPLVSI,
                                             IPLVIP,
                                             ObjexxFCL::Optional<const Real64>(),
                                             ObjexxFCL::Optional_int_const(),
                                             ObjexxFCL::Optional<const Real64>());

            IPLVSI_rpt_std229 = IPLVSI;
            IPLVIP_rpt_std229 = IPLVIP;

            this->IPLVFlag = false;
        }
        // create predefined report
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, this->Name, "Chiller:Electric:EIR");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, this->Name, this->RefCOP);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->RefCap);

        // std 229 new Chillers table
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerType, this->Name, "Chiller:Electric:EIR");
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
                                                 (this->CDPlantLoc.loop != nullptr) ? this->CDPlantLoc.branch->Name : "N/A");
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
                                                 (this->HRPlantLoc.loop != nullptr) ? this->HRPlantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRecRelCapFrac, this->Name, this->HeatRecCapacityFraction);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void ElectricEIRChillerSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   July 2004
    //       MODIFIED       Feb. 2010, Chandan Sharma, FSEC, Added basin heater
    //                      Jun. 2016, Rongpeng Zhang, Applied the chiller supply water temperature sensor fault model
    //                      Nov. 2016, Rongpeng Zhang, LBNL. Added Fouling Chiller fault

    // PURPOSE OF THIS SUBROUTINE:
    //  Simulate a vapor compression chiller using the DOE-2 model

    // METHODOLOGY EMPLOYED:
    //  Use empirical curve fits to model performance at off-reference conditions

    // REFERENCES:
    // 1. DOE-2 Engineers Manual, Version 2.1A, November 1982, LBL-11353

    static constexpr std::string_view RoutineName("CalcElectricEIRChillerModel");

    Real64 EvapOutletTempSetPoint(0.0); // Evaporator outlet temperature setpoint [C]
    Real64 EvapDeltaTemp(0.0);          // Evaporator temperature difference [C]
    Real64 TempLoad(0.0);               // Actual load to be met by chiller. This value is compared to MyLoad
    // and reset when necessary since this chiller can cycle, the load passed
    // should be the actual load. Instead the minimum PLR * RefCap is
    // passed in. [W]
    Real64 CurrentEndTime = 0.0; // end time of time step for current simulation time step

    // Set module level inlet and outlet nodes and initialize other local variables
    this->CondMassFlowRate = 0.0;
    Real64 FRAC = 1.0; // Chiller cycling ratio

    // Set performance curve outputs to 0.0 when chiller is off
    this->ChillerCapFT = 0.0;
    this->ChillerEIRFT = 0.0;
    this->ChillerEIRFPLR = 0.0;
    this->thermosiphonStatus = 0;

    // calculate end time of current time step
    CurrentEndTime = state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed;

    // Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
    // Wait for next time step to print warnings. If simulation iterates, print out
    // the warning for the last iteration only. Must wait for next time step to accomplish this.
    // If a warning occurs and the simulation down shifts, the warning is not valid.
    if (CurrentEndTime > this->CurrentEndTimeLast && state.dataHVACGlobal->TimeStepSys >= this->TimeStepSysLast) {
        if (this->PrintMessage) {
            ++this->MsgErrorCount;
            //     Show single warning and pass additional info to ShowRecurringWarningErrorAtEnd
            if (this->MsgErrorCount < 2) {
                ShowWarningError(state, format("{}.", this->MsgBuffer1));
                ShowContinueError(state, this->MsgBuffer2);
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state, this->MsgBuffer1 + " error continues.", this->ErrCount1, this->MsgDataLast, this->MsgDataLast, _, "[C]", "[C]");
            }
        }
    }

    // save last system time step and last end time of current time step (used to determine if warning is valid)
    this->TimeStepSysLast = state.dataHVACGlobal->TimeStepSys;
    this->CurrentEndTimeLast = CurrentEndTime;

    // If no loop demand or chiller OFF, return
    // If Chiller load is 0 or chiller is not running then leave the subroutine.Before leaving
    // if the component control is SERIESACTIVE we set the component flow to inlet flow so that
    // flow resolver will not shut down the branch
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
        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
        }
        this->PrintMessage = false;
        return;
    }

    // initialize outlet air humidity ratio of air or evap cooled chillers
    this->CondOutletHumRat = state.dataLoopNodes->Node(this->CondInletNodeNum).HumRat;

    if (this->CondenserType == DataPlant::CondenserType::AirCooled) { // Condenser inlet temp = outdoor temp
        state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).OutAirDryBulb;

        // Warn user if entering condenser dry-bulb temperature falls below 0 C
        if (state.dataLoopNodes->Node(this->CondInletNodeNum).Temp < 0.0 && std::abs(MyLoad) > 0 && RunFlag && !state.dataGlobal->WarmupFlag) {
            this->PrintMessage = true;

            this->MsgBuffer1 =
                "ElectricEIRChillerModel - CHILLER:ELECTRIC:EIR \"" + this->Name + "\" - Air Cooled Condenser Inlet Temperature below 0C";
            this->MsgBuffer2 = format("... Outdoor Dry-bulb Condition = {:6.2F} C. Occurrence info = {}, {} {}",
                                      state.dataLoopNodes->Node(this->CondInletNodeNum).Temp,
                                      state.dataEnvrn->EnvironmentName,
                                      state.dataEnvrn->CurMnDy,
                                      General::CreateSysTimeIntervalString(state));

            this->MsgDataLast = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        } else {
            this->PrintMessage = false;
        }
    } else if (this->CondenserType == DataPlant::CondenserType::EvapCooled) { // Condenser inlet temp = (outdoor wet bulb)
        state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).OutAirWetBulb;
        //  line above assumes evaporation pushes condenser inlet air humidity ratio to saturation
        this->CondOutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state,
                                                                state.dataLoopNodes->Node(this->CondInletNodeNum).Temp,
                                                                state.dataLoopNodes->Node(this->CondInletNodeNum).Temp,
                                                                state.dataLoopNodes->Node(this->CondInletNodeNum).Press);

        // Warn user if evap condenser wet-bulb temperature falls below 10 C
        if (state.dataLoopNodes->Node(this->CondInletNodeNum).Temp < 10.0 && std::abs(MyLoad) > 0 && RunFlag && !state.dataGlobal->WarmupFlag) {
            this->PrintMessage = true;
            this->MsgBuffer1 =
                "ElectricEIRChillerModel - CHILLER:ELECTRIC:EIR \"" + this->Name + "\" - Air Cooled Condenser Inlet Temperature below 10C";
            this->MsgBuffer2 = format("... Outdoor Wet-bulb Condition = {:6.2F} C. Occurrence info = {}, {} {}",
                                      state.dataLoopNodes->Node(this->CondInletNodeNum).Temp,
                                      state.dataEnvrn->EnvironmentName,
                                      state.dataEnvrn->CurMnDy,
                                      General::CreateSysTimeIntervalString(state));
            this->MsgDataLast = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        } else {
            this->PrintMessage = false;
        }
    } // End of the Air Cooled/Evap Cooled Logic block

    // If not air or evap cooled then set to the condenser node that is attached to a cooling tower
    Real64 condInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
    this->CondInletTemp = condInletTemp; // needed for thermosiphon model

    // LOAD LOCAL VARIABLES FROM DATA STRUCTURE (for code readability)
    Real64 ChillerRefCap = this->RefCap;
    Real64 ReferenceCOP = this->RefCOP;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
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
            // Shut chiller off if there is no condenser water flow
            MyLoad = 0.0;
            this->EvapMassFlowRate = 0.0;
            // Use PlantUtilities::SetComponentFlowRate to decide actual flow
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            return;
        }
    }

    switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        }
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
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
    Real64 AvgCondSinkTemp = condInletTemp;
    if (this->HeatRecActive) {
        if ((this->QHeatRecovered + this->QCondenser) > 0.0) { // protect div by zero
            AvgCondSinkTemp =
                (this->QHeatRecovered * this->HeatRecInletTemp + this->QCondenser * this->CondInletTemp) / (this->QHeatRecovered + this->QCondenser);
        } else {
            AvgCondSinkTemp = condInletTemp;
        }
    }

    // Get capacity curve info with respect to CW setpoint and entering condenser water temps
    this->ChillerCapFT = Curve::CurveValue(state, this->ChillerCapFTIndex, EvapOutletTempSetPoint, AvgCondSinkTemp);

    if (this->ChillerCapFT < 0) {
        if (this->ChillerCapFTError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerCapFTError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:EIR \"{}\":", this->Name));
            ShowContinueError(state, format(" Chiller Capacity as a Function of Temperature curve output is negative ({:.3R}).", this->ChillerCapFT));
            ShowContinueError(state,
                              format(" Negative value occurs using an Evaporator Outlet Temp of {:.1R} and a Condenser Inlet Temp of {:.1R}.",
                                     EvapOutletTempSetPoint,
                                     condInletTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerCapFTError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:EIR \"" + this->Name +
                                               "\": Chiller Capacity as a Function of Temperature curve output is negative warning continues...",
                                           this->ChillerCapFTErrorIndex,
                                           this->ChillerCapFT,
                                           this->ChillerCapFT);
        }
        this->ChillerCapFT = 0.0;
    }

    // Available chiller capacity as a function of temperature
    Real64 AvailChillerCap = ChillerRefCap * this->ChillerCapFT;

    // Only perform this check for temperature setpoint control
    if (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) {
        // Calculate water side load

        Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, RoutineName);
        this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            TempLoad = this->EvapMassFlowRate * Cp *
                       (state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint);
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            TempLoad = this->EvapMassFlowRate * Cp *
                       (state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi);
        } break;
        default: {
            assert(false);
        } break;
        }
        TempLoad = max(0.0, TempLoad);

        // MyLoad is capped at minimum PLR * RefCap, adjust load to actual water side load because this chiller can cycle
        if (std::abs(MyLoad) > TempLoad) {
            MyLoad = sign(TempLoad, MyLoad);
        }
    }

    // Part load ratio based on load and available chiller capacity, cap at max part load ratio
    Real64 PartLoadRat = 0.0; // Operating part load ratio
    if (AvailChillerCap > 0) {
        PartLoadRat = max(0.0, min(std::abs(MyLoad) / AvailChillerCap, this->MaxPartLoadRat));
    }

    Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, RoutineName);

    if (DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) {
        this->PossibleSubcooling = false;
    } else {
        this->PossibleSubcooling = true;
    }
    // Set evaporator heat transfer rate
    this->QEvaporator = AvailChillerCap * PartLoadRat;

    // Either set the flow to the Constant value or calculate the flow for the variable volume
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
        // Evaluate outlet temp based on delta
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;

    } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {

        // Calculate the Delta Temp from the inlet temp to the chiller outlet setpoint
        switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
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
            // Calculate desired flow to request based on load
            this->EvapMassFlowRate = std::abs(this->QEvaporator / Cp / EvapDeltaTemp);
            if ((this->EvapMassFlowRate - this->EvapMassFlowRateMax) > DataBranchAirLoopPlant::MassFlowTolerance) {
                this->PossibleSubcooling = true;
            }
            // Check to see if the Maximum is exceeded, if so set to maximum
            this->EvapMassFlowRate = min(this->EvapMassFlowRateMax, this->EvapMassFlowRate);
            // Use PlantUtilities::SetComponentFlowRate to decide actual flow
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
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
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            // No deltaT since component is not running
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
            this->QEvaporator = 0.0;
            PartLoadRat = 0.0;
            this->ChillerPartLoadRatio = PartLoadRat;

            // so what if the delta T is zero?  On FlowLock==0, the inlet temp could = setpoint, right?
            if (this->DeltaTErrCount < 1 && !state.dataGlobal->WarmupFlag) {
                ++this->DeltaTErrCount;
                ShowWarningError(state, "Evaporator DeltaTemp = 0 in mass flow calculation (Tevapin = Tsetpoint).");
                ShowContinueErrorTimeStamp(state, "");
            } else if (!state.dataGlobal->WarmupFlag) {
                ++this->ChillerCapFTError;
                ShowRecurringWarningErrorAtEnd(state,
                                               "CHILLER:ELECTRIC:EIR \"" + this->Name +
                                                   "\": Evaporator DeltaTemp = 0 in mass flow calculation warning continues...",
                                               this->DeltaTErrCountIndex,
                                               EvapDeltaTemp,
                                               EvapDeltaTemp);
            }
        }
    } // End of Constant Variable Flow If Block

    if (this->EvapMassFlowRate == 0.0) {
        MyLoad = 0.0;
        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
        }
        this->PrintMessage = false;
        return;
    }
    if (this->PossibleSubcooling) {
        this->QEvaporator = std::abs(MyLoad);
        EvapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapDeltaTemp;
    } else {
        EvapDeltaTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - EvapOutletTempSetPoint;
        this->QEvaporator = max(0.0, (this->EvapMassFlowRate * Cp * EvapDeltaTemp));
        this->EvapOutletTemp = EvapOutletTempSetPoint;
    }

    // Check that the Evap outlet temp honors both plant loop temp low limit and also the chiller low limit
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
    if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation) &&
        (this->EvapMassFlowRate > 0)) {
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

    // set the module level variable used for reporting FRAC
    this->ChillerCyclingRatio = FRAC;

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
    if (this->QEvaporator == 0.0 && this->CondenserType == DataPlant::CondenserType::EvapCooled) {
        CalcBasinHeaterPower(state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
    }

    this->ChillerEIRFT = Curve::CurveValue(state, this->ChillerEIRFTIndex, this->EvapOutletTemp, AvgCondSinkTemp);
    if (this->ChillerEIRFT < 0.0) {
        if (this->ChillerEIRFTError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFTError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:EIR \"{}\":", this->Name));
            ShowContinueError(state, format(" Chiller EIR as a Function of Temperature curve output is negative ({:.3R}).", this->ChillerEIRFT));
            ShowContinueError(state,
                              format(" Negative value occurs using an Evaporator Outlet Temp of {:.1R} and a Condenser Inlet Temp of {:.1R}.",
                                     this->EvapOutletTemp,
                                     condInletTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFTError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:EIR \"" + this->Name +
                                               "\": Chiller EIR as a Function of Temperature curve output is negative warning continues...",
                                           this->ChillerEIRFTErrorIndex,
                                           this->ChillerEIRFT,
                                           this->ChillerEIRFT);
        }
        this->ChillerEIRFT = 0.0;
    }

    this->ChillerEIRFPLR = Curve::CurveValue(state, this->ChillerEIRFPLRIndex, PartLoadRat);
    if (this->ChillerEIRFPLR < 0.0) {
        if (this->ChillerEIRFPLRError < 1 && this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFPLRError;
            ShowWarningError(state, format("CHILLER:ELECTRIC:EIR \"{}\":", this->Name));
            ShowContinueError(state, format(" Chiller EIR as a function of PLR curve output is negative ({:.3R}).", this->ChillerEIRFPLR));
            ShowContinueError(state, format(" Negative value occurs using a part-load ratio of {:.3R}.", PartLoadRat));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (this->CWPlantLoc.side->FlowLock != DataPlant::FlowLock::Unlocked && !state.dataGlobal->WarmupFlag) {
            ++this->ChillerEIRFPLRError;
            ShowRecurringWarningErrorAtEnd(state,
                                           "CHILLER:ELECTRIC:EIR \"" + this->Name +
                                               "\": Chiller EIR as a function of PLR curve output is negative warning continues...",
                                           this->ChillerEIRFPLRErrorIndex,
                                           this->ChillerEIRFPLR,
                                           this->ChillerEIRFPLR);
        }
        this->ChillerEIRFPLR = 0.0;
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
            Real64 condDT = 0.0;
            if (this->condDTSched != nullptr) {
                condDT = this->condDTSched->getCurrentVal();
            }
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

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        if (this->CondMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            // If Heat Recovery specified for this vapor compression chiller, then Qcondenser will be adjusted by this subroutine
            if (this->HeatRecActive) {
                this->calcHeatRecovery(state, this->QCondenser, this->CondMassFlowRate, condInletTemp, this->QHeatRecovered);
            }
            Real64 CpCond = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, condInletTemp, RoutineName);

            this->CondOutletTemp = this->QCondenser / this->CondMassFlowRate / CpCond + condInletTemp;
        } else {
            ShowSevereError(state, format("CalcElectricEIRChillerModel: Condenser flow = 0, for ElectricEIRChiller={}", this->Name));
            ShowContinueErrorTimeStamp(state, "");
            // maybe this could be handled earlier, check if this component has a load and an evap flow rate
            // then if cond flow is zero, just make a request to the condenser,
            // then just say it couldn't run until condenser loop wakes up.
        }
    } else { // Air Cooled or Evap Cooled

        if (this->QCondenser > 0.0) {
            this->CondMassFlowRate = this->CondMassFlowRateMax * PartLoadRat;
        } else {
            this->CondMassFlowRate = 0.0;
        }

        // If Heat Recovery specified for this vapor compression chiller, then Qcondenser will be adjusted by this subroutine
        if (this->HeatRecActive) {
            this->calcHeatRecovery(state, this->QCondenser, this->CondMassFlowRate, condInletTemp, this->QHeatRecovered);
        }

        if (CondMassFlowRate > 0.0) {
            Real64 CpCond = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->CondInletNodeNum).HumRat);
            CondOutletTemp = CondInletTemp + QCondenser / CondMassFlowRate / CpCond;
        } else {
            this->CondOutletTemp = condInletTemp;
        }

        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            Real64 const RhoWater = Psychrometrics::RhoH2O(Constant::InitConvTemp);
            // CondMassFlowRate is already multiplied by PLR, convert to water use rate
            this->EvapWaterConsumpRate =
                ((this->CondOutletHumRat - state.dataLoopNodes->Node(this->CondInletNodeNum).HumRat) * this->CondMassFlowRate) / RhoWater;
        }
    }

    // Calculate condenser fan power
    if (this->ChillerCapFT > 0.0) {
        this->CondenserFanPower = ChillerRefCap * this->CondenserFanPowerRatio * FRAC;
    } else {
        this->CondenserFanPower = 0.0;
    }
}

void ElectricEIRChillerSpecs::calcHeatRecovery(EnergyPlusData &state,
                                               Real64 &QCond,              // Current condenser load [W]
                                               Real64 const CondMassFlow,  // Current condenser mass flow [kg/s]
                                               Real64 const condInletTemp, // Current condenser inlet temp [C]
                                               Real64 &QHeatRec            // Amount of heat recovered [W]
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Richard Liesen
    //       DATE WRITTEN:    January 2004
    //       MODIFIED:        Richard Raustad, FSEC (occurrences of EIR only, calcs are identical to electric chiller)

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculate the heat recovered from the chiller condenser

    static constexpr std::string_view RoutineName("EIRChillerHeatRecovery");

    // Inlet node to the heat recovery heat exchanger
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
    } else {                          // use new algorithm to meet setpoint
        Real64 THeatRecSetPoint(0.0); // local value for heat recovery leaving setpoint [C]
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

        // load to heat recovery setpoint
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

void ElectricEIRChillerSpecs::update(EnergyPlusData &state, Real64 const MyLoad, bool const RunFlag)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Richard Raustad, FSEC
    //       DATE WRITTEN:    June 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  Reporting

    // Number of seconds per HVAC system time step, to convert from W (J/s) to J
    Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

    if (MyLoad >= 0 || !RunFlag) { // Chiller not running so pass inlet states to outlet states
        // Set node conditions
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        if (this->CondenserType != DataPlant::CondenserType::WaterCooled) {
            state.dataLoopNodes->Node(this->CondOutletNodeNum).HumRat = state.dataLoopNodes->Node(this->CondInletNodeNum).HumRat;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).Enthalpy = state.dataLoopNodes->Node(this->CondInletNodeNum).Enthalpy;
            state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRate = 0.0;
        }

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
        this->CondenserFanPower = 0.0;
        this->CondenserFanEnergyConsumption = 0.0;
        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            this->BasinHeaterConsumption = this->BasinHeaterPower * ReportingConstant;
            this->EvapWaterConsump = 0.0;
        }

        if (this->HeatRecActive) {

            PlantUtilities::SafeCopyPlantNode(state, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);

            this->QHeatRecovered = 0.0;
            this->EnergyHeatRecovery = 0.0;
            this->HeatRecInletTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
            this->HeatRecOutletTemp = state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp;
            this->HeatRecMassFlow = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;
        }

    } else { // Chiller is running, so pass calculated values
        // Set node temperatures
        if (this->CondMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance &&
            this->EvapMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
            if (this->CondenserType != DataPlant::CondenserType::WaterCooled) {
                state.dataLoopNodes->Node(this->CondOutletNodeNum).HumRat = state.dataLoopNodes->Node(this->CondInletNodeNum).HumRat;
                state.dataLoopNodes->Node(this->CondOutletNodeNum).Enthalpy = state.dataLoopNodes->Node(this->CondInletNodeNum).Enthalpy;
                state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRate = 0.0;
            }
        } else {
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = this->EvapOutletTemp;
            state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = this->CondOutletTemp;
            if (this->CondenserType != DataPlant::CondenserType::WaterCooled) {
                state.dataLoopNodes->Node(this->CondOutletNodeNum).HumRat = this->CondOutletHumRat;
                state.dataLoopNodes->Node(this->CondOutletNodeNum).Enthalpy =
                    Psychrometrics::PsyHFnTdbW(this->CondOutletTemp, this->CondOutletHumRat);
                state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate = this->CondMassFlowRate;
                state.dataLoopNodes->Node(this->CondOutletNodeNum).MassFlowRate = this->CondMassFlowRate;
            }
        }

        // Set node flow rates;  for these load based models
        // assume that sufficient evaporator flow rate is available
        this->ChillerFalseLoad = this->ChillerFalseLoadRate * state.dataHVACGlobal->TimeStepSysSec;
        this->Energy = this->Power * state.dataHVACGlobal->TimeStepSysSec;
        this->EvapEnergy = this->QEvaporator * state.dataHVACGlobal->TimeStepSysSec;
        this->CondEnergy = this->QCondenser * state.dataHVACGlobal->TimeStepSysSec;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->CondenserFanEnergyConsumption = this->CondenserFanPower * state.dataHVACGlobal->TimeStepSysSec;
        if (this->Power != 0.0) {
            this->ActualCOP = (this->QEvaporator + this->ChillerFalseLoadRate) / this->Power;
        } else {
            this->ActualCOP = 0.0;
        }
        if (this->CondenserType == DataPlant::CondenserType::EvapCooled) {
            this->BasinHeaterConsumption = this->BasinHeaterPower * ReportingConstant;
            this->EvapWaterConsump = this->EvapWaterConsumpRate * ReportingConstant;
        }

        if (this->HeatRecActive) {

            PlantUtilities::SafeCopyPlantNode(state, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);
            this->EnergyHeatRecovery = this->QHeatRecovered * state.dataHVACGlobal->TimeStepSysSec;
            state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp = this->HeatRecOutletTemp;
            this->HeatRecInletTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
            this->HeatRecMassFlow = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;
        }
    }
}

bool ElectricEIRChillerSpecs::thermosiphonDisabled(EnergyPlusData &state)
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

} // namespace EnergyPlus::ChillerElectricEIR
