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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantHeatExchangerFluidToFluid.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::PlantHeatExchangerFluidToFluid {

// Module containing the routines dealing with the HeatExchanger:FluidToFluid

// MODULE INFORMATION:
//       AUTHOR         B. Griffith, derived from legacy code by  Sankaranarayanan K P, and S. Rees
//       DATE WRITTEN   November 2012
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// Simulate a generic plant heat exchanger with a variety of control options

std::string const ComponentClassName("HeatExchanger:FluidToFluid");

[[maybe_unused]] constexpr std::array<std::string_view, (int)FluidHXType::Num> fluidHXTypeNames = {"CrossFlowBothUnMixed",
                                                                                                   "CrossFlowBothMixed",
                                                                                                   "CrossFlowSupplyMixedDemandUnMixed",
                                                                                                   "CrossFlowSupplyUnMixedDemandMixed",
                                                                                                   "CounterFlow",
                                                                                                   "ParallelFlow",
                                                                                                   "Ideal"};
constexpr std::array<std::string_view, (int)FluidHXType::Num> fluidHXTypeNamesUC = {"CROSSFLOWBOTHUNMIXED",
                                                                                    "CROSSFLOWBOTHMIXED",
                                                                                    "CROSSFLOWSUPPLYMIXEDDEMANDUNMIXED",
                                                                                    "CROSSFLOWSUPPLYUNMIXEDDEMANDMIXED",
                                                                                    "COUNTERFLOW",
                                                                                    "PARALLELFLOW",
                                                                                    "IDEAL"};

[[maybe_unused]] constexpr std::array<std::string_view, (int)ControlType::Num> controlTypeNames = {"UncontrolledOn",
                                                                                                   "OperationSchemeModulated",
                                                                                                   "OperationSchemeOnOff",
                                                                                                   "HeatingSetpointModulated",
                                                                                                   "HeatingSetpointOnOff",
                                                                                                   "CoolingSetpointModulated",
                                                                                                   "CoolingSetpointOnOff",
                                                                                                   "DualDeadbandSetpointModulated",
                                                                                                   "DualDeadbandSetpointOnOff",
                                                                                                   "CoolingDifferentialOnOff",
                                                                                                   "CoolingSetpointOnOffWithComponentOverride",
                                                                                                   "TrackComponentOnOff"};
constexpr std::array<std::string_view, (int)ControlType::Num> controlTypeNamesUC = {"UNCONTROLLEDON",
                                                                                    "OPERATIONSCHEMEMODULATED",
                                                                                    "OPERATIONSCHEMEONOFF",
                                                                                    "HEATINGSETPOINTMODULATED",
                                                                                    "HEATINGSETPOINTONOFF",
                                                                                    "COOLINGSETPOINTMODULATED",
                                                                                    "COOLINGSETPOINTONOFF",
                                                                                    "DUALDEADBANDSETPOINTMODULATED",
                                                                                    "DUALDEADBANDSETPOINTONOFF",
                                                                                    "COOLINGDIFFERENTIALONOFF",
                                                                                    "COOLINGSETPOINTONOFFWITHCOMPONENTOVERRIDE",
                                                                                    "TRACKCOMPONENTONOFF"};

[[maybe_unused]] constexpr std::array<std::string_view, (int)CtrlTempType::Num> ctrlTempTypeNames = {
    "WetBulbTemperature", "DryBulbTemperature", "Loop"};
constexpr std::array<std::string_view, (int)CtrlTempType::Num> ctrlTempTypeNamesUC = {"WETBULBTEMPERATURE", "DRYBULBTEMPERATURE", "LOOP"};

PlantComponent *HeatExchangerStruct::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data for heat exchangers if it hasn't been done already
    if (state.dataPlantHXFluidToFluid->GetInput) {
        GetFluidHeatExchangerInput(state);
        state.dataPlantHXFluidToFluid->GetInput = false;
    }
    // Now look for this particular object
    for (auto &obj : state.dataPlantHXFluidToFluid->FluidHX) {
        if (obj.Name == objectName) {
            return &obj;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalPlantFluidHXFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void HeatExchangerStruct::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    this->initialize(state);
}

void HeatExchangerStruct::getDesignCapacities(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    if (calledFromLocation.loopNum == this->DemandSideLoop.loopNum) {
        MinLoad = 0.0;
        MaxLoad = this->DemandSideLoop.MaxLoad;
        OptLoad = this->DemandSideLoop.MaxLoad * 0.9;
    } else if (calledFromLocation.loopNum == this->SupplySideLoop.loopNum) {
        this->size(state); // only call sizing from the loop that sizes are based on
        MinLoad = 0.0;
        MaxLoad = this->SupplySideLoop.MaxLoad;
        OptLoad = this->SupplySideLoop.MaxLoad * 0.9;
    }
}

void HeatExchangerStruct::simulate(EnergyPlusData &state,
                                   const PlantLocation &calledFromLocation,
                                   bool const FirstHVACIteration,
                                   Real64 &CurLoad,
                                   [[maybe_unused]] bool const RunFlag)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Main entry point and simulation manager for heat exchanger

    this->initialize(state);

    // for op scheme led HXs, only call controls if called from Loop Supply Side
    if ((this->controlMode == ControlType::OperationSchemeModulated) || (this->controlMode == ControlType::OperationSchemeOnOff)) {
        if (calledFromLocation.loopNum == this->SupplySideLoop.loopNum) {
            this->control(state, CurLoad, FirstHVACIteration);
        }
    } else {
        this->control(state, CurLoad, FirstHVACIteration);
    }

    this->calculate(state,
                    state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).MassFlowRate,
                    state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).MassFlowRate);
}

void GetFluidHeatExchangerInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // get input for heat exchanger model

    static constexpr std::string_view RoutineName("GetFluidHeatExchangerInput: ");
    static constexpr std::string_view routineName = "GetFluidHeatExchangerInput";

    bool ErrorsFound(false);
    int NumAlphas;        // Number of elements in the alpha array
    int NumNums;          // Number of elements in the numeric array
    int IOStat;           // IO Status when calling get input subroutine
    int MaxNumAlphas(0);  // argument for call to GetObjectDefMaxArgs
    int MaxNumNumbers(0); // argument for call to GetObjectDefMaxArgs
    int TotalArgs(0);     // argument for call to GetObjectDefMaxArgs
    Array1D_string cAlphaFieldNames;
    Array1D_string cNumericFieldNames;
    Array1D_bool lNumericFieldBlanks;
    Array1D_bool lAlphaFieldBlanks;
    Array1D_string cAlphaArgs;
    Array1D<Real64> rNumericArgs;
    std::string cCurrentModuleObject;

    cCurrentModuleObject = "HeatExchanger:FluidToFluid";

    state.dataPlantHXFluidToFluid->NumberOfPlantFluidHXs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    if (state.dataPlantHXFluidToFluid->NumberOfPlantFluidHXs == 0) {
        return;
    }

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
    MaxNumNumbers = NumNums;
    MaxNumAlphas = NumAlphas;

    cAlphaFieldNames.allocate(MaxNumAlphas);
    cAlphaArgs.allocate(MaxNumAlphas);
    lAlphaFieldBlanks.dimension(MaxNumAlphas, false);
    cNumericFieldNames.allocate(MaxNumNumbers);
    rNumericArgs.dimension(MaxNumNumbers, 0.0);
    lNumericFieldBlanks.dimension(MaxNumNumbers, false);

    if (state.dataPlantHXFluidToFluid->NumberOfPlantFluidHXs > 0) {
        state.dataPlantHXFluidToFluid->FluidHX.allocate(state.dataPlantHXFluidToFluid->NumberOfPlantFluidHXs);
        for (int CompLoop = 1; CompLoop <= state.dataPlantHXFluidToFluid->NumberOfPlantFluidHXs; ++CompLoop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     CompLoop,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            Util::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).Name = cAlphaArgs(1);

            if (lAlphaFieldBlanks(2)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                ErrorsFound = true;
            }

            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).DemandSideLoop.inletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    cAlphaArgs(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                    cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).DemandSideLoop.outletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    cAlphaArgs(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                    cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(
                state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(3), cAlphaArgs(4), "Loop Demand Side Plant Nodes");
            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).DemandSideLoop.DesignVolumeFlowRate = rNumericArgs(1);
            if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).DemandSideLoop.DesignVolumeFlowRate == DataSizing::AutoSize) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).DemandSideLoop.DesignVolumeFlowRateWasAutoSized = true;
            }

            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SupplySideLoop.inletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    cAlphaArgs(5),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                    cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SupplySideLoop.outletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    cAlphaArgs(6),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                    cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(
                state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(5), cAlphaArgs(6), "Loop Supply Side Plant Nodes");
            state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SupplySideLoop.DesignVolumeFlowRate = rNumericArgs(2);
            if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SupplySideLoop.DesignVolumeFlowRate == DataSizing::AutoSize) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SupplySideLoop.DesignVolumeFlowRateWasAutoSized = true;
            }

            if (lAlphaFieldBlanks(7)) {
                ShowSevereEmptyField(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
                ErrorsFound = true;
            } else if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatExchangeModelType =
                            static_cast<FluidHXType>(getEnumValue(fluidHXTypeNamesUC, cAlphaArgs(7)))) == FluidHXType::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
                ErrorsFound = true;
            }

            if (!lNumericFieldBlanks(3)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).UA = rNumericArgs(3);
                if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).UA == DataSizing::AutoSize) {
                    state.dataPlantHXFluidToFluid->FluidHX(CompLoop).UAWasAutoSized = true;
                }
            } else {
                if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatExchangeModelType != FluidHXType::Ideal) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid entry.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Missing entry for {}", cNumericFieldNames(3)));
                    ErrorsFound = true;
                }
            }

            if (lAlphaFieldBlanks(8)) {
                ShowSevereEmptyField(state, eoh, cAlphaFieldNames(8));
                ErrorsFound = true;
            } else if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode =
                            static_cast<ControlType>(getEnumValue(controlTypeNamesUC, cAlphaArgs(8)))) == ControlType::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8));
                ErrorsFound = true;
            }

            if (!lAlphaFieldBlanks(9)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        cAlphaArgs(9),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                        cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Sensor,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                // check that node actually has setpoints on it
                if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::HeatingSetPointModulated) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::HeatingSetPointOnOff) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointModulated) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOff) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride)) {
                    if (state.dataLoopNodes->Node(state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum).TempSetPoint ==
                        DataLoopNode::SensedNodeFlagValue) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state, format("{} Missing temperature setpoint for DataLoopNode::Node = {}", RoutineName, cAlphaArgs(9)));
                            ShowContinueError(state, format("Occurs for {}=\"{}", cCurrentModuleObject, cAlphaArgs(1)));
                            ShowContinueError(state, " Use a setpoint manager to place a single temperature setpoint on the node");
                            ErrorsFound = true;
                        } else {
                            // need call to EMS to check node
                            bool NodeEMSSetPointMissing = false;
                            EMSManager::CheckIfNodeSetPointManagedByEMS(state,
                                                                        state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum,
                                                                        HVAC::CtrlVarType::Temp,
                                                                        NodeEMSSetPointMissing);
                            if (NodeEMSSetPointMissing) {
                                ShowSevereError(state, format("{} Missing temperature setpoint for node = {}", RoutineName, cAlphaArgs(9)));
                                ShowContinueError(state, format("Occurs for {}=\"{}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(state, "Use a setpoint manager or EMS actuator to place a single temperature setpoint on the node");
                                ErrorsFound = true;
                            }
                        }
                    }
                } else if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::DualDeadBandSetPointModulated) ||
                           (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::DualDeadBandSetPointOnOff)) {
                    if ((state.dataLoopNodes->Node(state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum).TempSetPointHi ==
                         DataLoopNode::SensedNodeFlagValue) ||
                        (state.dataLoopNodes->Node(state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum).TempSetPointLo ==
                         DataLoopNode::SensedNodeFlagValue)) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state, format("{} Missing dual temperature setpoints for node = {}", RoutineName, cAlphaArgs(9)));
                            ShowContinueError(state, format("Occurs for {}=\"{}", cCurrentModuleObject, cAlphaArgs(1)));
                            ShowContinueError(state, " Use a setpoint manager to place a dual temperature setpoint on the node");
                            ErrorsFound = true;
                        } else {
                            // need call to EMS to check node
                            bool NodeEMSSetPointMissing = false;
                            EMSManager::CheckIfNodeSetPointManagedByEMS(state,
                                                                        state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum,
                                                                        HVAC::CtrlVarType::Temp,
                                                                        NodeEMSSetPointMissing);
                            EMSManager::CheckIfNodeSetPointManagedByEMS(state,
                                                                        state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SetPointNodeNum,
                                                                        HVAC::CtrlVarType::Temp,
                                                                        NodeEMSSetPointMissing);
                            if (NodeEMSSetPointMissing) {
                                ShowSevereError(state, format("{} Missing temperature setpoint for node = {}", RoutineName, cAlphaArgs(9)));
                                ShowContinueError(state, format("Occurs for {}=\"{}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(state, "Use a setpoint manager or EMS actuators to place a dual temperature setpoints on the node");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

            } else {
                // need to name a setpoint node if using a setpoint type control mode
                if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::HeatingSetPointModulated) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::HeatingSetPointOnOff) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointModulated) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOff) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::DualDeadBandSetPointModulated) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::DualDeadBandSetPointOnOff) ||
                    (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride)) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid entry.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Missing entry for {}", cAlphaFieldNames(9)));
                    ErrorsFound = true;
                }
            }

            if (!lNumericFieldBlanks(4)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).TempControlTol = rNumericArgs(4);
            } else {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).TempControlTol = 0.01;
            }

            std::string endUseCat = Util::makeUPPER(cAlphaArgs(10));
            if (endUseCat == "FREECOOLING") {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::FreeCooling;
            } else if (endUseCat == "HEATREJECTION") {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::HeatRejection;
            } else if (endUseCat == "HEATRECOVERYFORCOOLING") {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::HeatRecoveryForCooling;
            } else if (endUseCat == "HEATRECOVERYFORCOOLING") {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::HeatRecoveryForHeating;
            } else if (endUseCat == "LOOPTOLOOP") {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::LoopToLoop;
            } else {
                ShowWarningError(
                    state,
                    format("{} = {}, {} is an invalid value for {}", cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(10), cAlphaFieldNames(10)));
                ErrorsFound = true;
            }

            if (!lAlphaFieldBlanks(11)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).OtherCompSupplySideLoop.inletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        cAlphaArgs(11),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                        cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Actuator,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
            } else {
                if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid entry.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Missing entry for {}", cAlphaFieldNames(11)));
                    ErrorsFound = true;
                }
            }

            if (!lAlphaFieldBlanks(12)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).OtherCompDemandSideLoop.inletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        cAlphaArgs(12),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::HeatExchangerFluidToFluid,
                                                        cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Actuator,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
            } else {
                if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid entry.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Missing entry for {}", cAlphaFieldNames(12)));
                    ErrorsFound = true;
                }
            }

            if (lAlphaFieldBlanks(13)) {
                if (state.dataPlantHXFluidToFluid->FluidHX(CompLoop).controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(13), cAlphaFieldNames(8), cAlphaArgs(8));
                    ErrorsFound = true;
                }
            } else if ((state.dataPlantHXFluidToFluid->FluidHX(CompLoop).ControlSignalTemp =
                            static_cast<CtrlTempType>(getEnumValue(ctrlTempTypeNamesUC, cAlphaArgs(13)))) == CtrlTempType::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(13), cAlphaArgs(13));
                ErrorsFound = true;
            }

            if (!lNumericFieldBlanks(5)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SizingFactor = rNumericArgs(5);
            } else {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).SizingFactor = 1.0;
            }

            if (!lNumericFieldBlanks(6)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).MinOperationTemp = rNumericArgs(6);
            } else {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).MinOperationTemp = -9999.0;
            }

            if (!lNumericFieldBlanks(7)) {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).MaxOperationTemp = rNumericArgs(7);
            } else {
                state.dataPlantHXFluidToFluid->FluidHX(CompLoop).MaxOperationTemp = 9999.0;
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found in processing {} input.", RoutineName, cCurrentModuleObject));
    }
}

