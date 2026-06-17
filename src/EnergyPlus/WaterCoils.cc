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

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterDesAirInletHumRatSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterDesAirInletTempSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterDesAirOutletHumRatSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterDesAirOutletTempSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterDesWaterInletTempSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterNumofTubesPerRowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingWaterflowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirflowUASizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingWaterDesAirInletHumRatSizing.hh>
#include <EnergyPlus/Autosizing/HeatingWaterDesAirInletTempSizing.hh>
#include <EnergyPlus/Autosizing/HeatingWaterDesCoilLoadUsedForUASizing.hh>
#include <EnergyPlus/Autosizing/HeatingWaterDesCoilWaterVolFlowUsedForUASizing.hh>
#include <EnergyPlus/Autosizing/HeatingWaterflowSizing.hh>
#include <EnergyPlus/Autosizing/WaterHeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/WaterHeatingCoilUASizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus::WaterCoils {
// Module containing the WaterCoil simulation routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   April 1998
//       MODIFIED       April 2004: Rahul Chillar
//                      Feb. 2010, Brent Griffith, Plant Demand Side Update, general fluid properties
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the WaterCoil System Component

using Psychrometrics::PsyCpAirFnW;
using Psychrometrics::PsyHFnTdbRhPb;
using Psychrometrics::PsyHFnTdbW;
using Psychrometrics::PsyRhoAirFnPbTdbW;
using Psychrometrics::PsyTdbFnHW;
using Psychrometrics::PsyTdpFnWPb;
using Psychrometrics::PsyTsatFnHPb;
using Psychrometrics::PsyWFnTdbH;
using Psychrometrics::PsyWFnTdbRhPb;
using Psychrometrics::PsyWFnTdbTwbPb;
using Psychrometrics::PsyWFnTdpPb;

void SimulateWaterCoilComponents(EnergyPlusData &state,
                                 std::string_view CompName,
                                 bool const FirstHVACIteration,
                                 int &CompIndex,
                                 ObjexxFCL::Optional<Real64> QActual,
                                 ObjexxFCL::Optional<HVAC::FanOp const> fanOpMode,
                                 ObjexxFCL::Optional<Real64 const> PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages WaterCoil component simulation.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilNum;         // The WaterCoil that you are currently loading input into
    HVAC::FanOp fanOp;   // fan operating mode
    Real64 PartLoadFrac; // part-load fraction of heating coil

    // Obtains and Allocates WaterCoil related parameters from input file
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) { // First time subroutine has been entered
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    // Find the correct WaterCoilNumber with the Coil Name
    if (CompIndex == 0) {
        CoilNum = Util::FindItemInList(CompName, state.dataWaterCoils->WaterCoil);
        if (CoilNum == 0) {
            ShowFatalError(state, std::format("SimulateWaterCoilComponents: Coil not found={}", CompName));
        }
        CompIndex = CoilNum;
    } else {
        CoilNum = CompIndex;
        if (CoilNum > state.dataWaterCoils->NumWaterCoils || CoilNum < 1) {
            ShowFatalError(state,
                           std::format("SimulateWaterCoilComponents: Invalid CompIndex passed={}, Number of Water Coils={}, Coil name={}",
                                       CoilNum,
                                       state.dataWaterCoils->NumWaterCoils,
                                       CompName));
        }
        if (state.dataWaterCoils->CheckEquipName(CoilNum)) {
            auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
            if (CompName != waterCoil.Name) {
                ShowFatalError(
                    state,
                    std::format("SimulateWaterCoilComponents: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                CoilNum,
                                CompName,
                                waterCoil.Name));
            }
            state.dataWaterCoils->CheckEquipName(CoilNum) = false;
        }
    }

    // With the correct CoilNum Initialize
    InitWaterCoil(state, CoilNum, FirstHVACIteration); // Initialize all WaterCoil related parameters

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

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);

    // Calculate the Correct WaterCoil Model with the current CoilNum
    if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
        CalcDetailFlatFinCoolingCoil(state, CoilNum, state.dataWaterCoils->SimCalc, fanOp, PartLoadFrac);
        if (present(QActual)) {
            QActual = waterCoil.SenWaterCoolingCoilRate;
        }
    } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
        CoolingCoil(state, CoilNum, FirstHVACIteration, state.dataWaterCoils->SimCalc, fanOp, PartLoadFrac);
        if (present(QActual)) {
            QActual = waterCoil.SenWaterCoolingCoilRate;
        }
    }

    if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
        CalcSimpleHeatingCoil(state, CoilNum, fanOp, PartLoadFrac, state.dataWaterCoils->SimCalc);
        if (present(QActual)) {
            QActual = waterCoil.TotWaterHeatingCoilRate;
        }
    }

    // Update the current WaterCoil to the outlet nodes
    UpdateWaterCoil(state, CoilNum);

    // Report the current WaterCoil
    ReportWaterCoil(state, CoilNum);
}

// Get Input Section of the Module
//******************************************************************************

void GetWaterCoilInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   April 1998
    //       MODIFIED       April 2004: Rahul Chillar
    //                      November 2013: Tianzhen Hong for fouling coils
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for coils and stores it in coil data structures

    // METHODOLOGY EMPLOYED:
    // Uses "Get" routines to read in data.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetWaterCoilInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetWaterCoilInput";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilNum; // The WaterCoil that you are currently loading input into
    int NumSimpHeat(0);
    int NumFlatFin(0);
    int NumCooling(0);
    int SimpHeatNum;
    int FlatFinNum;
    int CoolingNum;
    int NumAlphas;
    int NumNums;
    int IOStat;
    std::string CurrentModuleObject; // for ease in getting objects
    Array1D_string AlphArray;        // Alpha input items for object
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D<Real64> NumArray;        // Numeric input items for object
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
    int MaxNums(0);                  // Maximum number of numeric input fields
    int MaxAlphas(0);                // Maximum number of alpha input fields
    int TotalArgs(0);                // Total number of alpha and numeric arguments (max) for a certain object in the input file
    bool ErrorsFound(false);         // If errors detected in input

    NumSimpHeat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:Water");
    NumFlatFin = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Cooling:Water:DetailedGeometry");
    NumCooling = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Cooling:Water");
    state.dataWaterCoils->NumWaterCoils = NumSimpHeat + NumFlatFin + NumCooling;

    if (state.dataWaterCoils->NumWaterCoils > 0) {
        state.dataWaterCoils->WaterCoil.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->WaterCoilNumericFields.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->WaterTempCoolCoilErrs.dimension(state.dataWaterCoils->NumWaterCoils, 0);
        state.dataWaterCoils->PartWetCoolCoilErrs.dimension(state.dataWaterCoils->NumWaterCoils, 0);
        state.dataWaterCoils->CheckEquipName.dimension(state.dataWaterCoils->NumWaterCoils, true);
    }

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Coil:Heating:Water", TotalArgs, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Coil:Cooling:Water:DetailedGeometry", TotalArgs, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Coil:Cooling:Water", TotalArgs, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);

    AlphArray.allocate(MaxAlphas);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNums);
    NumArray.dimension(MaxNums, 0.0);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNums, true);
    CurrentModuleObject = "Coil:Heating:Water";
    // Get the data for simple heating coils
    for (SimpHeatNum = 1; SimpHeatNum <= NumSimpHeat; ++SimpHeatNum) {

        CoilNum = SimpHeatNum;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 SimpHeatNum,
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

        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames.allocate(MaxNums);
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = "";
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = cNumericFields;

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");
        auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);

        waterCoil.Name = AlphArray(1);
        waterCoil.coilType = HVAC::CoilType::HeatingWater;
        waterCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, waterCoil.Name, waterCoil.coilType);

        if (lAlphaBlanks(2)) {
            waterCoil.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((waterCoil.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
            ErrorsFound = true;
        }

        waterCoil.WaterCoilModelA = "SIMPLE";
        waterCoil.WaterCoilModel = CoilModel::HeatingSimple; // 'SIMPLE'
        waterCoil.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;

        waterCoil.UACoil = NumArray(1);
        waterCoil.UACoilVariable = waterCoil.UACoil;
        waterCoil.MaxWaterVolFlowRate = NumArray(2);
        waterCoil.WaterInletNodeNum = Node::GetOnlySingleNode(state,
                                                              AlphArray(3),
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilHeatingWater,
                                                              AlphArray(1),
                                                              Node::FluidType::Water,
                                                              Node::ConnectionType::Inlet,
                                                              Node::CompFluidStream::Secondary,
                                                              Node::ObjectIsNotParent);
        waterCoil.WaterOutletNodeNum = Node::GetOnlySingleNode(state,
                                                               AlphArray(4),
                                                               ErrorsFound,
                                                               Node::ConnectionObjectType::CoilHeatingWater,
                                                               AlphArray(1),
                                                               Node::FluidType::Water,
                                                               Node::ConnectionType::Outlet,
                                                               Node::CompFluidStream::Secondary,
                                                               Node::ObjectIsNotParent);
        waterCoil.AirInletNodeNum = Node::GetOnlySingleNode(state,
                                                            AlphArray(5),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilHeatingWater,
                                                            AlphArray(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
        waterCoil.AirOutletNodeNum = Node::GetOnlySingleNode(state,
                                                             AlphArray(6),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilHeatingWater,
                                                             AlphArray(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);

        if (AlphArray(7) == "NOMINALCAPACITY") { // not "UFACTORTIMESAREAANDDESIGNWATERFLOWRATE"
            waterCoil.CoilPerfInpMeth = state.dataWaterCoils->NomCap;

        } else {
            // will be caught by input processor
            waterCoil.CoilPerfInpMeth = state.dataWaterCoils->UAandFlow;
        }

        waterCoil.DesTotWaterCoilLoad = NumArray(3);

        if (waterCoil.UACoil == DataSizing::AutoSize && waterCoil.CoilPerfInpMeth == state.dataWaterCoils->UAandFlow) {
            waterCoil.RequestingAutoSize = true;
        }
        if (waterCoil.MaxWaterVolFlowRate == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        if (waterCoil.DesTotWaterCoilLoad == DataSizing::AutoSize && waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap) {
            waterCoil.RequestingAutoSize = true;
        }

        waterCoil.DesInletWaterTemp = NumArray(4);
        waterCoil.DesInletAirTemp = NumArray(5);
        waterCoil.DesOutletWaterTemp = NumArray(6);
        waterCoil.DesOutletAirTemp = NumArray(7);
        waterCoil.RatioAirSideToWaterSideConvect = NumArray(8);
        if (!lNumericBlanks(9)) {
            waterCoil.DesignWaterDeltaTemp = NumArray(9);
            waterCoil.UseDesignWaterDeltaTemp = true;
        } else {
            waterCoil.UseDesignWaterDeltaTemp = false;
        }
        if (waterCoil.DesInletWaterTemp <= waterCoil.DesOutletWaterTemp) {
            ShowSevereError(state, std::format("For {}, {}", CurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, std::format("  the {} must be greater than the {}.", cNumericFields(4), cNumericFields(6)));
            ErrorsFound = true;
        }
        if (waterCoil.DesInletAirTemp >= waterCoil.DesOutletAirTemp) {
            ShowSevereError(state, std::format("For {}, {}", CurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, std::format("  the {} must be less than the {}.", cNumericFields(5), cNumericFields(7)));
            ErrorsFound = true;
        }
        if (waterCoil.DesInletAirTemp >= waterCoil.DesInletWaterTemp) {
            ShowSevereError(state, std::format("For {}, {}", CurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, std::format("  the {} must be less than the {}.", cNumericFields(5), cNumericFields(4)));
            ErrorsFound = true;
        }

        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(3), AlphArray(4), "Water Nodes");
        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(5), AlphArray(6), "Air Nodes");

        // Setup the Simple Heating Coil reporting variables
        // CurrentModuleObject = "Coil:Heating:Water"
        SetupOutputVariable(state,
                            "Heating Coil Heating Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterHeatingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::HeatingCoils);
        SetupOutputVariable(state,
                            "Heating Coil Source Side Heat Transfer Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterHeatingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::PlantLoopHeatingDemand,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::HeatingCoils);
        SetupOutputVariable(state,
                            "Heating Coil Heating Rate",
                            Constant::Units::W,
                            waterCoil.TotWaterHeatingCoilRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Heating Coil U Factor Times Area Value",
                            Constant::Units::W_K,
                            waterCoil.UACoilVariable,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);
    }

    CurrentModuleObject = "Coil:Cooling:Water:DetailedGeometry";
    // Get the data for detailed cooling coils.
    for (FlatFinNum = 1; FlatFinNum <= NumFlatFin; ++FlatFinNum) {

        CoilNum = NumSimpHeat + FlatFinNum;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 FlatFinNum,
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

        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames.allocate(MaxNums);
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = "";
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = cNumericFields;

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

        auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        waterCoil.Name = AlphArray(1);
        waterCoil.coilType = HVAC::CoilType::CoolingWaterDetailed;
        waterCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, waterCoil.Name, waterCoil.coilType);

        if (lAlphaBlanks(2)) {
            waterCoil.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((waterCoil.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
            ErrorsFound = true;
        }

        waterCoil.WaterCoilModelA = "DETAILED FLAT FIN";
        waterCoil.WaterCoilModel = CoilModel::CoolingDetailed; // 'DETAILED FLAT FIN'
        waterCoil.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;

        waterCoil.MaxWaterVolFlowRate = NumArray(1);
        if (waterCoil.MaxWaterVolFlowRate == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.TubeOutsideSurfArea = NumArray(2);
        if (waterCoil.TubeOutsideSurfArea == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.TotTubeInsideArea = NumArray(3);
        if (waterCoil.TotTubeInsideArea == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.FinSurfArea = NumArray(4);
        if (waterCoil.FinSurfArea == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.MinAirFlowArea = NumArray(5);
        if (waterCoil.MinAirFlowArea == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.CoilDepth = NumArray(6);
        if (waterCoil.CoilDepth == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.FinDiam = NumArray(7);
        if (waterCoil.FinDiam == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.FinThickness = NumArray(8);
        if (waterCoil.FinThickness <= 0.0) {
            ShowSevereError(
                state, std::format("{}: {} must be > 0.0, for {} = {}", CurrentModuleObject, cNumericFields(8), cAlphaFields(1), waterCoil.Name));
            ErrorsFound = true;
        }
        waterCoil.TubeInsideDiam = NumArray(9);
        waterCoil.TubeOutsideDiam = NumArray(10);
        waterCoil.TubeThermConductivity = NumArray(11);
        if (waterCoil.TubeThermConductivity <= 0.0) {
            ShowSevereError(
                state, std::format("{}: {} must be > 0.0, for {} = {}", CurrentModuleObject, cNumericFields(11), cAlphaFields(1), waterCoil.Name));
            ErrorsFound = true;
        }
        waterCoil.FinThermConductivity = NumArray(12);
        if (waterCoil.FinThermConductivity <= 0.0) {
            ShowSevereError(
                state, std::format("{}: {} must be > 0.0, for {} = {}", CurrentModuleObject, cNumericFields(12), cAlphaFields(1), waterCoil.Name));
            ErrorsFound = true;
        }
        waterCoil.FinSpacing = NumArray(13);
        waterCoil.TubeDepthSpacing = NumArray(14);
        waterCoil.NumOfTubeRows = NumArray(15);
        waterCoil.NumOfTubesPerRow = NumArray(16);
        if (waterCoil.NumOfTubesPerRow == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        if (!lNumericBlanks(17)) {
            waterCoil.DesignWaterDeltaTemp = NumArray(17);
            waterCoil.UseDesignWaterDeltaTemp = true;
        } else {
            waterCoil.UseDesignWaterDeltaTemp = false;
        }
        waterCoil.WaterInletNodeNum = Node::GetOnlySingleNode(state,
                                                              AlphArray(3),
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilCoolingWaterDetailedGeometry,
                                                              AlphArray(1),
                                                              Node::FluidType::Water,
                                                              Node::ConnectionType::Inlet,
                                                              Node::CompFluidStream::Secondary,
                                                              Node::ObjectIsNotParent);
        waterCoil.WaterOutletNodeNum = Node::GetOnlySingleNode(state,
                                                               AlphArray(4),
                                                               ErrorsFound,
                                                               Node::ConnectionObjectType::CoilCoolingWaterDetailedGeometry,
                                                               AlphArray(1),
                                                               Node::FluidType::Water,
                                                               Node::ConnectionType::Outlet,
                                                               Node::CompFluidStream::Secondary,
                                                               Node::ObjectIsNotParent);
        waterCoil.AirInletNodeNum = Node::GetOnlySingleNode(state,
                                                            AlphArray(5),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilCoolingWaterDetailedGeometry,
                                                            AlphArray(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
        waterCoil.AirOutletNodeNum = Node::GetOnlySingleNode(state,
                                                             AlphArray(6),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingWaterDetailedGeometry,
                                                             AlphArray(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);

        // A7 ; \field Name of Water Storage Tank for Condensate Collection
        waterCoil.CondensateCollectName = AlphArray(7);
        if (lAlphaBlanks(7)) {
            waterCoil.CondensateCollectMode = state.dataWaterCoils->CondensateDiscarded;
        } else {
            waterCoil.CondensateCollectMode = state.dataWaterCoils->CondensateToTank;
            WaterManager::SetupTankSupplyComponent(state,
                                                   waterCoil.Name,
                                                   CurrentModuleObject,
                                                   waterCoil.CondensateCollectName,
                                                   ErrorsFound,
                                                   waterCoil.CondensateTankID,
                                                   waterCoil.CondensateTankSupplyARRID);
        }

        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(3), AlphArray(4), "Water Nodes");
        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(5), AlphArray(6), "Air Nodes");

        // Setup Report variables for the Detailed Flat Fin Cooling Coils
        // CurrentModuleObject = "Coil:Cooling:Water:DetailedGeometry"
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Cooling Coil Source Side Heat Transfer Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::PlantLoopCoolingDemand,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Energy",
                            Constant::Units::J,
                            waterCoil.SenWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Rate",
                            Constant::Units::W,
                            waterCoil.TotWaterCoolingCoilRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Rate",
                            Constant::Units::W,
                            waterCoil.SenWaterCoolingCoilRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);

        if (waterCoil.CondensateCollectMode == state.dataWaterCoils->CondensateToTank) {

            SetupOutputVariable(state,
                                "Cooling Coil Condensate Volume Flow Rate",
                                Constant::Units::m3_s,
                                waterCoil.CondensateVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                waterCoil.Name);
            SetupOutputVariable(state,
                                "Cooling Coil Condensate Volume",
                                Constant::Units::m3,
                                waterCoil.CondensateVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                waterCoil.Name,
                                Constant::eResource::OnSiteWater,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Condensate);
        }
    }

    CurrentModuleObject = "Coil:Cooling:Water";
    // Get the data for Cooling coils.
    for (CoolingNum = 1; CoolingNum <= NumCooling; ++CoolingNum) {

        CoilNum = NumSimpHeat + NumFlatFin + CoolingNum;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 CoolingNum,
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
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames.allocate(MaxNums);
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = "";
        state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames = cNumericFields;

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

        auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        waterCoil.Name = AlphArray(1);
        waterCoil.coilType = HVAC::CoilType::CoolingWater;
        waterCoil.coilReportNum = ReportCoilSelection::getReportIndex(state, waterCoil.Name, waterCoil.coilType);

        if (lAlphaBlanks(2)) {
            waterCoil.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((waterCoil.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
            ErrorsFound = true;
        }

        waterCoil.WaterCoilModelA = "Cooling";
        waterCoil.WaterCoilModel = CoilModel::CoolingSimple; // 'Cooling'
        waterCoil.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;

        waterCoil.MaxWaterVolFlowRate = NumArray(1); // Liquid mass flow rate at Design  kg/s
        if (waterCoil.MaxWaterVolFlowRate == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesAirVolFlowRate = NumArray(2); // Dry air mass flow rate at Design (kg/s)
        if (waterCoil.DesAirVolFlowRate == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesInletWaterTemp = NumArray(3); // Entering water temperature at Design C
        if (waterCoil.DesInletWaterTemp == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesInletAirTemp = NumArray(4); // Entering air dry bulb temperature at Design(C)
        if (waterCoil.DesInletAirTemp == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesOutletAirTemp = NumArray(5); // Leaving air dry bulb temperature at Design(C)
        if (waterCoil.DesOutletAirTemp == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesInletAirHumRat = NumArray(6); // Entering air humidity ratio  at Design
        if (waterCoil.DesInletAirHumRat == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        waterCoil.DesOutletAirHumRat = NumArray(7); // Leaving air humidity ratio  at Design
        if (waterCoil.DesOutletAirHumRat == DataSizing::AutoSize) {
            waterCoil.RequestingAutoSize = true;
        }
        if (!lNumericBlanks(8)) {
            waterCoil.DesignWaterDeltaTemp = NumArray(8);
            waterCoil.UseDesignWaterDeltaTemp = true;
        } else {
            waterCoil.UseDesignWaterDeltaTemp = false;
        }

        waterCoil.WaterInletNodeNum = Node::GetOnlySingleNode(state,
                                                              AlphArray(3),
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilCoolingWater,
                                                              AlphArray(1),
                                                              Node::FluidType::Water,
                                                              Node::ConnectionType::Inlet,
                                                              Node::CompFluidStream::Secondary,
                                                              Node::ObjectIsNotParent);
        waterCoil.WaterOutletNodeNum = Node::GetOnlySingleNode(state,
                                                               AlphArray(4),
                                                               ErrorsFound,
                                                               Node::ConnectionObjectType::CoilCoolingWater,
                                                               AlphArray(1),
                                                               Node::FluidType::Water,
                                                               Node::ConnectionType::Outlet,
                                                               Node::CompFluidStream::Secondary,
                                                               Node::ObjectIsNotParent);
        waterCoil.AirInletNodeNum = Node::GetOnlySingleNode(state,
                                                            AlphArray(5),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilCoolingWater,
                                                            AlphArray(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
        waterCoil.AirOutletNodeNum = Node::GetOnlySingleNode(state,
                                                             AlphArray(6),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingWater,
                                                             AlphArray(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);

        // The default is SimpleAnalysis = 2.  and DetailedAnalysis   =1
        if (AlphArray(7) == "DETAILEDANALYSIS") { // not "SIMPLEANALYSIS"
            waterCoil.CoolingCoilAnalysisMode = state.dataWaterCoils->DetailedAnalysis;

        } else {
            waterCoil.CoolingCoilAnalysisMode = state.dataWaterCoils->SimpleAnalysis;
        }

        // The default is CrossFlow = 2.  and CounterFlow=1
        if (AlphArray(8) == "COUNTERFLOW") { // not "CROSSFLOW"
            waterCoil.HeatExchType = state.dataWaterCoils->CounterFlow;

        } else {
            waterCoil.HeatExchType = state.dataWaterCoils->CrossFlow;
        }

        // A9; \field Name of Water Storage Tank for Condensate Collection
        waterCoil.CondensateCollectName = AlphArray(9);
        if (lAlphaBlanks(9)) {
            waterCoil.CondensateCollectMode = state.dataWaterCoils->CondensateDiscarded;
        } else {
            waterCoil.CondensateCollectMode = state.dataWaterCoils->CondensateToTank;
            WaterManager::SetupTankSupplyComponent(state,
                                                   waterCoil.Name,
                                                   CurrentModuleObject,
                                                   waterCoil.CondensateCollectName,
                                                   ErrorsFound,
                                                   waterCoil.CondensateTankID,
                                                   waterCoil.CondensateTankSupplyARRID);
        }

        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(3), AlphArray(4), "Water Nodes");
        Node::TestCompSet(state, CurrentModuleObject, AlphArray(1), AlphArray(5), AlphArray(6), "Air Nodes");

        // Setup Report variables for the Design input Cooling Coils
        // CurrentModuleObject = "Coil:Cooling:Water"
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Cooling Coil Source Side Heat Transfer Energy",
                            Constant::Units::J,
                            waterCoil.TotWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name,
                            Constant::eResource::PlantLoopCoolingDemand,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Energy",
                            Constant::Units::J,
                            waterCoil.SenWaterCoolingCoilEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Rate",
                            Constant::Units::W,
                            waterCoil.TotWaterCoolingCoilRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Rate",
                            Constant::Units::W,
                            waterCoil.SenWaterCoolingCoilRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Wetted Area Fraction",
                            Constant::Units::None,
                            waterCoil.SurfAreaWetFraction,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            waterCoil.Name);

        if (waterCoil.CondensateCollectMode == state.dataWaterCoils->CondensateToTank) {

            SetupOutputVariable(state,
                                "Cooling Coil Condensate Volume Flow Rate",
                                Constant::Units::m3_s,
                                waterCoil.CondensateVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                waterCoil.Name);
            SetupOutputVariable(state,
                                "Cooling Coil Condensate Volume",
                                Constant::Units::m3,
                                waterCoil.CondensateVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                waterCoil.Name,
                                Constant::eResource::OnSiteWater,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Condensate);
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, std::format("{}Errors found in getting input.", RoutineName));
    }

    AlphArray.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
    NumArray.deallocate();
    lAlphaBlanks.deallocate();
    lNumericBlanks.deallocate();
}

void InitWaterCoil(EnergyPlusData &state, int const CoilNum, bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       April 2004: Rahul Chillar
    //                      November 2013: XP, Tianzhen Hong to handle fouling coils
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the WaterCoil Components.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 SmallNo(1.e-9); // SmallNo number in place of zero
    constexpr int itmax(10);
    static constexpr std::string_view RoutineName("InitWaterCoil");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int tempCoilNum;                   // loop variable
    Real64 DesInletAirEnth;            // Entering air enthalpy at rating (J/kg)
    Real64 DesOutletAirEnth;           // Leaving air enthalpy at rating(J/kg)
    Real64 DesAirApparatusDewPtEnth;   // Air enthalpy at apparatus dew point at rating(J/kg)
    Real64 DesSatEnthAtWaterInTemp;    // Saturated enthalpy at entering liquid temp(J/kg)
    Real64 DesHumRatAtWaterInTemp;     // Enthalpy at water inlet temp and entering air HumRat (J/kg)
    Real64 CapacitanceAir;             // Air-side capacity rate(W/C)
    Real64 DesAirTempApparatusDewPt;   // Temperature apparatus dew point at design capacity
    Real64 DesAirHumRatApparatusDewPt; // Humidity Ratio at apparatus dew point at design capacity
    Real64 DesBypassFactor;            // ByPass Factor at design condition
    Real64 SlopeTempVsHumRatio;        // Ratio temperature difference to humidity difference
    // between entering and leaving air states
    Real64 TempApparatusDewPtEstimate; // Estimate of TAdp from SlopeTempVsHumRatio
    Real64 Y1;                         // Previous values of dependent variable in ITERATE
    Real64 X1;                         // Previous values of independent variable in ITERATE
    Real64 error;                      // Deviation of dependent variable in iteration
    int iter;                          // Iteration counter
    int icvg;                          // Iteration convergence flag
    Real64 ResultX;                    // Output variable from ITERATE function.
    int Ipass;                         // loop index for App_Dewpoint_Loop
    int AirInletNode;
    int WaterInletNode;
    int WaterOutletNode;
    Real64 FinDiamVar;
    Real64 TubeToFinDiamRatio;
    Real64 CpAirStd; // specific heat of air at std conditions
    Real64 UA0;      // lower bound for UA
    Real64 UA1;      // upper bound for UA
    Real64 UA;
    Real64 DesUACoilExternalEnth; // enthalpy based UAExternal for wet coil surface {kg/s}
    Real64 LogMeanEnthDiff;       // long mean enthalpy difference {J/kg}
    Real64 LogMeanTempDiff;       // long mean temperature difference {C}
    Real64 DesOutletWaterTemp;
    Real64 DesSatEnthAtWaterOutTemp;
    Real64 DesEnthAtWaterOutTempAirInHumRat;
    Real64 DesEnthWaterOut;
    Real64 Cp;  // local fluid specific heat
    Real64 rho; // local fluid density
    bool errFlag;
    Real64 EnthCorrFrac(0.0); // enthalpy correction factor
    Real64 TempCorrFrac(0.0); // temperature correction factor

    if (state.dataWaterCoils->InitWaterCoilOneTimeFlag) {
        // initialize the environment and sizing flags
        state.dataWaterCoils->MyEnvrnFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->MySizeFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->CoilWarningOnceFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->DesCpAir.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->MyUAAndFlowCalcFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->MyCoilDesignFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->MyCoilReportFlag.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->DesUARangeCheck.allocate(state.dataWaterCoils->NumWaterCoils);
        state.dataWaterCoils->PlantLoopScanFlag.allocate(state.dataWaterCoils->NumWaterCoils);

        state.dataWaterCoils->DesCpAir = 0.0;
        state.dataWaterCoils->DesUARangeCheck = 0.0;
        state.dataWaterCoils->MyEnvrnFlag = true;
        state.dataWaterCoils->MySizeFlag = true;
        state.dataWaterCoils->CoilWarningOnceFlag = true;
        state.dataWaterCoils->MyUAAndFlowCalcFlag = true;
        state.dataWaterCoils->MyCoilDesignFlag = true;
        state.dataWaterCoils->MyCoilReportFlag = true;
        state.dataWaterCoils->InitWaterCoilOneTimeFlag = false;
        state.dataWaterCoils->PlantLoopScanFlag = true;

        for (tempCoilNum = 1; tempCoilNum <= state.dataWaterCoils->NumWaterCoils; ++tempCoilNum) {
            HVACControllers::GetControllerNameAndIndex(state,
                                                       state.dataWaterCoils->WaterCoil(tempCoilNum).WaterInletNodeNum,
                                                       state.dataWaterCoils->WaterCoil(tempCoilNum).ControllerName,
                                                       state.dataWaterCoils->WaterCoil(tempCoilNum).ControllerIndex,
                                                       errFlag);
        }
    }

    if (state.dataWaterCoils->WaterCoilControllerCheckOneTimeFlag && (state.dataHVACGlobal->GetAirPathDataDone)) {
        bool ErrorsFound = false;
        bool WaterCoilOnAirLoop = true;
        for (tempCoilNum = 1; tempCoilNum <= state.dataWaterCoils->NumWaterCoils; ++tempCoilNum) {
            if (state.dataWaterCoils->WaterCoil(tempCoilNum).ControllerIndex > 0) {
                SimAirServingZones::CompType CoilTypeNum(SimAirServingZones::CompType::Invalid);
                std::string CompType;
                std::string const &CompName = state.dataWaterCoils->WaterCoil(tempCoilNum).Name;
                if (state.dataWaterCoils->WaterCoil(tempCoilNum).WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                    CoilTypeNum = SimAirServingZones::CompType::WaterCoil_Cooling;
                    CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater];
                } else if (state.dataWaterCoils->WaterCoil(tempCoilNum).WaterCoilType ==
                           DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                    CoilTypeNum = SimAirServingZones::CompType::WaterCoil_DetailedCool;
                    CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterDetailed];
                } else if (state.dataWaterCoils->WaterCoil(tempCoilNum).WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                    CoilTypeNum = SimAirServingZones::CompType::WaterCoil_SimpleHeat;
                    CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater];
                }
                WaterCoilOnAirLoop = true;
                SimAirServingZones::CheckWaterCoilIsOnAirLoop(state, CoilTypeNum, CompType, CompName, WaterCoilOnAirLoop);
                if (!WaterCoilOnAirLoop) {
                    ShowContinueError(state,
                                      std::format("Controller:WaterCoil = {}. Invalid water controller entry.",
                                                  state.dataWaterCoils->WaterCoil(tempCoilNum).ControllerName));
                    ErrorsFound = true;
                }
            }
        }
        state.dataWaterCoils->WaterCoilControllerCheckOneTimeFlag = false;
        if (ErrorsFound) {
            ShowFatalError(state, "Program terminated for previous condition.");
        }
    }

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    if (state.dataWaterCoils->PlantLoopScanFlag(CoilNum) && allocated(state.dataPlnt->PlantLoop)) {
        errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state, waterCoil.Name, waterCoil.WaterCoilType, waterCoil.WaterPlantLoc, errFlag, _, _, _, _, _);
        if (errFlag) {
            ShowFatalError(state, "InitWaterCoil: Program terminated for previous conditions.");
        }
        state.dataWaterCoils->PlantLoopScanFlag(CoilNum) = false;
    }
    if (!state.dataGlobal->SysSizingCalc && state.dataWaterCoils->MySizeFlag(CoilNum)) {
        // for each coil, do the sizing once.
        SizeWaterCoil(state, CoilNum);

        state.dataWaterCoils->MySizeFlag(CoilNum) = false;
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && state.dataWaterCoils->MyEnvrnFlag(CoilNum)) {
        rho = waterCoil.WaterPlantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
        // Initialize all report variables to a known state at beginning of simulation
        waterCoil.TotWaterHeatingCoilEnergy = 0.0;
        waterCoil.TotWaterCoolingCoilEnergy = 0.0;
        waterCoil.SenWaterCoolingCoilEnergy = 0.0;
        waterCoil.TotWaterHeatingCoilRate = 0.0;
        waterCoil.TotWaterCoolingCoilRate = 0.0;
        waterCoil.SenWaterCoolingCoilRate = 0.0;

        // The rest of the one time initializations
        AirInletNode = waterCoil.AirInletNodeNum;
        WaterInletNode = waterCoil.WaterInletNodeNum;
        WaterOutletNode = waterCoil.WaterOutletNodeNum;

        state.dataWaterCoils->DesCpAir(CoilNum) = PsyCpAirFnW(0.0);
        state.dataWaterCoils->DesUARangeCheck(CoilNum) = (-1568.6 * waterCoil.DesInletAirHumRat + 20.157);

        if ((waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
            (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) { // 'Cooling'
            auto &waterInletNode = state.dataLoopNodes->Node(WaterInletNode);
            waterInletNode.Temp = 5.0;

            Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, waterInletNode.Temp, RoutineName);

            waterInletNode.Enthalpy = Cp * waterInletNode.Temp;
            waterInletNode.Quality = 0.0;
            waterInletNode.Press = 0.0;
            waterInletNode.HumRat = 0.0;
        }

        if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) { // 'Heating'
            auto &waterInletNode = state.dataLoopNodes->Node(WaterInletNode);
            waterInletNode.Temp = 60.0;

            Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, waterInletNode.Temp, RoutineName);

            waterInletNode.Enthalpy = Cp * waterInletNode.Temp;
            waterInletNode.Quality = 0.0;
            waterInletNode.Press = 0.0;
            waterInletNode.HumRat = 0.0;
            state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum) = false;
            // fill values for variable UA
            CpAirStd = PsyCpAirFnW(0.0);
            waterCoil.DesAirMassFlowRate = state.dataEnvrn->StdRhoAir * waterCoil.DesAirVolFlowRate;
            waterCoil.LiquidSideNominalConvect =
                waterCoil.UACoil * (waterCoil.RatioAirSideToWaterSideConvect + 1) / waterCoil.RatioAirSideToWaterSideConvect;
            waterCoil.AirSideNominalConvect = waterCoil.RatioAirSideToWaterSideConvect * waterCoil.LiquidSideNominalConvect;
        } else {
            state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum) = false;
        }

        waterCoil.MaxWaterMassFlowRate = rho * waterCoil.MaxWaterVolFlowRate;

        PlantUtilities::InitComponentNodes(state, 0.0, waterCoil.MaxWaterMassFlowRate, waterCoil.WaterInletNodeNum, waterCoil.WaterOutletNodeNum);

        // effective fin diameter for detailed flat fin coil
        if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
            waterCoil.EffectiveFinDiam =
                std::sqrt(4.0 * waterCoil.FinDiam * waterCoil.CoilDepth / (Constant::Pi * waterCoil.NumOfTubeRows * waterCoil.NumOfTubesPerRow));

            //   calculate fixed geometric parameters of the coil:
            //   Total Area
            waterCoil.TotCoilOutsideSurfArea = waterCoil.TubeOutsideSurfArea + waterCoil.FinSurfArea;
            //   Effective Tube Inside Diameter - the model assumes that the coil
            //   can be simulated as a tube with an equivalent hydraulic diameter.
            waterCoil.CoilEffectiveInsideDiam = 4.0 * waterCoil.MinAirFlowArea * waterCoil.CoilDepth / waterCoil.TotCoilOutsideSurfArea;
            //   Ratio of tube outside diameter to effective fin diameter should always
            //   be less than 1
            TubeToFinDiamRatio = waterCoil.TubeOutsideDiam / waterCoil.EffectiveFinDiam;
            if (TubeToFinDiamRatio > 1.0) {
                ShowWarningError(state, std::format("InitWaterCoil: Detailed Flat Fin Coil, TubetoFinDiamRatio > 1.0, [{:.4f}]", TubeToFinDiamRatio));
                // reset tube depth spacing and recalc dependent parameters
                waterCoil.TubeDepthSpacing *= (pow_2(TubeToFinDiamRatio) + 0.1);
                waterCoil.CoilDepth = waterCoil.TubeDepthSpacing * waterCoil.NumOfTubeRows;
                waterCoil.EffectiveFinDiam =
                    std::sqrt(4.0 * waterCoil.FinDiam * waterCoil.CoilDepth / (Constant::Pi * waterCoil.NumOfTubeRows * waterCoil.NumOfTubesPerRow));
                waterCoil.CoilEffectiveInsideDiam = 4.0 * waterCoil.MinAirFlowArea * waterCoil.CoilDepth / waterCoil.TotCoilOutsideSurfArea;
                TubeToFinDiamRatio = waterCoil.TubeOutsideDiam / waterCoil.EffectiveFinDiam;
                ShowContinueError(state, std::format("  Resetting tube depth spacing to {:.4f} meters", waterCoil.TubeDepthSpacing));
                ShowContinueError(state, std::format("  Resetting coil depth to {:.4f} meters", waterCoil.CoilDepth));
            }

            CalcDryFinEffCoef(state, TubeToFinDiamRatio, state.dataWaterCoils->CoefSeries);

            waterCoil.DryFinEfficncyCoef = state.dataWaterCoils->CoefSeries;

            FinDiamVar = 0.5 * (waterCoil.EffectiveFinDiam - waterCoil.TubeOutsideDiam);

            waterCoil.GeometryCoef1 = 0.159 * std::pow(waterCoil.FinThickness / waterCoil.CoilEffectiveInsideDiam, -0.065) *
                                      std::pow(waterCoil.FinThickness / FinDiamVar, 0.141);
            waterCoil.GeometryCoef2 = -0.323 * std::pow(waterCoil.FinSpacing / FinDiamVar, 0.049) *
                                      std::pow(waterCoil.EffectiveFinDiam / waterCoil.TubeDepthSpacing, 0.549) *
                                      std::pow(waterCoil.FinThickness / waterCoil.FinSpacing, -0.028);

            // Set some initial values for simulation
            waterCoil.SatEnthlCurveConstCoef = -10.57;
            waterCoil.SatEnthlCurveSlope = 3.3867;
            waterCoil.EnthVsTempCurveAppxSlope = 3.3867;
            waterCoil.EnthVsTempCurveConst = -10.57;
            // Set Saved Values to Zero
            waterCoil.SurfAreaWetSaved = 0.0;
            waterCoil.MeanWaterTempSaved = 0.0;
            waterCoil.InWaterTempSaved = 0.0;
            waterCoil.OutWaterTempSaved = 0.0;

        } // End the Detailed Flat Fin Coil Initialization

        // Calculation for Cooling Coil, The part between the '@@@' are design condition
        // and are calculated only once to calculate standard values for UAs and other physical parameters of
        // the cooling coil.
        // Basic Idea for UA:  Heat Transfer= UAenthalpybased*(Delta enthalpy), this is a necessity since the
        // coil may be Wet or Dry or Partially Wet-Dry, so latent effects are accounted for in this model while
        // calculating the UA. A fictitious specific heat is also defined to calculate the conventional UA.
        // On the air side, enthalpy capacity rate is the air mass flow rate,while on water side it is
        // enthalpy of saturated air at water temperature.
        //@@@ DESIGN CONDITION BEGIN HERE @@@

        // Check for zero design cooling capacity as specified by coil design inputs
        if (state.dataWaterCoils->MyCoilDesignFlag(CoilNum) && (waterCoil.WaterCoilModel == CoilModel::CoolingSimple) &&
            (waterCoil.DesAirVolFlowRate > 0.0) && (waterCoil.MaxWaterMassFlowRate > 0.0)) {

            DesInletAirEnth = PsyHFnTdbW(waterCoil.DesInletAirTemp, waterCoil.DesInletAirHumRat);
            DesOutletAirEnth = PsyHFnTdbW(waterCoil.DesOutletAirTemp, waterCoil.DesOutletAirHumRat);
            DesSatEnthAtWaterInTemp =
                PsyHFnTdbW(waterCoil.DesInletWaterTemp, PsyWFnTdpPb(state, waterCoil.DesInletWaterTemp, state.dataEnvrn->StdBaroPress));
            // check for dry coil
            DesHumRatAtWaterInTemp = PsyWFnTdbH(state, waterCoil.DesInletWaterTemp, DesSatEnthAtWaterInTemp, RoutineName);
            if (DesHumRatAtWaterInTemp > waterCoil.DesOutletAirHumRat && waterCoil.DesOutletAirTemp > waterCoil.DesInletWaterTemp) {
                // if the design outlet air humrat is lower than the saturated air humrat at the design inlet water temp
                // and the design outlet air temperature is higher than the design inlet water temp (i.e, cooling possible),
                // move the design outlet air saturated enthalpy down (i.e., to Twaterin, Wair,out) to allow the coil to size.
                DesSatEnthAtWaterInTemp = PsyHFnTdbW(waterCoil.DesInletWaterTemp, waterCoil.DesOutletAirHumRat) - 0.0001;
            }
            if (DesOutletAirEnth >= DesInletAirEnth || waterCoil.DesInletWaterTemp >= waterCoil.DesInletAirTemp) {
                ShowWarningError(state, std::format("The design cooling capacity is zero for Coil:Cooling:Water {}", waterCoil.Name));
                ShowContinueError(state, "  The maximum water flow rate for this coil will be set to zero and the coil will do no cooling.");
                ShowContinueError(state,
                                  std::format("  Check the following coil design inputs for problems: Tair,in = {:.4f}", waterCoil.DesInletAirTemp));
                ShowContinueError(
                    state, std::format("                                                       Wair,in = {:.6f}", waterCoil.DesInletAirHumRat));
                ShowContinueError(
                    state, std::format("                                                       Twater,in = {:.4f}", waterCoil.DesInletWaterTemp));
                ShowContinueError(
                    state, std::format("                                                       Tair,out = {:.4f}", waterCoil.DesOutletAirTemp));
                ShowContinueError(
                    state, std::format("                                                       Wair,out = {:.6f}", waterCoil.DesOutletAirHumRat));
                waterCoil.MaxWaterVolFlowRate = 0.0;
                waterCoil.MaxWaterMassFlowRate = 0.0;
            }
        }

        if (state.dataWaterCoils->MyCoilDesignFlag(CoilNum) && (waterCoil.WaterCoilModel == CoilModel::CoolingSimple) &&
            (waterCoil.DesAirVolFlowRate > 0.0) && (waterCoil.MaxWaterMassFlowRate > 0.0)) { // 'Cooling'

            state.dataWaterCoils->MyCoilDesignFlag(CoilNum) = false;
            state.dataWaterCoils->NoSatCurveIntersect = false;
            state.dataWaterCoils->BelowInletWaterTemp = false;
            state.dataWaterCoils->CBFTooLarge = false;
            state.dataWaterCoils->NoExitCondReset = false;
            for (Ipass = 1; Ipass <= 2; ++Ipass) {
                if (Ipass == 2) {
                    if (!state.dataWaterCoils->NoSatCurveIntersect && !state.dataWaterCoils->BelowInletWaterTemp &&
                        !state.dataWaterCoils->CBFTooLarge) {
                        goto Inlet_Conditions_Loop_exit; // coil UA calcs OK
                    } else {
                        ShowWarningError(state, std::format("In calculating the design coil UA for Coil:Cooling:Water {}", waterCoil.Name));
                        if (state.dataWaterCoils->NoSatCurveIntersect) {
                            ShowContinueError(state, "no apparatus dew-point can be found for the initial entering and leaving conditions;");
                        }
                        if (state.dataWaterCoils->BelowInletWaterTemp) {
                            ShowContinueError(state, "the apparatus dew-point is below the coil design inlet water temperature;");
                        }
                        if (state.dataWaterCoils->CBFTooLarge) {
                            ShowContinueError(state, "the coil bypass factor is unrealistically large;");
                        }
                        if (!state.dataWaterCoils->NoExitCondReset) {
                            ShowContinueError(state, "the coil outlet design conditions will be changed to correct the problem.");
                        }
                        ShowContinueError(state, std::format("The initial design conditions are: Tair,in = {:.4f}", waterCoil.DesInletAirTemp));
                        ShowContinueError(state, std::format("                                   Wair,in = {:.6f}", waterCoil.DesInletAirHumRat));
                        ShowContinueError(state, std::format("                                   Twater,in = {:.4f}", waterCoil.DesInletWaterTemp));
                        ShowContinueError(state, std::format("                                   Tair,out = {:.4f}", waterCoil.DesOutletAirTemp));
                        ShowContinueError(state, std::format("                                   Wair,out = {:.6f}", waterCoil.DesOutletAirHumRat));
                        if (!state.dataWaterCoils->NoExitCondReset) {
                            ShowContinueError(state,
                                              std::format("The revised design conditions are: Tair,out = {:.4f}", state.dataWaterCoils->TOutNew));
                            ShowContinueError(state,
                                              std::format("                                   Wair,out = {:.6f}", state.dataWaterCoils->WOutNew));
                            waterCoil.DesOutletAirHumRat = state.dataWaterCoils->WOutNew;
                            waterCoil.DesOutletAirTemp = state.dataWaterCoils->TOutNew;
                            // update outlet air conditions used for sizing
                            std::string CompType;
                            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) {
                                CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterDetailed];
                            } else {
                                CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater];
                            }
                            ReportCoilSelection::setCoilLvgAirTemp(state, waterCoil.coilReportNum, state.dataWaterCoils->TOutNew);
                            ReportCoilSelection::setCoilLvgAirHumRat(state, waterCoil.coilReportNum, state.dataWaterCoils->WOutNew);
                            // end update outlet air conditions used for sizing
                        }
                    }
                }

                // Volume flow rate being converted to mass flow rate for water
                waterCoil.DesAirMassFlowRate = state.dataEnvrn->StdRhoAir * waterCoil.DesAirVolFlowRate;

                // Enthalpy of Air at Inlet design conditions
                DesInletAirEnth = PsyHFnTdbW(waterCoil.DesInletAirTemp, waterCoil.DesInletAirHumRat);

                // Enthalpy of Air at outlet at design conditions
                DesOutletAirEnth = PsyHFnTdbW(waterCoil.DesOutletAirTemp, waterCoil.DesOutletAirHumRat);

                // already calculated above and possibly reset if dry coil
                //        ! Enthalpy of Water at Inlet design conditions
                //        DesSatEnthAtWaterInTemp =PsyHFnTdbW(WaterCoil(CoilNum)%DesInletWaterTemp, &
                //                                             PsyWFnTdpPb(state, WaterCoil(CoilNum)%DesInletWaterTemp,StdBaroPress))

                // Total Coil Load from Inlet and Outlet Air States (which include fan heat as appropriate).
                waterCoil.DesTotWaterCoilLoad = waterCoil.DesAirMassFlowRate * (DesInletAirEnth - DesOutletAirEnth);

                // Enthalpy of Water at Inlet design conditions
                Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, waterCoil.DesInletWaterTemp, RoutineName);

                DesOutletWaterTemp = waterCoil.DesInletWaterTemp + waterCoil.DesTotWaterCoilLoad / (waterCoil.MaxWaterMassFlowRate * Cp);

                DesSatEnthAtWaterOutTemp = PsyHFnTdbW(DesOutletWaterTemp, PsyWFnTdpPb(state, DesOutletWaterTemp, state.dataEnvrn->StdBaroPress));
                DesEnthAtWaterOutTempAirInHumRat = PsyHFnTdbW(DesOutletWaterTemp, waterCoil.DesInletAirHumRat);
                DesEnthWaterOut = min(DesSatEnthAtWaterOutTemp, DesEnthAtWaterOutTempAirInHumRat);

                // dry coil test
                if (waterCoil.DesOutletAirHumRat < waterCoil.DesInletAirHumRat && DesHumRatAtWaterInTemp < waterCoil.DesInletAirHumRat) { // wet coil

                    // Calculations for BYPASS FACTOR at design conditions
                    // Calculate "slope" of temperature vs. humidity ratio between entering and leaving states
                    SlopeTempVsHumRatio = (waterCoil.DesInletAirTemp - waterCoil.DesOutletAirTemp) /
                                          max((waterCoil.DesInletAirHumRat - waterCoil.DesOutletAirHumRat), SmallNo);

                    // Initialize iteration parameters
                    DesAirTempApparatusDewPt = PsyTdpFnWPb(state, waterCoil.DesOutletAirHumRat, state.dataEnvrn->OutBaroPress);

                    // Iterating to calculate Apparatus Dew Point Temperature at Design Conditions
                    for (iter = 1; iter <= itmax; ++iter) {

                        // Calculate apparatus dewpoint and compare with predicted value
                        // using entering conditions and SlopeTempVsHumRatio
                        DesAirHumRatApparatusDewPt = PsyWFnTdpPb(state, DesAirTempApparatusDewPt, state.dataEnvrn->OutBaroPress);

                        // Initial Estimate for apparatus Dew Point Temperature
                        TempApparatusDewPtEstimate =
                            waterCoil.DesInletAirTemp - SlopeTempVsHumRatio * (waterCoil.DesInletAirHumRat - DesAirHumRatApparatusDewPt);

                        // Iterating to calculate Apparatus Dew Point Temperature at Design Condition
                        error = DesAirTempApparatusDewPt - TempApparatusDewPtEstimate;
                        General::Iterate(ResultX, 0.01, DesAirTempApparatusDewPt, error, X1, Y1, iter, icvg);
                        DesAirTempApparatusDewPt = ResultX;

                        // If converged, exit loop
                        if (icvg == 1) {
                            goto App_DewPoint_Loop1_exit;
                        }

                        // If not converged due to low Humidity Ratio approximate value at outlet conditions
                        if (iter == itmax) {
                            state.dataWaterCoils->NoSatCurveIntersect = true;
                            DesAirTempApparatusDewPt = PsyTdpFnWPb(state, waterCoil.DesOutletAirHumRat, state.dataEnvrn->OutBaroPress);
                            DesAirHumRatApparatusDewPt = PsyWFnTdpPb(state, DesAirTempApparatusDewPt, state.dataEnvrn->OutBaroPress);
                            goto App_DewPoint_Loop1_exit;
                        }

                        // End of Loop for Iteration
                    }
                App_DewPoint_Loop1_exit:;

                    // Air enthalpy at apparatus dew point at design conditions
                    DesAirApparatusDewPtEnth = PsyHFnTdbW(DesAirTempApparatusDewPt, DesAirHumRatApparatusDewPt);

                    // Calculate bypass factor from enthalpies calculated above.
                    DesBypassFactor = (DesOutletAirEnth - DesAirApparatusDewPtEnth) / (DesInletAirEnth - DesAirApparatusDewPtEnth);

                    // Check for bypass factor for unsuitable value. Note that bypass factor is never used in the coil calculation
                    if ((DesBypassFactor > 0.5) || (DesBypassFactor < 0.0)) {
                        state.dataWaterCoils->CBFTooLarge = true;
                        DesBypassFactor = 0.37;
                    }

                    if (DesEnthWaterOut > DesInletAirEnth) {
                        ShowWarningError(state, std::format("In calculating the design coil UA for Coil:Cooling:Water {}", waterCoil.Name));
                        ShowContinueError(state, "the outlet chilled water design enthalpy is greater than the inlet air design enthalpy.");
                        ShowContinueError(state,
                                          std::format("To correct this condition the design chilled water flow rate will be increased from {:.5f}",
                                                      waterCoil.MaxWaterVolFlowRate));
                        EnthCorrFrac = (DesEnthWaterOut - DesInletAirEnth) / (DesEnthWaterOut - DesSatEnthAtWaterInTemp);
                        waterCoil.MaxWaterVolFlowRate *= (1.0 + 2.0 * EnthCorrFrac);
                        ShowContinueError(state, std::format("to {:.5f} m3/s", waterCoil.MaxWaterVolFlowRate));
                        waterCoil.MaxWaterMassFlowRate = rho * waterCoil.MaxWaterVolFlowRate;
                        DesOutletWaterTemp = waterCoil.DesInletWaterTemp + waterCoil.DesTotWaterCoilLoad / (waterCoil.MaxWaterMassFlowRate * Cp);
                        DesSatEnthAtWaterOutTemp =
                            PsyHFnTdbW(DesOutletWaterTemp, PsyWFnTdpPb(state, DesOutletWaterTemp, state.dataEnvrn->StdBaroPress));
                        DesEnthAtWaterOutTempAirInHumRat = PsyHFnTdbW(DesOutletWaterTemp, waterCoil.DesInletAirHumRat);
                        DesEnthWaterOut = min(DesSatEnthAtWaterOutTemp, DesEnthAtWaterOutTempAirInHumRat);
                    }

                    // Determine air-side coefficient, UACoilExternal, assuming that the
                    // surface temperature is at the apparatus dewpoint temperature
                    if (DesAirApparatusDewPtEnth <= DesSatEnthAtWaterInTemp) {
                        state.dataWaterCoils->BelowInletWaterTemp = true;
                    }
                    if ((DesInletAirEnth - DesEnthWaterOut) > SmallNo && (DesOutletAirEnth - DesSatEnthAtWaterInTemp) > SmallNo) {
                        LogMeanEnthDiff = ((DesInletAirEnth - DesEnthWaterOut) - (DesOutletAirEnth - DesSatEnthAtWaterInTemp)) /
                                          std::log((DesInletAirEnth - DesEnthWaterOut) / (DesOutletAirEnth - DesSatEnthAtWaterInTemp));
                    } else {
                        LogMeanEnthDiff = 2000.0; // UA will be 1/2 the design coil load
                    }
                    DesUACoilExternalEnth = waterCoil.DesTotWaterCoilLoad / LogMeanEnthDiff;
                    waterCoil.UACoilExternal = DesUACoilExternalEnth * PsyCpAirFnW(waterCoil.DesInletAirHumRat);

                    if (Ipass == 1 && (state.dataWaterCoils->NoSatCurveIntersect || state.dataWaterCoils->CBFTooLarge ||
                                       state.dataWaterCoils->BelowInletWaterTemp)) {
                        // reset outlet conditions to 90% relative humidity at the same outlet enthalpy
                        state.dataWaterCoils->TOutNew = TdbFnHRhPb(state, DesOutletAirEnth, 0.9, state.dataEnvrn->StdBaroPress);
                        state.dataWaterCoils->WOutNew = PsyWFnTdbH(state, state.dataWaterCoils->TOutNew, DesOutletAirEnth);
                        if (state.dataWaterCoils->WOutNew >= waterCoil.DesInletAirHumRat ||
                            state.dataWaterCoils->TOutNew > waterCoil.DesOutletAirTemp) {
                            state.dataWaterCoils->NoExitCondReset = true;
                        }
                        goto Inlet_Conditions_Loop_loop;
                    }

                    waterCoil.UACoilInternal = waterCoil.UACoilExternal * 3.30;
                    // Overall heat transfer coefficient
                    waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);

                } else { // dry coil

                    if (DesOutletWaterTemp > waterCoil.DesInletAirTemp) {
                        ShowWarningError(state, std::format("In calculating the design coil UA for Coil:Cooling:Water {}", waterCoil.Name));
                        ShowContinueError(state, "the outlet chilled water design temperature is greater than the inlet air design temperature.");
                        ShowContinueError(state,
                                          std::format("To correct this condition the design chilled water flow rate will be increased from {:.5f}",
                                                      waterCoil.MaxWaterVolFlowRate));
                        TempCorrFrac = (DesOutletWaterTemp - waterCoil.DesInletAirTemp) / (DesOutletWaterTemp - waterCoil.DesInletWaterTemp);
                        waterCoil.MaxWaterVolFlowRate *= (1.0 + 2.0 * TempCorrFrac);
                        ShowContinueError(state, std::format("to {:.5f} m3/s", waterCoil.MaxWaterVolFlowRate));
                        waterCoil.MaxWaterMassFlowRate = rho * waterCoil.MaxWaterVolFlowRate;
                        DesOutletWaterTemp = waterCoil.DesInletWaterTemp + waterCoil.DesTotWaterCoilLoad / (waterCoil.MaxWaterMassFlowRate * Cp);
                    }

                    if ((waterCoil.DesInletAirTemp - DesOutletWaterTemp) > SmallNo &&
                        (waterCoil.DesOutletAirTemp - waterCoil.DesInletWaterTemp) > SmallNo) {
                        LogMeanTempDiff =
                            ((waterCoil.DesInletAirTemp - DesOutletWaterTemp) - (waterCoil.DesOutletAirTemp - waterCoil.DesInletWaterTemp)) /
                            std::log((waterCoil.DesInletAirTemp - DesOutletWaterTemp) / (waterCoil.DesOutletAirTemp - waterCoil.DesInletWaterTemp));
                        waterCoil.UACoilExternal = waterCoil.DesTotWaterCoilLoad / LogMeanTempDiff;
                    } else {
                        waterCoil.UACoilExternal = waterCoil.DesTotWaterCoilLoad / 2.0; // make the UA large
                    }
                    waterCoil.UACoilInternal = waterCoil.UACoilExternal * 3.30;
                    // Overall heat transfer coefficient
                    waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);
                    goto Inlet_Conditions_Loop_exit;
                }

            Inlet_Conditions_Loop_loop:;
            }
        Inlet_Conditions_Loop_exit:;

            // estimate the heat external transfer surface area using typical design over all U value
            waterCoil.TotCoilOutsideSurfArea = EstimateHEXSurfaceArea(state, CoilNum);
            // calculate internal and external "UA per external surface area"
            waterCoil.UACoilInternalPerUnitArea = waterCoil.UACoilInternal / waterCoil.TotCoilOutsideSurfArea;
            waterCoil.UAWetExtPerUnitArea = waterCoil.UACoilExternal / waterCoil.TotCoilOutsideSurfArea;
            // approximate the dry UA as 1.0 times wet UA
            waterCoil.UADryExtPerUnitArea = waterCoil.UAWetExtPerUnitArea;

            // Now use SolveRoot to "invert" the cooling coil model to obtain the UA given the specified design inlet and outlet conditions
            // Note that the UAs we have obtained so far are rough estimates that are the starting points for the the following iterative
            //   calculation of the actual UAs.
            waterCoil.InletAirTemp = waterCoil.DesInletAirTemp;
            waterCoil.InletAirHumRat = waterCoil.DesInletAirHumRat;
            waterCoil.InletWaterTemp = waterCoil.DesInletWaterTemp;
            waterCoil.InletWaterMassFlowRate = rho * waterCoil.MaxWaterVolFlowRate;
            waterCoil.InletAirMassFlowRate = waterCoil.DesAirMassFlowRate;
            // set the lower and upper limits on the UA
            UA0 = 0.1 * waterCoil.UACoilExternal;
            UA1 = 10.0 * waterCoil.UACoilExternal;

            // Invert the simple cooling coil model: given the design inlet conditions and the design load, find the design UA
            auto f = [&state, CoilNum](Real64 const UA) {
                HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
                Real64 PartLoadRatio = 1.0;
                auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
                waterCoil.UACoilExternal = UA;
                waterCoil.UACoilInternal = waterCoil.UACoilExternal * 3.3;
                waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);
                waterCoil.TotCoilOutsideSurfArea = EstimateHEXSurfaceArea(state, CoilNum);
                waterCoil.UACoilInternalPerUnitArea = waterCoil.UACoilInternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UAWetExtPerUnitArea = waterCoil.UACoilExternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UADryExtPerUnitArea = waterCoil.UAWetExtPerUnitArea;

                CoolingCoil(state, CoilNum, true, state.dataWaterCoils->DesignCalc, fanOp, PartLoadRatio);

                return (waterCoil.DesTotWaterCoilLoad - waterCoil.TotWaterCoolingCoilRate) / waterCoil.DesTotWaterCoilLoad;
            };

            int SolFlag;

            UA = General::SolveRoot2(state, 0.001, 500, SolFlag, f, UA0, UA1, state.dataWaterCoils->WaterCoil(CoilNum).solveRootStats);

            // if the numerical inversion failed, issue error messages.
            if (SolFlag == General::SOLVEROOT_ERROR_ITER) {
                ShowSevereError(state, std::format("Calculation of cooling coil design UA failed for coil {}", waterCoil.Name));
                ShowContinueError(state, "  Iteration limit exceeded in calculating coil UA");
                waterCoil.UACoilExternal = UA0 * 10.0;
                waterCoil.UACoilInternal = waterCoil.UACoilExternal * 3.3;
                waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);
                waterCoil.TotCoilOutsideSurfArea = EstimateHEXSurfaceArea(state, CoilNum);
                waterCoil.UACoilInternalPerUnitArea = waterCoil.UACoilInternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UAWetExtPerUnitArea = waterCoil.UACoilExternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UADryExtPerUnitArea = waterCoil.UAWetExtPerUnitArea;
                ShowContinueError(state, std::format(" Coil design UA set to {:.6f} [W/C]", waterCoil.UACoilTotal));
            } else if (SolFlag == General::SOLVEROOT_ERROR_INIT) {
                ShowSevereError(state, std::format("Calculation of cooling coil design UA failed for coil {}", waterCoil.Name));
                ShowContinueError(state, "  Bad starting values for UA");
                waterCoil.UACoilExternal = UA0 * 10.0;
                waterCoil.UACoilInternal = waterCoil.UACoilExternal * 3.3;
                waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);
                waterCoil.TotCoilOutsideSurfArea = EstimateHEXSurfaceArea(state, CoilNum);
                waterCoil.UACoilInternalPerUnitArea = waterCoil.UACoilInternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UAWetExtPerUnitArea = waterCoil.UACoilExternal / waterCoil.TotCoilOutsideSurfArea;
                waterCoil.UADryExtPerUnitArea = waterCoil.UAWetExtPerUnitArea;
                ShowContinueError(state, std::format(" Coil design UA set to {:.6f} [W/C]", waterCoil.UACoilTotal));
            }

            // cooling coil surface area
            state.dataWaterCoils->SurfaceArea = waterCoil.TotCoilOutsideSurfArea;

            // cooling coil overall UA value
            state.dataWaterCoils->UATotal = waterCoil.UACoilTotal;

            // save the design internal and external UAs
            waterCoil.UACoilExternalDes = waterCoil.UACoilExternal;
            waterCoil.UACoilInternalDes = waterCoil.UACoilInternal;
        }

        //@@@@ DESIGN CONDITION END HERE @@@@

        // Calculate rated Total, latent, sensible capacity, SHR, effectiveness
        if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
            waterCoil.InletAirTemp = 16.6;
            waterCoil.InletAirHumRat = PsyWFnTdbRhPb(state, 16.6, 0.5, state.dataEnvrn->StdBaroPress, RoutineName);
            waterCoil.InletWaterTemp = 82.2;
        } else {
            waterCoil.InletAirTemp = 26.67;
            waterCoil.InletAirHumRat = PsyWFnTdbTwbPb(state, 26.67, 19.44, state.dataEnvrn->StdBaroPress, RoutineName);
            waterCoil.InletWaterTemp = 6.67;
        }
        waterCoil.InletAirEnthalpy = PsyHFnTdbW(waterCoil.InletAirTemp, waterCoil.InletAirHumRat);
        waterCoil.InletWaterMassFlowRate = waterCoil.MaxWaterMassFlowRate;
        waterCoil.InletAirMassFlowRate = state.dataEnvrn->StdRhoAir * waterCoil.DesAirVolFlowRate;
        CapacitanceAir = waterCoil.InletAirMassFlowRate * PsyCpAirFnW(waterCoil.InletAirHumRat);

        Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, waterCoil.InletWaterTemp, RoutineName);

        state.dataWaterCoils->CapacitanceWater = waterCoil.InletWaterMassFlowRate * Cp;
        state.dataWaterCoils->CMin = min(CapacitanceAir, state.dataWaterCoils->CapacitanceWater);
        if (state.dataWaterCoils->CMin > 0.0) {
            if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                CoolingCoil(state, CoilNum, FirstHVACIteration, state.dataWaterCoils->DesignCalc, HVAC::FanOp::Continuous, 1.0);
                state.dataWaterCoils->CoilEffectiveness = (waterCoil.InletAirTemp - waterCoil.OutletAirTemp) /
                                                          (waterCoil.InletAirTemp - waterCoil.InletWaterTemp) *
                                                          (CapacitanceAir / state.dataWaterCoils->CMin);
                state.dataWaterCoils->RatedLatentCapacity = waterCoil.TotWaterCoolingCoilRate - waterCoil.SenWaterCoolingCoilRate;
                state.dataWaterCoils->RatedSHR = waterCoil.SenWaterCoolingCoilRate / waterCoil.TotWaterCoolingCoilRate;
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                CalcDetailFlatFinCoolingCoil(state, CoilNum, state.dataWaterCoils->DesignCalc, HVAC::FanOp::Continuous, 1.0);
                state.dataWaterCoils->CoilEffectiveness = (waterCoil.InletAirTemp - waterCoil.OutletAirTemp) /
                                                          (waterCoil.InletAirTemp - waterCoil.InletWaterTemp) *
                                                          (CapacitanceAir / state.dataWaterCoils->CMin);
                state.dataWaterCoils->RatedLatentCapacity = waterCoil.TotWaterCoolingCoilRate - waterCoil.SenWaterCoolingCoilRate;
                state.dataWaterCoils->RatedSHR = waterCoil.SenWaterCoolingCoilRate / waterCoil.TotWaterCoolingCoilRate;
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                CalcSimpleHeatingCoil(state, CoilNum, HVAC::FanOp::Continuous, 1.0, state.dataWaterCoils->DesignCalc);
                state.dataWaterCoils->CoilEffectiveness = (waterCoil.OutletAirTemp - waterCoil.InletAirTemp) /
                                                          (waterCoil.InletWaterTemp - waterCoil.InletAirTemp) *
                                                          (CapacitanceAir / state.dataWaterCoils->CMin);
            }
        } else {
            state.dataWaterCoils->CoilEffectiveness = 0.0;
            waterCoil.TotWaterHeatingCoilRate = 0.0;
            waterCoil.TotWaterCoolingCoilRate = 0.0;
            waterCoil.SenWaterCoolingCoilRate = 0.0;
            state.dataWaterCoils->RatedLatentCapacity = 0.0;
            state.dataWaterCoils->RatedSHR = 0.0;
        }
        state.dataWaterCoils->MyEnvrnFlag(CoilNum) = false;

    } // End If for the Begin Environment initializations

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataWaterCoils->MyEnvrnFlag(CoilNum) = true;
    }

    if (!state.dataGlobal->DoingSizing) {
        if (state.dataWaterCoils->MyCoilReportFlag(CoilNum)) {
            // create predefined report entries
            state.dataWaterCoils->MyCoilReportFlag(CoilNum) = false;
            switch (waterCoil.WaterCoilType) {
            case DataPlant::PlantEquipmentType::CoilWaterSimpleHeating: {
                if (state.dataWaterCoils->RptCoilHeaderFlag(1)) {
                    print(state.files.eio, "{}", "! <Water Heating Coil Capacity Information>,Component Type,Name,Nominal Total Capacity {W}\n");
                    state.dataWaterCoils->RptCoilHeaderFlag(1) = false;
                }
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilType, waterCoil.Name, "Coil:Heating:Water");
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchHeatCoilDesCap, waterCoil.Name, waterCoil.DesWaterHeatingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchHeatCoilNomCap, waterCoil.Name, waterCoil.TotWaterHeatingCoilRate);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilNomEff, waterCoil.Name, "-");
                OutputReportPredefined::addFootNoteSubTable(
                    state,
                    state.dataOutRptPredefined->pdstHeatCoil,
                    "Nominal values are gross at rated conditions, i.e., the supply air fan heat and electric power NOT accounted for.");
                print(state.files.eio,
                      "{},{},{:.2f}\n",
                      "Water Heating Coil Capacity Information,Coil:Heating:Water",
                      waterCoil.Name,
                      waterCoil.TotWaterHeatingCoilRate);
                ReportCoilSelection::setCoilAirFlow(state, waterCoil.coilReportNum, waterCoil.DesAirVolFlowRate, waterCoil.RequestingAutoSize);
                ReportCoilSelection::setCoilWaterHeaterCapacityNodeNums(state,
                                                                        waterCoil.coilReportNum,
                                                                        waterCoil.DesWaterHeatingCoilRate,
                                                                        waterCoil.RequestingAutoSize,
                                                                        waterCoil.WaterInletNodeNum,
                                                                        waterCoil.WaterOutletNodeNum,
                                                                        waterCoil.WaterPlantLoc.loopNum); // coil report
                break;
            }
            case DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling: {
                if (state.dataWaterCoils->RptCoilHeaderFlag(2)) {
                    print(state.files.eio,
                          "{}\n",
                          "! <Water Cooling Coil Capacity Information>,Component Type,Name,Nominal Total "
                          "Capacity {W},Nominal Sensible Capacity {W},Nominal Latent Capacity {W},Nominal "
                          "Sensible Heat Ratio");
                    state.dataWaterCoils->RptCoilHeaderFlag(2) = false;
                }
                state.dataWaterCoils->RatedLatentCapacity = waterCoil.TotWaterCoolingCoilRate - waterCoil.SenWaterCoolingCoilRate;
                state.dataWaterCoils->RatedSHR = General::SafeDivide(waterCoil.SenWaterCoolingCoilRate, waterCoil.TotWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilType, waterCoil.Name, "Coil:Cooling:Water:DetailedGeometry");
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilDesCap, waterCoil.Name, waterCoil.DesWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilTotCap, waterCoil.Name, waterCoil.TotWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilSensCap, waterCoil.Name, waterCoil.SenWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilLatCap, waterCoil.Name, state.dataWaterCoils->RatedLatentCapacity);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilSHR, waterCoil.Name, state.dataWaterCoils->RatedSHR);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilNomEff, waterCoil.Name, "-");
                OutputReportPredefined::addFootNoteSubTable(
                    state,
                    state.dataOutRptPredefined->pdstCoolCoil,
                    "Nominal values are gross at rated conditions, i.e., the supply air fan heat and electric power NOT accounted for.");
                print(state.files.eio,
                      "{},{},{:.2f},{:.2f},{:.2f},{:.2f}\n",
                      "Water Cooling Coil Capacity Information,Coil:Cooling:Water:DetailedGeometry",
                      waterCoil.Name,
                      waterCoil.TotWaterCoolingCoilRate,
                      waterCoil.SenWaterCoolingCoilRate,
                      state.dataWaterCoils->RatedLatentCapacity,
                      state.dataWaterCoils->RatedSHR);
                ReportCoilSelection::setCoilAirFlow(state,
                                                    waterCoil.coilReportNum,
                                                    waterCoil.DesAirVolFlowRate,
                                                    waterCoil.RequestingAutoSize); // Coil Report
                ReportCoilSelection::setCoilWaterCoolingCapacity(state,
                                                                 waterCoil.coilReportNum,
                                                                 waterCoil.DesWaterCoolingCoilRate,
                                                                 waterCoil.RequestingAutoSize,
                                                                 waterCoil.WaterInletNodeNum,
                                                                 waterCoil.WaterOutletNodeNum,
                                                                 waterCoil.WaterPlantLoc.loopNum); // Coil Report
                break;
            }
            case DataPlant::PlantEquipmentType::CoilWaterCooling: {
                if (state.dataWaterCoils->RptCoilHeaderFlag(2)) {
                    print(state.files.eio,
                          "{}\n",
                          "! <Water Cooling Coil Capacity Information>,Component Type,Name,Nominal Total "
                          "Capacity {W},Nominal Sensible Capacity {W},Nominal Latent Capacity {W},Nominal "
                          "Sensible Heat Ratio, Nominal Coil UA Value {W/C}, Nominal Coil Surface Area {m2}");
                    state.dataWaterCoils->RptCoilHeaderFlag(2) = false;
                }
                state.dataWaterCoils->RatedLatentCapacity = waterCoil.TotWaterCoolingCoilRate - waterCoil.SenWaterCoolingCoilRate;
                state.dataWaterCoils->RatedSHR = General::SafeDivide(waterCoil.SenWaterCoolingCoilRate, waterCoil.TotWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilType, waterCoil.Name, "Coil:Cooling:Water");
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilDesCap, waterCoil.Name, waterCoil.DesWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilTotCap, waterCoil.Name, waterCoil.TotWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilSensCap, waterCoil.Name, waterCoil.SenWaterCoolingCoilRate);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilLatCap, waterCoil.Name, state.dataWaterCoils->RatedLatentCapacity);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilSHR, waterCoil.Name, state.dataWaterCoils->RatedSHR);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilNomEff, waterCoil.Name, "-");
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilUATotal, waterCoil.Name, waterCoil.UACoilTotal);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilArea, waterCoil.Name, waterCoil.TotCoilOutsideSurfArea);
                OutputReportPredefined::addFootNoteSubTable(
                    state,
                    state.dataOutRptPredefined->pdstCoolCoil,
                    "Nominal values are gross at rated conditions, i.e., the supply air fan heat and electric power NOT accounted for.");
                print(state.files.eio,
                      "{},{},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f}\n",
                      "Water Cooling Coil Capacity Information,Coil:Cooling:Water",
                      waterCoil.Name,
                      waterCoil.TotWaterCoolingCoilRate,
                      waterCoil.SenWaterCoolingCoilRate,
                      state.dataWaterCoils->RatedLatentCapacity,
                      state.dataWaterCoils->RatedSHR,
                      state.dataWaterCoils->UATotal,
                      state.dataWaterCoils->SurfaceArea);
                ReportCoilSelection::setCoilAirFlow(state,
                                                    waterCoil.coilReportNum,
                                                    waterCoil.DesAirVolFlowRate,
                                                    waterCoil.RequestingAutoSize); // Coil Report
                ReportCoilSelection::setCoilWaterCoolingCapacity(state,
                                                                 waterCoil.coilReportNum,
                                                                 waterCoil.DesWaterCoolingCoilRate,
                                                                 waterCoil.RequestingAutoSize,
                                                                 waterCoil.WaterInletNodeNum,
                                                                 waterCoil.WaterOutletNodeNum,
                                                                 waterCoil.WaterPlantLoc.loopNum); // Coil Report
                break;
            }
            default:
                break;
            }
            if (waterCoil.DesWaterCoolingCoilRate <= 0.0) {
                waterCoil.DesWaterCoolingCoilRate = waterCoil.TotWaterCoolingCoilRate;
            }
            if (waterCoil.DesWaterHeatingCoilRate <= 0.0) {
                waterCoil.DesWaterHeatingCoilRate = waterCoil.TotWaterHeatingCoilRate;
            }

            // call coil model with everything set at rating point
            waterCoil.InletAirMassFlowRate = waterCoil.DesAirMassFlowRate;
            waterCoil.InletAirTemp = waterCoil.DesInletAirTemp;
            waterCoil.InletAirHumRat = waterCoil.DesInletAirHumRat; // fixed in sizing routine
            waterCoil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(waterCoil.DesInletAirTemp, waterCoil.DesInletAirHumRat);
            Real64 DesInletWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, waterCoil.DesInletAirTemp, waterCoil.DesInletAirHumRat, DataEnvironment::StdPressureSeaLevel, "InitWaterCoils");
            waterCoil.InletWaterMassFlowRate = waterCoil.MaxWaterMassFlowRate;
            waterCoil.InletWaterTemp = waterCoil.DesInletWaterTemp;
            Real64 cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, waterCoil.DesInletWaterTemp, "InitWaterCoil");
            waterCoil.InletWaterEnthalpy = cp * waterCoil.InletWaterTemp;

            waterCoil.UACoilVariable = waterCoil.UACoil;
            waterCoil.FaultyCoilFoulingFactor = 0.0;
            Real64 holdOutBaroPress = state.dataEnvrn->OutBaroPress;
            state.dataEnvrn->OutBaroPress = DataEnvironment::StdPressureSeaLevel; // assume rating is for sea level.
            CalcAdjustedCoilUA(state, CoilNum);

            std::string coilTypeName(" ");
            // calculate coil sim model at rating point, full load, continuous fan
            if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                CalcDetailFlatFinCoolingCoil(state, CoilNum, state.dataWaterCoils->SimCalc, HVAC::FanOp::Continuous, 1.0);
                coilTypeName = "Coil:Cooling:Water:DetailedGeometry";
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                CoolingCoil(state, CoilNum, FirstHVACIteration, state.dataWaterCoils->SimCalc, HVAC::FanOp::Continuous, 1.0);
                coilTypeName = "Coil:Cooling:Water";
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                CalcSimpleHeatingCoil(state, CoilNum, HVAC::FanOp::Continuous, 1.0, state.dataWaterCoils->SimCalc);
                coilTypeName = "Coil:Heating:Water";
            }
            ReportCoilSelection::setCoilEqNum(
                state, waterCoil.coilReportNum, state.dataSize->CurSysNum, state.dataSize->CurOASysNum, state.dataSize->CurZoneEqNum);

            // coil outlets
            Real64 RatedOutletWetBulb(0.0);
            RatedOutletWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, waterCoil.OutletAirTemp, waterCoil.OutletAirHumRat, DataEnvironment::StdPressureSeaLevel, "InitWaterCoil");

            // call set routine in coil report
            if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling ||
                waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                ReportCoilSelection::setRatedCoilConditions(state,
                                                            waterCoil.coilReportNum,
                                                            waterCoil.TotWaterCoolingCoilRate, // this is the report variable
                                                            waterCoil.SenWaterCoolingCoilRate, // this is the report variable
                                                            waterCoil.InletAirMassFlowRate,
                                                            waterCoil.InletAirTemp,
                                                            waterCoil.InletAirHumRat,
                                                            DesInletWetBulb,
                                                            waterCoil.OutletAirTemp,
                                                            waterCoil.OutletAirHumRat,
                                                            RatedOutletWetBulb,
                                                            -999.0,
                                                            -999.0,
                                                            -999.0,
                                                            -999.0); // coil effectiveness
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                ReportCoilSelection::setRatedCoilConditions(state,
                                                            waterCoil.coilReportNum,
                                                            waterCoil.TotWaterHeatingCoilRate, // this is the report variable
                                                            waterCoil.TotWaterHeatingCoilRate, // this is the report variable
                                                            waterCoil.InletAirMassFlowRate,
                                                            waterCoil.InletAirTemp,
                                                            waterCoil.InletAirHumRat,
                                                            DesInletWetBulb,
                                                            waterCoil.OutletAirTemp,
                                                            waterCoil.OutletAirHumRat,
                                                            RatedOutletWetBulb,
                                                            -999.0,
                                                            -999.0,
                                                            -999.0,
                                                            -999.0); // coil effectiveness
            }
            // now replace the outdoor air conditions set above for one time rating point calc
            state.dataEnvrn->OutBaroPress = holdOutBaroPress;
        }
    }

    // Do the Begin Day initializations
    // NONE

    // Do the begin HVAC time step initializations
    // NONE

    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.
    // First set the conditions for the air into the coil model
    AirInletNode = waterCoil.AirInletNodeNum;
    WaterInletNode = waterCoil.WaterInletNodeNum;
    auto const &airInletNode = state.dataLoopNodes->Node(AirInletNode);
    waterCoil.InletAirMassFlowRate = airInletNode.MassFlowRate;
    waterCoil.InletAirTemp = airInletNode.Temp;
    waterCoil.InletAirHumRat = airInletNode.HumRat;
    waterCoil.InletAirEnthalpy = airInletNode.Enthalpy;

    auto const &waterInletNode = state.dataLoopNodes->Node(WaterInletNode);
    waterCoil.InletWaterMassFlowRate = waterInletNode.MassFlowRate;
    waterCoil.InletWaterTemp = waterInletNode.Temp;
    waterCoil.InletWaterEnthalpy = waterInletNode.Enthalpy;

    waterCoil.UACoilVariable = waterCoil.UACoil;

    CalcAdjustedCoilUA(state, CoilNum);

    waterCoil.TotWaterHeatingCoilRate = 0.0;
    waterCoil.TotWaterCoolingCoilRate = 0.0;
    waterCoil.SenWaterCoolingCoilRate = 0.0;
}

