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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace HXAssistCoil {
    // Module containing the simulation routines for a heat exchanger-
    // assisted cooling coil

    // MODULE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   Sept 2003

    // PURPOSE OF THIS MODULE:
    //  To encapsulate the data and algorithms required to
    //  manage the heat exchanger-assisted cooling coil compound component

    // METHODOLOGY EMPLOYED:
    //  Call the air-to-air heat exchanger and cooling coil repeatedly to converge
    //  on the solution instead of relying on the air loop manager for iterations

    // REFERENCES:
    // Kosar, D. 2006. Dehumidification Enhancements, ASHRAE Journal, Vol. 48, No. 2, February 2006.
    // Kosar, D. et al. 2006. Dehumidification Enhancement of Direct Expansion Systems Through Component
    //   Augmentation of the Cooling Coil. 15th Symposium on Improving Building Systems in Hot and Humid
    //   Climates, July 24-26, 2006.

    void SimHXAssistedCoolingCoil(EnergyPlusData &state,
                                  std::string_view HXAssistedCoilName,   // Name of HXAssistedCoolingCoil
                                  bool const FirstHVACIteration,         // FirstHVACIteration flag
                                  HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                                  Real64 const PartLoadRatio,            // Part load ratio of Coil:DX:CoolingBypassFactorEmpirical
                                  int &CompIndex,
                                  HVAC::FanOp const fanOp,                     // Allows the parent object to control fan operation
                                  ObjexxFCL::Optional_bool_const HXUnitEnable, // flag to enable heat exchanger heat recovery
                                  ObjexxFCL::Optional<Real64 const> OnOffAFR,  // Ratio of compressor ON air mass flow rate to AVERAGE over time step
                                  ObjexxFCL::Optional_bool_const EconomizerFlag,                  // OA sys or air loop economizer status
                                  ObjexxFCL::Optional<Real64> QTotOut,                            // the total cooling output of unit
                                  ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode, // Optional dehumbidication mode
                                  ObjexxFCL::Optional<Real64 const> LoadSHR                       // Optional CoilSHR pass over
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Sept 2003

        // PURPOSE OF THIS SUBROUTINE:
        //  This subroutine manages the simulation of the
        //  cooling coil/heat exchanger combination.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HXAssistedCoilNum; // Index for HXAssistedCoolingCoil

        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { // First time subroutine has been called, get input data
            // Get the HXAssistedCoolingCoil input
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag =
                false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        // Find the correct HXAssistedCoolingCoil number
        if (CompIndex == 0) {
            HXAssistedCoilNum = Util::FindItemInList(HXAssistedCoilName, state.dataHVACAssistedCC->HXAssistedCoils);
            if (HXAssistedCoilNum == 0) {
                ShowFatalError(state, format("HX Assisted Coil not found={}", HXAssistedCoilName));
            }
            CompIndex = HXAssistedCoilNum;
        } else {
            HXAssistedCoilNum = CompIndex;
            if (HXAssistedCoilNum > state.dataHVACAssistedCC->TotalNumHXAssistedCoils || HXAssistedCoilNum < 1) {
                ShowFatalError(state,
                               format("SimHXAssistedCoolingCoil: Invalid CompIndex passed={}, Number of HX Assisted Cooling Coils={}, Coil name={}",
                                      HXAssistedCoilNum,
                                      state.dataHVACAssistedCC->TotalNumHXAssistedCoils,
                                      HXAssistedCoilName));
            }
            if (state.dataHVACAssistedCC->CheckEquipName(HXAssistedCoilNum)) {
                if (!HXAssistedCoilName.empty() && HXAssistedCoilName != state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum).Name) {
                    ShowFatalError(state,
                                   format("SimHXAssistedCoolingCoil: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                          HXAssistedCoilNum,
                                          HXAssistedCoilName,
                                          state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum).Name));
                }
                state.dataHVACAssistedCC->CheckEquipName(HXAssistedCoilNum) = false;
            }
        }
        
        SimHXAssistedCoolingCoil(state,
                                 HXAssistedCoilNum, 
                                 FirstHVACIteration,
                                 compressorOp,
                                 PartLoadRatio,
                                 fanOp,
                                 HXUnitEnable,
                                 OnOffAFR,
                                 EconomizerFlag,
                                 QTotOut,
                                 DehumidificationMode,
                                 LoadSHR);
    }

    void SimHXAssistedCoolingCoil(EnergyPlusData &state,
                                  int const coilNum, 
                                  bool const FirstHVACIteration,         // FirstHVACIteration flag
                                  HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                                  Real64 const PartLoadRatio,            // Part load ratio of Coil:DX:CoolingBypassFactorEmpirical
                                  HVAC::FanOp const fanOp,                     // Allows the parent object to control fan operation
                                  ObjexxFCL::Optional_bool_const HXUnitEnable, // flag to enable heat exchanger heat recovery
                                  ObjexxFCL::Optional<Real64 const> OnOffAFR,  // Ratio of compressor ON air mass flow rate to AVERAGE over time step
                                  ObjexxFCL::Optional_bool_const EconomizerFlag,                  // OA sys or air loop economizer status
                                  ObjexxFCL::Optional<Real64> QTotOut,                            // the total cooling output of unit
                                  ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode, // Optional dehumbidication mode
                                  ObjexxFCL::Optional<Real64 const> LoadSHR                       // Optional CoilSHR pass over
    )
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        Real64 AirFlowRatio;   // Ratio of compressor ON air mass flow rate to AVEARAGE over time step
        bool HXUnitOn;         // flag to enable heat exchanger

        // Initialize HXAssistedCoolingCoil Flows
        InitHXAssistedCoolingCoil(state, coilNum);

        if (present(HXUnitEnable)) {
            HXUnitOn = HXUnitEnable;
        } else {
            HXUnitOn = true;
        }

        if (compressorOp == HVAC::CompressorOp::Off) {
            HXUnitOn = false;
        }

        // Calculate the HXAssistedCoolingCoil performance and the coil outlet conditions
        if (present(OnOffAFR)) {
            AirFlowRatio = OnOffAFR;
        } else {
            AirFlowRatio = 1.0;
        }
        
        if (present(DehumidificationMode) && present(LoadSHR) &&
            state.dataHVACAssistedCC->HXAssistedCoils(coilNum).coolCoilType == HVAC::CoilType::CoolingDX) {
            CalcHXAssistedCoolingCoil(state,
                                      coilNum,
                                      FirstHVACIteration,
                                      compressorOp,
                                      PartLoadRatio,
                                      HXUnitOn,
                                      fanOp,
                                      AirFlowRatio,
                                      EconomizerFlag,
                                      DehumidificationMode,
                                      LoadSHR);
        } else {
            CalcHXAssistedCoolingCoil(
                state, coilNum, FirstHVACIteration, compressorOp, PartLoadRatio, HXUnitOn, fanOp, AirFlowRatio, EconomizerFlag);
        }

        // Update the current HXAssistedCoil output
        //  Call UpdateHXAssistedCoolingCoil(HXAssistedCoilNum), not required. Updates done by the HX and cooling coil components.

        // Report the current HXAssistedCoil output
        //  Call ReportHXAssistedCoolingCoil(HXAssistedCoilNum), not required. No reporting variables for this compound component.

        if (present(QTotOut)) {
            int InletNodeNum = state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilInNodeNum;
            int OutletNodeNum = state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilOutNodeNum;
            Real64 AirMassFlow = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate;
            QTotOut = AirMassFlow * (state.dataLoopNodes->Node(InletNodeNum).Enthalpy - state.dataLoopNodes->Node(OutletNodeNum).Enthalpy);
        }
    }
  
    // Get Input Section of the Module
    //******************************************************************************

    void GetHXAssistedCoolingCoilInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Sept 2003

        // PURPOSE OF THIS SUBROUTINE:
        //  Obtains input data for this compound object and stores it in data structure

        // METHODOLOGY EMPLOYED:
        //  Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetHXAssistedCoolingCoilInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetHXAssistedCoolingCoilInput";

        auto &s_node = state.dataLoopNodes;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                    // Number of alpha inputs
        int NumNums;                      // Number of number inputs
        int IOStat;                       // Return status from GetObjectItem call
        bool ErrorsFound(false);          // set TRUE if errors detected in input
        bool CoolingCoilErrFlag;          // Error flag for cooling coil node numbers mining call
        std::string CurrentModuleObject;  // Object type for getting and error messages
        Array1D_string AlphArray;         // Alpha input items for object
        Array1D_string cAlphaFields;      // Alpha field names
        Array1D_string cNumericFields;    // Numeric field names
        Array1D<Real64> NumArray;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;        // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;      // Logical array, numeric field input BLANK = .TRUE.
        int TotalArgs(0);                 // Total number of alpha and numeric arguments (max) for a

        int NumHXAssistedDXCoils =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "CoilSystem:Cooling:DX:HeatExchangerAssisted");
        int NumHXAssistedWaterCoils =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "CoilSystem:Cooling:Water:HeatExchangerAssisted");
        state.dataHVACAssistedCC->TotalNumHXAssistedCoils = NumHXAssistedDXCoils + NumHXAssistedWaterCoils;
        if (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) {
            state.dataHVACAssistedCC->HXAssistedCoils.allocate(state.dataHVACAssistedCC->TotalNumHXAssistedCoils);
            state.dataHVACAssistedCC->HXAssistedCoilOutletTemp.allocate(state.dataHVACAssistedCC->TotalNumHXAssistedCoils);
            state.dataHVACAssistedCC->HXAssistedCoilOutletHumRat.allocate(state.dataHVACAssistedCC->TotalNumHXAssistedCoils);
            state.dataHVACAssistedCC->CheckEquipName.dimension(state.dataHVACAssistedCC->TotalNumHXAssistedCoils, true);
            state.dataHVACAssistedCC->UniqueHXAssistedCoilNames.reserve(state.dataHVACAssistedCC->TotalNumHXAssistedCoils);
        }

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "CoilSystem:Cooling:DX:HeatExchangerAssisted", TotalArgs, NumAlphas, NumNums);
        int MaxNums = NumNums;
        int MaxAlphas = NumAlphas;
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "CoilSystem:Cooling:Water:HeatExchangerAssisted", TotalArgs, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        AlphArray.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNums);
        NumArray.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        // Get the data for the Coil:DX:CoolingHeatExchangerAssisted objects
        CurrentModuleObject = "CoilSystem:Cooling:DX:HeatExchangerAssisted";

        for (int HXAssistedCoilNum = 1; HXAssistedCoilNum <= NumHXAssistedDXCoils; ++HXAssistedCoilNum) {
            auto &hxCoil = state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum);
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     HXAssistedCoilNum,
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
            
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataHVACAssistedCC->UniqueHXAssistedCoilNames, AlphArray(1), CurrentModuleObject, ErrorsFound);

            hxCoil.Name = AlphArray(1);
            hxCoil.hxType = static_cast<HVAC::HXType>(getEnumValue(HVAC::hxTypeNamesUC, AlphArray(2)));
            hxCoil.HeatExchangerName = AlphArray(3);
            hxCoil.HeatExchangerNum = HeatRecovery::GetHeatExchangerIndex(state, hxCoil.HeatExchangerName);
            if (hxCoil.HeatExchangerNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(3), hxCoil.HeatExchangerName);
                ErrorsFound = true;
            } else {
                hxCoil.MaxAirFlowRate = HeatRecovery::GetSupplyAirFlowRate(state, hxCoil.HeatExchangerNum);
                hxCoil.SupplyAirInNodeNum = HeatRecovery::GetSupplyInletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SupplyAirOutNodeNum = HeatRecovery::GetSupplyOutletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SecAirInNodeNum = HeatRecovery::GetSecondaryInletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SecAirOutNodeNum = HeatRecovery::GetSecondaryOutletNode(state, hxCoil.HeatExchangerNum);
            }
            
            hxCoil.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, AlphArray(4)));
            hxCoil.CoolCoilName = AlphArray(5);

            if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX) {
                hxCoil.coilType = HVAC::CoilType::CoolingDXHXAssisted;
                // What is this factory thing, seriously?
                hxCoil.CoolCoilNum = CoilCoolingDX::factory(state, hxCoil.CoolCoilName); 
                if (hxCoil.CoolCoilNum < 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), hxCoil.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    auto &coilDX = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum];
                    hxCoil.Capacity = coilDX.performance->ratedGrossTotalCap();
                    hxCoil.DXCoilNumOfSpeeds = coilDX.performance->numSpeeds();
                }
                
            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                hxCoil.coilType = HVAC::CoilType::CoolingDXHXAssisted;
                hxCoil.CoolCoilNum = DXCoils::GetCoilIndex(state, hxCoil.CoolCoilName);
                if (hxCoil.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), hxCoil.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    hxCoil.DXCoilNumOfSpeeds = DXCoils::GetCoilNumberOfSpeeds(state, hxCoil.CoolCoilNum);
                    hxCoil.Capacity = DXCoils::GetCoilCapacity(state, hxCoil.CoolCoilNum);
                }
                
            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                hxCoil.coilType = HVAC::CoilType::CoolingDXHXAssisted;
                hxCoil.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, AlphArray(5));
                if (hxCoil.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), AlphArray(5));
                    ErrorsFound = true;
                } else {
                    hxCoil.Capacity = VariableSpeedCoils::GetCoilCapacity(state, hxCoil.CoolCoilNum);
                    hxCoil.DXCoilNumOfSpeeds = VariableSpeedCoils::GetCoilNumOfSpeeds(state, hxCoil.CoolCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(4), AlphArray(4));
                ErrorsFound = true;
            }

            if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX) {
                hxCoil.CoolCoilInNodeNum = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].evapInletNodeIndex;
                if (hxCoil.SupplyAirOutNodeNum != hxCoil.CoolCoilInNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The supply air outlet node name in heat exchanger {}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(
                        state,
                        format("must match the cooling coil inlet node name in {}=\"{}\"", HVAC::coilTypeNames[(int)hxCoil.coolCoilType], hxCoil.CoolCoilName));
                    ShowContinueError(state,
                                      format("Heat exchanger supply air outlet node name=\"{}\"", s_node->NodeID(hxCoil.SupplyAirOutNodeNum)));
                    ShowContinueError(state, format("Cooling coil air inlet node name=\"{}\"", s_node->NodeID(hxCoil.CoolCoilInNodeNum)));
                    ErrorsFound = true;
                }

                hxCoil.CoolCoilOutNodeNum = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].evapOutletNodeIndex;
                if (hxCoil.SecAirInNodeNum != hxCoil.CoolCoilOutNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The secondary air inlet node name in heat exchanger {}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(state,
                                      format("must match the cooling coil air outlet node name in {}=\"{}\"",
                                             HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                             hxCoil.CoolCoilName));
                    ShowContinueError(
                        state, format("Heat exchanger secondary air inlet node name =\"{}\".", s_node->NodeID(hxCoil.SecAirInNodeNum)));
                    ShowContinueError(state,
                                      format("Cooling coil air outlet node name =\"{}\".", s_node->NodeID(hxCoil.CoolCoilOutNodeNum)));
                    ErrorsFound = true;
                }
                hxCoil.CoolCoilCondenserInNodeNum = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].condInletNodeIndex;

            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                //         Check node names in heat exchanger and coil objects for consistency
                hxCoil.CoolCoilInNodeNum = DXCoils::GetCoilAirInletNode(state, hxCoil.CoolCoilNum);
                if (hxCoil.SupplyAirOutNodeNum != hxCoil.CoolCoilInNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The supply air outlet node name in heat exchanger = {}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(
                        state,
                        format("must match the cooling coil inlet node name in = {}=\"{}\"", HVAC::coilTypeNames[(int)hxCoil.coolCoilType], hxCoil.CoolCoilName));
                    ShowContinueError(state,
                                      format("Heat exchanger supply air outlet node name=\"{}\"", s_node->NodeID(hxCoil.SupplyAirOutNodeNum)));
                    ShowContinueError(state, format("Cooling coil air inlet node name=\"{}\"", s_node->NodeID(hxCoil.CoolCoilInNodeNum)));
                    ErrorsFound = true;
                }

                hxCoil.CoolCoilOutNodeNum = DXCoils::GetCoilAirOutletNode(state, hxCoil.CoolCoilNum);
                if (hxCoil.SecAirInNodeNum != hxCoil.CoolCoilOutNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The secondary air inlet node name in heat exchanger ={}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(state,
                                      format("must match the cooling coil air outlet node name in = {}=\"{}\".",
                                             HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                             hxCoil.CoolCoilName));
                    ShowContinueError(
                        state, format("Heat exchanger secondary air inlet node name =\"{}\".", s_node->NodeID(hxCoil.SecAirInNodeNum)));
                    ShowContinueError(state,
                                      format("Cooling coil air outlet node name =\"{}\".", s_node->NodeID(hxCoil.CoolCoilOutNodeNum)));
                    ErrorsFound = true;
                }

                hxCoil.CoolCoilCondenserInNodeNum = DXCoils::GetCoilCondenserInletNode(state, hxCoil.CoolCoilNum);

            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                //         Check node names in heat exchanger and coil objects for consistency
                hxCoil.CoolCoilInNodeNum = VariableSpeedCoils::GetCoilAirInletNode(state, hxCoil.CoolCoilNum);
                if (hxCoil.SupplyAirOutNodeNum != hxCoil.CoolCoilInNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The supply air outlet node name in heat exchanger = {}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(
                        state,
                        format("must match the cooling coil inlet node name in = {}=\"{}\"", HVAC::coilTypeNames[(int)hxCoil.coolCoilType], hxCoil.CoolCoilName));
                    ShowContinueError(state,
                                      format("Heat exchanger supply air outlet node name=\"{}\"", s_node->NodeID(hxCoil.SupplyAirOutNodeNum)));
                    ShowContinueError(state, format("Cooling coil air inlet node name=\"{}\"", s_node->NodeID(hxCoil.CoolCoilInNodeNum)));
                    ErrorsFound = true;
                }

                hxCoil.CoolCoilOutNodeNum = VariableSpeedCoils::GetCoilAirOutletNode(state, hxCoil.CoolCoilNum);
                if (hxCoil.CoolCoilOutNodeNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), AlphArray(5));
                    ErrorsFound = true;
                }
                if (hxCoil.SecAirInNodeNum != hxCoil.CoolCoilOutNodeNum) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                    ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                    ShowContinueError(state,
                                      format("The secondary air inlet node name in heat exchanger ={}=\"{}\"",
                                             HVAC::hxTypeNames[(int)hxCoil.hxType],
                                             hxCoil.HeatExchangerName));
                    ShowContinueError(state,
                                      format("must match the cooling coil air outlet node name in = {}=\"{}\".",
                                             HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                             hxCoil.CoolCoilName));
                    ShowContinueError(
                        state, format("Heat exchanger secondary air inlet node name =\"{}\".", s_node->NodeID(hxCoil.SecAirInNodeNum)));
                    ShowContinueError(state,
                                      format("Cooling coil air outlet node name =\"{}\".", s_node->NodeID(hxCoil.CoolCoilOutNodeNum)));
                    ErrorsFound = true;
                }
                
                hxCoil.CoolCoilCondenserInNodeNum = VariableSpeedCoils::GetCoilCondenserInletNode(state, hxCoil.CoolCoilNum);
            }

            BranchNodeConnections::TestCompSet(state,
                                               HVAC::coilTypeNames[(int)hxCoil.coilType],
                                               hxCoil.Name,
                                               s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                               s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                               "Air Nodes");

            hxCoil.HXCoilInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingDXHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);
            // no need to capture CoolingCoilInletNodeNum as the return value, it's not used anywhere
            // Actually, it's a good idea to capture it anyway
            hxCoil.CoolCoilInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::CoilSystemCoolingDXHeatExchangerAssisted,
                                                hxCoil.Name,
                                                DataLoopNode::NodeFluidType::Air,
                                                DataLoopNode::ConnectionType::Internal,
                                                NodeInputManager::CompFluidStream::Primary,
                                                DataLoopNode::ObjectIsParent);

            hxCoil.HXCoilExhaustAirInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingDXHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Internal,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);
            hxCoil.HXCoilOutNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingDXHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                                 hxCoil.CoolCoilName,
                                                 s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                 s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                 "Air Nodes");
            // Add heat exchanger to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                 hxCoil.HeatExchangerName,
                                                 s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                                 s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                 "Process Air Nodes");
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                 hxCoil.HeatExchangerName,
                                                 s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                 s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                                 "Secondary Air Nodes");

        } // End of the Coil:DX:CoolingHXAssisted Loop

        // Get the data for the Coil:Water:CoolingHeatExchangerAssisted objects
        CurrentModuleObject = "CoilSystem:Cooling:Water:HeatExchangerAssisted";

        for (int HXAssistedCoilNum = NumHXAssistedDXCoils + 1; HXAssistedCoilNum <= state.dataHVACAssistedCC->TotalNumHXAssistedCoils;
             ++HXAssistedCoilNum) {
            int thisWaterHXNum = HXAssistedCoilNum - NumHXAssistedDXCoils;
            auto &hxCoil = state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     thisWaterHXNum,
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

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataHVACAssistedCC->UniqueHXAssistedCoilNames, AlphArray(1), CurrentModuleObject, ErrorsFound);

            hxCoil.Name = AlphArray(1);

            hxCoil.hxType = static_cast<HVAC::HXType>(getEnumValue(HVAC::hxTypeNamesUC, AlphArray(2)));
            if (hxCoil.hxType == HVAC::HXType::Desiccant_Balanced) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(2), AlphArray(2));
                ErrorsFound = true;
            }
            
            hxCoil.HeatExchangerName = AlphArray(3);
            hxCoil.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, AlphArray(4)));
            hxCoil.CoolCoilName = AlphArray(5);

            hxCoil.HeatExchangerNum = HeatRecovery::GetHeatExchangerIndex(state, hxCoil.HeatExchangerName);
            if (hxCoil.HeatExchangerNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(5), hxCoil.HeatExchangerName);
                ErrorsFound = true;
            } else {
                hxCoil.SupplyAirInNodeNum = HeatRecovery::GetSupplyInletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SupplyAirOutNodeNum = HeatRecovery::GetSupplyOutletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SecAirInNodeNum = HeatRecovery::GetSecondaryInletNode(state, hxCoil.HeatExchangerNum);
                hxCoil.SecAirOutNodeNum = HeatRecovery::GetSecondaryOutletNode(state, hxCoil.HeatExchangerNum);
            }

            if (hxCoil.coolCoilType == HVAC::CoilType::CoolingWater  ||
                hxCoil.coolCoilType == HVAC::CoilType::CoolingWaterDetailed) {

                hxCoil.coilType = HVAC::CoilType::CoolingWaterHXAssisted;

                hxCoil.CoolCoilNum = WaterCoils::GetCoilIndex(state, hxCoil.CoolCoilName);
                if (hxCoil.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), hxCoil.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    hxCoil.Capacity = WaterCoils::GetCoilCapacity(state, hxCoil.CoolCoilNum);
                    hxCoil.MaxWaterFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, hxCoil.CoolCoilNum);
                    
                    //         Check node names in heat exchanger and coil objects for consistency
                    hxCoil.CoolCoilInNodeNum = WaterCoils::GetCoilAirInletNode(state, hxCoil.CoolCoilNum);
                    hxCoil.CoolCoilWaterInNodeNum = WaterCoils::GetCoilWaterInletNode(state, hxCoil.CoolCoilNum);
                    hxCoil.CoolCoilWaterOutNodeNum = WaterCoils::GetCoilWaterOutletNode(state, hxCoil.CoolCoilNum);

                    CoolingCoilErrFlag = false;
                    HVACControllers::GetControllerNameAndIndex(
                        state, hxCoil.CoolCoilWaterInNodeNum, hxCoil.ControllerName, hxCoil.ControllerIndex, CoolingCoilErrFlag);
                    if (CoolingCoilErrFlag) ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, hxCoil.Name));

                    if (hxCoil.SupplyAirOutNodeNum != hxCoil.CoolCoilInNodeNum) {
                        ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                        ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                        ShowContinueError(state,
                                          format("The supply air outlet node name in heat exchanger = {}=\"{}\"",
                                                 HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                 hxCoil.HeatExchangerName));
                        ShowContinueError(
                                          state,
                                          format("must match the cooling coil inlet node name in = {}=\"{}\"",
                                                 HVAC::coilTypeNames[(int)hxCoil.coolCoilType], hxCoil.CoolCoilName));
                        ShowContinueError(state,
                                          format("Heat exchanger supply air outlet node name =\"{}\"", s_node->NodeID(hxCoil.SupplyAirOutNodeNum)));
                        ShowContinueError(state,
                                          format("Cooling coil air inlet node name = \"{}\"", s_node->NodeID(hxCoil.CoolCoilInNodeNum)));
                        ErrorsFound = true;
                    }
                    CoolingCoilErrFlag = false;

                    hxCoil.CoolCoilOutNodeNum = WaterCoils::GetCoilAirOutletNode(state, hxCoil.CoolCoilNum);
                    if (hxCoil.SecAirInNodeNum != hxCoil.CoolCoilOutNodeNum) {
                         ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, hxCoil.Name));
                         ShowContinueError(state, "Node names are inconsistent in heat exchanger and cooling coil object.");
                         ShowContinueError(state,
                                           format("The secondary air inlet node name in heat exchanger = {}=\"{}\"",
                                                  HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                  hxCoil.HeatExchangerName));
                         ShowContinueError(state,
                                           format("must match the cooling coil air outlet node name in = {}=\"{}\".",
                                                  HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                                  hxCoil.CoolCoilName));
                         ShowContinueError(
                                           state, format("Heat exchanger secondary air inlet node name = \"{}\".", s_node->NodeID(hxCoil.SecAirInNodeNum)));
                         ShowContinueError(state,
                                           format("Cooling coil air outlet node name = \"{}\".", s_node->NodeID(hxCoil.CoolCoilOutNodeNum)));
                         ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(4), AlphArray(4));
                ErrorsFound = true;
            }
            BranchNodeConnections::TestCompSet(state,
                                               HVAC::coilTypeNames[(int)hxCoil.coilType],
                                               hxCoil.Name,
                                               s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                               s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                               "Air Nodes");

            hxCoil.HXCoilInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingWaterHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);
            // no need to capture CoolingCoilInletNodeNum as the return value, it's not used anywhere
            NodeInputManager::GetOnlySingleNode(state,
                                                s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::CoilSystemCoolingWaterHeatExchangerAssisted,
                                                state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum).Name,
                                                DataLoopNode::NodeFluidType::Air,
                                                DataLoopNode::ConnectionType::Internal,
                                                NodeInputManager::CompFluidStream::Primary,
                                                DataLoopNode::ObjectIsParent);
            hxCoil.HXCoilExhaustAirInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingWaterHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Internal,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);
            hxCoil.HXCoilOutNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::CoilSystemCoolingWaterHeatExchangerAssisted,
                                                    hxCoil.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::coilTypeNames[(int)hxCoil.coolCoilType],
                                                 hxCoil.CoolCoilName,
                                                 s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                 s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                 "Air Nodes");
            // Add heat exchanger to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                 hxCoil.HeatExchangerName,
                                                 s_node->NodeID(hxCoil.SupplyAirInNodeNum),
                                                 s_node->NodeID(hxCoil.SupplyAirOutNodeNum),
                                                 "Process Air Nodes");
            BranchNodeConnections::SetUpCompSets(state,
                                                 HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                 hxCoil.Name,
                                                 HVAC::hxTypeNames[(int)hxCoil.hxType],
                                                 hxCoil.HeatExchangerName,
                                                 s_node->NodeID(hxCoil.SecAirInNodeNum),
                                                 s_node->NodeID(hxCoil.SecAirOutNodeNum),
                                                 "Secondary Air Nodes");

        } // End of the Coil:Water:CoolingHXAssisted Loop

        AlphArray.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        NumArray.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Previous error condition causes termination.", RoutineName));
        }
    }

    // End of Get Input subroutines for this Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitHXAssistedCoolingCoil(EnergyPlusData &state, int const HXAssistedCoilNum) // index for HXAssistedCoolingCoil
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Sep 2003
        //       MODIFIED       R. Raustad, June 2007 now using FullLoadOutletConditions from DX Coil data structure

        // PURPOSE OF THIS SUBROUTINE:
        //  This subroutine is for initializations of the HXAssistedCoolingCoil components

        // METHODOLOGY EMPLOYED:
        //  Uses the status flags to trigger initializations.

        auto &s_node = state.dataLoopNodes;
      
        // Do these initializations every time
        auto &hxCoil = state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum);
        hxCoil.MassFlowRate = s_node->Node(hxCoil.HXCoilInNodeNum).MassFlowRate;

        if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX) {
            //
            // state.dataCoilCoolingDX->coilCoolingDXs[thisHXCoil.CoolingCoilIndex]
            //     .outletAirDryBulbTemp = 0.0;
            // state.dataCoilCoolingDX->coilCoolingDXs[thisHXCoil.CoolingCoilIndex].outletAirHumRat =
            //     0.0;
        } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
            state.dataDXCoils->DXCoilFullLoadOutAirTemp(hxCoil.CoolCoilNum) = 0.0;
            state.dataDXCoils->DXCoilFullLoadOutAirHumRat(hxCoil.CoolCoilNum) = 0.0;
        } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
            //
        }
    }

    // End Initialization Section of the Module
    //******************************************************************************

    void CalcHXAssistedCoolingCoil(EnergyPlusData &state,
                                   int const HXAssistedCoilNum,                    // Index number for HXAssistedCoolingCoil
                                   bool const FirstHVACIteration,                  // FirstHVACIteration flag
                                   HVAC::CompressorOp const compressorOp,          // compressor operation; 1=on, 0=off
                                   Real64 const PartLoadRatio,                     // Cooling coil part load ratio
                                   bool const HXUnitOn,                            // Flag to enable heat exchanger
                                   HVAC::FanOp const fanOp,                        // Allows parent object to control fan operation
                                   ObjexxFCL::Optional<Real64 const> OnOffAirFlow, // Ratio of compressor ON air mass flow to AVERAGE over time step
                                   ObjexxFCL::Optional_bool_const EconomizerFlag,  // OA (or airloop) econommizer status
                                   ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode, // Optional dehumbidication mode
                                   [[maybe_unused]] ObjexxFCL::Optional<Real64 const> LoadSHR      // Optional coil SHR pass over
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Sept 2003

        // PURPOSE OF THIS SUBROUTINE:
        //  This subroutine models the cooling coil/air-to-air heat exchanger
        //  combination. The cooling coil exiting air temperature is used as
        //  an indicator of convergence.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(50); // Maximum number of iterations

        int CompanionCoilNum; // Index to DX coil

        auto &s_node = state.dataLoopNodes;
        
        auto &hxCoil = state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum);
        Real64 AirMassFlow = hxCoil.MassFlowRate;
        Real64 Error = 1.0;       // Initialize error (CoilOutputTemp last iteration minus current CoilOutputTemp)
        Real64 ErrorLast = Error; // initialize variable used to test loop termination
        int Iter = 0;             // Initialize iteration counter to zero

        // Set mass flow rate at inlet of exhaust side of heat exchanger to supply side air mass flow rate entering this compound object
        s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).MassFlowRate = AirMassFlow;

        if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX ||
            hxCoil.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed ||
            hxCoil.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
            CompanionCoilNum = hxCoil.CoolCoilNum;
        } else {
            CompanionCoilNum = 0;
        }

        // First call to RegulaFalsi uses PLR=0. Nodes are typically setup at full output on this call.
        // A large number of iterations are required to get to result (~36 iterations to get to PLR=0 node conditions).
        // Reset node data to minimize iteration. This initialization reduces the number of iterations by 50%.
        // CAUTION: Do not use Node(x) = Node(y) here, this can overwrite the coil outlet node setpoint.
        if (PartLoadRatio == 0.0) {
            s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).Temp = s_node->Node(hxCoil.HXCoilInNodeNum).Temp;
            s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).HumRat = s_node->Node(hxCoil.HXCoilInNodeNum).HumRat;
            s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).Enthalpy = s_node->Node(hxCoil.HXCoilInNodeNum).Enthalpy;
            s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).MassFlowRate = s_node->Node(hxCoil.HXCoilInNodeNum).MassFlowRate;
        }

        // Force at least 2 iterations to pass outlet node information
        while ((std::abs(Error) > 0.0005 && Iter <= MaxIter) || Iter < 2) {

            HeatRecovery::SimHeatRecovery(state,
                                          hxCoil.HeatExchangerName,
                                          FirstHVACIteration,
                                          hxCoil.HeatExchangerNum,
                                          fanOp,
                                          PartLoadRatio,
                                          HXUnitOn,
                                          CompanionCoilNum,
                                          _,
                                          EconomizerFlag,
                                          _,
                                          hxCoil.coolCoilType);

            if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX) {

                int mSingleMode = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].getNumModes();
                bool singleMode = (mSingleMode == 1);

                Real64 mCoolingSpeedNum = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum]
                                              .performance->numSpeeds(); // used the same for the original variable speed coil

                HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
                if (state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].subcoolReheatFlag) {
                    coilMode = HVAC::CoilMode::SubcoolReheat;
                } else if (DehumidificationMode == HVAC::CoilMode::Enhanced) {
                    coilMode = HVAC::CoilMode::Enhanced;
                }

                Real64 CoilPLR = 1.0;
                if (compressorOp == HVAC::CompressorOp::Off) {
                    mCoolingSpeedNum = 1; // Bypass mixed-speed calculations in called functions
                } else {
                    if (singleMode) {
                        CoilPLR =
                            (mCoolingSpeedNum == 1) ? PartLoadRatio : 0.0; // singleMode allows cycling, but not part load operation at higher speeds
                    } else {
                        CoilPLR = PartLoadRatio;
                    }
                }

                state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum].simulate(state,
                                                                                       coilMode, // partially implemented for HXAssistedCoil
                                                                                       mCoolingSpeedNum,
                                                                                       CoilPLR,
                                                                                       fanOp,
                                                                                       singleMode); //
                
            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                DXCoils::SimDXCoil(state,
                                   hxCoil.CoolCoilName,
                                   compressorOp,
                                   FirstHVACIteration,
                                   hxCoil.CoolCoilNum,
                                   fanOp,
                                   PartLoadRatio,
                                   OnOffAirFlow);
            } else if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                Real64 QZnReq(-1.0);           // Zone load (W), input to variable-speed DX coil
                Real64 QLatReq(0.0);           // Zone latent load, input to variable-speed DX coil
                Real64 OnOffAirFlowRatio(1.0); // ratio of compressor on flow to average flow over time step
                HVAC::CompressorOp compressorOn = compressorOp;
                if (PartLoadRatio == 0.0) {
                    compressorOn = HVAC::CompressorOp::Off;
                }
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          hxCoil.CoolCoilName,
                                                          hxCoil.CoolCoilNum,
                                                          fanOp,
                                                          compressorOn,
                                                          PartLoadRatio,
                                                          hxCoil.DXCoilNumOfSpeeds,
                                                          QZnReq,
                                                          QLatReq,
                                                          OnOffAirFlowRatio); // call vs coil model at top speed.
            } else {
                WaterCoils::SimulateWaterCoilComponents(state, hxCoil.CoolCoilName, FirstHVACIteration, hxCoil.CoolCoilNum);
            }

            Error = state.dataHVACAssistedCC->CoilOutputTempLast - s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).Temp;
            if (Iter > 40) { // check for oscillation (one of these being negative and one positive) before hitting max iteration limit
                if (Error + ErrorLast < 0.000001) {
                    Error = 0.0; // result bounced back and forth with same positive and negative result, no possible solution without this check
                }
            }
            ErrorLast = Error;
            state.dataHVACAssistedCC->CoilOutputTempLast = s_node->Node(hxCoil.HXCoilExhaustAirInNodeNum).Temp;
            ++Iter;
        }

        // Write excessive iteration warning messages
        if (Iter > MaxIter) {
            if (hxCoil.MaxIterCounter < 1) {
                ++hxCoil.MaxIterCounter;
                ShowWarningError(state,
                                 format("{} \"{}\" -- Exceeded max iterations ({}) while calculating operating conditions.",
                                        HVAC::coilTypeNames[(int)hxCoil.coilType],
                                        hxCoil.Name,
                                        MaxIter));
                ShowContinueErrorTimeStamp(state, "");
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               format("{} \"{}\" -- Exceeded max iterations error continues...",
                                                      HVAC::coilTypeNames[(int)hxCoil.coilType],
                                                      hxCoil.Name),
                                               hxCoil.MaxIterIndex);
            }
        }

        state.dataHVACAssistedCC->HXAssistedCoilOutletTemp(HXAssistedCoilNum) = s_node->Node(hxCoil.HXCoilOutNodeNum).Temp;
        state.dataHVACAssistedCC->HXAssistedCoilOutletHumRat(HXAssistedCoilNum) = s_node->Node(hxCoil.HXCoilOutNodeNum).HumRat;
    }

    //        End of Reporting subroutines for the HXAssistedCoil Module
    // *****************************************************************************

    void CheckHXAssistedCoolingCoilSchedule(EnergyPlusData &state,
                                            [[maybe_unused]] std::string const &CompType, // unused1208
                                            std::string_view CompName,
                                            Real64 &Value,
                                            int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2005

        // PURPOSE OF THIS SUBROUTINE:
        // This routine provides a method for outside routines to check if
        // the hx assisted cooling coil is scheduled to be on.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HXAssistedCoilNum;

        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { // First time subroutine has been called, get input data
            // Get the HXAssistedCoolingCoil input
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag =
                false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        // Find the correct Coil number
        if (CompIndex == 0) {
            if (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) {
                HXAssistedCoilNum = Util::FindItem(CompName, state.dataHVACAssistedCC->HXAssistedCoils);
            } else {
                HXAssistedCoilNum = 0;
            }

            if (HXAssistedCoilNum == 0) {
                ShowFatalError(state, format("CheckHXAssistedCoolingCoilSchedule: HX Assisted Coil not found={}", CompName));
            }
            CompIndex = HXAssistedCoilNum;
            Value = 1.0; // not scheduled?
        } else {
            HXAssistedCoilNum = CompIndex;
            if (HXAssistedCoilNum > state.dataHVACAssistedCC->TotalNumHXAssistedCoils || HXAssistedCoilNum < 1) {
                ShowFatalError(state,
                               format("CheckHXAssistedCoolingCoilSchedule: Invalid CompIndex passed={}, Number of Heating Coils={}, Coil name={}",
                                      HXAssistedCoilNum,
                                      state.dataHVACAssistedCC->TotalNumHXAssistedCoils,
                                      CompName));
            }
            if (CompName != state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum).Name) {
                ShowFatalError(
                    state,
                    format("CheckHXAssistedCoolingCoilSchedule: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                           HXAssistedCoilNum,
                           CompName,
                           state.dataHVACAssistedCC->HXAssistedCoils(HXAssistedCoilNum).Name));
            }

            Value = 1.0; // not scheduled?
        }
    }

    Real64 GetHXCoilCapacity(EnergyPlusData &state,
                             std::string_view const coilType, 
                             std::string const &coilName, 
                             bool &ErrorsFound            
    )
    {
        int coilNum = GetCoilIndex(state, coilType, coilName, ErrorsFound);
        return (coilNum == 0) ? -1000.0 : state.dataHVACAssistedCC->HXAssistedCoils(coilNum).Capacity;
#ifdef OLD_CODE
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil capacity for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Return value
        Real64 CoilCapacity(0.0); // returned capacity of matched coil

        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { // First time subroutine has been called, get input data
            // Get the HXAssistedCoolingCoil input
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag =
                false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        bool errFlag = false;

        int WhichCoil = 0;
        if (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) {
            WhichCoil = Util::FindItem(CoilName, state.dataHVACAssistedCC->HXAssistedCoil);
        }

        if (Util::SameString(CoilType, "CoilSystem:Cooling:DX:HeatExchangerAssisted")) {
            if (WhichCoil != 0) {
                // coil does not have capacity in input so mine information from DX cooling coil

                if (state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType_Num == HVAC::CoilDX_Cooling) {
                    int coolingCoilDXIndex = state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilIndex;
                    CoilCapacity = state.dataCoilCoolingDX->coilCoolingDXs[coolingCoilDXIndex].performance->ratedGrossTotalCap();
                } else if (state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType_Num == HVAC::CoilDX_CoolingSingleSpeed) {
                    CoilCapacity = DXCoils::GetCoilCapacity(state,
                                                            state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType,
                                                            state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilName,
                                                            errFlag);
                } else if (state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType_Num == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    CoilCapacity =
                        VariableSpeedCoils::GetCoilCapacityVariableSpeed(state,
                                                                         state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType,
                                                                         state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilName,
                                                                         errFlag);
                }
                if (errFlag) {
                    ShowRecurringWarningErrorAtEnd(
                        state, "Requested DX Coil from CoilSystem:Cooling:DX:HeatExchangerAssisted not found", state.dataHVACAssistedCC->ErrCount);
                }
            }
        } else if (Util::SameString(CoilType, "CoilSystem:Cooling:Water:HeatExchangerAssisted")) {
            if (WhichCoil != 0) {
                // coil does not have capacity in input so mine information from DX cooling coil
                CoilCapacity = WaterCoils::GetWaterCoilCapacity(state,
                                                                state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilType,
                                                                state.dataHVACAssistedCC->HXAssistedCoil(WhichCoil).CoolingCoilName,
                                                                errFlag);
                if (errFlag) {
                    ShowRecurringWarningErrorAtEnd(
                        state, "Requested DX Coil from CoilSystem:Cooling:DX:HeatExchangerAssisted not found", state.dataHVACAssistedCC->ErrCount);
                }
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, format("GetCoilCapacity: Could not find Coil, Type=\"{}\" Name=\"{}\"", CoilType, CoilName));
            ShowContinueError(state, "... Coil Capacity returned as -1000.");
            ErrorsFound = true;
            CoilCapacity = -1000.0;
        }

        if (errFlag) {
            ErrorsFound = true;
        }
        return CoilCapacity;
#endif // OLD_CODE
    }

    int GetCoilAirInletNode(EnergyPlusData &state,
                           std::string_view coilType,   
                           std::string const &coilName, 
                           bool &ErrorsFound            
    )
    {
        int coilNum = GetCoilIndex(state, coilType, coilName, ErrorsFound);
        return (coilNum == 0) ? 0 : state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilInNodeNum;
    }

    int GetCoilWaterInletNode(EnergyPlusData &state,
                              std::string_view const coilType, // must match coil types in this module
                              std::string const &coilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    )
    {
        int coilNum = GetCoilIndex(state, coilType, coilName, ErrorsFound);
        return (coilNum == 0) ? 0 : state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilWaterInNodeNum;
    }

    int GetCoilAirOutletNode(EnergyPlusData &state,
                             std::string_view CoilType,   // must match coil types in this module
                             std::string const &CoilName, // must match coil names for the coil type
                             bool &ErrorsFound            // set to true if problem
                             )
    {
        int coilNum = GetCoilIndex(state, CoilType, CoilName, ErrorsFound);
        return (coilNum == 0) ? 0 : state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilOutNodeNum;
    }

    int GetCoilIndex(EnergyPlusData &state,
                     std::string_view const coilType, 
                     std::string const &coilName, 
                     bool &ErrorsFound            
    )
    {
        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { 
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag = false; 
        }

        int coilNum = (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) ?
            Util::FindItem(coilName, state.dataHVACAssistedCC->HXAssistedCoils) : 0;

        if (coilNum == 0) {
            ShowSevereError(state, format("GetHXCoilIndex: Could not find Coil, Type=\"{}\" Name=\"{}", coilType, coilName));
            ErrorsFound = true;
        }

        return coilNum;
    }

    int GetCoilIndex(EnergyPlusData &state, std::string const &coilName)
    {
        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { 
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag = false; 
        }

        return Util::FindItem(coilName, state.dataHVACAssistedCC->HXAssistedCoils);
    }
  
    bool VerifyHeatExchangerParent(EnergyPlusData &state,
                                   std::string const &HXType, // must match coil types in this module
                                   std::string const &HXName  // must match coil names for the coil type
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   January 2009

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given heat exchanger name and type and returns true or false.

        // Obtains and allocates HXAssistedCoolingCoil related parameters from input file
        if (state.dataHVACAssistedCC->GetCoilsInputFlag) { // First time subroutine has been called, get input data
            // Get the HXAssistedCoolingCoil input
            GetHXAssistedCoolingCoilInput(state);
            state.dataHVACAssistedCC->GetCoilsInputFlag =
                false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        int WhichCoil = 0;
        if (state.dataHVACAssistedCC->TotalNumHXAssistedCoils > 0) {
            WhichCoil = Util::FindItem(HXName, state.dataHVACAssistedCC->HXAssistedCoils, &HXAssistedCoilParameters::HeatExchangerName);
        }

        if (WhichCoil != 0) {
            if (Util::SameString(HVAC::hxTypeNames[(int)state.dataHVACAssistedCC->HXAssistedCoils(WhichCoil).hxType], HXType)) {
                return true;
            }
        }
        return false;
    }

    //        End of Utility subroutines for the HXAssistedCoil Module
    // *****************************************************************************

    HVAC::CoilType GetCoilChildCoilType(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).coolCoilType;
    }

    std::string GetCoilChildCoilName(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilName;
    }

    int GetCoilChildCoilIndex(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilNum;
    }
  
    Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).MaxWaterFlowRate;
    }

    Real64 GetCoilMaxAirFlowRate(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).MaxAirFlowRate;
    }
  
    Real64 GetCoilCapacity(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).Capacity;
    }

    int GetCoilAirInletNode(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilInNodeNum;
    }

    int GetCoilAirOutletNode(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).HXCoilOutNodeNum;
    }
  
    int GetCoilWaterInletNode(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilWaterInNodeNum;
    }

    int GetCoilWaterOutletNode(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilWaterOutNodeNum;
    }

    int GetCoilCondenserInletNode(EnergyPlusData &state, int const coilNum)
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return state.dataHVACAssistedCC->HXAssistedCoils(coilNum).CoolCoilCondenserInNodeNum;
    }


    Real64 GetCoilScheduleValue(EnergyPlusData &state, int const coilNum) 
    {
        assert(coilNum > 0 && coilNum <= (int)state.dataHVACAssistedCC->HXAssistedCoils.size());
        return 1.0; // Not scheduled? Always available?
    }

} // namespace HXAssistCoil
  
} // namespace EnergyPlus