void HeatExchangerStruct::setupOutputVars(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Heat Transfer Rate",
                        Constant::Units::W,
                        this->HeatTransferRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Heat Transfer Energy",
                        Constant::Units::J,
                        this->HeatTransferEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        this->HeatTransferMeteringEndUse);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Supply Side Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->SupplySideLoop.InletMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Supply Side Inlet Temperature",
                        Constant::Units::C,
                        this->SupplySideLoop.InletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Supply Side Outlet Temperature",
                        Constant::Units::C,
                        this->SupplySideLoop.OutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Demand Side Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->DemandSideLoop.InletMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Demand Side Inlet Temperature",
                        Constant::Units::C,
                        this->DemandSideLoop.InletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Loop Demand Side Outlet Temperature",
                        Constant::Units::C,
                        this->DemandSideLoop.OutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Operation Status",
                        Constant::Units::None,
                        this->OperationStatus,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Fluid Heat Exchanger Effectiveness",
                        Constant::Units::None,
                        this->Effectiveness,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
}

void HeatExchangerStruct::initialize(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   november, 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Initialize heat exchanger model

    static constexpr std::string_view RoutineNameNoColon("InitFluidHeatExchanger");

    this->oneTimeInit(state); // plant setup

    if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {

        Real64 rho = this->DemandSideLoop.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineNameNoColon);
        this->DemandSideLoop.MassFlowRateMax = rho * this->DemandSideLoop.DesignVolumeFlowRate;
        PlantUtilities::InitComponentNodes(state,
                                           this->DemandSideLoop.MassFlowRateMin,
                                           this->DemandSideLoop.MassFlowRateMax,
                                           this->DemandSideLoop.inletNodeNum,
                                           this->DemandSideLoop.outletNodeNum);

        rho = this->SupplySideLoop.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineNameNoColon);
        this->SupplySideLoop.MassFlowRateMax = rho * this->SupplySideLoop.DesignVolumeFlowRate;
        PlantUtilities::InitComponentNodes(state,
                                           this->SupplySideLoop.MassFlowRateMin,
                                           this->SupplySideLoop.MassFlowRateMax,
                                           this->SupplySideLoop.inletNodeNum,
                                           this->SupplySideLoop.outletNodeNum);
        this->MyEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    this->DemandSideLoop.InletTemp = state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp;
    this->SupplySideLoop.InletTemp = state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp;

    if (this->controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride) {
        // store current value for setpoint in central plant loop data structure
        this->OtherCompSupplySideLoop.comp->FreeCoolCntrlMinCntrlTemp =
            state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint - this->TempControlTol; // issue #5626, include control tolerance
    }
}

void HeatExchangerStruct::size(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   December 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Size plant heat exchanger flow rates, UA, and max capacity

    // METHODOLOGY EMPLOYED:
    // the supply side flow rate is obtained from the plant sizing structure
    // the demand side is sized to match the supply side
    // the UA is sized for an effectiveness of 1.0 using sizing temps
    // the capacity uses the full HX model

    static constexpr std::string_view RoutineName("SizeFluidHeatExchanger");

    // first deal with Loop Supply Side
    int PltSizNumSupSide = this->SupplySideLoop.loop->PlantSizNum;
    int PltSizNumDmdSide = this->DemandSideLoop.loop->PlantSizNum;
    Real64 tmpSupSideDesignVolFlowRate = this->SupplySideLoop.DesignVolumeFlowRate;
    if (this->SupplySideLoop.DesignVolumeFlowRateWasAutoSized) {
        if (PltSizNumSupSide > 0) {
            if (state.dataSize->PlantSizData(PltSizNumSupSide).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                tmpSupSideDesignVolFlowRate = state.dataSize->PlantSizData(PltSizNumSupSide).DesVolFlowRate * this->SizingFactor;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->SupplySideLoop.DesignVolumeFlowRate = tmpSupSideDesignVolFlowRate;
                }
            } else {
                tmpSupSideDesignVolFlowRate = 0.0;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->SupplySideLoop.DesignVolumeFlowRate = tmpSupSideDesignVolFlowRate;
                }
            }
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             "HeatExchanger:FluidToFluid",
                                             this->Name,
                                             "Loop Supply Side Design Fluid Flow Rate [m3/s]",
                                             this->SupplySideLoop.DesignVolumeFlowRate);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             "HeatExchanger:FluidToFluid",
                                             this->Name,
                                             "Initial Loop Supply Side Design Fluid Flow Rate [m3/s]",
                                             this->SupplySideLoop.DesignVolumeFlowRate);
            }
        } else {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "SizeFluidHeatExchanger: Autosizing of requires a loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in heat exchanger object={}", this->Name));
            }
        }
    }
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->SupplySideLoop.inletNodeNum, tmpSupSideDesignVolFlowRate);

    // second deal with Loop Demand Side
    Real64 tmpDmdSideDesignVolFlowRate = this->DemandSideLoop.DesignVolumeFlowRate;
    if (this->DemandSideLoop.DesignVolumeFlowRateWasAutoSized) {
        if (tmpSupSideDesignVolFlowRate > HVAC::SmallWaterVolFlow) {
            tmpDmdSideDesignVolFlowRate = tmpSupSideDesignVolFlowRate;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->DemandSideLoop.DesignVolumeFlowRate = tmpDmdSideDesignVolFlowRate;
            }
        } else {
            tmpDmdSideDesignVolFlowRate = 0.0;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->DemandSideLoop.DesignVolumeFlowRate = tmpDmdSideDesignVolFlowRate;
            }
        }
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state,
                                         "HeatExchanger:FluidToFluid",
                                         this->Name,
                                         "Loop Demand Side Design Fluid Flow Rate [m3/s]",
                                         this->DemandSideLoop.DesignVolumeFlowRate);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state,
                                         "HeatExchanger:FluidToFluid",
                                         this->Name,
                                         "Initial Loop Demand Side Design Fluid Flow Rate [m3/s]",
                                         this->DemandSideLoop.DesignVolumeFlowRate);
        }
    }
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->DemandSideLoop.inletNodeNum, tmpDmdSideDesignVolFlowRate);

    // size UA if needed
    if (this->UAWasAutoSized) {
        // get nominal delta T between two loops
        if (PltSizNumSupSide > 0 && PltSizNumDmdSide > 0) {

            Real64 tmpDeltaTloopToLoop(0.0);

            switch (state.dataSize->PlantSizData(PltSizNumSupSide).LoopType) {

            case DataSizing::TypeOfPlantLoop::Heating:
            case DataSizing::TypeOfPlantLoop::Steam: {
                tmpDeltaTloopToLoop =
                    std::abs((state.dataSize->PlantSizData(PltSizNumSupSide).ExitTemp - state.dataSize->PlantSizData(PltSizNumSupSide).DeltaT) -
                             state.dataSize->PlantSizData(PltSizNumDmdSide).ExitTemp);
                break;
            }
            case DataSizing::TypeOfPlantLoop::Cooling:
            case DataSizing::TypeOfPlantLoop::Condenser: {
                tmpDeltaTloopToLoop =
                    std::abs((state.dataSize->PlantSizData(PltSizNumSupSide).ExitTemp + state.dataSize->PlantSizData(PltSizNumSupSide).DeltaT) -
                             state.dataSize->PlantSizData(PltSizNumDmdSide).ExitTemp);
                break;
            }
            default:
                assert(false);
                break;
            }

            tmpDeltaTloopToLoop = max(2.0, tmpDeltaTloopToLoop);
            Real64 tmpDeltaTSupLoop = state.dataSize->PlantSizData(PltSizNumSupSide).DeltaT;
            if (tmpSupSideDesignVolFlowRate >= HVAC::SmallWaterVolFlow) {

                Real64 Cp = this->SupplySideLoop.loop->glycol->getSpecificHeat(state, Constant::InitConvTemp, RoutineName);

                Real64 rho = this->SupplySideLoop.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

                Real64 tmpDesCap = Cp * rho * tmpDeltaTSupLoop * tmpSupSideDesignVolFlowRate;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->UA = tmpDesCap / tmpDeltaTloopToLoop;
                }
            } else {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->UA = 0.0;
                }
            }
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(
                    state, "HeatExchanger:FluidToFluid", this->Name, "Heat Exchanger U-Factor Times Area Value [W/C]", this->UA);
                BaseSizer::reportSizerOutput(state,
                                             "HeatExchanger:FluidToFluid",
                                             this->Name,
                                             "Loop-to-loop Temperature Difference Used to Size Heat Exchanger U-Factor Times Area Value [C]",
                                             tmpDeltaTloopToLoop);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(
                    state, "HeatExchanger:FluidToFluid", this->Name, "Initial Heat Exchanger U-Factor Times Area Value [W/C]", this->UA);
                BaseSizer::reportSizerOutput(state,
                                             "HeatExchanger:FluidToFluid",
                                             this->Name,
                                             "Initial Loop-to-loop Temperature Difference Used to Size Heat Exchanger U-Factor Times Area Value [C]",
                                             tmpDeltaTloopToLoop);
            }
        } else {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "SizeFluidHeatExchanger: Autosizing of heat Exchanger UA requires a loop Sizing:Plant objects for both loops");
                ShowContinueError(state, format("Occurs in heat exchanger object={}", this->Name));
            }
        }
    }

    // size capacities for load range based op schemes
    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {

        if (PltSizNumSupSide > 0) {
            switch (state.dataSize->PlantSizData(PltSizNumSupSide).LoopType) {
            case DataSizing::TypeOfPlantLoop::Heating:
            case DataSizing::TypeOfPlantLoop::Steam: {
                state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp =
                    (state.dataSize->PlantSizData(PltSizNumSupSide).ExitTemp - state.dataSize->PlantSizData(PltSizNumSupSide).DeltaT);
                break;
            }
            case DataSizing::TypeOfPlantLoop::Cooling:
            case DataSizing::TypeOfPlantLoop::Condenser: {
                state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp =
                    (state.dataSize->PlantSizData(PltSizNumSupSide).ExitTemp + state.dataSize->PlantSizData(PltSizNumSupSide).DeltaT);
                break;
            }
            default:
                break;
            }

        } else { // don't rely on sizing, use loop setpoints
            // loop supply side
            if (this->SupplySideLoop.loop->LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {
                state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp =
                    state.dataLoopNodes->Node(this->SupplySideLoop.loop->TempSetPointNodeNum).TempSetPoint;
            } else if (this->SupplySideLoop.loop->LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand) {
                state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp =
                    (state.dataLoopNodes->Node(this->SupplySideLoop.loop->TempSetPointNodeNum).TempSetPointHi +
                     state.dataLoopNodes->Node(this->SupplySideLoop.loop->TempSetPointNodeNum).TempSetPointLo) /
                    2.0;
            }
        }

        if (PltSizNumDmdSide > 0) {
            state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp = state.dataSize->PlantSizData(PltSizNumDmdSide).ExitTemp;
        } else { // don't rely on sizing, use loop setpoints
            // loop demand side
            if (this->DemandSideLoop.loop->LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {
                state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp =
                    state.dataLoopNodes->Node(this->DemandSideLoop.loop->TempSetPointNodeNum).TempSetPoint;
            } else if (this->DemandSideLoop.loop->LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand) {
                state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp =
                    (state.dataLoopNodes->Node(this->DemandSideLoop.loop->TempSetPointNodeNum).TempSetPointHi +
                     state.dataLoopNodes->Node(this->DemandSideLoop.loop->TempSetPointNodeNum).TempSetPointLo) /
                    2.0;
            }
        }

        Real64 rho = this->SupplySideLoop.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
        Real64 SupSideMdot = this->SupplySideLoop.DesignVolumeFlowRate * rho;
        rho = this->DemandSideLoop.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
        Real64 DmdSideMdot = this->DemandSideLoop.DesignVolumeFlowRate * rho;

        this->calculate(state, SupSideMdot, DmdSideMdot);
        this->SupplySideLoop.MaxLoad = std::abs(this->HeatTransferRate);
    }
    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, this->Name, "HeatExchanger:FluidToFluid");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->SupplySideLoop.MaxLoad);
    }
    this->updateCompFlowData(state);
}