// refactor coilUA adjustment into separate routine, for use with rating calc
void CalcAdjustedCoilUA(EnergyPlusData &state, int const CoilNum)
{
    // Pull these precalc routines out of big init routine
    // modify the coil UA based on model in Wetter 1999
    Real64 x_a;                  // result of Eq.70 in Wetter 1999
    Real64 x_w;                  // result of Eq.72 in Wetter 1999
    Real64 AirConvectTerm;       // result of Eq.71 in Wetter 1999
    Real64 WaterConvectTerm;     // result of Eq.73 in Wetter 1999
    Real64 WaterConvSensitivity; // "s" in Wetter 1999, temperature sensitivity in water side convection

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    // Coil:Heating:Water
    if ((waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) &&
        (!(state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum)))) { // update Coil UA based on inlet mass flows and temps
        x_a = 1.0 + 4.769E-3 * (waterCoil.InletAirTemp - waterCoil.DesInletAirTemp);
        if (waterCoil.DesAirMassFlowRate > 0.0) {
            AirConvectTerm = x_a * std::pow(waterCoil.InletAirMassFlowRate / waterCoil.DesAirMassFlowRate, 0.8) * waterCoil.AirSideNominalConvect;
        } else {
            AirConvectTerm = 0.0;
        }
        WaterConvSensitivity = 0.014 / (1.0 + 0.014 * waterCoil.DesInletWaterTemp);
        x_w = 1.0 + WaterConvSensitivity * (waterCoil.InletWaterTemp - waterCoil.DesInletWaterTemp);
        if (waterCoil.MaxWaterMassFlowRate > 0.0) {
            WaterConvectTerm =
                x_w * std::pow(waterCoil.InletWaterMassFlowRate / waterCoil.MaxWaterMassFlowRate, 0.85) * waterCoil.LiquidSideNominalConvect;
        } else {
            WaterConvectTerm = 0.0;
        }
        if ((AirConvectTerm > 0.0) && (WaterConvectTerm > 0.0)) {
            waterCoil.UACoilVariable = 1.0 / ((1.0 / WaterConvectTerm) + (1.0 / AirConvectTerm));
        } else {
            // use nominal UA since variable UA cannot be calculated
            waterCoil.UACoilVariable = waterCoil.UACoil;
        }

        // calculate the Faulty Coil Fouling (thermal insulance) Factor using fault information
        if (waterCoil.FaultyCoilFoulingFlag &&
            // The fault shouldn't apply during sizing.
            (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation) &&
            // This was preexisting
            !(state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum))) {
            // Store original value
            waterCoil.OriginalUACoilVariable = waterCoil.UACoilVariable;

            int FaultIndex = waterCoil.FaultyCoilFoulingIndex;
            FaultsManager::FaultPropertiesFoulingCoil &fouling = state.dataFaultsMgr->FouledCoils(FaultIndex);
            Real64 FaultFrac = fouling.FaultFraction(state);

            if (fouling.FoulingInputMethod == FaultsManager::FouledCoil::UARated) {
                // 1/UA' = Frac * (1/UAFouled) + (1-Frac) / UA
                waterCoil.UACoilVariable = 1 / (FaultFrac / (fouling.UAFouled) + (1 - FaultFrac) / waterCoil.UACoilVariable);
            } else {
                // R' = R + Rfoul
                // Rfoul = r_air/A_air + r_water/A_water (FoulingFactor = thermal insulance [K/W, A] = Area [m2], r=fouling factor [m2.K/W]
                Real64 FoulingFactor = FaultFrac * (fouling.Rfw / (fouling.Aratio * fouling.Aout) + fouling.Rfa / fouling.Aout);
                waterCoil.UACoilVariable = 1.0 / ((1.0 / waterCoil.UACoilVariable) + FoulingFactor);
            }

            // Do not allow improving coil performance
            waterCoil.UACoilVariable = min(waterCoil.UACoilVariable, waterCoil.OriginalUACoilVariable);

            // Only for reporting purposes
            waterCoil.FaultyCoilFoulingFactor = (1.0 / waterCoil.UACoilVariable) - (1.0 / waterCoil.OriginalUACoilVariable);
        } else {
            waterCoil.FaultyCoilFoulingFactor = 0;
        }
    }

    // Coil:Cooling:Water
    // update Coil UA based on inlet mass flows and temps
    if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling && (!state.dataWaterCoils->MyCoilDesignFlag(CoilNum))) {
        if (waterCoil.DesAirMassFlowRate > 0.0) {
            x_a = 1.0 + 4.769E-3 * (waterCoil.InletAirTemp - waterCoil.DesInletAirTemp);
            waterCoil.UACoilExternal =
                x_a * std::pow(waterCoil.InletAirMassFlowRate / waterCoil.DesAirMassFlowRate, 0.8) * waterCoil.UACoilExternalDes;
        } else {
            waterCoil.UACoilExternal = waterCoil.UACoilExternalDes;
        }

        if (waterCoil.MaxWaterMassFlowRate > 0.0) {
            WaterConvSensitivity = 0.014 / (1.0 + 0.014 * waterCoil.DesInletWaterTemp);
            x_w = 1.0 + WaterConvSensitivity * (waterCoil.InletWaterTemp - waterCoil.DesInletWaterTemp);
            waterCoil.UACoilInternal =
                x_w * std::pow(waterCoil.InletWaterMassFlowRate / waterCoil.MaxWaterMassFlowRate, 0.85) * waterCoil.UACoilInternalDes;
        } else {
            waterCoil.UACoilInternal = waterCoil.UACoilInternalDes;
        }

        if (!(waterCoil.UACoilInternal > 0.0 && waterCoil.UACoilExternal > 0.0)) {
            waterCoil.UACoilInternal = waterCoil.UACoilInternalDes;
            waterCoil.UACoilExternal = waterCoil.UACoilExternalDes;
        }

        // If Fouling
        if (waterCoil.FaultyCoilFoulingFlag &&
            // The fault shouldn't apply during sizing.
            (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation) &&
            // This was preexisting
            !(state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum))) {
            // Store original value
            // This is really UACoilTotal technically, but I don't see the point of declaring another Real on the struct just for that
            waterCoil.OriginalUACoilVariable = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);

            waterCoil.OriginalUACoilExternal = waterCoil.UACoilExternal;
            waterCoil.OriginalUACoilInternal = waterCoil.UACoilInternal;

            int FaultIndex = waterCoil.FaultyCoilFoulingIndex;

            FaultsManager::FaultPropertiesFoulingCoil &fouling = state.dataFaultsMgr->FouledCoils(FaultIndex);
            Real64 FaultFrac = fouling.FaultFraction(state);

            if (fouling.FoulingInputMethod == FaultsManager::FouledCoil::FoulingFactor) {
                // Adjust the External (air) UA and Internal (water) UA accordingly
                Real64 Rfoul_air = FaultFrac * (fouling.Rfa / fouling.Aout);
                Real64 Rfoul_water = FaultFrac * (fouling.Rfw / (fouling.Aratio * fouling.Aout));

                waterCoil.UACoilInternal = 1.0 / (1.0 / waterCoil.UACoilInternal + Rfoul_water);
                waterCoil.UACoilExternal = 1.0 / (1.0 / waterCoil.UACoilExternal + Rfoul_air);
                //
            } else { // iFouledCoil_UARated
                // FouledUARated is supposed to be the overall UA. So we need to split between Internal and External UAs

                // How should I split fouling between internal/external?
                // We can actually use the current ratio before fouling...
                // splitRatio = UACoilInternal/UACoilExternal
                // UACoilInternal = UACoilExternal * splitRatio

                // UACoilTotal = 1 / (1 / UACoilExternal + 1 / UACoilInternal)
                // UACoilTotal = 1 / (1 / UACoilExternal + 1 / (UACoilExernal * splitRatio))
                // UACoilTotal = UACoilExternal / (1 + 1 / splitRatio) = UACoilExternal  * splitRatio / (1 + splitRatio)
                // UACoilExternal = UACoilTotal * (1 + splitRatio) / splitRatio
                // UACoilInternal = UACoilTotal * (1 + splitRatio)

                // Adding in FaultFrac:
                // UACoilExternal = FaultFrac * [UAFouled * (1+splitRatio) / splitRatio] + (1-FaultFrac) * UACoilExternal
                // UACoilInternal = FaultFrac * [UAFouled * splitRatio] + (1-FaultFrac) * UACoilInternal

                Real64 splitRatio = waterCoil.UACoilInternal / waterCoil.UACoilExternal;

                waterCoil.UACoilExternal =
                    1.0 / ((FaultFrac * splitRatio) / ((1 + splitRatio) * fouling.UAFouled) + (1 - FaultFrac) / waterCoil.UACoilExternal);

                // WaterCoil(CoilNum).UACoilInternal =   1.0 /
                //( FaultFrac / ((1 + splitRatio) * fouling.UAFouled) +
                //(1-FaultFrac) / WaterCoil(CoilNum).UACoilInternal);

                waterCoil.UACoilInternal = splitRatio * waterCoil.UACoilExternal;
            }

            // Do not allow improving coil performance
            waterCoil.UACoilExternal = min(waterCoil.UACoilExternal, waterCoil.OriginalUACoilExternal);
            waterCoil.UACoilInternal = min(waterCoil.UACoilInternal, waterCoil.OriginalUACoilInternal);

            // Only for reporting purposes
            waterCoil.FaultyCoilFoulingFactor = (1.0 / waterCoil.UACoilExternal) - (1.0 / waterCoil.OriginalUACoilExternal) +
                                                (1.0 / waterCoil.UACoilInternal) - (1.0 / waterCoil.OriginalUACoilInternal);
        } else {
            waterCoil.FaultyCoilFoulingFactor = 0;
        }

        waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);

        waterCoil.UACoilInternalPerUnitArea = waterCoil.UACoilInternal / waterCoil.TotCoilOutsideSurfArea;
        waterCoil.UAWetExtPerUnitArea = waterCoil.UACoilExternal / waterCoil.TotCoilOutsideSurfArea;
        waterCoil.UADryExtPerUnitArea = waterCoil.UAWetExtPerUnitArea;
    }
}

