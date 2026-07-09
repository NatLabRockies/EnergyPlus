// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus { // NOLINT(modernize-concat-nested-namespaces) // TODO: Take out this lint when we want to apply formatting for nested
                       // namespacing

namespace HeatingCoils {
    // Module containing the HeatingCoil simulation routines other than the Water coils

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   May 2000
    //       MODIFIED       Therese Stovall June 2008 to add references to refrigeration condensers

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the HeatingCoil System Component

    void SimulateHeatingCoilComponents(EnergyPlusData &state,
                                       std::string_view CompName,
                                       bool const FirstHVACIteration,
                                       ObjexxFCL::Optional<Real64 const> QCoilReq, // coil load to be met
                                       ObjexxFCL::Optional_int CompIndex,
                                       ObjexxFCL::Optional<Real64> QCoilActual, // coil load actually delivered returned to calling component
                                       ObjexxFCL::Optional_bool_const SuppHeat, // True if current heating coil is a supplemental heating coil
                                       ObjexxFCL::Optional<HVAC::FanOp const> fanOpMode, // fan operating mode, FanOp::Cycling or FanOp::Continuous
                                       ObjexxFCL::Optional<Real64 const> PartLoadRatio,  // part-load ratio of heating coil
                                       ObjexxFCL::Optional_int StageNum,
                                       ObjexxFCL::Optional<Real64 const> SpeedRatio // Speed ratio of MultiStage heating coil
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages HeatingCoil component simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CoilNum(0);       // The HeatingCoil that you are currently loading input into
        Real64 QCoilActual2;  // coil load actually delivered returned from specific coil
        HVAC::FanOp fanOp;    // fan operating mode
        Real64 PartLoadFrac;  // part-load fraction of heating coil
        Real64 QCoilRequired; // local variable for optional argument

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        // Find the correct HeatingCoilNumber with the Coil Name
        if (present(CompIndex)) {
            if (CompIndex == 0) {
                CoilNum = Util::FindItemInList(CompName, state.dataHeatingCoils->HeatingCoil);
                if (CoilNum == 0) {
                    ShowFatalError(state, std::format("SimulateHeatingCoilComponents: Coil not found={}", CompName));
                }
                //    CompIndex=CoilNum
            } else {
                CoilNum = CompIndex;
                if (CoilNum > state.dataHeatingCoils->NumHeatingCoils || CoilNum < 1) {
                    ShowFatalError(state,
                                   std::format("SimulateHeatingCoilComponents: Invalid CompIndex passed={}, Number of Heating Coils={}, Coil name={}",
                                               CoilNum,
                                               state.dataHeatingCoils->NumHeatingCoils,
                                               CompName));
                }
                if (state.dataHeatingCoils->CheckEquipName(CoilNum)) {
                    if (!CompName.empty() && CompName != state.dataHeatingCoils->HeatingCoil(CoilNum).Name) {
                        ShowFatalError(
                            state,
                            std::format(
                                "SimulateHeatingCoilComponents: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                CoilNum,
                                CompName,
                                state.dataHeatingCoils->HeatingCoil(CoilNum).Name));
                    }
                    state.dataHeatingCoils->CheckEquipName(CoilNum) = false;
                }
            }
        } else {
            ShowSevereError(state, "SimulateHeatingCoilComponents: CompIndex argument not used.");
            ShowContinueError(state, std::format("..CompName = {}", CompName));
            ShowFatalError(state, "Preceding conditions cause termination.");
        }

        if (present(SuppHeat)) {
            state.dataHeatingCoils->CoilIsSuppHeater = SuppHeat;
        } else {
            state.dataHeatingCoils->CoilIsSuppHeater = false;
        }

        if (present(fanOpMode)) {
            fanOp = fanOpMode;
        } else {
            fanOp = HVAC::FanOp::Continuous;
        }

        if (present(PartLoadRatio)) {
            PartLoadFrac = PartLoadRatio;
        } else {
            PartLoadFrac = 1.0;
        }

        if (present(QCoilReq)) {
            QCoilRequired = QCoilReq;
        } else {
            QCoilRequired = Node::SensedLoadFlagValue;
        }

        // With the correct CoilNum Initialize
        InitHeatingCoil(state, CoilNum, FirstHVACIteration, QCoilRequired); // Initialize all HeatingCoil related parameters

        // Calculate the Correct HeatingCoil Model with the current CoilNum
        switch (state.dataHeatingCoils->HeatingCoil(CoilNum).coilType) {
        case HVAC::CoilType::HeatingElectric: {
            CalcElectricHeatingCoil(state, CoilNum, QCoilRequired, QCoilActual2, fanOp, PartLoadFrac);
        } break;
        case HVAC::CoilType::HeatingElectricMultiStage: {
            CalcMultiStageElectricHeatingCoil(
                state,
                CoilNum,
                SpeedRatio,
                PartLoadRatio,
                StageNum,
                fanOp,
                QCoilActual2,
                state.dataHeatingCoils->CoilIsSuppHeater); // Autodesk:OPTIONAL SpeedRatio, PartLoadRatio, StageNum used without PRESENT check
        } break;
        case HVAC::CoilType::HeatingGasOrOtherFuel: {
            CalcFuelHeatingCoil(state, CoilNum, QCoilRequired, QCoilActual2, fanOp, PartLoadFrac);
        } break;
        case HVAC::CoilType::HeatingGasMultiStage: {
            CalcMultiStageGasHeatingCoil(state,
                                         CoilNum,
                                         SpeedRatio,
                                         PartLoadRatio,
                                         StageNum,
                                         fanOp); // Autodesk:OPTIONAL SpeedRatio, PartLoadRatio, StageNum used without PRESENT check
        } break;
        case HVAC::CoilType::HeatingDesuperheater: {
            CalcDesuperheaterHeatingCoil(state, CoilNum, QCoilRequired, QCoilActual2);
        } break;
        default:
            QCoilActual2 = 0.0;
            break;
        }

        // Update the current HeatingCoil to the outlet nodes
        UpdateHeatingCoil(state, CoilNum);

        // Report the current HeatingCoil
        ReportHeatingCoil(state, CoilNum, state.dataHeatingCoils->CoilIsSuppHeater);

        if (present(QCoilActual)) {
            QCoilActual = QCoilActual2;
        }
    }

    void GetHeatingCoilInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for coils and stores it in coil data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName = "GetHeatingCoilInput: "; // include trailing blank space
        static constexpr std::string_view routineName = "GetHeatingCoilInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string Alphas;           // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> Numbers;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        int NumAlphas;
        int NumNums;
        int IOStat;
        int StageNum;
        bool DXCoilErrFlag; // Used in GetDXCoil mining functions
        bool errFlag;

