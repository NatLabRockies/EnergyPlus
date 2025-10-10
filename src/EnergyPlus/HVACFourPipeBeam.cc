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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirTerminalUnit.hh>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACFourPipeBeam.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace FourPipeBeam {

    //    HVACFourPipeBeam::HVACFourPipeBeam(){}
    ///// Note use of shared_ptr here is not a good pattern, not to be replicated without further discussion.
    std::shared_ptr<AirTerminalUnit> HVACFourPipeBeam::fourPipeBeamFactory(EnergyPlusData &state, std::string objectName)
    {

        using BranchNodeConnections::TestCompSet;
        using DataLoopNode::ObjectIsNotParent;
        using DataLoopNode::ObjectIsParent;
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataSizing;
        using Curve::GetCurveIndex;
        static constexpr std::string_view routineName("FourPipeBeamFactory "); // include trailing blank space

        int beamIndex; // loop index

        //  certain object in the input file
        bool errFlag = false;
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        bool found = false;
        bool airNodeFound;
        int aDUIndex;

        ///// Note use of shared_ptr here is not a good pattern, not to be replicated without further discussion.
        std::shared_ptr<HVACFourPipeBeam> thisBeam(new HVACFourPipeBeam());
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        // find the number of cooled beam units
        cCurrentModuleObject = "AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam";

        // find beam index from name
        beamIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, cCurrentModuleObject, objectName);
        if (beamIndex > 0) {
            int IOStatus;
            int NumAlphas = 16;
            int NumNumbers = 11;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     beamIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            found = true;
        } else {
            ErrorsFound = true;
        }

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        errFlag = false;
        GlobalNames::VerifyUniqueADUName(state, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1), errFlag, cCurrentModuleObject + " Name");
        if (errFlag) {
            ErrorsFound = true;
        }
        thisBeam->name = state.dataIPShortCut->cAlphaArgs(1);
        thisBeam->unitType = cCurrentModuleObject;

        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisBeam->airAvailSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisBeam->airAvailSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            thisBeam->coolingAvailSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisBeam->coolingAvailSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
            ErrorsFound = true;
        }
        if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
            thisBeam->heatingAvailSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisBeam->heatingAvailSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
            ErrorsFound = true;
        }

        thisBeam->airInNodeNum = GetOnlySingleNode(state,
                                                   state.dataIPShortCut->cAlphaArgs(5),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   DataLoopNode::NodeFluidType::Air,
                                                   DataLoopNode::ConnectionType::Inlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent,
                                                   state.dataIPShortCut->cAlphaFieldNames(5));
        thisBeam->airOutNodeNum = GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(6),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    ObjectIsNotParent,
                                                    state.dataIPShortCut->cAlphaFieldNames(6));
        if (state.dataIPShortCut->lAlphaFieldBlanks(7) && state.dataIPShortCut->lAlphaFieldBlanks(8)) { // no chilled water nodes, no beam cooling
            thisBeam->beamCoolingPresent = false;
        } else if (state.dataIPShortCut->lAlphaFieldBlanks(7) &&
                   !state.dataIPShortCut->lAlphaFieldBlanks(8)) { // outlet node but no inlet node for chilled water
            thisBeam->beamCoolingPresent = false;
            ShowWarningError(state,
                             format("{}{}: missing {} for {}={}, simulation continues with no beam cooling",
                                    routineName,
                                    cCurrentModuleObject,
                                    state.dataIPShortCut->cAlphaFieldNames(7),
                                    state.dataIPShortCut->cAlphaFieldNames(1),
                                    state.dataIPShortCut->cAlphaArgs(1)));
        } else if (!state.dataIPShortCut->lAlphaFieldBlanks(7) &&
                   state.dataIPShortCut->lAlphaFieldBlanks(8)) { // inlet node but no outlet node for chilled water
            thisBeam->beamCoolingPresent = false;
            ShowWarningError(state,
                             format("{}{}: missing {} for {}={}, simulation continues with no beam cooling",
                                    routineName,
                                    cCurrentModuleObject,
                                    state.dataIPShortCut->cAlphaFieldNames(8),
                                    state.dataIPShortCut->cAlphaFieldNames(1),
                                    state.dataIPShortCut->cAlphaArgs(1)));
        } else {
            thisBeam->beamCoolingPresent = true;
            thisBeam->cWInNodeNum = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(7),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Water,
                                                      DataLoopNode::ConnectionType::Inlet,
                                                      NodeInputManager::CompFluidStream::Secondary,
                                                      ObjectIsParent,
                                                      state.dataIPShortCut->cAlphaFieldNames(7));
            thisBeam->cWOutNodeNum = GetOnlySingleNode(state,
                                                       state.dataIPShortCut->cAlphaArgs(8),
                                                       ErrorsFound,
                                                       DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                       state.dataIPShortCut->cAlphaArgs(1),
                                                       DataLoopNode::NodeFluidType::Water,
                                                       DataLoopNode::ConnectionType::Outlet,
                                                       NodeInputManager::CompFluidStream::Secondary,
                                                       ObjectIsParent,
                                                       state.dataIPShortCut->cAlphaFieldNames(8));
        }
        if (state.dataIPShortCut->lAlphaFieldBlanks(9) && state.dataIPShortCut->lAlphaFieldBlanks(10)) { // no hot water nodes, no beam heating
            thisBeam->beamHeatingPresent = false;
        } else if (state.dataIPShortCut->lAlphaFieldBlanks(9) &&
                   !state.dataIPShortCut->lAlphaFieldBlanks(10)) { // outlet node but no inlet node for hot water
            thisBeam->beamHeatingPresent = false;
            ShowWarningError(state,
                             format("{}{}: missing {} for {}={}, simulation continues with no beam heating",
                                    routineName,
                                    cCurrentModuleObject,
                                    state.dataIPShortCut->cAlphaFieldNames(9),
                                    state.dataIPShortCut->cAlphaFieldNames(1),
                                    state.dataIPShortCut->cAlphaArgs(1)));
        } else if (!state.dataIPShortCut->lAlphaFieldBlanks(9) &&
                   state.dataIPShortCut->lAlphaFieldBlanks(10)) { // inlet node but no outlet node for hot water
            thisBeam->beamHeatingPresent = false;
            ShowWarningError(state,
                             format("{}{}: missing {} for {}={}, simulation continues with no beam heating",
                                    routineName,
                                    cCurrentModuleObject,
                                    state.dataIPShortCut->cAlphaFieldNames(10),
                                    state.dataIPShortCut->cAlphaFieldNames(1),
                                    state.dataIPShortCut->cAlphaArgs(1)));
        } else {
            thisBeam->beamHeatingPresent = true;
            thisBeam->hWInNodeNum = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(9),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Water,
                                                      DataLoopNode::ConnectionType::Inlet,
                                                      NodeInputManager::CompFluidStream::Secondary,
                                                      ObjectIsParent,
                                                      state.dataIPShortCut->cAlphaFieldNames(9));
            thisBeam->hWOutNodeNum = GetOnlySingleNode(state,
                                                       state.dataIPShortCut->cAlphaArgs(10),
                                                       ErrorsFound,
                                                       DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeBeam,
                                                       state.dataIPShortCut->cAlphaArgs(1),
                                                       DataLoopNode::NodeFluidType::Water,
                                                       DataLoopNode::ConnectionType::Outlet,
                                                       NodeInputManager::CompFluidStream::Secondary,
                                                       ObjectIsParent,
                                                       state.dataIPShortCut->cAlphaFieldNames(10));
        }
        thisBeam->vDotDesignPrimAir = state.dataIPShortCut->rNumericArgs(1);
        if (thisBeam->vDotDesignPrimAir == AutoSize) {
            thisBeam->vDotDesignPrimAirWasAutosized = true;
        }
        thisBeam->vDotDesignCW = state.dataIPShortCut->rNumericArgs(2);
        if (thisBeam->vDotDesignCW == AutoSize && thisBeam->beamCoolingPresent) {
            thisBeam->vDotDesignCWWasAutosized = true;
        }
        thisBeam->vDotDesignHW = state.dataIPShortCut->rNumericArgs(3);
        if (thisBeam->vDotDesignHW == AutoSize && thisBeam->beamHeatingPresent) {
            thisBeam->vDotDesignHWWasAutosized = true;
        }
        thisBeam->totBeamLength = state.dataIPShortCut->rNumericArgs(4);
        if (thisBeam->totBeamLength == AutoSize) {
            thisBeam->totBeamLengthWasAutosized = true;
        }
        thisBeam->vDotNormRatedPrimAir = state.dataIPShortCut->rNumericArgs(5);
        thisBeam->qDotNormRatedCooling = state.dataIPShortCut->rNumericArgs(6);
        thisBeam->deltaTempRatedCooling = state.dataIPShortCut->rNumericArgs(7);
        thisBeam->vDotNormRatedCW = state.dataIPShortCut->rNumericArgs(8);

        thisBeam->modCoolingQdotDeltaTFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(11));
        if (thisBeam->modCoolingQdotDeltaTFuncNum == 0 && thisBeam->beamCoolingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(11), state.dataIPShortCut->cAlphaArgs(11)));
            ErrorsFound = true;
        }
        thisBeam->modCoolingQdotAirFlowFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(12));
        if (thisBeam->modCoolingQdotAirFlowFuncNum == 0 && thisBeam->beamCoolingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(12), state.dataIPShortCut->cAlphaArgs(12)));
            ErrorsFound = true;
        }
        thisBeam->modCoolingQdotCWFlowFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(13));
        if (thisBeam->modCoolingQdotCWFlowFuncNum == 0 && thisBeam->beamCoolingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(13), state.dataIPShortCut->cAlphaArgs(13)));
            ErrorsFound = true;
        }
        thisBeam->qDotNormRatedHeating = state.dataIPShortCut->rNumericArgs(9);
        thisBeam->deltaTempRatedHeating = state.dataIPShortCut->rNumericArgs(10);
        thisBeam->vDotNormRatedHW = state.dataIPShortCut->rNumericArgs(11);
        thisBeam->modHeatingQdotDeltaTFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(14));
        if (thisBeam->modHeatingQdotDeltaTFuncNum == 0 && thisBeam->beamHeatingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(14), state.dataIPShortCut->cAlphaArgs(14)));
            ErrorsFound = true;
        }
        thisBeam->modHeatingQdotAirFlowFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(15));
        if (thisBeam->modHeatingQdotAirFlowFuncNum == 0 && thisBeam->beamHeatingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(15), state.dataIPShortCut->cAlphaArgs(15)));
            ErrorsFound = true;
        }
        thisBeam->modHeatingQdotHWFlowFuncNum = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(16));
        if (thisBeam->modHeatingQdotHWFlowFuncNum == 0 && thisBeam->beamHeatingPresent) {
            ShowSevereError(state, format("{}{}=\"{}\"", routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(16), state.dataIPShortCut->cAlphaArgs(16)));
            ErrorsFound = true;
        }
        // Register component set data
        TestCompSet(state,
                    cCurrentModuleObject,
                    thisBeam->name,
                    state.dataLoopNodes->NodeID(thisBeam->airInNodeNum),
                    state.dataLoopNodes->NodeID(thisBeam->airOutNodeNum),
                    "Air Nodes");
        if (thisBeam->beamCoolingPresent) {
            TestCompSet(state,
                        cCurrentModuleObject,
                        thisBeam->name,
                        state.dataLoopNodes->NodeID(thisBeam->cWInNodeNum),
                        state.dataLoopNodes->NodeID(thisBeam->cWOutNodeNum),
                        "Chilled Water Nodes");
        }
        if (thisBeam->beamHeatingPresent) {
            TestCompSet(state,
                        cCurrentModuleObject,
                        thisBeam->name,
                        state.dataLoopNodes->NodeID(thisBeam->hWInNodeNum),
                        state.dataLoopNodes->NodeID(thisBeam->hWOutNodeNum),
                        "Hot Water Nodes");
        }

        // Setup the Cooled Beam reporting variables
        if (thisBeam->beamCoolingPresent) {
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Cooling Energy",
                                Constant::Units::J,
                                thisBeam->beamCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBeam->name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Cooling Rate",
                                Constant::Units::W,
                                thisBeam->beamCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBeam->name);
        }
        if (thisBeam->beamHeatingPresent) {
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Heating Energy",
                                Constant::Units::J,
                                thisBeam->beamHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBeam->name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Heating Rate",
                                Constant::Units::W,
                                thisBeam->beamHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBeam->name);
        }
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Sensible Cooling Energy",
                            Constant::Units::J,
                            thisBeam->supAirCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisBeam->name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Sensible Cooling Rate",
                            Constant::Units::W,
                            thisBeam->supAirCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisBeam->name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Sensible Heating Energy",
                            Constant::Units::J,
                            thisBeam->supAirHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisBeam->name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Sensible Heating Rate",
                            Constant::Units::W,
                            thisBeam->supAirHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisBeam->name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Flow Rate",
                            Constant::Units::m3_s,
                            thisBeam->primAirFlow,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisBeam->name);

        SetupOutputVariable(state,
                            "Zone Air Terminal Outdoor Air Volume Flow Rate",
                            Constant::Units::m3_s,
                            thisBeam->OutdoorAirFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisBeam->name);

        airNodeFound = false;
        for (aDUIndex = 1; aDUIndex <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++aDUIndex) {
            if (thisBeam->airOutNodeNum == state.dataDefineEquipment->AirDistUnit(aDUIndex).OutletNodeNum) {
                thisBeam->aDUNum = aDUIndex;
                state.dataDefineEquipment->AirDistUnit(aDUIndex).InletNodeNum = thisBeam->airInNodeNum;
            }
        }
        // assumes if there isn't one assigned, it's an error
        if (thisBeam->aDUNum == 0) {
            ShowSevereError(state,
                            format("{}No matching Air Distribution Unit, for Unit = [{},{}].", routineName, cCurrentModuleObject, thisBeam->name));
            ShowContinueError(state, format("...should have outlet node={}", state.dataLoopNodes->NodeID(thisBeam->airOutNodeNum)));
            ErrorsFound = true;
        } else {

            // Fill the Zone Equipment data with the supply air inlet node number of this unit.
            for (int ctrlZone = 1; ctrlZone <= state.dataGlobal->NumOfZones; ++ctrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(ctrlZone).IsControlled) {
                    continue;
                }
                for (int supAirIn = 1; supAirIn <= state.dataZoneEquip->ZoneEquipConfig(ctrlZone).NumInletNodes; ++supAirIn) {
                    if (thisBeam->airOutNodeNum == state.dataZoneEquip->ZoneEquipConfig(ctrlZone).InletNode(supAirIn)) {
                        thisBeam->zoneIndex = ctrlZone;
                        thisBeam->zoneNodeIndex = state.dataZoneEquip->ZoneEquipConfig(ctrlZone).ZoneNode;
                        thisBeam->ctrlZoneInNodeIndex = supAirIn;
                        state.dataZoneEquip->ZoneEquipConfig(ctrlZone).AirDistUnitCool(supAirIn).InNode = thisBeam->airInNodeNum;
                        state.dataZoneEquip->ZoneEquipConfig(ctrlZone).AirDistUnitCool(supAirIn).OutNode = thisBeam->airOutNodeNum;
                        state.dataDefineEquipment->AirDistUnit(thisBeam->aDUNum).TermUnitSizingNum =
                            state.dataZoneEquip->ZoneEquipConfig(ctrlZone).AirDistUnitCool(supAirIn).TermUnitSizingIndex;
                        thisBeam->termUnitSizingNum = state.dataDefineEquipment->AirDistUnit(thisBeam->aDUNum).TermUnitSizingNum;
                        state.dataDefineEquipment->AirDistUnit(thisBeam->aDUNum).ZoneEqNum = ctrlZone;
                        if (thisBeam->beamHeatingPresent) {
                            state.dataZoneEquip->ZoneEquipConfig(ctrlZone).AirDistUnitHeat(supAirIn).InNode = thisBeam->airInNodeNum;
                            state.dataZoneEquip->ZoneEquipConfig(ctrlZone).AirDistUnitHeat(supAirIn).OutNode = thisBeam->airOutNodeNum;
                        }
                        airNodeFound = true;
                        break;
                    }
                }
            }
        }
        if (!airNodeFound) {
            ShowSevereError(state, format("The outlet air node from the {} = {}", cCurrentModuleObject, thisBeam->name));
            ShowContinueError(state, format("did not have a matching Zone Equipment Inlet Node, Node ={}", state.dataIPShortCut->cAlphaArgs(5)));
            ErrorsFound = true;
        }

        if (found && !ErrorsFound) {
            state.dataFourPipeBeam->FourPipeBeams.push_back(thisBeam);
            return thisBeam;
        } else {
            ShowFatalError(state, format("{}Errors found in getting input. Preceding conditions cause termination.", routineName));
            return nullptr;
        }
    }

    int HVACFourPipeBeam::getAirLoopNum()
    {
        return airLoopNum;
    }

    int HVACFourPipeBeam::getZoneIndex()
    {
        return zoneIndex;
    }

    Real64 HVACFourPipeBeam::getPrimAirDesignVolFlow()
    {
        return vDotDesignPrimAir;
    }

    int HVACFourPipeBeam::getTermUnitSizingIndex()
    {
        return termUnitSizingNum;
    }

    void HVACFourPipeBeam::simulate(EnergyPlusData &state,
                                    bool const FirstHVACIteration, // TRUE if first HVAC iteration in time step
                                    Real64 &NonAirSysOutput        // convective cooling by the beam system [W]
    )
    {

        // initialize the unit
        this->init(state, FirstHVACIteration);

        // control and simulate the beam
        if (!this->mySizeFlag) {
            this->control(state, FirstHVACIteration, NonAirSysOutput);

            // Update the current unit's outlet nodes.
            this->update(state);

            // Fill the report variables.
            this->report(state);
        }
    }

    void HVACFourPipeBeam::init(EnergyPlusData &state,
                                bool const FirstHVACIteration // TRUE if first air loop solution this HVAC step
    )
    {

        // Using
        using DataZoneEquipment::CheckZoneEquipmentList;
        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using PlantUtilities::SetComponentFlowRate;

        static constexpr std::string_view routineName("HVACFourPipeBeam::init");

        if (this->plantLoopScanFlag && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            if (this->beamCoolingPresent) {
                ScanPlantLoopsForObject(state,
                                        this->name,
                                        DataPlant::PlantEquipmentType::FourPipeBeamAirTerminal,
                                        this->cWplantLoc,
                                        errFlag,
                                        _,
                                        _,
                                        _,
                                        this->cWInNodeNum,
                                        _);
                if (errFlag) {
                    ShowFatalError(state, format("{} Program terminated for previous conditions.", routineName));
                }
            }
            if (this->beamHeatingPresent) {
                ScanPlantLoopsForObject(state,
                                        this->name,
                                        DataPlant::PlantEquipmentType::FourPipeBeamAirTerminal,
                                        this->hWplantLoc,
                                        errFlag,
                                        _,
                                        _,
                                        _,
                                        this->hWInNodeNum,
                                        _);
                if (errFlag) {
                    ShowFatalError(state, format("{} Program terminated for previous conditions.", routineName));
                }
            }
            this->plantLoopScanFlag = false;
        }

        if (!this->zoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            // Check to see if there is a Air Distribution Unit on the Zone Equipment List
            if (this->aDUNum != 0) {
                if (!CheckZoneEquipmentList(state, "ZONEHVAC:AIRDISTRIBUTIONUNIT", state.dataDefineEquipment->AirDistUnit(this->aDUNum).Name)) {
                    ShowSevereError(state,
                                    format("{}: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                           routineName,
                                           state.dataDefineEquipment->AirDistUnit(this->aDUNum).Name));
                    ShowContinueError(state, format("...Unit=[{},{}] will not be simulated.", this->unitType, this->name));
                }
                this->zoneEquipmentListChecked = true;
            }
        }

        if (!state.dataGlobal->SysSizingCalc && this->mySizeFlag && !this->plantLoopScanFlag) {
            //    if ( SysSizingCalc && this->mySizeFlag && ! this->plantLoopScanFlag ) {
            this->airLoopNum = state.dataZoneEquip->ZoneEquipConfig(this->zoneIndex).InletNodeAirLoopNum(this->ctrlZoneInNodeIndex);
            state.dataDefineEquipment->AirDistUnit(this->aDUNum).AirLoopNum = this->airLoopNum;
            this->set_size(state);          // calculate autosize values (in any) and convert volume flow rates to mass flow rates
            if (this->beamCoolingPresent) { // initialize chilled water design mass flow rate in plant routines
                InitComponentNodes(state, 0.0, this->mDotDesignCW, this->cWInNodeNum, this->cWOutNodeNum);
            }
            if (this->beamHeatingPresent) { // initialize hot water design mass flow rate in plant routines
                InitComponentNodes(state, 0.0, this->mDotDesignHW, this->hWInNodeNum, this->hWOutNodeNum);
            }
            this->mySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && this->myEnvrnFlag) {

            state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMax = this->mDotDesignPrimAir;
            state.dataLoopNodes->Node(this->airOutNodeNum).MassFlowRateMax = this->mDotDesignPrimAir;
            state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(this->airOutNodeNum).MassFlowRateMin = 0.0;

            if (this->beamCoolingPresent) { // initialize chilled water design mass flow rate in plant routines
                InitComponentNodes(state, 0.0, this->mDotDesignCW, this->cWInNodeNum, this->cWOutNodeNum);
            }
            if (this->beamHeatingPresent) { // initialize hot water design mass flow rate in plant routines
                InitComponentNodes(state, 0.0, this->mDotDesignHW, this->hWInNodeNum, this->hWOutNodeNum);
            }

            if (this->airLoopNum == 0) { // fill air loop index
                if (this->zoneIndex > 0 && this->ctrlZoneInNodeIndex > 0) {
                    this->airLoopNum = state.dataZoneEquip->ZoneEquipConfig(this->zoneIndex).InletNodeAirLoopNum(this->ctrlZoneInNodeIndex);
                }
            }

            this->myEnvrnFlag = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->myEnvrnFlag = true;
        }

        // Do the start of HVAC time step initializations
        if (FirstHVACIteration) {
            // check availability schedules and set flags
            this->airAvailable = (this->airAvailSched->getCurrentVal() > 0.0);
            this->coolingAvailable = (this->airAvailable && beamCoolingPresent && (this->coolingAvailSched->getCurrentVal() > 0.0));
            this->heatingAvailable = (this->airAvailable && beamHeatingPresent && (this->heatingAvailSched->getCurrentVal() > 0.0));

            // check for upstream zero flow. If nonzero and air available, set primary flow to max
            if (this->airAvailable && state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRate > 0.0) {
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRate = this->mDotDesignPrimAir;
            } else {
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRate = 0.0;
            }
            // reset the max and min avail flows
            if (this->airAvailable && state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMaxAvail > 0.0) {
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMaxAvail = this->mDotDesignPrimAir;
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMinAvail = this->mDotDesignPrimAir;
            } else {
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMinAvail = 0.0;
            }
        }

        // do these initializations every time step
        if (beamCoolingPresent) {
            this->cWTempIn = state.dataLoopNodes->Node(this->cWInNodeNum).Temp;
            this->cWTempOut = this->cWTempIn;
        }
        if (beamHeatingPresent) {
            this->hWTempIn = state.dataLoopNodes->Node(this->hWInNodeNum).Temp;
            this->hWTempOut = this->hWTempIn;
        }
        this->mDotSystemAir = state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(this->airInNodeNum).MassFlowRate = this->mDotSystemAir;
        this->tDBZoneAirTemp = state.dataLoopNodes->Node(this->zoneNodeIndex).Temp;
        this->tDBSystemAir = state.dataLoopNodes->Node(this->airInNodeNum).Temp;
        this->cpZoneAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->zoneNodeIndex).HumRat);
        this->cpSystemAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->airInNodeNum).HumRat);
        this->qDotBeamCooling = 0.0;
        this->qDotBeamHeating = 0.0;
        this->supAirCoolingRate = 0.0;
        this->supAirHeatingRate = 0.0;
        this->beamCoolingRate = 0.0;
        this->beamHeatingRate = 0.0;
        this->primAirFlow = 0.0;

    } // init

    void HVACFourPipeBeam::set_size(EnergyPlusData &state)
    {

        // Using
        using namespace DataSizing;
        using PlantUtilities::MyPlantSizingIndex;
        using PlantUtilities::RegisterPlantCompDesignFlow;
        using Psychrometrics::PsyCpAirFnW;
        using namespace std::placeholders;

        static constexpr std::string_view routineName("HVACFourPipeBeam::set_size ");

        bool ErrorsFound = false;
        Real64 rho;                     // local fluid density
        bool noHardSizeAnchorAvailable; // aid for complex logic surrounding mix of hard size and autosizes
        Real64 cpAir = 0.0;
        Real64 ErrTolerance = 0.001;

        Real64 mDotAirSolutionHeating = 0.0;
        Real64 mDotAirSolutionCooling = 0.0;
        Real64 originalTermUnitSizeMaxVDot = 0.0;
        Real64 originalTermUnitSizeCoolVDot = 0.0;
        Real64 originalTermUnitSizeHeatVDot = 0.0;

        // convert rated primary flow rate to mass flow rate using standard pressure and dry air at 20.0
        this->mDotNormRatedPrimAir = this->vDotNormRatedPrimAir * state.dataEnvrn->rhoAirSTP;

        noHardSizeAnchorAvailable = false;

        if (state.dataSize->CurTermUnitSizingNum > 0) {
            originalTermUnitSizeMaxVDot = std::max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow,
                                                   state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow);
            originalTermUnitSizeCoolVDot = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow;
            originalTermUnitSizeHeatVDot = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow;
        }

        if (this->totBeamLengthWasAutosized && this->vDotDesignPrimAirWasAutosized && this->vDotDesignCWWasAutosized &&
            this->vDotDesignHWWasAutosized) {
            noHardSizeAnchorAvailable = true;
        } else if (this->totBeamLengthWasAutosized && this->vDotDesignPrimAirWasAutosized && this->vDotDesignCWWasAutosized && !beamHeatingPresent) {
            noHardSizeAnchorAvailable = true;
        } else if (this->totBeamLengthWasAutosized && this->vDotDesignPrimAirWasAutosized && !this->beamCoolingPresent &&
                   this->vDotDesignHWWasAutosized) {
            noHardSizeAnchorAvailable = true;
        } else if (!this->totBeamLengthWasAutosized) { // the simplest case is where length is not autosized
            // use the normalized rated values (likely defaulted ) with length to calculate any that are autosized
            if (this->vDotDesignPrimAirWasAutosized) {
                this->vDotDesignPrimAir = this->vDotNormRatedPrimAir * this->totBeamLength;
            }
            if (this->vDotDesignCWWasAutosized) {
                this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
            }
            if (vDotDesignHWWasAutosized) {
                this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
            }
        } else { // need to find beam length
            // the next simplest case is if the supply air rate is given
            if (!this->vDotDesignPrimAirWasAutosized) { //
                // find length from air flow rate and then proceed
                this->totBeamLength = this->vDotDesignPrimAir / this->vDotNormRatedPrimAir;
                if (this->vDotDesignCWWasAutosized) {
                    this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
                }
                if (vDotDesignHWWasAutosized) {
                    this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
                }
            } else {                                                               // both air and length are autosized
                if (this->beamCoolingPresent && !this->vDotDesignCWWasAutosized) { // we have a chilled water flow rate to use
                    this->totBeamLength = this->vDotDesignCW / this->vDotNormRatedCW;
                    this->vDotDesignPrimAir = this->vDotNormRatedPrimAir * this->totBeamLength;
                    if (vDotDesignHWWasAutosized) {
                        this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
                    }
                } else if (this->beamHeatingPresent && !this->vDotDesignHWWasAutosized) { // we have a hot water flow rate to use
                    this->totBeamLength = this->vDotDesignHW / this->vDotNormRatedHW;
                    this->vDotDesignPrimAir = this->vDotNormRatedPrimAir * this->totBeamLength;
                    if (this->vDotDesignCWWasAutosized) { // don't think it can come here but...
                        this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
                    }
                } else {
                    // should not come here, developer exception
                }
            } // no air flow rate
        } // no beam length

        if (noHardSizeAnchorAvailable && (state.dataSize->CurZoneEqNum > 0) &&
            (state.dataSize->CurTermUnitSizingNum > 0)) { // need to use central sizing results to calculate

            // set up for solver

            CheckZoneSizing(state, this->unitType, this->name);
            // minimum flow rate is from air flow rate on the terminal unit final zone size ( typically ventilation minimum and may be too low)
            Real64 minFlow(0.0);
            Real64 maxFlowCool(0.0);
            minFlow = std::min(state.dataEnvrn->StdRhoAir * originalTermUnitSizeMaxVDot,
                               state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).MinOA * state.dataEnvrn->StdRhoAir);
            minFlow = std::max(0.0, minFlow);
            // max flow is as if the air supply was sufficient to provide all the conditioning

            if (beamCoolingPresent) {
                cpAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInHumRatTU);

                if ((state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak -
                     state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU) >
                    2.0) { // avoid div by zero and blow up
                    maxFlowCool = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolLoad /
                                  (cpAir * (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak -
                                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU));
                } else {
                    maxFlowCool = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolLoad / (cpAir * 2.0);
                }
                if (minFlow * 3.0 >= maxFlowCool) {
                    minFlow = maxFlowCool / 3.0; // make sure min is significantly lower than max.
                }

                int pltSizCoolNum = MyPlantSizingIndex(state, "four pipe beam unit", this->name, this->cWInNodeNum, this->cWOutNodeNum, ErrorsFound);
                if (pltSizCoolNum == 0) {
                    ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                    ShowContinueError(state, format("Occurs in {} Object={}", this->unitType, this->name));
                    ErrorsFound = true;
                } else {
                    this->cWTempIn = state.dataSize->PlantSizData(pltSizCoolNum).ExitTemp;
                }
                this->mDotHW = 0.0;
                this->tDBZoneAirTemp = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak;
                this->tDBSystemAir = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU;
                this->cpZoneAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneHumRatAtCoolPeak);
                this->cpSystemAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInHumRatTU);
                this->qDotZoneReq = -1.0 * state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolLoad;
                this->qDotZoneToCoolSetPt = -1.0 * state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolLoad;
                this->airAvailable = true;
                this->coolingAvailable = true;
                this->heatingAvailable = false;
                auto f = [&state, this](Real64 const airFlow) {
                    static constexpr std::string_view routineName("Real64 HVACFourPipeBeam::residualSizing ");
                    this->mDotSystemAir = airFlow;
                    this->vDotDesignPrimAir = this->mDotSystemAir / state.dataEnvrn->StdRhoAir;
                    this->totBeamLength = this->vDotDesignPrimAir / this->vDotNormRatedPrimAir;
                    if (this->vDotDesignCWWasAutosized) {
                        this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
                        Real64 const rho =
                            state.dataPlnt->PlantLoop(this->cWplantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, routineName);
                        this->mDotNormRatedCW = this->vDotNormRatedCW * rho;
                        this->mDotCW = this->vDotDesignCW * rho;
                        if (this->beamCoolingPresent) {
                            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum);
                        }
                    }
                    if (vDotDesignHWWasAutosized) {
                        this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
                        Real64 const rho =
                            state.dataPlnt->PlantLoop(this->hWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, routineName);
                        this->mDotNormRatedHW = this->vDotNormRatedHW * rho;
                        this->mDotHW = this->vDotDesignHW * rho;
                        if (this->beamHeatingPresent) {
                            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum);
                        }
                    }
                    this->calc(state);
                    if (this->qDotZoneReq != 0.0) {
                        return ((this->qDotZoneReq - this->qDotTotalDelivered) / this->qDotZoneReq);
                    } else {
                        return 1.0;
                    }
                };
                int SolFlag = 0;
                General::SolveRoot(state, ErrTolerance, 50, SolFlag, mDotAirSolutionCooling, f, minFlow, maxFlowCool);
                if (SolFlag == -1) {
                    ShowWarningError(state, format("Cooling load sizing search failed in four pipe beam unit called {}", this->name));
                    ShowContinueError(state, "  Iteration limit exceeded in calculating size for design cooling load");
                } else if (SolFlag == -2) {
                    ShowWarningError(state, format("Cooling load sizing search failed in four pipe beam unit called {}", this->name));
                    ShowContinueError(state, "  Bad size limits");
                }
            }

            if (beamHeatingPresent) {
                cpAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInHumRatTU);
                Real64 maxFlowHeat = 0.0;
                if ((state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU -
                     state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtHeatPeak) >
                    2.0) { // avoid div by zero and blow up
                    maxFlowHeat = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatLoad /
                                  (cpAir * (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU -
                                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtHeatPeak));
                } else {
                    maxFlowHeat = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatLoad / (cpAir * 2.0);
                }

                int pltSizHeatNum = MyPlantSizingIndex(state, "four pipe beam unit", this->name, this->hWInNodeNum, this->hWOutNodeNum, ErrorsFound);
                if (pltSizHeatNum == 0) {
                    ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                    ShowContinueError(state, format("Occurs in {} Object={}", this->unitType, this->name));
                    ErrorsFound = true;
                } else {
                    this->hWTempIn = state.dataSize->PlantSizData(pltSizHeatNum).ExitTemp;
                }
                this->mDotCW = 0.0;
                this->tDBZoneAirTemp = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtHeatPeak;
                this->tDBSystemAir = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU;
                this->cpZoneAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneHumRatAtHeatPeak);
                this->cpSystemAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInHumRatTU);
                this->qDotZoneReq = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatLoad;
                this->qDotZoneToHeatSetPt = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatLoad;
                this->airAvailable = true;
                this->heatingAvailable = true;
                this->coolingAvailable = false;
                auto f = [&state, this](Real64 const airFlow) {
                    static constexpr std::string_view routineName("Real64 HVACFourPipeBeam::residualSizing ");
                    this->mDotSystemAir = airFlow;
                    this->vDotDesignPrimAir = this->mDotSystemAir / state.dataEnvrn->StdRhoAir;
                    this->totBeamLength = this->vDotDesignPrimAir / this->vDotNormRatedPrimAir;
                    if (this->vDotDesignCWWasAutosized) {
                        this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
                        Real64 const rho =
                            state.dataPlnt->PlantLoop(this->cWplantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, routineName);
                        this->mDotNormRatedCW = this->vDotNormRatedCW * rho;
                        this->mDotCW = this->vDotDesignCW * rho;
                        if (this->beamCoolingPresent) {
                            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum);
                        }
                    }
                    if (vDotDesignHWWasAutosized) {
                        this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
                        Real64 const rho =
                            state.dataPlnt->PlantLoop(this->hWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, routineName);
                        this->mDotNormRatedHW = this->vDotNormRatedHW * rho;
                        this->mDotHW = this->vDotDesignHW * rho;
                        if (this->beamHeatingPresent) {
                            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum);
                        }
                    }
                    this->calc(state);
                    if (this->qDotZoneReq != 0.0) {
                        return ((this->qDotZoneReq - this->qDotTotalDelivered) / this->qDotZoneReq);
                    } else {
                        return 1.0;
                    }
                };
                int SolFlag = 0;
                General::SolveRoot(state, ErrTolerance, 50, SolFlag, mDotAirSolutionHeating, f, 0.0, maxFlowHeat);
                if (SolFlag == -1) {
                    ShowWarningError(state, format("Heating load sizing search failed in four pipe beam unit called {}", this->name));
                    ShowContinueError(state, "  Iteration limit exceeded in calculating size for design heating load");
                } else if (SolFlag == -2) {
                    ShowWarningError(state, format("Heating load sizing search failed in four pipe beam unit called {}", this->name));
                    ShowContinueError(state, "  Bad size limits");
                }
            }

            // take the larger of heating and cooling
            this->mDotDesignPrimAir = std::max(mDotAirSolutionHeating, mDotAirSolutionCooling);
            // make sure this is higher than the zone OA requirement
            this->mDotDesignPrimAir =
                std::max(this->mDotDesignPrimAir,
                         state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).MinOA * state.dataEnvrn->StdRhoAir);
            this->vDotDesignPrimAir = this->mDotDesignPrimAir / state.dataEnvrn->StdRhoAir;
            this->totBeamLength = this->vDotDesignPrimAir / this->vDotNormRatedPrimAir;
            if (this->vDotDesignCWWasAutosized) {
                this->vDotDesignCW = this->vDotNormRatedCW * this->totBeamLength;
            }
            if (vDotDesignHWWasAutosized) {
                this->vDotDesignHW = this->vDotNormRatedHW * this->totBeamLength;
            }
        }
        // fill in mass flow rate versions of working variables (regardless of autosizing )
        this->mDotDesignPrimAir = this->vDotDesignPrimAir * state.dataEnvrn->StdRhoAir;

        if ((originalTermUnitSizeMaxVDot > 0.0) && (originalTermUnitSizeMaxVDot != this->vDotDesignPrimAir) && (state.dataSize->CurZoneEqNum > 0)) {
            if ((state.dataSize->SysSizingRunDone) && (this->airLoopNum > 0)) {
                // perturb system size to handle change in system size calculated without knowing about 4 pipe beam
                // Note that this approach is not necessarily appropriate for coincident system design option
                // and it might be moved to make such adjustments in SizingManager::ManageSystemSizingAdjustments()
                state.dataSize->FinalSysSizing(this->airLoopNum).DesMainVolFlow += (this->vDotDesignPrimAir - originalTermUnitSizeMaxVDot);
                state.dataSize->FinalSysSizing(this->airLoopNum).DesCoolVolFlow += (this->vDotDesignPrimAir - originalTermUnitSizeCoolVDot);
                state.dataSize->FinalSysSizing(this->airLoopNum).DesHeatVolFlow += (this->vDotDesignPrimAir - originalTermUnitSizeHeatVDot);
                state.dataSize->FinalSysSizing(this->airLoopNum).MassFlowAtCoolPeak +=
                    (this->vDotDesignPrimAir - originalTermUnitSizeCoolVDot) * state.dataEnvrn->StdRhoAir;

                BaseSizer::reportSizerOutput(state,
                                             this->unitType,
                                             this->name,
                                             "AirLoopHVAC Design Supply Air Flow Rate Adjustment [m3/s]",
                                             (this->vDotDesignPrimAir - originalTermUnitSizeMaxVDot));
            } else {
                ShowSevereError(state, "Four pipe beam requires system sizing. Turn on system sizing.");
                ShowFatalError(state, "Program terminating due to previous errors");
            }
        }

        if (this->beamCoolingPresent) {
            rho = state.dataPlnt->PlantLoop(this->cWplantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, routineName);
            this->mDotNormRatedCW = this->vDotNormRatedCW * rho;
            this->mDotDesignCW = this->vDotDesignCW * rho;
            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotDesignCW, this->cWInNodeNum, this->cWOutNodeNum);
        }
        if (this->beamHeatingPresent) {
            rho = state.dataPlnt->PlantLoop(this->hWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, routineName);
            this->mDotNormRatedHW = this->vDotNormRatedHW * rho;
            this->mDotDesignHW = this->vDotDesignHW * rho;
            PlantUtilities::InitComponentNodes(state, 0.0, this->mDotDesignHW, this->hWInNodeNum, this->hWOutNodeNum);
        }

        // report final sizes if autosized
        if (this->vDotDesignPrimAirWasAutosized) {
            BaseSizer::reportSizerOutput(state, this->unitType, this->name, "Supply Air Flow Rate [m3/s]", this->vDotDesignPrimAir);
        }
        if (this->vDotDesignCWWasAutosized) {
            BaseSizer::reportSizerOutput(state, this->unitType, this->name, "Maximum Total Chilled Water Flow Rate [m3/s]", this->vDotDesignCW);
        }
        if (this->vDotDesignHWWasAutosized) {
            BaseSizer::reportSizerOutput(state, this->unitType, this->name, "Maximum Total Hot Water Flow Rate [m3/s]", this->vDotDesignHW);
        }
        if (this->totBeamLengthWasAutosized) {
            BaseSizer::reportSizerOutput(state, this->unitType, this->name, "Zone Total Beam Length [m]", this->totBeamLength);
        }
        // save the design water volume flow rate for use by the water loop sizing algorithms
        if (this->vDotDesignCW > 0.0 && this->beamCoolingPresent) {
            RegisterPlantCompDesignFlow(state, this->cWInNodeNum, this->vDotDesignCW);
        }
        if (this->vDotDesignHW > 0.0 && this->beamHeatingPresent) {
            RegisterPlantCompDesignFlow(state, this->hWInNodeNum, this->vDotDesignHW);
        }
        if (ErrorsFound) {
            ShowFatalError(state, "Preceding four pipe beam sizing errors cause program termination");
        }

    } // set_size

    void HVACFourPipeBeam::control(EnergyPlusData &state,
                                   [[maybe_unused]] bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                   Real64 &NonAirSysOutput                         // convective cooling by the beam system [W]
    )
    {

        // Using/Aliasing
        using namespace DataZoneEnergyDemands;
        using PlantUtilities::SetComponentFlowRate;
        using namespace std::placeholders;

        int SolFlag;
        Real64 ErrTolerance;

        NonAirSysOutput = 0.0; // initialize

        if (this->mDotSystemAir < HVAC::VerySmallMassFlow ||
            (!this->airAvailable && !this->coolingAvailable && !this->heatingAvailable)) { // unit is off
            this->mDotHW = 0.0;
            if (this->beamHeatingPresent) {
                SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            }
            this->hWTempOut = this->hWTempIn;
            // assume if there is still flow that unit has an internal bypass and convector does not still heat
            this->mDotCW = 0.0;
            this->cWTempOut = this->cWTempIn;
            if (this->beamCoolingPresent) {
                SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            }
            // assume if there is still flow that unit has an internal bypass and convector does not still cool
            // don't even need to run calc
            return;
        }

        if (this->airAvailable && this->mDotSystemAir > HVAC::VerySmallMassFlow && !this->coolingAvailable && !this->heatingAvailable) {
            this->mDotHW = 0.0;
            if (this->beamHeatingPresent) {
                SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            }
            // assume if there is still flow that unit has an internal bypass and convector does not still heat
            this->hWTempOut = this->hWTempIn;
            this->mDotCW = 0.0;
            if (this->beamCoolingPresent) {
                SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            }
            // assume if there is still flow that unit has an internal bypass and convector does not still cool
            this->cWTempOut = this->cWTempIn;
            this->calc(state);

            return;
        }

        // get zone loads
        this->qDotZoneReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->zoneIndex).RemainingOutputRequired;
        this->qDotZoneToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->zoneIndex).RemainingOutputReqToHeatSP;
        this->qDotZoneToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(this->zoneIndex).RemainingOutputReqToCoolSP;

        // decide if beam is in heating or cooling

        this->qDotSystemAir = this->mDotSystemAir * ((this->cpSystemAir * this->tDBSystemAir) - (this->cpZoneAir * this->tDBZoneAirTemp));

        this->qDotBeamReq = this->qDotZoneReq - this->qDotSystemAir;

        if (this->qDotBeamReq < -HVAC::SmallLoad && this->coolingAvailable) { // beam cooling needed
            // first calc with max chilled water flow
            this->mDotHW = 0.0;
            if (this->beamHeatingPresent) {
                SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            }
            this->hWTempOut = this->hWTempIn;
            this->mDotCW = this->mDotDesignCW;
            this->calc(state);
            if (this->qDotBeamCooling < (qDotBeamReq - HVAC::SmallLoad)) {
                // can overcool, modulate chilled water flow rate to meet load
                this->qDotBeamCoolingMax = this->qDotBeamCooling;
                ErrTolerance = 0.01;
                auto f = [&state, this](Real64 const cWFlow) {
                    this->mDotHW = 0.0;
                    this->mDotCW = cWFlow;
                    this->calc(state);
                    if (this->qDotBeamCoolingMax != 0.0) {
                        return (((this->qDotZoneToCoolSetPt - this->qDotSystemAir) - this->qDotBeamCooling) / this->qDotBeamCoolingMax);
                    } else {
                        return 1.0;
                    }
                };
                General::SolveRoot(state, ErrTolerance, 50, SolFlag, this->mDotCW, f, 0.0, this->mDotDesignCW);
                if (SolFlag == -1) {
                    // ShowWarningError( "Cold water control failed in four pipe beam unit called " + this->name );
                    // ShowContinueError(state,  "  Iteration limit exceeded in calculating cold water mass flow rate" );
                } else if (SolFlag == -2) {
                    // ShowWarningError( "Cold water control failed in four pipe beam unit called " + this->name );
                    // ShowContinueError(state,  "  Bad cold water flow limits" );
                }
                this->calc(state);
                NonAirSysOutput = this->qDotBeamCooling;
                return;
            } else { // can run flat out without overcooling, which we just did
                NonAirSysOutput = this->qDotBeamCooling;
                return;
            }

        } else if (qDotBeamReq > HVAC::SmallLoad && this->heatingAvailable) { // beam heating needed
            // first calc with max hot water flow
            this->mDotCW = 0.0;
            if (this->beamCoolingPresent) {
                SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            }
            this->cWTempOut = this->cWTempIn;
            this->mDotHW = this->mDotDesignHW;
            this->calc(state);
            if (this->qDotBeamHeating > (qDotBeamReq + HVAC::SmallLoad)) {
                this->qDotBeamHeatingMax = this->qDotBeamHeating;
                // can overheat, modulate hot water flow to meet load
                ErrTolerance = 0.01;
                auto f = [&state, this](Real64 const hWFlow) {
                    this->mDotHW = hWFlow;
                    this->mDotCW = 0.0;
                    this->calc(state);
                    if (this->qDotBeamHeatingMax != 0.0) {
                        return (((this->qDotZoneToHeatSetPt - this->qDotSystemAir) - this->qDotBeamHeating) / this->qDotBeamHeatingMax);
                    } else {
                        return 1.0;
                    }
                };
                General::SolveRoot(state, ErrTolerance, 50, SolFlag, this->mDotHW, f, 0.0, this->mDotDesignHW);
                if (SolFlag == -1) {
                    // ShowWarningError( "Hot water control failed in four pipe beam unit called " + this->name );
                    // ShowContinueError(state,  "  Iteration limit exceeded in calculating hot water mass flow rate" );
                } else if (SolFlag == -2) {
                    // ShowWarningError( "Hot water control failed in four pipe beam called " + this->name );
                    // ShowContinueError(state,  "  Bad hot water flow limits" );
                }
                this->calc(state);
                NonAirSysOutput = this->qDotBeamHeating;
                return;

            } else { // can run flat out without overheating, which we just did
                NonAirSysOutput = this->qDotBeamHeating;
                return;
            }

        } else {
            this->mDotHW = 0.0;
            if (this->beamHeatingPresent) {
                SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            }
            this->hWTempOut = this->hWTempIn;
            // assume if there is still flow that unit has an internal bypass and convector does not still heat
            this->mDotCW = 0.0;
            this->cWTempOut = this->cWTempIn;
            if (this->beamCoolingPresent) {
                SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            }
            // assume if there is still flow that unit has an internal bypass and convector does not still cool
            // don't even need to run calc
            return;
        }
    }

    void HVACFourPipeBeam::calc(EnergyPlusData &state)
    {

        // Using/Aliasing
        using PlantUtilities::SetComponentFlowRate;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName("HVACFourPipeBeam::calc ");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 fModCoolCWMdot;  // Cooling capacity modification factor function of chilled water flow rate
        Real64 fModCoolDeltaT;  // Cooling capacity modification factor function of air-water temperature difference
        Real64 fModCoolAirMdot; // Cooling capacity modification factor function of primary air flow rate
        Real64 fModHeatHWMdot;  // Heating capacity modification factor function of hot water flow rate
        Real64 fModHeatDeltaT;  // Heating capacity modification factor function of water - air temperature difference
        Real64 fModHeatAirMdot; // Heating capacity modification factor function of primary air flow rate
        Real64 cp;              // local fluid specific heat

        this->qDotBeamHeating = 0.0;
        this->qDotBeamCooling = 0.0;
        this->qDotSystemAir = this->mDotSystemAir * ((this->cpSystemAir * this->tDBSystemAir) - (this->cpZoneAir * this->tDBZoneAirTemp));

        if (this->coolingAvailable && this->mDotCW > HVAC::VerySmallMassFlow) {
            // test chilled water flow against plant, it might not all be available
            SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            fModCoolCWMdot =
                Curve::CurveValue(state, this->modCoolingQdotCWFlowFuncNum, ((this->mDotCW / this->totBeamLength) / this->mDotNormRatedCW));
            fModCoolDeltaT =
                Curve::CurveValue(state, this->modCoolingQdotDeltaTFuncNum, ((this->tDBZoneAirTemp - this->cWTempIn) / this->deltaTempRatedCooling));
            fModCoolAirMdot = Curve::CurveValue(
                state, this->modCoolingQdotAirFlowFuncNum, ((this->mDotSystemAir / this->totBeamLength) / this->mDotNormRatedPrimAir));
            this->qDotBeamCooling = -1.0 * this->qDotNormRatedCooling * fModCoolDeltaT * fModCoolAirMdot * fModCoolCWMdot * this->totBeamLength;
            cp = state.dataPlnt->PlantLoop(this->cWplantLoc.loopNum).glycol->getSpecificHeat(state, this->cWTempIn, routineName);
            if (this->mDotCW > 0.0) {
                this->cWTempOut = this->cWTempIn - (this->qDotBeamCooling / (this->mDotCW * cp));
            } else {
                this->cWTempOut = this->cWTempIn;
            }
            // check if non physical temperature rise, can't be warmer than air
            if (this->cWTempOut > (std::max(this->tDBSystemAir, this->tDBZoneAirTemp) - 1.0)) {
                // throw recurring warning as this indicates a problem in beam model input
                ShowRecurringWarningErrorAtEnd(state,
                                               std::string{routineName} + " four pipe beam name " + this->name +
                                                   ", chilled water outlet temperature is too warm. Capacity was limited. check beam capacity input ",
                                               this->cWTempOutErrorCount,
                                               this->cWTempOut,
                                               this->cWTempOut);
                //  restrict it within 1.0 C of warmest air and recalculate cooling
                this->cWTempOut = (std::max(this->tDBSystemAir, this->tDBZoneAirTemp) - 1.0);
                this->qDotBeamCooling = this->mDotCW * cp * (this->cWTempIn - this->cWTempOut);
            }
        } else {
            this->mDotCW = 0.0;
            if (this->beamCoolingPresent) {
                SetComponentFlowRate(state, this->mDotCW, this->cWInNodeNum, this->cWOutNodeNum, this->cWplantLoc);
            }
            this->cWTempOut = this->cWTempIn;
            this->qDotBeamCooling = 0.0;
        }
        if (this->heatingAvailable && this->mDotHW > HVAC::VerySmallMassFlow) {
            // test hot water flow against plant, it might not all be available
            SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            fModHeatHWMdot =
                Curve::CurveValue(state, this->modHeatingQdotHWFlowFuncNum, ((this->mDotHW / this->totBeamLength) / this->mDotNormRatedHW));
            fModHeatDeltaT =
                Curve::CurveValue(state, this->modHeatingQdotDeltaTFuncNum, ((this->hWTempIn - this->tDBZoneAirTemp) / this->deltaTempRatedHeating));
            fModHeatAirMdot = Curve::CurveValue(
                state, this->modHeatingQdotAirFlowFuncNum, ((this->mDotSystemAir / this->totBeamLength) / this->mDotNormRatedPrimAir));
            this->qDotBeamHeating = this->qDotNormRatedHeating * fModHeatDeltaT * fModHeatAirMdot * fModHeatHWMdot * this->totBeamLength;
            cp = state.dataPlnt->PlantLoop(this->hWplantLoc.loopNum).glycol->getSpecificHeat(state, this->hWTempIn, routineName);
            if (this->mDotHW > 0.0) {
                this->hWTempOut = this->hWTempIn - (this->qDotBeamHeating / (this->mDotHW * cp));
            } else {
                this->hWTempOut = this->hWTempIn;
            }
            // check if non physical temperature drop, can't be cooler than air
            if (this->hWTempOut < (std::min(this->tDBSystemAir, this->tDBZoneAirTemp) + 1.0)) {
                // throw recurring warning as this indicates a problem in beam model input
                ShowRecurringWarningErrorAtEnd(state,
                                               std::string{routineName} + " four pipe beam name " + this->name +
                                                   ", hot water outlet temperature is too cool. Capacity was limited. check beam capacity input ",
                                               this->hWTempOutErrorCount,
                                               this->hWTempOut,
                                               this->hWTempOut);
                //  restrict it within 1.0 C of warmest air and recalculate cooling
                this->hWTempOut = (std::min(this->tDBSystemAir, this->tDBZoneAirTemp) + 1.0);
                this->qDotBeamHeating = this->mDotHW * cp * (this->hWTempIn - this->hWTempOut);
            }
        } else {
            this->mDotHW = 0.0;
            if (this->beamHeatingPresent) {
                SetComponentFlowRate(state, this->mDotHW, this->hWInNodeNum, this->hWOutNodeNum, this->hWplantLoc);
            }
            this->hWTempOut = this->hWTempIn;
            this->qDotBeamHeating = 0.0;
        }

        this->qDotTotalDelivered = this->qDotSystemAir + this->qDotBeamCooling + this->qDotBeamHeating;
    }

    void HVACFourPipeBeam::update(EnergyPlusData &state) const // update node date elsewhere in EnergyPlus, does not change state of this
    {
        auto &Node(state.dataLoopNodes->Node);

        using PlantUtilities::SafeCopyPlantNode;

        // Set the outlet air nodes of the unit; note that all quantities are unchanged from inlet to outlet
        Node(this->airOutNodeNum).MassFlowRate = Node(this->airInNodeNum).MassFlowRate;
        Node(this->airOutNodeNum).Temp = Node(this->airInNodeNum).Temp;
        Node(this->airOutNodeNum).HumRat = Node(this->airInNodeNum).HumRat;
        Node(this->airOutNodeNum).Enthalpy = Node(this->airInNodeNum).Enthalpy;
        Node(this->airOutNodeNum).Quality = Node(this->airInNodeNum).Quality;
        Node(this->airOutNodeNum).Press = Node(this->airInNodeNum).Press;
        Node(this->airOutNodeNum).MassFlowRateMin = Node(this->airInNodeNum).MassFlowRateMin;
        Node(this->airOutNodeNum).MassFlowRateMax = Node(this->airInNodeNum).MassFlowRateMax;
        Node(this->airOutNodeNum).MassFlowRateMinAvail = Node(this->airInNodeNum).MassFlowRateMinAvail;
        Node(this->airOutNodeNum).MassFlowRateMaxAvail = Node(this->airInNodeNum).MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            Node(this->airOutNodeNum).CO2 = Node(this->airInNodeNum).CO2;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            Node(this->airOutNodeNum).GenContam = Node(this->airInNodeNum).GenContam;
        }

        // Set the outlet water nodes for the unit

        if (this->beamCoolingPresent) {
            SafeCopyPlantNode(state, this->cWInNodeNum, this->cWOutNodeNum);
            Node(this->cWOutNodeNum).Temp = this->cWTempOut;
        }
        if (this->beamHeatingPresent) {
            SafeCopyPlantNode(state, this->hWInNodeNum, this->hWOutNodeNum);
            Node(this->hWOutNodeNum).Temp = this->hWTempOut;
        }
    }

    void HVACFourPipeBeam::report(EnergyPlusData &state) // fill out local output variables for reporting
    {

        Real64 ReportingConstant;

        ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        if (this->beamCoolingPresent) {
            this->beamCoolingRate = std::abs(this->qDotBeamCooling); // report var has positive sign convention
            this->beamCoolingEnergy = this->beamCoolingRate * ReportingConstant;
        }
        if (this->beamHeatingPresent) {
            this->beamHeatingRate = this->qDotBeamHeating;
            this->beamHeatingEnergy = this->beamHeatingRate * ReportingConstant;
        }
        if (qDotSystemAir <= 0.0) { // cooling
            this->supAirCoolingRate = std::abs(this->qDotSystemAir);
            this->supAirHeatingRate = 0.0;
        } else {
            this->supAirHeatingRate = this->qDotSystemAir;
            this->supAirCoolingRate = 0.0;
        }
        this->supAirCoolingEnergy = this->supAirCoolingRate * ReportingConstant;
        this->supAirHeatingEnergy = this->supAirHeatingRate * ReportingConstant;

        this->primAirFlow = this->mDotSystemAir / state.dataEnvrn->StdRhoAir;

        this->CalcOutdoorAirVolumeFlowRate(state);
    }

    void HVACFourPipeBeam::CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state)
    {
        // calculates zone outdoor air volume flow rate using the supply air flow rate and OA fraction
        if (this->airLoopNum > 0) {
            this->OutdoorAirFlowRate = (state.dataLoopNodes->Node(this->airOutNodeNum).MassFlowRate / state.dataEnvrn->StdRhoAir) *
                                       state.dataAirLoop->AirLoopFlow(this->airLoopNum).OAFrac;
        } else {
            this->OutdoorAirFlowRate = 0.0;
        }
    }

    void HVACFourPipeBeam::reportTerminalUnit(EnergyPlusData &state)
    {
        // populate the predefined equipment summary report related to air terminals
        auto &orp = state.dataOutRptPredefined;
        auto &adu = state.dataDefineEquipment->AirDistUnit(this->aDUNum);
        if (!state.dataSize->TermUnitFinalZoneSizing.empty()) {
            auto &sizing = state.dataSize->TermUnitFinalZoneSizing(adu.TermUnitSizingNum);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlow, adu.Name, sizing.DesCoolVolFlowMin);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOutdoorFlow, adu.Name, sizing.MinOA);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupCoolingSP, adu.Name, sizing.CoolDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupHeatingSP, adu.Name, sizing.HeatDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatingCap, adu.Name, sizing.DesHeatLoad);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolingCap, adu.Name, sizing.DesCoolLoad);
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermTypeInp, adu.Name, "AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPrimFlow, adu.Name, this->vDotNormRatedPrimAir);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSecdFlow, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMaxFlowReh, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOAflowSch, adu.Name, "n/a");
        if (this->beamHeatingPresent) {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, "Included");
        } else {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, "None");
        }

        if (this->beamCoolingPresent) {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, "Included");
        } else {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, "None");
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanName, adu.Name, "n/a");
    }

} // namespace FourPipeBeam

} // namespace EnergyPlus