void SizeWaterCoil(EnergyPlusData &state, int const CoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   November 2001
    //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Water Coil Components for which flow rates and UAs have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays and plant sizing data. UAs are
    // calculated by numerically inverting the individual coil calculation routines.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizeWaterCoil");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 rho;
    std::string CompType;                  // component type
    std::string SizingString;              // input field sizing description (e.g., Nominal Capacity)
    Real64 TempSize;                       // autosized value
    Real64 DesCoilWaterInTempSaved;        // coil water inlet temp used for error checking UA sizing
    Real64 DesCoilInletWaterTempUsed(0.0); // coil design inlet water temp for UA sizing only
    Real64 Cp;

    bool ErrorsFound = false;
    bool LoopErrorsFound = false;
    int PltSizCoolNum = 0;
    int PltSizHeatNum = 0;
    Real64 DesCoilAirFlow = 0.0;
    Real64 DesCoilExitTemp = 0.0;
    Real64 CpAirStd = PsyCpAirFnW(0.0);
    state.dataSize->DataCoilNum = CoilNum;

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    // cooling coils
    if (((waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
         (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) &&
        waterCoil.RequestingAutoSize) {
        // find the appropriate Plant Sizing object
        PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(
            state, "chilled water coil", waterCoil.Name, waterCoil.WaterInletNodeNum, waterCoil.WaterOutletNodeNum, LoopErrorsFound);
    }

    if (((waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
         (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling))) { // 'Cooling'

        if (waterCoil.UseDesignWaterDeltaTemp) {
            state.dataSize->DataWaterCoilSizCoolDeltaT = waterCoil.DesignWaterDeltaTemp;
        } else {
            if (PltSizCoolNum > 0) {
                state.dataSize->DataWaterCoilSizCoolDeltaT = state.dataSize->PlantSizData(PltSizCoolNum).DeltaT;
            }
        }

        if (PltSizCoolNum > 0) {

            // int FieldNum = 0;
            state.dataSize->DataPltSizCoolNum = PltSizCoolNum;
            state.dataSize->DataWaterLoopNum = waterCoil.WaterPlantLoc.loopNum;

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) {                  // 'DETAILED FLAT FIN'
                CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterDetailed]; // Coil:Cooling:Water:DetailedGeometry
            } else {
                CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater]; // Coil:Cooling:Water
            }

            bool bPRINT = false; // do not print this sizing request since the autosized value is needed and this input may not be autosized (we
                                 // should print this!)
            if (waterCoil.DesAirVolFlowRate == state.dataSize->DataFlowUsedForSizing) {
                TempSize = waterCoil.DesAirVolFlowRate; // represents parent object has hard-sized airflow
            } else {
                TempSize = DataSizing::AutoSize; // get the autosized air volume flow rate for use in other calculations
            }

            ErrorsFound = false;
            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string const &CompName = waterCoil.Name;
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            Real64 autoSizedValue = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
            waterCoil.InletAirMassFlowRate = state.dataEnvrn->StdRhoAir * autoSizedValue; // inlet air mass flow rate is the autosized value

            // Check if the air volume flow rate is defined in parent HVAC equipment and set water coil design air volume flow rate accordingly
            if (state.dataSize->CurZoneEqNum > 0) {
                auto const &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
                if (ZoneEqSizing.DesignSizeFromParent && waterCoil.DesAirVolFlowRate == autoSizedValue) {
                    state.dataSize->DataAirFlowUsedForSizing = ZoneEqSizing.AirVolFlow;
                    state.dataSize->DataFlowUsedForSizing = ZoneEqSizing.AirVolFlow;
                    waterCoil.DesAirVolFlowRate = DataSizing::AutoSize; // represents water coil being autosized
                } else {
                    state.dataSize->DataAirFlowUsedForSizing =
                        autoSizedValue; // many autosized inputs use the design (autosized) air volume flow rate, save this value
                    state.dataSize->DataFlowUsedForSizing = autoSizedValue;
                }
            } else {
                state.dataSize->DataAirFlowUsedForSizing =
                    autoSizedValue; // many autosized inputs use the design (autosized) air volume flow rate, save this value
                state.dataSize->DataFlowUsedForSizing = autoSizedValue;
            }

            if (state.dataSize->CurSysNum > 0 && state.dataSize->CurOASysNum == 0) {
                Real64 DesCoilExitHumRat(0.0); // fix coil sizing inconsistency
                DataSizing::GetCoilDesFlowT(state, state.dataSize->CurSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
                state.dataSize->DataAirFlowUsedForSizing = DesCoilAirFlow;
                state.dataSize->DataFlowUsedForSizing = DesCoilAirFlow;
                state.dataSize->DataDesOutletAirTemp = DesCoilExitTemp;
                state.dataSize->DataDesOutletAirHumRat = DesCoilExitHumRat; // need to test for dry coil but inlet conditions not yet known
            }

            // calculate pre-sizing data needed for specific functions (e.g., CoolingWaterDesAirInletTempSizing needs HRin and air flow)
            // these will be calculated again after other parameters are known
            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                TempSize = DataSizing::AutoSize;                          // coil report
            } else {
                TempSize = waterCoil.DesInletAirHumRat; // preserve input if entered
            }
            CoolingWaterDesAirInletHumRatSizer sizerCWDesInHumRat;
            sizerCWDesInHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            state.dataSize->DataDesInletAirHumRat = sizerCWDesInHumRat.size(state, TempSize, ErrorsFound);

            TempSize = DataSizing::AutoSize;
            CoolingCapacitySizer sizerCoolingCapacity;
            sizerCoolingCapacity.overrideSizingString(SizingString);
            sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            state.dataSize->DataCapacityUsedForSizing = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
            TempSize = waterCoil.MaxWaterVolFlowRate;
            CoolingWaterflowSizer sizerCWWaterflow;
            sizerCWWaterflow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            Real64 autoSizedCWFlow = sizerCWWaterflow.size(state, TempSize, ErrorsFound);
            // Check if the water flow rate is defined in parent HVAC equipment and set water coil design water flow rate accordingly
            if (state.dataSize->CurZoneEqNum > 0) {
                auto const &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
                if (ZoneEqSizing.DesignSizeFromParent) {
                    state.dataSize->DataWaterFlowUsedForSizing = ZoneEqSizing.MaxCWVolFlow;
                } else {
                    state.dataSize->DataWaterFlowUsedForSizing = autoSizedCWFlow;
                }
            } else {
                state.dataSize->DataWaterFlowUsedForSizing = autoSizedCWFlow;
            }
            // end pre-sizing data calculations

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                bPRINT = false; // do not print this sizing request since this coil does not have a design inlet air temp input field (we
                                // should print this!)
                TempSize = DataSizing::AutoSize; // not an input for this model
            } else {
                bPRINT = true;
                TempSize = waterCoil.DesInletAirTemp; // preserve input if entered
            }

            CoolingWaterDesAirInletTempSizer sizerCWDesInletAirTemp;
            sizerCWDesInletAirTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesInletAirTemp = sizerCWDesInletAirTemp.size(state, TempSize, ErrorsFound);
            state.dataSize->DataDesInletAirTemp = waterCoil.DesInletAirTemp;

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                bPRINT = false;                                           // no field for detailed water coil, should print to eio anyway
                TempSize = DataSizing::AutoSize;                          // coil report
            } else {
                bPRINT = true;
                TempSize = waterCoil.DesInletWaterTemp; // preserve input if entered
            }
            CoolingWaterDesWaterInletTempSizer sizerCWDesWaterInTemp;
            sizerCWDesWaterInTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesInletWaterTemp = sizerCWDesWaterInTemp.size(state, TempSize, ErrorsFound);

            if ((waterCoil.DesInletWaterTemp > state.dataSize->DataDesOutletAirTemp) && state.dataSize->DataDesOutletAirTemp > 0.0) {
                ShowWarningError(state, std::format("Invalid design inlet water temperature for {} = {}", CompType, CompName));
                ShowContinueError(state, std::format("...design inlet water temperature = {:.3f} C", waterCoil.DesInletWaterTemp));
                ShowContinueError(state, std::format("...design outlet air temperature = {:.3f} C", state.dataSize->DataDesOutletAirTemp));
                ShowContinueError(state, "...design inlet water temperature should be less than the design outlet air temperature");
                ShowContinueError(state, "...design inlet water temperature is set to the design outlet air temperature minus 5.0C");
                waterCoil.DesInletWaterTemp = state.dataSize->DataDesOutletAirTemp - 5.0;
            }

            if (state.dataSize->CurZoneEqNum > 0) { // zone equipment use air inlet humrat to calculate design outlet air temperature
                if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                    bPRINT = false;                                           // no field for detailed water coil, should print to eio anyway
                    TempSize = DataSizing::AutoSize;                          // coil report
                } else {
                    bPRINT = true;
                    TempSize = waterCoil.DesInletAirHumRat; // preserve input if entered
                }
                sizerCWDesInHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesInletAirHumRat = sizerCWDesInHumRat.size(state, TempSize, ErrorsFound);
            }

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                bPRINT = false;                                           // no field for detailed water coil, should print to eio anyway
                TempSize = DataSizing::AutoSize;                          // coil report
            } else {
                bPRINT = true;
                TempSize = waterCoil.DesOutletAirTemp; // preserve input if entered
            }

            state.dataSize->DataDesInletWaterTemp = waterCoil.DesInletWaterTemp; // used for warning messages
            CoolingWaterDesAirOutletTempSizer sizerCWDesAirOutTemp;
            sizerCWDesAirOutTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesOutletAirTemp = sizerCWDesAirOutTemp.size(state, TempSize, ErrorsFound);
            state.dataSize->DataDesOutletAirTemp = waterCoil.DesOutletAirTemp;

            if (state.dataSize->CurSysNum > 0) { // This call can be deleted at a future time and remove the if ( CurZoneEqNum > 0 ) check above. This
                                                 // will change the order of the eio file.
                if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                    bPRINT = false;                                           // no field for detailed water coil, should print this to eio anyway
                    TempSize = DataSizing::AutoSize;                          // coil report
                } else {
                    bPRINT = true;
                    TempSize = waterCoil.DesInletAirHumRat;
                }
                sizerCWDesInHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesInletAirHumRat = sizerCWDesInHumRat.size(state, TempSize, ErrorsFound);
            }

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                bPRINT = false;                                           // no field for detailed water coil, should print this to eio anyway
                TempSize = DataSizing::AutoSize;                          // coil report
            } else {
                bPRINT = true;
                TempSize = waterCoil.DesOutletAirHumRat; // preserve input if entered
            }
            CoolingWaterDesAirOutletHumRatSizer sizerCWDesOutHumRat;
            sizerCWDesOutHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesOutletAirHumRat = sizerCWDesOutHumRat.size(state, TempSize, ErrorsFound);
            state.dataSize->DataDesOutletAirHumRat = waterCoil.DesOutletAirHumRat;

            TempSize = DataSizing::AutoSize;
            bPRINT = true;
            if (waterCoil.MaxWaterVolFlowRate != DataSizing::AutoSize) {
                bPRINT = false;
            }
            if (state.dataSize->CurSysNum == 0) {
                bPRINT = false;
            }
            SizingString = "Design Coil Load [W]"; // there is no input field for this value and this is not the rated capacity (we should
                                                   // always print this!)
            // air inlet/outlet conditions should be known. Don't include fan heat in capacity calculation.
            state.dataSize->DataDesAccountForFanHeat = false;
            CoolingCapacitySizer sizerCoolingCapacity2;
            sizerCoolingCapacity2.overrideSizingString(SizingString);
            sizerCoolingCapacity2.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesWaterCoolingCoilRate = sizerCoolingCapacity2.size(state, TempSize, ErrorsFound);
            waterCoil.InletAirMassFlowRate =
                state.dataEnvrn->StdRhoAir * state.dataSize->DataFlowUsedForSizing; // inlet air mass flow rate is the autosized value
            state.dataSize->DataCapacityUsedForSizing = waterCoil.DesWaterCoolingCoilRate;

            // Why isn't the water volume flow rate based on the user inputs for inlet/outlet air/water temps? Water volume flow rate is
            // always based on autosized inputs.
            bPRINT = true;
            TempSize = waterCoil.MaxWaterVolFlowRate;
            sizerCWWaterflow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.MaxWaterVolFlowRate = sizerCWWaterflow.size(state, TempSize, ErrorsFound);
            state.dataSize->DataWaterFlowUsedForSizing = waterCoil.MaxWaterVolFlowRate;
            state.dataSize->DataCoilNum = 0;

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) { // 'DETAILED FLAT FIN'
                bPRINT = false; // do not print this sizing request since this coil does not have a design air flow rate input field (we
                                // should print this!)
            } else {
                bPRINT = true;
            }
            TempSize = waterCoil.DesAirVolFlowRate;
            CoolingAirFlowSizer sizingCoolingAirFlow2;
            std::string stringOverride = "Design Air Flow Rate [m3/s]";
            sizingCoolingAirFlow2.overrideSizingString(stringOverride);
            // sizingCoolingAirFlow2.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingCoolingAirFlow2.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.DesAirVolFlowRate = sizingCoolingAirFlow2.size(state, TempSize, ErrorsFound);
            waterCoil.DesAirMassFlowRate = waterCoil.DesAirVolFlowRate * state.dataEnvrn->StdRhoAir;

            if (waterCoil.DesAirVolFlowRate <= 0.0) {
                waterCoil.DesAirVolFlowRate = 0.0;
                ShowWarningError(state, std::format("The design air flow rate is zero for {} = {}", CompType, CompName));
                ShowContinueError(state, "The autosize value for max air volume flow rate is zero");
            }

            if (waterCoil.WaterCoilModel == CoilModel::CoolingDetailed) {

                int FieldNum = 16; //  N16, \field Number of Tubes per Row
                bPRINT = true;
                SizingString = state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(FieldNum);
                // Auto size detailed cooling coil number of tubes per row = int( 13750.0 * WaterCoil( CoilNum ).MaxWaterVolFlowRate ) + 1
                state.dataSize->DataFlowUsedForSizing = waterCoil.MaxWaterVolFlowRate;
                TempSize = float(waterCoil.NumOfTubesPerRow);
                CoolingWaterNumofTubesPerRowSizer sizerCWNumofTubesPerRow;
                sizerCWNumofTubesPerRow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.NumOfTubesPerRow = sizerCWNumofTubesPerRow.size(state, TempSize, ErrorsFound);

                // Auto size water coil fin diameter = 0.335 * WaterCoil( CoilNum ).InletAirMassFlowRate
                state.dataSize->DataConstantUsedForSizing = waterCoil.InletAirMassFlowRate;
                state.dataSize->DataFractionUsedForSizing = 0.335;
                TempSize = waterCoil.FinDiam;

                AutoCalculateSizer sizerFinDiameter;
                stringOverride = "Fin Diameter [m]";
                sizerFinDiameter.overrideSizingString(stringOverride);
                sizerFinDiameter.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.FinDiam = sizerFinDiameter.size(state, TempSize, ErrorsFound);

                // Auto size water coil minimum airflow area = 0.44 * WaterCoil( CoilNum ).InletAirMassFlowRate
                state.dataSize->DataConstantUsedForSizing = waterCoil.InletAirMassFlowRate;
                state.dataSize->DataFractionUsedForSizing = 0.44;
                TempSize = waterCoil.MinAirFlowArea;

                AutoCalculateSizer sizerMinAirFlowArea;
                stringOverride = "Minimum Airflow Area [m2]";
                sizerMinAirFlowArea.overrideSizingString(stringOverride);
                sizerMinAirFlowArea.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.MinAirFlowArea = sizerMinAirFlowArea.size(state, TempSize, ErrorsFound);

                if (waterCoil.MinAirFlowArea <= 0.0) {
                    ShowSevereError(state, std::format("Coil:Cooling:Water:DetailedGeometry: \"{}\"", waterCoil.Name));
                    ShowContinueError(state,
                                      std::format("Coil Minimum Airflow Area must be greater than 0. Coil area = {:.6f}", waterCoil.MinAirFlowArea));
                    ErrorsFound = true;
                }

                // Auto size water coil finned surface area = 78.5 * WaterCoil( CoilNum ).InletAirMassFlowRate
                state.dataSize->DataConstantUsedForSizing =
                    waterCoil.InletAirMassFlowRate; // actual autosized air mass flow rate, not calculated from user input
                state.dataSize->DataFractionUsedForSizing = 78.5;
                TempSize = waterCoil.FinSurfArea;

                AutoCalculateSizer sizerFinSurfaceArea;
                stringOverride = "Fin Surface Area [m2]";
                sizerFinSurfaceArea.overrideSizingString(stringOverride);
                sizerFinSurfaceArea.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.FinSurfArea = sizerFinSurfaceArea.size(state, TempSize, ErrorsFound);

                // Auto size water coil total tube inside surface area = 4.4 * WaterCoil( CoilNum ).TubeInsideDiam * WaterCoil( CoilNum
                // ).NumOfTubeRows * WaterCoil( CoilNum ).NumOfTubesPerRow
                state.dataSize->DataConstantUsedForSizing = waterCoil.TubeInsideDiam * waterCoil.NumOfTubeRows * waterCoil.NumOfTubesPerRow;
                state.dataSize->DataFractionUsedForSizing = 4.4;
                TempSize = waterCoil.TotTubeInsideArea;

                AutoCalculateSizer sizerTubeInsideArea;
                stringOverride = "Total Tube Inside Area [m2]";
                sizerTubeInsideArea.overrideSizingString(stringOverride);
                sizerTubeInsideArea.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.TotTubeInsideArea = sizerTubeInsideArea.size(state, TempSize, ErrorsFound);

                // Auto size water coil total tube outside surface area = 4.1 * WaterCoil( CoilNum ).TubeOutsideDiam * WaterCoil( CoilNum
                // ).NumOfTubeRows * WaterCoil( CoilNum ).NumOfTubesPerRow
                state.dataSize->DataConstantUsedForSizing = waterCoil.TubeOutsideDiam * waterCoil.NumOfTubeRows * waterCoil.NumOfTubesPerRow;
                state.dataSize->DataFractionUsedForSizing = 4.1;
                TempSize = waterCoil.TubeOutsideSurfArea;

                AutoCalculateSizer sizerTubeOutsideArea;
                stringOverride = "Tube Outside Surface Area [m2]";
                sizerTubeOutsideArea.overrideSizingString(stringOverride);
                sizerTubeOutsideArea.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.TubeOutsideSurfArea = sizerTubeOutsideArea.size(state, TempSize, ErrorsFound);

                if ((waterCoil.FinSurfArea + waterCoil.TubeOutsideSurfArea) <= 0.0) {
                    ShowSevereError(state, std::format("Coil:Cooling:Water:DetailedGeometry: \"{}\"", waterCoil.Name));
                    ShowContinueError(
                        state,
                        std::format("Coil Fin Surface Area plus Coil Tube Outside Surface Area must be greater than 0. Total surface area = {:.6f}",
                                    (waterCoil.FinSurfArea + waterCoil.TubeOutsideSurfArea)));
                    ErrorsFound = true;
                }

                // Auto size water coil coil depth = WaterCoil( CoilNum ).TubeDepthSpacing * WaterCoil( CoilNum ).NumOfTubeRows
                state.dataSize->DataConstantUsedForSizing = waterCoil.TubeDepthSpacing;
                state.dataSize->DataFractionUsedForSizing = waterCoil.NumOfTubeRows;
                TempSize = waterCoil.CoilDepth;

                AutoCalculateSizer sizerCoilDepth;
                stringOverride = "Coil Depth [m]";
                sizerCoilDepth.overrideSizingString(stringOverride);
                sizerCoilDepth.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.CoilDepth = sizerCoilDepth.size(state, TempSize, ErrorsFound);
            }
            state.dataSize->DataPltSizCoolNum = 0; // reset all globals to 0 to ensure correct sizing for other child components
            state.dataSize->DataWaterLoopNum = 0;
            state.dataSize->DataConstantUsedForSizing = 0.0;
            state.dataSize->DataFractionUsedForSizing = 0.0;
            state.dataSize->DataAirFlowUsedForSizing = 0.0;
            state.dataSize->DataFlowUsedForSizing = 0.0;
            state.dataSize->DataWaterFlowUsedForSizing = 0.0;
            state.dataSize->DataCapacityUsedForSizing = 0.0;
            state.dataSize->DataDesInletAirTemp = 0.0;
            state.dataSize->DataDesOutletAirTemp = 0.0;
            state.dataSize->DataDesOutletAirHumRat = 0.0;
            state.dataSize->DataDesInletAirHumRat = 0.0;
            state.dataSize->DataDesInletWaterTemp = 0.0;
            state.dataSize->DataWaterCoilSizCoolDeltaT = 0.0;
            state.dataSize->DataDesAccountForFanHeat = true;
        } else {
            // If there is no cooling Plant Sizing object and autosizing was requested, issue fatal error message
            if (waterCoil.RequestingAutoSize) {
                ShowSevereError(state, "Autosizing of water coil requires a cooling loop Sizing:Plant object");
                ShowContinueError(state, std::format("Occurs in water coil object= {}", waterCoil.Name));
                ErrorsFound = true;
            }
        }
        //} // end of cooling Plant Sizing existence IF - ELSE
    } // end cooling coil IF

    // if this is a heating coil
    if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating && waterCoil.RequestingAutoSize) {
        // find the appropriate heating Plant Sizing object
        PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
            state, "hot water coil", waterCoil.Name, waterCoil.WaterInletNodeNum, waterCoil.WaterOutletNodeNum, LoopErrorsFound);
    }

    if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {

        if (waterCoil.UseDesignWaterDeltaTemp) {
            // use water design deltaT specified in the heating water coils
            state.dataSize->DataWaterCoilSizHeatDeltaT = waterCoil.DesignWaterDeltaTemp;
        } else {
            if (PltSizHeatNum > 0) {
                state.dataSize->DataWaterCoilSizHeatDeltaT = state.dataSize->PlantSizData(PltSizHeatNum).DeltaT;
            }
        }

        if (PltSizHeatNum > 0) {

            int FieldNum = 0;
            bool NomCapUserInp = false; // flag for whether user has onput a nominal heating capacity

            state.dataSize->DataPltSizHeatNum = PltSizHeatNum;
            state.dataSize->DataWaterLoopNum = waterCoil.WaterPlantLoc.loopNum;
            rho = waterCoil.WaterPlantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
            Cp = state.dataPlnt->PlantLoop(state.dataSize->DataWaterLoopNum).glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
            if (waterCoil.DesTotWaterCoilLoad > 0.0) {
                NomCapUserInp = true;
            } else if (state.dataSize->CurSysNum > 0 && state.dataSize->CurSysNum <= state.dataHVACGlobal->NumPrimaryAirSys) {
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatingCapMethod == DataSizing::CapacityPerFloorArea) {
                    NomCapUserInp = true;
                } else if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatingCapMethod == DataSizing::HeatingDesignCapacity &&
                           state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatingTotalCapacity > 0.0) {
                    NomCapUserInp = true;
                }
            } else {
                NomCapUserInp = false;
            }
            bool bPRINT = false;                                               // do not print this sizing request
            TempSize = DataSizing::AutoSize;                                   // get the autosized air volume flow rate for use in other calculations
            SizingString.clear();                                              // doesn't matter
            CompType = HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater]; // "Coil:Heating:Water"
            std::string const &CompName = waterCoil.Name;
            if (waterCoil.DesiccantRegenerationCoil) {
                state.dataSize->DataDesicRegCoil = true;
                state.dataSize->DataDesicDehumNum = waterCoil.DesiccantDehumNum;
                HeatingCoilDesAirInletTempSizer sizerHeatingDesInletTemp;
                ErrorsFound = false;
                sizerHeatingDesInletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataDesInletAirTemp = sizerHeatingDesInletTemp.size(state, DataSizing::AutoSize, ErrorsFound);

                HeatingCoilDesAirOutletTempSizer sizerHeatingDesOutletTemp;
                ErrorsFound = false;
                sizerHeatingDesOutletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataDesOutletAirTemp = sizerHeatingDesOutletTemp.size(state, DataSizing::AutoSize, ErrorsFound);

                if (state.dataSize->CurOASysNum > 0) {
                    auto &OASysEqSizing = state.dataSize->OASysEqSizing(state.dataSize->CurOASysNum);
                    OASysEqSizing.AirFlow = true;
                    OASysEqSizing.AirVolFlow = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow;
                }
                TempSize = DataSizing::AutoSize; // reset back
            }
            ErrorsFound = false;
            HeatingAirFlowSizer sizingHeatingAirFlow;
            sizingHeatingAirFlow.overrideSizingString(SizingString);
            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            TempSize = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
            // reset the design air volume flow rate for air loop coils only
            if (state.dataSize->CurSysNum > 0) {
                waterCoil.DesAirVolFlowRate = TempSize;
            }
            waterCoil.InletAirMassFlowRate = state.dataEnvrn->StdRhoAir * TempSize; // inlet air mass flow rate is not the autosized value
            state.dataSize->DataAirFlowUsedForSizing = TempSize;
            state.dataSize->DataFlowUsedForSizing = TempSize; // many autosized inputs use the design (autosized) air flow rate, save this value

            bPRINT = true;
            if (waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp) {
                TempSize = waterCoil.DesTotWaterCoilLoad;
                state.dataSize->DataNomCapInpMeth = true;
            } else {
                TempSize = DataSizing::AutoSize;
            }
            FieldNum = 3; //  N3 , \field Rated Capacity
            SizingString = state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(FieldNum) + " [W]";
            ErrorsFound = false;
            if (state.dataSize->CurSysNum > 0) {
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesWaterHeatingCoilRate = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
            } else {
                WaterHeatingCapacitySizer sizerWaterHeatingCapacity;
                sizerWaterHeatingCapacity.overrideSizingString(SizingString);
                sizerWaterHeatingCapacity.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesWaterHeatingCoilRate = sizerWaterHeatingCapacity.size(state, TempSize, ErrorsFound);
            }
            waterCoil.DesTotWaterCoilLoad = waterCoil.DesWaterHeatingCoilRate;
            state.dataSize->DataCapacityUsedForSizing = waterCoil.DesWaterHeatingCoilRate;

            // We now have the design load if it was autosized. For the case of CoilPerfInpMeth == NomCap, calculate the air flow rate
            // specified by the NomCap inputs. This overrides all previous values
            if (waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp) {
                waterCoil.InletAirMassFlowRate =
                    waterCoil.DesTotWaterCoilLoad / (CpAirStd * (waterCoil.DesOutletAirTemp - waterCoil.DesInletAirTemp));
                waterCoil.DesAirVolFlowRate = waterCoil.InletAirMassFlowRate / state.dataEnvrn->StdRhoAir;
                state.dataSize->DataAirFlowUsedForSizing = waterCoil.DesAirVolFlowRate;
                state.dataSize->DataFlowUsedForSizing = waterCoil.DesAirVolFlowRate;
            }

            TempSize = waterCoil.MaxWaterVolFlowRate;

            if (waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp) {
                if (waterCoil.DesTotWaterCoilLoad > HVAC::SmallLoad) {
                    waterCoil.MaxWaterVolFlowRate =
                        state.dataSize->DataCapacityUsedForSizing / (Cp * rho * (waterCoil.DesInletWaterTemp - waterCoil.DesOutletWaterTemp));
                } else {
                    waterCoil.MaxWaterVolFlowRate = 0.0;
                }
                state.dataSize->DataConstantUsedForSizing = waterCoil.MaxWaterVolFlowRate;
                state.dataSize->DataFractionUsedForSizing = 1.0;
            }
            state.dataSize->DataCoilNum = CoilNum;
            HeatingWaterflowSizer sizerHWWaterflow;
            sizerHWWaterflow.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            Real64 sizedMaxWaterVolFlowRate = sizerHWWaterflow.size(state, TempSize, ErrorsFound);
            state.dataSize->DataCoilNum = 0;
            // Check if the water flow rate is defined in parent HVAC equipment and set water coil design water flow rate accordingly
            if (state.dataSize->CurZoneEqNum > 0) {
                auto const &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
                if (ZoneEqSizing.DesignSizeFromParent) {
                    state.dataSize->DataWaterFlowUsedForSizing = ZoneEqSizing.MaxHWVolFlow;
                    waterCoil.MaxWaterVolFlowRate = ZoneEqSizing.MaxHWVolFlow;
                } else {
                    state.dataSize->DataWaterFlowUsedForSizing = sizedMaxWaterVolFlowRate;
                    waterCoil.MaxWaterVolFlowRate = sizedMaxWaterVolFlowRate;
                }
            } else {
                state.dataSize->DataWaterFlowUsedForSizing = sizedMaxWaterVolFlowRate;
                waterCoil.MaxWaterVolFlowRate = sizedMaxWaterVolFlowRate;
            }
            state.dataSize->DataConstantUsedForSizing = 0.0; // reset these in case NomCapUserInp was true
            state.dataSize->DataFractionUsedForSizing = 0.0;
            if (waterCoil.MaxWaterVolFlowRate <= 0.0) {
                //                    MaxWaterVolFlowRateDes = 0.0;
                ShowWarningError(state, std::format("The design coil load is zero for Coil:Heating:Water {}", waterCoil.Name));
                ShowContinueError(state, "The autosize value for maximum water flow rate is zero");
                ShowContinueError(state, "To change this, input a value for UA, change the heating design day, or raise the");
                ShowContinueError(state, "  system heating design supply air temperature. Also check to make sure the Preheat");
                ShowContinueError(state, "  Design Temperature is not the same as the Central Heating Design Supply Air Temperature. ");
            }

            // initialize the water coil inlet conditions
            bPRINT = false; // no need to print to eio since we only need the values
            state.dataSize->DataFlowUsedForSizing = state.dataSize->DataAirFlowUsedForSizing;
            if (waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp) {
                waterCoil.InletAirTemp = waterCoil.DesInletAirTemp;
                waterCoil.InletAirHumRat = PsyWFnTdbRhPb(state, waterCoil.DesInletAirTemp, 0.5, state.dataEnvrn->StdBaroPress, RoutineName);
                waterCoil.InletAirMassFlowRate = state.dataSize->DataAirFlowUsedForSizing * state.dataEnvrn->StdRhoAir; // don't need this
                state.dataSize->DataDesOutletAirTemp = waterCoil.DesOutletAirTemp;                                      // for error messages
                state.dataSize->DataDesOutletAirHumRat =
                    PsyWFnTdbRhPb(state, state.dataSize->DataDesOutletAirTemp, 0.5, state.dataEnvrn->StdBaroPress, RoutineName); // for error messages
                waterCoil.InletWaterMassFlowRate = rho * state.dataSize->DataWaterFlowUsedForSizing;
                waterCoil.MaxWaterMassFlowRate = rho * state.dataSize->DataWaterFlowUsedForSizing;
                waterCoil.InletWaterTemp = waterCoil.DesInletWaterTemp;
            } else if (waterCoil.DesiccantRegenerationCoil) {
                waterCoil.InletAirTemp = state.dataSize->DataDesInletAirTemp;
                HeatingCoilDesAirInletHumRatSizer sizerHeatingDesInletHumRat;
                ErrorsFound = false;
                sizerHeatingDesInletHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesInletAirHumRat = sizerHeatingDesInletHumRat.size(state, DataSizing::AutoSize, ErrorsFound);
                waterCoil.InletAirHumRat = waterCoil.DesInletAirHumRat;

                waterCoil.DesAirVolFlowRate = state.dataSize->DataAirFlowUsedForSizing;                                 // coil report
                waterCoil.InletAirMassFlowRate = state.dataSize->DataAirFlowUsedForSizing * state.dataEnvrn->StdRhoAir; // this is stiil volume flow!
            } else {
                HeatingWaterDesAirInletTempSizer sizerHWDesInletTemp;
                sizerHWDesInletTemp.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.InletAirTemp = sizerHWDesInletTemp.size(state, DataSizing::AutoSize, ErrorsFound);

                HeatingWaterDesAirInletHumRatSizer sizerHWAirInletHumRat;
                sizerHWAirInletHumRat.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesInletAirHumRat = sizerHWAirInletHumRat.size(state, DataSizing::AutoSize, ErrorsFound);
                waterCoil.InletAirHumRat = waterCoil.DesInletAirHumRat;

                HeatingAirflowUASizer sizerHWAirFlowUA;
                sizerHWAirFlowUA.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                waterCoil.DesAirMassFlowRate = sizerHWAirFlowUA.size(state, DataSizing::AutoSize, ErrorsFound);
                waterCoil.InletAirMassFlowRate = waterCoil.DesAirMassFlowRate;
            }

            // zone and air loop coils use different design coil load calculations, air loop coils use air side capacity,
            // zone coils use water side capacity
            state.dataSize->DataDesInletAirTemp = waterCoil.InletAirTemp;                                                  // used in error messages
            state.dataSize->DataDesInletAirHumRat = waterCoil.InletAirHumRat;                                              // used in error messages
            state.dataSize->DataFlowUsedForSizing = state.dataSize->DataAirFlowUsedForSizing * state.dataEnvrn->StdRhoAir; // used in error messages
            waterCoil.MaxWaterVolFlowRate = state.dataSize->DataWaterFlowUsedForSizing;                                    // why is this here?
            if (!(waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp)) {
                // get the design coil load used to size UA
                HeatingWaterDesCoilLoadUsedForUASizer sizerHWDesCoilLoadForUA;
                sizerHWDesCoilLoadForUA.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataCapacityUsedForSizing = sizerHWDesCoilLoadForUA.size(state, DataSizing::AutoSize, ErrorsFound);
                // get the water volume flow rate used to size UA
                HeatingWaterDesCoilWaterVolFlowUsedForUASizer sizerHWWaterVolFlowUA;
                sizerHWWaterVolFlowUA.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
                state.dataSize->DataWaterFlowUsedForSizing = sizerHWWaterVolFlowUA.size(state, DataSizing::AutoSize, ErrorsFound);
                waterCoil.InletWaterTemp = state.dataSize->PlantSizData(PltSizHeatNum).ExitTemp;
                waterCoil.InletWaterMassFlowRate = rho * state.dataSize->DataWaterFlowUsedForSizing;
                waterCoil.MaxWaterMassFlowRate = rho * state.dataSize->DataWaterFlowUsedForSizing;
                waterCoil.DesWaterHeatingCoilRate = state.dataSize->DataCapacityUsedForSizing;
            }
            // calculate UA
            if (state.dataSize->CurSysNum > 0) {
                waterCoil.DesTotWaterCoilLoad = state.dataSize->DataCapacityUsedForSizing;
            }
            FieldNum = 1;  // N1 , \field U-Factor Times Area Value
            bPRINT = true; // report to eio the UA value
            SizingString = state.dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(FieldNum) + " [W/K]";
            state.dataSize->DataCoilNum = CoilNum;
            state.dataSize->DataFanOp = HVAC::FanOp::Continuous;
            if (waterCoil.CoilPerfInpMeth == state.dataWaterCoils->NomCap && NomCapUserInp) {
                TempSize = DataSizing::AutoSize;
            } else {
                TempSize = waterCoil.UACoil;
            }

            state.dataSize->DataFlowUsedForSizing = waterCoil.InletAirMassFlowRate;
            DesCoilWaterInTempSaved = state.dataWaterCoils->WaterCoil(state.dataSize->DataCoilNum).InletWaterTemp;
            if (DesCoilWaterInTempSaved < HVAC::DesCoilHWInletTempMin) {
                // at low coil design water inlet temp, sizing has convergence issue hence slightly higher water inlet temperature
                // is estimated in "EstimateCoilInletWaterTemp" and used for UA autosizing only
                EstimateCoilInletWaterTemp(state,
                                           state.dataSize->DataCoilNum,
                                           state.dataSize->DataFanOp,
                                           1.0,
                                           state.dataSize->DataCapacityUsedForSizing,
                                           DesCoilInletWaterTempUsed);
                state.dataWaterCoils->WaterCoil(state.dataSize->DataCoilNum).InletWaterTemp = DesCoilInletWaterTempUsed;
            }
            // must set DataCapacityUsedForSizing, DataWaterFlowUsedForSizing and DataFlowUsedForSizing to size UA. Any value of 0 will result
            // in UA = 1.
            WaterHeatingCoilUASizer sizerHWCoilUA;
            sizerHWCoilUA.initializeWithinEP(state, CompType, CompName, bPRINT, RoutineName);
            waterCoil.UACoil = sizerHWCoilUA.size(state, TempSize, ErrorsFound);
            if (DesCoilWaterInTempSaved < HVAC::DesCoilHWInletTempMin) {
                ShowWarningError(state, std::format("Autosizing of heating coil UA for Coil:Heating:Water \"{}\"", CompName));
                ShowContinueError(state,
                                  std::format(" Plant design loop exit temperature = {:.2f} C",
                                              state.dataSize->PlantSizData(state.dataSize->DataPltSizHeatNum).ExitTemp));
                ShowContinueError(state, " Plant design loop exit temperature is low for design load and leaving air temperature anticipated.");
                ShowContinueError(
                    state, std::format(" Heating coil UA-value is sized using coil water inlet temperature = {:.2f} C", DesCoilInletWaterTempUsed));
                state.dataWaterCoils->WaterCoil(state.dataSize->DataCoilNum).InletWaterTemp =
                    DesCoilWaterInTempSaved; // reset the Design Coil Inlet Water Temperature
            }
            // if coil UA did not size due to one of these variables being 0, must set UACoilVariable to avoid crash later on
            if (state.dataSize->DataCapacityUsedForSizing == 0.0 || state.dataSize->DataWaterFlowUsedForSizing == 0.0 ||
                state.dataSize->DataFlowUsedForSizing == 0.0) {
                if (waterCoil.UACoilVariable == DataSizing::AutoSize) {
                    waterCoil.UACoilVariable = waterCoil.UACoil;
                }
            }
            // WaterCoil(CoilNum).UACoilVariable = WaterCoil(CoilNum).UACoil;
            waterCoil.DesWaterHeatingCoilRate = state.dataSize->DataCapacityUsedForSizing;
            state.dataWaterCoils->WaterCoil(state.dataSize->DataCoilNum).InletWaterTemp =
                DesCoilWaterInTempSaved; // reset the Design Coil Inlet Water Temperature

            state.dataSize->DataWaterLoopNum = 0; // reset all globals to 0 to ensure correct sizing for other child components
            state.dataSize->DataPltSizHeatNum = 0;
            state.dataSize->DataCoilNum = 0;
            state.dataSize->DataFanOp = HVAC::FanOp::Invalid;
            state.dataSize->DataCapacityUsedForSizing = 0.0;
            state.dataSize->DataWaterFlowUsedForSizing = 0.0;
            state.dataSize->DataDesInletAirTemp = 0.0;
            state.dataSize->DataDesInletAirHumRat = 0.0;
            state.dataSize->DataDesOutletAirTemp = 0.0;
            state.dataSize->DataDesOutletAirHumRat = 0.0;
            state.dataSize->DataAirFlowUsedForSizing = 0.0;
            state.dataSize->DataFlowUsedForSizing = 0.0;
            state.dataSize->DataDesicDehumNum = 0;
            state.dataSize->DataDesicRegCoil = false;
            state.dataSize->DataWaterCoilSizHeatDeltaT = 0.0;
            state.dataSize->DataNomCapInpMeth = false;

        } else {
            // if there is no heating Plant Sizing object and autosizng was requested, issue an error message
            if (waterCoil.RequestingAutoSize) {
                ShowSevereError(state, "Autosizing of water coil requires a heating loop Sizing:Plant object");
                ShowContinueError(state, std::format("Occurs in water coil object= {}", waterCoil.Name));
                ErrorsFound = true;
            }
        }
        //} // end of heating Plant Sizing existence IF - ELSE
    } // end heating coil IF

    // save the design water volumetric flow rate for use by the water loop sizing algorithms
    if (waterCoil.MaxWaterVolFlowRate > 0.0) {
        PlantUtilities::RegisterPlantCompDesignFlow(state, waterCoil.WaterInletNodeNum, waterCoil.MaxWaterVolFlowRate);
    }

    if (ErrorsFound || state.dataSize->DataErrorsFound) {
        ShowFatalError(state, "Preceding water coil sizing errors cause program termination");
    }
}

// End Initialization Section of the Module
//******************************************************************************

// Begin Algorithm Section of the Module
//******************************************************************************

void CalcSimpleHeatingCoil(EnergyPlusData &state,
                           int const CoilNum,          // index to heating coil
                           HVAC::FanOp const fanOp,    // fan operating mode
                           Real64 const PartLoadRatio, // part-load ratio of heating coil
                           int const CalcMode          // 1 = design calc; 2 = simulation calculation
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rich Liesen
    //       DATE WRITTEN
    //       MODIFIED       Aug. 2007 - R. Raustad, added fan operating mode and part-load ratio to
    //                                  calculate the outlet conditions when fan and coil cycle.
    //                                  Air and water outlet temperature are full output with average
    //                                  air and water mass flow rate when fan and coil cycle.
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Simulates a simple NTU effectiveness model heating coil

    // METHODOLOGY EMPLOYED:
    // (1) outlet conditions are calculated from the effectiveness and the inlet conditions.
    // (2) Effectiveness is calculated from the NTU formula for a cross flow heat exchanger
    //     with both streams unmixed.
    // Note: UA is input by user and is fixed.

    // REFERENCES:
    // See for instance ASHRAE HVAC 2 Toolkit, page 4-4, formula (4-7)

    // Using/Aliasing

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcSimpleHeatingCoil");

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 WaterMassFlowRate;
    Real64 AirMassFlow; // [kg/sec]
    Real64 TempAirIn;   // [C]
    Real64 TempAirOut;  // [C]
    Real64 Win;
    Real64 TempWaterIn;
    Real64 TempWaterOut;
    Real64 UA;
    Real64 CapacitanceAir;
    Real64 CapacitanceWater;
    Real64 CapacitanceMin;
    Real64 CapacitanceMax;
    Real64 HeatingCoilLoad;
    Real64 NTU;
    Real64 ETA;
    Real64 A;
    Real64 CapRatio;
    Real64 E1;
    Real64 E2;
    Real64 Effec;
    Real64 Cp;

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    UA = waterCoil.UACoilVariable;
    TempAirIn = waterCoil.InletAirTemp;
    Win = waterCoil.InletAirHumRat;
    TempWaterIn = waterCoil.InletWaterTemp;

    // adjust mass flow rates for cycling fan cycling coil operation
    if (fanOp == HVAC::FanOp::Cycling) {
        if (PartLoadRatio > 0.0) {
            AirMassFlow = waterCoil.InletAirMassFlowRate / PartLoadRatio;
            WaterMassFlowRate = min(waterCoil.InletWaterMassFlowRate / PartLoadRatio, waterCoil.MaxWaterMassFlowRate);
        } else {
            AirMassFlow = 0.0;
            WaterMassFlowRate = 0.0;
        }
    } else {
        AirMassFlow = waterCoil.InletAirMassFlowRate;
        WaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    }

    if (WaterMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) { // If the coil is operating
        CapacitanceAir = PsyCpAirFnW(Win) * AirMassFlow;
        Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, TempWaterIn, RoutineName);
        CapacitanceWater = Cp * WaterMassFlowRate;
        CapacitanceMin = min(CapacitanceAir, CapacitanceWater);
        CapacitanceMax = max(CapacitanceAir, CapacitanceWater);
    } else {
        CapacitanceAir = 0.0;
        CapacitanceWater = 0.0;
    }

    // If the coil is operating there should be some heating capacitance
    //  across the coil, so do the simulation. If not set outlet to inlet and no load.
    //  Also the coil has to be scheduled to be available
    if (((CapacitanceAir > 0.0) && (CapacitanceWater > 0.0)) &&
        (CalcMode == state.dataWaterCoils->DesignCalc || state.dataWaterCoils->MySizeFlag(CoilNum) ||
         state.dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum) || waterCoil.availSched->getCurrentVal() > 0.0)) {

        if (UA <= 0.0) {
            ShowFatalError(state, std::format("UA is zero for COIL:Heating:Water {}", waterCoil.Name));
        }
        NTU = UA / CapacitanceMin;
        ETA = std::pow(NTU, 0.22);
        CapRatio = CapacitanceMin / CapacitanceMax;
        A = CapRatio * NTU / ETA;

        if (A > 20.0) {
            A = ETA * 1.0 / CapRatio;
        } else {
            E1 = std::exp(-A);
            A = ETA * (1.0 - E1) / CapRatio;
        }

        if (A > 20.0) {
            Effec = 1.0;
        } else {
            E2 = std::exp(-A);
            Effec = 1.0 - E2;
        }

        TempAirOut = TempAirIn + Effec * CapacitanceMin * (TempWaterIn - TempAirIn) / CapacitanceAir;
        TempWaterOut = TempWaterIn - CapacitanceAir * (TempAirOut - TempAirIn) / CapacitanceWater;
        HeatingCoilLoad = CapacitanceWater * (TempWaterIn - TempWaterOut);
        // The HeatingCoilLoad is the change in the enthalpy of the water
        waterCoil.OutletWaterEnthalpy = waterCoil.InletWaterEnthalpy - HeatingCoilLoad / waterCoil.InletWaterMassFlowRate;
        waterCoil.OutletWaterMassFlowRate = waterCoil.InletWaterMassFlowRate;

    } else { // If not running Conditions do not change across coil from inlet to outlet

        TempAirOut = TempAirIn;
        TempWaterOut = TempWaterIn;
        HeatingCoilLoad = 0.0;
        waterCoil.OutletWaterEnthalpy = waterCoil.InletWaterEnthalpy;
        waterCoil.OutletWaterMassFlowRate = 0.0;
    }

    if (fanOp == HVAC::FanOp::Cycling) {
        HeatingCoilLoad *= PartLoadRatio;
    }

    // Set the outlet conditions
    waterCoil.TotWaterHeatingCoilRate = HeatingCoilLoad;
    waterCoil.OutletAirTemp = TempAirOut;
    waterCoil.OutletWaterTemp = TempWaterOut;

    // This WaterCoil does not change the moisture or Mass Flow across the component
    waterCoil.OutletAirHumRat = waterCoil.InletAirHumRat;
    waterCoil.OutletAirMassFlowRate = waterCoil.InletAirMassFlowRate;
    // Set the outlet enthalpys for air and water
    waterCoil.OutletAirEnthalpy = PsyHFnTdbW(waterCoil.OutletAirTemp, waterCoil.OutletAirHumRat);
}