        state.dataHeatingCoils->NumElecCoil = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Electric");
        state.dataHeatingCoils->NumElecCoilMultiStage =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Electric:MultiStage");
        state.dataHeatingCoils->NumFuelCoil = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Fuel");
        state.dataHeatingCoils->NumGasCoilMultiStage =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Gas:MultiStage");
        state.dataHeatingCoils->NumDesuperheaterCoil =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Desuperheater");
        state.dataHeatingCoils->NumHeatingCoils = state.dataHeatingCoils->NumElecCoil + state.dataHeatingCoils->NumElecCoilMultiStage +
                                                  state.dataHeatingCoils->NumFuelCoil + state.dataHeatingCoils->NumGasCoilMultiStage +
                                                  state.dataHeatingCoils->NumDesuperheaterCoil;
        if (state.dataHeatingCoils->NumHeatingCoils > 0) {
            state.dataHeatingCoils->HeatingCoil.allocate(state.dataHeatingCoils->NumHeatingCoils);
            state.dataHeatingCoils->HeatingCoilNumericFields.allocate(state.dataHeatingCoils->NumHeatingCoils);
            state.dataHeatingCoils->ValidSourceType.dimension(state.dataHeatingCoils->NumHeatingCoils, false);
            state.dataHeatingCoils->CheckEquipName.dimension(state.dataHeatingCoils->NumHeatingCoils, true);
        }

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "Coil:Heating:Electric", state.dataHeatingCoils->TotalArgs, NumAlphas, NumNums);
        state.dataHeatingCoils->MaxNums = max(state.dataHeatingCoils->MaxNums, NumNums);
        state.dataHeatingCoils->MaxAlphas = max(state.dataHeatingCoils->MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "Coil:Heating:Electric:MultiStage", state.dataHeatingCoils->TotalArgs, NumAlphas, NumNums);
        state.dataHeatingCoils->MaxNums = max(state.dataHeatingCoils->MaxNums, NumNums);
        state.dataHeatingCoils->MaxAlphas = max(state.dataHeatingCoils->MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "Coil:Heating:Fuel", state.dataHeatingCoils->TotalArgs, NumAlphas, NumNums);
        state.dataHeatingCoils->MaxNums = max(state.dataHeatingCoils->MaxNums, NumNums);
        state.dataHeatingCoils->MaxAlphas = max(state.dataHeatingCoils->MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "Coil:Heating:Gas:MultiStage", state.dataHeatingCoils->TotalArgs, NumAlphas, NumNums);
        state.dataHeatingCoils->MaxNums = max(state.dataHeatingCoils->MaxNums, NumNums);
        state.dataHeatingCoils->MaxAlphas = max(state.dataHeatingCoils->MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "Coil:Heating:Desuperheater", state.dataHeatingCoils->TotalArgs, NumAlphas, NumNums);
        state.dataHeatingCoils->MaxNums = max(state.dataHeatingCoils->MaxNums, NumNums);
        state.dataHeatingCoils->MaxAlphas = max(state.dataHeatingCoils->MaxAlphas, NumAlphas);

        Alphas.allocate(state.dataHeatingCoils->MaxAlphas);
        cAlphaFields.allocate(state.dataHeatingCoils->MaxAlphas);
        cNumericFields.allocate(state.dataHeatingCoils->MaxNums);
        Numbers.dimension(state.dataHeatingCoils->MaxNums, 0.0);
        lAlphaBlanks.dimension(state.dataHeatingCoils->MaxAlphas, true);
        lNumericBlanks.dimension(state.dataHeatingCoils->MaxNums, true);

        // Get the data for electric heating coils
        for (int ElecCoilNum = 1; ElecCoilNum <= state.dataHeatingCoils->NumElecCoil; ++ElecCoilNum) {

            auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(ElecCoilNum);
            auto &heatingCoilNumericFields = state.dataHeatingCoils->HeatingCoilNumericFields(ElecCoilNum);

            CurrentModuleObject = "Coil:Heating:Electric";
            heatingCoil.FuelType = Constant::eFuel::Electricity;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     ElecCoilNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};
            heatingCoilNumericFields.FieldNames.allocate(state.dataHeatingCoils->MaxNums);
            heatingCoilNumericFields.FieldNames = cNumericFields;

            // InputErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueCoilName(
                state, CurrentModuleObject, Alphas(1), state.dataHeatingCoils->InputErrorsFound, CurrentModuleObject + " Name");

            heatingCoil.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                heatingCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((heatingCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.HeatingCoilType = "Heating";
            heatingCoil.HeatingCoilModel = "Electric";
            heatingCoil.coilType = HVAC::CoilType::HeatingElectric;

            heatingCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, heatingCoil.Name, heatingCoil.coilType);

            heatingCoil.Efficiency = Numbers(1);
            heatingCoil.NominalCapacity = Numbers(2);
            errFlag = false;
            heatingCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                            Alphas(3),
                                                            errFlag,
                                                            Node::ConnectionObjectType::CoilHeatingElectric,
                                                            Alphas(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;
            errFlag = false;
            heatingCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                             Alphas(4),
                                                             errFlag,
                                                             Node::ConnectionObjectType::CoilHeatingElectric,
                                                             Alphas(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            Node::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            errFlag = false;
            heatingCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                Alphas(5),
                                                                errFlag,
                                                                Node::ConnectionObjectType::CoilHeatingElectric,
                                                                Alphas(1),
                                                                Node::FluidType::Air,
                                                                Node::ConnectionType::Sensor,
                                                                Node::CompFluidStream::Primary,
                                                                Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            // Setup Report variables for the Electric Coils
            // CurrentModuleObject = "Coil:Heating:Electric"
            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                heatingCoil.HeatingCoilLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                heatingCoil.HeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Energy",
                                Constant::Units::J,
                                heatingCoil.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Rate",
                                Constant::Units::W,
                                heatingCoil.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
        }

        // Get the data for electric heating coils
        for (int ElecCoilNum = 1; ElecCoilNum <= state.dataHeatingCoils->NumElecCoilMultiStage; ++ElecCoilNum) {

            int CoilNum = state.dataHeatingCoils->NumElecCoil + ElecCoilNum;
            auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
            auto &heatingCoilNumericFields = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum);

            CurrentModuleObject = "Coil:Heating:Electric:MultiStage";
            heatingCoil.FuelType = Constant::eFuel::Electricity;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     ElecCoilNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};
            heatingCoilNumericFields.FieldNames.allocate(state.dataHeatingCoils->MaxNums);
            heatingCoilNumericFields.FieldNames = cNumericFields;

            // InputErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueCoilName(
                state, CurrentModuleObject, Alphas(1), state.dataHeatingCoils->InputErrorsFound, CurrentModuleObject + " Name");
            heatingCoil.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                heatingCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((heatingCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.HeatingCoilType = "Heating";
            heatingCoil.HeatingCoilModel = "Electric:MultiStage";
            heatingCoil.coilType = HVAC::CoilType::HeatingElectricMultiStage;

            heatingCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, heatingCoil.Name, heatingCoil.coilType);

            heatingCoil.NumOfStages = static_cast<int>(Numbers(1));

            heatingCoil.MSEfficiency.allocate(heatingCoil.NumOfStages);
            heatingCoil.MSNominalCapacity.allocate(heatingCoil.NumOfStages);

            for (StageNum = 1; StageNum <= heatingCoil.NumOfStages; ++StageNum) {

                heatingCoil.MSEfficiency(StageNum) = Numbers(StageNum * 2);
                heatingCoil.MSNominalCapacity(StageNum) = Numbers(StageNum * 2 + 1);
            }

            errFlag = false;
            heatingCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                            Alphas(3),
                                                            errFlag,
                                                            Node::ConnectionObjectType::CoilHeatingElectricMultiStage,
                                                            Alphas(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;
            errFlag = false;
            heatingCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                             Alphas(4),
                                                             errFlag,
                                                             Node::ConnectionObjectType::CoilHeatingElectricMultiStage,
                                                             Alphas(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            Node::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            errFlag = false;
            heatingCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                Alphas(5),
                                                                errFlag,
                                                                Node::ConnectionObjectType::CoilHeatingElectricMultiStage,
                                                                Alphas(1),
                                                                Node::FluidType::Air,
                                                                Node::ConnectionType::Sensor,
                                                                Node::CompFluidStream::Primary,
                                                                Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            // Setup Report variables for the Electric Coils
            // CurrentModuleObject = "Coil:Heating:Electric:MultiStage"
            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                heatingCoil.HeatingCoilLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                heatingCoil.HeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Energy",
                                Constant::Units::J,
                                heatingCoil.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Rate",
                                Constant::Units::W,
                                heatingCoil.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
        }

        // Get the data for for fuel heating coils
        for (int FuelCoilNum = 1; FuelCoilNum <= state.dataHeatingCoils->NumFuelCoil; ++FuelCoilNum) {

            int CoilNum = state.dataHeatingCoils->NumElecCoil + state.dataHeatingCoils->NumElecCoilMultiStage + FuelCoilNum;
            auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
            auto &heatingCoilNumericFields = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum);

            CurrentModuleObject = "Coil:Heating:Fuel";

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     FuelCoilNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};
            heatingCoilNumericFields.FieldNames.allocate(state.dataHeatingCoils->MaxNums);
            heatingCoilNumericFields.FieldNames = cNumericFields;

            // InputErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueCoilName(
                state, CurrentModuleObject, Alphas(1), state.dataHeatingCoils->InputErrorsFound, CurrentModuleObject + " Name");
            heatingCoil.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                heatingCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((heatingCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.HeatingCoilType = "Heating";
            heatingCoil.HeatingCoilModel = "Fuel";
            heatingCoil.coilType = HVAC::CoilType::HeatingGasOrOtherFuel;

            heatingCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, heatingCoil.Name, heatingCoil.coilType);

            heatingCoil.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, Alphas(3)));
            if (!(heatingCoil.FuelType == Constant::eFuel::NaturalGas || heatingCoil.FuelType == Constant::eFuel::Propane ||
                  heatingCoil.FuelType == Constant::eFuel::Diesel || heatingCoil.FuelType == Constant::eFuel::Gasoline ||
                  heatingCoil.FuelType == Constant::eFuel::FuelOilNo1 || heatingCoil.FuelType == Constant::eFuel::FuelOilNo2 ||
                  heatingCoil.FuelType == Constant::eFuel::OtherFuel1 || heatingCoil.FuelType == Constant::eFuel::OtherFuel2 ||
                  heatingCoil.FuelType == Constant::eFuel::Coal)) {
                ShowSevereError(state,
                                std::format("{}{}: Invalid {} entered ={} for {}={}",
                                            RoutineName,
                                            CurrentModuleObject,
                                            cAlphaFields(3),
                                            Alphas(3),
                                            cAlphaFields(1),
                                            Alphas(1)));
                state.dataHeatingCoils->InputErrorsFound = true;
            }
            std::string const sFuelType(Constant::eFuelNames[static_cast<int>(heatingCoil.FuelType)]);

            heatingCoil.Efficiency = Numbers(1);
            heatingCoil.NominalCapacity = Numbers(2);
            errFlag = false;
            heatingCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                            Alphas(4),
                                                            errFlag,
                                                            Node::ConnectionObjectType::CoilHeatingFuel,
                                                            Alphas(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;
            errFlag = false;
            heatingCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                             Alphas(5),
                                                             errFlag,
                                                             Node::ConnectionObjectType::CoilHeatingFuel,
                                                             Alphas(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            Node::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(4), Alphas(5), "Air Nodes");

            errFlag = false;
            heatingCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                Alphas(6),
                                                                errFlag,
                                                                Node::ConnectionObjectType::CoilHeatingFuel,
                                                                Alphas(1),
                                                                Node::FluidType::Air,
                                                                Node::ConnectionType::Sensor,
                                                                Node::CompFluidStream::Primary,
                                                                Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            // parasitic electric load associated with the fuel heating coil
            heatingCoil.ParasiticElecLoad = Numbers(3);

            heatingCoil.PLFCurveIndex = Curve::GetCurveIndex(state, Alphas(7)); // convert curve name to number

            // parasitic fuel load associated with the gas heating coil (standing pilot light)
            heatingCoil.ParasiticFuelCapacity = Numbers(4);

            // Setup Report variables for the Fuel Coils
            // CurrentModuleObject = "Coil:Heating:OtherFuel"

            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                heatingCoil.HeatingCoilLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                heatingCoil.HeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                std::format("Heating Coil {} Energy", sFuelType),
                                Constant::Units::J,
                                heatingCoil.FuelUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eFuel2eResource[(int)heatingCoil.FuelType],
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                std::format("Heating Coil {} Rate", sFuelType),
                                Constant::Units::W,
                                heatingCoil.FuelUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Energy",
                                Constant::Units::J,
                                heatingCoil.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Rate",
                                Constant::Units::W,
                                heatingCoil.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Runtime Fraction",
                                Constant::Units::None,
                                heatingCoil.RTF,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Ancillary " + sFuelType + " Rate",
                                Constant::Units::W,
                                heatingCoil.ParasiticFuelRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Ancillary " + sFuelType + " Energy",
                                Constant::Units::J,
                                heatingCoil.ParasiticFuelConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eFuel2eResource[(int)heatingCoil.FuelType],
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
        }

        // Get the data for for gas multistage heating coils
        for (int FuelCoilNum = 1; FuelCoilNum <= state.dataHeatingCoils->NumGasCoilMultiStage; ++FuelCoilNum) {

            int CoilNum = state.dataHeatingCoils->NumElecCoil + state.dataHeatingCoils->NumElecCoilMultiStage + state.dataHeatingCoils->NumFuelCoil +
                          FuelCoilNum;
            auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
            auto &heatingCoilNumericFields = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum);
            CurrentModuleObject = "Coil:Heating:Gas:MultiStage";
            heatingCoil.FuelType = Constant::eFuel::NaturalGas;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     FuelCoilNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            heatingCoilNumericFields.FieldNames.allocate(state.dataHeatingCoils->MaxNums);
            heatingCoilNumericFields.FieldNames = cNumericFields;

            // InputErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueCoilName(
                state, CurrentModuleObject, Alphas(1), state.dataHeatingCoils->InputErrorsFound, CurrentModuleObject + " Name");
            heatingCoil.Name = Alphas(1);

            if (lAlphaBlanks(2)) {
                heatingCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((heatingCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.HeatingCoilType = "Heating";
            heatingCoil.HeatingCoilModel = "Gas:MultiStage";
            heatingCoil.coilType = HVAC::CoilType::HeatingGasMultiStage;

            heatingCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, heatingCoil.Name, heatingCoil.coilType);

            heatingCoil.ParasiticFuelCapacity = Numbers(1);

            heatingCoil.NumOfStages = static_cast<int>(Numbers(2));

            heatingCoil.MSEfficiency.allocate(heatingCoil.NumOfStages);
            heatingCoil.MSNominalCapacity.allocate(heatingCoil.NumOfStages);
            heatingCoil.MSParasiticElecLoad.allocate(heatingCoil.NumOfStages);

            for (StageNum = 1; StageNum <= heatingCoil.NumOfStages; ++StageNum) {

                heatingCoil.MSEfficiency(StageNum) = Numbers(StageNum * 3);
                heatingCoil.MSNominalCapacity(StageNum) = Numbers(StageNum * 3 + 1);
                heatingCoil.MSParasiticElecLoad(StageNum) = Numbers(StageNum * 3 + 2);
            }

            errFlag = false;
            heatingCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                            Alphas(3),
                                                            errFlag,
                                                            Node::ConnectionObjectType::CoilHeatingGasMultiStage,
                                                            Alphas(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;
            errFlag = false;
            heatingCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                             Alphas(4),
                                                             errFlag,
                                                             Node::ConnectionObjectType::CoilHeatingGasMultiStage,
                                                             Alphas(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            Node::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            errFlag = false;
            heatingCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                Alphas(5),
                                                                errFlag,
                                                                Node::ConnectionObjectType::CoilHeatingGasMultiStage,
                                                                Alphas(1),
                                                                Node::FluidType::Air,
                                                                Node::ConnectionType::Sensor,
                                                                Node::CompFluidStream::Primary,
                                                                Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            // parasitic electric load associated with the gas heating coil
            heatingCoil.ParasiticElecLoad = Numbers(10);

            heatingCoil.PLFCurveIndex = Curve::GetCurveIndex(state, Alphas(6)); // convert curve name to number

            // parasitic gas load associated with the gas heating coil (standing pilot light)

            // Setup Report variables for the Gas Coils
            // CurrentModuleObject = "Coil:Heating:Gas:MultiStage"
            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                heatingCoil.HeatingCoilLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                heatingCoil.HeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil NaturalGas Energy",
                                Constant::Units::J,
                                heatingCoil.FuelUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::NaturalGas,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil NaturalGas Rate",
                                Constant::Units::W,
                                heatingCoil.FuelUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Energy",
                                Constant::Units::J,
                                heatingCoil.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Rate",
                                Constant::Units::W,
                                heatingCoil.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Runtime Fraction",
                                Constant::Units::None,
                                heatingCoil.RTF,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Ancillary NaturalGas Rate",
                                Constant::Units::W,
                                heatingCoil.ParasiticFuelRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Ancillary NaturalGas Energy",
                                Constant::Units::J,
                                heatingCoil.ParasiticFuelConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::NaturalGas,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
        }

        // Get the data for for desuperheater heating coils
        for (int DesuperheaterCoilNum = 1; DesuperheaterCoilNum <= state.dataHeatingCoils->NumDesuperheaterCoil; ++DesuperheaterCoilNum) {

            int CoilNum = state.dataHeatingCoils->NumElecCoil + state.dataHeatingCoils->NumElecCoilMultiStage + state.dataHeatingCoils->NumFuelCoil +
                          state.dataHeatingCoils->NumGasCoilMultiStage + DesuperheaterCoilNum;
            auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
            auto &heatingCoilNumericFields = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum);
            CurrentModuleObject = "Coil:Heating:Desuperheater";
            heatingCoil.FuelType = Constant::eFuel::Electricity;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     DesuperheaterCoilNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};
            heatingCoilNumericFields.FieldNames.allocate(state.dataHeatingCoils->MaxNums);
            heatingCoilNumericFields.FieldNames = cNumericFields;

            // InputErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueCoilName(
                state, CurrentModuleObject, Alphas(1), state.dataHeatingCoils->InputErrorsFound, CurrentModuleObject + " Name");
            heatingCoil.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                heatingCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((heatingCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                state.dataHeatingCoils->InputErrorsFound = true;
            } else if (!heatingCoil.availSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(2), Alphas(2), Clusive::In, 0.0, Clusive::In, 1.0);
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.HeatingCoilType = "Heating";
            heatingCoil.HeatingCoilModel = "Desuperheater";
            heatingCoil.coilType = HVAC::CoilType::HeatingDesuperheater;

            heatingCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, heatingCoil.Name, heatingCoil.coilType);

            // HeatingCoil(CoilNum)%Efficiency       = Numbers(1)
            //(Numbers(1)) error limits checked and defaults applied on efficiency after
            //       identifying source type.

            errFlag = false;
            heatingCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                            Alphas(3),
                                                            errFlag,
                                                            Node::ConnectionObjectType::CoilHeatingDesuperheater,
                                                            Alphas(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;
            errFlag = false;
            heatingCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                             Alphas(4),
                                                             errFlag,
                                                             Node::ConnectionObjectType::CoilHeatingDesuperheater,
                                                             Alphas(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            Node::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            if ((Util::SameString(Alphas(5), "Refrigeration:Condenser:AirCooled")) ||
                (Util::SameString(Alphas(5), "Refrigeration:Condenser:EvaporativeCooled")) ||
                (Util::SameString(Alphas(5), "Refrigeration:Condenser:WaterCooled"))) {
                if (lNumericBlanks(1)) {
                    heatingCoil.Efficiency = 0.8;
                } else {
                    heatingCoil.Efficiency = Numbers(1);
                    if (Numbers(1) < 0.0 || Numbers(1) > 0.9) {
                        ShowSevereError(
                            state,
                            std::format("{}, \"{}\" heat reclaim recovery efficiency must be >= 0 and <=0.9", CurrentModuleObject, heatingCoil.Name));
                        state.dataHeatingCoils->InputErrorsFound = true;
                    }
                }
            } else {
                if (lNumericBlanks(1)) {
                    heatingCoil.Efficiency = 0.25;
                } else {
                    heatingCoil.Efficiency = Numbers(1);
                    if (Numbers(1) < 0.0 || Numbers(1) > 0.3) {
                        ShowSevereError(
                            state,
                            std::format("{}, \"{}\" heat reclaim recovery efficiency must be >= 0 and <=0.3", CurrentModuleObject, heatingCoil.Name));
                        state.dataHeatingCoils->InputErrorsFound = true;
                    }
                }
            }

            // Find the DX equipment index associated with the desuperheater heating coil.
            // The CoilNum may not be found here when zone heating equip. exists. Check again in InitHeatingCoil.
            // (when zone equipment heating coils are included in the input, the air loop DX equipment has not yet been read in)
            if (Util::SameString(Alphas(5), "Refrigeration:CompressorRack")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COMPRESSORRACK_REFRIGERATEDCASE;
                // Refrigeration equipment hasn't been loaded yet, so handle this in InitHeatingCoil
            } else if ((Util::SameString(Alphas(5), "Refrigeration:Condenser:AirCooled")) ||
                       (Util::SameString(Alphas(5), "Refrigeration:Condenser:EvaporativeCooled")) ||
                       (Util::SameString(Alphas(5), "Refrigeration:Condenser:WaterCooled"))) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::CONDENSER_REFRIGERATION;
                // Refrigeration equipment hasn't been loaded yet, so handle this in InitHeatingCoil
            } else if (Util::SameString(Alphas(5), "Coil:Cooling:DX:SingleSpeed")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COIL_DX_COOLING;
                DXCoils::GetDXCoilIndex(state, Alphas(6), heatingCoil.ReclaimHeatingSourceIndexNum, DXCoilErrFlag, Alphas(5));
                if (heatingCoil.ReclaimHeatingSourceIndexNum > 0) {
                    if (allocated(state.dataHeatBal->HeatReclaimDXCoil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimDXCoil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                        }
                        HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                        if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                            ShowSevereError(
                                state,
                                std::format(
                                    "{}, \"{}\" sum of heat reclaim recovery efficiencies from the same source coil: \"{} \" cannot be over 0.3",
                                    HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                    heatingCoil.Name,
                                    heatingCoil.ReclaimHeatingCoilName));
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                }
                if (heatingCoil.ReclaimHeatingSourceIndexNum > 0) {
                    state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                }
            } else if (Util::SameString(Alphas(5), "Coil:Cooling:DX:VariableSpeed")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COIL_DX_VARIABLE_COOLING;
                heatingCoil.ReclaimHeatingSourceIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(state, Alphas(5), Alphas(6), DXCoilErrFlag);
                if (heatingCoil.ReclaimHeatingSourceIndexNum > 0) {
                    if (allocated(state.dataHeatBal->HeatReclaimVS_Coil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimVS_Coil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                        }
                        HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                        if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                            ShowSevereError(
                                state,
                                std::format(
                                    "{}, \"{}\" sum of heat reclaim recovery efficiencies from the same source coil: \"{} \" cannot be over 0.3",
                                    HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                    heatingCoil.Name,
                                    heatingCoil.ReclaimHeatingCoilName));
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                }
            } else if (Util::SameString(Alphas(5), "Coil:Cooling:DX:TwoSpeed")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COIL_DX_MULTISPEED;
                DXCoils::GetDXCoilIndex(state, Alphas(6), heatingCoil.ReclaimHeatingSourceIndexNum, DXCoilErrFlag, Alphas(5));
                if (heatingCoil.ReclaimHeatingSourceIndexNum > 0) {
                    if (allocated(state.dataHeatBal->HeatReclaimDXCoil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimDXCoil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                        }
                        HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                        if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                            ShowSevereError(
                                state,
                                std::format(
                                    "{}, \"{}\" sum of heat reclaim recovery efficiencies from the same source coil: \"{} \" cannot be over 0.3",
                                    HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                    heatingCoil.Name,
                                    heatingCoil.ReclaimHeatingCoilName));
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                }
            } else if (Util::SameString(Alphas(5), "Coil:Cooling:DX:TwoStageWithHumidityControlMode")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COIL_DX_MULTIMODE;
                DXCoils::GetDXCoilIndex(state, Alphas(6), heatingCoil.ReclaimHeatingSourceIndexNum, DXCoilErrFlag, Alphas(5));
                if (heatingCoil.ReclaimHeatingSourceIndexNum > 0) {
                    if (allocated(state.dataHeatBal->HeatReclaimDXCoil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimDXCoil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                        }
                        HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                        if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                            ShowSevereError(
                                state,
                                std::format(
                                    R"({}, "{}" sum of heat reclaim recovery efficiencies from the same source coil: "{} " cannot be over 0.3)",
                                    HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                    heatingCoil.Name,
                                    heatingCoil.ReclaimHeatingCoilName));
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                }
            } else if (Util::SameString(Alphas(5), "Coil:Cooling:DX")) {
                heatingCoil.ReclaimHeatingSource = HeatObjTypes::COIL_COOLING_DX_NEW;
                heatingCoil.ReclaimHeatingSourceIndexNum = CoilCoolingDX::factory(state, Alphas(6));
                if (heatingCoil.ReclaimHeatingSourceIndexNum < 0) {
                    ShowSevereError(
                        state,
                        std::format("{}={}, could not find desuperheater coil {}={}", CurrentModuleObject, heatingCoil.Name, Alphas(5), Alphas(6)));
                    state.dataHeatingCoils->InputErrorsFound = true;
                }
                DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                    state.dataCoilCoolingDX->coilCoolingDXs[heatingCoil.ReclaimHeatingSourceIndexNum].reclaimHeat;
                if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                    HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                    std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                }
                HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                    ShowSevereError(
                        state,
                        std::format("{}, \"{}\" sum of heat reclaim recovery efficiencies from the same source coil: \"{}\" cannot be over 0.3",
                                    HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                    heatingCoil.Name,
                                    heatingCoil.ReclaimHeatingCoilName));
                }
                state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
            } else {
                ShowSevereError(
                    state,
                    std::format(
                        "{}, \"{}\" valid desuperheater heat source object type not found: {}", CurrentModuleObject, heatingCoil.Name, Alphas(5)));
                ShowContinueError(state, "Valid desuperheater heat source objects are:");
                ShowContinueError(state,
                                  "Refrigeration:CompressorRack, Coil:Cooling:DX:SingleSpeed, Refrigeration:Condenser:AirCooled, "
                                  "Refrigeration:Condenser:EvaporativeCooled, Refrigeration:Condenser:WaterCooled,Coil:Cooling:DX:TwoSpeed, and "
                                  "Coil:Cooling:DX:TwoStageWithHumidityControlMode");
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            heatingCoil.ReclaimHeatingCoilName = Alphas(6);

            errFlag = false;
            heatingCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                Alphas(7),
                                                                errFlag,
                                                                Node::ConnectionObjectType::CoilHeatingDesuperheater,
                                                                Alphas(1),
                                                                Node::FluidType::Air,
                                                                Node::ConnectionType::Sensor,
                                                                Node::CompFluidStream::Primary,
                                                                Node::ObjectIsNotParent);
            state.dataHeatingCoils->InputErrorsFound = errFlag || state.dataHeatingCoils->InputErrorsFound;

            // parasitic electric load associated with the desuperheater heating coil
            heatingCoil.ParasiticElecLoad = Numbers(2);

            if (Numbers(2) < 0.0) {
                ShowSevereError(state, std::format("{}, \"{}\" parasitic electric load must be >= 0", CurrentModuleObject, heatingCoil.Name));
                state.dataHeatingCoils->InputErrorsFound = true;
            }

            // Setup Report variables for the Desuperheater Heating Coils
            // CurrentModuleObject = "Coil:Heating:Desuperheater"
            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                heatingCoil.HeatingCoilLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                heatingCoil.HeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Energy",
                                Constant::Units::J,
                                heatingCoil.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                heatingCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Heating Coil Electricity Rate",
                                Constant::Units::W,
                                heatingCoil.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Runtime Fraction",
                                Constant::Units::None,
                                heatingCoil.RTF,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                heatingCoil.Name);
        }

        if (state.dataHeatingCoils->InputErrorsFound) {
            ShowFatalError(state, std::format("{}Errors found in input.  Program terminates.", RoutineName));
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    void InitHeatingCoil(EnergyPlusData &state, int const CoilNum, bool const FirstHVACIteration, Real64 const QCoilRequired)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   May 2000
        //       MODIFIED       B. Griffith, May 2009 added EMS setpoint check

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the HeatingCoil Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        if (state.dataHeatingCoils->MyOneTimeFlag) {
            // initialize the environment and sizing flags
            Real64 numHeatingCoils = state.dataHeatingCoils->NumHeatingCoils;
            state.dataHeatingCoils->MyEnvrnFlag.allocate(numHeatingCoils);
            state.dataHeatingCoils->MySizeFlag.allocate(numHeatingCoils);
            state.dataHeatingCoils->ShowSingleWarning.allocate(numHeatingCoils);
            state.dataHeatingCoils->MySPTestFlag.allocate(numHeatingCoils);
            state.dataHeatingCoils->MyEnvrnFlag = true;
            state.dataHeatingCoils->MySizeFlag = true;
            state.dataHeatingCoils->ShowSingleWarning = true;
            state.dataHeatingCoils->MyOneTimeFlag = false;
            state.dataHeatingCoils->MySPTestFlag = true;
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHeatingCoils->MySizeFlag(CoilNum)) {
            // for each coil, do the sizing once.
            SizeHeatingCoil(state, CoilNum);

            state.dataHeatingCoils->MySizeFlag(CoilNum) = false;
        }

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.
        // First set the conditions for the air into the coil model
        int AirOutletNodeNum = heatingCoil.AirOutletNodeNum;
        int ControlNodeNum = heatingCoil.TempSetPointNodeNum;
        auto const &airInletNode = state.dataLoopNodes->Node(heatingCoil.AirInletNodeNum);
        auto const &airOutletNode = state.dataLoopNodes->Node(AirOutletNodeNum);
        heatingCoil.InletAirMassFlowRate = airInletNode.MassFlowRate;
        heatingCoil.InletAirTemp = airInletNode.Temp;
        heatingCoil.InletAirHumRat = airInletNode.HumRat;
        heatingCoil.InletAirEnthalpy = airInletNode.Enthalpy;

        // Set the reporting variables to zero at each timestep.
        heatingCoil.HeatingCoilLoad = 0.0;
        heatingCoil.FuelUseLoad = 0.0;
        heatingCoil.ElecUseLoad = 0.0;
        heatingCoil.RTF = 0.0;

        // If a temperature setpoint controlled coil must set the desired outlet temp every time
        if (ControlNodeNum == 0) {
            heatingCoil.DesiredOutletTemp = 0.0;
        } else {
            auto const &controlNode = state.dataLoopNodes->Node(ControlNodeNum);
            heatingCoil.DesiredOutletTemp =
                controlNode.TempSetPoint - ((ControlNodeNum == AirOutletNodeNum) ? 0 : (controlNode.Temp - airOutletNode.Temp));
        }

        if (QCoilRequired == Node::SensedLoadFlagValue && state.dataHeatingCoils->MySPTestFlag(CoilNum) &&
            heatingCoil.coilType != HVAC::CoilType::HeatingElectricMultiStage && heatingCoil.coilType != HVAC::CoilType::HeatingGasMultiStage) {

            //   If the coil is temperature controlled (QCoilReq == -999.0), both a control node and setpoint are required.
            if (!state.dataGlobal->SysSizingCalc && state.dataHVACGlobal->DoSetPointTest) {
                //     3 possibilities here:
                //     1) TempSetPointNodeNum .GT. 0 and TempSetPoint /= SensedNodeFlagValue, this is correct
                //     2) TempSetPointNodeNum .EQ. 0, this is not correct, control node is required
                //     3) TempSetPointNodeNum .GT. 0 and TempSetPoint == SensedNodeFlagValue, this is not correct, missing temperature setpoint
                //     test 2) here (fatal message)
                if (ControlNodeNum == 0) {
                    ShowSevereError(state, std::format("{} \"{}\"", HVAC::coilTypeNames[(int)heatingCoil.coilType], heatingCoil.Name));
                    ShowContinueError(state, "... Missing control node for heating coil.");
                    ShowContinueError(state, "... enter a control node name in the coil temperature setpoint node field for this heating coil.");
                    ShowContinueError(state, "... use a Setpoint Manager to establish a setpoint at the coil temperature setpoint node.");
                    state.dataHeatingCoils->HeatingCoilFatalError = true;
                    //     test 3) here (fatal message)
                } else { // IF(ControlNode .GT. 0)THEN
                    auto const &controlNode = state.dataLoopNodes->Node(ControlNodeNum);
                    if (controlNode.TempSetPoint == Node::SensedNodeFlagValue) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state, std::format("{} \"{}\"", HVAC::coilTypeNames[(int)heatingCoil.coilType], heatingCoil.Name));
                            ShowContinueError(state, "... Missing temperature setpoint for heating coil.");
                            ShowContinueError(state, "... use a Setpoint Manager to establish a setpoint at the coil temperature setpoint node.");
                            state.dataHeatingCoils->HeatingCoilFatalError = true;
                        } else {
                            EMSManager::CheckIfNodeSetPointManagedByEMS(
                                state, ControlNodeNum, HVAC::CtrlVarType::Temp, state.dataHeatingCoils->HeatingCoilFatalError);
                            if (state.dataHeatingCoils->HeatingCoilFatalError) {
                                ShowSevereError(state, std::format("{} \"{}\"", HVAC::coilTypeNames[(int)heatingCoil.coilType], heatingCoil.Name));
                                ShowContinueError(state, "... Missing temperature setpoint for heating coil.");
                                ShowContinueError(state, "... use a Setpoint Manager to establish a setpoint at the coil temperature setpoint node.");
                                ShowContinueError(state, "... or use an EMS Actuator to establish a setpoint at the coil temperature setpoint node.");
                            }
                        }
                    }
                }
                state.dataHeatingCoils->MySPTestFlag(CoilNum) = false;
            }
        } else if (state.dataHeatingCoils->MySPTestFlag(CoilNum)) {
            //  If QCoilReq /= SensedLoadFlagValue, the coil is load controlled and does not require a control node
            //   4 possibilities here:
            //   1) TempSetPointNodeNum .EQ. 0 and TempSetPoint == SensedNodeFlagValue, this is correct
            //   2) TempSetPointNodeNum .EQ. 0 and TempSetPoint /= SensedNodeFlagValue, this may be correct,
            //      (if no control node specified and SP on heating coil outlet do not show warning, other SP managers may be using SP)
            //   3) TempSetPointNodeNum .GT. 0 and TempSetPoint == SensedNodeFlagValue, control node not required if load based control
            //   4) TempSetPointNodeNum .GT. 0 and TempSetPoint /= SensedNodeFlagValue, control node not required if load based control
            //   test 3) and 4) here (warning only)
            if (ControlNodeNum > 0) {
                ShowWarningError(state, std::format("{} \"{}\"", HVAC::coilTypeNames[(int)heatingCoil.coilType], heatingCoil.Name));
                ShowContinueError(state, " The \"Temperature Setpoint Node Name\" input is not required for this heating coil.");
                ShowContinueError(state, " Leaving the input field \"Temperature Setpoint Node Name\" blank will eliminate this warning.");
            }
            state.dataHeatingCoils->MySPTestFlag(CoilNum) = false;
        }

        // delay fatal error until all coils are called
        if (!FirstHVACIteration && state.dataHeatingCoils->HeatingCoilFatalError) {
            ShowFatalError(state, "... errors found in heating coil input.");
        }

        // Find the heating source index for the desuperheater heating coil if not already found. This occurs when zone heating
        // equip. exists. (when zone equipment heating coils are included in the input, the air loop DX equipment has not yet been read)
        // Issue a single warning if the coil is not found and continue the simulation
        if (!state.dataHeatingCoils->ValidSourceType(CoilNum) && (heatingCoil.coilType == HVAC::CoilType::HeatingDesuperheater) &&
            state.dataHeatingCoils->ShowSingleWarning(CoilNum)) {
            ++state.dataHeatingCoils->ValidSourceTypeCounter;
            switch (heatingCoil.ReclaimHeatingSource) {
            case HeatObjTypes::COMPRESSORRACK_REFRIGERATEDCASE: {
                for (int RackNum = 1; RackNum <= state.dataRefrigCase->NumRefrigeratedRacks; ++RackNum) {
                    if (!Util::SameString(state.dataHeatBal->HeatReclaimRefrigeratedRack(RackNum).Name, heatingCoil.ReclaimHeatingCoilName)) {
                        continue;
                    }
                    heatingCoil.ReclaimHeatingSourceIndexNum = RackNum;
                    if (allocated(state.dataHeatBal->HeatReclaimRefrigeratedRack)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimRefrigeratedRack(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                            HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                            if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                                ShowSevereError(
                                    state,
                                    std::format(
                                        R"({}, "{}" sum of heat reclaim recovery efficiencies from the same source coil: "{}" cannot be over 0.3)",
                                        HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                        heatingCoil.Name,
                                        heatingCoil.ReclaimHeatingCoilName));
                            }
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                    break;
                }
            } break;
            case HeatObjTypes::CONDENSER_REFRIGERATION: {
                for (int CondNum = 1; CondNum <= state.dataRefrigCase->NumRefrigCondensers; ++CondNum) {
                    if (!Util::SameString(state.dataHeatBal->HeatReclaimRefrigCondenser(CondNum).Name, heatingCoil.ReclaimHeatingCoilName)) {
                        continue;
                    }
                    heatingCoil.ReclaimHeatingSourceIndexNum = CondNum;
                    if (allocated(state.dataHeatBal->HeatReclaimRefrigCondenser)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimRefrigCondenser(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                            HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                            if (HeatReclaim.ReclaimEfficiencyTotal > 0.9) {
                                ShowSevereError(
                                    state,
                                    std::format(
                                        R"({}, "{}" sum of heat reclaim recovery efficiencies from the same source coil: "{}" cannot be over 0.9)",
                                        HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                        heatingCoil.Name,
                                        heatingCoil.ReclaimHeatingCoilName));
                            }
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                    break;
                }
            } break;
            case HeatObjTypes::COIL_DX_COOLING:
            case HeatObjTypes::COIL_DX_MULTISPEED:
            case HeatObjTypes::COIL_DX_MULTIMODE: {
                for (int DXCoilNum = 1; DXCoilNum <= state.dataDXCoils->NumDXCoils; ++DXCoilNum) {
                    if (!Util::SameString(state.dataHeatBal->HeatReclaimDXCoil(DXCoilNum).Name, heatingCoil.ReclaimHeatingCoilName)) {
                        continue;
                    }
                    heatingCoil.ReclaimHeatingSourceIndexNum = DXCoilNum;
                    if (allocated(state.dataHeatBal->HeatReclaimDXCoil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimDXCoil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                            HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                            if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                                ShowSevereError(
                                    state,
                                    std::format(
                                        R"({}, "{}" sum of heat reclaim recovery efficiencies from the same source coil: "{}" cannot be over 0.3)",
                                        HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                        heatingCoil.Name,
                                        heatingCoil.ReclaimHeatingCoilName));
                            }
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                    break;
                }
            } break;
            case HeatObjTypes::COIL_DX_VARIABLE_COOLING: {
                for (int DXCoilNum = 1; DXCoilNum <= state.dataVariableSpeedCoils->NumVarSpeedCoils; ++DXCoilNum) {
                    if (!Util::SameString(state.dataHeatBal->HeatReclaimVS_Coil(DXCoilNum).Name, heatingCoil.ReclaimHeatingCoilName)) {
                        continue;
                    }
                    heatingCoil.ReclaimHeatingSourceIndexNum = DXCoilNum;
                    if (allocated(state.dataHeatBal->HeatReclaimVS_Coil)) {
                        DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                            state.dataHeatBal->HeatReclaimVS_Coil(heatingCoil.ReclaimHeatingSourceIndexNum);
                        if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                            HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                            std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                            HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                            if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                                ShowSevereError(
                                    state,
                                    std::format(
                                        R"({}, "{}" sum of heat reclaim recovery efficiencies from the same source coil: "{}" cannot be over 0.3)",
                                        HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                        heatingCoil.Name,
                                        heatingCoil.ReclaimHeatingCoilName));
                            }
                        }
                        state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                    }
                    break;
                }
            case HeatObjTypes::COIL_COOLING_DX_NEW:
                DataHeatBalance::HeatReclaimDataBase &HeatReclaim =
                    state.dataCoilCoolingDX->coilCoolingDXs[heatingCoil.ReclaimHeatingSourceIndexNum].reclaimHeat;
                if (!allocated(HeatReclaim.HVACDesuperheaterReclaimedHeat)) {
                    HeatReclaim.HVACDesuperheaterReclaimedHeat.allocate(state.dataHeatingCoils->NumDesuperheaterCoil);
                    std::fill(HeatReclaim.HVACDesuperheaterReclaimedHeat.begin(), HeatReclaim.HVACDesuperheaterReclaimedHeat.end(), 0.0);
                    HeatReclaim.ReclaimEfficiencyTotal += heatingCoil.Efficiency;
                    if (HeatReclaim.ReclaimEfficiencyTotal > 0.3) {
                        ShowSevereError(
                            state,
                            std::format("{}, \"{}\" sum of heat reclaim recovery efficiencies from the same source coil: \"{}\" cannot be over 0.3",
                                        HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                        heatingCoil.Name,
                                        heatingCoil.ReclaimHeatingCoilName));
                    }
                }
                state.dataHeatingCoils->ValidSourceType(CoilNum) = true;
                break;
            } break;
            default:
                break;
            }
            if ((state.dataHeatingCoils->ValidSourceTypeCounter > state.dataHeatingCoils->NumDesuperheaterCoil * 2) &&
                state.dataHeatingCoils->ShowSingleWarning(CoilNum) && !state.dataHeatingCoils->ValidSourceType(CoilNum)) {
                ShowWarningError(state,
                                 std::format("Coil:Heating:Desuperheater, \"{}\" desuperheater heat source object name not found: {}",
                                             heatingCoil.Name,
                                             heatingCoil.ReclaimHeatingCoilName));
                ShowContinueError(state, " Desuperheater heating coil is not modeled and simulation continues.");
                state.dataHeatingCoils->ShowSingleWarning(CoilNum) = false;
            }
        }
    }

    void SizeHeatingCoil(EnergyPlusData &state, int const CoilNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //       RE-ENGINEERED  Mar 2014 FSEC, moved calculations to common routine in BaseSizer

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Heating Coil Components for which nominal capacities have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains heating capacities from the zone or system sizing arrays or parent object as necessary.
        // heating coil or other routine sets up any required data variables (e.g., DataCoilIsSuppHeater, TermUnitPIU, etc.),
        // sizing variable (e.g., HeatingCoil( CoilNum ).NominalCapacity in this routine since it can be multi-staged and new routine
        // currently only handles single values) and associated string representing that sizing variable.
        // Sizer functions handles the actual sizing and reporting.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeHeatingCoil: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CompName;       // component name
        std::string CompType;       // component type
        std::string SizingString;   // input field sizing description (e.g., Nominal Capacity)
        bool bPRINT = true;         // TRUE if sizing is reported to output (eio)
        Real64 NominalCapacityDes;  // Autosized nominal capacity for reporting
        Real64 NominalCapacityUser; // Hardsized nominal capacity for reporting
        Real64 TempCap;             // autosized capacity of heating coil [W]
        int FieldNum = 2;           // IDD numeric field number where input field description is found
        int NumCoilsSized = 0;      // counter used to deallocate temporary string array after all coils have been sized

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        if (heatingCoil.coilType == HVAC::CoilType::HeatingElectricMultiStage) {
            FieldNum = 1 + (heatingCoil.NumOfStages * 2);
            TempCap = heatingCoil.MSNominalCapacity(heatingCoil.NumOfStages);
        } else if (heatingCoil.coilType == HVAC::CoilType::HeatingGasMultiStage) {
            FieldNum = 1 + (heatingCoil.NumOfStages * 3);
            TempCap = heatingCoil.MSNominalCapacity(heatingCoil.NumOfStages);
        } else if (heatingCoil.coilType == HVAC::CoilType::HeatingDesuperheater) {
            return; // no autosizable inputs for desupterheater
        } else {
            FieldNum = 2;
            TempCap = heatingCoil.NominalCapacity;
        }
        SizingString = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum).FieldNames(FieldNum) + " [W]";
        CompType = "Coil:" + heatingCoil.HeatingCoilType + ':' + heatingCoil.HeatingCoilModel;
        CompName = heatingCoil.Name;
        state.dataSize->DataCoilIsSuppHeater = state.dataHeatingCoils->CoilIsSuppHeater; // set global instead of using optional argument
        state.dataSize->DataCoolCoilCap =
            0.0; // global only used for heat pump heating coils, non-HP heating coils are sized with other global variables

        if (TempCap == DataSizing::AutoSize) {
            if (heatingCoil.DesiccantRegenerationCoil) {
                state.dataSize->DataDesicRegCoil = true;
                bPRINT = false;
                state.dataSize->DataDesicDehumNum = heatingCoil.DesiccantDehumNum;
                HeatingCoilDesAirInletTempSizer sizerHeatingDesInletTemp;
                bool ErrorsFound = false;
                sizerHeatingDesInletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataDesInletAirTemp = sizerHeatingDesInletTemp.size(state, DataSizing::AutoSize, ErrorsFound);

                HeatingCoilDesAirOutletTempSizer sizerHeatingDesOutletTemp;
                ErrorsFound = false;
                sizerHeatingDesOutletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataDesOutletAirTemp = sizerHeatingDesOutletTemp.size(state, DataSizing::AutoSize, ErrorsFound);

                if (state.dataSize->CurOASysNum > 0) {
                    auto &OASysEqSizing(state.dataSize->OASysEqSizing(state.dataSize->CurOASysNum));
                    OASysEqSizing.AirFlow = true;
                    OASysEqSizing.AirVolFlow = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow;
                }
                state.dataSize->DataDesicDehumNum = 0;
                bPRINT = true;
            }
        }
        bool errorsFound = false;
        HeatingCapacitySizer sizerHeatingCapacity;
        sizerHeatingCapacity.overrideSizingString(SizingString);
        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
        TempCap = sizerHeatingCapacity.size(state, TempCap, errorsFound);
        state.dataSize->DataCoilIsSuppHeater = false; // reset global to false so other heating coils are not affected
        state.dataSize->DataDesicRegCoil = false;     // reset global to false so other heating coils are not affected
        state.dataSize->DataDesInletAirTemp = 0.0;    // reset global data to zero so other heating coils are not
        state.dataSize->DataDesOutletAirTemp = 0.0;   // reset global data to zero so other heating coils are not affected

        if (heatingCoil.coilType == HVAC::CoilType::HeatingElectricMultiStage || heatingCoil.coilType == HVAC::CoilType::HeatingGasMultiStage) {
            heatingCoil.MSNominalCapacity(heatingCoil.NumOfStages) = TempCap;
            bool IsAutoSize = false;
            int NumOfStages; // total number of stages of multi-stage heating coil
            if (any_eq(heatingCoil.MSNominalCapacity, DataSizing::AutoSize)) {
                IsAutoSize = true;
            }
            if (IsAutoSize) {
                NumOfStages = heatingCoil.NumOfStages;
                for (int StageNum = NumOfStages - 1; StageNum >= 1; --StageNum) {
                    bool ThisStageAutoSize = false;
                    FieldNum = 1 + StageNum * ((heatingCoil.coilType == HVAC::CoilType::HeatingElectricMultiStage) ? 2 : 3);
                    SizingString = state.dataHeatingCoils->HeatingCoilNumericFields(CoilNum).FieldNames(FieldNum) + " [W]";
                    if (heatingCoil.MSNominalCapacity(StageNum) == DataSizing::AutoSize) {
                        ThisStageAutoSize = true;
                    }
                    NominalCapacityDes = TempCap * StageNum / NumOfStages;
                    if (ThisStageAutoSize) {
                        heatingCoil.MSNominalCapacity(StageNum) = NominalCapacityDes;
                        BaseSizer::reportSizerOutput(state, CompType, CompName, "Design Size " + SizingString, NominalCapacityDes);
                    } else {
                        if (heatingCoil.MSNominalCapacity(StageNum) > 0.0 && NominalCapacityDes > 0.0) {
                            NominalCapacityUser = TempCap * StageNum / NumOfStages; // HeatingCoil( CoilNum ).MSNominalCapacity( StageNum );
                            BaseSizer::reportSizerOutput(state,
                                                         CompType,
                                                         CompName,
                                                         "Design Size " + SizingString,
                                                         NominalCapacityDes,
                                                         "User-Specified " + SizingString,
                                                         NominalCapacityUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(NominalCapacityDes - NominalCapacityUser) / NominalCapacityUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                std::format("SizeHeatingCoil: Potential issue with equipment sizing for {}, {}", CompType, CompName));
                                    ShowContinueError(state, std::format("User-Specified Nominal Capacity of {:#G} [W]", NominalCapacityUser));
                                    ShowContinueError(state,
                                                      std::format("differs from Design Size Nominal Capacity of {:#G} [W]", NominalCapacityDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            } else { // No autosize
                NumOfStages = heatingCoil.NumOfStages;
                for (int StageNum = NumOfStages - 1; StageNum >= 1; --StageNum) {
                    if (heatingCoil.MSNominalCapacity(StageNum) > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, CompType, CompName, "User-Specified " + SizingString, heatingCoil.MSNominalCapacity(StageNum));
                    }
                }
            }
            // Ensure capacity at lower Stage must be lower or equal to the capacity at higher Stage.
            for (int StageNum = 1; StageNum <= heatingCoil.NumOfStages - 1; ++StageNum) {
                if (heatingCoil.MSNominalCapacity(StageNum) > heatingCoil.MSNominalCapacity(StageNum + 1)) {
                    ShowSevereError(state,
                                    std::format("SizeHeatingCoil: {} {}, Stage {} Nominal Capacity ({:#G} W) must be less than or equal to Stage {} "
                                                "Nominal Capacity ({:#G} W).",
                                                static_cast<int>(heatingCoil.coilType),
                                                heatingCoil.Name,
                                                StageNum,
                                                heatingCoil.MSNominalCapacity(StageNum),
                                                StageNum + 1,
                                                heatingCoil.MSNominalCapacity(StageNum + 1)));
                    ShowFatalError(state, "Preceding conditions cause termination.");
                }
            }
        } else { // not a multi-speed coil
            heatingCoil.NominalCapacity = TempCap;
        }

        if (++NumCoilsSized == state.dataHeatingCoils->NumHeatingCoils) {
            state.dataHeatingCoils->HeatingCoilNumericFields.deallocate(); // remove temporary array for field names at end of sizing
        }

        // create predefined report entries
        switch (heatingCoil.coilType) {
        case HVAC::CoilType::HeatingElectric: {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilType, heatingCoil.Name, "Coil:Heating:Electric");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatingCoil.Name, heatingCoil.NominalCapacity);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatingCoil.Name, heatingCoil.Efficiency);
        } break;
        case HVAC::CoilType::HeatingElectricMultiStage: {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilType, heatingCoil.Name, "Coil:Heating:Electric:MultiStage");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatingCoil.Name, heatingCoil.MSNominalCapacity(heatingCoil.NumOfStages));
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatingCoil.Name, heatingCoil.MSEfficiency(heatingCoil.NumOfStages));
        } break;
        case HVAC::CoilType::HeatingGasOrOtherFuel: {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilType, heatingCoil.Name, "Coil:Heating:Fuel");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatingCoil.Name, heatingCoil.NominalCapacity);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatingCoil.Name, heatingCoil.Efficiency);
        } break;
        case HVAC::CoilType::HeatingGasMultiStage: {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilType, heatingCoil.Name, "Coil:Heating:Gas:MultiStage");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatingCoil.Name, heatingCoil.MSNominalCapacity(heatingCoil.NumOfStages));
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatingCoil.Name, heatingCoil.MSEfficiency(heatingCoil.NumOfStages));
        } break;
        case HVAC::CoilType::HeatingDesuperheater: {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilType, heatingCoil.Name, "Coil:Heating:Desuperheater");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatingCoil.Name, heatingCoil.NominalCapacity);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatingCoil.Name, heatingCoil.Efficiency);
        } break;
        default:
            break;
        }

        // std 229 heating coils existing table adding new variables:
        // pdchHeatCoilUsedAsSupHeat is now reported at coil selection report
        // pdchHeatCoilAirloopName is now reported at coil selection report
        // std 229 Coil Connections New table: now all reported at coil selection report
    }

    void CalcElectricHeatingCoil(EnergyPlusData &state,
                                 int const CoilNum, // index to heating coil
                                 Real64 &QCoilReq,
                                 Real64 &QCoilActual,       // coil load actually delivered (W)
                                 HVAC::FanOp const fanOp,   // fan operating mode
                                 Real64 const PartLoadRatio // part-load ratio of heating coil
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rich Liesen
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Jul. 2016, R. Zhang, Applied the coil supply air temperature sensor offset

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates a simple Electric heating coil with an efficiency

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirMassFlow; // [kg/sec]
        Real64 TempAirOut;  // [C]
        Real64 HeatingCoilLoad;
        Real64 QCoilCap;

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        Real64 Effic = heatingCoil.Efficiency;
        Real64 TempAirIn = heatingCoil.InletAirTemp;
        Real64 Win = heatingCoil.InletAirHumRat;
        Real64 TempSetPoint = heatingCoil.DesiredOutletTemp;

        // If there is a fault of coil SAT Sensor
        if (heatingCoil.FaultyCoilSATFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            // calculate the sensor offset using fault information
            int FaultIndex = heatingCoil.FaultyCoilSATIndex;
            heatingCoil.FaultyCoilSATOffset = state.dataFaultsMgr->FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint -= heatingCoil.FaultyCoilSATOffset;
        }

        //  adjust mass flow rates for cycling fan cycling coil operation
        if (fanOp == HVAC::FanOp::Cycling) {
            if (PartLoadRatio > 0.0) {
                AirMassFlow = heatingCoil.InletAirMassFlowRate / PartLoadRatio;
                QCoilReq /= PartLoadRatio;
            } else {
                AirMassFlow = 0.0;
            }
        } else {
            AirMassFlow = heatingCoil.InletAirMassFlowRate;
        }

        Real64 CapacitanceAir = Psychrometrics::PsyCpAirFnW(Win) * AirMassFlow;

        // If the coil is operating there should be some heating capacitance
        //  across the coil, so do the simulation. If not set outlet to inlet and no load.
        //  Also the coil has to be scheduled to be available.

        // Control output to meet load QCoilReq (QCoilReq is passed in if load controlled, otherwise QCoilReq=-999)
        if ((AirMassFlow > 0.0 && heatingCoil.NominalCapacity > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) && (QCoilReq > 0.0)) {

            // check to see if the Required heating capacity is greater than the user specified capacity.
            if (QCoilReq > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
            } else {
                QCoilCap = QCoilReq;
            }

            TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
            HeatingCoilLoad = QCoilCap;

            // The HeatingCoilLoad is the change in the enthalpy of the Heating
            heatingCoil.ElecUseLoad = HeatingCoilLoad / Effic;

            // Control coil output to meet a setpoint temperature.
        } else if ((AirMassFlow > 0.0 && heatingCoil.NominalCapacity > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) &&
                   (QCoilReq == Node::SensedLoadFlagValue) && (std::abs(TempSetPoint - TempAirIn) > HVAC::TempControlTol)) {

            QCoilCap = CapacitanceAir * (TempSetPoint - TempAirIn);
            // check to see if setpoint above entering temperature. If not, set
            // output to zero.
            if (QCoilCap <= 0.0) {
                QCoilCap = 0.0;
                TempAirOut = TempAirIn;
                // check to see if the Required heating capacity is greater than the user
                // specified capacity.
            } else if (QCoilCap > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
                TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
            } else {
                TempAirOut = TempSetPoint;
            }

            HeatingCoilLoad = QCoilCap;

            // The HeatingCoilLoad is the change in the enthalpy of the Heating
            heatingCoil.ElecUseLoad = HeatingCoilLoad / Effic;

        } else { // If not running Conditions do not change across coil from inlet to outlet

            TempAirOut = TempAirIn;
            HeatingCoilLoad = 0.0;
            heatingCoil.ElecUseLoad = 0.0;
        }

        if (fanOp == HVAC::FanOp::Cycling) {
            heatingCoil.ElecUseLoad *= PartLoadRatio;
            HeatingCoilLoad *= PartLoadRatio;
        }

        heatingCoil.HeatingCoilLoad = HeatingCoilLoad;

        // Set the outlet conditions
        heatingCoil.OutletAirTemp = TempAirOut;

        // This HeatingCoil does not change the moisture or Mass Flow across the component
        heatingCoil.OutletAirHumRat = heatingCoil.InletAirHumRat;
        heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;
        // Set the outlet enthalpys for air and Heating
        heatingCoil.OutletAirEnthalpy = Psychrometrics::PsyHFnTdbW(heatingCoil.OutletAirTemp, heatingCoil.OutletAirHumRat);

        QCoilActual = HeatingCoilLoad;

        // set outlet node temp so parent objects can call calc directly without have to simulate entire model
        state.dataLoopNodes->Node(heatingCoil.AirOutletNodeNum).Temp = heatingCoil.OutletAirTemp;
    }

    void CalcMultiStageElectricHeatingCoil(EnergyPlusData &state,
                                           int const CoilNum,       // the number of the electric heating coil to be simulated
                                           Real64 const SpeedRatio, // SpeedRatio varies between 1.0 (maximum speed) and 0.0 (minimum speed)
                                           Real64 const CycRatio,   // cycling part load ratio
                                           int const StageNum,      // Stage number
                                           HVAC::FanOp const fanOp, // Fan operation mode
                                           Real64 &QCoilActual,     // coil load actually delivered (W)
                                           bool const SuppHeat)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma, FSEC
        //       DATE WRITTEN   January 2013

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the air-side performance and electrical energy use of multistage electric heating coil.

        // METHODOLOGY EMPLOYED:
        // Uses the same methodology as the single stage electric heating unit model (SUBROUTINE CalcelectricHeatingCoil).
        // In addition it assumes that the unit performance is obtained by interpolating between
        // the performance at high stage and that at low stage. If the output needed is below
        // that produced at low stage, the coil cycles between off and low stage.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName = "CalcMultiStageElectricHeatingCoil";
        static constexpr std::string_view RoutineNameAverageLoad = "CalcMultiStageElectricHeatingCoil:Averageload";
        static constexpr std::string_view RoutineNameFullLoad = "CalcMultiStageElectricHeatingCoil:fullload";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OutletAirEnthalpy;    // outlet air enthalpy [J/kg]
        Real64 OutletAirHumRat;      // outlet air humidity ratio [kg/kg]
        Real64 TotCapHS;             // total capacity at high stage [W]
        Real64 TotCapLS;             // total capacity at low stage [W]
        Real64 TotCap;               // total capacity at current stage [W]
        Real64 EffHS;                // total capacity at high stage [W]
        Real64 EffLS;                // total capacity at low stage [W]
        int StageNumHS;              // High stage number
        int StageNumLS;              // Low stage number
        Real64 FullLoadOutAirEnth;   // Outlet full load enthalpy
        Real64 FullLoadOutAirHumRat; // Outlet humidity ratio at full load
        Real64 FullLoadOutAirTemp;   // Outlet temperature at full load
        Real64 FullLoadOutAirRH;     // Outler relative humidity at full load
        Real64 OutletAirTemp;        // Supply ari temperature
        Real64 LSElecHeatingPower;   // Full load power at low stage
        Real64 HSElecHeatingPower;   // Full load power at high stage
        Real64 PartLoadRat;          // part load ratio

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
        if (StageNum > 1) {
            StageNumLS = StageNum - 1;
            StageNumHS = StageNum;
            if (StageNum > heatingCoil.NumOfStages) {
                StageNumLS = heatingCoil.NumOfStages - 1;
                StageNumHS = heatingCoil.NumOfStages;
            }
        } else {
            StageNumLS = 1;
            StageNumHS = 1;
        }

        Real64 AirMassFlow = heatingCoil.InletAirMassFlowRate;
        Real64 InletAirDryBulbTemp = heatingCoil.InletAirTemp;
        Real64 InletAirEnthalpy = heatingCoil.InletAirEnthalpy;
        Real64 InletAirHumRat = heatingCoil.InletAirHumRat;

        Real64 OutdoorPressure = state.dataEnvrn->OutBaroPress;

        if ((AirMassFlow > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) && ((CycRatio > 0.0) || (SpeedRatio > 0.0))) {

            if (StageNum > 1) {

                TotCapLS = heatingCoil.MSNominalCapacity(StageNumLS);
                TotCapHS = heatingCoil.MSNominalCapacity(StageNumHS);

                EffLS = heatingCoil.MSEfficiency(StageNumLS);
                EffHS = heatingCoil.MSEfficiency(StageNumHS);

                // Get full load output and power
                LSElecHeatingPower = TotCapLS / EffLS;
                HSElecHeatingPower = TotCapHS / EffHS;
                OutletAirHumRat = InletAirHumRat;

                // if cycling fan, send coil part-load fraction to on/off fan via HVACDataGlobals
                // IF (FanOpMode .EQ. FanOp::Cycling) OnOffFanPartLoadFraction = 1.0d0

                // Power calculation
                heatingCoil.ElecUseLoad = SpeedRatio * HSElecHeatingPower + (1.0 - SpeedRatio) * LSElecHeatingPower;

                heatingCoil.HeatingCoilLoad = TotCapHS * SpeedRatio + TotCapLS * (1.0 - SpeedRatio);

                OutletAirEnthalpy = InletAirEnthalpy + heatingCoil.HeatingCoilLoad / heatingCoil.InletAirMassFlowRate;
                OutletAirTemp = Psychrometrics::PsyTdbFnHW(OutletAirEnthalpy, OutletAirHumRat);
                FullLoadOutAirRH = Psychrometrics::PsyRhFnTdbWPb(state, OutletAirTemp, OutletAirHumRat, OutdoorPressure, RoutineNameAverageLoad);

                if (FullLoadOutAirRH > 1.0) { // Limit to saturated conditions at FullLoadOutAirEnth
                    OutletAirTemp = Psychrometrics::PsyTsatFnHPb(state, OutletAirEnthalpy, OutdoorPressure, RoutineName);
                    OutletAirHumRat = Psychrometrics::PsyWFnTdbH(state, OutletAirTemp, OutletAirEnthalpy, RoutineName);
                }

                heatingCoil.OutletAirTemp = OutletAirTemp;
                heatingCoil.OutletAirHumRat = OutletAirHumRat;
                heatingCoil.OutletAirEnthalpy = OutletAirEnthalpy;
                heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;

                // Stage 1
            } else if (CycRatio > 0.0) {

                PartLoadRat = min(1.0, CycRatio);

                // for cycling fan, reset mass flow to full on rate
                if (fanOp == HVAC::FanOp::Cycling) {
                    AirMassFlow /= PartLoadRat;
                } else if (fanOp == HVAC::FanOp::Continuous) {
                    if (!SuppHeat) {
                        AirMassFlow = state.dataHVACGlobal->MSHPMassFlowRateLow;
                    }
                }

                TotCap = heatingCoil.MSNominalCapacity(StageNumLS);

                // Calculate full load outlet conditions
                FullLoadOutAirEnth = InletAirEnthalpy + TotCap / AirMassFlow;
                FullLoadOutAirHumRat = InletAirHumRat;
                FullLoadOutAirTemp = Psychrometrics::PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
                FullLoadOutAirRH =
                    Psychrometrics::PsyRhFnTdbWPb(state, FullLoadOutAirTemp, FullLoadOutAirHumRat, OutdoorPressure, RoutineNameFullLoad);

                if (FullLoadOutAirRH > 1.0) { // Limit to saturated conditions at FullLoadOutAirEnth
                    FullLoadOutAirTemp = Psychrometrics::PsyTsatFnHPb(state, FullLoadOutAirEnth, OutdoorPressure, RoutineName);
                    //  Eventually inlet air conditions will be used in electric Coil, these lines are commented out and marked with this comment
                    //  line FullLoadOutAirTemp = PsyTsatFnHPb(FullLoadOutAirEnth,InletAirPressure)
                    FullLoadOutAirHumRat = Psychrometrics::PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
                }

                // Set outlet conditions from the full load calculation
                if (fanOp == HVAC::FanOp::Cycling) {
                    OutletAirEnthalpy = FullLoadOutAirEnth;
                    OutletAirHumRat = FullLoadOutAirHumRat;
                    OutletAirTemp = FullLoadOutAirTemp;
                } else {
                    OutletAirEnthalpy = PartLoadRat * FullLoadOutAirEnth + (1.0 - PartLoadRat) * InletAirEnthalpy;
                    OutletAirHumRat = PartLoadRat * FullLoadOutAirHumRat + (1.0 - PartLoadRat) * InletAirHumRat;
                    OutletAirTemp = PartLoadRat * FullLoadOutAirTemp + (1.0 - PartLoadRat) * InletAirDryBulbTemp;
                }

                EffLS = heatingCoil.MSEfficiency(StageNumLS);

                //    HeatingCoil(CoilNum)%HeatingCoilLoad = TotCap
                //   This would require a CR to change
                heatingCoil.HeatingCoilLoad = TotCap * PartLoadRat;

                heatingCoil.ElecUseLoad = heatingCoil.HeatingCoilLoad / EffLS;

                heatingCoil.OutletAirTemp = OutletAirTemp;
                heatingCoil.OutletAirHumRat = OutletAirHumRat;
                heatingCoil.OutletAirEnthalpy = OutletAirEnthalpy;
                heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;
                // this would require a CR to correct (i.e., calculate outputs when coil is off)
                //  ELSE
                //    ! electric coil is off; just pass through conditions
                //    HeatingCoil(CoilNum)%OutletAirEnthalpy = HeatingCoil(CoilNum)%InletAirEnthalpy
                //    HeatingCoil(CoilNum)%OutletAirHumRat   = HeatingCoil(CoilNum)%InletAirHumRat
                //    HeatingCoil(CoilNum)%OutletAirTemp     = HeatingCoil(CoilNum)%InletAirTemp
                //    HeatingCoil(CoilNum)%OutletAirMassFlowRate = HeatingCoil(CoilNum)%InletAirMassFlowRate
                //    HeatingCoil(CoilNum)%ElecUseLoad      = 0.0
                //    HeatingCoil(CoilNum)%HeatingCoilLoad  = 0.0
                //    ElecHeatingCoilPower                  = 0.0
            }

        } else {

            // electric coil is off; just pass through conditions
            heatingCoil.OutletAirEnthalpy = heatingCoil.InletAirEnthalpy;
            heatingCoil.OutletAirHumRat = heatingCoil.InletAirHumRat;
            heatingCoil.OutletAirTemp = heatingCoil.InletAirTemp;
            heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;

            // some of these are reset in Init, can be removed to speed up code
            heatingCoil.ElecUseLoad = 0.0;
            heatingCoil.HeatingCoilLoad = 0.0;

        } // end of on/off if - else

        // set outlet node temp so parent objects can call calc directly without have to simulate entire model
        state.dataLoopNodes->Node(heatingCoil.AirOutletNodeNum).Temp = heatingCoil.OutletAirTemp;

        QCoilActual = heatingCoil.HeatingCoilLoad;
    }

    void CalcFuelHeatingCoil(EnergyPlusData &state,
                             int const CoilNum, // index to heating coil
                             Real64 const QCoilReq,
                             Real64 &QCoilActual,                        // coil load actually delivered (W)
                             HVAC::FanOp const fanOp,                    // fan operating mode
                             [[maybe_unused]] Real64 const PartLoadRatio // part-load ratio of heating coil
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rich Liesen
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Jul. 2016, R. Zhang, Applied the coil supply air temperature sensor offset

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates a simple Gas heating coil with a burner efficiency

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TempAirOut; // [C]
        Real64 HeatingCoilLoad;
        Real64 QCoilCap;
        Real64 PartLoadRat;
        Real64 PLF;

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        Real64 Effic = heatingCoil.Efficiency;
        Real64 TempAirIn = heatingCoil.InletAirTemp;
        Real64 Win = heatingCoil.InletAirHumRat;
        Real64 TempSetPoint = heatingCoil.DesiredOutletTemp;
        Real64 AirMassFlow = heatingCoil.InletAirMassFlowRate;

        Real64 CapacitanceAir = Psychrometrics::PsyCpAirFnW(Win) * AirMassFlow;

        // If there is a fault of coil SAT Sensor
        if (heatingCoil.FaultyCoilSATFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            // calculate the sensor offset using fault information
            int FaultIndex = heatingCoil.FaultyCoilSATIndex;
            heatingCoil.FaultyCoilSATOffset = state.dataFaultsMgr->FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint -= heatingCoil.FaultyCoilSATOffset;
        }

        // If the coil is operating there should be some heating capacitance
        //  across the coil, so do the simulation. If not set outlet to inlet and no load.
        //  Also the coil has to be scheduled to be available.

        // Control output to meet load QCoilReq (QCoilReq is passed in if load controlled, otherwise QCoilReq=-999)
        if ((AirMassFlow > 0.0 && heatingCoil.NominalCapacity > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) && (QCoilReq > 0.0)) {

            // check to see if the Required heating capacity is greater than the user specified capacity.
            if (QCoilReq > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
            } else {
                QCoilCap = QCoilReq;
            }

            TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
            HeatingCoilLoad = QCoilCap;

            PartLoadRat = HeatingCoilLoad / heatingCoil.NominalCapacity;

            // The HeatingCoilLoad is the change in the enthalpy of the Heating
            heatingCoil.FuelUseLoad = HeatingCoilLoad / Effic;
            heatingCoil.ElecUseLoad = heatingCoil.ParasiticElecLoad * PartLoadRat;
            heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity * (1.0 - PartLoadRat);

            // Control coil output to meet a setpoint temperature.
        } else if ((AirMassFlow > 0.0 && heatingCoil.NominalCapacity > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) &&
                   (QCoilReq == Node::SensedLoadFlagValue) && (std::abs(TempSetPoint - TempAirIn) > HVAC::TempControlTol)) {

            QCoilCap = CapacitanceAir * (TempSetPoint - TempAirIn);
            // check to see if setpoint above entering temperature. If not, set
            // output to zero.
            if (QCoilCap <= 0.0) {
                QCoilCap = 0.0;
                TempAirOut = TempAirIn;
                // check to see if the Required heating capacity is greater than the user
                // specified capacity.
            } else if (QCoilCap > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
                TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
            } else {
                TempAirOut = TempSetPoint;
            }

            HeatingCoilLoad = QCoilCap;

            PartLoadRat = HeatingCoilLoad / heatingCoil.NominalCapacity;

            // The HeatingCoilLoad is the change in the enthalpy of the Heating
            heatingCoil.FuelUseLoad = HeatingCoilLoad / Effic;
            heatingCoil.ElecUseLoad = heatingCoil.ParasiticElecLoad * PartLoadRat;
            heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity * (1.0 - PartLoadRat);

        } else { // If not running Conditions do not change across coil from inlet to outlet

            TempAirOut = TempAirIn;
            HeatingCoilLoad = 0.0;
            PartLoadRat = 0.0;
            heatingCoil.FuelUseLoad = 0.0;
            heatingCoil.ElecUseLoad = 0.0;
            heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity;
        }

        heatingCoil.RTF = PartLoadRat;

        // If the PLF curve is defined the gas usage needs to be modified
        if (heatingCoil.PLFCurveIndex > 0) {
            if (PartLoadRat == 0) {
                heatingCoil.FuelUseLoad = 0.0;
            } else {
                PLF = Curve::CurveValue(state, heatingCoil.PLFCurveIndex, PartLoadRat);
                if (PLF < 0.7) {
                    if (heatingCoil.PLFErrorCount < 1) {
                        ++heatingCoil.PLFErrorCount;
                        ShowWarningError(state,
                                         std::format("CalcFuelHeatingCoil: {}=\"{}\", PLF curve values",
                                                     HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                                     heatingCoil.Name));
                        ShowContinueError(state, std::format("The PLF curve value = {:.5f} for part-load ratio = {:.5f}", PLF, PartLoadRat));
                        ShowContinueError(state, "PLF curve values must be >= 0.7. PLF has been reset to 0.7 and the simulation continues...");
                        ShowContinueError(state, "Check the IO reference manual for PLF curve guidance [Coil:Heating:Fuel].");
                    } else {
                        ShowRecurringWarningErrorAtEnd(
                            state, heatingCoil.Name + ", Heating coil PLF curve < 0.7 warning continues... ", heatingCoil.PLFErrorIndex, PLF, PLF);
                    }
                    PLF = 0.7;
                }
                // Modify the Gas Coil Consumption and parasitic loads based on PLF curve
                heatingCoil.RTF = PartLoadRat / PLF;
                if (heatingCoil.RTF > 1.0 && std::abs(heatingCoil.RTF - 1.0) > 0.001) {
                    if (heatingCoil.RTFErrorCount < 1) {
                        ++heatingCoil.RTFErrorCount;
                        ShowWarningError(state,
                                         std::format("CalcFuelHeatingCoil: {}=\"{}\", runtime fraction",
                                                     HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                                     heatingCoil.Name));
                        ShowContinueError(state, std::format("The runtime fraction exceeded 1.0. [{:.4f}].", heatingCoil.RTF));
                        ShowContinueError(state, "Runtime fraction is set to 1.0 and the simulation continues...");
                        ShowContinueError(state, "Check the IO reference manual for PLF curve guidance [Coil:Heating:Fuel].");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::format("{}, Heating coil runtime fraction > 1.0 warning continues... ", heatingCoil.Name),
                                                       heatingCoil.RTFErrorIndex,
                                                       heatingCoil.RTF,
                                                       heatingCoil.RTF);
                    }
                    heatingCoil.RTF = 1.0; // Reset coil runtime fraction to 1.0
                } else if (heatingCoil.RTF > 1.0) {
                    heatingCoil.RTF = 1.0; // Reset coil runtime fraction to 1.0
                }
                heatingCoil.ElecUseLoad = heatingCoil.ParasiticElecLoad * heatingCoil.RTF;
                heatingCoil.FuelUseLoad = heatingCoil.NominalCapacity / Effic * heatingCoil.RTF;
                heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity * (1.0 - heatingCoil.RTF);
                // Fan power will also be modified by the heating coil's part load fraction
                // OnOffFanPartLoadFraction passed to fan via DataHVACGlobals (cycling fan only)
                if (fanOp == HVAC::FanOp::Cycling) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = PLF;
                }
            }
        }

        // Set the outlet conditions
        heatingCoil.HeatingCoilLoad = HeatingCoilLoad;
        heatingCoil.OutletAirTemp = TempAirOut;

        // This HeatingCoil does not change the moisture or Mass Flow across the component
        heatingCoil.OutletAirHumRat = heatingCoil.InletAirHumRat;
        heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;
        // Set the outlet enthalpys for air and Heating
        heatingCoil.OutletAirEnthalpy = Psychrometrics::PsyHFnTdbW(heatingCoil.OutletAirTemp, heatingCoil.OutletAirHumRat);

        QCoilActual = HeatingCoilLoad;
        state.dataHVACGlobal->ElecHeatingCoilPower = heatingCoil.ElecUseLoad;

        // set outlet node temp so parent objects can call calc directly without have to simulate entire model
        state.dataLoopNodes->Node(heatingCoil.AirOutletNodeNum).Temp = heatingCoil.OutletAirTemp;
    }

    void CalcMultiStageGasHeatingCoil(EnergyPlusData &state,
                                      int const CoilNum,       // the number of the Gas heating coil to be simulated
                                      Real64 const SpeedRatio, // SpeedRatio varies between 1.0 (maximum speed) and 0.0 (minimum speed)
                                      Real64 const CycRatio,   // cycling part load ratio
                                      int const StageNum,      // Speed number
                                      HVAC::FanOp const fanOp  // Fan operation mode
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma, FSEC
        //       DATE WRITTEN   January 2013

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the air-side performance and energy use of a multi stage gas heating coil.

        // METHODOLOGY EMPLOYED:
        // Uses the same methodology as the single speed Gas heating unit model (SUBROUTINE CalcFuelHeatingCoil).
        // In addition it assumes that the unit performance is obtained by interpolating between
        // the performance at high stage and that at low stage. If the output needed is below
        // that produced at low stage, the coil cycles between off and low stage.

        Real64 const MSHPMassFlowRateHigh = state.dataHVACGlobal->MSHPMassFlowRateHigh;
        Real64 const MSHPMassFlowRateLow = state.dataHVACGlobal->MSHPMassFlowRateLow;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("CalcMultiStageGasHeatingCoil");
        static constexpr std::string_view RoutineNameAverageLoad("CalcMultiStageGasHeatingCoil:Averageload");
        static constexpr std::string_view RoutineNameFullLoad("CalcMultiStageGasHeatingCoil:fullload");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OutletAirEnthalpy;    // outlet air enthalpy [J/kg]
        Real64 OutletAirHumRat;      // outlet air humidity ratio [kg/kg]
        Real64 TotCapHS;             // total capacity at high stage [W]
        Real64 TotCapLS;             // total capacity at low stage [W]
        Real64 TotCap;               // total capacity at current stage [W]
        Real64 EffHS;                // efficiency at high stage
        Real64 EffLS(0.0);           // efficiency at low stage
        Real64 EffAvg;               // average efficiency
        int StageNumHS;              // High stage number
        int StageNumLS;              // Low stage number
        Real64 FullLoadOutAirEnth;   // Outlet full load enthalpy
        Real64 FullLoadOutAirHumRat; // Outlet humidity ratio at full load
        Real64 FullLoadOutAirTemp;   // Outlet temperature at full load
        Real64 FullLoadOutAirRH;     // Outler relative humidity at full load
        Real64 OutletAirTemp;        // Supply ari temperature
        Real64 LSFullLoadOutAirEnth; // Outlet full load enthalpy at low stage
        Real64 HSFullLoadOutAirEnth; // Outlet full load enthalpy at high stage
        Real64 LSGasHeatingPower;    // Full load power at low stage
        Real64 HSGasHeatingPower;    // Full load power at high stage
        Real64 PartLoadRat(0.0);     // part load ratio
        Real64 PLF;                  // part load factor used to calculate RTF

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        if (StageNum > 1) {
            StageNumLS = StageNum - 1;
            StageNumHS = StageNum;
            if (StageNum > heatingCoil.NumOfStages) {
                StageNumLS = heatingCoil.NumOfStages - 1;
                StageNumHS = heatingCoil.NumOfStages;
            }
        } else {
            StageNumLS = 1;
            StageNumHS = 1;
        }

        Real64 AirMassFlow = heatingCoil.InletAirMassFlowRate;
        Real64 InletAirEnthalpy = heatingCoil.InletAirEnthalpy;
        Real64 InletAirHumRat = heatingCoil.InletAirHumRat;
        Real64 OutdoorPressure = state.dataEnvrn->OutBaroPress;

        if ((AirMassFlow > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) && ((CycRatio > 0.0) || (SpeedRatio > 0.0))) {

            if (StageNum > 1) {

                TotCapLS = heatingCoil.MSNominalCapacity(StageNumLS);
                TotCapHS = heatingCoil.MSNominalCapacity(StageNumHS);

                EffLS = heatingCoil.MSEfficiency(StageNumLS);
                EffHS = heatingCoil.MSEfficiency(StageNumHS);

                PartLoadRat = min(1.0, SpeedRatio);
                heatingCoil.RTF = 1.0;

                // Get full load output and power
                LSFullLoadOutAirEnth = InletAirEnthalpy + TotCapLS / MSHPMassFlowRateLow;
                HSFullLoadOutAirEnth = InletAirEnthalpy + TotCapHS / MSHPMassFlowRateHigh;
                LSGasHeatingPower = TotCapLS / EffLS;
                HSGasHeatingPower = TotCapHS / EffHS;
                OutletAirHumRat = InletAirHumRat;

                // if cycling fan, send coil part-load fraction to on/off fan via HVACDataGlobals
                // IF (FanOpMode .EQ. FanOp::Cycling) OnOffFanPartLoadFraction = 1.0d0

                // Power calculation. If PartLoadRat (SpeedRatio) = 0, operate at LS the whole time step
                heatingCoil.ElecUseLoad =
                    PartLoadRat * heatingCoil.MSParasiticElecLoad(StageNumHS) + (1.0 - PartLoadRat) * heatingCoil.MSParasiticElecLoad(StageNumLS);

                state.dataHVACGlobal->ElecHeatingCoilPower = heatingCoil.ElecUseLoad;
                heatingCoil.HeatingCoilLoad = MSHPMassFlowRateHigh * (HSFullLoadOutAirEnth - InletAirEnthalpy) * PartLoadRat +
                                              MSHPMassFlowRateLow * (LSFullLoadOutAirEnth - InletAirEnthalpy) * (1.0 - PartLoadRat);
                EffAvg = (EffHS * PartLoadRat) + (EffLS * (1.0 - PartLoadRat));
                heatingCoil.FuelUseLoad = heatingCoil.HeatingCoilLoad / EffAvg;
                heatingCoil.ParasiticFuelRate = 0.0;

                OutletAirEnthalpy = InletAirEnthalpy + heatingCoil.HeatingCoilLoad / heatingCoil.InletAirMassFlowRate;
                OutletAirTemp = Psychrometrics::PsyTdbFnHW(OutletAirEnthalpy, OutletAirHumRat);
                FullLoadOutAirRH = Psychrometrics::PsyRhFnTdbWPb(state, OutletAirTemp, OutletAirHumRat, OutdoorPressure, RoutineNameAverageLoad);

                if (FullLoadOutAirRH > 1.0) { // Limit to saturated conditions at FullLoadOutAirEnth
                    OutletAirTemp = Psychrometrics::PsyTsatFnHPb(state, OutletAirEnthalpy, OutdoorPressure, RoutineName);
                    OutletAirHumRat = Psychrometrics::PsyWFnTdbH(state, OutletAirTemp, OutletAirEnthalpy, RoutineName);
                }

                heatingCoil.OutletAirTemp = OutletAirTemp;
                heatingCoil.OutletAirHumRat = OutletAirHumRat;
                heatingCoil.OutletAirEnthalpy = OutletAirEnthalpy;
                heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;

                // Stage 1
            } else if (CycRatio > 0.0) {

                // for cycling fan, reset mass flow to full on rate
                if (fanOp == HVAC::FanOp::Cycling) {
                    AirMassFlow /= CycRatio;
                } else if (fanOp == HVAC::FanOp::Continuous) {
                    AirMassFlow = MSHPMassFlowRateLow;
                }

                TotCap = heatingCoil.MSNominalCapacity(StageNumLS);

                PartLoadRat = min(1.0, CycRatio);
                heatingCoil.RTF = PartLoadRat;

                // Calculate full load outlet conditions
                FullLoadOutAirEnth = InletAirEnthalpy + TotCap / AirMassFlow;
                FullLoadOutAirHumRat = InletAirHumRat;
                FullLoadOutAirTemp = Psychrometrics::PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
                FullLoadOutAirRH =
                    Psychrometrics::PsyRhFnTdbWPb(state, FullLoadOutAirTemp, FullLoadOutAirHumRat, OutdoorPressure, RoutineNameFullLoad);

                if (FullLoadOutAirRH > 1.0) { // Limit to saturated conditions at FullLoadOutAirEnth
                    FullLoadOutAirTemp = Psychrometrics::PsyTsatFnHPb(state, FullLoadOutAirEnth, OutdoorPressure, RoutineName);
                    //  Eventually inlet air conditions will be used in Gas Coil, these lines are commented out and marked with this comment line
                    //  FullLoadOutAirTemp = PsyTsatFnHPb(FullLoadOutAirEnth,InletAirPressure)
                    FullLoadOutAirHumRat = Psychrometrics::PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
                }

                // Set outlet conditions from the full load calculation
                if (fanOp == HVAC::FanOp::Cycling) {
                    OutletAirEnthalpy = FullLoadOutAirEnth;
                    OutletAirHumRat = FullLoadOutAirHumRat;
                    OutletAirTemp = FullLoadOutAirTemp;
                } else {
                    OutletAirEnthalpy =
                        PartLoadRat * AirMassFlow / heatingCoil.InletAirMassFlowRate * (FullLoadOutAirEnth - InletAirEnthalpy) + InletAirEnthalpy;
                    OutletAirHumRat =
                        PartLoadRat * AirMassFlow / heatingCoil.InletAirMassFlowRate * (FullLoadOutAirHumRat - InletAirHumRat) + InletAirHumRat;
                    OutletAirTemp = Psychrometrics::PsyTdbFnHW(OutletAirEnthalpy, OutletAirHumRat);
                }

                EffLS = heatingCoil.MSEfficiency(StageNumLS);

                heatingCoil.HeatingCoilLoad = TotCap * PartLoadRat;

                heatingCoil.FuelUseLoad = heatingCoil.HeatingCoilLoad / EffLS;
                //   parasitics are calculated when the coil is off (1-PLR)
                heatingCoil.ElecUseLoad = heatingCoil.MSParasiticElecLoad(StageNumLS) * (1.0 - PartLoadRat);
                heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity * (1.0 - PartLoadRat);
                state.dataHVACGlobal->ElecHeatingCoilPower = heatingCoil.ElecUseLoad;

                heatingCoil.OutletAirTemp = OutletAirTemp;
                heatingCoil.OutletAirHumRat = OutletAirHumRat;
                heatingCoil.OutletAirEnthalpy = OutletAirEnthalpy;
                heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;
            }

            // This requires a CR to correct (i.e., calculate outputs when coil is off)
        } else {

            // Gas coil is off; just pass through conditions
            heatingCoil.OutletAirEnthalpy = heatingCoil.InletAirEnthalpy;
            heatingCoil.OutletAirHumRat = heatingCoil.InletAirHumRat;
            heatingCoil.OutletAirTemp = heatingCoil.InletAirTemp;
            heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;

            // some of these are reset in Init, can be removed to speed up code
            heatingCoil.ElecUseLoad = 0.0;
            heatingCoil.HeatingCoilLoad = 0.0;
            heatingCoil.FuelUseLoad = 0.0;
            heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity;
            state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
            PartLoadRat = 0.0;

        } // end of on/off if - else

        // If the PLF curve is defined the gas usage needs to be modified.
        // The PLF curve is only used when the coil cycles.
        if (heatingCoil.PLFCurveIndex > 0) {
            if (PartLoadRat > 0.0 && StageNum < 2) {
                PLF = Curve::CurveValue(state, heatingCoil.PLFCurveIndex, PartLoadRat);
                if (PLF < 0.7) {
                    if (heatingCoil.PLFErrorCount < 1) {
                        ++heatingCoil.PLFErrorCount;
                        ShowWarningError(state,
                                         std::format("CalcFuelHeatingCoil: {}=\"{}\", PLF curve values",
                                                     HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                                     heatingCoil.Name));
                        ShowContinueError(state, std::format("The PLF curve value = {:.5f} for part-load ratio = {:.5f}", PLF, PartLoadRat));
                        ShowContinueError(state, "PLF curve values must be >= 0.7. PLF has been reset to 0.7 and the simulation continues...");
                        ShowContinueError(state, "Check the IO reference manual for PLF curve guidance [Coil:Heating:Fuel].");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::format("{}, Heating coil PLF curve < 0.7 warning continues... ", heatingCoil.Name),
                                                       heatingCoil.PLFErrorIndex,
                                                       PLF,
                                                       PLF);
                    }
                    PLF = 0.7;
                }
                // Modify the Gas Coil Consumption and parasitic loads based on PLF curve
                heatingCoil.RTF = PartLoadRat / PLF;
                if (heatingCoil.RTF > 1.0 && std::abs(heatingCoil.RTF - 1.0) > 0.001) {
                    if (heatingCoil.RTFErrorCount < 1) {
                        ++heatingCoil.RTFErrorCount;
                        ShowWarningError(state,
                                         std::format("CalcFuelHeatingCoil: {}=\"{}\", runtime fraction",
                                                     HVAC::coilTypeNames[(int)heatingCoil.coilType],
                                                     heatingCoil.Name));
                        ShowContinueError(state, std::format("The runtime fraction exceeded 1.0. [{:.4f}].", heatingCoil.RTF));
                        ShowContinueError(state, "Runtime fraction is set to 1.0 and the simulation continues...");
                        ShowContinueError(state, "Check the IO reference manual for PLF curve guidance [Coil:Heating:Fuel].");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::format("{}, Heating coil runtime fraction > 1.0 warning continues... ", heatingCoil.Name),
                                                       heatingCoil.RTFErrorIndex,
                                                       heatingCoil.RTF,
                                                       heatingCoil.RTF);
                    }
                    heatingCoil.RTF = 1.0; // Reset coil runtime fraction to 1.0
                } else if (heatingCoil.RTF > 1.0) {
                    heatingCoil.RTF = 1.0; // Reset coil runtime fraction to 1.0
                }
                heatingCoil.ElecUseLoad = heatingCoil.MSParasiticElecLoad(StageNum) * heatingCoil.RTF;
                heatingCoil.FuelUseLoad = (heatingCoil.MSNominalCapacity(StageNum) / EffLS) * heatingCoil.RTF;
                heatingCoil.ParasiticFuelRate = heatingCoil.ParasiticFuelCapacity * (1.0 - heatingCoil.RTF);
                // Fan power will also be modified by the heating coil's part load fraction
                // OnOffFanPartLoadFraction passed to fan via DataHVACGlobals (cycling fan only)
                if (fanOp == HVAC::FanOp::Cycling) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = PLF;
                }
            }
        }

        // set outlet node temp so parent objects can call calc directly without have to simulate entire model
        state.dataLoopNodes->Node(heatingCoil.AirOutletNodeNum).Temp = heatingCoil.OutletAirTemp;
    }

    void CalcDesuperheaterHeatingCoil(EnergyPlusData &state,
                                      int const CoilNum,     // index to desuperheater heating coil
                                      Real64 const QCoilReq, // load requested by the simulation for load based control [W]
                                      Real64 &QCoilActual    // coil load actually delivered
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   January 2005
        //       MODIFIED       Jul. 2016, R. Zhang, Applied the coil supply air temperature sensor offset

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates a simple desuperheater heating coil with a heat reclaim efficiency
        // (eff = ratio of condenser waste heat reclaimed to total condenser waste heat rejected)

        // METHODOLOGY EMPLOYED:
        // The available capacity of the desuperheater heating coil is determined by the
        // amount of heat rejected at the heating source condenser multiplied by the
        // desuperheater heat reclaim efficiency. This capacity is either applied towards
        // a requested load (load based control) or applied to the air stream to meet a
        // heating setpoint (temperature based control). This subroutine is similar to
        // the electric or gas heating coil except that the NominalCapacity is variable
        // and based on the runtime fraction and heat rejection of the heat source object.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AvailTemp;       // Lowest temperature available from desuperheater (~T condensing)[C]
        Real64 TempAirOut;      // temperature of the air leaving the desuperheater heating coil [C]
        Real64 HeatingCoilLoad; // actual load delivered by the desuperheater heating coil [W]
        Real64 QCoilCap;        // available capacity of the desuperheater heating coil [W]
        int SourceID;           // waste heat source id number

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        Real64 Effic = heatingCoil.Efficiency;
        Real64 AirMassFlow = heatingCoil.InletAirMassFlowRate;
        Real64 TempAirIn = heatingCoil.InletAirTemp;
        Real64 Win = heatingCoil.InletAirHumRat;
        Real64 CapacitanceAir = Psychrometrics::PsyCpAirFnW(Win) * AirMassFlow;
        Real64 TempSetPoint = heatingCoil.DesiredOutletTemp;

        // If there is a fault of coil SAT Sensor
        if (heatingCoil.FaultyCoilSATFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            // calculate the sensor offset using fault information
            int FaultIndex = heatingCoil.FaultyCoilSATIndex;
            heatingCoil.FaultyCoilSATOffset = state.dataFaultsMgr->FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint -= heatingCoil.FaultyCoilSATOffset;
        }

        // Access the appropriate structure to find the available heating capacity of the desuperheater heating coil
        // The nominal capacity of the desuperheater heating coil varies based on the amount of heat rejected by the source
        // Stovall 2011, add comparison to available temperature of heat reclaim source
        if (state.dataHeatingCoils->ValidSourceType(CoilNum)) {
            SourceID = heatingCoil.ReclaimHeatingSourceIndexNum;
            switch (heatingCoil.ReclaimHeatingSource) {
            case HeatObjTypes::COMPRESSORRACK_REFRIGERATEDCASE:
                // Added last term to available energy equations to avoid double counting reclaimed energy
                // because refrigeration systems are solved outside the hvac time step iterations
                heatingCoil.RTF = 1.0;
                heatingCoil.NominalCapacity = state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).AvailCapacity * Effic -
                                              state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).WaterHeatingDesuperheaterReclaimedHeatTotal;
                break;
            case HeatObjTypes::CONDENSER_REFRIGERATION:
                AvailTemp = state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).AvailTemperature;
                heatingCoil.RTF = 1.0;
                if (AvailTemp <= TempAirIn) {
                    heatingCoil.NominalCapacity = 0.0;
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        std::format("Coil:Heating:Desuperheater {} - Waste heat source temperature was too low to be useful.", heatingCoil.Name),
                        heatingCoil.InsuffTemperatureWarn);
                } else {
                    heatingCoil.NominalCapacity = state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).AvailCapacity * Effic -
                                                  state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).WaterHeatingDesuperheaterReclaimedHeatTotal;
                }
                break;
            case HeatObjTypes::COIL_DX_COOLING:
            case HeatObjTypes::COIL_DX_MULTISPEED:
            case HeatObjTypes::COIL_DX_MULTIMODE:
                heatingCoil.RTF = state.dataDXCoils->DXCoil(SourceID).CoolingCoilRuntimeFraction;
                heatingCoil.NominalCapacity = state.dataHeatBal->HeatReclaimDXCoil(SourceID).AvailCapacity * Effic -
                                              state.dataHeatBal->HeatReclaimDXCoil(SourceID).WaterHeatingDesuperheaterReclaimedHeatTotal;
                break;
            case HeatObjTypes::COIL_DX_VARIABLE_COOLING:
                // condenser heat rejection
                heatingCoil.RTF = state.dataVariableSpeedCoils->VarSpeedCoil(SourceID).RunFrac;
                heatingCoil.NominalCapacity = state.dataHeatBal->HeatReclaimVS_Coil(SourceID).AvailCapacity * Effic -
                                              state.dataHeatBal->HeatReclaimVS_Coil(SourceID).WaterHeatingDesuperheaterReclaimedHeatTotal;
                break;
            case HeatObjTypes::COIL_COOLING_DX_NEW:
                // get RTF and NominalCapacity from Coil:CoolingDX
                {
                    auto const &thisCoolingCoil = state.dataCoilCoolingDX->coilCoolingDXs[SourceID];
                    heatingCoil.RTF = thisCoolingCoil.runTimeFraction;
                    heatingCoil.NominalCapacity =
                        thisCoolingCoil.reclaimHeat.AvailCapacity * Effic - thisCoolingCoil.reclaimHeat.WaterHeatingDesuperheaterReclaimedHeatTotal;
                }
                break;
            default:
                assert(false);
            }

        } else {
            heatingCoil.NominalCapacity = 0.0;
        }

        // Control output to meet load (QCoilReq)
        if ((AirMassFlow > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) && (QCoilReq > 0.0)) {

            // check to see if the Required heating capacity is greater than the available heating capacity.
            if (QCoilReq > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
            } else {
                QCoilCap = QCoilReq;
            }

            // report the runtime fraction of the desuperheater heating coil
            if (heatingCoil.NominalCapacity > 0.0) {
                heatingCoil.RTF *= (QCoilCap / heatingCoil.NominalCapacity);
                TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
                HeatingCoilLoad = QCoilCap;
            } else {
                heatingCoil.RTF = 0.0;
                TempAirOut = TempAirIn;
                HeatingCoilLoad = 0.0;
            }

            // Control coil output to meet a setpoint temperature.
        } else if ((AirMassFlow > 0.0 && heatingCoil.NominalCapacity > 0.0) && (heatingCoil.availSched->getCurrentVal() > 0.0) &&
                   (QCoilReq == Node::SensedLoadFlagValue) && (std::abs(TempSetPoint - TempAirIn) > HVAC::TempControlTol)) {

            QCoilCap = CapacitanceAir * (TempSetPoint - TempAirIn);
            // check to see if setpoint is above entering air temperature. If not, set output to zero.
            if (QCoilCap <= 0.0) {
                QCoilCap = 0.0;
                TempAirOut = TempAirIn;
                // check to see if the required heating capacity is greater than the available capacity.
            } else if (QCoilCap > heatingCoil.NominalCapacity) {
                QCoilCap = heatingCoil.NominalCapacity;
                TempAirOut = TempAirIn + QCoilCap / CapacitanceAir;
            } else {
                TempAirOut = TempSetPoint;
            }

            HeatingCoilLoad = QCoilCap;
            //     report the runtime fraction of the desuperheater heating coil
            heatingCoil.RTF *= (QCoilCap / heatingCoil.NominalCapacity);

        } else { // If not running, conditions do not change across heating coil from inlet to outlet

            TempAirOut = TempAirIn;
            HeatingCoilLoad = 0.0;
            heatingCoil.ElecUseLoad = 0.0;
            heatingCoil.RTF = 0.0;
        }

        // Set the outlet conditions
        heatingCoil.HeatingCoilLoad = HeatingCoilLoad;
        heatingCoil.OutletAirTemp = TempAirOut;

        // This HeatingCoil does not change the moisture or Mass Flow across the component
        heatingCoil.OutletAirHumRat = heatingCoil.InletAirHumRat;
        heatingCoil.OutletAirMassFlowRate = heatingCoil.InletAirMassFlowRate;
        // Set the outlet enthalpy
        heatingCoil.OutletAirEnthalpy = Psychrometrics::PsyHFnTdbW(heatingCoil.OutletAirTemp, heatingCoil.OutletAirHumRat);

        heatingCoil.ElecUseLoad = heatingCoil.ParasiticElecLoad * heatingCoil.RTF;
        QCoilActual = HeatingCoilLoad;

        // Update remaining waste heat (just in case multiple users of waste heat use same source)
        if (state.dataHeatingCoils->ValidSourceType(CoilNum)) {
            SourceID = heatingCoil.ReclaimHeatingSourceIndexNum;
            //   Refrigerated cases are simulated at the zone time step, do not decrement available capacity
            //   (the heat reclaim available capacity will not get reinitialized as the air loop iterates)
            int DesuperheaterNum = CoilNum - state.dataHeatingCoils->NumElecCoil - state.dataHeatingCoils->NumElecCoilMultiStage -
                                   state.dataHeatingCoils->NumFuelCoil - state.dataHeatingCoils->NumGasCoilMultiStage;
            switch (heatingCoil.ReclaimHeatingSource) {
            case HeatObjTypes::COMPRESSORRACK_REFRIGERATEDCASE: {
                state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).HVACDesuperheaterReclaimedHeat(DesuperheaterNum) = HeatingCoilLoad;
                state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).HVACDesuperheaterReclaimedHeatTotal = 0.0;
                for (auto const &num : state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).HVACDesuperheaterReclaimedHeat) {
                    state.dataHeatBal->HeatReclaimRefrigeratedRack(SourceID).HVACDesuperheaterReclaimedHeatTotal += num;
                }
            } break;
            case HeatObjTypes::CONDENSER_REFRIGERATION: {
                state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).HVACDesuperheaterReclaimedHeat(DesuperheaterNum) = HeatingCoilLoad;
                state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).HVACDesuperheaterReclaimedHeatTotal = 0.0;
                for (auto const &num : state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).HVACDesuperheaterReclaimedHeat) {
                    state.dataHeatBal->HeatReclaimRefrigCondenser(SourceID).HVACDesuperheaterReclaimedHeatTotal += num;
                }
            } break;
            case HeatObjTypes::COIL_DX_COOLING:
            case HeatObjTypes::COIL_DX_MULTISPEED:
            case HeatObjTypes::COIL_DX_MULTIMODE: {
                state.dataHeatBal->HeatReclaimDXCoil(SourceID).HVACDesuperheaterReclaimedHeat(DesuperheaterNum) = HeatingCoilLoad;
                state.dataHeatBal->HeatReclaimDXCoil(SourceID).HVACDesuperheaterReclaimedHeatTotal = 0.0;
                for (auto const &num : state.dataHeatBal->HeatReclaimDXCoil(SourceID).HVACDesuperheaterReclaimedHeat) {
                    state.dataHeatBal->HeatReclaimDXCoil(SourceID).HVACDesuperheaterReclaimedHeatTotal += num;
                }
            } break;
            case HeatObjTypes::COIL_DX_VARIABLE_COOLING: {
                state.dataHeatBal->HeatReclaimVS_Coil(SourceID).HVACDesuperheaterReclaimedHeat(DesuperheaterNum) = HeatingCoilLoad;
                state.dataHeatBal->HeatReclaimVS_Coil(SourceID).HVACDesuperheaterReclaimedHeatTotal = 0.0;
                for (auto const &num : state.dataHeatBal->HeatReclaimVS_Coil(SourceID).HVACDesuperheaterReclaimedHeat) {
                    state.dataHeatBal->HeatReclaimVS_Coil(SourceID).HVACDesuperheaterReclaimedHeatTotal += num;
                }
            } break;
            default:
                break;
            }
        }
    }

    void UpdateHeatingCoil(EnergyPlusData &state, int const CoilNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the coil outlet nodes.

        // METHODOLOGY EMPLOYED:
        // Data is moved from the coil data structure to the coil outlet nodes.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto const &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
        auto const &airInletNode = state.dataLoopNodes->Node(heatingCoil.AirInletNodeNum);
        auto &airOuletNode = state.dataLoopNodes->Node(heatingCoil.AirOutletNodeNum);

        // Set the outlet air nodes of the HeatingCoil
        airOuletNode.MassFlowRate = heatingCoil.OutletAirMassFlowRate;
        airOuletNode.Temp = heatingCoil.OutletAirTemp;
        airOuletNode.HumRat = heatingCoil.OutletAirHumRat;
        airOuletNode.Enthalpy = heatingCoil.OutletAirEnthalpy;

        // Set the outlet nodes for properties that just pass through & not used
        airOuletNode.Quality = airInletNode.Quality;
        airOuletNode.Press = airInletNode.Press;
        airOuletNode.MassFlowRateMin = airInletNode.MassFlowRateMin;
        airOuletNode.MassFlowRateMax = airInletNode.MassFlowRateMax;
        airOuletNode.MassFlowRateMinAvail = airInletNode.MassFlowRateMinAvail;
        airOuletNode.MassFlowRateMaxAvail = airInletNode.MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airOuletNode.CO2 = airInletNode.CO2;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airOuletNode.GenContam = airInletNode.GenContam;
        }
    }

    void ReportHeatingCoil(EnergyPlusData &state, int const CoilNum, bool const coilIsSuppHeater)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variable for the coils.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);

        // report the HeatingCoil energy from this component
        heatingCoil.HeatingCoilRate = heatingCoil.HeatingCoilLoad;
        heatingCoil.HeatingCoilLoad *= TimeStepSysSec;

        heatingCoil.FuelUseRate = heatingCoil.FuelUseLoad;
        heatingCoil.ElecUseRate = heatingCoil.ElecUseLoad;
        if (coilIsSuppHeater) {
            state.dataHVACGlobal->SuppHeatingCoilPower = heatingCoil.ElecUseLoad;
        } else {
            state.dataHVACGlobal->ElecHeatingCoilPower = heatingCoil.ElecUseLoad;
        }
        heatingCoil.FuelUseLoad *= TimeStepSysSec;
        heatingCoil.ElecUseLoad *= TimeStepSysSec;

        heatingCoil.ParasiticFuelConsumption = heatingCoil.ParasiticFuelRate * TimeStepSysSec;

        if (heatingCoil.reportCoilFinalSizes) {
            if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingHVACSizingSimulations && !state.dataGlobal->DoingSizing) {
                ReportCoilSelection::setCoilFinalSizes(
                    state, heatingCoil.coilReportNum, heatingCoil.NominalCapacity, heatingCoil.NominalCapacity, -999.0, -999.0);
                heatingCoil.reportCoilFinalSizes = false;
            }
        }
    }

    //        End of Reporting subroutines for the HeatingCoil Module

    void GetCoilIndex(EnergyPlusData &state, std::string const &HeatingCoilName, int &HeatingCoilIndex, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   March 2005

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets an index for a given DX Coil -- issues error message if that
        // DX Coil is not a legal DX Coil.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HeatingCoilIndex = Util::FindItem(HeatingCoilName, state.dataHeatingCoils->HeatingCoil);
        if (HeatingCoilIndex == 0) {
            ShowSevereError(state, std::format("GetCoilIndex: Heating coil not found={}", HeatingCoilName));
            ErrorsFound = true;
        }
    }

    void CheckHeatingCoilSchedule(EnergyPlusData &state,
                                  std::string_view const CompType, // unused1208
                                  std::string const &CompName,
                                  Real64 &Value,
                                  int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2005

        // PURPOSE OF THIS SUBROUTINE:
        // This routine provides a method for outside routines to check if
        // the heating coil is scheduled to be on.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        // Find the correct Coil number
        if (CompIndex == 0) {
            int CoilNum = Util::FindItem(CompName, state.dataHeatingCoils->HeatingCoil);
            if (CoilNum == 0) {
                ShowFatalError(state, std::format("CheckHeatingCoilSchedule: Coil not found=\"{}\".", CompName));
            }
            if (!Util::SameString(CompType, HVAC::coilTypeNames[(int)state.dataHeatingCoils->HeatingCoil(CoilNum).coilType])) {
                ShowSevereError(state, std::format("CheckHeatingCoilSchedule: Coil=\"{}\"", CompName));
                ShowContinueError(state,
                                  std::format("...expected type=\"{}\", actual type=\"{}\".",
                                              CompType,
                                              HVAC::coilTypeNames[(int)state.dataHeatingCoils->HeatingCoil(CoilNum).coilType]));
                ShowFatalError(state, "Program terminates due to preceding conditions.");
            }
            CompIndex = CoilNum;
            Value = state.dataHeatingCoils->HeatingCoil(CoilNum).availSched->getCurrentVal(); // not scheduled?
        } else {
            int CoilNum = CompIndex;
            if (CoilNum > state.dataHeatingCoils->NumHeatingCoils || CoilNum < 1) {
                ShowFatalError(state,
                               std::format("CheckHeatingCoilSchedule: Invalid CompIndex passed={}, Number of Heating Coils={}, Coil name={}",
                                           CoilNum,
                                           state.dataHeatingCoils->NumHeatingCoils,
                                           CompName));
            }
            if (CompName != state.dataHeatingCoils->HeatingCoil(CoilNum).Name) {
                ShowSevereError(state,
                                std::format("CheckHeatingCoilSchedule: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                            CoilNum,
                                            CompName,
                                            state.dataHeatingCoils->HeatingCoil(CoilNum).Name));
                ShowContinueError(state,
                                  std::format("...expected type=\"{}\", actual type=\"{}\".",
                                              CompType,
                                              HVAC::coilTypeNames[(int)state.dataHeatingCoils->HeatingCoil(CoilNum).coilType]));
                ShowFatalError(state, "Program terminates due to preceding conditions.");
            }
            Value = state.dataHeatingCoils->HeatingCoil(CoilNum).availSched->getCurrentVal(); // not scheduled?
        }
    }

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string_view const CoilType, // must match coil types in this module
                           std::string const &CoilName,     // must match coil names for the coil type
                           bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil capacity for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Return value
        Real64 CoilCapacity = 0.0; // returned capacity of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingGasOrOtherFuel ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                CoilCapacity = state.dataHeatingCoils->HeatingCoil(WhichCoil).NominalCapacity;
            }
        } else if (FoundType == HVAC::CoilType::HeatingElectricMultiStage || FoundType == HVAC::CoilType::HeatingGasMultiStage) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                CoilCapacity =
                    state.dataHeatingCoils->HeatingCoil(WhichCoil).MSNominalCapacity(state.dataHeatingCoils->HeatingCoil(WhichCoil).NumOfStages);
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) { // Autodesk:Return Reworked block to assure CoilCapacity is set before return
            if (FoundType == HVAC::CoilType::Invalid) {
                ShowSevereError(state, std::format("GetCoilCapacity: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            } else {
                ShowSevereError(state, std::format("GetCoilCapacity: Invalid coil type for capacity, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
                ShowContinueError(state,
                                  std::format("...only {}, {} or {} are valid in this context.",
                                              HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingElectric],
                                              HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingGasOrOtherFuel],
                                              HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingDesuperheater]));
            }
            ShowContinueError(state, "... returning Coil Capacity as -1000.");
            ErrorsFound = true;
            CoilCapacity = -1000.0;
        }

        return CoilCapacity;
    }

    Sched::Schedule *GetCoilAvailSched(EnergyPlusData &state,
                                       std::string const &CoilType, // must match coil types in this module
                                       std::string const &CoilName, // must match coil names for the coil type
                                       bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   February 2013

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the availability schedule index.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int WhichCoil = 0;

        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                return state.dataHeatingCoils->HeatingCoil(WhichCoil).availSched;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("GetCoilAvailScheduleIndex: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
        }

        return nullptr;
    }

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string_view CoilType,   // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the inlet node number.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int WhichCoil = 0;
        int NodeNumber = 0;
        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                NodeNumber = state.dataHeatingCoils->HeatingCoil(WhichCoil).AirInletNodeNum;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("GetCoilInletNode: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the outlet node number.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int WhichCoil = 0;
        int NodeNumber = 0;
        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                NodeNumber = state.dataHeatingCoils->HeatingCoil(WhichCoil).AirOutletNodeNum;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("GetCoilOutletNode: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    int GetHeatReclaimSourceIndex(EnergyPlusData &state,
                                  std::string const &CoilType, // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   June 2007

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the heating coil index number if it is a desuperheating coil.
        // If incorrect coil type or name is given, ErrorsFound is returned as true and index number is returned
        // as zero.

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        bool GetCoilErrFlag;
        int NumCoil;
        int CoilNum(0);

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int CoilFound = 0;

        // note should eventually get rid of this string comparison
        if (Util::SameString(CoilType, "COIL:COOLING:DX:SINGLESPEED") || Util::SameString(CoilType, "COIL:COOLING:DX:TWOSPEED") ||
            Util::SameString(CoilType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
            bool SuppressWarning = true;
            DXCoils::GetDXCoilIndex(state, CoilName, CoilNum, GetCoilErrFlag, CoilType, SuppressWarning);
            for (NumCoil = 1; NumCoil <= state.dataHeatingCoils->NumHeatingCoils; ++NumCoil) {
                if (state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingSource != HeatObjTypes::COIL_DX_COOLING &&
                    state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingSource != HeatObjTypes::COIL_DX_MULTISPEED &&
                    state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingSource != HeatObjTypes::COIL_DX_MULTIMODE &&
                    state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingCoilName != CoilName) {
                    continue;
                }
                CoilFound = CoilNum;
                break;
            }
        } else if (Util::SameString(CoilType, "COIL:COOLING:DX:VARIABLESPEED")) {
            CoilNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(state, CoilType, CoilName, GetCoilErrFlag);
            for (NumCoil = 1; NumCoil <= state.dataHeatingCoils->NumHeatingCoils; ++NumCoil) {
                if (state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingSource != HeatObjTypes::COIL_DX_VARIABLE_COOLING &&
                    state.dataHeatingCoils->HeatingCoil(NumCoil).ReclaimHeatingCoilName != CoilName) {
                    continue;
                }
                CoilFound = CoilNum;
                break;
            }
        }

        if (CoilNum == 0) {
            ErrorsFound = true;
        }

        return CoilFound;
    }

    int GetCoilControlNodeNum(EnergyPlusData &state,
                              std::string const &CoilType, // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   June 2007

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the control node number.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            int WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                return state.dataHeatingCoils->HeatingCoil(WhichCoil).TempSetPointNodeNum;
            }
        }

        ShowSevereError(state, std::format("GetCoilControlNodeNum: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        return 0;
    }

    HVAC::CoilType GetHeatingCoilTypeNum(EnergyPlusData &state,
                                         std::string_view const CoilType, // must match coil types in this module
                                         std::string const &CoilName,     // must match coil names for the coil type
                                         bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2008

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the type number.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and type number is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            int WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                return state.dataHeatingCoils->HeatingCoil(WhichCoil).coilType;
            }
        }

        ShowSevereError(state, std::format("GetHeatingCoilTypeNum: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        return HVAC::CoilType::Invalid;
    }

    int GetHeatingCoilIndex(EnergyPlusData &state,
                            std::string const &CoilType, // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2011

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the index into the structure.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
        // as zero.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int WhichCoil = 0;
        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("GetHeatingCoilIndex: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
        }

        return WhichCoil;
    }

    int GetHeatingCoilPLFCurveIndex(EnergyPlusData &state,
                                    std::string const &CoilType, // must match coil types in this module
                                    std::string const &CoilName, // must match coil names for the coil type
                                    bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the PLF curve index.  If
        // incorrect coil name is given for gas or electric heating coils, ErrorsFound
        // is returned as true and curve index is returned as zero.
        // If not a gas or electric heating coil, ErrorsFound is unchanged and index is 0.

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HVAC::CoilType FoundType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Util::makeUPPER(CoilType)));
        if (FoundType == HVAC::CoilType::HeatingElectric || FoundType == HVAC::CoilType::HeatingElectricMultiStage ||
            FoundType == HVAC::CoilType::HeatingGasOrOtherFuel || FoundType == HVAC::CoilType::HeatingGasMultiStage ||
            FoundType == HVAC::CoilType::HeatingDesuperheater) {
            int WhichCoil = Util::FindItem(CoilName, state.dataHeatingCoils->HeatingCoil);
            if (WhichCoil != 0) {
                return state.dataHeatingCoils->HeatingCoil(WhichCoil).PLFCurveIndex;
            }
            ShowSevereError(state, std::format("GetHeatingCoilPLFCurveIndex: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            return 0;
        }
        return 0;
    }

    int GetHeatingCoilNumberOfStages(EnergyPlusData &state,
                                     std::string const &CoilType, // must match coil types in this module
                                     std::string const &CoilName, // must match coil names for the coil type
                                     bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   February 2013

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the number of speeds for multistage coils.
        // If incorrect coil type or name is given, ErrorsFound is returned as true.

        // Obtains and Allocates HeatingCoils
        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        int WhichCoil = Util::FindItemInList(CoilName, state.dataHeatingCoils->HeatingCoil);
        if (WhichCoil != 0) {
            return state.dataHeatingCoils->HeatingCoil(WhichCoil).NumOfStages;
        }
        ShowSevereError(state, std::format("GetHeatingCoilNumberOfSpeeds: Invalid Heating Coil Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        return 0;
    }

    void SetHeatingCoilData(EnergyPlusData &state,
                            int const CoilNum,                                  // Number of electric or gas heating Coil
                            bool &ErrorsFound,                                  // Set to true if certain errors found
                            ObjexxFCL::Optional_bool DesiccantRegenerationCoil, // Flag that this coil is used as regeneration air heating coil
                            ObjexxFCL::Optional_int DesiccantDehumIndex         // Index for the desiccant dehum system where this coil is used
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   February 2016

        // PURPOSE OF THIS FUNCTION:
        // This function sets data to Heating Coil using the coil index and arguments passed

        auto &heatingCoil = state.dataHeatingCoils->HeatingCoil(CoilNum);
        if (state.dataHeatingCoils->GetCoilsInputFlag) {
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        if (CoilNum <= 0 || CoilNum > state.dataHeatingCoils->NumHeatingCoils) {
            ShowSevereError(state,
                            std::format("SetHeatingCoilData: called with heating coil Number out of range={} should be >0 and <{}",
                                        CoilNum,
                                        state.dataHeatingCoils->NumHeatingCoils));
            ErrorsFound = true;
            return;
        }

        if (present(DesiccantRegenerationCoil)) {
            heatingCoil.DesiccantRegenerationCoil = DesiccantRegenerationCoil;
        }

        if (present(DesiccantDehumIndex)) {
            heatingCoil.DesiccantDehumNum = DesiccantDehumIndex;
        }
    }

    void SetHeatingCoilAirLoopNumber(EnergyPlusData &state, std::string const &HeatingCoilName, int AirLoopNum, bool &ErrorsFound)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         L.Gu
        //       DATE WRITTEN   March 2018

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets an AirLoopNum for a given heating Coil

        int HeatingCoilIndex;

        if (state.dataHeatingCoils->GetCoilsInputFlag) { // First time subroutine has been entered
            GetHeatingCoilInput(state);
            state.dataHeatingCoils->GetCoilsInputFlag = false;
        }

        HeatingCoilIndex = Util::FindItem(HeatingCoilName, state.dataHeatingCoils->HeatingCoil);
        if (HeatingCoilIndex == 0) {
            ShowSevereError(state, std::format("GetCoilIndex: Heating coil not found={}", HeatingCoilName));
            ErrorsFound = true;
        } else {
            state.dataHeatingCoils->HeatingCoil(HeatingCoilIndex).AirLoopNum = AirLoopNum;
        }
    }
} // namespace HeatingCoils

} // namespace EnergyPlus