void HeatExchangerStruct::control(EnergyPlusData &state, Real64 MyLoad, bool FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // determine control state for fluid to fluid heat exchanger
    // make fluid flow requests accordingly

    // METHODOLOGY EMPLOYED:
    // long CASE statement for different control options

    static constexpr std::string_view RoutineName("ControlFluidHeatExchanger");

    Real64 mdotSupSide;
    Real64 mdotDmdSide;

    // check if available by schedule
    bool ScheduledOff;
    Real64 AvailSchedValue = this->availSched->getCurrentVal();
    if (AvailSchedValue <= 0) {
        ScheduledOff = true;
    } else {
        ScheduledOff = false;
    }

    // check if operational limits trip off unit
    bool LimitTrippedOff = false;
    if ((state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp < this->MinOperationTemp) ||
        (state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp < this->MinOperationTemp)) {
        LimitTrippedOff = true;
    }
    if ((state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp > this->MaxOperationTemp) ||
        (state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp > this->MaxOperationTemp)) {
        LimitTrippedOff = true;
    }

    if (!ScheduledOff && !LimitTrippedOff) {

        switch (this->controlMode) {

        case ControlType::UncontrolledOn: {

            // make passive request for supply side loop flow
            mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
            PlantUtilities::SetComponentFlowRate(
                state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
            if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                // if supply side loop has massflow, request demand side flow
                mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
            } else {
                mdotDmdSide = 0.0;
            }
            PlantUtilities::SetComponentFlowRate(
                state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);

            break;
        }
        case ControlType::OperationSchemeModulated: {

            if (std::abs(MyLoad) > HVAC::SmallLoad) {
                if (MyLoad < -1.0 * HVAC::SmallLoad) { // requesting cooling
                    Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
                    if (DeltaTCooling > this->TempControlTol) { // can do cooling so turn on
                        mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                            // if supply side loop has massflow, request demand side flow
                            Real64 cp = this->SupplySideLoop.loop->glycol->getSpecificHeat(state, this->SupplySideLoop.InletTemp, RoutineName);
                            Real64 TargetLeavingTemp = this->SupplySideLoop.InletTemp - std::abs(MyLoad) / (cp * mdotSupSide);

                            this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::CoolingSupplySideLoop);
                        } else { // no flow on supply side so do not request flow on demand side
                            mdotDmdSide = 0.0;
                            PlantUtilities::SetComponentFlowRate(
                                state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                        }
                    } else { // not able to cool so turn off
                        mdotSupSide = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a
                        // fresh demand side inlet temperature value
                        if (FirstHVACIteration) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    }

                } else { // requesting heating
                    Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
                    if (DeltaTHeating > this->TempControlTol) { // can do heating so turn on
                        mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                            Real64 cp = this->SupplySideLoop.loop->glycol->getSpecificHeat(state, this->SupplySideLoop.InletTemp, RoutineName);
                            Real64 TargetLeavingTemp = this->SupplySideLoop.InletTemp + std::abs(MyLoad) / (cp * mdotSupSide);

                            this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::HeatingSupplySideLoop);
                        } else { // no flow on supply side so do not request flow on demand side
                            mdotDmdSide = 0.0;
                            PlantUtilities::SetComponentFlowRate(
                                state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                        }
                    } else { // not able to heat so turn off
                        mdotSupSide = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a
                        // fresh demand side inlet temperature value
                        if (FirstHVACIteration) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    }
                }

            } else { //  no load
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                mdotDmdSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::OperationSchemeOnOff: {
            if (std::abs(MyLoad) > HVAC::SmallLoad) {
                if (MyLoad < HVAC::SmallLoad) { // requesting cooling
                    Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
                    if (DeltaTCooling > this->TempControlTol) { // can do cooling so turn on
                        mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }

                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    } else { // not able to cool so turn off
                        mdotSupSide = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a
                        // fresh demand side inlet temperature value
                        if (FirstHVACIteration) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    }

                } else { // requesting heating
                    Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
                    if (DeltaTHeating > this->TempControlTol) { // can do heating so turn on
                        mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    } else { // not able to heat so turn off
                        mdotSupSide = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                        // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a
                        // fresh demand side inlet temperature value
                        if (FirstHVACIteration) {
                            mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                        } else {
                            mdotDmdSide = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(
                            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                    }
                }

            } else { // no load
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                mdotDmdSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::HeatingSetPointModulated: {

            Real64 SetPointTemp = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint;
            Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
            if ((DeltaTHeating > this->TempControlTol) && (SetPointTemp > this->SupplySideLoop.InletTemp)) {
                // can and want to heat
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {

                    Real64 TargetLeavingTemp = SetPointTemp;
                    this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::HeatingSupplySideLoop);
                } else {
                    mdotDmdSide = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                }
            } else { // not able are wanting to heat so turn off
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::HeatingSetPointOnOff: {

            Real64 SetPointTemp = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint;
            Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
            if ((DeltaTHeating > this->TempControlTol) && (SetPointTemp > this->SupplySideLoop.InletTemp)) {
                // can and want to heat
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            } else { // not able or are wanting to heat so turn off
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::CoolingSetPointModulated: {

            Real64 SetPointTemp = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint;
            Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
            if ((DeltaTCooling > this->TempControlTol) && (SetPointTemp < this->SupplySideLoop.InletTemp)) {
                // can and want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    Real64 TargetLeavingTemp = SetPointTemp;
                    this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::CoolingSupplySideLoop);
                } else {
                    mdotDmdSide = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                }
            } else { // not able or are wanting to cool so turn off
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::CoolingSetPointOnOff: {

            Real64 SetPointTemp = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint;
            Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
            if ((DeltaTCooling > this->TempControlTol) && (SetPointTemp < this->SupplySideLoop.InletTemp)) {
                // can and want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            } else { // not able or are wanting to cool so turn off
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::DualDeadBandSetPointModulated: {

            Real64 SetPointTempLo = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPointLo;
            Real64 SetPointTempHi = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPointHi;
            Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
            Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
            if ((DeltaTCooling > this->TempControlTol) && (SetPointTempHi < this->SupplySideLoop.InletTemp)) {

                // can and want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    Real64 TargetLeavingTemp = SetPointTempHi;
                    this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::CoolingSupplySideLoop);
                } else {
                    mdotDmdSide = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                }
            } else if ((DeltaTHeating > this->TempControlTol) && (SetPointTempLo > this->SupplySideLoop.InletTemp)) {
                // can and want to heat
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    Real64 TargetLeavingTemp = SetPointTempLo;
                    this->findDemandSideLoopFlow(state, TargetLeavingTemp, HXAction::HeatingSupplySideLoop);
                } else {
                    mdotDmdSide = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
                }
            } else { // not able or don't want conditioning
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::DualDeadBandSetPointOnOff: {

            Real64 SetPointTempLo = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPointLo;
            Real64 SetPointTempHi = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPointHi;
            Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
            Real64 DeltaTHeating = this->DemandSideLoop.InletTemp - this->SupplySideLoop.InletTemp;
            if ((DeltaTCooling > this->TempControlTol) && (SetPointTempHi < this->SupplySideLoop.InletTemp)) {
                // can and want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            } else if ((DeltaTHeating > this->TempControlTol) && (SetPointTempLo > this->SupplySideLoop.InletTemp)) {
                // can and want to heat
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            } else { // not able or don't want conditioning
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::CoolingDifferentialOnOff: {

            Real64 DeltaTCooling = this->SupplySideLoop.InletTemp - this->DemandSideLoop.InletTemp;
            if (DeltaTCooling > this->TempControlTol) {
                //  want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            } else { // not wanting to cool so turn off
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }

            break;
        }
        case ControlType::CoolingSetPointOnOffWithComponentOverride: {

            Real64 ControlSignalValue(0.0);

            switch (this->ControlSignalTemp) {
            case CtrlTempType::WetBulbTemperature: {
                ControlSignalValue = state.dataEnvrn->OutWetBulbTemp;
                break;
            }
            case CtrlTempType::DryBulbTemperature: {
                ControlSignalValue = state.dataEnvrn->OutDryBulbTemp;
                break;
            }
            case CtrlTempType::LoopTemperature: {
                ControlSignalValue = state.dataLoopNodes->Node(this->OtherCompDemandSideLoop.inletNodeNum).TempLastTimestep;
                break;
            }
            default:
                assert(false);
                break;
            }

            Real64 SetPointTemp = state.dataLoopNodes->Node(this->SetPointNodeNum).TempSetPoint;
            Real64 DeltaTCooling = SetPointTemp - ControlSignalValue;
            // obtain shut down state
            bool ChillerShutDown = this->OtherCompSupplySideLoop.comp->FreeCoolCntrlShutDown;
            if (ChillerShutDown && (DeltaTCooling > this->TempControlTol)) {
                // can and want to cool
                mdotSupSide = this->SupplySideLoop.MassFlowRateMax;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                if (mdotSupSide > DataBranchAirLoopPlant::MassFlowTolerance) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);

            } else {
                mdotSupSide = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
                // issue 4959, make demand side flow request on first hvac iteration so demand side loop can run as a trial to get a fresh
                // demand side inlet temperature value
                if (FirstHVACIteration) {
                    mdotDmdSide = this->DemandSideLoop.MassFlowRateMax;
                } else {
                    mdotDmdSide = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(
                    state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
            }
            break;
        }
        default:
            break;
        }

    } else { // scheduled off
        mdotSupSide = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, mdotSupSide, this->SupplySideLoop.inletNodeNum, this->SupplySideLoop.outletNodeNum, this->SupplySideLoop);
        mdotDmdSide = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, mdotDmdSide, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
    }
}

void HeatExchangerStruct::calculate(EnergyPlusData &state, Real64 const SupSideMdot, Real64 const DmdSideMdot)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B.Griffith, derived from CalcEconHeatExchanger by Sankaranarayanan K P aug. 2007
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Evalutate heat exchanger model and calculate leaving temperatures

    // METHODOLOGY EMPLOYED:
    // apply heat transfer model depending on type of HX used

    static constexpr std::string_view RoutineName("CalcFluidHeatExchanger");

    int constexpr CmaxMixedCminUnmixed(40);
    int constexpr CmaxUnMixedCminMixed(41);

    Real64 SupSideLoopInletTemp = state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).Temp;
    Real64 DmdSideLoopInletTemp = state.dataLoopNodes->Node(this->DemandSideLoop.inletNodeNum).Temp;

    // specific heat of fluid entering from supply side loop at inlet temp
    Real64 SupSideLoopInletCp = this->SupplySideLoop.loop->glycol->getSpecificHeat(state, SupSideLoopInletTemp, RoutineName);

    // specific heat of fluid entering from demand side loop at inlet temp
    Real64 DmdSideLoopInletCp = this->DemandSideLoop.loop->glycol->getSpecificHeat(state, DmdSideLoopInletTemp, RoutineName);

    Real64 SupSideCapRate = SupSideMdot * SupSideLoopInletCp;
    Real64 DmdSideCapRate = DmdSideMdot * DmdSideLoopInletCp;
    Real64 MinCapRate = min(SupSideCapRate, DmdSideCapRate);
    Real64 MaxCapRate = max(SupSideCapRate, DmdSideCapRate);

    if (MinCapRate > 0.0) {

        switch (this->HeatExchangeModelType) {

        case FluidHXType::CrossFlowBothUnMixed: {
            Real64 NTU = this->UA / MinCapRate;
            Real64 CapRatio = MinCapRate / MaxCapRate;
            Real64 ExpCheckValue1 = std::pow(NTU, 0.22) / CapRatio;
            Real64 ExpCheckValue2 = -CapRatio * std::pow(NTU, 0.78);
            if ((ExpCheckValue1 > DataPrecisionGlobals::EXP_UpperLimit) || (ExpCheckValue2 > DataPrecisionGlobals::EXP_UpperLimit)) {
                if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 1.0 - std::exp(-NTU);
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = 1.0;
                }
            } else {
                this->Effectiveness = 1.0 - std::exp((std::pow(NTU, 0.22) / CapRatio) * (std::exp(-CapRatio * std::pow(NTU, 0.78)) - 1.0));
                this->Effectiveness = min(1.0, this->Effectiveness);
            }

            break;
        }
        case FluidHXType::CrossFlowBothMixed: {
            Real64 NTU = this->UA / MinCapRate;
            Real64 CapRatio = MinCapRate / MaxCapRate;
            Real64 ExpCheckValue1 = -CapRatio * NTU;
            Real64 ExpCheckValue2 = -NTU;
            if (ExpCheckValue1 < DataPrecisionGlobals::EXP_LowerLimit) {
                if (ExpCheckValue2 >= DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 1.0 - std::exp(-NTU);
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = 1.0;
                }
            } else if (ExpCheckValue2 < DataPrecisionGlobals::EXP_LowerLimit) {
                this->Effectiveness = 1.0;
            } else if ((std::exp(-NTU) == 1.0) || (NTU == 0.0) || (std::exp(-CapRatio * NTU) == 1.0)) { // don't div by zero

                this->Effectiveness = 0.0;
            } else {
                this->Effectiveness = 1.0 / ((1.0 / (1.0 - std::exp(-NTU))) + (CapRatio / (1.0 - std::exp(-CapRatio * NTU))) - (1.0 / NTU));
                this->Effectiveness = min(1.0, this->Effectiveness);
            }

            break;
        }
        case FluidHXType::CrossFlowSupplyLoopMixedDemandLoopUnMixed:
        case FluidHXType::CrossFlowSupplyLoopUnMixedDemandLoopMixed: {

            int CrossFlowEquation;
            if (SupSideCapRate == MaxCapRate && this->HeatExchangeModelType == FluidHXType::CrossFlowSupplyLoopMixedDemandLoopUnMixed) {
                CrossFlowEquation = CmaxMixedCminUnmixed;
            } else if (SupSideCapRate == MinCapRate && this->HeatExchangeModelType == FluidHXType::CrossFlowSupplyLoopMixedDemandLoopUnMixed) {
                CrossFlowEquation = CmaxUnMixedCminMixed;
            } else if (DmdSideCapRate == MaxCapRate && this->HeatExchangeModelType == FluidHXType::CrossFlowSupplyLoopUnMixedDemandLoopMixed) {
                CrossFlowEquation = CmaxMixedCminUnmixed;
            } else if (DmdSideCapRate == MinCapRate && this->HeatExchangeModelType == FluidHXType::CrossFlowSupplyLoopUnMixedDemandLoopMixed) {
                CrossFlowEquation = CmaxUnMixedCminMixed;
            } else {
                CrossFlowEquation = CmaxMixedCminUnmixed;
            }

            Real64 NTU = this->UA / MinCapRate;
            Real64 CapRatio = MinCapRate / MaxCapRate;
            if (CrossFlowEquation == CmaxMixedCminUnmixed) {
                Real64 ExpCheckValue1 = -NTU;
                if (CapRatio == 0.0) { // protect div by zero
                    if (ExpCheckValue1 >= DataPrecisionGlobals::EXP_LowerLimit) {
                        this->Effectiveness = 1.0 - std::exp(-NTU);
                        this->Effectiveness = min(1.0, this->Effectiveness);
                    } else {
                        this->Effectiveness = 1.0;
                    }
                } else if (ExpCheckValue1 < DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 0.632 / CapRatio;
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = (1.0 / CapRatio) * (1.0 - std::exp(CapRatio * std::exp(-NTU) - 1.0));
                    this->Effectiveness = min(1.0, this->Effectiveness);
                }
            } else if (CrossFlowEquation == CmaxUnMixedCminMixed) {
                Real64 ExpCheckValue1 = -CapRatio * NTU;
                if (CapRatio == 0.0) {
                    if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
                        this->Effectiveness = 1.0 - std::exp(-NTU);
                        this->Effectiveness = min(1.0, this->Effectiveness);
                    } else {
                        this->Effectiveness = 1.0;
                    }
                } else {
                    if (ExpCheckValue1 >= DataPrecisionGlobals::EXP_LowerLimit) {
                        Real64 ExpCheckValue2 = -(1.0 / CapRatio) * (1.0 - std::exp(-CapRatio * NTU));
                        if (ExpCheckValue2 < DataPrecisionGlobals::EXP_LowerLimit) {
                            this->Effectiveness = 1.0;
                        } else {
                            this->Effectiveness = 1.0 - std::exp(ExpCheckValue2);
                            this->Effectiveness = min(1.0, this->Effectiveness);
                        }
                    } else {
                        this->Effectiveness = 1.0;
                    }
                }
            } else {
                assert(false);
            }

            break;
        }
        case FluidHXType::CounterFlow: {
            Real64 NTU = this->UA / MinCapRate;
            Real64 CapRatio = MinCapRate / MaxCapRate;
            Real64 ExpCheckValue1 = -NTU * (1.0 - CapRatio);
            if (ExpCheckValue1 > DataPrecisionGlobals::EXP_UpperLimit) {
                if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 1.0 - std::exp(-NTU);
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = 1.0;
                }
            } else if (CapRatio * std::exp(-NTU * (1.0 - CapRatio)) == 1.0) {
                if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 1.0 - std::exp(-NTU);
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = 1.0;
                }
            } else {
                this->Effectiveness = (1.0 - std::exp(-NTU * (1.0 - CapRatio))) / (1.0 - CapRatio * std::exp(-NTU * (1.0 - CapRatio)));
                this->Effectiveness = min(1.0, this->Effectiveness);
            }

            break;
        }
        case FluidHXType::ParallelFlow: {
            Real64 NTU = this->UA / MinCapRate;
            Real64 CapRatio = MinCapRate / MaxCapRate;
            Real64 ExpCheckValue1 = -NTU * (1.0 + CapRatio);
            if (ExpCheckValue1 > DataPrecisionGlobals::EXP_UpperLimit) {
                if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
                    this->Effectiveness = 1.0 - std::exp(-NTU);
                    this->Effectiveness = min(1.0, this->Effectiveness);
                } else {
                    this->Effectiveness = 1.0;
                }
            } else {
                this->Effectiveness = (1.0 - std::exp(-NTU * (1.0 + CapRatio))) / (1.0 + CapRatio);
                this->Effectiveness = min(1.0, this->Effectiveness);
            }

            break;
        }
        case FluidHXType::Ideal: {
            this->Effectiveness = 1.0;
            break;
        }
        default:
            assert(false);
            break;
        }

    } else { // no capacity
        this->Effectiveness = 0.0;
    }

    this->HeatTransferRate = this->Effectiveness * MinCapRate * (SupSideLoopInletTemp - DmdSideLoopInletTemp); // + means supply side is cooled

    if (SupSideMdot > 0.0) {
        this->SupplySideLoop.OutletTemp = SupSideLoopInletTemp - this->HeatTransferRate / (SupSideLoopInletCp * SupSideMdot);
    } else {
        this->SupplySideLoop.OutletTemp = SupSideLoopInletTemp;
    }

    if (DmdSideMdot > 0.0) {
        this->DemandSideLoop.OutletTemp = DmdSideLoopInletTemp + this->HeatTransferRate / (DmdSideLoopInletCp * DmdSideMdot);
    } else {
        this->DemandSideLoop.OutletTemp = DmdSideLoopInletTemp;
    }

    this->SupplySideLoop.InletTemp = SupSideLoopInletTemp;
    this->SupplySideLoop.InletMassFlowRate = SupSideMdot;
    this->DemandSideLoop.InletTemp = DmdSideLoopInletTemp;
    this->DemandSideLoop.InletMassFlowRate = DmdSideMdot;

    state.dataLoopNodes->Node(this->DemandSideLoop.outletNodeNum).Temp = this->DemandSideLoop.OutletTemp;
    state.dataLoopNodes->Node(this->SupplySideLoop.outletNodeNum).Temp = this->SupplySideLoop.OutletTemp;

    this->HeatTransferEnergy = this->HeatTransferRate * state.dataHVACGlobal->TimeStepSysSec;

    if ((std::abs(this->HeatTransferRate) > HVAC::SmallLoad) && (this->DemandSideLoop.InletMassFlowRate > 0.0) &&
        (this->SupplySideLoop.InletMassFlowRate > 0.0)) {
        this->OperationStatus = 1.0;
    } else {
        this->OperationStatus = 0.0;
    }
}

void HeatExchangerStruct::findDemandSideLoopFlow(EnergyPlusData &state, Real64 const TargetSupplySideLoopLeavingTemp, HXAction const HXActionMode)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // modulate demand side flow rate to hit a target leaving temperature (within tolerance)

    // METHODOLOGY EMPLOYED:
    // uses E+'s Regula Falsi numerical method

    int constexpr MaxIte(500);   // Maximum number of iterations for solver
    Real64 constexpr Acc(1.e-3); // Accuracy of solver result

    int SolFla; // Flag of solver

    // mass flow rate of fluid entering from supply side loop
    Real64 SupSideMdot = state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).MassFlowRate;
    // first see if root is bracketed
    // min demand flow

    // mass flow rate of fluid entering from demand side loop
    Real64 DmdSideMdot = this->DemandSideLoop.MassFlowRateMin;
    this->calculate(state, SupSideMdot, DmdSideMdot);
    Real64 LeavingTempMinFlow = this->SupplySideLoop.OutletTemp;

    // full demand flow
    DmdSideMdot = this->DemandSideLoop.MassFlowRateMax;
    this->calculate(state, SupSideMdot, DmdSideMdot);
    Real64 LeavingTempFullFlow = this->SupplySideLoop.OutletTemp;

    switch (HXActionMode) {

    case HXAction::HeatingSupplySideLoop: {
        if ((LeavingTempFullFlow > TargetSupplySideLoopLeavingTemp) && (TargetSupplySideLoopLeavingTemp > LeavingTempMinFlow)) {
            auto f = [&state, this, TargetSupplySideLoopLeavingTemp](Real64 const DmdSideMassFlowRate) {
                Real64 SupSideMdot = state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).MassFlowRate;
                this->calculate(state, SupSideMdot, DmdSideMassFlowRate);
                return TargetSupplySideLoopLeavingTemp - this->SupplySideLoop.OutletTemp;
            };
            General::SolveRoot(
                state, Acc, MaxIte, SolFla, DmdSideMdot, f, this->DemandSideLoop.MassFlowRateMin, this->DemandSideLoop.MassFlowRateMax);

            if (SolFla == -1) { // no convergence
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->DmdSideModulatSolvNoConvergeErrorCount < 1) {
                        ++this->DmdSideModulatSolvNoConvergeErrorCount;
                        ShowWarningError(
                            state,
                            format("{} named {} - Iteration Limit exceeded calculating demand side loop flow rate", ComponentClassName, this->Name));
                        ShowContinueError(state, format("Simulation continues with calculated demand side mass flow rate = {:.7R}", DmdSideMdot));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   ComponentClassName + " named " + this->Name +
                                                       " - Iteration Limit exceeded calculating demand side loop flow rate continues.",
                                                   this->DmdSideModulatSolvNoConvergeErrorIndex,
                                                   DmdSideMdot,
                                                   DmdSideMdot);
                }
            } else if (SolFla == -2) { // f(x0) and f(x1) have the same sign
                DmdSideMdot = this->DemandSideLoop.MassFlowRateMax * (LeavingTempFullFlow - TargetSupplySideLoopLeavingTemp) /
                              (LeavingTempFullFlow - LeavingTempMinFlow);
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->DmdSideModulatSolvFailErrorCount < 1) {
                        ++this->DmdSideModulatSolvFailErrorCount;
                        ShowWarningError(
                            state, format("{} named {} - Solver failed to calculate demand side loop flow rate", ComponentClassName, this->Name));
                        ShowContinueError(state, format("Simulation continues with estimated demand side mass flow rate = {:.7R}", DmdSideMdot));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   ComponentClassName + " named " + this->Name +
                                                       " - Solver failed to calculate demand side loop flow rate continues.",
                                                   this->DmdSideModulatSolvFailErrorIndex,
                                                   DmdSideMdot,
                                                   DmdSideMdot);
                }
            }
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);

        } else if ((TargetSupplySideLoopLeavingTemp >= LeavingTempFullFlow) && (LeavingTempFullFlow > LeavingTempMinFlow)) {
            // run at full flow
            DmdSideMdot = this->DemandSideLoop.MassFlowRateMax;
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);

        } else if (LeavingTempMinFlow >= TargetSupplySideLoopLeavingTemp) {

            // run at min flow
            DmdSideMdot = this->DemandSideLoop.MassFlowRateMin;
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
        }
        break;
    }
    case HXAction::CoolingSupplySideLoop: {
        if ((LeavingTempFullFlow < TargetSupplySideLoopLeavingTemp) && (TargetSupplySideLoopLeavingTemp < LeavingTempMinFlow)) {
            auto f = [&state, this, TargetSupplySideLoopLeavingTemp](Real64 const DmdSideMassFlowRate) {
                Real64 SupSideMdot = state.dataLoopNodes->Node(this->SupplySideLoop.inletNodeNum).MassFlowRate;
                this->calculate(state, SupSideMdot, DmdSideMassFlowRate);
                return TargetSupplySideLoopLeavingTemp - this->SupplySideLoop.OutletTemp;
            };
            General::SolveRoot(
                state, Acc, MaxIte, SolFla, DmdSideMdot, f, this->DemandSideLoop.MassFlowRateMin, this->DemandSideLoop.MassFlowRateMax);

            if (SolFla == -1) { // no convergence
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->DmdSideModulatSolvNoConvergeErrorCount < 1) {
                        ++this->DmdSideModulatSolvNoConvergeErrorCount;
                        ShowWarningError(
                            state,
                            format("{} named {} - Iteration Limit exceeded calculating demand side loop flow rate", ComponentClassName, this->Name));
                        ShowContinueError(state, format("Simulation continues with calculated demand side mass flow rate = {:.7R}", DmdSideMdot));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   ComponentClassName + " named " + this->Name +
                                                       " - Iteration Limit exceeded calculating demand side loop flow rate continues.",
                                                   this->DmdSideModulatSolvNoConvergeErrorIndex,
                                                   DmdSideMdot,
                                                   DmdSideMdot);
                }
            } else if (SolFla == -2) { // f(x0) and f(x1) have the same sign
                DmdSideMdot = this->DemandSideLoop.MassFlowRateMax * (LeavingTempFullFlow - TargetSupplySideLoopLeavingTemp) /
                              (LeavingTempFullFlow - LeavingTempMinFlow);
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->DmdSideModulatSolvFailErrorCount < 1) {
                        ++this->DmdSideModulatSolvFailErrorCount;
                        ShowWarningError(
                            state, format("{} named {} - Solver failed to calculate demand side loop flow rate", ComponentClassName, this->Name));
                        ShowContinueError(state, format("Simulation continues with estimated demand side mass flow rate = {:.7R}", DmdSideMdot));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   ComponentClassName + " named " + this->Name +
                                                       " - Solver failed to calculate demand side loop flow rate continues.",
                                                   this->DmdSideModulatSolvFailErrorIndex,
                                                   DmdSideMdot,
                                                   DmdSideMdot);
                }
            }
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
        } else if ((TargetSupplySideLoopLeavingTemp <= LeavingTempFullFlow) && (LeavingTempFullFlow < LeavingTempMinFlow)) {
            // run at full flow
            DmdSideMdot = this->DemandSideLoop.MassFlowRateMax;
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
        } else if (LeavingTempMinFlow <= TargetSupplySideLoopLeavingTemp) {

            // run at min flow
            DmdSideMdot = this->DemandSideLoop.MassFlowRateMin;
            PlantUtilities::SetComponentFlowRate(
                state, DmdSideMdot, this->DemandSideLoop.inletNodeNum, this->DemandSideLoop.outletNodeNum, this->DemandSideLoop);
        }
        break;
    }
    default:
        break;
    }
}