void CalcDetailFlatFinCoolingCoil(EnergyPlusData &state,
                                  int const CoilNum,
                                  int const CalcMode,
                                  HVAC::FanOp const fanOp,   // fan operating mode
                                  Real64 const PartLoadRatio // part-load ratio of heating coil
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR(S)      Russell Taylor / Richard Liesen
    //       DATE WRITTEN   Mar 1997
    //       MODIFIED       Feb 2010, B. Nigusse, FSEC, corrected units inconsistency for tube and fins
    //                      materials thermal conductivties. Now input values in the idf are in {W/(m.K)}
    //       RE-ENGINEERED  Sept 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates a chilled water cooling coil.  Provided with
    // the coil geometry and the flow (i.e. air and water) inlet conditions,
    // it will calculate the flow outlet conditions and the total and latent
    // heat extraction rates from the air.  The coil model has some limitations
    // as noted in the code.

    // METHODOLOGY EMPLOYED:
    // successive substitution, solve coil as if all wet, then
    // again if partly or entirely dry

    // REFERENCES:
    // First found in Type 12 from MODSIM, but now
    // programmed directly from Elmahdy, A.H. and Mitalas, G.P.  "A
    // Simple Model for Cooling and Dehumidifying Coils for Use in
    // Calculating Energy Requirements for Buildings"  _ASHRAE
    // Transactions_ Vol. 83, Part 2, pp. 103-117 (1977).

    // OTHER NOTES:
    // Routine was originally adapted for use in IBLAST by R.D. Taylor in l993.
    // Subsequently rewritten and improved by J.C. Vanderzee in 1994
    // Revised and further enhanced by R.D. Taylor in Jan 1996
    // Re-engineered for EnergyPlus by Richard Liesen PhD in 1998

    // Using/Aliasing

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    static Real64 const exp_47(std::exp(-0.41718));
    static Real64 const exp_35(std::exp(-0.3574));
    static constexpr std::string_view RoutineName("CalcDetailFlatFinCoolingCoil");

    constexpr Real64 AirViscosity(1.846e-5); // Dynamic Viscosity of Air in kg/(m.s)
    constexpr Real64 ConvK(1.0e-3);          // Unit conversion factor
    constexpr Real64 unity(1.0);
    constexpr Real64 zero(0.0);
    constexpr Real64 TubeFoulFactor(5.0e-2); // Inside tube fouling factor for water, in m2K/kW
    // Changed from m2K/W to m2K/kW for consistency with the
    // other parameters in "TubeFoulThermResis" calculation

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    //    INTEGER :: CoolCoilErrs = 0

    Real64 AirEnthAtRsdInletWaterTemp;
    Real64 AirExitEnthlAtCoilSurfTemp;
    Real64 AirExitCoilSurfTemp;
    Real64 AirReynoldsNo;
    Real64 AirEnthAtWetDryIntrfcSurfTemp;
    Real64 AirSideDrySurfFilmCoef;
    Real64 AirSideWetSurfFilmCoef;
    Real64 AirWetDryInterfcTemp;
    Real64 CoilToAirThermResistDrySurf;
    Real64 CoilToAirThermResistWetSurf;
    Real64 DryAirSpecHeat;
    Real64 DryCoilCoeff1;
    Real64 DryCoilCoeff;
    Real64 DryCoilEfficiency;
    Real64 DryFinEfficncy;
    Real64 DryCoilInThermResist;
    Real64 DrySideEffectiveWaterTemp;
    Real64 EnterAirDewPoint;
    Real64 EnterAirHumRatDiff;
    Real64 WetDryInterSurfTempErrorLast;
    Real64 WetDryInterSurfTempError;
    Real64 expon;
    Real64 FilmCoefEqnFactor;
    Real64 FilmCoefReynldsCorrelatnFact;
    Real64 FinToTotSurfAreaRatio;
    Real64 InCoilSurfTemp;
    Real64 InsdToOutsdThermResistRatio;
    Real64 InSurfTempSatAirEnthl;
    Real64 K1;
    Real64 MeanWaterTemp;
    Real64 MoistAirSpecificHeat;
    Real64 OutCoilSurfTemp;
    Real64 OutSurfTempSatAirEnthl;
    Real64 RaisedInletWaterTemp;
    Real64 RsdInletWaterTempSatAirHumRat;
    Real64 ScaledAirMassFlowRate;
    Real64 ScaledCoilAirThermResistWetSurf;
    Real64 ScaledWaterSpecHeat;
    Real64 ScaledWaterToTubeThermResist;
    Real64 SensToTotEnthDiffRatio;
    Real64 SurfAreaWet;
    Real64 TubeFoulThermResist;
    Real64 TubeWaterVel;
    Real64 UACoilAllWet;
    Real64 UACoilPartWet;
    Real64 UADryCoil;
    Real64 WaterToTubeThermResist;
    Real64 WetAreaChange;
    Real64 WetAreaLast;
    Real64 WetCoilCoeff;
    Real64 WetCoilFinEfficncy;
    Real64 WetDryInterfcAirEnthl;
    Real64 WetDryInterfcSurfTemp;
    Real64 WetDryInterfcWaterTemp;
    Real64 WetFinEfficncy;
    Real64 WetSideEffctvWaterTemp;
    Real64 y;
    Real64 TempAirIn;
    Real64 TempAirOut;
    Real64 InletAirHumRat;
    Real64 OutletAirHumRat;
    Real64 InletAirEnthalpy;
    Real64 OutletAirEnthalpy;
    Real64 WaterMassFlowRate;
    Real64 AirMassFlow;
    Real64 TempWaterIn;
    Real64 TempWaterOut;
    Real64 TotWaterCoilLoad;
    Real64 SenWaterCoilLoad;
    Real64 AirDensity;
    Real64 AirVelocity;
    Real64 denom;
    Real64 rho;
    Real64 Cp;

    // Set derived type variables to shorter local variables
    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    TempAirIn = waterCoil.InletAirTemp;
    InletAirHumRat = waterCoil.InletAirHumRat;
    TempWaterIn = waterCoil.InletWaterTemp;

    //  adjust mass flow rates for cycling fan cycling coil operation
    if (fanOp == HVAC::FanOp::Cycling) {
        if (PartLoadRatio > 0.0) {
            AirMassFlow = waterCoil.InletAirMassFlowRate / PartLoadRatio;
            WaterMassFlowRate = min(waterCoil.InletWaterMassFlowRate / PartLoadRatio, waterCoil.MaxWaterMassFlowRate);
        } else {
            AirMassFlow = 0.0;
            WaterMassFlowRate = 0.0;
        }
    } else {
        AirMassFlow = waterCoil.InletAirMassFlowRate;
        WaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    }

    if (WaterMassFlowRate < waterCoil.MaxWaterMassFlowRate * WaterCoils::MinWaterMassFlowFrac) {
        WaterMassFlowRate = 0.0;
    }
    if (TempAirIn <= TempWaterIn) {
        WaterMassFlowRate = 0.0;
    }
    WetDryInterfcAirEnthl = 0.0;
    OutletAirEnthalpy = 0.0;
    InletAirEnthalpy = 0.0;

    // Warning and error messages for large flow rates for the given user input geometry
    AirDensity = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, TempAirIn, InletAirHumRat, RoutineName);
    if (AirMassFlow > (5.0 * waterCoil.MinAirFlowArea / AirDensity) && state.dataWaterCoils->CoilWarningOnceFlag(CoilNum)) {
        ShowWarningError(state, std::format("Coil:Cooling:Water:DetailedGeometry in Coil ={}", waterCoil.Name));
        ShowContinueError(state, "Air Flow Rate Velocity has greatly exceeded upper design guidelines of ~2.5 m/s");
        ShowContinueError(state, std::format("Air Mass Flow Rate[kg/s]={:.6f}", AirMassFlow));
        // [m/s] = [kg/s] / ([m2] * [kg/m3])
        AirVelocity = AirMassFlow / (waterCoil.MinAirFlowArea * AirDensity);
        ShowContinueError(state, std::format("Air Face Velocity[m/s]={:.6f}", AirVelocity));
        ShowContinueError(state,
                          std::format("Approximate Mass Flow Rate limit for Face Area[kg/s]={:.6f}", 2.5 * waterCoil.MinAirFlowArea * AirDensity));
        ShowContinueError(state, "Coil:Cooling:Water:DetailedGeometry could be resized/autosized to handle capacity");
        state.dataWaterCoils->CoilWarningOnceFlag(CoilNum) = false;
    } else if (AirMassFlow > (44.7 * waterCoil.MinAirFlowArea * AirDensity)) {
        ShowSevereError(state, std::format("Coil:Cooling:Water:DetailedGeometry in Coil ={}", waterCoil.Name));
        ShowContinueError(state, "Air Flow Rate Velocity is > 100MPH (44.7m/s) and simulation cannot continue");
        ShowContinueError(state, std::format("Air Mass Flow Rate[kg/s]={:.6f}", AirMassFlow));
        AirVelocity = AirMassFlow / (waterCoil.MinAirFlowArea * AirDensity);
        ShowContinueError(state, std::format("Air Face Velocity[m/s]={:.6f}", AirVelocity));
        ShowContinueError(state,
                          std::format("Approximate Mass Flow Rate limit for Face Area[kg/s]={:.6f}", 44.7 * waterCoil.MinAirFlowArea * AirDensity));
        ShowFatalError(state, "Coil:Cooling:Water:DetailedGeometry needs to be resized/autosized to handle capacity");
    }

    // If Coil is Scheduled ON then do the simulation
    if (((waterCoil.availSched->getCurrentVal() > 0.0) && (WaterMassFlowRate > 0.0) && (AirMassFlow >= WaterCoils::MinAirMassFlow)) ||
        (CalcMode == state.dataWaterCoils->DesignCalc)) {
        //        transfer inputs to simulation variables and calculate
        //        known thermodynamic functions
        // All coil calcs are done in KJoules.  Convert to KJ here and then convert
        //  back to Joules at the end of the Subroutine.
        DryAirSpecHeat = PsyCpAirFnW(zero) * ConvK;
        MoistAirSpecificHeat = PsyCpAirFnW(InletAirHumRat) * ConvK;
        InletAirEnthalpy = waterCoil.InletAirEnthalpy * ConvK;

        EnterAirDewPoint = PsyTdpFnWPb(state, InletAirHumRat, state.dataEnvrn->OutBaroPress, RoutineName);
        //       Ratio of secondary (fin) to total (secondary plus primary) surface areas
        FinToTotSurfAreaRatio = waterCoil.FinSurfArea / waterCoil.TotCoilOutsideSurfArea;
        //      known water and air flow parameters:
        rho = waterCoil.WaterPlantLoc.loop->glycol->getDensity(state, TempWaterIn, RoutineName);
        //      water flow velocity - assuming number of water circuits = NumOfTubesPerRow
        TubeWaterVel =
            WaterMassFlowRate * 4.0 / (waterCoil.NumOfTubesPerRow * rho * Constant::Pi * waterCoil.TubeInsideDiam * waterCoil.TubeInsideDiam);
        //      air mass flow rate per unit area
        ScaledAirMassFlowRate = (1.0 + InletAirHumRat) * AirMassFlow / waterCoil.MinAirFlowArea;
        //      air flow Reynold's Number
        AirReynoldsNo = waterCoil.CoilEffectiveInsideDiam * ScaledAirMassFlowRate / AirViscosity;
        //       heat transfer coefficients and resistance components:
        //              inside (water)
        WaterToTubeThermResist = std::pow(waterCoil.TubeInsideDiam, 0.2) / (waterCoil.TotTubeInsideArea * 1.429 * std::pow(TubeWaterVel, 0.8));
        //              metal and fouling
        TubeFoulThermResist =
            (0.5 * (waterCoil.TubeOutsideDiam - waterCoil.TubeInsideDiam) / (ConvK * waterCoil.TubeThermConductivity) + TubeFoulFactor) /
            waterCoil.TotTubeInsideArea;
        //              outside (wet and dry coil)
        FilmCoefEqnFactor = waterCoil.GeometryCoef1 * std::pow(AirReynoldsNo, waterCoil.GeometryCoef2);
        //       (1.23 is 1/Prandt(air)**(2/3))
        AirSideDrySurfFilmCoef = 1.23 * FilmCoefEqnFactor * MoistAirSpecificHeat * ScaledAirMassFlowRate;
        FilmCoefReynldsCorrelatnFact = 1.425 + AirReynoldsNo * (-0.51e-3 + AirReynoldsNo * 0.263e-6);
        //       NOTE: the equation for FilmCoefReynldsCorrelatnFact generates valid results over
        //             a limited range of Air Reynolds Numbers as indicated by
        //             deleted code below.  Reynolds Numbers outside this range
        //             may result in inaccurate results or failure of the coil
        //             simulation to obtain a solution
        //             Deleted code by J.C. Vanderzee

        AirSideWetSurfFilmCoef = FilmCoefReynldsCorrelatnFact * AirSideDrySurfFilmCoef;
        //--                     need wet fin efficiency for outside
        RaisedInletWaterTemp = TempWaterIn + 0.5;

        // By this statement the Inlet Air enthalpy will never be equal to AirEnthAtRsdInletWaterTemp
        if ((RaisedInletWaterTemp - TempAirIn) < 0.000001) {
            RaisedInletWaterTemp = TempWaterIn + 0.3;
        }
        if (TempAirIn < RaisedInletWaterTemp) {
            RaisedInletWaterTemp = TempAirIn - 0.3;
        }

        RsdInletWaterTempSatAirHumRat = PsyWFnTdbRhPb(state, RaisedInletWaterTemp, unity, state.dataEnvrn->OutBaroPress, RoutineName);
        AirEnthAtRsdInletWaterTemp = PsyHFnTdbW(RaisedInletWaterTemp, RsdInletWaterTempSatAirHumRat) * ConvK;

        SensToTotEnthDiffRatio = DryAirSpecHeat * (TempAirIn - RaisedInletWaterTemp) / (InletAirEnthalpy - AirEnthAtRsdInletWaterTemp);

        EnterAirHumRatDiff = InletAirHumRat - RsdInletWaterTempSatAirHumRat;
        DryFinEfficncy = 0.5 * (waterCoil.EffectiveFinDiam - waterCoil.TubeOutsideDiam) *
                         std::sqrt(2.0 * AirSideWetSurfFilmCoef / (ConvK * waterCoil.FinThermConductivity * waterCoil.FinThickness));
        if (EnterAirHumRatDiff < 0) {
            //       note that this condition indicates dry coil
            EnterAirHumRatDiff = -EnterAirHumRatDiff;
            SensToTotEnthDiffRatio = std::abs(SensToTotEnthDiffRatio);
        }

        if (EnterAirHumRatDiff > 1.0) {
            EnterAirHumRatDiff = 1.0;
        } else if (EnterAirHumRatDiff < 0.00001) {
            EnterAirHumRatDiff = 0.00001;
        }

        if (DryFinEfficncy > 1.0) {
            DryFinEfficncy = 1.0;
        } else if (DryFinEfficncy < 0.00001) {
            DryFinEfficncy = 0.00001;
        }

        if (TempAirIn > 48.0 / 1.8) {
            WetFinEfficncy =
                exp_47 * std::pow(SensToTotEnthDiffRatio, 0.09471) * std::pow(EnterAirHumRatDiff, 0.0108) * std::pow(DryFinEfficncy, -0.50303);
        } else {
            WetFinEfficncy =
                exp_35 * std::pow(SensToTotEnthDiffRatio, 0.16081) * std::pow(EnterAirHumRatDiff, 0.01995) * std::pow(DryFinEfficncy, -0.52951);
        }

        if (WetFinEfficncy > 1.0) {
            WetFinEfficncy = 0.99;
        }
        if (WetFinEfficncy < 0.0) {
            WetFinEfficncy = 0.001;
        }
        //       wet coil fin efficiency

        WetCoilFinEfficncy = 1.0 + FinToTotSurfAreaRatio * (WetFinEfficncy - 1.0);
        //       wet coil outside thermal resistance = [1/UA] (wet coil)
        CoilToAirThermResistWetSurf = MoistAirSpecificHeat / (waterCoil.TotCoilOutsideSurfArea * AirSideWetSurfFilmCoef * WetCoilFinEfficncy);
        //--                     and dry fin efficiency
        DryFinEfficncy = 0.5 * (waterCoil.EffectiveFinDiam - waterCoil.TubeOutsideDiam) *
                         std::sqrt(2.0 * AirSideDrySurfFilmCoef / (ConvK * waterCoil.FinThermConductivity * waterCoil.FinThickness));
        //      NOTE: The same caveats on the validity of the FilmCoefReynldsCorrelatnFact equation
        //            hold for the DryFinEfficncy equation.  Values of DryFinEfficncy outside the
        //            specified range of validity are not guaranteed to
        //            produce results
        //             Deleted code by J.C. Vanderzee
        //       dry coil fin efficiency
        DryCoilEfficiency = 0.0;
        // Tuned Replaced by below to eliminate pow calls
        //            for ( CoefPointer = 1; CoefPointer <= 5; ++CoefPointer ) {
        //                DryCoilEfficiency += WaterCoil( CoilNum ).DryFinEfficncyCoef( CoefPointer ) * std::pow(
        // DryFinEfficncy,
        // CoefPointer
        //-
        // 1
        //);             } // CoefPointer
        auto const &dry_fin_eff_coef = waterCoil.DryFinEfficncyCoef;
        Real64 DryFinEfficncy_pow = 1.0;
        for (int CoefPointer = 1; CoefPointer <= 5; ++CoefPointer) {
            DryCoilEfficiency += dry_fin_eff_coef(CoefPointer) * DryFinEfficncy_pow;
            DryFinEfficncy_pow *= DryFinEfficncy;
        } // CoefPointer
        DryCoilEfficiency = 1.0 + FinToTotSurfAreaRatio * (DryCoilEfficiency - 1.0);
        //       dry coil outside thermal resistance = [1/UA] (dry coil)
        CoilToAirThermResistDrySurf = 1.0 / (waterCoil.TotCoilOutsideSurfArea * AirSideDrySurfFilmCoef * DryCoilEfficiency);
        //       definitions made to simplify some of the expressions used below
        Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, TempWaterIn, RoutineName);
        ScaledWaterSpecHeat = WaterMassFlowRate * Cp * ConvK / AirMassFlow;
        DryCoilCoeff1 = 1.0 / (AirMassFlow * MoistAirSpecificHeat) - 1.0 / (WaterMassFlowRate * Cp * ConvK);
        //       perform initialisations for all wet solution
        WetSideEffctvWaterTemp = waterCoil.MeanWaterTempSaved + (TempWaterIn - waterCoil.InWaterTempSaved);
        int WaterTempConvgLoop = 0;
        bool WaterTempConvg = false;
        //       Loop to solve coil as if all wet, converges on MeanWaterTemp eq WetSideEffctvWaterTemp
        //       if conv=.TRUE. at any time program exits loop and proceeds
        //       to part wet / part dry solution
        while (WaterTempConvgLoop < 8 && !WaterTempConvg) {
            ++WaterTempConvgLoop;
            ScaledWaterToTubeThermResist = WaterToTubeThermResist / (1.0 + 0.0146 * WetSideEffctvWaterTemp);
            ScaledCoilAirThermResistWetSurf = CoilToAirThermResistWetSurf / waterCoil.SatEnthlCurveSlope;
            UACoilAllWet =
                1.0 / (waterCoil.SatEnthlCurveSlope * (TubeFoulThermResist + ScaledWaterToTubeThermResist + ScaledCoilAirThermResistWetSurf));
            //       prevents floating point error when taking exponential
            //       of a very large number
            expon = UACoilAllWet * (1.0 / AirMassFlow - waterCoil.SatEnthlCurveSlope / (WaterMassFlowRate * Cp * ConvK));
            if (expon < 20.0) { // CR7189 changed from ABS(expon) < 20
                //       negative expon can happen, but lead to tiny WetCoilCoef that aren't a problem
                WetCoilCoeff = std::exp(expon);
                // following appears similar to eq. 320 in Eng Ref but neglects K1 term
                TempWaterOut = ((1.0 - WetCoilCoeff) * (InletAirEnthalpy - waterCoil.SatEnthlCurveConstCoef) +
                                WetCoilCoeff * TempWaterIn * (waterCoil.SatEnthlCurveSlope - ScaledWaterSpecHeat)) /
                               (waterCoil.SatEnthlCurveSlope - WetCoilCoeff * ScaledWaterSpecHeat);
            } else {
                // following appears to be same as above with equation simplified to use only significant terms when WetCoilCoeff very large
                TempWaterOut =
                    ((InletAirEnthalpy - waterCoil.SatEnthlCurveConstCoef) - TempWaterIn * (waterCoil.SatEnthlCurveSlope - ScaledWaterSpecHeat)) /
                    ScaledWaterSpecHeat;
            }
            //      above is inverted form of WaterMassFlowRate*cpw*(TempWaterOut-TempWaterIn) = UA(LMHD)
            //      note simplification that hsat = WaterCoil(CoilNum)%SatEnthlCurveConstCoef +  &
            //                                      WaterCoil(CoilNum)%SatEnthlCurveSlope*WetSideEffctvWaterTemp
            MeanWaterTemp = 0.5 * (TempWaterIn + TempWaterOut);
            OutletAirEnthalpy = InletAirEnthalpy - (TempWaterOut - TempWaterIn) * ScaledWaterSpecHeat;

            InsdToOutsdThermResistRatio = (TubeFoulThermResist + ScaledWaterToTubeThermResist) / ScaledCoilAirThermResistWetSurf;
            InCoilSurfTemp =
                UACoilAllWet * ScaledCoilAirThermResistWetSurf *
                (waterCoil.SatEnthlCurveSlope * TempWaterIn + (OutletAirEnthalpy - waterCoil.SatEnthlCurveConstCoef) * InsdToOutsdThermResistRatio);
            OutCoilSurfTemp =
                UACoilAllWet * ScaledCoilAirThermResistWetSurf *
                (waterCoil.SatEnthlCurveSlope * TempWaterOut + (InletAirEnthalpy - waterCoil.SatEnthlCurveConstCoef) * InsdToOutsdThermResistRatio);

            if (std::abs(MeanWaterTemp - WetSideEffctvWaterTemp) > 0.01) {
                WetSideEffctvWaterTemp = MeanWaterTemp;
                InSurfTempSatAirEnthl = PsyHFnTdbRhPb(state, InCoilSurfTemp, unity, state.dataEnvrn->OutBaroPress, RoutineName) * ConvK;
                OutSurfTempSatAirEnthl = PsyHFnTdbRhPb(state, OutCoilSurfTemp, unity, state.dataEnvrn->OutBaroPress, RoutineName) * ConvK;

                waterCoil.SatEnthlCurveSlope = (OutSurfTempSatAirEnthl - InSurfTempSatAirEnthl) / (OutCoilSurfTemp - InCoilSurfTemp);
                waterCoil.SatEnthlCurveConstCoef = InSurfTempSatAirEnthl - waterCoil.SatEnthlCurveSlope * InCoilSurfTemp;
            } else {
                WaterTempConvg = true;
            }
        } // End of iteration loop to get MeanWaterTemp=WetSideEffctvWaterTemp
        //      if 8 CoolCoilErrs are reached without convergence and the
        //      predicted coil surface temperature at the outlet is less than
        //      the dew point coil is apparently all wet but a solution
        //      cannot be obtained
        if (!WaterTempConvg && !state.dataGlobal->WarmupFlag && (OutCoilSurfTemp < EnterAirDewPoint)) {
            ShowRecurringWarningErrorAtEnd(state,
                                           waterCoil.Name + " not converged (8 iterations) due to \"Wet Convergence\" conditions.",
                                           state.dataWaterCoils->WaterTempCoolCoilErrs(CoilNum),
                                           std::abs(MeanWaterTemp - WetSideEffctvWaterTemp),
                                           std::abs(MeanWaterTemp - WetSideEffctvWaterTemp));
            //       CoolCoilErrs = CoolCoilErrs + 1
            //       IF (CoolCoilErrs .LE. MaxCoolCoilErrs) THEN
            //          CALL ShowWarningError(state, 'tp12c0:  not converged in 8 CoolCoilErrs')
            //       END IF
        }
        waterCoil.MeanWaterTempSaved = MeanWaterTemp;
        //      now simulate wet dry coil - test outlet condition from all
        //      wet case to give an idea of the expected solution
        int PartWetIterations = 0;
        WetDryInterSurfTempError = 0.0;
        bool CoilPartWetConvg = false;
        //      Surface temp at coil water outlet (air inlet) is less than
        //      the dew point - Coil must be completely wet so no need to
        //      simulate wet/dry case
        if (OutCoilSurfTemp < EnterAirDewPoint) {
            CoilPartWetConvg = true;
            waterCoil.SurfAreaWetFraction = 1.0;
            TotWaterCoilLoad = AirMassFlow * (InletAirEnthalpy - OutletAirEnthalpy);
            AirWetDryInterfcTemp = TempAirIn;
            WetDryInterfcAirEnthl = InletAirEnthalpy;
            //      Surface temperature at coil water inlet is greater than the
            //      dewpoint - coil cannot be all wet but may be all dry -
            //      initialise with all dry solution
        } else if (InCoilSurfTemp > EnterAirDewPoint) {
            SurfAreaWet = 0.0;
            waterCoil.SurfAreaWetFraction = 0.0;
            WetDryInterfcWaterTemp = TempWaterIn;
            TempWaterOut = waterCoil.OutWaterTempSaved + (TempWaterIn - waterCoil.InWaterTempSaved);
            WetAreaLast = 0.05 * waterCoil.TotCoilOutsideSurfArea;
            //      General case - must be part-wet/part-dry - initialise
            //      accordingly with some non-zero wet area
        } else {
            if (waterCoil.SurfAreaWetSaved != 0.0) {
                SurfAreaWet = waterCoil.SurfAreaWetSaved;
            } else {
                SurfAreaWet = 0.8 * waterCoil.TotCoilOutsideSurfArea * (EnterAirDewPoint - InCoilSurfTemp) / (OutCoilSurfTemp - InCoilSurfTemp);
            }
            WetDryInterfcWaterTemp = TempWaterIn + EnterAirDewPoint - InCoilSurfTemp;
            WetAreaLast = 0.0;
        }
        //       Loop to solve partly wet coil, converges on wet area and
        //       boundary temperature at dew point
        //       Dry coil is special case with zero wet area, converges on
        //       mean water temperature
        while (PartWetIterations < 40 && !CoilPartWetConvg) {
            ++PartWetIterations;
            //      effective water temp on dry side of coil
            DrySideEffectiveWaterTemp = 0.5 * (TempWaterOut + WetDryInterfcWaterTemp);
            //      tube inside thermal resistance
            DryCoilInThermResist = WaterToTubeThermResist / (1.0 + 0.0146 * DrySideEffectiveWaterTemp);
            //      overall UA, from water to air, of dry portion of coil

            UADryCoil = (waterCoil.TotCoilOutsideSurfArea - SurfAreaWet) /
                        (waterCoil.TotCoilOutsideSurfArea * (TubeFoulThermResist + DryCoilInThermResist + CoilToAirThermResistDrySurf));

            // This is a numerical trap for a very small number in the EXP function that is approaching zero
            if (UADryCoil * DryCoilCoeff1 < -60.0) {
                DryCoilCoeff = 0.0;
            } else {
                DryCoilCoeff = std::exp(UADryCoil * DryCoilCoeff1);
            }

            K1 = WaterMassFlowRate * Cp * ConvK * (DryCoilCoeff - 1.0) /
                 (WaterMassFlowRate * Cp * ConvK * DryCoilCoeff - AirMassFlow * MoistAirSpecificHeat);
            if (SurfAreaWet != 0) {
                waterCoil.SurfAreaWetFraction = SurfAreaWet / waterCoil.TotCoilOutsideSurfArea;
                //      effective water temp on wet side of coil
                WetSideEffctvWaterTemp = 0.5 * (TempWaterIn + WetDryInterfcWaterTemp);
                //      tube inside thermal resistance
                ScaledWaterToTubeThermResist = WaterToTubeThermResist / (1.0 + 0.0146 * WetSideEffctvWaterTemp);
                ScaledCoilAirThermResistWetSurf = CoilToAirThermResistWetSurf / waterCoil.EnthVsTempCurveAppxSlope;
                //      overall UA, from water to air, of wet portion of coil
                UACoilAllWet = 1.0 / (waterCoil.EnthVsTempCurveAppxSlope *
                                      (TubeFoulThermResist + ScaledWaterToTubeThermResist + ScaledCoilAirThermResistWetSurf));
                UACoilPartWet = waterCoil.SurfAreaWetFraction * UACoilAllWet;
                expon = UACoilPartWet * (1.0 / AirMassFlow - waterCoil.EnthVsTempCurveAppxSlope / (WaterMassFlowRate * Cp * ConvK));
                //        prevents floating point error when taking exponential
                //        of a very large number
                if (expon < 20.0) {
                    WetCoilCoeff = std::exp(expon);
                    //          write(outputfiledebug,*) ' wcc=',wetcoilcoeff
                    denom =
                        (waterCoil.EnthVsTempCurveAppxSlope - WetCoilCoeff * ScaledWaterSpecHeat - (1.0 - WetCoilCoeff) * K1 * MoistAirSpecificHeat);
                    //          write(outputfiledebug,*) ' denom=',denom
                    //          WetDryInterfcWaterTemp = ((1.0 - WetCoilCoeff) * (InletAirEnthalpy - WaterCoil(CoilNum)%EnthVsTempCurveConst -
                    //          K1
                    //          *  &
                    //                                     MoistAirSpecificHeat * TempAirIn) + WetCoilCoeff * &
                    //                                     TempWaterIn * (WaterCoil(CoilNum)%EnthVsTempCurveAppxSlope -  &
                    //                                     ScaledWaterSpecHeat)) / (WaterCoil(CoilNum)%EnthVsTempCurveAppxSlope -  &
                    //                                      WetCoilCoeff * ScaledWaterSpecHeat - (1.0 - WetCoilCoeff) * K1 * &
                    //                                     MoistAirSpecificHeat)
                    WetDryInterfcWaterTemp =
                        ((1.0 - WetCoilCoeff) * (InletAirEnthalpy - waterCoil.EnthVsTempCurveConst - K1 * MoistAirSpecificHeat * TempAirIn) +
                         WetCoilCoeff * TempWaterIn * (waterCoil.EnthVsTempCurveAppxSlope - ScaledWaterSpecHeat)) /
                        denom;
                } else {
                    //         approximation to equation for WetDryInterfcWaterTemp when WetCoilCoeff-->inf.
                    WetDryInterfcWaterTemp = (TempWaterIn * (waterCoil.EnthVsTempCurveAppxSlope - ScaledWaterSpecHeat) -
                                              (InletAirEnthalpy - waterCoil.EnthVsTempCurveConst - K1 * MoistAirSpecificHeat * TempAirIn)) /
                                             (K1 * MoistAirSpecificHeat - ScaledWaterSpecHeat);
                }
            }
            //        air temperature at wet-dry interface
            AirWetDryInterfcTemp = TempAirIn - (TempAirIn - WetDryInterfcWaterTemp) * K1;
            //        coil surface temperature at wet-dry interface
            WetDryInterfcSurfTemp = WetDryInterfcWaterTemp + (AirWetDryInterfcTemp - WetDryInterfcWaterTemp) *
                                                                 (TubeFoulThermResist + DryCoilInThermResist) /
                                                                 (TubeFoulThermResist + DryCoilInThermResist + CoilToAirThermResistDrySurf);
            if (SurfAreaWet != 0) {
                WetDryInterfcAirEnthl = InletAirEnthalpy - MoistAirSpecificHeat * (TempAirIn - AirWetDryInterfcTemp);
                //        conservation of energy - wet portion of coil
                OutletAirEnthalpy = WetDryInterfcAirEnthl - WaterMassFlowRate * Cp * ConvK * (WetDryInterfcWaterTemp - TempWaterIn) / AirMassFlow;
                //        ratio of inside to outside thermal resistance
                InsdToOutsdThermResistRatio = (TubeFoulThermResist + ScaledWaterToTubeThermResist) / ScaledCoilAirThermResistWetSurf;
                //        coil surface temperature at water inlet (air outlet)
                InCoilSurfTemp = UACoilAllWet * ScaledCoilAirThermResistWetSurf *
                                 (waterCoil.EnthVsTempCurveAppxSlope * TempWaterIn +
                                  (OutletAirEnthalpy - waterCoil.EnthVsTempCurveConst) * InsdToOutsdThermResistRatio);
                WetDryInterSurfTempErrorLast = WetDryInterSurfTempError;
                //        in part-wet/part-dry solution EnterAirDewPoint=WetDryInterfcSurfTemp drives WetDryInterSurfTempError->0
                WetDryInterSurfTempError = EnterAirDewPoint - WetDryInterfcSurfTemp;
            } else {
                //        dry coil solution
                WetDryInterfcAirEnthl = 0.0;
                OutletAirEnthalpy = InletAirEnthalpy - MoistAirSpecificHeat * (TempAirIn - AirWetDryInterfcTemp);
            }
            //        total cooling = change in air enthalpy across coil
            TotWaterCoilLoad = AirMassFlow * (InletAirEnthalpy - OutletAirEnthalpy);
            //        conservation of energy on water stream gives water outlet
            //        temperature
            TempWaterOut = WaterMassFlowRate * Cp * ConvK; // Temp for next calc
            TempWaterOut = min(TempWaterIn + TotWaterCoilLoad / TempWaterOut, TempAirIn);
            //        update estimate of coil wet area

            if (SurfAreaWet == 0) {
                MeanWaterTemp = 0.5 * (TempWaterOut + WetDryInterfcWaterTemp);
                if (EnterAirDewPoint > WetDryInterfcSurfTemp) {
                    SurfAreaWet = 0.5 * WetAreaLast;
                } else if (std::abs(MeanWaterTemp - DrySideEffectiveWaterTemp) <= 0.00002) {
                    CoilPartWetConvg = true;
                }
            } else if (std::abs(WetDryInterSurfTempError) > 0.00002 ||
                       std::abs(SurfAreaWet - WetAreaLast) / waterCoil.TotCoilOutsideSurfArea > 0.00001) {
                if (WetAreaLast == 0) {
                    WetAreaLast = SurfAreaWet;
                    SurfAreaWet += 0.4 * waterCoil.TotCoilOutsideSurfArea * WetDryInterSurfTempError / (OutCoilSurfTemp - InCoilSurfTemp);
                } else if (WetDryInterSurfTempError != WetDryInterSurfTempErrorLast) {
                    WetAreaChange = SurfAreaWet - WetAreaLast;
                    WetAreaLast = SurfAreaWet;
                    SurfAreaWet -= 0.8 * WetDryInterSurfTempError * WetAreaChange / (WetDryInterSurfTempError - WetDryInterSurfTempErrorLast);
                }
                if (SurfAreaWet >= waterCoil.TotCoilOutsideSurfArea) {
                    SurfAreaWet = waterCoil.TotCoilOutsideSurfArea;
                    MeanWaterTemp = 0.5 * (TempWaterIn + WetDryInterfcWaterTemp);
                    if (WetAreaLast == waterCoil.TotCoilOutsideSurfArea && std::abs(MeanWaterTemp - WetSideEffctvWaterTemp) <= 0.00002) {
                        CoilPartWetConvg = true;
                    }
                }
                if (SurfAreaWet <= 0) {
                    SurfAreaWet = 0.0;
                    waterCoil.SurfAreaWetFraction = 0.0;
                    WetDryInterfcWaterTemp = TempWaterIn;
                }
                InSurfTempSatAirEnthl = PsyHFnTdbRhPb(state, InCoilSurfTemp, unity, state.dataEnvrn->OutBaroPress, RoutineName) * ConvK;
                if ((EnterAirDewPoint - InCoilSurfTemp) >= 0.0001) {
                    AirEnthAtWetDryIntrfcSurfTemp = PsyHFnTdbRhPb(state, EnterAirDewPoint, unity, state.dataEnvrn->OutBaroPress, RoutineName) * ConvK;
                    waterCoil.EnthVsTempCurveAppxSlope =
                        (AirEnthAtWetDryIntrfcSurfTemp - InSurfTempSatAirEnthl) / (EnterAirDewPoint - InCoilSurfTemp);
                } else {
                    AirEnthAtWetDryIntrfcSurfTemp =
                        PsyHFnTdbRhPb(state, InCoilSurfTemp + 0.0001, unity, state.dataEnvrn->OutBaroPress, RoutineName) * ConvK;
                    waterCoil.EnthVsTempCurveAppxSlope = (AirEnthAtWetDryIntrfcSurfTemp - InSurfTempSatAirEnthl) / 0.0001;
                }
                waterCoil.EnthVsTempCurveConst = InSurfTempSatAirEnthl - waterCoil.EnthVsTempCurveAppxSlope * InCoilSurfTemp;
            } else {
                CoilPartWetConvg = true;
            }
        }
        //      error checking to see if convergence has been achieved
        if (!CoilPartWetConvg && !state.dataGlobal->WarmupFlag) {
            ShowRecurringWarningErrorAtEnd(state,
                                           waterCoil.Name + " not converged (40 iterations) due to \"Partial Wet Convergence\" conditions.",
                                           state.dataWaterCoils->PartWetCoolCoilErrs(CoilNum));
            //      CoolCoilErrs = CoolCoilErrs + 1
            //      IF (CoolCoilErrs .LE. MaxCoolCoilErrs) THEN
            //        CALL ShowWarningError(state, 'tp12c0:  not converged in 20 CoolCoilErrs')
            //      END IF
        }
        if (waterCoil.SurfAreaWetFraction > 0 && waterCoil.SurfAreaWetFraction < 1) {
            waterCoil.SurfAreaWetSaved = SurfAreaWet;
        }
        //       calculate TempAirOut, OutletAirHumRat, and SensCoolRate based on equations from
        //       TYPE12 and the ASHRAE toolkit
        if (waterCoil.SurfAreaWetFraction == 0) {
            //       dry coil
            TempAirOut = TempAirIn - TotWaterCoilLoad / (AirMassFlow * MoistAirSpecificHeat);
            OutletAirHumRat = InletAirHumRat;
            SenWaterCoilLoad = TotWaterCoilLoad;
        } else {
            //       coil effectiveness
            expon = waterCoil.SurfAreaWetFraction / (CoilToAirThermResistWetSurf * AirMassFlow);
            y = 0.0;
            if (expon < 20.0) {
                y = std::exp(-expon);
            }
            AirExitEnthlAtCoilSurfTemp = WetDryInterfcAirEnthl - (WetDryInterfcAirEnthl - OutletAirEnthalpy) / (1.0 - y);
            AirExitCoilSurfTemp = AirExitEnthlAtCoilSurfTemp / ConvK; // TEmporary calc
            AirExitCoilSurfTemp = PsyTsatFnHPb(state, AirExitCoilSurfTemp, state.dataEnvrn->OutBaroPress);
            //       Implementation of epsilon*NTU method
            TempAirOut = AirExitCoilSurfTemp + (AirWetDryInterfcTemp - AirExitCoilSurfTemp) * y;
            OutletAirHumRat = PsyWFnTdbH(state, TempAirOut, 1000.0 * OutletAirEnthalpy, RoutineName);
            SenWaterCoilLoad = AirMassFlow * (PsyCpAirFnW(InletAirHumRat) * TempAirIn - PsyCpAirFnW(OutletAirHumRat) * TempAirOut) * ConvK;
        }

        if (fanOp == HVAC::FanOp::Cycling) {
            TotWaterCoilLoad *= PartLoadRatio;
            SenWaterCoilLoad *= PartLoadRatio;
        }

        // Set the outlet conditions
        waterCoil.TotWaterCoolingCoilRate = TotWaterCoilLoad * 1000.0;
        waterCoil.SenWaterCoolingCoilRate = SenWaterCoilLoad * 1000.0;
        waterCoil.OutletAirTemp = TempAirOut;
        waterCoil.OutletWaterTemp = TempWaterOut;
        waterCoil.OutletAirEnthalpy = OutletAirEnthalpy * 1000.0;
        waterCoil.OutletAirHumRat = OutletAirHumRat;
        // The CoolingCoilLoad is the change in the enthalpy of the water
        waterCoil.OutletWaterEnthalpy = waterCoil.InletWaterEnthalpy + waterCoil.TotWaterCoolingCoilRate / waterCoil.InletWaterMassFlowRate;

        // This WaterCoil does not change the Mass Flow across the component
        waterCoil.OutletAirMassFlowRate = waterCoil.InletAirMassFlowRate;
        waterCoil.OutletWaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    } else {
        // If Coil is scheduled OFF then Outlet conditions are set to Inlet Conditions
        waterCoil.TotWaterCoolingCoilRate = 0.0;
        waterCoil.SenWaterCoolingCoilRate = 0.0;
        TempAirOut = TempAirIn;
        TempWaterOut = TempWaterIn;
        // set the outlet conditions to the coil derived type
        waterCoil.OutletAirTemp = TempAirOut;
        waterCoil.OutletWaterTemp = TempWaterOut;
        waterCoil.OutletAirEnthalpy = waterCoil.InletAirEnthalpy;
        waterCoil.OutletAirHumRat = waterCoil.InletAirHumRat;
        // The CoolingCoilLoad is the change in the enthalpy of the water
        waterCoil.OutletWaterEnthalpy = waterCoil.InletWaterEnthalpy;

        // This WaterCoil does not change the Mass Flow across the component
        waterCoil.OutletAirMassFlowRate = waterCoil.InletAirMassFlowRate;
        waterCoil.OutletWaterMassFlowRate = 0.0;
    }

    // Save some of the Values for next Time step
    waterCoil.InWaterTempSaved = TempWaterIn;
    waterCoil.OutWaterTempSaved = TempWaterOut;
}

