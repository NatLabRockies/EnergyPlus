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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SteamCoils {

    // Module containing the SteamCoil simulation routines

    // MODULE INFORMATION:
    //   AUTHOR         Rahul Chillar
    //   DATE WRITTEN   Jan 2005
    //   MODIFIED       na
    //   RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the SteamCoil System Component.

    using namespace Psychrometrics;

    using PlantUtilities::MyPlantSizingIndex;
    using PlantUtilities::ScanPlantLoopsForObject;

    constexpr std::array<std::string_view, static_cast<int>(CoilControlType::Num)> coilControlTypeNames = {"TEMPERATURESETPOINTCONTROL",
                                                                                                           "ZONELOADCONTROL"};

    void SimulateSteamCoilComponents(EnergyPlusData &state,
                                     std::string_view CompName,
                                     bool const FirstHVACIteration,
                                     int &CompIndex,
                                     ObjexxFCL::Optional<Real64 const> QCoilReq, // coil load to be met
                                     ObjexxFCL::Optional<Real64> QCoilActual,    // coil load actually delivered returned to calling component
                                     ObjexxFCL::Optional<HVAC::FanOp const> fanOpMode,
                                     ObjexxFCL::Optional<Real64 const> PartLoadRatio)
    {

        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       na
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages SteamCoil component simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QCoilActualTemp; // coil load actually delivered returned to calling component
        int CoilNum;            // The SteamCoil that you are currently loading input into
        HVAC::FanOp fanOp;      // fan operating mode
        Real64 PartLoadFrac;    // part-load fraction of heating coil
        Real64 QCoilReqLocal;   // local required heating load optional

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        // Find the correct SteamCoilNumber with the Coil Name
        if (CompIndex == 0) {
            CoilNum = Util::FindItemInList(CompName, state.dataSteamCoils->SteamCoil);
            if (CoilNum == 0) {
                ShowFatalError(state, EnergyPlus::format("SimulateSteamCoilComponents: Coil not found={}", CompName));
            }
            CompIndex = CoilNum;
        } else {
            CoilNum = CompIndex;
            if (CoilNum > state.dataSteamCoils->NumSteamCoils || CoilNum < 1) {
                ShowFatalError(state,
                               EnergyPlus::format("SimulateSteamCoilComponents: Invalid CompIndex passed={}, Number of Steam Coils={}, Coil name={}",
                                                  CoilNum,
                                                  state.dataSteamCoils->NumSteamCoils,
                                                  CompName));
            }
            if (state.dataSteamCoils->CheckEquipName(CoilNum)) {
                if (CompName != state.dataSteamCoils->SteamCoil(CoilNum).Name) {
                    ShowFatalError(state,
                                   EnergyPlus::format(
                                       "SimulateSteamCoilComponents: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                       CoilNum,
                                       CompName,
                                       state.dataSteamCoils->SteamCoil(CoilNum).Name));
                }
                state.dataSteamCoils->CheckEquipName(CoilNum) = false;
            }
        }

        // With the correct CoilNum Initialize
        InitSteamCoil(state, CoilNum, FirstHVACIteration); // Initialize all SteamCoil related parameters

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
            QCoilReqLocal = QCoilReq;
        } else {
            QCoilReqLocal = 0.0;
        }

        CalcSteamAirCoil(
            state, CoilNum, QCoilReqLocal, QCoilActualTemp, fanOp, PartLoadFrac); // Autodesk:OPTIONAL QCoilReq used without PRESENT check
        if (present(QCoilActual)) {
            QCoilActual = QCoilActualTemp;
        }

        // Update the current SteamCoil to the outlet nodes
        UpdateSteamCoil(state, CoilNum);

        // Report the current SteamCoil
        ReportSteamCoil(state, CoilNum);
    }

    // Get Input Section of the Module

    void GetSteamCoilInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       na
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for coils and stores it in coil data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // Using/Aliasing
        using GlobalNames::VerifyUniqueCoilName;
        using Node::GetOnlySingleNode;
        using Node::TestCompSet;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetSteamCoilInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetSteamCoilInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CoilNum; // The SteamCoil that you are currently loading input into
        int NumStmHeat;
        int StmHeatNum;
        int NumAlphas;
        int NumNums;
        int IOStat;
        bool ErrorsFound(false);         // If errors detected in input
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string AlphArray;        // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> NumArray;        // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        int TotalArgs(0);                // Total number of alpha and numeric arguments (max) for a
                                         //  certain object in the input file

        CurrentModuleObject = "Coil:Heating:Steam";
        NumStmHeat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataSteamCoils->NumSteamCoils = NumStmHeat;
        if (state.dataSteamCoils->NumSteamCoils > 0) {
            state.dataSteamCoils->SteamCoil.allocate(state.dataSteamCoils->NumSteamCoils);
            state.dataSteamCoils->CheckEquipName.dimension(state.dataSteamCoils->NumSteamCoils, true);
        }

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        AlphArray.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNums);
        NumArray.dimension(NumNums, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNums, true);

        // Get the data for steam heating coils
        for (StmHeatNum = 1; StmHeatNum <= NumStmHeat; ++StmHeatNum) {

            CoilNum = StmHeatNum;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     StmHeatNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

            auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
            steamCoil.Name = AlphArray(1);
            steamCoil.coilType = HVAC::CoilType::HeatingSteam;
            steamCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, steamCoil.Name, steamCoil.coilType);

            if (lAlphaBlanks(2)) {
                steamCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((steamCoil.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
                ErrorsFound = true;
            }

            steamCoil.CoilType = DataPlant::PlantEquipmentType::CoilSteamAirHeating;
            steamCoil.MaxSteamVolFlowRate = NumArray(1);
            steamCoil.DegOfSubcooling = NumArray(2);
            steamCoil.LoopSubcoolReturn = NumArray(3);

            steamCoil.SteamInletNodeNum = GetOnlySingleNode(state,
                                                            AlphArray(3),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilHeatingSteam,
                                                            AlphArray(1),
                                                            Node::FluidType::Steam,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Secondary,
                                                            Node::ObjectIsNotParent);

            steamCoil.SteamOutletNodeNum = GetOnlySingleNode(state,
                                                             AlphArray(4),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilHeatingSteam,
                                                             AlphArray(1),
                                                             Node::FluidType::Steam,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Secondary,
                                                             Node::ObjectIsNotParent);

            steamCoil.AirInletNodeNum = GetOnlySingleNode(state,
                                                          AlphArray(5),
                                                          ErrorsFound,
                                                          Node::ConnectionObjectType::CoilHeatingSteam,
                                                          AlphArray(1),
                                                          Node::FluidType::Air,
                                                          Node::ConnectionType::Inlet,
                                                          Node::CompFluidStream::Primary,
                                                          Node::ObjectIsNotParent);

            steamCoil.AirOutletNodeNum = GetOnlySingleNode(state,
                                                           AlphArray(6),
                                                           ErrorsFound,
                                                           Node::ConnectionObjectType::CoilHeatingSteam,
                                                           AlphArray(1),
                                                           Node::FluidType::Air,
                                                           Node::ConnectionType::Outlet,
                                                           Node::CompFluidStream::Primary,
                                                           Node::ObjectIsNotParent);

            std::string controlMode = Util::makeUPPER(AlphArray(7));
            steamCoil.TypeOfCoil = static_cast<CoilControlType>(getEnumValue(coilControlTypeNames, controlMode));

            switch (steamCoil.TypeOfCoil) {
            case CoilControlType::TemperatureSetPoint:
                steamCoil.TempSetPointNodeNum = GetOnlySingleNode(state,
                                                                  AlphArray(8),
                                                                  ErrorsFound,
                                                                  Node::ConnectionObjectType::CoilHeatingSteam,
                                                                  AlphArray(1),
                                                                  Node::FluidType::Air,
                                                                  Node::ConnectionType::Sensor,
                                                                  Node::CompFluidStream::Primary,
                                                                  Node::ObjectIsNotParent);
                if (steamCoil.TempSetPointNodeNum == 0) {
                    ShowSevereError(
                        state, EnergyPlus::format("{}{} not found for {} = {}", RoutineName, cAlphaFields(8), CurrentModuleObject, AlphArray(1)));
                    ShowContinueError(state, "..required for Temperature Setpoint Controlled Coils.");
                    ErrorsFound = true;
                }
                break;
            case CoilControlType::ZoneLoadControl:
                if (!lAlphaBlanks(8)) {
                    ShowWarningError(state, EnergyPlus::format("{}ZoneLoad Controlled Coil, so {} not needed", RoutineName, cAlphaFields(8)));
                    ShowContinueError(state, EnergyPlus::format("for {} = {}", CurrentModuleObject, AlphArray(1)));
                    steamCoil.TempSetPointNodeNum = 0;
                }
                break;
            default:
                ShowSevereError(
                    state,
                    EnergyPlus::format(
                        "{}Invalid {} [{}] specified for {} = {}", RoutineName, cAlphaFields(7), AlphArray(7), CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }

            TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(3), AlphArray(4), "Steam Nodes");
            TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(5), AlphArray(6), "Air Nodes");

            steamCoil.steam = Fluid::GetSteam(state);
            if (steamCoil.steam == nullptr && CoilNum == 1) {
                ShowSevereError(state, EnergyPlus::format("{}Steam Properties for {} not found.", RoutineName, AlphArray(1)));
                ShowContinueError(state, "Steam Fluid Properties should have been included in the input file.");
                ErrorsFound = true;
            }
        }

        for (CoilNum = 1; CoilNum <= NumStmHeat; ++CoilNum) {

            auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
            // Setup the Simple Heating Coil reporting variables
            // CurrentModuleObject = "Coil:Heating:Steam"
            SetupOutputVariable(state,
                                "Heating Coil Heating Energy",
                                Constant::Units::J,
                                steamCoil.TotSteamHeatingCoilEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                steamCoil.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Heating Coil Heating Rate",
                                Constant::Units::W,
                                steamCoil.TotSteamHeatingCoilRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                steamCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Steam Mass Flow Rate",
                                Constant::Units::kg_s,
                                steamCoil.OutletSteamMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                steamCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Steam Inlet Temperature",
                                Constant::Units::C,
                                steamCoil.InletSteamTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                steamCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Steam Outlet Temperature",
                                Constant::Units::C,
                                steamCoil.OutletSteamTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                steamCoil.Name);
            SetupOutputVariable(state,
                                "Heating Coil Steam Trap Loss Rate",
                                Constant::Units::W,
                                steamCoil.LoopLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                steamCoil.Name);
        }

        if (ErrorsFound) {
            ShowFatalError(state, EnergyPlus::format("{}Errors found in getting input.", RoutineName));
        }

        AlphArray.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        NumArray.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    // End of Get Input subroutines for the HB Module

    // Beginning Initialization Section of the Module

    void InitSteamCoil(EnergyPlusData &state, int const CoilNum, bool const FirstHVACIteration)
    {
        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       na
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the SteamCoil Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:
        // na

        // Using/Aliasing
        using PlantUtilities::InitComponentNodes;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitSteamCoil");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirInletNode;
        int SteamInletNode;
        int ControlNode;
        int AirOutletNode;
        Real64 SteamDensity;
        Real64 StartEnthSteam;

        if (state.dataSteamCoils->MyOneTimeFlag) {
            // initialize the environment and sizing flags
            state.dataSteamCoils->MyEnvrnFlag.dimension(state.dataSteamCoils->NumSteamCoils, true);
            state.dataSteamCoils->MySizeFlag.dimension(state.dataSteamCoils->NumSteamCoils, true);
            state.dataSteamCoils->CoilWarningOnceFlag.dimension(state.dataSteamCoils->NumSteamCoils, true);
            state.dataSteamCoils->MyPlantScanFlag.dimension(state.dataSteamCoils->NumSteamCoils, true);
            state.dataSteamCoils->MyOneTimeFlag = false;
        }

        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
        if (state.dataSteamCoils->MyPlantScanFlag(CoilNum) && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            ScanPlantLoopsForObject(state, steamCoil.Name, steamCoil.CoilType, steamCoil.plantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitSteamCoil: Program terminated for previous conditions.");
            }
            state.dataSteamCoils->MyPlantScanFlag(CoilNum) = false;
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataSteamCoils->MySizeFlag(CoilNum)) {
            // for each coil, do the sizing once.
            SizeSteamCoil(state, CoilNum);
            state.dataSteamCoils->MySizeFlag(CoilNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataSteamCoils->MyEnvrnFlag(CoilNum)) {
            // Initialize all report variables to a known state at beginning of simulation
            steamCoil.TotSteamHeatingCoilEnergy = 0.0;
            steamCoil.TotSteamCoolingCoilEnergy = 0.0;
            steamCoil.SenSteamCoolingCoilEnergy = 0.0;
            steamCoil.TotSteamHeatingCoilRate = 0.0;
            steamCoil.TotSteamCoolingCoilRate = 0.0;
            steamCoil.SenSteamCoolingCoilRate = 0.0;
            // Initialize other module level variables
            steamCoil.InletAirMassFlowRate = 0.0;
            steamCoil.OutletAirMassFlowRate = 0.0;
            steamCoil.InletAirTemp = 0.0;
            steamCoil.OutletAirTemp = 0.0;
            steamCoil.InletAirHumRat = 0.0;
            steamCoil.OutletAirHumRat = 0.0;
            steamCoil.InletAirEnthalpy = 0.0;
            steamCoil.OutletAirEnthalpy = 0.0;
            steamCoil.TotSteamCoilLoad = 0.0;
            steamCoil.SenSteamCoilLoad = 0.0;
            steamCoil.LoopLoss = 0.0;
            steamCoil.LeavingRelHum = 0.0;
            steamCoil.DesiredOutletTemp = 0.0;
            steamCoil.DesiredOutletHumRat = 0.0;
            steamCoil.InletSteamTemp = 0.0;
            steamCoil.OutletSteamTemp = 0.0;
            steamCoil.InletSteamMassFlowRate = 0.0;
            steamCoil.OutletSteamMassFlowRate = 0.0;
            steamCoil.InletSteamEnthalpy = 0.0;
            steamCoil.OutletWaterEnthalpy = 0.0;
            steamCoil.InletSteamPress = 0.0;
            steamCoil.InletSteamQuality = 0.0;
            steamCoil.OutletSteamQuality = 0.0;

            // More Environment initializations
            SteamInletNode = steamCoil.SteamInletNodeNum;

            state.dataLoopNodes->Node(SteamInletNode).Temp = 100.0;
            state.dataLoopNodes->Node(SteamInletNode).Press = 101325.0;
            auto *steam = Fluid::GetSteam(state);
            SteamDensity = steam->getSatDensity(state, state.dataLoopNodes->Node(SteamInletNode).Temp, 1.0, RoutineName);
            StartEnthSteam = steam->getSatEnthalpy(state, state.dataLoopNodes->Node(SteamInletNode).Temp, 1.0, RoutineName);
            state.dataLoopNodes->Node(SteamInletNode).Enthalpy = StartEnthSteam;
            state.dataLoopNodes->Node(SteamInletNode).Quality = 1.0;
            state.dataLoopNodes->Node(SteamInletNode).HumRat = 0.0;
            steamCoil.MaxSteamMassFlowRate = SteamDensity * steamCoil.MaxSteamVolFlowRate;
            //     Node(SteamInletNode)%MassFlowRate         = SteamCoil(CoilNum)%MaxSteamMassFlowRate
            //     Node(SteamInletNode)%MassFlowRateMinAvail = 0.0
            //     Node(SteamInletNode)%MassFlowRateMaxAvail = SteamCoil(CoilNum)%MaxSteamMassFlowRate
            InitComponentNodes(state, 0.0, steamCoil.MaxSteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum);
            state.dataSteamCoils->MyEnvrnFlag(CoilNum) = false;
        } // End If for the Begin Environment initializations

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataSteamCoils->MyEnvrnFlag(CoilNum) = true;
        }

        // Do the Begin Day initializations
        // NONE

        // Do the begin HVAC time step initializations
        // NONE

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.

        AirInletNode = steamCoil.AirInletNodeNum;
        SteamInletNode = steamCoil.SteamInletNodeNum;
        ControlNode = steamCoil.TempSetPointNodeNum;
        AirOutletNode = steamCoil.AirOutletNodeNum;

        // First set the conditions for the air into the coil model

        // If a temperature setpoint controlled coil must set the desired outlet temp every time
        if (ControlNode == 0) {
            steamCoil.DesiredOutletTemp = 0.0;
        } else if (ControlNode == AirOutletNode) {
            steamCoil.DesiredOutletTemp = state.dataLoopNodes->Node(ControlNode).TempSetPoint;
        } else {
            steamCoil.DesiredOutletTemp = state.dataLoopNodes->Node(ControlNode).TempSetPoint -
                                          (state.dataLoopNodes->Node(ControlNode).Temp - state.dataLoopNodes->Node(AirOutletNode).Temp);
        }

        steamCoil.InletAirMassFlowRate = state.dataLoopNodes->Node(AirInletNode).MassFlowRate;
        steamCoil.InletAirTemp = state.dataLoopNodes->Node(AirInletNode).Temp;
        steamCoil.InletAirHumRat = state.dataLoopNodes->Node(AirInletNode).HumRat;
        steamCoil.InletAirEnthalpy = state.dataLoopNodes->Node(AirInletNode).Enthalpy;
        if (FirstHVACIteration) {
            steamCoil.InletSteamMassFlowRate = steamCoil.MaxSteamMassFlowRate;
        } else {
            steamCoil.InletSteamMassFlowRate = state.dataLoopNodes->Node(SteamInletNode).MassFlowRate;
        }
        steamCoil.InletSteamTemp = state.dataLoopNodes->Node(SteamInletNode).Temp;
        steamCoil.InletSteamEnthalpy = state.dataLoopNodes->Node(SteamInletNode).Enthalpy;
        steamCoil.InletSteamPress = state.dataLoopNodes->Node(SteamInletNode).Press;
        steamCoil.InletSteamQuality = state.dataLoopNodes->Node(SteamInletNode).Quality;
        steamCoil.TotSteamHeatingCoilRate = 0.0;
        steamCoil.TotSteamCoolingCoilRate = 0.0;
        steamCoil.SenSteamCoolingCoilRate = 0.0;
        //   Node(SteamInletNode)%MassFlowRateMaxAvail = MIN(Node(SteamInletNode)%MassFlowRateMaxAvail,&
        //                                                   SteamCoil(CoilNum)%MaxSteamMassFlowRate)
    }

    void SizeSteamCoil(EnergyPlusData &state, int const CoilNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Jan 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Steam Coil Components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone or system sizing arrays and plant sizing data.

        // Using/Aliasing
        using namespace DataSizing;
        using PlantUtilities::RegisterPlantCompDesignFlow;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeSteamCoil");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PltSizNum;      // do loop index for plant sizing
        int PltSizSteamNum; // index of plant sizing object for 1st steam loop
        bool ErrorsFound;   // If errors detected in input
        Real64 CoilInTemp;
        Real64 CoilOutTemp;
        Real64 CoilOutHumRat;
        Real64 CoilInHumRat;
        Real64 DesCoilLoad;
        Real64 DesMassFlow;
        Real64 DesVolFlow;
        Real64 MinFlowFrac;
        Real64 OutAirFrac;
        Real64 TempSteamIn(100.0);
        Real64 EnthSteamInDry;
        Real64 EnthSteamOutWet;
        Real64 LatentHeatSteam;
        Real64 SteamDensity;
        Real64 RhoAirStd; // density of air at standard conditions
        Real64 CpAirStd;  // specific heat of air at std conditions
        Real64 CpWater;   // specific heat of water (condensed steam)
        Real64 TempSize;  // autosized value

        ErrorsFound = false;
        PltSizSteamNum = 0;
        PltSizNum = 0;
        CoilInTemp = 0.0;
        CoilInHumRat = 0.0;
        CoilOutTemp = 0.0;
        DesCoilLoad = 0.0;
        MinFlowFrac = 0.0;
        DesMassFlow = 0.0;
        DesVolFlow = 0.0;
        CpWater = 0.0;
        RhoAirStd = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, 20.0, 0.0);
        CpAirStd = PsyCpAirFnW(0.0);
        bool coilWasAutosized(false); // coil report

        auto &TermUnitSizing = state.dataSize->TermUnitSizing;

        // If this is a steam coil
        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
        // Find the appropriate steam Plant Sizing object
        if (steamCoil.MaxSteamVolFlowRate == AutoSize) {
            coilWasAutosized = true; // coil report
            PltSizSteamNum = MyPlantSizingIndex(
                state, "steam heating coil", steamCoil.Name, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, ErrorsFound);
        }

        if (PltSizSteamNum > 0) {
            // If this is a central air system heating coil
            if (state.dataSize->CurSysNum > 0) {
                auto &finalSysSizing = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum);

                // If the coil water volume flow rate needs autosizing, then do it
                if (steamCoil.MaxSteamVolFlowRate == AutoSize) {
                    CheckSysSizing(state, "Coil:Heating:Steam", steamCoil.Name);

                    std::string CompName; // component name
                    std::string CompType; // component type
                    bool bPRINT = false;  // TRUE if sizing is reported to output (eio)
                    if (steamCoil.DesiccantRegenerationCoil) {
                        state.dataSize->DataDesicRegCoil = true;
                        state.dataSize->DataDesicDehumNum = steamCoil.DesiccantDehumNum;
                        CompType = HVAC::coilTypeNames[(int)steamCoil.coilType];
                        CompName = steamCoil.Name;
                        bPRINT = false;
                        HeatingCoilDesAirInletTempSizer sizerHeatingDesInletTemp;
                        bool localErrorsFound = false;
                        sizerHeatingDesInletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                        state.dataSize->DataDesInletAirTemp = sizerHeatingDesInletTemp.size(state, DataSizing::AutoSize, localErrorsFound);

                        HeatingCoilDesAirOutletTempSizer sizerHeatingDesOutletTemp;
                        localErrorsFound = false;
                        sizerHeatingDesOutletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                        state.dataSize->DataDesOutletAirTemp = sizerHeatingDesOutletTemp.size(state, DataSizing::AutoSize, localErrorsFound);

                        if (state.dataSize->CurOASysNum > 0) {
                            auto &OASysEqSizing = state.dataSize->OASysEqSizing;
                            OASysEqSizing(state.dataSize->CurOASysNum).AirFlow = true;
                            OASysEqSizing(state.dataSize->CurOASysNum).AirVolFlow = finalSysSizing.DesOutAirVolFlow;
                        }
                        TempSize = AutoSize; // reset back
                    }

                    // Set the duct flow rate
                    switch (state.dataSize->CurDuctType) {
                    case HVAC::AirDuctType::Main:
                        DesVolFlow = finalSysSizing.SysAirMinFlowRat * finalSysSizing.DesMainVolFlow;
                        break;
                    case HVAC::AirDuctType::Cooling:
                        DesVolFlow = finalSysSizing.SysAirMinFlowRat * finalSysSizing.DesCoolVolFlow;
                        break;
                    case HVAC::AirDuctType::Heating:
                        DesVolFlow = finalSysSizing.DesHeatVolFlow;
                        break;
                    case HVAC::AirDuctType::Other:
                        DesVolFlow = finalSysSizing.DesMainVolFlow;
                        break;
                    default:
                        DesVolFlow = finalSysSizing.DesMainVolFlow;
                    }
                    if (state.dataSize->DataDesicRegCoil) {
                        bPRINT = false;
                        TempSize = AutoSize;
                        bool errorsFound = false;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        std::string SizingString; // input field sizing description (e.g., Nominal Capacity)
                        sizingHeatingAirFlow.overrideSizingString(SizingString);
                        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                        DesVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                    }
                    DesMassFlow = RhoAirStd * DesVolFlow;
                    // get the outside air fraction
                    if (finalSysSizing.HeatOAOption == DataSizing::OAControl::MinOA) {
                        if (DesVolFlow > 0.0) {
                            OutAirFrac = finalSysSizing.DesOutAirVolFlow / DesVolFlow;
                        } else {
                            OutAirFrac = 1.0;
                        }
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    } else {
                        OutAirFrac = 1.0;
                    }

                    if (state.dataSize->DataDesicRegCoil) {
                        DesCoilLoad = CpAirStd * DesMassFlow * (state.dataSize->DataDesOutletAirTemp - state.dataSize->DataDesInletAirTemp);
                    } else {
                        // mixed air temp
                        CoilInTemp = OutAirFrac * finalSysSizing.HeatOutTemp + (1.0 - OutAirFrac) * finalSysSizing.HeatRetTemp;
                        // coil load
                        DesCoilLoad = CpAirStd * DesMassFlow * (finalSysSizing.HeatSupTemp - CoilInTemp);
                    }
                    // AUTOSTEAMCOIL
                    if (DesCoilLoad >= HVAC::SmallLoad) {
                        // TempSteamIn=SteamCoil(CoilNum)%InletSteamTemp
                        // TempSteamIn=PlantSizData(PltSizSteamNum)%ExitTemp
                        TempSteamIn = 100.0; // Should be from the PlantSizing object (ExitTemp) instead of hardwired to 100?
                        // RefrigIndex is set during GetInput for this module
                        EnthSteamInDry = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                        EnthSteamOutWet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                        LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                        SteamDensity = steamCoil.steam->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                        // SteamCoil(CoilNum)%MaxSteamVolFlowRate = DesCoilLoad/(SteamDensity * LatentHeatSteam)
                        //            CpWater  =  GetSpecificHeatGlycol('WATER',  &
                        //                                              TempSteamIn, &
                        //                                              PlantLoop(SteamCoil(CoilNum)%LoopNum)%FluidIndex, &
                        //                                             'SizeSteamCoil')
                        CpWater = steamCoil.steam->getSatSpecificHeat(state, TempSteamIn, 0.0, RoutineName);

                        steamCoil.MaxSteamVolFlowRate = DesCoilLoad / (SteamDensity * (LatentHeatSteam + steamCoil.DegOfSubcooling * CpWater));
                        //             PlantSizData(PltSizSteamNum)%DeltaT*CPHW(PlantSizData(PltSizSteamNum)%ExitTemp)))
                    } else {
                        steamCoil.MaxSteamVolFlowRate = 0.0;
                        ShowWarningError(state, EnergyPlus::format("The design coil load is zero for COIL:Heating:Steam {}", steamCoil.Name));
                    }
                    BaseSizer::reportSizerOutput(
                        state, "Coil:Heating:Steam", steamCoil.Name, "Maximum Steam Flow Rate [m3/s]", steamCoil.MaxSteamVolFlowRate);
                }
                state.dataSize->DataDesicRegCoil = false; // reset all globals to 0 to ensure correct sizing for other child components
                // Coil report, set fan info for airloopnum

                if (state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum > 0) {
                    ReportCoilSelection::setCoilSupplyFanInfo(
                        state,
                        steamCoil.coilReportNum,
                        state.dataFans->fans(state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum)->Name,
                        state.dataFans->fans(state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum)->type,
                        state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum);
                }

                // if this is a zone coil
            } else if (state.dataSize->CurZoneEqNum > 0) {
                CheckZoneSizing(state, "Coil:Heating:Steam", steamCoil.Name);
                // autosize the coil steam volume flow rate if needed
                if (steamCoil.MaxSteamVolFlowRate == AutoSize) {
                    // if coil is part of a terminal unit just use the terminal unit value
                    if (state.dataSize->TermUnitSingDuct || state.dataSize->TermUnitPIU || state.dataSize->TermUnitIU) {
                        if (state.dataSize->CurTermUnitSizingNum > 0) {
                            steamCoil.MaxSteamVolFlowRate = TermUnitSizing(state.dataSize->CurTermUnitSizingNum).MaxSTVolFlow;
                        } else {
                            steamCoil.MaxSteamVolFlowRate = 0.0;
                        }
                        // if coil is part of a zonal unit, calc coil load to get hot Steam flow rate
                        DesCoilLoad = TermUnitSizing(state.dataSize->CurTermUnitSizingNum).DesHeatingLoad; // coil report
                        DesVolFlow = TermUnitSizing(state.dataSize->CurTermUnitSizingNum).AirVolFlow *
                                     TermUnitSizing(state.dataSize->CurTermUnitSizingNum).ReheatAirFlowMult; // coil report
                    } else {
                        CoilInTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatCoilInTemp;
                        CoilOutTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).HeatDesTemp;
                        CoilOutHumRat = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).HeatDesHumRat;
                        DesMassFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatMassFlow;
                        DesVolFlow = DesMassFlow / RhoAirStd;
                        DesCoilLoad = PsyCpAirFnW(CoilOutHumRat) * DesMassFlow * (CoilOutTemp - CoilInTemp);
                        if (DesCoilLoad >= HVAC::SmallLoad) {
                            TempSteamIn = 100.0;
                            // RefrigIndex is set during GetInput for this module
                            EnthSteamInDry = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                            EnthSteamOutWet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                            LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                            SteamDensity = steamCoil.steam->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                            // SteamCoil(CoilNum)%MaxSteamVolFlowRate = DesCoilLoad/(SteamDensity * LatentHeatSteam)
                            //           CpWater  =  GetSpecificHeatGlycol('WATER',  &
                            //                                             TempSteamIn, &
                            //                                             PlantLoop(SteamCoil(CoilNum)%LoopNum)%FluidIndex, &
                            //                                            'SizeSteamCoil')
                            CpWater = steamCoil.steam->getSatSpecificHeat(state, TempSteamIn, 0.0, RoutineName);

                            steamCoil.MaxSteamVolFlowRate = DesCoilLoad / (SteamDensity * (LatentHeatSteam + steamCoil.DegOfSubcooling * CpWater));
                            //             PlantSizData(PltSizSteamNum)%DeltaT*CPHW(PlantSizData(PltSizSteamNum)%ExitTemp)))
                        } else {
                            steamCoil.MaxSteamVolFlowRate = 0.0;
                        }
                    }
                    // issue warning if hw coil has zero flow
                    if (steamCoil.MaxSteamVolFlowRate == 0.0) {
                        ShowWarningError(state, EnergyPlus::format("The design coil load is zero for COIL:Heating:Steam {}", steamCoil.Name));
                        ShowContinueError(state, "The autosize value for max Steam flow rate is zero");
                    }
                    BaseSizer::reportSizerOutput(
                        state, "Coil:Heating:Steam", steamCoil.Name, "Maximum Steam Flow Rate [m3/s]", steamCoil.MaxSteamVolFlowRate);
                }
            } // end zone coil ELSE - IF

        } else {
            // if there is no heating Plant Sizing object and autosizng was requested, issue an error message
            if (steamCoil.MaxSteamVolFlowRate == AutoSize) {
                ShowSevereError(state, "Autosizing of Steam coil requires a heating loop Sizing:Plant object");
                ShowContinueError(state, EnergyPlus::format("Occurs in Steam coil object= {}", steamCoil.Name));
                ErrorsFound = true;
            }
        } // end of heating Plant Sizing existence IF - ELSE

        // save the design Steam volumetric flow rate for use by the Steam loop sizing algorithms
        RegisterPlantCompDesignFlow(state, steamCoil.SteamInletNodeNum, steamCoil.MaxSteamVolFlowRate);

        ReportCoilSelection::setCoilHeatingCapacity(state,
                                                    steamCoil.coilReportNum,
                                                    DesCoilLoad,
                                                    coilWasAutosized,
                                                    state.dataSize->CurSysNum,
                                                    state.dataSize->CurZoneEqNum,
                                                    state.dataSize->CurOASysNum,
                                                    0.0,
                                                    1.0,
                                                    -999.0,
                                                    -999.0);
        ReportCoilSelection::setCoilWaterFlowNodeNums(state,
                                                      steamCoil.coilReportNum,
                                                      steamCoil.MaxSteamVolFlowRate,
                                                      coilWasAutosized,
                                                      steamCoil.SteamInletNodeNum,
                                                      steamCoil.SteamOutletNodeNum,
                                                      steamCoil.plantLoc.loopNum);
        ReportCoilSelection::setCoilWaterHeaterCapacityNodeNums(state,
                                                                steamCoil.coilReportNum,
                                                                DesCoilLoad,
                                                                coilWasAutosized,
                                                                steamCoil.SteamInletNodeNum,
                                                                steamCoil.SteamOutletNodeNum,
                                                                steamCoil.plantLoc.loopNum);
        ReportCoilSelection::setCoilEntWaterTemp(state, steamCoil.coilReportNum, TempSteamIn);
        ReportCoilSelection::setCoilLvgWaterTemp(state, steamCoil.coilReportNum, TempSteamIn - steamCoil.DegOfSubcooling);
        ReportCoilSelection::setCoilWaterDeltaT(state, steamCoil.coilReportNum, steamCoil.DegOfSubcooling);

        steamCoil.DesCoilCapacity = DesCoilLoad;
        steamCoil.DesAirVolFlow = DesVolFlow;
        if (ErrorsFound) {
            ShowFatalError(state, "Preceding Steam coil sizing errors cause program termination");
        }

        // There is no standard rating for heating coils at this point, so fill with dummy flag values
        ReportCoilSelection::setRatedCoilConditions(
            state, steamCoil.coilReportNum, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0, -999.0);
    }

    // End Initialization Section of the Module

    // Begin Algorithm Section of the Module

    void CalcSteamAirCoil(EnergyPlusData &state,
                          int const CoilNum,
                          Real64 const QCoilRequested, // requested coil load
                          Real64 &QCoilActual,         // coil load actually delivered
                          HVAC::FanOp const fanOp,     // fan operating mode
                          Real64 const PartLoadRatio   // part-load ratio of heating coil
    )
    {
        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       Sep. 2012, B. Griffith, add calls to SetComponentFlowRate for plant interactions
        //                  Jul. 2016, R. Zhang, Applied the coil supply air temperature sensor offset fault model
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Simple Steam to air heat exchanger which,
        // serves as an interface for distributing heat from boiler to zones.

        // METHODOLOGY EMPLOYED:
        // Steam coils are different, All of steam condenses in heat exchanger
        // Steam traps allow only water to leave the coil,the degree of subcooling
        // desired is input by the user, which is used to calculate water outlet temp.
        // Heat exchange is = Latent Heat + Sensible heat,coil effectiveness is 1.0

        using HVAC::TempControlTol;
        using PlantUtilities::SetComponentFlowRate;

        static constexpr std::string_view RoutineName("CalcSteamAirCoil");
        static constexpr std::string_view RoutineNameSizeSteamCoil("SizeSteamCoil");

        Real64 SteamMassFlowRate(0.0);
        Real64 AirMassFlow(0.0); // [kg/sec]
        Real64 TempAirIn(0.0);   // [C]
        Real64 TempAirOut(0.0);  // [C]
        Real64 Win(0.0);
        Real64 TempSteamIn(0.0);
        Real64 TempWaterOut(0.0);
        Real64 CapacitanceAir(0.0);
        Real64 HeatingCoilLoad(0.0);
        Real64 CoilPress(0.0);
        Real64 EnthSteamInDry(0.0);
        Real64 EnthSteamOutWet(0.0);
        Real64 LatentHeatSteam(0.0);
        Real64 SubcoolDeltaTemp(0.0);
        Real64 TempSetPoint(0.0);
        Real64 QCoilReq(0.0);
        Real64 QCoilCap(0.0);
        Real64 QSteamCoilMaxHT(0.0);
        Real64 TempWaterAtmPress(0.0);
        Real64 TempLoopOutToPump(0.0);
        Real64 EnergyLossToEnvironment(0.0);
        Real64 EnthCoilOutlet(0.0);
        Real64 EnthPumpInlet(0.0);
        Real64 EnthAtAtmPress(0.0);
        Real64 CpWater(0.0);

        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);

        QCoilReq = QCoilRequested;
        TempAirIn = steamCoil.InletAirTemp;
        Win = steamCoil.InletAirHumRat;
        TempSteamIn = steamCoil.InletSteamTemp;
        CoilPress = steamCoil.InletSteamPress;
        SubcoolDeltaTemp = steamCoil.DegOfSubcooling;
        TempSetPoint = steamCoil.DesiredOutletTemp;

        // If there is a fault of coil SAT Sensor
        if (steamCoil.FaultyCoilSATFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            // calculate the sensor offset using fault information
            int FaultIndex = steamCoil.FaultyCoilSATIndex;
            steamCoil.FaultyCoilSATOffset = state.dataFaultsMgr->FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint -= steamCoil.FaultyCoilSATOffset;
        }

        //  adjust mass flow rates for cycling fan cycling coil operation
        if (fanOp == HVAC::FanOp::Cycling) {
            if (PartLoadRatio > 0.0) {
                AirMassFlow = steamCoil.InletAirMassFlowRate / PartLoadRatio;
                SteamMassFlowRate = min(steamCoil.InletSteamMassFlowRate / PartLoadRatio, steamCoil.MaxSteamMassFlowRate);
                QCoilReq /= PartLoadRatio;
            } else {
                AirMassFlow = 0.0;
                SteamMassFlowRate = 0.0;
            }
        } else {
            AirMassFlow = steamCoil.InletAirMassFlowRate;
            SteamMassFlowRate = steamCoil.InletSteamMassFlowRate;
        }

        if (AirMassFlow > 0.0) { // If the coil is operating
            CapacitanceAir = PsyCpAirFnW(Win) * AirMassFlow;
        } else {
            CapacitanceAir = 0.0;
        }

        // If the coil is operating there should be some heating capacitance
        //  across the coil, so do the simulation. If not set outlet to inlet and no load.
        //  Also the coil has to be scheduled to be available
        //  Control output to meet load QCoilReq. Load Controlled Coil.
        switch (steamCoil.TypeOfCoil) {

        case CoilControlType::ZoneLoadControl:
            if ((CapacitanceAir > 0.0) && ((steamCoil.InletSteamMassFlowRate) > 0.0) &&
                (steamCoil.availSched->getCurrentVal() > 0.0 || state.dataSteamCoils->MySizeFlag(CoilNum)) && (QCoilReq > 0.0)) {

                // Steam heat exchangers would not have effectiveness, since all of the steam is
                // converted to water and only then the steam trap allows it to leave the heat
                // exchanger, subsequently heat exchange is latent heat + subcooling.
                EnthSteamInDry = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                EnthSteamOutWet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);

                LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;

                //          CpWater = GetSpecificHeatGlycol('WATER',  &
                //                                           TempSteamIn, &
                //                                           PlantLoop(SteamCoil(CoilNum)%LoopNum)%FluidIndex, &
                //                                           'CalcSteamAirCoil')

                CpWater = steamCoil.steam->getSatSpecificHeat(state, TempSteamIn, 0.0, RoutineNameSizeSteamCoil);

                // Max Heat Transfer
                QSteamCoilMaxHT = steamCoil.MaxSteamMassFlowRate * (LatentHeatSteam + SubcoolDeltaTemp * CpWater);
                steamCoil.OperatingCapacity = QSteamCoilMaxHT;

                // Determine the Max coil capacity and check for the same.
                if (QCoilReq > QSteamCoilMaxHT) {
                    QCoilCap = QSteamCoilMaxHT;
                } else {
                    QCoilCap = QCoilReq;
                }

                // Steam Mass Flow Rate Required
                SteamMassFlowRate = QCoilCap / (LatentHeatSteam + SubcoolDeltaTemp * CpWater);

                SetComponentFlowRate(state, SteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, steamCoil.plantLoc);

                // recalculate if mass flow rate changed in previous call.
                QCoilCap = SteamMassFlowRate * (LatentHeatSteam + SubcoolDeltaTemp * CpWater);

                // In practice Sensible & Superheated heat transfer is negligible compared to latent part.
                // This is required for outlet water temperature, otherwise it will be saturation temperature.
                // Steam Trap drains off all the Water formed.
                // Here Degree of Subcooling is used to calculate hot water return temperature.

                // Calculating Water outlet temperature
                TempWaterOut = TempSteamIn - SubcoolDeltaTemp;

                // Total Heat Transfer to air
                HeatingCoilLoad = QCoilCap;

                // Temperature of air at outlet
                TempAirOut = TempAirIn + QCoilCap / (AirMassFlow * PsyCpAirFnW(Win));

                steamCoil.OutletSteamMassFlowRate = SteamMassFlowRate;
                steamCoil.InletSteamMassFlowRate = SteamMassFlowRate;

                //************************* Loop Losses *****************************
                // Loop pressure return considerations included in steam coil since the pipes are
                // perfect and do not account for losses.
                // Return water is condensate at atmoshperic pressure
                // Process is considered constant enthalpy expansion
                // No quality function in EnergyPlus hence no option left apart from
                // considering saturated state.
                //              StdBaroPress=101325

                TempWaterAtmPress = steamCoil.steam->getSatTemperature(state, state.dataEnvrn->StdBaroPress, RoutineName);

                // Point 4 at atm - loop delta subcool during return journery back to pump
                TempLoopOutToPump = TempWaterAtmPress - steamCoil.LoopSubcoolReturn;

                // Actual Steam Coil Outlet Enthalpy
                EnthCoilOutlet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName) - CpWater * SubcoolDeltaTemp;

                // Enthalpy at Point 4
                EnthAtAtmPress = steamCoil.steam->getSatEnthalpy(state, TempWaterAtmPress, 0.0, RoutineName);

                // Reported value of coil outlet enthalpy at the node to match the node outlet temperature
                CpWater = steamCoil.steam->getSatSpecificHeat(state, TempLoopOutToPump, 0.0, RoutineNameSizeSteamCoil);

                EnthPumpInlet = EnthAtAtmPress - CpWater * steamCoil.LoopSubcoolReturn;

                steamCoil.OutletWaterEnthalpy = EnthPumpInlet;

                // Point 3-Point 5,
                EnergyLossToEnvironment = SteamMassFlowRate * (EnthCoilOutlet - EnthPumpInlet);

                // Loss to environment due to pressure drop
                steamCoil.LoopLoss = EnergyLossToEnvironment;
                //************************* Loop Losses *****************************
            } else { // Coil is not running.

                TempAirOut = TempAirIn;
                TempWaterOut = TempSteamIn;
                HeatingCoilLoad = 0.0;
                steamCoil.OutletWaterEnthalpy = steamCoil.InletSteamEnthalpy;
                steamCoil.OutletSteamMassFlowRate = 0.0;
                steamCoil.OutletSteamQuality = 0.0;
                steamCoil.LoopLoss = 0.0;
                TempLoopOutToPump = TempWaterOut;
            }
            break;
        case CoilControlType::TemperatureSetPoint:
            // Control coil output to meet a Setpoint Temperature.
            if ((CapacitanceAir > 0.0) && ((steamCoil.InletSteamMassFlowRate) > 0.0) &&
                (steamCoil.availSched->getCurrentVal() > 0.0 || state.dataSteamCoils->MySizeFlag(CoilNum)) &&
                (std::abs(TempSetPoint - TempAirIn) > TempControlTol)) {

                // Steam heat exchangers would not have effectiveness, since all of the steam is
                // converted to water and only then the steam trap allows it to leave the heat
                // exchanger, subsequently heat exchange is latent heat + subcooling.
                EnthSteamInDry = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                EnthSteamOutWet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;

                //          CpWater = GetSpecificHeatGlycol('WATER',  &
                //                                           TempSteamIn, &
                //                                           PlantLoop(SteamCoil(CoilNum)%LoopNum)%FluidIndex, &
                //                                           'CalcSteamAirCoil')
                CpWater = steamCoil.steam->getSatSpecificHeat(state, TempSteamIn, 0.0, RoutineNameSizeSteamCoil);

                // Max Heat Transfer
                QSteamCoilMaxHT = steamCoil.MaxSteamMassFlowRate * (LatentHeatSteam + SubcoolDeltaTemp * CpWater);

                // Coil Load in case of temperature setpoint
                QCoilCap = CapacitanceAir * (TempSetPoint - TempAirIn);

                // Check to see if setpoint above entering temperature. If not, set
                // output to zero.
                if (QCoilCap <= 0.0) {
                    QCoilCap = 0.0;
                    TempAirOut = TempAirIn;

                    // Steam Mass Flow Rate Required
                    SteamMassFlowRate = 0.0;
                    SetComponentFlowRate(state, SteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, steamCoil.plantLoc);
                    // Inlet equal to outlet when not required to run.
                    TempWaterOut = TempSteamIn;

                    // Total Heat Transfer to air
                    HeatingCoilLoad = QCoilCap;

                    // The HeatingCoilLoad is the change in the enthalpy of the water
                    steamCoil.OutletWaterEnthalpy = steamCoil.InletSteamEnthalpy;

                    // Outlet flow rate set to inlet
                    steamCoil.OutletSteamMassFlowRate = SteamMassFlowRate;
                    steamCoil.InletSteamMassFlowRate = SteamMassFlowRate;

                } else if (QCoilCap > QSteamCoilMaxHT) {
                    // Setting to Maximum Coil Capacity
                    QCoilCap = QSteamCoilMaxHT;

                    // In practice Sensible & Superheated heat transfer is negligible compared to latent part.
                    // This is required for outlet water temperature, otherwise it will be saturation temperature.
                    // Steam Trap drains off all the Water formed.
                    // Here Degree of Subcooling is used to calculate hot water return temperature.

                    // Calculating Water outlet temperature
                    TempWaterOut = TempSteamIn - SubcoolDeltaTemp;

                    // Steam Mass Flow Rate Required
                    SteamMassFlowRate = QCoilCap / (LatentHeatSteam + SubcoolDeltaTemp * CpWater);
                    SetComponentFlowRate(state, SteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, steamCoil.plantLoc);

                    // recalculate in case previous call changed mass flow rate
                    QCoilCap = SteamMassFlowRate * (LatentHeatSteam + SubcoolDeltaTemp * CpWater);
                    TempAirOut = TempAirIn + QCoilCap / (AirMassFlow * PsyCpAirFnW(Win));

                    // Total Heat Transfer to air
                    HeatingCoilLoad = QCoilCap;

                    // The HeatingCoilLoad is the change in the enthalpy of the water
                    steamCoil.OutletWaterEnthalpy = steamCoil.InletSteamEnthalpy - HeatingCoilLoad / SteamMassFlowRate;
                    steamCoil.OutletSteamMassFlowRate = SteamMassFlowRate;
                    steamCoil.InletSteamMassFlowRate = SteamMassFlowRate;

                } else {
                    // In practice Sensible & Superheated heat transfer is negligible compared to latent part.
                    // This is required for outlet water temperature, otherwise it will be saturation temperature.
                    // Steam Trap drains off all the Water formed.
                    // Here Degree of Subcooling is used to calculate hot water return temperature.

                    // Calculating Water outlet temperature
                    TempWaterOut = TempSteamIn - SubcoolDeltaTemp;

                    // Steam Mass Flow Rate Required
                    SteamMassFlowRate = QCoilCap / (LatentHeatSteam + SubcoolDeltaTemp * CpWater);
                    SetComponentFlowRate(state, SteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, steamCoil.plantLoc);

                    // recalculate in case previous call changed mass flow rate
                    QCoilCap = SteamMassFlowRate * (LatentHeatSteam + SubcoolDeltaTemp * CpWater);
                    TempAirOut = TempAirIn + QCoilCap / (AirMassFlow * PsyCpAirFnW(Win));

                    // Total Heat Transfer to air
                    HeatingCoilLoad = QCoilCap;

                    steamCoil.OutletSteamMassFlowRate = SteamMassFlowRate;
                    steamCoil.InletSteamMassFlowRate = SteamMassFlowRate;

                    //************************* Loop Losses *****************************
                    // Loop pressure return considerations included in steam coil since the pipes are
                    // perfect and do not account for losses.

                    // Return water is condensate at atmoshperic pressure
                    // Process is considered constant enthalpy expansion
                    // No quality function in EnergyPlus hence no option left apart from
                    // considering saturated state.
                    //              StdBaroPress=101325

                    TempWaterAtmPress = steamCoil.steam->getSatTemperature(state, state.dataEnvrn->StdBaroPress, RoutineName);

                    // Point 4 at atm - loop delta subcool during return journery back to pump
                    TempLoopOutToPump = TempWaterAtmPress - steamCoil.LoopSubcoolReturn;

                    // Actual Steam Coil Outlet Enthalpy
                    EnthCoilOutlet = steamCoil.steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName) - CpWater * SubcoolDeltaTemp;

                    // Enthalpy at Point 4
                    EnthAtAtmPress = steamCoil.steam->getSatEnthalpy(state, TempWaterAtmPress, 0.0, RoutineName);

                    CpWater = steamCoil.steam->getSatSpecificHeat(state, TempLoopOutToPump, 0.0, RoutineNameSizeSteamCoil);

                    // Reported value of coil outlet enthalpy at the node to match the node outlet temperature
                    EnthPumpInlet = EnthAtAtmPress - CpWater * steamCoil.LoopSubcoolReturn;

                    steamCoil.OutletWaterEnthalpy = EnthPumpInlet;

                    // Point 3-Point 5,
                    EnergyLossToEnvironment = SteamMassFlowRate * (EnthCoilOutlet - EnthPumpInlet);

                    // Loss to environment due to pressure drop
                    steamCoil.LoopLoss = EnergyLossToEnvironment;
                    //************************* Loop Losses *****************************
                }

            } else { // If not running Conditions do not change across coil from inlet to outlet
                SteamMassFlowRate = 0.0;
                SetComponentFlowRate(state, SteamMassFlowRate, steamCoil.SteamInletNodeNum, steamCoil.SteamOutletNodeNum, steamCoil.plantLoc);
                TempAirOut = TempAirIn;
                TempWaterOut = TempSteamIn;
                HeatingCoilLoad = 0.0;
                steamCoil.OutletWaterEnthalpy = steamCoil.InletSteamEnthalpy;
                steamCoil.OutletSteamMassFlowRate = 0.0;
                steamCoil.OutletSteamQuality = 0.0;
                steamCoil.LoopLoss = 0.0;
                TempLoopOutToPump = TempWaterOut;
            }
            break;
        default:
            assert(false);
        }

        if (fanOp == HVAC::FanOp::Cycling) {
            HeatingCoilLoad *= PartLoadRatio;
        }

        // Set the outlet conditions
        steamCoil.TotSteamHeatingCoilRate = HeatingCoilLoad;
        steamCoil.OutletAirTemp = TempAirOut;
        steamCoil.OutletSteamTemp = TempLoopOutToPump;
        steamCoil.OutletSteamQuality = 0.0;
        QCoilActual = HeatingCoilLoad;

        // This SteamCoil does not change the moisture or Mass Flow across the component
        steamCoil.OutletAirHumRat = steamCoil.InletAirHumRat;
        steamCoil.OutletAirMassFlowRate = steamCoil.InletAirMassFlowRate;
        // Set the outlet enthalpys for air and water
        steamCoil.OutletAirEnthalpy = PsyHFnTdbW(steamCoil.OutletAirTemp, steamCoil.OutletAirHumRat);
    }

    // Beginning of Update subroutines for the SteamCoil Module

    void UpdateSteamCoil(EnergyPlusData &state, int const CoilNum)
    {
        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       na
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the coil outlet nodes.

        // METHODOLOGY EMPLOYED:
        // Data is moved from the coil data structure to the coil outlet nodes.

        using PlantUtilities::SafeCopyPlantNode;

        int AirInletNode;
        int SteamInletNode;
        int AirOutletNode;
        int SteamOutletNode;

        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);

        AirInletNode = steamCoil.AirInletNodeNum;
        SteamInletNode = steamCoil.SteamInletNodeNum;
        AirOutletNode = steamCoil.AirOutletNodeNum;
        SteamOutletNode = steamCoil.SteamOutletNodeNum;

        // Set the outlet air nodes of the SteamCoil
        state.dataLoopNodes->Node(AirOutletNode).MassFlowRate = steamCoil.OutletAirMassFlowRate;
        state.dataLoopNodes->Node(AirOutletNode).Temp = steamCoil.OutletAirTemp;
        state.dataLoopNodes->Node(AirOutletNode).HumRat = steamCoil.OutletAirHumRat;
        state.dataLoopNodes->Node(AirOutletNode).Enthalpy = steamCoil.OutletAirEnthalpy;

        SafeCopyPlantNode(state, SteamInletNode, SteamOutletNode);

        // Set the outlet Steam nodes for the Coil
        //   Node(SteamOutletNode)%MassFlowRate = SteamCoil(CoilNum)%OutletSteamMassFlowRate
        state.dataLoopNodes->Node(SteamOutletNode).Temp = steamCoil.OutletSteamTemp;
        state.dataLoopNodes->Node(SteamOutletNode).Enthalpy = steamCoil.OutletWaterEnthalpy;
        state.dataLoopNodes->Node(SteamOutletNode).Quality = steamCoil.OutletSteamQuality;
        // Node(SteamInletNode)%MassFlowRate  = SteamCoil(CoilNum)%OutletSteamMassFlowRate

        // Set the outlet nodes for properties that just pass through & not used
        state.dataLoopNodes->Node(AirOutletNode).Quality = state.dataLoopNodes->Node(AirInletNode).Quality;
        state.dataLoopNodes->Node(AirOutletNode).Press = state.dataLoopNodes->Node(AirInletNode).Press;
        state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMin = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMin;
        state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMax = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMax;
        state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMaxAvail;

        // Set the outlet nodes for properties that just pass through & not used

        // Node(SteamOutletNode)%Press              = Node(SteamInletNode)%Press
        //   Node(SteamOutletNode)%Press               = StdBaroPress  ! Water out at atm pressure
        //   Node(SteamOutletNode)%HumRat              = Node(SteamInletNode)%HumRat
        //   Node(SteamOutletNode)%MassFlowRateMin     = Node(SteamInletNode)%MassFlowRateMin
        //   Node(SteamOutletNode)%MassFlowRateMax     = Node(SteamInletNode)%MassFlowRateMax
        //   Node(SteamOutletNode)%MassFlowRateMinAvail= Node(SteamInletNode)%MassFlowRateMinAvail
        //   Node(SteamOutletNode)%MassFlowRateMaxAvail= Node(SteamInletNode)%MassFlowRateMaxAvail

        //   IF (SteamCoil(CoilNum)%InletSteamMassFlowRate.EQ.0.0) THEN
        //     Node(SteamInletNode)%MassFlowRate         = 0.0
        //     Node(SteamInletNode)%MassFlowRateMinAvail = 0.0
        //     Node(SteamOutletNode)%MassFlowRateMinAvail= 0.0
        //   END IF

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataLoopNodes->Node(AirOutletNode).CO2 = state.dataLoopNodes->Node(AirInletNode).CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataLoopNodes->Node(AirOutletNode).GenContam = state.dataLoopNodes->Node(AirInletNode).GenContam;
        }
    }

    // End of Update subroutines for the SteamCoil Module

    // Beginning of Reporting subroutines for the SteamCoil Module

    void ReportSteamCoil(EnergyPlusData &state, int const CoilNum)
    {
        // SUBROUTINE INFORMATION:
        //   AUTHOR         Rahul Chillar
        //   DATE WRITTEN   Jan 2005
        //   MODIFIED       na
        //   RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variable for the coils.

        // Report the SteamCoil energy from this component
        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
        steamCoil.TotSteamHeatingCoilEnergy = steamCoil.TotSteamHeatingCoilRate * state.dataHVACGlobal->TimeStepSysSec;
    }

    // End of Reporting subroutines for the SteamCoil Module

    // Utility subroutines for the SteamCoil Module

    int GetSteamCoilIndex(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the index for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int IndexNum; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilType == "COIL:HEATING:STEAM") {
            IndexNum = Util::FindItemInList(CoilName, state.dataSteamCoils->SteamCoil);
        } else {
            IndexNum = 0;
        }

        if (IndexNum == 0) {
            ShowSevereError(state, EnergyPlus::format(R"(GetSteamCoilIndex: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
            ErrorsFound = true;
        }

        return IndexNum;
    }

    int GetCompIndex(EnergyPlusData &state, std::string_view const coilName)
    {
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        int indexNum = Util::FindItemInList(coilName, state.dataSteamCoils->SteamCoil);

        if (indexNum == 0) { // may not find coil name
            ShowSevereError(state,
                            EnergyPlus::format("GetSteamCoilIndex: Could not find CoilType = Coil:Heating:Steam with Name = \"{}\"", coilName));
        }

        return indexNum;
    }

    void CheckSteamCoilSchedule(
        EnergyPlusData &state, [[maybe_unused]] std::string const &CompType, std::string_view CompName, Real64 &Value, int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the correct schedule value for this coil

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CoilNum;

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        // Find the correct Coil number
        if (CompIndex == 0) {
            CoilNum = Util::FindItemInList(CompName, state.dataSteamCoils->SteamCoil);
            if (CoilNum == 0) {
                ShowFatalError(state, EnergyPlus::format("CheckSteamCoilSchedule: Coil not found={}", CompName));
            }
            CompIndex = CoilNum;
            auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
            Value = steamCoil.availSched->getCurrentVal(); // not scheduled?
        } else {
            CoilNum = CompIndex;
            if (CoilNum > state.dataSteamCoils->NumSteamCoils || CoilNum < 1) {
                ShowFatalError(state,
                               EnergyPlus::format("SimulateSteamCoilComponents: Invalid CompIndex passed={}, Number of Steam Coils={}, Coil name={}",
                                                  CoilNum,
                                                  state.dataSteamCoils->NumSteamCoils,
                                                  CompName));
            }
            auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
            if (CompName != steamCoil.Name) {
                ShowFatalError(
                    state,
                    EnergyPlus::format("SimulateSteamCoilComponents: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                       CoilNum,
                                       CompName,
                                       steamCoil.Name));
            }
            Value = steamCoil.availSched->getCurrentVal(); // not scheduled?
        }
    }

    Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state,
                                   std::string const &CoilType, // must match coil types in this module
                                   std::string const &CoilName, // must match coil names for the coil type
                                   bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the max water flow rate for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Return value
        Real64 MaxWaterFlowRate; // returned max water flow rate of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            WhichCoil = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
            if (WhichCoil != 0) {
                // coil does not specify MaxWaterFlowRate
                MaxWaterFlowRate = 0.0;
                ShowRecurringWarningErrorAtEnd(state, "Requested Max Water Flow Rate from COIL:Heating:Steam N/A", state.dataSteamCoils->ErrCount);
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilMaxWaterFlowRate: Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            MaxWaterFlowRate = -1000.0;
        }

        return MaxWaterFlowRate;
    }

    Real64 GetCoilMaxSteamFlowRate(EnergyPlusData &state,
                                   int const CoilIndex, // must match coil types in this module
                                   bool &ErrorsFound    // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the max steam flow rate for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and flow rate is returned
        // as zero.

        // Return value
        Real64 MaxSteamFlowRate; // returned max steam flow rate of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state, "GetCoilMaxSteamFlowRate: Could not find CoilType = \"Coil:Heating:Steam\"");
            ErrorsFound = true;
            MaxSteamFlowRate = 0.0;
        } else {
            MaxSteamFlowRate = state.dataSteamCoils->SteamCoil(CoilIndex).MaxSteamVolFlowRate;
        }

        return MaxSteamFlowRate;
    }

    int GetCoilAirInletNode(EnergyPlusData &state,
                            int const CoilIndex,         // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the air inlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilAirInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(CoilIndex).AirInletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilAirOutletNode(EnergyPlusData &state,
                             int const CoilIndex,         // must match coil types in this module
                             std::string const &CoilName, // must match coil names for the coil type
                             bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the air outlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilAirOutletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(CoilIndex).AirOutletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilAirOutletNode(EnergyPlusData &state,
                             std::string const &CoilType,       // must match coil types in this module
                             std::string const &CoilName,       // must match coil names for the coil type
                             [[maybe_unused]] bool &ErrorsFound // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the air outlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int IndexNum; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            IndexNum = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
        } else {
            IndexNum = 0;
        }

        if (IndexNum == 0) {
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(IndexNum).AirOutletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilSteamInletNode(EnergyPlusData &state,
                              int const CoilIndex,         // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam inlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(CoilIndex).SteamInletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilSteamInletNode(EnergyPlusData &state,
                              std::string const &CoilType, // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         L. Lawrie (based on R. Raustad)
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam inlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int IndexNum; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            IndexNum = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
        } else {
            IndexNum = 0;
        }

        if (IndexNum == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(IndexNum).SteamInletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilSteamOutletNode(EnergyPlusData &state,
                               int const CoilIndex,         // must match coil types in this module
                               std::string const &CoilName, // must match coil names for the coil type
                               bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam inlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(CoilIndex).SteamOutletNodeNum;
        }

        return NodeNumber;
    }

    int GetCoilSteamOutletNode(EnergyPlusData &state,
                               std::string_view CoilType,   // must match coil types in this module
                               std::string const &CoilName, // must match coil names for the coil type
                               bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         L. Lawrie (based on R. Raustad)
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam inlet node number for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned air inlet node number of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int IndexNum; // returned air inlet node number of matched coil

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            IndexNum = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
        } else {
            IndexNum = 0;
        }

        if (IndexNum == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        } else {
            NodeNumber = state.dataSteamCoils->SteamCoil(IndexNum).SteamOutletNodeNum;
        }

        return NodeNumber;
    }

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string_view const CoilType, // must match coil types in this module
                           std::string const &CoilName,     // must match coil names for the coil type
                           bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam coils operating capacity and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        Real64 Capacity; // returned operating capacity of matched coil (W)

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            WhichCoil = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
            if (WhichCoil != 0) {
                // coil does not specify MaxWaterFlowRate
                Capacity = state.dataSteamCoils->SteamCoil(WhichCoil).OperatingCapacity;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            Capacity = 0.0;
        }

        return Capacity;
    }

    CoilControlType GetTypeOfCoil(EnergyPlusData &state,
                                  int const CoilIndex,         // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam coils operating capacity and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetCoilSteamInletNode: Could not find CoilType = \"Coil:Heating:Steam\" with Name = {}", CoilName));
            ErrorsFound = true;
            return CoilControlType::Invalid;
        }
        return state.dataSteamCoils->SteamCoil(CoilIndex).TypeOfCoil;
    }

    int GetSteamCoilControlNodeNum(EnergyPlusData &state,
                                   std::string const &CoilType, // must match coil types in this module
                                   std::string const &CoilName, // must match coil names for the coil type
                                   bool &ErrorFlag              // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   January 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the steam coils and returns the steam control node number.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
        // as zero.

        // Return value
        int NodeNumber; // returned node number of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates SteamCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        WhichCoil = 0;
        NodeNumber = 0;
        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            WhichCoil = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
            if (WhichCoil != 0) {
                NodeNumber = state.dataSteamCoils->SteamCoil(WhichCoil).TempSetPointNodeNum;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state,
                            EnergyPlus::format("GetSteamCoilControlNodeNum: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorFlag = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    int GetSteamCoilAvailScheduleIndex(EnergyPlusData &state,
                                       std::string const &CoilType, // must match coil types in this module
                                       std::string const &CoilName, // must match coil names for the coil type
                                       bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Chandan Sharma, FSEC
        //       DATE WRITTEN   February 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the availability schedule index.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
        // as zero.

        // Return value
        int AvailSchIndex; // returned availability schedule of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates HeatingCoil related parameters from input file
        if (state.dataSteamCoils->GetSteamCoilsInputFlag) { // First time subroutine has been entered
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        WhichCoil = 0;
        AvailSchIndex = 0;

        if (Util::SameString(CoilType, "Coil:Heating:Steam")) {
            WhichCoil = Util::FindItem(CoilName, state.dataSteamCoils->SteamCoil);
            if (WhichCoil != 0) {
                AvailSchIndex = state.dataSteamCoils->SteamCoil(WhichCoil).availSched->Num;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, EnergyPlus::format("GetCoilAvailScheduleIndex: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            AvailSchIndex = 0;
        }

        return AvailSchIndex;
    }

    void SetSteamCoilData(EnergyPlusData &state,
                          int const CoilNum,                                  // Number of hot water heating Coil
                          bool &ErrorsFound,                                  // Set to true if certain errors found
                          ObjexxFCL::Optional_bool DesiccantRegenerationCoil, // Flag that this coil is used as regeneration air heating coil
                          ObjexxFCL::Optional_int DesiccantDehumIndex         // Index for the desiccant dehum system where this caoil is used
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   February 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function sets data to water Heating Coil using the coil index and arguments passed

        if (state.dataSteamCoils->GetSteamCoilsInputFlag) {
            GetSteamCoilInput(state);
            state.dataSteamCoils->GetSteamCoilsInputFlag = false;
        }

        if (CoilNum <= 0 || CoilNum > state.dataSteamCoils->NumSteamCoils) {
            ShowSevereError(state,
                            EnergyPlus::format("SetHeatingCoilData: called with heating coil Number out of range={} should be >0 and <{}",
                                               CoilNum,
                                               state.dataSteamCoils->NumSteamCoils));
            ErrorsFound = true;
            return;
        }

        auto &steamCoil = state.dataSteamCoils->SteamCoil(CoilNum);
        if (present(DesiccantRegenerationCoil)) {
            steamCoil.DesiccantRegenerationCoil = DesiccantRegenerationCoil;
        }

        if (present(DesiccantDehumIndex)) {
            steamCoil.DesiccantDehumNum = DesiccantDehumIndex;
        }
    }
    // End of Utility subroutines for the SteamCoil Module

} // namespace SteamCoils

} // namespace EnergyPlus