void HeatExchangerStruct::oneTimeInit(EnergyPlusData &state)
{

    static constexpr std::string_view RoutineName("InitFluidHeatExchanger: ");

    if (this->MyOneTimeFlag) {
        this->setupOutputVars(state);
        this->MyFlag = true;
        this->MyEnvrnFlag = true;
        this->MyOneTimeFlag = false;
    }

    if (this->MyFlag) {
        // locate the main two connections to the plant loops
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg,
                                                this->DemandSideLoop,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->DemandSideLoop.inletNodeNum,
                                                _);

        if (this->DemandSideLoop.loopSideNum != DataPlant::LoopSideLocation::Demand) { // throw error
            ShowSevereError(state,
                            format("{} Invalid connections for {} name = \"{}\"",
                                   RoutineName,
                                   DataPlant::PlantEquipTypeNames[static_cast<int>(DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg)],
                                   this->Name));
            ShowContinueError(state, "The \"Loop Demand Side\" connections are not on the Demand Side of a plant loop");
            errFlag = true;
        }

        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg,
                                                this->SupplySideLoop,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->SupplySideLoop.inletNodeNum,
                                                _);

        if (this->SupplySideLoop.loopSideNum != DataPlant::LoopSideLocation::Supply) { // throw error
            ShowSevereError(state,
                            format("{} Invalid connections for {} name = \"{}\"",
                                   RoutineName,
                                   DataPlant::PlantEquipTypeNames[static_cast<int>(DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg)],
                                   this->Name));
            ShowContinueError(state, "The \"Loop Supply Side\" connections are not on the Supply Side of a plant loop");
            errFlag = true;
        }

        // make sure it is not the same loop on both sides.
        if (this->SupplySideLoop.loopNum == this->DemandSideLoop.loopNum) { // user is being too tricky, don't allow
            ShowSevereError(state,
                            format("{} Invalid connections for {} name = \"{}\"",
                                   RoutineName,
                                   DataPlant::PlantEquipTypeNames[static_cast<int>(DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg)],
                                   this->Name));
            ShowContinueError(state, R"(The "Loop Supply Side" and "Loop Demand Side" need to be on different loops.)");
            errFlag = true;
        } else {

            PlantUtilities::InterConnectTwoPlantLoopSides(
                state, this->SupplySideLoop, this->DemandSideLoop, DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg, true);
        }

        // find remote component if control mode is of that type.
        if (this->controlMode == ControlType::CoolingSetPointOnOffWithComponentOverride) {

            PlantUtilities::ScanPlantLoopsForNodeNum(
                state, RoutineName, this->OtherCompSupplySideLoop.inletNodeNum, this->OtherCompSupplySideLoop, this->OtherCompSupplySideLoop.compNum);

            PlantUtilities::ScanPlantLoopsForNodeNum(
                state, RoutineName, this->OtherCompDemandSideLoop.inletNodeNum, this->OtherCompDemandSideLoop, this->OtherCompDemandSideLoop.compNum);

            // revise how loads served category for other controlled equipment
            switch (this->OtherCompSupplySideLoop.comp->HowLoadServed) {

            case DataPlant::HowMet::ByNominalCap: {
                this->OtherCompSupplySideLoop.comp->HowLoadServed = DataPlant::HowMet::ByNominalCapFreeCoolCntrl;
                break;
            }
            case DataPlant::HowMet::ByNominalCapLowOutLimit: {
                this->OtherCompSupplySideLoop.comp->HowLoadServed = DataPlant::HowMet::ByNominalCapLowOutLimitFreeCoolCntrl;
                break;
            }
            default:
                break;
            }

            switch (this->ControlSignalTemp) {
            case CtrlTempType::WetBulbTemperature: {
                this->OtherCompSupplySideLoop.comp->FreeCoolCntrlMode = DataPlant::FreeCoolControlMode::WetBulb;
                break;
            }
            case CtrlTempType::DryBulbTemperature: {
                this->OtherCompSupplySideLoop.comp->FreeCoolCntrlMode = DataPlant::FreeCoolControlMode::DryBulb;
                break;
            }
            case CtrlTempType::LoopTemperature: {
                this->OtherCompSupplySideLoop.comp->FreeCoolCntrlMode = DataPlant::FreeCoolControlMode::Loop;
                this->OtherCompSupplySideLoop.comp->FreeCoolCntrlNodeNum = this->OtherCompDemandSideLoop.inletNodeNum;
                break;
            }
            default:
                break;
            }
        }
        if (this->controlMode == ControlType::TrackComponentOnOff) {
            if (this->OtherCompSupplySideLoop.inletNodeNum > 0) {
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        this->ComponentUserName,
                                                        this->ComponentType,
                                                        this->OtherCompSupplySideLoop,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        this->OtherCompSupplySideLoop.inletNodeNum,
                                                        _);
            }
            if (this->OtherCompDemandSideLoop.inletNodeNum > 0) {
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        this->ComponentUserName,
                                                        this->ComponentType,
                                                        this->OtherCompDemandSideLoop,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        this->OtherCompDemandSideLoop.inletNodeNum,
                                                        _);
            }
        }

        if (errFlag) {
            ShowFatalError(state, format("{} Program terminated due to previous condition(s).", RoutineName));
        }
        this->MyFlag = false;
    }
}