void CoolingCoil(EnergyPlusData &state,
                 int const CoilNum,
                 bool const FirstHVACIteration,
                 int const CalcMode,
                 HVAC::FanOp const fanOp,   // fan operating mode
                 Real64 const PartLoadRatio // part-load ratio of heating coil
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   Mar 2004

    // PURPOSE OF THIS FUNCTION:
    // The subroutine has the coil logic. Three types of Cooling Coils exist:
    // They are 1.CoilDry , 2.CoilWet, 3. CoilPartDryPartWet. The logic for
    // the three individual cases is in this subroutine.

    // METHODOLOGY EMPLOYED:
    // Simulates a Coil Model from Design conditions and subsequently uses
    // configuration values (example: UA)calculated from those design conditions
    // to calculate new performance of coil from operating inputs.The values are
    // calculated in the Subroutine InitWaterCoil

    // REFERENCES:
    // ASHRAE Secondary HVAC Toolkit TRNSYS.  1990.  A Transient System
    // Simulation Program: Reference Manual. Solar Energy Laboratory, Univ. Wisconsin-
    // Madison, pp. 4.6.8-1 - 4.6.8-12.
    // Threlkeld, J.L.  1970.  Thermal Environmental Engineering, 2nd Edition,
    // Englewood Cliffs: Prentice-Hall,Inc. pp. 254-270.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 AirInletCoilSurfTemp; // Coil surface temperature at air entrance(C)
    Real64 AirDewPointTemp;      // Temperature dew point at operating condition
    Real64 OutletAirTemp;        // Outlet air temperature at operating condition
    Real64 OutletAirHumRat;      // Outlet air humidity ratio at operating condition
    Real64 OutletWaterTemp;      // Outlet water temperature at operating condtitons
    Real64 TotWaterCoilLoad;     // Total heat transfer rate(W)
    Real64 SenWaterCoilLoad;     // Sensible heat transfer rate
    Real64 SurfAreaWetFraction;  // Fraction of surface area wet
    Real64 AirMassFlowRate;      // Air mass flow rate for the calculation

    AirInletCoilSurfTemp = 0.0; // Coil surface temperature at air entrance(C)
    AirDewPointTemp = 0.0;      // Temperature dew point at operating condition
    OutletAirTemp = 0.0;        // Outlet air temperature at operating condition
    OutletAirHumRat = 0.0;      // Outlet air humidity ratio at operating condition
    OutletWaterTemp = 0.0;      // Outlet water temperature at operating condtitons
    TotWaterCoilLoad = 0.0;     // Total heat transfer rate(W)
    SenWaterCoilLoad = 0.0;     // Sensible heat transfer rate
    SurfAreaWetFraction = 0.0;  // Fraction of surface area wet

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    if (fanOp == HVAC::FanOp::Cycling && PartLoadRatio > 0.0) { // FB Start
        AirMassFlowRate = waterCoil.InletAirMassFlowRate / PartLoadRatio;
    } else {
        AirMassFlowRate = waterCoil.InletAirMassFlowRate;
    }

    // If Coil is Scheduled ON then do the simulation
    if (((waterCoil.availSched->getCurrentVal() > 0.0) && (waterCoil.InletWaterMassFlowRate > 0.0) &&
         (AirMassFlowRate >= WaterCoils::MinAirMassFlow) && (waterCoil.DesAirVolFlowRate > 0.0) && (waterCoil.MaxWaterMassFlowRate > 0.0)) ||
        (CalcMode == state.dataWaterCoils->DesignCalc)) {

        // Calculate Temperature Dew Point at operating conditions.
        AirDewPointTemp = PsyTdpFnWPb(state, waterCoil.InletAirHumRat, state.dataEnvrn->OutBaroPress);

        if (waterCoil.CoolingCoilAnalysisMode == state.dataWaterCoils->DetailedAnalysis) {
            // Coil is completely dry if AirDewPointTemp is less than InletWaterTemp,hence Call CoilCompletelyDry
            if (AirDewPointTemp <= waterCoil.InletWaterTemp) {

                // Calculate the leaving conditions and performance of dry coil
                CoilCompletelyDry(state,
                                  CoilNum,
                                  waterCoil.InletWaterTemp,
                                  waterCoil.InletAirTemp,
                                  waterCoil.UACoilTotal,
                                  OutletWaterTemp,
                                  OutletAirTemp,
                                  OutletAirHumRat,
                                  TotWaterCoilLoad,
                                  fanOp,
                                  PartLoadRatio);

                SenWaterCoilLoad = TotWaterCoilLoad;
                SurfAreaWetFraction = 0.0;

            } else {
                // Else If AirDewPointTemp is greater than InletWaterTemp then assume the
                // external surface of coil is completely wet,hence Call CoilCompletelyWet
                // Calculate the leaving conditions and performance of wet coil
                CoilCompletelyWet(state,
                                  CoilNum,
                                  waterCoil.InletWaterTemp,
                                  waterCoil.InletAirTemp,
                                  waterCoil.InletAirHumRat,
                                  waterCoil.UACoilInternal,
                                  waterCoil.UACoilExternal,
                                  OutletWaterTemp,
                                  OutletAirTemp,
                                  OutletAirHumRat,
                                  TotWaterCoilLoad,
                                  SenWaterCoilLoad,
                                  SurfAreaWetFraction,
                                  AirInletCoilSurfTemp,
                                  fanOp,
                                  PartLoadRatio);

                // If AirDewPointTemp is less than temp of coil surface at entry of air
                if (AirDewPointTemp < AirInletCoilSurfTemp) {

                    // Then coil is partially wet and dry hence call CoilPartWetPartDry
                    // Calculate the leaving conditions and performance of dry coil
                    CoilPartWetPartDry(state,
                                       CoilNum,
                                       FirstHVACIteration,
                                       waterCoil.InletWaterTemp,
                                       waterCoil.InletAirTemp,
                                       AirDewPointTemp,
                                       OutletWaterTemp,
                                       OutletAirTemp,
                                       OutletAirHumRat,
                                       TotWaterCoilLoad,
                                       SenWaterCoilLoad,
                                       SurfAreaWetFraction,
                                       fanOp,
                                       PartLoadRatio);

                } // End if for part wet part dry coil
            } // End if for dry coil

        } else if (waterCoil.CoolingCoilAnalysisMode == state.dataWaterCoils->SimpleAnalysis) {
            // Coil is completely dry if AirDewPointTemp is less than InletWaterTemp,hence Call CoilCompletelyDry
            if (AirDewPointTemp <= waterCoil.InletWaterTemp) {

                // Calculate the leaving conditions and performance of dry coil
                CoilCompletelyDry(state,
                                  CoilNum,
                                  waterCoil.InletWaterTemp,
                                  waterCoil.InletAirTemp,
                                  waterCoil.UACoilTotal,
                                  OutletWaterTemp,
                                  OutletAirTemp,
                                  OutletAirHumRat,
                                  TotWaterCoilLoad,
                                  fanOp,
                                  PartLoadRatio);

                SenWaterCoilLoad = TotWaterCoilLoad;
                SurfAreaWetFraction = 0.0;

            } else {
                // Else If AirDewPointTemp is greater than InletWaterTemp then assume the
                // external surface of coil is completely wet,hence Call CoilCompletelyWet
                // Calculate the leaving conditions and performance of wet coil
                CoilCompletelyWet(state,
                                  CoilNum,
                                  waterCoil.InletWaterTemp,
                                  waterCoil.InletAirTemp,
                                  waterCoil.InletAirHumRat,
                                  waterCoil.UACoilInternal,
                                  waterCoil.UACoilExternal,
                                  OutletWaterTemp,
                                  OutletAirTemp,
                                  OutletAirHumRat,
                                  TotWaterCoilLoad,
                                  SenWaterCoilLoad,
                                  SurfAreaWetFraction,
                                  AirInletCoilSurfTemp,
                                  fanOp,
                                  PartLoadRatio);

            } // End if for dry coil
        }

        // Report outlet variables at nodes
        waterCoil.OutletAirTemp = OutletAirTemp;
        waterCoil.OutletAirHumRat = OutletAirHumRat;
        waterCoil.OutletWaterTemp = OutletWaterTemp;
        // Report output results if the coil was operating

        if (fanOp == HVAC::FanOp::Cycling) {
            TotWaterCoilLoad *= PartLoadRatio;
            SenWaterCoilLoad *= PartLoadRatio;
        }

        waterCoil.TotWaterCoolingCoilRate = TotWaterCoilLoad;
        waterCoil.SenWaterCoolingCoilRate = SenWaterCoilLoad;
        waterCoil.SurfAreaWetFraction = SurfAreaWetFraction;
        //       WaterCoil(CoilNum)%OutletWaterEnthalpy = WaterCoil(CoilNum)%InletWaterEnthalpy+ &
        //                                WaterCoil(CoilNum)%TotWaterCoolingCoilRate/WaterCoil(CoilNum)%InletWaterMassFlowRate
        waterCoil.OutletWaterEnthalpy =
            waterCoil.InletWaterEnthalpy + General::SafeDivide(waterCoil.TotWaterCoolingCoilRate, waterCoil.InletWaterMassFlowRate);

    } else {
        // If both mass flow rates are zero, set outputs to inputs and return
        waterCoil.OutletWaterTemp = waterCoil.InletWaterTemp;
        waterCoil.OutletAirTemp = waterCoil.InletAirTemp;
        waterCoil.OutletAirHumRat = waterCoil.InletAirHumRat;
        waterCoil.OutletWaterEnthalpy = waterCoil.InletWaterEnthalpy;
        waterCoil.TotWaterCoolingCoilEnergy = 0.0;
        waterCoil.SenWaterCoolingCoilEnergy = 0.0;
        waterCoil.SurfAreaWetFraction = 0.0;

    } // End of the Flow or No flow If block
    waterCoil.OutletWaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    waterCoil.OutletAirMassFlowRate = waterCoil.InletAirMassFlowRate;
    waterCoil.OutletAirEnthalpy = PsyHFnTdbW(waterCoil.OutletAirTemp, waterCoil.OutletAirHumRat);
}

// End Algorithm Section of the Module

// Coil Completely Dry Subroutine for Cooling Coil

void CoilCompletelyDry(EnergyPlusData &state,
                       int const CoilNum,
                       Real64 const WaterTempIn,  // Entering water temperature
                       Real64 const AirTempIn,    // Entering air dry bulb temperature
                       Real64 const CoilUA,       // Overall heat transfer coefficient
                       Real64 &OutletWaterTemp,   // Leaving water temperature
                       Real64 &OutletAirTemp,     // Leaving air dry bulb temperature
                       Real64 &OutletAirHumRat,   // Leaving air humidity ratio
                       Real64 &Q,                 // Heat transfer rate
                       HVAC::FanOp const fanOp,   // fan operating mode
                       Real64 const PartLoadRatio // part-load ratio of heating coil
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   March 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the performance of a sensible air-liquid heat exchanger.  Calculated
    // results include outlet air temperature and humidity, outlet water temperature,
    // and heat transfer rate.

    // METHODOLOGY EMPLOYED:
    // Models coil using effectiveness-NTU model.

    // REFERENCES:
    // Kays, W.M. and A.L. London.  1964,Compact Heat Exchangers, 2nd Edition,
    // New York: McGraw-Hill.

    // FUNCTION PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CoilCompletelyDry");

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 CapacitanceAir;   // Air-side capacity rate(W/C)
    Real64 CapacitanceWater; // Water-side capacity rate(W/C)
    Real64 AirMassFlow;
    Real64 WaterMassFlowRate;
    Real64 Cp;

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    //  adjust mass flow rates for cycling fan cycling coil operation
    if (fanOp == HVAC::FanOp::Cycling) {
        if (PartLoadRatio > 0.0) {
            AirMassFlow = waterCoil.InletAirMassFlowRate / PartLoadRatio;
            WaterMassFlowRate = min(waterCoil.InletWaterMassFlowRate / PartLoadRatio, waterCoil.MaxWaterMassFlowRate);
        } else {
            AirMassFlow = 0.0;
            WaterMassFlowRate = 0.0;
        }
    } else {
        AirMassFlow = waterCoil.InletAirMassFlowRate;
        WaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    }

    // Calculate air and water capacity rates
    CapacitanceAir = AirMassFlow * PsyCpAirFnW(waterCoil.InletAirHumRat);
    // Water Capacity Rate
    Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, WaterTempIn, RoutineName);

    CapacitanceWater = WaterMassFlowRate * Cp;

    // Determine the air and water outlet conditions
    CoilOutletStreamCondition(state, CoilNum, CapacitanceWater, WaterTempIn, CapacitanceAir, AirTempIn, CoilUA, OutletWaterTemp, OutletAirTemp);

    // Calculate the total and sensible heat transfer rate both are equal in case of Dry Coil
    Q = CapacitanceAir * (AirTempIn - OutletAirTemp);

    // Outlet humidity is equal to Inlet Humidity because its a dry coil
    OutletAirHumRat = waterCoil.InletAirHumRat;
}

// Coil Completely Wet Subroutine for Cooling Coil

void CoilCompletelyWet(EnergyPlusData &state,
                       int const CoilNum,            // Number of Coil
                       Real64 const WaterTempIn,     // Water temperature IN to this function (C)
                       Real64 const AirTempIn,       // Air dry bulb temperature IN to this function(C)
                       Real64 const AirHumRat,       // Air Humidity Ratio IN to this function (C)
                       Real64 const UAInternalTotal, // Internal overall heat transfer coefficient(W/m2 C)
                       Real64 const UAExternalTotal, // External overall heat transfer coefficient(W/m2 C)
                       Real64 &OutletWaterTemp,      // Leaving water temperature (C)
                       Real64 &OutletAirTemp,        // Leaving air dry bulb temperature(C)
                       Real64 &OutletAirHumRat,      // Leaving air humidity ratio
                       Real64 &TotWaterCoilLoad,     // Total heat transfer rate(W)
                       Real64 &SenWaterCoilLoad,     // Sensible heat transfer rate(W)
                       Real64 &SurfAreaWetFraction,  // Fraction of surface area wet
                       Real64 &AirInletCoilSurfTemp, // Surface temperature at air entrance(C)
                       HVAC::FanOp const fanOp,      // fan operating mode
                       Real64 const PartLoadRatio    // part-load ratio of heating coil
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   Mar 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the performance of a cooling coil when the external fin surface is
    // complete wet.  Results include outlet air temperature and humidity,
    // outlet water temperature, sensible and total cooling capacities, and the wet
    // fraction of the air-side surface area.

    // METHODOLOGY EMPLOYED:
    // Models coil as counterflow heat exchanger. Approximates saturated air enthalpy as
    // a linear function of temperature
    // TRNSYS.  1990.  A Transient System Simulation Program: Reference Manual.
    // Solar Energy Laboratory, Univ. Wisconsin Madison, pp. 4.6.8-1 - 4.6.8-12.
    // Threlkeld, J.L.  1970.  Thermal Environmental Engineering, 2nd Edition,
    // Englewood Cliffs: Prentice-Hall,Inc. pp. 254-270.
    // Coil Uses Enthalpy Based Heat Transfer Coefficients and converts them to
    // conventional UA values. Intermediate value of fictitious Cp is defined. This follow
    // the same procedure followed in the Design Calculation of the Coil. See the node in
    // the one time calculation for design condition.

    // REFERENCES:
    // Elmahdy, A.H. and Mitalas, G.P.  1977."A Simple Model for Cooling and
    // Dehumidifying Coils for Use In Calculating Energy Requirements for Buildings,"
    // ASHRAE Transactions,Vol.83 Part 2, pp. 103-117.

    // FUNCTION PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CoilCompletelyWet");

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 AirSideResist;                  // Air-side resistance to heat transfer(m2 C/W)
    Real64 WaterSideResist;                // Liquid-side resistance to heat transfer(m2 C/W)
    Real64 EnteringAirDewPt;               // Entering air dew point(C)
    Real64 UACoilTotalEnth;                // Overall enthalpy heat transfer coefficient(kg/s)
    Real64 CapacityRateAirWet;             // Air-side capacity rate(kg/s)
    Real64 CapacityRateWaterWet;           // Liquid-side capacity rate(kg/s)
    Real64 ResistRatio;                    // Ratio of resistances
    Real64 EnthAirOutlet;                  // Outlet air enthalpy
    Real64 EnthSatAirInletWaterTemp;       // Saturated enthalpy of air at entering water temperature(J/kg)
    Real64 EnthSatAirOutletWaterTemp;      // Saturated enthalpy of air at exit water temperature(J/kg)
    Real64 EnthSatAirCoilSurfaceEntryTemp; // Saturated enthalpy of air at entering surface temperature(J/kg)
    Real64 EnthSatAirCoilSurfaceExitTemp;  // Saturated enthalpy of air at exit surface temperature(J/kg)
    Real64 EnthAirInlet;                   // Enthalpy of air at inlet
    Real64 IntermediateCpSat;              // Coefficient for equation below(J/kg C)
    // EnthSat1-EnthSat2 = IntermediateCpSat*(TSat1-TSat2)
    // (all water and surface temperatures are
    // related to saturated air enthalpies for
    // wet surface heat transfer calculations)
    Real64 constexpr SmallNo(1.e-9); // smallNo used in place of 0
    Real64 AirMassFlow;
    Real64 WaterMassFlowRate;
    Real64 Cp;

    SurfAreaWetFraction = 1.0;
    AirSideResist = 1.0 / max(UAExternalTotal, SmallNo);
    WaterSideResist = 1.0 / max(UAInternalTotal, SmallNo);

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    //  adjust mass flow rates for cycling fan cycling coil operation
    if (fanOp == HVAC::FanOp::Cycling) {
        if (PartLoadRatio > 0.0) {
            AirMassFlow = waterCoil.InletAirMassFlowRate / PartLoadRatio;
            WaterMassFlowRate = min(waterCoil.InletWaterMassFlowRate / PartLoadRatio, waterCoil.MaxWaterMassFlowRate);
        } else {
            AirMassFlow = 0.0;
            WaterMassFlowRate = 0.0;
        }
    } else {
        AirMassFlow = waterCoil.InletAirMassFlowRate;
        WaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    }

    // Calculate enthalpies of entering air and water

    // Enthalpy of air at inlet to the coil
    EnthAirInlet = PsyHFnTdbW(AirTempIn, AirHumRat);

    // Saturation Enthalpy of Air at inlet water temperature
    EnthSatAirInletWaterTemp = PsyHFnTdbW(WaterTempIn, PsyWFnTdpPb(state, WaterTempIn, state.dataEnvrn->OutBaroPress));

    // Estimate IntermediateCpSat using entering air dewpoint and water temperature
    EnteringAirDewPt = PsyTdpFnWPb(state, AirHumRat, state.dataEnvrn->OutBaroPress);

    // An intermediate value of Specific heat . EnthSat1-EnthSat2 = IntermediateCpSat*(TSat1-TSat2)
    IntermediateCpSat =
        (PsyHFnTdbW(EnteringAirDewPt, PsyWFnTdpPb(state, EnteringAirDewPt, state.dataEnvrn->OutBaroPress)) - EnthSatAirInletWaterTemp) /
        (EnteringAirDewPt - WaterTempIn);

    // Determine air and water enthalpy outlet conditions by modeling
    // coil as counterflow enthalpy heat exchanger
    UACoilTotalEnth = 1.0 / (IntermediateCpSat * WaterSideResist + AirSideResist * PsyCpAirFnW(0.0));
    CapacityRateAirWet = AirMassFlow;
    Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, WaterTempIn, RoutineName);
    CapacityRateWaterWet = WaterMassFlowRate * (Cp / IntermediateCpSat);
    CoilOutletStreamCondition(state,
                              CoilNum,
                              CapacityRateAirWet,
                              EnthAirInlet,
                              CapacityRateWaterWet,
                              EnthSatAirInletWaterTemp,
                              UACoilTotalEnth,
                              EnthAirOutlet,
                              EnthSatAirOutletWaterTemp);

    // Calculate entering and leaving external surface conditions from
    // air and water conditions and the ratio of resistances
    ResistRatio = (WaterSideResist) / (WaterSideResist + PsyCpAirFnW(0.0) / IntermediateCpSat * AirSideResist);
    EnthSatAirCoilSurfaceEntryTemp = EnthSatAirOutletWaterTemp + ResistRatio * (EnthAirInlet - EnthSatAirOutletWaterTemp);
    EnthSatAirCoilSurfaceExitTemp = EnthSatAirInletWaterTemp + ResistRatio * (EnthAirOutlet - EnthSatAirInletWaterTemp);

    // Calculate Coil Surface Temperature at air entry to the coil
    AirInletCoilSurfTemp = PsyTsatFnHPb(state, EnthSatAirCoilSurfaceEntryTemp, state.dataEnvrn->OutBaroPress);

    // Calculate outlet air temperature and humidity from enthalpies and surface conditions.
    TotWaterCoilLoad = AirMassFlow * (EnthAirInlet - EnthAirOutlet);
    OutletWaterTemp = WaterTempIn + TotWaterCoilLoad / max(WaterMassFlowRate, SmallNo) / Cp;

    // Calculates out put variable for  the completely wet coil
    WetCoilOutletCondition(state, CoilNum, AirTempIn, EnthAirInlet, EnthAirOutlet, UAExternalTotal, OutletAirTemp, OutletAirHumRat, SenWaterCoilLoad);
}

// Coil Part Wet Part Dry Subroutine for Cooling Coil

void CoilPartWetPartDry(EnergyPlusData &state,
                        int const CoilNum,             // Number of Coil
                        bool const FirstHVACIteration, // Saving Old values
                        Real64 const InletWaterTemp,   // Entering liquid temperature(C)
                        Real64 const InletAirTemp,     // Entering air dry bulb temperature(C)
                        Real64 const AirDewPointTemp,  // Entering air dew point(C)
                        Real64 &OutletWaterTemp,       // Leaving liquid temperature(C)
                        Real64 &OutletAirTemp,         // Leaving air dry bulb temperature(C)
                        Real64 &OutletAirHumRat,       // Leaving air humidity ratio
                        Real64 &TotWaterCoilLoad,      // Total heat transfer rate (W)
                        Real64 &SenWaterCoilLoad,      // Sensible heat transfer rate (W)
                        Real64 &SurfAreaWetFraction,   // Fraction of surface area wet
                        HVAC::FanOp const fanOp,       // fan operating mode
                        Real64 const PartLoadRatio     // part-load ratio of heating coil
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   March 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the performance of a cooling  coil when the external fin surface is
    // part wet and part dry.  Results include outlet air temperature and humidity,
    // outlet liquid temperature, sensible and total cooling capacities, and the wet
    // fraction of the air-side surface area.

    // METHODOLOGY EMPLOYED:
    // Models coil using effectiveness NTU model

    // REFERENCES:
    // Elmahdy, A.H. and Mitalas, G.P.  1977. "A Simple Model for Cooling and
    // Dehumidifying Coils for Use In Calculating Energy Requirements for Buildings,"
    // ASHRAE Transactions,Vol.83 Part 2, pp. 103-117.
    // TRNSYS.  1990.  A Transient System Simulation Program: Reference Manual.
    // Solar Energy Laboratory, Univ. Wisconsin- Madison, pp. 4.6.8-1 - 4.6.8-12.
    // Threlkeld, J.L.  1970.  Thermal Environmental Engineering, 2nd Edition,
    // Englewood Cliffs: Prentice-Hall,Inc. pp. 254-270.

    // FUNCTION PARAMETER DEFINITIONS:
    int constexpr itmax(60);
    Real64 constexpr smalltempdiff(1.0e-9);

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 DryCoilHeatTranfer;             // Heat transfer rate for dry coil(W)
    Real64 WetCoilTotalHeatTransfer;       // Total heat transfer rate for wet coil(W)
    Real64 WetCoilSensibleHeatTransfer;    // Sensible heat transfer rate for wet coil(W)
    Real64 SurfAreaWet;                    // Air-side area of wet coil(m2)
    Real64 SurfAreaDry;                    // Air-side area of dry coil(m2)
    Real64 DryCoilUA;                      // Overall heat transfer coefficient for dry coil(W/C)
    Real64 WetDryInterfcWaterTemp;         // Liquid temperature at wet/dry boundary(C)
    Real64 WetDryInterfcAirTemp;           // Air temperature at wet/dry boundary(C)
    Real64 WetDryInterfcSurfTemp;          // Surface temperature at wet/dry boundary(C)
    Real64 EstimateWetDryInterfcWaterTemp; // Estimated liquid temperature at wet/dry boundary(C)
    Real64 EstimateSurfAreaWetFraction;    // Initial Estimate for Fraction of Surface Wet with condensation
    Real64 WetPartUAInternal;              // UA of Wet Coil Internal
    Real64 WetPartUAExternal;              // UA of Dry Coil External
    Real64 WetDryInterfcHumRat;            // Humidity Ratio at interface of the wet dry transition
    Real64 X1T;                            // Variables used in the two iteration in this subroutine.
    Real64 NewSurfAreaWetFrac;             // Variables used in the two iteration in this subroutine.
    Real64 ResultXT;                       // Variables used in the two iteration in this subroutine.
    Real64 Y1T;                            // Variables used in the two iterations in this subroutine.
    Real64 errorT;                         // Error in interation for First If loop
    Real64 error;                          // Deviation of dependent variable in iteration
    Real64 SurfAreaFracPrevious;
    Real64 ErrorPrevious;
    Real64 SurfAreaFracLast;
    Real64 ErrorLast;
    int iter;  // Iteration counter
    int icvg;  // Iteration convergence flag
    int icvgT; // Iteration Convergence Flag for First If loop
    int itT;   // Iteration Counter for First If Loop

    // Iterates on SurfAreaWetFraction to converge on surface temperature equal to
    // entering air dewpoint at wet/dry boundary.

    // Preliminary estimates of coil performance to begin iteration
    OutletWaterTemp = InletAirTemp;
    DryCoilHeatTranfer = 0.0;
    WetCoilTotalHeatTransfer = 0.0;
    WetCoilSensibleHeatTransfer = 0.0;

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    if (FirstHVACIteration) {
        // Estimate liquid temperature at boundary as entering air dew point
        WetDryInterfcWaterTemp = AirDewPointTemp;

        // Estimate fraction wet surface area based on liquid temperatures
        if (std::abs(OutletWaterTemp - InletWaterTemp) > smalltempdiff) {
            SurfAreaWetFraction = (WetDryInterfcWaterTemp - InletWaterTemp) / (OutletWaterTemp - InletWaterTemp);
        } else {
            SurfAreaWetFraction = 0.0;
        }

    } else {
        SurfAreaWetFraction = waterCoil.SurfAreaWetFractionSaved;
    }
    // BEGIN LOOP to converge on SurfAreaWetFraction
    // The method employed in this loop is as follows: The coil is partially wet and partially dry,
    // we calculate the temperature of the coil at the interface, (the point at which the moisture begins
    // to condense) temperature of the  water  at interface and air temp is dew point at that location.
    // This is done by Iterating between the Completely Dry and Completely Wet Coil until the outlet
    // water temperature of one coil equals the inlet water temperature of another.
    // Using this value of interface temperature we now iterate to calculate Surface Fraction Wet, Iterate
    // function perturbs the value of Surface Fraction Wet and based on this new value the entire loop is
    // repeated to get a new interface water temperature and then surface fraction wet is again calculated.
    // This process continues till the error between the Wet Dry Interface Temp and Air Dew Point becomes
    // very negligible and in 95% of the cases its is a complete convergence to give the exact surface Wet
    // fraction.
    NewSurfAreaWetFrac = SurfAreaWetFraction;
    error = 0.0;
    SurfAreaFracPrevious = SurfAreaWetFraction;
    ErrorPrevious = 0.0;
    SurfAreaFracLast = SurfAreaWetFraction;
    ErrorLast = 0.0;

    for (iter = 1; iter <= itmax; ++iter) {

        // Calculating Surface Area Wet and Surface Area Dry
        SurfAreaWet = SurfAreaWetFraction * waterCoil.TotCoilOutsideSurfArea;
        SurfAreaDry = waterCoil.TotCoilOutsideSurfArea - SurfAreaWet;

        // Calculating UA values for the Dry Part of the Coil
        DryCoilUA = SurfAreaDry / (1.0 / waterCoil.UACoilInternalPerUnitArea + 1.0 / waterCoil.UADryExtPerUnitArea);

        // Calculating UA Value for the Wet part of the Coil
        WetPartUAExternal = waterCoil.UAWetExtPerUnitArea * SurfAreaWet;
        WetPartUAInternal = waterCoil.UACoilInternalPerUnitArea * SurfAreaWet;

        // Calculating Water Temperature at Wet Dry Interface of the coil
        WetDryInterfcWaterTemp = InletWaterTemp + SurfAreaWetFraction * (OutletWaterTemp - InletWaterTemp);

        // BEGIN LOOP to converge on liquid temperature at wet/dry boundary
        for (itT = 1; itT <= itmax; ++itT) {

            // Calculate dry coil performance with estimated liquid temperature at the boundary.
            CoilCompletelyDry(state,
                              CoilNum,
                              WetDryInterfcWaterTemp,
                              InletAirTemp,
                              DryCoilUA,
                              OutletWaterTemp,
                              WetDryInterfcAirTemp,
                              WetDryInterfcHumRat,
                              DryCoilHeatTranfer,
                              fanOp,
                              PartLoadRatio);

            // Calculate wet coil performance with calculated air temperature at the boundary.
            CoilCompletelyWet(state,
                              CoilNum,
                              InletWaterTemp,
                              WetDryInterfcAirTemp,
                              WetDryInterfcHumRat,
                              WetPartUAInternal,
                              WetPartUAExternal,
                              EstimateWetDryInterfcWaterTemp,
                              OutletAirTemp,
                              OutletAirHumRat,
                              WetCoilTotalHeatTransfer,
                              WetCoilSensibleHeatTransfer,
                              EstimateSurfAreaWetFraction,
                              WetDryInterfcSurfTemp,
                              fanOp,
                              PartLoadRatio);

            // Iterating to calculate the actual wet dry interface water temperature.
            errorT = EstimateWetDryInterfcWaterTemp - WetDryInterfcWaterTemp;
            General::Iterate(ResultXT, 0.001, WetDryInterfcWaterTemp, errorT, X1T, Y1T, itT, icvgT);
            WetDryInterfcWaterTemp = ResultXT;

            // IF convergence is achieved then exit the itT to itmax Do loop.
            if (icvgT == 1) {
                break;
            }

        } // End Do for Liq Boundary temp Convergence

        // Wet Dry Interface temperature not converged after maximum specified iterations.
        // Print error message, set return error flag
        if ((itT > itmax) && (!state.dataGlobal->WarmupFlag)) {
            ShowWarningError(state, std::format("For Coil:Cooling:Water {}", waterCoil.Name));
            ShowContinueError(state, "CoilPartWetPartDry: Maximum iterations exceeded for Liq Temp, at Interface");
        }

        // If Following condition prevails then surface is dry, calculate dry coil performance and return
        if (SurfAreaWetFraction <= 0.0 && WetDryInterfcSurfTemp >= AirDewPointTemp) {

            // Calculating Value of Dry UA for the coil
            DryCoilUA = waterCoil.TotCoilOutsideSurfArea / (1.0 / waterCoil.UACoilInternalPerUnitArea + 1.0 / waterCoil.UADryExtPerUnitArea);

            // Calling the Completely Dry Coil for outputs
            CoilCompletelyDry(state,
                              CoilNum,
                              InletWaterTemp,
                              InletAirTemp,
                              DryCoilUA,
                              OutletWaterTemp,
                              OutletAirTemp,
                              OutletAirHumRat,
                              TotWaterCoilLoad,
                              fanOp,
                              PartLoadRatio);

            // Sensible load = Total load in a Completely Dry Coil
            SenWaterCoilLoad = TotWaterCoilLoad;

            // All coil is Dry so fraction wet is ofcourse =0
            SurfAreaWetFraction = 0.0;
            return;
        }

        // IF the coil is not Dry then iterate to calculate Fraction of surface area that is wet.
        error = WetDryInterfcSurfTemp - AirDewPointTemp;
        CoilAreaFracIter(
            NewSurfAreaWetFrac, SurfAreaWetFraction, error, SurfAreaFracPrevious, ErrorPrevious, SurfAreaFracLast, ErrorLast, iter, icvg);
        SurfAreaWetFraction = NewSurfAreaWetFrac;

        // If converged, leave iteration loop
        if (icvg == 1) {
            break;
        }

        // Surface temperature not converged.  Repeat calculations with new
        // estimate of fraction wet surface area.
        if (SurfAreaWetFraction > 1.0) {
            SurfAreaWetFraction = 1.0;
        }
        if (SurfAreaWetFraction <= 0.0) {
            SurfAreaWetFraction = 0.0098;
        }

    } // End do for the overall iteration

    // Calculate sum of total and sensible heat transfer from dry and wet parts.
    TotWaterCoilLoad = DryCoilHeatTranfer + WetCoilTotalHeatTransfer;
    SenWaterCoilLoad = DryCoilHeatTranfer + WetCoilSensibleHeatTransfer;

    // Save last iterations values for this current time step
    waterCoil.SurfAreaWetFractionSaved = SurfAreaWetFraction;
}

// Calculating coil UA for Cooling Coil

Real64 CalcCoilUAbyEffectNTU(EnergyPlusData &state,
                             int const CoilNum,
                             Real64 const CapacityStream1,     // Capacity rate of stream1.(W/C)
                             Real64 const EnergyInStreamOne,   // Inlet state of stream1.(C)
                             Real64 const CapacityStream2,     // Capacity rate of stream2.(W/C)
                             Real64 const EnergyInStreamTwo,   // Inlet state of stream2.(C)
                             Real64 const DesTotalHeatTransfer // Heat transfer rate(W)
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   March 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the UA of a heat exchanger using the effectiveness-NTU relationships
    // given the entering capacity rate and temperature of each flow stream, the
    // heat transfer rate under these conditions and the heat exchanger configuration.

    // METHODOLOGY EMPLOYED:
    // Models coil using effectiveness NTU model

    // Enforce explicit typing of all variables in this routine

    // Return value
    Real64 CalcCoilUAbyEffectNTU; // Overall heat transfer coefficient(W/C)

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr SmallNo(1.e-9);
    int constexpr itmax(12);

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 MaxHeatTransfer;       // Maximum heat transfer from inlet conditions (W)
    Real64 EstimatedHeatTransfer; // Estimated heat transfer in iteration(W)
    Real64 CoilUA;                // Estimated heat transfer coefficient(W/C)
    Real64 error;                 // Deviation of dependent variable in iteration
    Real64 X1;                    // Previous values of independent variable in iteration
    Real64 Y1;
    Real64 ResultX;
    Real64 EnergyOutStreamOne;        // Intermediate Variable used
    Real64 EnergyOutStreamTwo;        // Intermediate variable used
    Real64 DesTotalHeatTransferCheck; // Check value to keep design total heat transfer in range
    int iter;                         // Iteration index
    int icvg;                         // Iteration convergence flag

    // Check for Q out of range (effectiveness > 1)
    MaxHeatTransfer = std::abs(min(CapacityStream1, CapacityStream2) * (EnergyInStreamOne - EnergyInStreamTwo));

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    // Error Message
    if ((std::abs(DesTotalHeatTransfer) - MaxHeatTransfer) / max(MaxHeatTransfer, SmallNo) > SmallNo) {
        ShowWarningError(state, std::format("For Coil:Cooling:Water {}", waterCoil.Name));
        ShowContinueError(state, "CalcCoilUAbyEffectNTU:Given Q impossible for given inlet states, proceeding with MaxHeat Transfer");
        ShowContinueError(state, "Check the Sizing:System and Sizing:Zone cooling design supply air temperature and ");
        ShowContinueError(state,
                          "the Sizing:Plant design Loop exit temperature.  There must be sufficient difference between these two temperatures.");
    }

    // Design Heat Transfer cannot exceed Max heat Transfer , setting it value such that effectiveness<1.0
    if ((DesTotalHeatTransfer) > (MaxHeatTransfer)) {
        // Pegging value so that effectiveness is less than 1.
        DesTotalHeatTransferCheck = 0.9 * MaxHeatTransfer;

        // Estimate CalcCoilUAbyEffectNTU
        CoilUA = std::abs(DesTotalHeatTransferCheck / (EnergyInStreamOne - EnergyInStreamTwo));

    } else {

        // Estimate CalcCoilUAbyEffectNTU
        CoilUA = std::abs(DesTotalHeatTransfer / (EnergyInStreamOne - EnergyInStreamTwo));
    }

    // BEGIN LOOP to iteratively calculate CalcCoilUAbyEffectNTU
    for (iter = 1; iter <= itmax; ++iter) {

        // Calculate heat transfer rate for estimated CalcCoilUAbyEffectNTU
        CoilOutletStreamCondition(
            state, CoilNum, CapacityStream1, EnergyInStreamOne, CapacityStream2, EnergyInStreamTwo, CoilUA, EnergyOutStreamOne, EnergyOutStreamTwo);

        // Initial Guess for a value of heat transfer
        EstimatedHeatTransfer = CapacityStream1 * (EnergyInStreamOne - EnergyOutStreamOne);

        // Calculate new estimate for CalcCoilUAbyEffectNTU by iteration
        if (DesTotalHeatTransfer > MaxHeatTransfer) {
            error = std::abs(EstimatedHeatTransfer) - std::abs(DesTotalHeatTransferCheck);
        } else {
            error = std::abs(EstimatedHeatTransfer) - std::abs(DesTotalHeatTransfer);
        }
        General::Iterate(ResultX, 0.01, CoilUA, error, X1, Y1, iter, icvg);
        CoilUA = ResultX;
        // If converged, leave loop
        if (icvg == 1) {
            break;
        }
    }

    // If not converged after itmax iterations, return error code
    if ((iter > itmax) && (!state.dataGlobal->WarmupFlag)) {
        ShowWarningError(state, std::format("For Coil:Cooling:Water {}", waterCoil.Name));
        ShowContinueError(state, "CalcCoilUAbyEffectNTU: Maximum iterations exceeded:Coil UA calculation");
        CalcCoilUAbyEffectNTU = 0.0; // Autodesk:Return Line added to set return value: Using non-converged CoilUA value may be preferred but
                                     // that was not happening
    } else {

        // Assign value to CalcCoilUAbyEffectNTU
        CalcCoilUAbyEffectNTU = CoilUA;
    }

    return CalcCoilUAbyEffectNTU;
}

// Calculating coil outlet stream conditions and coil UA for Cooling Coil