void HeatExchangerStruct::updateCompFlowData(EnergyPlusData &state)
{
    int demandArrayIndex = -1;
    int supplyArrayIndex = -1;
    auto &demandCompNames = state.dataPlnt->PlantLoop(this->DemandSideLoop.loopNum).plantCoilObjectNames;
    auto &demandCompTypes = state.dataPlnt->PlantLoop(this->DemandSideLoop.loopNum).plantCoilObjectTypes;
    auto &supplyCompNames = state.dataPlnt->PlantLoop(this->SupplySideLoop.loopNum).plantCoilObjectNames;
    auto &supplyCompTypes = state.dataPlnt->PlantLoop(this->SupplySideLoop.loopNum).plantCoilObjectTypes;
    for (size_t i = 0; i < demandCompNames.size(); ++i) {
        if (demandCompNames[i] == this->Name && demandCompTypes[i] == this->DemandSideLoop.comp->Type) {
            demandArrayIndex = i;
            break;
        }
    }
    for (size_t i = 0; i < supplyCompNames.size(); ++i) {
        if (supplyCompNames[i] == this->Name && supplyCompTypes[i] == this->SupplySideLoop.comp->Type) {
            supplyArrayIndex = i;
            break;
        }
    }
    size_t demandCompSize = state.dataPlnt->PlantLoop(this->DemandSideLoop.loopNum).compDesWaterFlowRate.size();
    auto &demandCoilData = state.dataPlnt->PlantLoop(this->DemandSideLoop.loopNum).compDesWaterFlowRate;
    size_t supplyCompSize = state.dataPlnt->PlantLoop(this->SupplySideLoop.loopNum).compDesWaterFlowRate.size();
    auto &supplyCoilData = state.dataPlnt->PlantLoop(this->SupplySideLoop.loopNum).compDesWaterFlowRate;
    std::vector<Real64> supplyFlowData;
    supplyFlowData.resize(size_t(24 * state.dataGlobal->TimeStepsInHour + 1));
    for (size_t i = 0; i < supplyFlowData.size(); ++i) {
        supplyFlowData[i] = 0.0;
    }
    if (supplyCompSize > 0) {
        for (size_t comp = 0; comp < supplyCoilData.size(); ++comp) {
            if (static_cast<int>(comp) == supplyArrayIndex) {
                continue;
            }
            for (size_t ts = 0; ts < supplyCoilData[comp].tsDesWaterFlowRate.size(); ++ts) {
                supplyFlowData[ts] += supplyCoilData[comp].tsDesWaterFlowRate[ts];
            }
            supplyFlowData[0] = -1;
        }
        if (demandArrayIndex == -1) {
            demandCompNames.emplace_back(this->Name);
            demandCompTypes.emplace_back(this->SupplySideLoop.comp->Type);
            size_t arrayIndex = demandCoilData.size() + 1;
            demandCoilData.resize(arrayIndex);
            demandCoilData[arrayIndex - 1].tsDesWaterFlowRate.resize(size_t(24 * state.dataGlobal->TimeStepsInHour));
            demandCoilData[arrayIndex - 1].tsDesWaterFlowRate = supplyFlowData;
        } else {
            demandCoilData[demandArrayIndex].tsDesWaterFlowRate = supplyFlowData;
        }
    }
    if (hasSupplySideTES(state)) {
        // if the supply side of the HX contains a TES system then copy demand side coil data to supply side so TES can size on the whole load
        std::vector<Real64> demandFlowData;
        demandFlowData.resize(size_t(24 * state.dataGlobal->TimeStepsInHour + 1));
        for (size_t i = 0; i < demandFlowData.size(); ++i) {
            demandFlowData[i] = 0.0;
        }
        if (demandCompSize > 0) {
            for (size_t comp = 0; comp < demandCoilData.size(); ++comp) {
                if (static_cast<int>(comp) == demandArrayIndex) {
                    continue;
                }
                for (size_t ts = 0; ts < demandCoilData.size(); ++ts) {
                    demandFlowData[ts] += demandCoilData[comp].tsDesWaterFlowRate[ts];
                }
                demandFlowData[0] = -1;
            }
        }
        if (demandCompSize > 0) {
            if (supplyArrayIndex == -1) {
                supplyCompNames.emplace_back(this->Name);
                supplyCompTypes.emplace_back(this->SupplySideLoop.comp->Type);
                size_t arrayIndex = supplyCoilData.size() + 1;
                supplyCoilData.resize(arrayIndex);
                supplyCoilData[arrayIndex - 1].tsDesWaterFlowRate.resize(size_t(24 * state.dataGlobal->TimeStepsInHour));
                supplyCoilData[arrayIndex - 1].tsDesWaterFlowRate = demandFlowData;
            } else {
                supplyCoilData[demandArrayIndex].tsDesWaterFlowRate = demandFlowData;
            }
        }
    }
}

bool HeatExchangerStruct::hasSupplySideTES(EnergyPlusData &state)
{
    for (auto const &loopSide : state.dataPlnt->PlantLoop(this->SupplySideLoop.loopNum).LoopSide) {
        for (auto const &branch : loopSide.Branch) {
            for (auto const &comp : branch.Comp) {
                if (comp.Type == DataPlant::PlantEquipmentType::TS_IceDetailed || comp.Type == DataPlant::PlantEquipmentType::TS_IceSimple) {
                    return true;
                }
            }
        }
    }
    return false;
}

} // namespace EnergyPlus::PlantHeatExchangerFluidToFluid