void CoilOutletStreamCondition(EnergyPlusData &state,
                               int const CoilNum,
                               Real64 const CapacityStream1,   // Capacity rate of stream1(W/C)
                               Real64 const EnergyInStreamOne, // Inlet state of stream1 (C)
                               Real64 const CapacityStream2,   // Capacity rate of stream2 (W/C)
                               Real64 const EnergyInStreamTwo, // Inlet state of stream2 (C)
                               Real64 const CoilUA,            // Heat transfer rateW)
                               Real64 &EnergyOutStreamOne,     // Outlet state of stream1 (C)
                               Real64 &EnergyOutStreamTwo      // Outlet state of stream2 (C)
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   March 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the outlet states of a simple heat exchanger using the effectiveness-Ntu
    // method of analysis.

    // REFERENCES:
    // Kays, W.M. and A.L. London.  1964.Compact Heat Exchangers, 2nd Ed.McGraw-Hill:New York.

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr LargeNo(1.e10);  // value used in place of infinity
    Real64 constexpr SmallNo(1.e-15); // value used in place of zero

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 MinimumCapacityStream; // Minimum capacity rate of the streams(W/C)
    Real64 MaximumCapacityStream; // Maximum capacity rate of the streams(W/C)
    Real64 RatioStreamCapacity;   // Ratio of minimum to maximum capacity rate
    Real64 NTU;                   // Number of transfer units
    Real64 effectiveness(0.0);    // Heat exchanger effectiveness
    Real64 MaxHeatTransfer;       // Maximum heat transfer possible(W)
    Real64 e;                     // Intermediate variables in effectiveness equation
    Real64 eta;
    Real64 b;
    Real64 d;

    // NTU and MinimumCapacityStream/MaximumCapacityStream (RatioStreamCapacity) calculations
    MinimumCapacityStream = min(CapacityStream1, CapacityStream2);
    MaximumCapacityStream = max(CapacityStream1, CapacityStream2);

    if (std::abs(MaximumCapacityStream) <= 1.e-6) { // .EQ. 0.0d0) THEN
        RatioStreamCapacity = 1.0;
    } else {
        RatioStreamCapacity = MinimumCapacityStream / MaximumCapacityStream;
    }

    if (std::abs(MinimumCapacityStream) <= 1.e-6) { // .EQ. 0.0d0) THEN
        NTU = LargeNo;
    } else {
        NTU = CoilUA / MinimumCapacityStream;
    }

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    // Calculate effectiveness for special limiting cases
    if (NTU <= 0.0) {
        effectiveness = 0.0;

    } else if (RatioStreamCapacity < SmallNo) {
        // MinimumCapacityStream/MaximumCapacityStream = 0 and effectiveness is independent of configuration
        // 20 is the Limit Chosen for Exponential Function, beyond which there is float point error.
        if (NTU > 20.0) {
            effectiveness = 1.0;
        } else {
            effectiveness = 1.0 - std::exp(-NTU);
        }
        // Calculate effectiveness depending on heat exchanger configuration
    } else if (waterCoil.HeatExchType == state.dataWaterCoils->CounterFlow) {

        // Counterflow Heat Exchanger Configuration
        if (std::abs(RatioStreamCapacity - 1.0) < SmallNo) {
            effectiveness = NTU / (NTU + 1.0);
        } else {
            if (NTU * (1.0 - RatioStreamCapacity) > 20.0) {
                e = 0.0;
            } else {
                e = std::exp(-NTU * (1.0 - RatioStreamCapacity));
            }
            effectiveness = (1.0 - e) / (1.0 - RatioStreamCapacity * e);
        }

    } else if (waterCoil.HeatExchType == state.dataWaterCoils->CrossFlow) {
        // Cross flow, both streams unmixed
        eta = std::pow(NTU, -0.22);
        if ((NTU * RatioStreamCapacity * eta) > 20.0) {
            b = 1.0 / (RatioStreamCapacity * eta);
            if (b > 20.0) {
                effectiveness = 1.0;
            } else {
                effectiveness = 1.0 - std::exp(-b);
                if (effectiveness < 0.0) {
                    effectiveness = 0.0;
                }
            }
        } else {
            d = ((std::exp(-NTU * RatioStreamCapacity * eta) - 1.0) / (RatioStreamCapacity * eta));
            if (d < -20.0 || d > 0.0) {
                effectiveness = 1.0;
            } else {
                effectiveness = 1.0 - std::exp((std::exp(-NTU * RatioStreamCapacity * eta) - 1.0) / (RatioStreamCapacity * eta));
                if (effectiveness < 0.0) {
                    effectiveness = 0.0;
                }
            }
        }
    }

    // Determine leaving conditions for the two streams
    MaxHeatTransfer = max(MinimumCapacityStream, SmallNo) * (EnergyInStreamOne - EnergyInStreamTwo);
    EnergyOutStreamOne = EnergyInStreamOne - effectiveness * MaxHeatTransfer / max(CapacityStream1, SmallNo);
    EnergyOutStreamTwo = EnergyInStreamTwo + effectiveness * MaxHeatTransfer / max(CapacityStream2, SmallNo);
}

// Subroutine for calculating outlet condition if coil is wet , for Cooling Coil

void WetCoilOutletCondition(EnergyPlusData &state,
                            int const CoilNum,
                            Real64 const AirTempIn,      // Entering air dry bulb temperature(C)
                            Real64 const EnthAirInlet,   // Entering air enthalpy(J/kg)
                            Real64 const EnthAirOutlet,  // Leaving air enthalpy(J/kg)
                            Real64 const UACoilExternal, // Heat transfer coefficient for external surface (W/C)
                            Real64 &OutletAirTemp,       // Leaving air dry bulb temperature(C)
                            Real64 &OutletAirHumRat,     // Leaving air humidity ratio
                            Real64 &SenWaterCoilLoad     // Sensible heat transfer rate(W)
)
{

    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   Mar 2004

    // PURPOSE OF THIS FUNCTION:
    // Calculate the leaving air temperature,the leaving air humidity ratio and the
    // sensible cooling capacity of wet cooling coil.

    // METHODOLOGY EMPLOYED:
    // Assumes condensate at uniform temperature.

    // REFERENCES:
    // Elmahdy, A.H. and Mitalas, G.P.  1977."A Simple Model for Cooling and
    // Dehumidifying Coils for Use In Calculating Energy Requirements for Buildings,"
    // ASHRAE Transactions,Vol.83 Part 2, pp. 103-117.

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr SmallNo(1.e-9); // SmallNo value used in place of zero

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 CapacitanceAir;        // Air capacity rate(W/C)
    Real64 NTU;                   // Number of heat transfer units
    Real64 effectiveness;         // Heat exchanger effectiveness
    Real64 EnthAirCondensateTemp; // Saturated air enthalpy at temperature of condensate(J/kg)
    Real64 TempCondensation;      // Temperature of condensate(C)
    Real64 TempAirDewPoint;       // Temperature air dew point

    // Determine the temperature effectiveness, assuming the temperature
    // of the condensate is constant (MinimumCapacityStream/MaximumCapacityStream = 0) and the specific heat
    // of moist air is constant

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    CapacitanceAir = waterCoil.InletAirMassFlowRate * PsyCpAirFnW(waterCoil.InletAirHumRat);

    // Calculating NTU from UA and Capacitance.
    // del      NTU = UACoilExternal/MAX(CapacitanceAir,SmallNo)
    // del      effectiveness = 1 - EXP(-MAX(0.0d0,NTU))
    // Calculating NTU from UA and Capacitance.
    if (UACoilExternal > 0.0) {
        if (CapacitanceAir > 0.0) {
            NTU = UACoilExternal / CapacitanceAir;
        } else {
            NTU = 0.0;
        }
        effectiveness = 1.0 - std::exp(-NTU);
    } else {
        effectiveness = 0.0;
    }

    // Calculate coil surface enthalpy and temperature at the exit
    // of the wet part of the coil using the effectiveness relation
    effectiveness = max(effectiveness, SmallNo);
    EnthAirCondensateTemp = EnthAirInlet - (EnthAirInlet - EnthAirOutlet) / effectiveness;

    // Calculate condensate temperature as the saturation temperature
    // at given saturation enthalpy
    TempCondensation = PsyTsatFnHPb(state, EnthAirCondensateTemp, state.dataEnvrn->OutBaroPress);

    TempAirDewPoint = PsyTdpFnWPb(state, waterCoil.InletAirHumRat, state.dataEnvrn->OutBaroPress);

    if ((TempAirDewPoint - TempCondensation) > 0.1) {

        // Calculate Outlet Air Temperature using effectiveness
        OutletAirTemp = AirTempIn - (AirTempIn - TempCondensation) * effectiveness;
        // Calculate Outlet air humidity ratio from PsyWFnTdbH routine
        OutletAirHumRat = PsyWFnTdbH(state, OutletAirTemp, EnthAirOutlet);

    } else {
        OutletAirHumRat = waterCoil.InletAirHumRat;
        OutletAirTemp = PsyTdbFnHW(EnthAirOutlet, OutletAirHumRat);
    }

    // Calculate Sensible Coil Load
    SenWaterCoilLoad = CapacitanceAir * (AirTempIn - OutletAirTemp);
}

// Beginning of Update subroutines for the WaterCoil Module
// *****************************************************************************

void UpdateWaterCoil(EnergyPlusData &state, int const CoilNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   1998
    //       MODIFIED       April 2004: Rahul Chillar
    //                      Feb 2010 B. Griffith, plant upgrades
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the coil outlet nodes.

    // METHODOLOGY EMPLOYED:
    // Data is moved from the coil data structure to the coil outlet nodes.

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    int const AirInletNode = waterCoil.AirInletNodeNum;
    int const AirOutletNode = waterCoil.AirOutletNodeNum;
    int const WaterOutletNode = waterCoil.WaterOutletNodeNum;

    // Set the outlet air nodes of the WaterCoil
    state.dataLoopNodes->Node(AirOutletNode).MassFlowRate = waterCoil.OutletAirMassFlowRate;
    state.dataLoopNodes->Node(AirOutletNode).Temp = waterCoil.OutletAirTemp;
    state.dataLoopNodes->Node(AirOutletNode).HumRat = waterCoil.OutletAirHumRat;
    state.dataLoopNodes->Node(AirOutletNode).Enthalpy = waterCoil.OutletAirEnthalpy;

    state.dataLoopNodes->Node(WaterOutletNode).Temp = waterCoil.OutletWaterTemp;
    state.dataLoopNodes->Node(WaterOutletNode).Enthalpy = waterCoil.OutletWaterEnthalpy;

    // Set the outlet nodes for properties that just pass through & not used
    state.dataLoopNodes->Node(AirOutletNode).Quality = state.dataLoopNodes->Node(AirInletNode).Quality;
    state.dataLoopNodes->Node(AirOutletNode).Press = state.dataLoopNodes->Node(AirInletNode).Press;
    state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMin = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMin;
    state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMax = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMax;
    state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMinAvail;
    state.dataLoopNodes->Node(AirOutletNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(AirInletNode).MassFlowRateMaxAvail;
    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        state.dataLoopNodes->Node(AirOutletNode).CO2 = state.dataLoopNodes->Node(AirInletNode).CO2;
    }
    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        state.dataLoopNodes->Node(AirOutletNode).GenContam = state.dataLoopNodes->Node(AirInletNode).GenContam;
    }
}

//        End of Update subroutines for the WaterCoil Module
// *****************************************************************************

// Beginning of Reporting subroutines for the WaterCoil Module
// *****************************************************************************

void ReportWaterCoil(EnergyPlusData &state, int const CoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variable for the coils.

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    if (waterCoil.reportCoilFinalSizes) {
        if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingHVACSizingSimulations && !state.dataGlobal->DoingSizing) {
            if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                ReportCoilSelection::setCoilFinalSizes(state,
                                                       waterCoil.coilReportNum,
                                                       waterCoil.DesWaterHeatingCoilRate,
                                                       waterCoil.DesWaterHeatingCoilRate,
                                                       waterCoil.DesAirVolFlowRate,
                                                       waterCoil.MaxWaterVolFlowRate);
                waterCoil.reportCoilFinalSizes = false;
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                ReportCoilSelection::setCoilFinalSizes(state,
                                                       waterCoil.coilReportNum,
                                                       waterCoil.DesWaterCoolingCoilRate,
                                                       -999.0,
                                                       waterCoil.DesAirVolFlowRate,
                                                       waterCoil.MaxWaterVolFlowRate);
                waterCoil.reportCoilFinalSizes = false;
            } else if (waterCoil.WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                ReportCoilSelection::setCoilFinalSizes(state,
                                                       waterCoil.coilReportNum,
                                                       waterCoil.DesWaterCoolingCoilRate,
                                                       -999.0,
                                                       waterCoil.DesAirVolFlowRate,
                                                       waterCoil.MaxWaterVolFlowRate);
                waterCoil.reportCoilFinalSizes = false;
            }
        }
    }
    Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;
    // report the WaterCoil energy from this component
    waterCoil.TotWaterHeatingCoilEnergy = waterCoil.TotWaterHeatingCoilRate * ReportingConstant;
    waterCoil.TotWaterCoolingCoilEnergy = waterCoil.TotWaterCoolingCoilRate * ReportingConstant;
    waterCoil.SenWaterCoolingCoilEnergy = waterCoil.SenWaterCoolingCoilRate * ReportingConstant;

    // report the WaterCoil water collection to water storage tank (if needed)

    if (waterCoil.CondensateCollectMode == state.dataWaterCoils->CondensateToTank) {
        // calculate and report condensation rates  (how much water extracted from the air stream)
        // water volumetric flow of water in m3/s for water system interactions
        //  put here to catch all types of DX coils
        Real64 Tavg = (waterCoil.InletAirTemp + waterCoil.OutletAirTemp) / 2.0;
        //   CR9155 Remove specific humidity calculations
        //  mdot * del HumRat / rho water
        waterCoil.CondensateVdot =
            max(0.0, (waterCoil.InletAirMassFlowRate * (waterCoil.InletAirHumRat - waterCoil.OutletAirHumRat) / Psychrometrics::RhoH2O(Tavg)));
        waterCoil.CondensateVol = waterCoil.CondensateVdot * ReportingConstant;

        state.dataWaterData->WaterStorage(waterCoil.CondensateTankID).VdotAvailSupply(waterCoil.CondensateTankSupplyARRID) = waterCoil.CondensateVdot;
        state.dataWaterData->WaterStorage(waterCoil.CondensateTankID).TwaterSupply(waterCoil.CondensateTankSupplyARRID) = waterCoil.OutletAirTemp;
    }
}

//        End of Reporting subroutines for the WaterCoil Module
// *****************************************************************************

// Beginning of Coil Utility subroutines for the Detailed Model
// *****************************************************************************

void CalcDryFinEffCoef(EnergyPlusData &state, Real64 const OutTubeEffFinDiamRatio, Array1D<Real64> &PolynomCoef)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR   Unknown
    //       DATE WRITTEN   Unknown
    //       DATE REWRITTEN  April 1997 by Russell D. Taylor, Ph.D.
    //       MODIFIED
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // The following subroutines are used once per cooling coil
    // simulation to obtain the coefficients of the dry fin
    // efficiency equation.  CalcDryFinEffCoef is the main calling
    // routine which manages calls to the Bessel function and polynomial
    // fit routines.

    // REFERENCES:
    // First found in MODSIM.
    // USE STATEMENTS:
    // na

    // Argument array dimensioning

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 FAI;
    Real64 FED;
    Real64 FEDnumerator;
    int I;
    int IE1;
    int IE2;
    int IE3;
    int IE4;
    int IE5;
    int IE6;
    Real64 R1;
    Real64 R1I1;
    Real64 R1K1;
    Real64 R2;
    Real64 R2I0;
    Real64 R2I1;
    Real64 R2K0;
    Real64 R2K1;
    Real64 RO;

    FAI = 0.02;
    for (I = 1; I <= WaterCoils::MaxOrderedPairs; ++I) {
        FAI += 0.035;
        R1 = FAI / (1.0 - OutTubeEffFinDiamRatio);
        R2 = R1 * OutTubeEffFinDiamRatio;
        RO = 2.0 * OutTubeEffFinDiamRatio / (FAI * (1.0 + OutTubeEffFinDiamRatio));
        CalcIBesselFunc(R1, 1, R1I1, IE1);
        CalcKBesselFunc(R2, 1, R2K1, IE2);
        CalcIBesselFunc(R2, 1, R2I1, IE3);
        CalcKBesselFunc(R1, 1, R1K1, IE4);
        CalcIBesselFunc(R2, 0, R2I0, IE5);
        CalcKBesselFunc(R2, 0, R2K0, IE6);
        FEDnumerator = RO * (R1I1 * R2K1 - R2I1 * R1K1);
        if (FEDnumerator != 0.0) {
            FED = FEDnumerator / (R2I0 * R1K1 + R1I1 * R2K0);
        } else {
            FED = 0.0;
        }
        //      FED = RO * (R1I1 * R2K1 - R2I1 * R1K1) / (R2I0 * R1K1 + R1I1 * R2K0)
        state.dataWaterCoils->OrderedPair(I, 1) = FAI;
        state.dataWaterCoils->OrderedPair(I, 2) = FED;
    }
    CalcPolynomCoef(state, state.dataWaterCoils->OrderedPair, PolynomCoef);
}

void CalcIBesselFunc(Real64 const BessFuncArg, int const BessFuncOrd, Real64 &IBessFunc, int &ErrorCode)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR   Unknown
    //       DATE WRITTEN   Unknown
    //       DATE REWRITTEN  April 1997 by Russell D. Taylor, Ph.D.
    //       MODIFIED
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // To calculate the modified Bessel Function from order 0 to BessFuncOrd
    // BessFuncArg    ARGUMENT OF BESSEL FUNCTION
    // BessFuncOrd    ORDER OF BESSEL FUNCTION, GREATER THAN OR EQUAL TO ZERO
    // IBessFunc   RESULTANT VALUE OF I BESSEL FUNCTION
    // ErrorCode  RESULTANT ERROR CODE:
    //       ErrorCode = 0   NO ERROR
    //       ErrorCode = 1   BessFuncOrd .LT. 0
    //       ErrorCode = 2   BessFuncArg .LT. 0
    //       ErrorCode = 3   IBessFunc .LT. 10**(-30),     IBessFunc IS SET TO 0
    //       ErrorCode = 4   BessFuncArg .GT. BessFuncOrd & BessFuncArg .GT. 90,  IBessFunc IS SET TO 10**38

    // REFERENCES:
    // First found in MODSIM.

    // USE STATEMENTS:
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr ErrorTol(1.0e-06);

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LoopCount;

    Real64 FI;
    Real64 FK;
    Real64 TERM;

    ErrorCode = 0;
    IBessFunc = 1.0;
    if (BessFuncArg == 0.0 && BessFuncOrd == 0) {
        return;
    }

    if (BessFuncOrd < 0) {
        ErrorCode = 1;
        return;
    }
    if (BessFuncArg < 0.0) {
        ErrorCode = 2;
        return;
    }
    if (BessFuncArg > 12.0 && BessFuncArg > BessFuncOrd) {
        if (BessFuncArg > 90.0) {
            ErrorCode = 4;
            IBessFunc = 1.0e30;
            return;
        }
        TERM = 1.0;
        IBessFunc = 1.0;
        for (LoopCount = 1; LoopCount <= 30; ++LoopCount) { // Start of 1st LoopCount Loop
            if (std::abs(TERM) <= std::abs(ErrorTol * IBessFunc)) {
                IBessFunc *= std::exp(BessFuncArg) / std::sqrt(2.0 * Constant::Pi * BessFuncArg);
                return;
            }
            TERM *= 0.125 / BessFuncArg * (pow_2(2 * LoopCount - 1) - 4 * BessFuncOrd * BessFuncOrd) / double(LoopCount);
            IBessFunc += TERM;
        } // End of 1st LoopCount loop
    }

    TERM = 1.0;
    if (BessFuncOrd > 0) {
        for (LoopCount = 1; LoopCount <= BessFuncOrd; ++LoopCount) { // Start of 2nd LoopCount Loop
            FI = LoopCount;
            if (std::abs(TERM) < 1.0e-30 * FI / (BessFuncArg * 2.0)) {
                ErrorCode = 3;
                IBessFunc = 0.0;
                return;
            }
            TERM *= BessFuncArg / (2.0 * FI);
        } // End of 2nd LoopCount loop
    }

    IBessFunc = TERM;
    for (LoopCount = 1; LoopCount <= 1000; ++LoopCount) { // Start of 3rd LoopCount Loop
        if (std::abs(TERM) <= std::abs(IBessFunc * ErrorTol)) {
            return;
        }
        FK = LoopCount * (BessFuncOrd + LoopCount);
        TERM *= pow_2(BessFuncArg) / (4.0 * FK);
        IBessFunc += TERM;
    } // End of  3rd LoopCount loop
}

void CalcKBesselFunc(Real64 const BessFuncArg, int const BessFuncOrd, Real64 &KBessFunc, int &ErrorCode)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR   Unknown
    //       DATE WRITTEN   Unknown
    //       DATE REWRITTEN  April 1997 by Russell D. Taylor, Ph.D.
    //       MODIFIED
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // To calculate the K Bessel Function for a given argument and
    // order
    //  BessFuncArg    THE ARGUMENT OF THE K BESSEL FUNCTION DESIRED
    //  BessFuncOrd    THE ORDER OF THE K BESSEL FUNCTION DESIRED
    //  KBessFunc   THE RESULTANT K BESSEL FUNCTION
    //  ErrorCode  RESULTANT ERROR CODE:
    //        ErrorCode=0  NO ERROR
    //        ErrorCode=1  BessFuncOrd IS NEGATIVE
    //        ErrorCode=2  BessFuncArg IS ZERO OR NEGATIVE
    //        ErrorCode=3  BessFuncArg .GT. 85, KBessFunc .LT. 10**-38; KBessFunc SET TO 0.
    //        ErrorCode=4  KBessFunc .GT. 10**38; KBessFunc SET TO 10**38
    // NOTE: BessFuncOrd MUST BE GREATER THAN OR EQUAL TO ZERO
    // METHOD:
    //  COMPUTES ZERO ORDER AND FIRST ORDER BESSEL FUNCTIONS USING
    //  SERIES APPROXIMATIONS AND THEN COMPUTES BessFuncOrd TH ORDER FUNCTION
    //  USING RECURRENCE RELATION.
    //  RECURRENCE RELATION AND POLYNOMIAL APPROXIMATION TECHNIQUE
    //  AS DESCRIBED BY A.J.M. HITCHCOCK, 'POLYNOMIAL APPROXIMATIONS
    //  TO BESSEL FUNCTIONS OF ORDER ZERO AND ONE AND TO RELATED
    //  FUNCTIONS,' M.T.A.C., V.11, 1957, PP. 86-88, AND G.BessFuncOrd. WATSON,
    //  'A TREATISE ON THE THEORY OF BESSEL FUNCTIONS,' CAMBRIDGE
    //  UNIVERSITY PRESS, 1958, P.62

    // USE STATEMENTS:
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr GJMAX(1.0e+38);

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LoopCount;
    bool StopLoop;

    Real64 FACT;
    Real64 G0;
    Real64 G1;
    Real64 GJ;
    Real64 HJ;
    Array1D<Real64> T(12);
    Real64 X2J;

    KBessFunc = 0.0;
    G0 = 0.0;
    GJ = 0.0;

    if (BessFuncOrd < 0.0) {
        ErrorCode = 1;
        return;
    }
    if (BessFuncArg <= 0.0) {
        ErrorCode = 2;
        return;
    }
    if (BessFuncArg > 85.0) {
        ErrorCode = 3;
        KBessFunc = 0.0;
        return;
    }

    ErrorCode = 0;

    //     Use polynomial approximation if BessFuncArg > 1.

    if (BessFuncArg > 1.0) {
        T(1) = 1.0 / BessFuncArg;
        for (LoopCount = 2; LoopCount <= 12; ++LoopCount) {
            T(LoopCount) = T(LoopCount - 1) / BessFuncArg;
        } // End of LoopCount Loop
        if (BessFuncOrd != 1) {

            //     Compute K0 using polynomial approximation

            G0 = std::exp(-BessFuncArg) *
                 (1.2533141 - 0.1566642 * T(1) + 0.08811128 * T(2) - 0.09139095 * T(3) + 0.1344596 * T(4) - 0.2299850 * T(5) + 0.3792410 * T(6) -
                  0.5247277 * T(7) + 0.5575368 * T(8) - 0.4262633 * T(9) + 0.2184518 * T(10) - 0.06680977 * T(11) + 0.009189383 * T(12)) *
                 std::sqrt(1.0 / BessFuncArg);
            if (BessFuncOrd == 0) {
                KBessFunc = G0;
                return;
            }
        }

        //     Compute K1 using polynomial approximation

        G1 = std::exp(-BessFuncArg) *
             (1.2533141 + 0.4699927 * T(1) - 0.1468583 * T(2) + 0.1280427 * T(3) - 0.1736432 * T(4) + 0.2847618 * T(5) - 0.4594342 * T(6) +
              0.6283381 * T(7) - 0.6632295 * T(8) + 0.5050239 * T(9) - 0.2581304 * T(10) + 0.07880001 * T(11) - 0.01082418 * T(12)) *
             std::sqrt(1.0 / BessFuncArg);
        if (BessFuncOrd == 1) {
            KBessFunc = G1;
            return;
        }
    } else {

        //     Use series expansion if BessFuncArg <= 1.

        if (BessFuncOrd != 1) {

            //     Compute K0 using series expansion

            G0 = -(0.5772157 + std::log(BessFuncArg / 2.0));
            X2J = 1.0;
            FACT = 1.0;
            HJ = 0.0;
            for (LoopCount = 1; LoopCount <= 6; ++LoopCount) {
                X2J *= pow_2(BessFuncArg) / 4.0;
                FACT *= pow_2(1.0 / double(LoopCount));
                HJ += 1.0 / double(LoopCount);
                G0 += X2J * FACT * (HJ - (0.5772157 + std::log(BessFuncArg / 2.0)));
            } // End of LoopCount Loop
            if (BessFuncOrd == 0.0) {
                KBessFunc = G0;
                return;
            }
        }

        //     Compute K1 using series expansion

        X2J = BessFuncArg / 2.0;
        FACT = 1.0;
        HJ = 1.0;
        G1 = 1.0 / BessFuncArg + X2J * (0.5 + (0.5772157 + std::log(BessFuncArg / 2.0)) - HJ);
        for (LoopCount = 2; LoopCount <= 8; ++LoopCount) {
            X2J *= pow_2(BessFuncArg) / 4.0;
            FACT *= pow_2(1.0 / double(LoopCount));
            HJ += 1.0 / double(LoopCount);
            G1 += X2J * FACT * (0.5 + ((0.5772157 + std::log(BessFuncArg / 2.0)) - HJ) * double(LoopCount));
        } // End of LoopCount Loop
        if (BessFuncOrd == 1) {
            KBessFunc = G1;
            return;
        }
    }

    //     From K0 and K1 compute KN using recurrence relation

    LoopCount = 2;
    StopLoop = false;
    while (LoopCount <= BessFuncOrd && !StopLoop) {
        GJ = 2.0 * (double(LoopCount) - 1.0) * G1 / BessFuncArg + G0;
        if (GJ - GJMAX > 0.0) {
            ErrorCode = 4;
            GJ = GJMAX;
            StopLoop = true;
        } else {
            G0 = G1;
            G1 = GJ;
            ++LoopCount;
        }
    } // End of LoopCount Loop
    KBessFunc = GJ;
}

void CalcPolynomCoef(EnergyPlusData &state, Array2<Real64> const &OrderedPair, Array1D<Real64> &PolynomCoef)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR   Unknown
    //       DATE WRITTEN   Unknown
    //       DATE REWRITTEN  April 1997 by Russell D. Taylor, Ph.D.
    //       MODIFIED
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // Fits polynomial of order from 1 to MaxPolynomOrder to the
    // ordered pairs of data points X,Y

    // USE STATEMENTS:
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool Converged;
    Real64 B;
    int I;
    int II;
    int J;
    int PolynomOrder;
    int CurrentOrder;
    int CurrentOrdPair;
    Real64 S1;
    Real64 S2;

    auto &OrdPairSum = state.dataWaterCoils->OrdPairSum;

    OrdPairSum = 0.0;
    OrdPairSum(1, 1) = WaterCoils::MaxOrderedPairs;
    PolynomCoef = 0.0;
    for (CurrentOrdPair = 1; CurrentOrdPair <= WaterCoils::MaxOrderedPairs; ++CurrentOrdPair) {
        OrdPairSum(2, 1) += OrderedPair(CurrentOrdPair, 1);
        OrdPairSum(3, 1) += OrderedPair(CurrentOrdPair, 1) * OrderedPair(CurrentOrdPair, 1);
        OrdPairSum(1, 2) += OrderedPair(CurrentOrdPair, 2);
        OrdPairSum(2, 2) += OrderedPair(CurrentOrdPair, 1) * OrderedPair(CurrentOrdPair, 2);
    }
    PolynomOrder = 1;
    Converged = false;
    while (!Converged) {
        for (CurrentOrder = 1; CurrentOrder <= PolynomOrder + 1; ++CurrentOrder) {
            for (J = 1; J <= PolynomOrder + 1; ++J) {
                state.dataWaterCoils->OrdPairSumMatrix(J, CurrentOrder) = OrdPairSum(J - 1 + CurrentOrder, 1);
            } // End of J loop
            state.dataWaterCoils->OrdPairSumMatrix(PolynomOrder + 2, CurrentOrder) = OrdPairSum(CurrentOrder, 2);
        } // End of CurrentOrder loop

        for (CurrentOrder = 1; CurrentOrder <= PolynomOrder + 1; ++CurrentOrder) {
            state.dataWaterCoils->OrdPairSumMatrix(CurrentOrder, PolynomOrder + 2) = -1.0;
            for (J = CurrentOrder + 1; J <= PolynomOrder + 2; ++J) {
                state.dataWaterCoils->OrdPairSumMatrix(J, PolynomOrder + 2) = 0.0;
            } // End of J loop

            for (II = 2; II <= PolynomOrder + 2; ++II) {
                for (J = CurrentOrder + 1; J <= PolynomOrder + 2; ++J) {
                    state.dataWaterCoils->OrdPairSumMatrix(J, II) -= state.dataWaterCoils->OrdPairSumMatrix(J, 1) *
                                                                     state.dataWaterCoils->OrdPairSumMatrix(CurrentOrder, II) /
                                                                     state.dataWaterCoils->OrdPairSumMatrix(CurrentOrder, 1);
                } // End of J loop
            } // End of II loop
            for (II = 1; II <= PolynomOrder + 1; ++II) {
                for (J = CurrentOrder + 1; J <= PolynomOrder + 2; ++J) {
                    state.dataWaterCoils->OrdPairSumMatrix(J, II) = state.dataWaterCoils->OrdPairSumMatrix(J, II + 1);
                } // End of J loop
            } // End of II loop
        } // End of CurrentOrder loop

        S2 = 0.0;
        for (CurrentOrdPair = 1; CurrentOrdPair <= WaterCoils::MaxOrderedPairs; ++CurrentOrdPair) {
            S1 = state.dataWaterCoils->OrdPairSumMatrix(PolynomOrder + 2, 1);
            auto const OrderedPair1C = OrderedPair(CurrentOrdPair, 1); // (AUTO_OK_OBJ)
            Real64 OrderedPair1C_pow = 1.0;
            for (CurrentOrder = 1; CurrentOrder <= PolynomOrder; ++CurrentOrder) {
                OrderedPair1C_pow *= OrderedPair1C;
                S1 += state.dataWaterCoils->OrdPairSumMatrix(PolynomOrder + 2, CurrentOrder + 1) * OrderedPair1C_pow;
            } // End of CurrentOrder loop
            S2 += (S1 - OrderedPair(CurrentOrdPair, 2)) * (S1 - OrderedPair(CurrentOrdPair, 2));
        } // End of CurrentOrdPair loop
        B = WaterCoils::MaxOrderedPairs - (PolynomOrder + 1);
        if (S2 > 0.0001) {
            S2 = std::sqrt(S2 / B);
        }
        for (CurrentOrder = 1; CurrentOrder <= PolynomOrder + 1; ++CurrentOrder) {
            PolynomCoef(CurrentOrder) = state.dataWaterCoils->OrdPairSumMatrix(PolynomOrder + 2, CurrentOrder);
        } // End of CurrentOrder loop

        if ((PolynomOrder - WaterCoils::MaxPolynomOrder < 0) && (S2 - WaterCoils::PolyConvgTol > 0.0)) {
            ++PolynomOrder;
            J = 2 * PolynomOrder;
            OrdPairSum(J, 1) = OrdPairSum(J + 1, 1) = 0.0;
            auto OrdPairSum2P = OrdPairSum(PolynomOrder + 1, 2) = 0.0; // (AUTO_OK_OBJ)
            for (I = 1; I <= WaterCoils::MaxOrderedPairs; ++I) {
                auto const OrderedPair1I = OrderedPair(I, 1); // (AUTO_OK_OBJ)
                Real64 OrderedPair_pow = std::pow(OrderedPair1I, J - 1);
                OrdPairSum(J, 1) += OrderedPair_pow;
                OrderedPair_pow *= OrderedPair1I;
                OrdPairSum(J + 1, 1) += OrderedPair_pow;
                OrdPairSum2P += OrderedPair(I, 2) * std::pow(OrderedPair1I, PolynomOrder);
            }
            OrdPairSum(PolynomOrder + 1, 2) = OrdPairSum2P;
        } else {
            Converged = true;
        }
    }
}

// Iterate Routine for Cooling Coil

void CoilAreaFracIter(Real64 &NewSurfAreaWetFrac,       // Out Value of variable
                      Real64 const SurfAreaFracCurrent, // Driver Value
                      Real64 const ErrorCurrent,        // Objective Function
                      Real64 &SurfAreaFracPrevious,     // First Previous value of Surf Area Fraction
                      Real64 &ErrorPrevious,            // First Previous value of error
                      Real64 &SurfAreaFracLast,         // Second Previous value of Surf Area Fraction
                      Real64 &ErrorLast,                // Second Previous value of error
                      int const IterNum,                // Number of Iterations
                      int &icvg                         // Iteration convergence flag
)
{
    // FUNCTION INFORMATION:
    // AUTHOR         Rahul Chillar
    // DATE WRITTEN   June 2004

    // PURPOSE OF THIS FUNCTION:
    // Iterately solves for the value of SurfAreaWetFraction for the Cooling Coil.

    // METHODOLOGY EMPLOYED:
    // First function generates 2 sets of guess points by perturbation and subsequently
    // by Linear Fit and using the generated points calculates coeffecients for Quadratic
    // fit to predict the next value of surface area wet fraction.

    // REFERENCES:
    // ME 423 Design of Thermal Systems Class Notes.UIUC. W.F.Stoecker

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr Tolerance(1.e-5);         // Relative error tolerance
    Real64 constexpr PerturbSurfAreaFrac(0.1); // Perturbation applied to Surf Fraction to initialize iteration
    Real64 constexpr SmallNum(1.e-9);          // Small Number

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 check;             // Validity Check for moving to Quad Solution
    Real64 QuadCoefThree;     // Term under radical in quadratic solution
    Real64 QuadCoefOne;       // Term under radical in quadratic solution
    Real64 QuadCoefTwo;       // Term under radical in quadratic solution
    Real64 Slope;             // Slope for linear fit
    Real64 SurfAreaFracOther; // Intermediate Value of Surf Area
    int mode;                 // Linear/ perturbation option

    // Convergence Check  by comparing previous and current value of surf area fraction
    if ((std::abs(SurfAreaFracCurrent - SurfAreaFracPrevious) < Tolerance * max(std::abs(SurfAreaFracCurrent), SmallNum) && IterNum != 1) ||
        ErrorCurrent == 0.0) {
        // Setting value for surface area fraction for coil
        NewSurfAreaWetFrac = SurfAreaFracCurrent;
        icvg = 1; // Convergence Flag
        return;
    }

    // If Icvg = 0 , it has not converged.By perturbation for getting second set of
    // data (mode=1), Getting Third set of data by performing a  linear fit(Mode=2).
    // Now using the above 3 points generated by perturbation and Linear Fit to perform
    // a quadratic fit.This will happen after second iteration only.
    icvg = 0; // Convergence flag = false
    // For First Iteration Start with perturbation, For second iteration start with linear fit
    // from the previous two values
    mode = IterNum;

Label10:;
    if (mode == 1) {

        // FirstGuess Set of Points provided by perturbation
        if (std::abs(SurfAreaFracCurrent) > SmallNum) {
            NewSurfAreaWetFrac = SurfAreaFracCurrent * (1.0 + PerturbSurfAreaFrac);
        } else {
            NewSurfAreaWetFrac = PerturbSurfAreaFrac;
        }

        // Second set of values being calculated from the first set of values (incoming & perturb)
    } else if (mode == 2) {

        // Calculating Slope for interpolating to the New Point (Simple Linear Extrapolation)
        Slope = (ErrorPrevious - ErrorCurrent) / (SurfAreaFracPrevious - SurfAreaFracCurrent);
        // Error Check for value or Slope
        if (Slope == 0.0) {
            mode = 1; // Go back to Perturbation
            goto Label10;
        }
        // Guessing New Value for Surface Area Fraction
        NewSurfAreaWetFrac = SurfAreaFracCurrent - ErrorCurrent / Slope;
    } else {

        // Check for Quadratic Fit possible here ,Previous value of surf area fraction
        // equals current value then Try linear fit for another point.
        if (SurfAreaFracCurrent == SurfAreaFracPrevious) {
            // Assign Value of previous point to Last Variable for storing
            // Go back and calculate new value for Previous.
            SurfAreaFracPrevious = SurfAreaFracLast;
            ErrorPrevious = ErrorLast;
            mode = 2;
            goto Label10;
        } else if (SurfAreaFracCurrent == SurfAreaFracLast) {
            // Calculate another value using Linear Fit.
            mode = 2;
            goto Label10;
        }

        // Now We have enough previous points to calculate coefficients and
        // perform a quadratic fit for new guess value of surface area fraction

        // Calculating First Coefficients for Quadratic Curve Fit
        QuadCoefThree = ((ErrorLast - ErrorCurrent) / (SurfAreaFracLast - SurfAreaFracCurrent) -
                         (ErrorPrevious - ErrorCurrent) / (SurfAreaFracPrevious - SurfAreaFracCurrent)) /
                        (SurfAreaFracLast - SurfAreaFracPrevious);
        // Calculating Second Coefficients for Quadratic Curve Fit
        QuadCoefTwo = (ErrorPrevious - ErrorCurrent) / (SurfAreaFracPrevious - SurfAreaFracCurrent) -
                      (SurfAreaFracPrevious + SurfAreaFracCurrent) * QuadCoefThree;

        // Calculating Third Coefficients for Quadratic Curve Fit
        QuadCoefOne = ErrorCurrent - (QuadCoefTwo + QuadCoefThree * SurfAreaFracCurrent) * SurfAreaFracCurrent;

        // Check for validity of coefficients , if not REAL(r64) ,Then fit is linear
        if (std::abs(QuadCoefThree) < 1.E-10) {
            mode = 2; // going to Linear mode, due to colinear points.
            goto Label10;
        }

        // If value of Quadratic coefficients not suitable enough due to round off errors
        // to predict new point go to linear fit and acertain new values for the coefficients.
        if (std::abs((QuadCoefOne + (QuadCoefTwo + QuadCoefThree * SurfAreaFracPrevious) * SurfAreaFracPrevious - ErrorPrevious) / ErrorPrevious) >
            1.E-4) {
            mode = 2; // go to linear mode
            goto Label10;
        }

        // Validity Check for Imaginary roots, In this case go back to linear fit.
        check = pow_2(QuadCoefTwo) - 4.0 * QuadCoefOne * QuadCoefThree;
        // Imaginary Root Exist
        if (check < 0) {
            mode = 2;
            goto Label10;
        } else if (check > 0) {
            // real unequal roots exist, Determine the roots nearest to most recent guess
            NewSurfAreaWetFrac = (-QuadCoefTwo + std::sqrt(check)) / QuadCoefThree / 2.0;
            SurfAreaFracOther = -NewSurfAreaWetFrac - QuadCoefTwo / QuadCoefThree;
            // Assigning value to Surface Area Fraction with recent
            if (std::abs(NewSurfAreaWetFrac - SurfAreaFracCurrent) > std::abs(SurfAreaFracOther - SurfAreaFracCurrent)) {
                NewSurfAreaWetFrac = SurfAreaFracOther;
            }
        } else {
            // The roots are real, one solution exists.
            NewSurfAreaWetFrac = -QuadCoefTwo / QuadCoefThree / 2;
        }
    }

    if (mode < 3) {
        // No valid previous points to eliminate, since it just has 2 points.
        // Loading previous values into last
        SurfAreaFracLast = SurfAreaFracPrevious;
        ErrorLast = ErrorPrevious;
        // Loading Current Values into previous
        SurfAreaFracPrevious = SurfAreaFracCurrent;
        ErrorPrevious = ErrorCurrent;
    } else {

        // Elimination the most distance previous point from the answer based on sign and
        // magnitute of the error. Keeping Current Point
        if (ErrorPrevious * ErrorCurrent > 0 && ErrorLast * ErrorCurrent > 0) {
            // If sign are same , simply eliminate the one with biggest error value.
            if (std::abs(ErrorLast) > std::abs(ErrorPrevious)) {
                // Eliminating Last Value
                SurfAreaFracLast = SurfAreaFracPrevious;
                ErrorLast = ErrorPrevious;
            }
        } else {
            // If signs are different eliminate previous error with same sign as current error
            if (ErrorLast * ErrorCurrent > 0) {
                // Previous Loaded to Last
                SurfAreaFracLast = SurfAreaFracPrevious;
                ErrorLast = ErrorPrevious;
            }
        }
        // Current Loaded into previous.
        SurfAreaFracPrevious = SurfAreaFracCurrent;
        ErrorPrevious = ErrorCurrent;
    }
}

void CheckWaterCoilSchedule(EnergyPlusData &state, std::string_view CompName, Real64 &Value, int &CompIndex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2005

    // Obtains and Allocates WaterCoil related parameters from input file
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) { // First time subroutine has been entered
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int CoilNum = 0;
    // Find the correct Coil number
    if (CompIndex == 0) {
        CoilNum = Util::FindItemInList(CompName, state.dataWaterCoils->WaterCoil);
        if (CoilNum == 0) {
            ShowFatalError(state, std::format("CheckWaterCoilSchedule: Coil not found={}", CompName));
        }
        CompIndex = CoilNum;
        Value = state.dataWaterCoils->WaterCoil(CoilNum).availSched->getCurrentVal(); // not scheduled?
    } else {
        CoilNum = CompIndex;
        if (CoilNum > state.dataWaterCoils->NumWaterCoils || CoilNum < 1) {
            ShowFatalError(state,
                           std::format("CheckWaterCoilSchedule: Invalid CompIndex passed={}, Number of Heating Coils={}, Coil name={}",
                                       CoilNum,
                                       state.dataWaterCoils->NumWaterCoils,
                                       CompName));
        }
        auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        if (CompName != waterCoil.Name) {
            ShowFatalError(state,
                           std::format("CheckWaterCoilSchedule: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                       CoilNum,
                                       CompName,
                                       waterCoil.Name));
        }
        Value = waterCoil.availSched->getCurrentVal(); // not scheduled?
    }
}

Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state,
                               std::string_view CoilType,   // must match coil types in this module
                               std::string const &CoilName, // must match coil names for the coil type
                               bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2006

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the max water flow rate for the given coil and returns it.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
    // as negative.

    // Return value
    Real64 MaxWaterFlowRate; // returned max water flow rate of matched coil

    // FUNCTION LOCAL VARIABLE DECLARATIONS:

    // Obtains and Allocates WaterCoil related parameters from input file
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) { // First time subroutine has been entered
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int WhichCoil = 0;
    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            // coil does not specify MaxWaterFlowRate
            MaxWaterFlowRate = state.dataWaterCoils->WaterCoil(WhichCoil).MaxWaterVolFlowRate;
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format("GetCoilMaxWaterFlowRate: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ShowContinueError(state, "... Max Water Flow rate returned as -1000.");
        ErrorsFound = true;
        MaxWaterFlowRate = -1000.0;
    }

    return MaxWaterFlowRate;
}

int GetCoilInletNode(EnergyPlusData &state,
                     std::string_view CoilType,   // must match coil types in this module
                     std::string const &CoilName, // must match coil names for the coil type
                     bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad
    //       DATE WRITTEN   March 2007

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the inlet node number.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
    // as zero.

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int NodeNumber = 0;
    int WhichCoil = 0;
    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterCoils->WaterCoil(WhichCoil).AirInletNodeNum;
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
    //       AUTHOR         R. Raustad
    //       DATE WRITTEN   March 2007

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the inlet node number.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
    // as zero.

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int WhichCoil = 0;
    int NodeNumber = 0; // returned node number of matched coil
    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterCoils->WaterCoil(WhichCoil).AirOutletNodeNum;
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state,
                        std::format("GetCoilOutletNode: Could not find Coil, Type=\"{}\" Name=\"{}\" when accessing coil outlet node number.",
                                    CoilType,
                                    CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

int GetCoilWaterInletNode(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad
    //       DATE WRITTEN   July 2007

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the inlet water control node number.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
    // as zero.

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int NodeNumber = 0; // returned node number of matched coil
    int WhichCoil = 0;
    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterCoils->WaterCoil(WhichCoil).WaterInletNodeNum;
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format("GetCoilWaterInletNode: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

int GetCoilWaterOutletNode(EnergyPlusData &state,
                           std::string_view CoilType,   // must match coil types in this module
                           std::string const &CoilName, // must match coil names for the coil type
                           bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad
    //       DATE WRITTEN   July 2007

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the outlet water node number.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and node number is returned
    // as zero.

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int NodeNumber = 0; // returned node number of matched coil
    int WhichCoil = 0;
    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterCoils->WaterCoil(WhichCoil).WaterOutletNodeNum;
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format("GetCoilWaterOutletNode: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

void SetCoilDesFlow(EnergyPlusData &state,
                    std::string_view CoilType,   // must match coil types in this module
                    std::string const &CoilName, // must match coil names for the coil type
                    Real64 const CoilDesFlow,    // coil volumetric air flow rate [m3/s]
                    bool &ErrorsFound            // set to true if problem
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // This routine is designed to set the design air volume flow rate in the
    // water coil data structure. Some of the coil types do not have this datum as
    // an input parameter and it is needed for calculating capacity for output reporting.

    if (state.dataWaterCoils->GetWaterCoilsInputFlag) { // First time subroutine has been entered
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry") ||
        Util::SameString(CoilType, "Coil:Cooling:Water")) {
        int WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            if (state.dataWaterCoils->WaterCoil(WhichCoil).DesAirVolFlowRate <= 0.0) {
                state.dataWaterCoils->WaterCoil(WhichCoil).DesAirVolFlowRate = CoilDesFlow;
            } else {
                // WaterCoil(WhichCoil).DesAirVolFlowRate = CoilDesFlow;
            }
        } else {
            ShowSevereError(state, std::format("GetCoilMaxWaterFlowRate: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
        }
    }
}

Real64 GetWaterCoilDesAirFlow(EnergyPlusData &state,
                              std::string const &CoilType, // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // This routine is designed to set the design air volume flow rate in the
    // water coil data structure. Some of the coil types do not have this datum as
    // an input parameter and it is needed for calculating capacity for output reporting.

    Real64 CoilDesAirFlow = 0.0;

    if (state.dataWaterCoils->GetWaterCoilsInputFlag) { // First time subroutine has been entered
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    if (Util::SameString(CoilType, "Coil:Cooling:Water")) {
        int WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            CoilDesAirFlow = state.dataWaterCoils->WaterCoil(WhichCoil).DesAirVolFlowRate;
        } else {
            ShowSevereError(state, std::format("GetWaterCoilDesAirFlowRate: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
        }
    } else {
        ShowSevereError(state, std::format("GetWaterCoilDesAirFlowRate: Function not valid for Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
    }

    return CoilDesAirFlow;
}

void CheckActuatorNode(EnergyPlusData &state,
                       int const ActuatorNodeNum,                    // input actuator node number
                       DataPlant::PlantEquipmentType &WaterCoilType, // Cooling or Heating or 0
                       bool &NodeNotFound                            // true if matching water inlet node not found
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   January 2009

    // PURPOSE OF THIS FUNCTION:
    // This subroutine checks that the input actuator node number is matched by
    // the water inlet node number of some water coil

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    WaterCoilType = DataPlant::PlantEquipmentType::Invalid;
    NodeNotFound = true;
    for (int CoilNum = 1; CoilNum <= state.dataWaterCoils->NumWaterCoils; ++CoilNum) {
        auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        if (waterCoil.WaterInletNodeNum == ActuatorNodeNum) {
            WaterCoilType = waterCoil.WaterCoilType;
            NodeNotFound = false;
            break;
        }
    }
}

void CheckForSensorAndSetPointNode(EnergyPlusData &state,
                                   int const SensorNodeNum,                          // controller sensor node number
                                   HVACControllers::CtrlVarType const ControlledVar, // controlled variable type
                                   bool &NodeNotFound                                // true if matching air outlet node not found
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   March 2013

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine checks that the sensor node number matches the air outlet node number
    // of some water coils

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CheckForSensorAndSetpointNode: ");

    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int WhichCoil = 0;
    NodeNotFound = true;

    for (int CoilNum = 1; CoilNum <= state.dataWaterCoils->NumWaterCoils; ++CoilNum) {
        auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        if (SensorNodeNum != waterCoil.AirOutletNodeNum) {
            continue;
        }
        NodeNotFound = false;
        WhichCoil = CoilNum;
        break;
    }
    // now if the sensor node is on the water coil air outlet node then check that
    // a setpoint is also specified on the water coil outlet node
    if (!NodeNotFound) {
        if (WhichCoil > 0) {
            bool EMSSetPointErrorFlag = false;
            switch (ControlledVar) {
            case HVACControllers::CtrlVarType::Temperature: {
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, SensorNodeNum, HVAC::CtrlVarType::Temp, EMSSetPointErrorFlag);
                state.dataLoopNodes->NodeSetpointCheck(SensorNodeNum).needsSetpointChecking = false;
                if (EMSSetPointErrorFlag) {
                    if (!SetPointManager::NodeHasSPMCtrlVarType(state, SensorNodeNum, HVAC::CtrlVarType::Temp)) {
                        std::string_view WaterCoilType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataWaterCoils->WaterCoil(WhichCoil).WaterCoilType)];
                        ShowWarningError(state,
                                         std::format("{}{}=\"{}\". ", RoutineName, WaterCoilType, state.dataWaterCoils->WaterCoil(WhichCoil).Name));
                        ShowContinueError(state, " ..Temperature setpoint not found on coil air outlet node.");
                        ShowContinueError(state,
                                          " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                        ShowContinueError(state, " ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                    }
                }
                break;
            }
            case HVACControllers::CtrlVarType::HumidityRatio: {
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, SensorNodeNum, HVAC::CtrlVarType::MaxHumRat, EMSSetPointErrorFlag);
                state.dataLoopNodes->NodeSetpointCheck(SensorNodeNum).needsSetpointChecking = false;
                if (EMSSetPointErrorFlag) {
                    if (!SetPointManager::NodeHasSPMCtrlVarType(state, SensorNodeNum, HVAC::CtrlVarType::MaxHumRat)) {
                        std::string_view WaterCoilType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataWaterCoils->WaterCoil(WhichCoil).WaterCoilType)];
                        ShowWarningError(state,
                                         std::format("{}{}=\"{}\". ", RoutineName, WaterCoilType, state.dataWaterCoils->WaterCoil(WhichCoil).Name));
                        ShowContinueError(state, " ..Humidity ratio setpoint not found on coil air outlet node.");
                        ShowContinueError(state,
                                          " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                        ShowContinueError(state, " ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                    }
                }
                break;
            }
            case HVACControllers::CtrlVarType::TemperatureAndHumidityRatio: {
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, SensorNodeNum, HVAC::CtrlVarType::Temp, EMSSetPointErrorFlag);
                state.dataLoopNodes->NodeSetpointCheck(SensorNodeNum).needsSetpointChecking = false;
                if (EMSSetPointErrorFlag) {
                    if (!SetPointManager::NodeHasSPMCtrlVarType(state, SensorNodeNum, HVAC::CtrlVarType::Temp)) {
                        std::string_view WaterCoilType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataWaterCoils->WaterCoil(WhichCoil).WaterCoilType)];
                        ShowWarningError(state,
                                         std::format("{}{}=\"{}\". ", RoutineName, WaterCoilType, state.dataWaterCoils->WaterCoil(WhichCoil).Name));
                        ShowContinueError(state, " ..Temperature setpoint not found on coil air outlet node.");
                        ShowContinueError(state,
                                          " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                        ShowContinueError(state, " ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                    }
                }
                EMSSetPointErrorFlag = false;
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, SensorNodeNum, HVAC::CtrlVarType::MaxHumRat, EMSSetPointErrorFlag);
                state.dataLoopNodes->NodeSetpointCheck(SensorNodeNum).needsSetpointChecking = false;
                if (EMSSetPointErrorFlag) {
                    if (!SetPointManager::NodeHasSPMCtrlVarType(state, SensorNodeNum, HVAC::CtrlVarType::MaxHumRat)) {
                        std::string_view WaterCoilType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataWaterCoils->WaterCoil(WhichCoil).WaterCoilType)];
                        ShowWarningError(state,
                                         std::format("{}{}=\"{}\". ", RoutineName, WaterCoilType, state.dataWaterCoils->WaterCoil(WhichCoil).Name));
                        ShowContinueError(state, " ..Humidity ratio setpoint not found on coil air outlet node.");
                        ShowContinueError(state,
                                          " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                        ShowContinueError(state, " ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

Real64 TdbFnHRhPb(EnergyPlusData &state,
                  Real64 const H,  // specific enthalpy {J/kg}
                  Real64 const RH, // relative humidity value (0.0-1.0)
                  Real64 const PB  // barometric pressure {Pascals}
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 1, 2009

    // PURPOSE OF THIS FUNCTION:
    // Given the specific enthalpy, relative humidity, and the
    // barometric pressure, the function returns the dry bulb temperature.

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr Acc(1.0); // Accuracy of result

    auto f = [&state, H, RH, PB](Real64 const Tprov) { return H - Psychrometrics::PsyHFnTdbRhPb(state, Tprov, RH, PB); };

    thread_local General::SolveRootStats solveRootStats;
    int SolFla;

    Real64 Tprov = General::SolveRoot2(state, Acc, 500, SolFla, f, 1.0, 50.0, solveRootStats);
    // if the numerical inversion failed, issue error messages.
    if (SolFla == General::SOLVEROOT_ERROR_ITER) {
        ShowSevereError(state, "Calculation of drybulb temperature failed in TdbFnHRhPb(H,RH,PB)");
        ShowContinueError(state, "   Iteration limit exceeded");
        ShowContinueError(state, std::format("   H=[{:.6f}], RH=[{:.4f}], PB=[{:.5f}].", H, RH, PB));
        return 0.0;
    } else if (SolFla == General::SOLVEROOT_ERROR_INIT) {
        ShowSevereError(state, "Calculation of drybulb temperature failed in TdbFnHRhPb(H,RH,PB)");
        ShowContinueError(state, "  Bad starting values for Tdb");
        ShowContinueError(state, std::format("   H=[{:.6f}], RH=[{:.4f}], PB=[{:.5f}].", H, RH, PB));
        return 0.0;
    } else {
        return Tprov;
    }
}

Real64 EstimateHEXSurfaceArea(EnergyPlusData &state, int const CoilNum) // coil number, [-]
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket A Nigusse, FSEC
    //       DATE WRITTEN   July 2010

    // PURPOSE OF THIS FUNCTION:
    // Splits the UA value of a simple coil:cooling:water heat exchanger model into
    // "A" and U" values.

    // METHODOLOGY EMPLOYED:
    // A typical design U overall heat transfer coefficient is used to split the "UA" into "A"
    // and "U" values. Currently a constant U value calculated for a typical cooling coil is
    // used. The assumptions used to calculate a typical U value are:
    //     (1) tube side water velocity of 2.0 [m/s]
    //     (2) inside to outside total surface area ratio (Ai/Ao) =  0.07 [-]
    //     (3) fins overall efficiency = 0.92 based on aluminum fin, 12 fins per inch, and
    //         fins area to total outside surafce area ratio of about 90%.
    //     (4) air side convection coefficient of 140.0 [W/m2C].  Assumes sensible convection
    //         of 58.0 [W/m2C] and 82.0 [W/m2C] sensible convection equivalent of the mass
    //         transfer coefficient converted using the approximate relation:
    //         hequivalent = hmasstransfer/CpAir.

    // FUNCTION PARAMETER DEFINITIONS:
    constexpr Real64 OverallFinEfficiency(0.92); // Assumes aluminum fins, 12 fins per inch, fins
    // area of about 90% of external surface area Ao.

    constexpr Real64 AreaRatio(0.07); // Heat exchanger Inside to Outside surface area ratio
    // design values range from (Ai/Ao) = 0.06 to 0.08

    // Constant value air side heat transfer coefficient is assumed. This coefficient has sensible
    // (58.d0 [W/m2C]) and latent (82.d0 [W/m2C]) heat transfer coefficient components.
    constexpr Real64 hAirTubeOutside(58.0 + 82.0); // Air side heat transfer coefficient [W/m2C]

    // Tube side water convection heat transfer coefficient of the cooling coil is calculated for
    // inside tube diameter of 0.0122m (~0.5 inch nominal diameter) and water velocity 2.0 m/s:
    static Real64 const hWaterTubeInside(1429.0 * std::pow(2.0, 0.8) * std::pow(0.0122, -0.2)); // water (tube) side heat transfer coefficient [W/m2C]

    // Estimate the overall heat transfer coefficient, UOverallHeatTransferCoef in [W/(m2C)].
    // Neglecting tube wall and fouling resistance, the overall U value can be estimated as:
    // 1/UOverallHeatTransferCoef = 1/(hi*AreaRatio) + 1/(ho*OverallFinEfficiency)
    static Real64 const UOverallHeatTransferCoef_inv(
        1.0 / (hWaterTubeInside * AreaRatio) +
        1.0 / (hAirTubeOutside * OverallFinEfficiency)); // Inverse of overall heat transfer coefficient for coil [W/m2C]

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    waterCoil.UACoilTotal = 1.0 / (1.0 / waterCoil.UACoilExternal + 1.0 / waterCoil.UACoilInternal);

    // the heat exchanger surface area is calculated as follows:
    return waterCoil.UACoilTotal * UOverallHeatTransferCoef_inv; // Heat exchanger surface area [m2]
}

int GetWaterCoilIndex(EnergyPlusData &state,
                      std::string_view CoilType,   // must match coil types in this module
                      std::string const &CoilName, // must match coil names for the coil type
                      bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B. Nigusse, FSEC
    //       DATE WRITTEN   Feb 2012

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the index for the given coil and returns it.  If incorrect coil
    // type or name is given, ErrorsFound is returned as true and node number is returned
    // as zero.

    // Return value
    int IndexNum; // returned coil index if matched coil

    // Obtains and allocates WaterCoil related parameters from input file
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    IndexNum = 0;
    if (CoilType == "COIL:HEATING:WATER") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
    } else if (CoilType == "COIL:COOLING:WATER") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
    } else if (CoilType == "COIL:COOLING:WATER:DETAILEDGEOMETRY") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
    } else {
        IndexNum = 0;
    }

    if (IndexNum == 0) {
        ShowSevereError(state, std::format("GetWaterCoilIndex: Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
    }

    return IndexNum;
}
int GetCompIndex(EnergyPlusData &state, CoilModel compType, std::string_view const coilName)
{
    static constexpr std::array<std::string_view, (int)WaterCoils::CoilModel::Num> CoilModelNamesUC = {
        "COIL:HEATING:WATER", "COIL:COOLING:WATER", "COIL:COOLING:WATER:DETAILED"};

    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int index = Util::FindItemInList(coilName, state.dataWaterCoils->WaterCoil);

    if (index == 0) { // may not find coil name
        ShowSevereError(
            state, std::format("GetWaterCoilIndex: Could not find CoilType = \"{}\" with Name = \"{}\"", CoilModelNamesUC[(int)compType], coilName));
    }
    return index;
}

Real64 GetWaterCoilCapacity(EnergyPlusData &state,
                            std::string_view const CoilType, // must match coil types in this module
                            std::string const &CoilName,     // must match coil names for the coil type
                            bool &ErrorsFound                // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad, FSEC
    //       DATE WRITTEN   Sep 2013

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the capacity for the given coil and returns it.  If incorrect coil
    // type or name is given, ErrorsFound is returned as true and capacity is returned
    // as zero.

    // Obtains and allocates WaterCoil related parameters from input file
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int IndexNum;           // index to water coil
    Real64 Capacity = -1.0; // returned coil capacity if matched coil

    if (CoilType == "COIL:HEATING:WATER") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
        Capacity = state.dataWaterCoils->WaterCoil(IndexNum).DesWaterHeatingCoilRate;
    } else if (CoilType == "COIL:COOLING:WATER") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
        Capacity = state.dataWaterCoils->WaterCoil(IndexNum).DesWaterCoolingCoilRate;
    } else if (CoilType == "COIL:COOLING:WATER:DETAILEDGEOMETRY") {
        IndexNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
        Capacity = state.dataWaterCoils->WaterCoil(IndexNum).DesWaterCoolingCoilRate;
    } else {
        IndexNum = 0;
    }

    if (IndexNum == 0) {
        ShowSevereError(state, std::format("GetWaterCoilCapacity: Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
    }

    return Capacity;
}

void UpdateWaterToAirCoilPlantConnection(EnergyPlusData &state,
                                         DataPlant::PlantEquipmentType const CoilType,
                                         std::string const &CoilName,
                                         [[maybe_unused]] int const EquipFlowCtrl,   // Flow control mode for the equipment
                                         int const LoopNum,                          // Plant loop index for where called from
                                         const DataPlant::LoopSideLocation LoopSide, // Plant loop side index for where called from
                                         int &CompIndex,                             // Chiller number pointer
                                         [[maybe_unused]] bool const FirstHVACIteration,
                                         bool const &InitLoopEquip // If not zero, calculate the max load for operating conditions
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   February 2010

    // PURPOSE OF THIS SUBROUTINE:
    // update sim routine called from plant

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    int CoilNum;
    bool DidAnythingChange(false); // set to true if conditions changed
    int InletNodeNum;
    int OutletNodeNum;

    // Find the correct water coil
    if (CompIndex == 0) {
        CoilNum = Util::FindItemInList(CoilName, state.dataWaterCoils->WaterCoil);
        if (CoilNum == 0) {
            ShowFatalError(state, std::format("UpdateWaterToAirCoilPlantConnection: Specified Coil not one of Valid water coils={}", CoilName));
        }
        CompIndex = CoilNum;
    } else {
        CoilNum = CompIndex;
        if (CoilNum > state.dataWaterCoils->NumWaterCoils || CoilNum < 1) {
            ShowFatalError(state,
                           std::format("UpdateWaterToAirCoilPlantConnection:  Invalid CompIndex passed={}, Number of Coils={}, Entered Coil name={}",
                                       CoilNum,
                                       state.dataWaterCoils->NumWaterCoils,
                                       CoilName));
        }
        auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
        if (state.dataGlobal->KickOffSimulation) {
            if (CoilName != waterCoil.Name) {
                ShowFatalError(
                    state,
                    std::format("UpdateWaterToAirCoilPlantConnection: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                CoilNum,
                                CoilName,
                                waterCoil.Name));
            }
            if (CoilType != waterCoil.WaterCoilType) {
                ShowFatalError(
                    state,
                    std::format("UpdateWaterToAirCoilPlantConnection: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                CoilNum,
                                CoilName,
                                DataPlant::PlantEquipTypeNames[static_cast<int>(CoilType)]));
            }
        }
    }

    if (InitLoopEquip) {
        return;
    }

    DidAnythingChange = false;

    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    InletNodeNum = waterCoil.WaterInletNodeNum;
    OutletNodeNum = waterCoil.WaterOutletNodeNum;

    if (state.dataLoopNodes->Node(InletNodeNum).Temp != waterCoil.InletWaterTemp) {
        DidAnythingChange = true;
    }

    if (state.dataLoopNodes->Node(OutletNodeNum).Temp != waterCoil.OutletWaterTemp) {
        DidAnythingChange = true;
    }

    if (state.dataLoopNodes->Node(InletNodeNum).MassFlowRate != waterCoil.OutletWaterMassFlowRate) {
        DidAnythingChange = true;
        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(InletNodeNum).MassFlowRate; // make sure flows are consistent
    }

    if (state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate != waterCoil.OutletWaterMassFlowRate) {
        DidAnythingChange = true;
    }

    if (DidAnythingChange) {
        // set sim flag for this loop
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = true;
        // set sim flags for air side users of coils

        state.dataHVACGlobal->SimAirLoopsFlag = true;
        state.dataHVACGlobal->SimZoneEquipmentFlag = true;
    } else { // nothing changed so turn off sim flag
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = false;
    }
}

Sched::Schedule *GetWaterCoilAvailSched(EnergyPlusData &state,
                                        std::string_view const CoilType, // must match coil types in this module
                                        std::string const &CoilName,     // must match coil names for the coil type
                                        bool &ErrorsFound                // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Chandan Sharma, FSEC
    //       DATE WRITTEN   February 2013

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the availability schedule index.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
    // as zero.

    // Obtains and Allocates HeatingCoil related parameters from input file
    // Obtains and Allocates DXCoils
    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    int WhichCoil = 0;

    if (Util::SameString(CoilType, "Coil:Heating:Water") || Util::SameString(CoilType, "Coil:Cooling:Water") ||
        Util::SameString(CoilType, "Coil:Cooling:Water:DetailedGeometry")) {
        WhichCoil = Util::FindItem(CoilName, state.dataWaterCoils->WaterCoil);
        if (WhichCoil != 0) {
            return state.dataWaterCoils->WaterCoil(WhichCoil).availSched;
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format("GetCoilAvailScheduleIndex: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
        ErrorsFound = true;
        return nullptr;
    }

    return nullptr;
}

void SetWaterCoilData(EnergyPlusData &state,
                      int const CoilNum,                                  // Number of hot water heating Coil
                      bool &ErrorsFound,                                  // Set to true if certain errors found
                      ObjexxFCL::Optional_bool DesiccantRegenerationCoil, // Flag that this coil is used as regeneration air heating coil
                      ObjexxFCL::Optional_int DesiccantDehumIndex,        // Index for the desiccant dehum system where this caoil is used
                      ObjexxFCL::Optional_bool heatRecoveryCoil)          // true if water coil is connected to heat recovery loop
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   February 2016

    // PURPOSE OF THIS FUNCTION:
    // This function sets data to water Heating Coil using the coil index and arguments passed

    if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
        GetWaterCoilInput(state);
        state.dataWaterCoils->GetWaterCoilsInputFlag = false;
    }

    if (CoilNum <= 0 || CoilNum > state.dataWaterCoils->NumWaterCoils) {
        ShowSevereError(state,
                        std::format("SetHeatingCoilData: called with heating coil Number out of range={} should be >0 and <{}",
                                    CoilNum,
                                    state.dataWaterCoils->NumWaterCoils));
        ErrorsFound = true;
        return;
    }

    auto &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    if (present(DesiccantRegenerationCoil)) {
        waterCoil.DesiccantRegenerationCoil = DesiccantRegenerationCoil;
    }

    if (present(DesiccantDehumIndex)) {
        waterCoil.DesiccantDehumNum = DesiccantDehumIndex;
    }

    if (present(heatRecoveryCoil)) {
        waterCoil.heatRecoveryCoil = heatRecoveryCoil;
    }
}

void EstimateCoilInletWaterTemp(EnergyPlusData &state,
                                int const CoilNum,                // index to heating coil
                                HVAC::FanOp const fanOp,          // fan operating mode
                                Real64 const PartLoadRatio,       // part-load ratio of heating coil
                                Real64 const UAMax,               // maximum UA-Value = design heating capacity
                                Real64 &DesCoilInletWaterTempUsed // estimated coil design inlet water temperature
)
{
    // SUBROUTINE INFORMATION:

    // PURPOSE OF THIS SUBROUTINE:
    // returns estimated coil inlet water temperature given UA value for assumed
    // maximum effectiveness value for heating coil

    // METHODOLOGY EMPLOYED:
    // applies energy balance around the water coil and estimates coil water inlet temperature
    // assuming coil effectiveness of 0.8

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("EstimateCoilInletWaterTemp");
    constexpr Real64 EffectivenessMaxAssumed(0.80);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 WaterMassFlowRate;
    Real64 AirMassFlow;
    Real64 TempAirIn;
    Real64 TempAirOut; // [C]
    Real64 Win;
    Real64 TempWaterIn;
    Real64 UA;
    Real64 CapacitanceAir;
    Real64 CapacitanceWater;
    Real64 CapacitanceMin;
    Real64 CapacitanceMax;
    Real64 NTU;
    Real64 ETA;
    Real64 A;
    Real64 CapRatio;
    Real64 E1;
    Real64 E2;
    Real64 Effec;
    Real64 Cp;

    UA = UAMax;
    DesCoilInletWaterTempUsed = HVAC::DesCoilHWInletTempMin;
    auto const &waterCoil = state.dataWaterCoils->WaterCoil(CoilNum);
    TempAirIn = waterCoil.InletAirTemp;
    Win = waterCoil.InletAirHumRat;
    TempWaterIn = waterCoil.InletWaterTemp;
    // adjust mass flow rates for cycling fan cycling coil operation
    if (fanOp == HVAC::FanOp::Cycling) {
        if (PartLoadRatio > 0.0) {
            AirMassFlow = waterCoil.InletAirMassFlowRate / PartLoadRatio;
            WaterMassFlowRate = min(waterCoil.InletWaterMassFlowRate / PartLoadRatio, waterCoil.MaxWaterMassFlowRate);
        } else {
            AirMassFlow = 0.0;
            WaterMassFlowRate = 0.0;
            return;
        }
    } else {
        AirMassFlow = waterCoil.InletAirMassFlowRate;
        WaterMassFlowRate = waterCoil.InletWaterMassFlowRate;
    }
    if (WaterMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) { // if the coil is operating
        CapacitanceAir = PsyCpAirFnW(Win) * AirMassFlow;
        Cp = waterCoil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, TempWaterIn, RoutineName);
        CapacitanceWater = Cp * WaterMassFlowRate;
        CapacitanceMin = min(CapacitanceAir, CapacitanceWater);
        CapacitanceMax = max(CapacitanceAir, CapacitanceWater);
    } else {
        CapacitanceAir = 0.0;
        CapacitanceWater = 0.0;
        return;
    }
    // calculate DesCoilInletWaterTempUsed
    if (((CapacitanceAir > 0.0) && (CapacitanceWater > 0.0))) {

        if (UA <= 0.0) {
            ShowWarningError(state, std::format("UA is zero for COIL:Heating:Water {}", waterCoil.Name));
            return;
        }
        NTU = UA / CapacitanceMin;
        ETA = std::pow(NTU, 0.22);
        CapRatio = CapacitanceMin / CapacitanceMax;
        A = CapRatio * NTU / ETA;

        if (A > 20.0) {
            A = ETA * 1.0 / CapRatio;
        } else {
            E1 = std::exp(-A);
            A = ETA * (1.0 - E1) / CapRatio;
        }

        if (A > 20.0) {
            Effec = 1.0;
        } else {
            E2 = std::exp(-A);
            Effec = 1.0 - E2;
        }
        TempAirOut = TempAirIn + Effec * CapacitanceMin * (TempWaterIn - TempAirIn) / CapacitanceAir;
        // this formulation assumes coil effectiveness of 0.80 to increase the estimated coil water inlet temperatures
        DesCoilInletWaterTempUsed = CapacitanceAir * (TempAirOut - TempAirIn) / (CapacitanceMin * EffectivenessMaxAssumed) + TempAirIn;
        // water coil should not be sized at coil water inlet temperature lower than 46.0C (for convergence problem in Regulafalsi)
        DesCoilInletWaterTempUsed = max(DesCoilInletWaterTempUsed, HVAC::DesCoilHWInletTempMin);
    }
}

// End of Coil Utility subroutines
// *****************************************************************************

} // namespace EnergyPlus::WaterCoils
