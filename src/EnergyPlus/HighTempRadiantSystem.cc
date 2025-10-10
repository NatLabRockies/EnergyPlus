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

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/HighTempRadiantSystem.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HighTempRadiantSystem {

    // Module containing the routines dealing with the high temperature radiant systems

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   February 2001

    // PURPOSE OF THIS MODULE:
    // The purpose of this module is to simulate high temperature radiant systems.
    // It is the intention of this module to cover all types of high temperature
    // radiant systems (gas and electric)

    // METHODOLOGY EMPLOYED:
    // Based on work done in BLAST, the EnergyPlus low temperature radiant system
    // model, this model has similar inherent challenges that are similar to the
    // low temperature radiant system.  Because it is a system that directly
    // effects the surface heat balances, it must be a part of both the heat
    // balance routines and linked in with the HVAC system.
    // REFERENCES:
    // Building Systems Laboratory, BLAST User's Guide/Reference.
    // Maloney, Dan. 1987. "Development of a radiant heater model and the
    //   incorporation of thermal comfort considerations into the BLAST
    //   energy analysis program", M.S. thesis, University of Illinois at
    //   Urbana-Champaign (Dept. of Mechanical and Industrial Engineering).

    // MODULE PARAMETER DEFINITIONS:
    constexpr std::array<std::string_view, static_cast<int>(RadControlType::Num)> radControlTypeNamesUC = {"MEANAIRTEMPERATURE",
                                                                                                           "MEANRADIANTTEMPERATURE",
                                                                                                           "OPERATIVETEMPERATURE",
                                                                                                           "MEANAIRTEMPERATURESETPOINT",
                                                                                                           "MEANRADIANTTEMPERATURESETPOINT",
                                                                                                           "OPERATIVETEMPERATURESETPOINT"};

    void SimHighTempRadiantSystem(EnergyPlusData &state,
                                  std::string_view CompName,     // name of the low temperature radiant system
                                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                  Real64 &LoadMet,               // load met by the radiant system, in Watts
                                  int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the "manager" for the high temperature radiant
        // system model.  It is called from the outside and controls the
        // actions and subroutine calls to lower levels as appropriate.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus manager subroutine layout

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int RadSysNum; // Radiant system number/index in local derived types

        if (state.dataHighTempRadSys->GetInputFlag) {
            bool ErrorsFoundInGet = false;
            GetHighTempRadiantSystem(state, ErrorsFoundInGet);
            if (ErrorsFoundInGet) {
                ShowFatalError(state, "GetHighTempRadiantSystem: Errors found in input.  Preceding condition(s) cause termination.");
            }
            state.dataHighTempRadSys->GetInputFlag = false;
        }

        // Find the correct ZoneHVAC:HighTemperatureRadiant
        if (CompIndex == 0) {
            RadSysNum = Util::FindItemInList(CompName, state.dataHighTempRadSys->HighTempRadSys);
            if (RadSysNum == 0) {
                ShowFatalError(state, format("SimHighTempRadiantSystem: Unit not found={}", CompName));
            }
            CompIndex = RadSysNum;
        } else {
            RadSysNum = CompIndex;
            if (RadSysNum > state.dataHighTempRadSys->NumOfHighTempRadSys || RadSysNum < 1) {
                ShowFatalError(state,
                               format("SimHighTempRadiantSystem:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      RadSysNum,
                                      state.dataHighTempRadSys->NumOfHighTempRadSys,
                                      CompName));
            }
            if (state.dataHighTempRadSys->CheckEquipName(RadSysNum)) {
                if (CompName != state.dataHighTempRadSys->HighTempRadSys(RadSysNum).Name) {
                    ShowFatalError(state,
                                   format("SimHighTempRadiantSystem: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          RadSysNum,
                                          CompName,
                                          state.dataHighTempRadSys->HighTempRadSys(RadSysNum).Name));
                }
                state.dataHighTempRadSys->CheckEquipName(RadSysNum) = false;
            }
        }

        InitHighTempRadiantSystem(state, FirstHVACIteration, RadSysNum);

        switch (state.dataHighTempRadSys->HighTempRadSys(RadSysNum).ControlType) {
        case RadControlType::MATControl:
        case RadControlType::MRTControl:
        case RadControlType::OperativeControl: {
            CalcHighTempRadiantSystem(state, RadSysNum);
        } break;
        case RadControlType::MATSPControl:
        case RadControlType::MRTSPControl:
        case RadControlType::OperativeSPControl: {
            CalcHighTempRadiantSystemSP(state, FirstHVACIteration, RadSysNum);
        } break;
        default:
            break;
        }

        UpdateHighTempRadiantSystem(state, RadSysNum, LoadMet);

        ReportHighTempRadiantSystem(state, RadSysNum);
    }

    void GetHighTempRadiantSystem(EnergyPlusData &state, bool &ErrorsFound // TRUE if errors are found on processing the input
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads the input for high temperature radiant systems
        // from the user input file.  This will contain all of the information
        // needed to simulate a high temperature radiant system.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.
        static constexpr std::string_view routineName = "GetHighTempRadiantSystem";

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MaxCombustionEffic = 1.0;                // Limit the combustion efficiency to perfection
        Real64 constexpr MaxFraction = 1.0;                       // Limit the highest allowed fraction for heat transfer parts
        Real64 constexpr MinCombustionEffic = 0.01;               // Limit the minimum combustion efficiency
        Real64 constexpr MinFraction = 0.0;                       // Limit the lowest allowed fraction for heat transfer parts
        Real64 constexpr MinThrottlingRange = 0.5;                // Smallest throttling range allowed in degrees Celsius
        int constexpr iHeatCAPMAlphaNum = 4;                      // get input index to High Temperature Radiant system heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum = 1;          // get input index to High Temperature Radiant system heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum = 2;    // index to High Temperature Radiant system heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum = 3; // index to system capacity sizing as fraction of autosized heating capacity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FracOfRadPotentiallyLost; // Difference between unity and AllFracsSummed for error reporting
        int IOStatus;                    // Used in GetObjectItem
        int NumAlphas;                   // Number of Alphas for each GetObjectItem call
        int NumNumbers;                  // Number of Numbers for each GetObjectItem call

        // Initializations and allocations
        state.dataHighTempRadSys->NumOfHighTempRadSys =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:HighTemperatureRadiant");

        state.dataHighTempRadSys->HighTempRadSys.allocate(state.dataHighTempRadSys->NumOfHighTempRadSys);
        state.dataHighTempRadSys->CheckEquipName.allocate(state.dataHighTempRadSys->NumOfHighTempRadSys);
        state.dataHighTempRadSys->HighTempRadSysNumericFields.allocate(state.dataHighTempRadSys->NumOfHighTempRadSys);
        state.dataHighTempRadSys->CheckEquipName = true;

        // extensible object, do not need max args because using IPShortCuts
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "ZoneHVAC:HighTemperatureRadiant";
        // Obtain all of the user data related to high temperature radiant systems...
        for (int Item = 1; Item <= state.dataHighTempRadSys->NumOfHighTempRadSys; ++Item) {
            auto &highTempRadSys = state.dataHighTempRadSys->HighTempRadSys(Item);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            state.dataHighTempRadSys->HighTempRadSysNumericFields(Item).FieldNames.allocate(NumNumbers);
            state.dataHighTempRadSys->HighTempRadSysNumericFields(Item).FieldNames = "";
            state.dataHighTempRadSys->HighTempRadSysNumericFields(Item).FieldNames = state.dataIPShortCut->cNumericFieldNames;
            // General user input data
            highTempRadSys.Name = state.dataIPShortCut->cAlphaArgs(1);

            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                highTempRadSys.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((highTempRadSys.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }

            highTempRadSys.ZonePtr = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataHeatBal->Zone);
            if (highTempRadSys.ZonePtr == 0) {
                ShowSevereError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            // state.dataHighTempRadSys->HighTempRadSys( Item ).MaxPowerCapac = state.dataIPShortCut->rNumericArgs( 1 );

            // Determine High Temp Radiant heating design capacity sizing method
            highTempRadSys.HeatingCapMethod = static_cast<DataSizing::DesignSizingType>(
                getEnumValue(DataSizing::DesignSizingTypeNamesUC, state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
            if (highTempRadSys.HeatingCapMethod == DataSizing::DesignSizingType::HeatingDesignCapacity) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatDesignCapacityNumericNum)) {
                    highTempRadSys.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatDesignCapacityNumericNum);
                    if (highTempRadSys.ScaledHeatingCapacity < 0.0 && highTempRadSys.ScaledHeatingCapacity != DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatDesignCapacityNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state, format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatDesignCapacityNumericNum)));
                    ErrorsFound = true;
                }
            } else if (highTempRadSys.HeatingCapMethod == DataSizing::DesignSizingType::CapacityPerFloorArea) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                    highTempRadSys.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum);
                    if (highTempRadSys.ScaledHeatingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                        ShowContinueError(state,
                                          format("Input for {} = {}",
                                                 state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                                 state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    } else if (highTempRadSys.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                        ShowContinueError(state,
                                          format("Input for {} = {}",
                                                 state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                                 state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(
                            state, format("Illegal {} = Autosize", state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state,
                        format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                    ErrorsFound = true;
                }
            } else if (highTempRadSys.HeatingCapMethod == DataSizing::DesignSizingType::FractionOfAutosizedHeatingCapacity) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                    highTempRadSys.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum);
                    if (highTempRadSys.ScaledHeatingCapacity < 0.0) {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", cCurrentModuleObject, highTempRadSys.Name));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state,
                        format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum)));
                    ErrorsFound = true;
                }
            }

            highTempRadSys.HeaterType =
                static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, state.dataIPShortCut->cAlphaArgs(5)));

            if (highTempRadSys.HeaterType == Constant::eResource::NaturalGas) {
                highTempRadSys.CombustionEffic = state.dataIPShortCut->rNumericArgs(4);
                // Limit the combustion efficiency to between zero and one...
                if (highTempRadSys.CombustionEffic < MinCombustionEffic) {
                    highTempRadSys.CombustionEffic = MinCombustionEffic;
                    ShowWarningError(
                        state,
                        format("{} was less than the allowable minimum, reset to minimum value.", state.dataIPShortCut->cNumericFieldNames(4)));
                    ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                }
                if (highTempRadSys.CombustionEffic > MaxCombustionEffic) {
                    highTempRadSys.CombustionEffic = MaxCombustionEffic;
                    ShowWarningError(
                        state,
                        format("{} was greater than the allowable maximum, reset to maximum value.", state.dataIPShortCut->cNumericFieldNames(4)));
                    ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                }
            } else {
                highTempRadSys.CombustionEffic = MaxCombustionEffic; // No inefficiency in the heater
            }

            highTempRadSys.FracRadiant = state.dataIPShortCut->rNumericArgs(5);
            if (highTempRadSys.FracRadiant < MinFraction) {
                highTempRadSys.FracRadiant = MinFraction;
                ShowWarningError(
                    state, format("{} was less than the allowable minimum, reset to minimum value.", state.dataIPShortCut->cNumericFieldNames(5)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }
            if (highTempRadSys.FracRadiant > MaxFraction) {
                highTempRadSys.FracRadiant = MaxFraction;
                ShowWarningError(
                    state, format("{} was greater than the allowable maximum, reset to maximum value.", state.dataIPShortCut->cNumericFieldNames(5)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }

            highTempRadSys.FracLatent = state.dataIPShortCut->rNumericArgs(6);
            if (highTempRadSys.FracLatent < MinFraction) {
                highTempRadSys.FracLatent = MinFraction;
                ShowWarningError(
                    state, format("{} was less than the allowable minimum, reset to minimum value.", state.dataIPShortCut->cNumericFieldNames(6)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }
            if (highTempRadSys.FracLatent > MaxFraction) {
                highTempRadSys.FracLatent = MaxFraction;
                ShowWarningError(
                    state, format("{} was greater than the allowable maximum, reset to maximum value.", state.dataIPShortCut->cNumericFieldNames(6)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }

            highTempRadSys.FracLost = state.dataIPShortCut->rNumericArgs(7);
            if (highTempRadSys.FracLost < MinFraction) {
                highTempRadSys.FracLost = MinFraction;
                ShowWarningError(
                    state, format("{} was less than the allowable minimum, reset to minimum value.", state.dataIPShortCut->cNumericFieldNames(7)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }
            if (highTempRadSys.FracLost > MaxFraction) {
                highTempRadSys.FracLost = MaxFraction;
                ShowWarningError(
                    state, format("{} was greater than the allowable maximum, reset to maximum value.", state.dataIPShortCut->cNumericFieldNames(7)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }

            // Based on the input for fractions radiant, latent, and lost, determine the fraction convective (remaining fraction)
            Real64 AllFracsSummed = highTempRadSys.FracRadiant + highTempRadSys.FracLatent + highTempRadSys.FracLost;
            if (AllFracsSummed > MaxFraction) {
                ShowSevereError(state,
                                format("Fractions radiant, latent, and lost sum up to greater than 1 for{}", state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
                highTempRadSys.FracConvect = 0.0;
            } else {
                highTempRadSys.FracConvect = 1.0 - AllFracsSummed;
            }

            // Process the temperature control type
            if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
                ShowWarningEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), "OperativeTemperature");
                highTempRadSys.ControlType = RadControlType::OperativeControl;
            } else if ((highTempRadSys.ControlType = static_cast<RadControlType>(
                            getEnumValue(radControlTypeNamesUC, state.dataIPShortCut->cAlphaArgs(6)))) == RadControlType::Invalid) {
                ShowSevereInvalidKey(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6));
                ErrorsFound = true;
            }

            highTempRadSys.ThrottlRange = state.dataIPShortCut->rNumericArgs(8);
            if (highTempRadSys.ThrottlRange < MinThrottlingRange) {
                highTempRadSys.ThrottlRange = 1.0;
                ShowWarningError(state, format("{} is below the minimum allowed.", state.dataIPShortCut->cNumericFieldNames(8)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Thus, the throttling range value has been reset to 1.0");
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
                ShowSevereEmptyField(state,
                                     eoh,
                                     state.dataIPShortCut->cAlphaFieldNames(7),
                                     state.dataIPShortCut->cAlphaFieldNames(6),
                                     state.dataIPShortCut->cAlphaArgs(6));
                ErrorsFound = true;
            } else if ((highTempRadSys.setptSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7));
                ErrorsFound = true;
            }

            highTempRadSys.FracDistribPerson = state.dataIPShortCut->rNumericArgs(9);
            if (highTempRadSys.FracDistribPerson < MinFraction) {
                highTempRadSys.FracDistribPerson = MinFraction;
                ShowWarningError(
                    state, format("{} was less than the allowable minimum, reset to minimum value.", state.dataIPShortCut->cNumericFieldNames(9)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }
            if (highTempRadSys.FracDistribPerson > MaxFraction) {
                highTempRadSys.FracDistribPerson = MaxFraction;
                ShowWarningError(
                    state, format("{} was greater than the allowable maximum, reset to maximum value.", state.dataIPShortCut->cNumericFieldNames(9)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            }

            highTempRadSys.TotSurfToDistrib = NumNumbers - 9;
            //    IF (highTempRadSys%TotSurfToDistrib > MaxDistribSurfaces) THEN
            //      CALL ShowSevereError(state, 'Trying to distribute radiant energy to too many surfaces for heater
            //      '//TRIM(state.dataIPShortCut->cAlphaArgs(1))) CALL ShowContinueError(state, 'Occurs for '//TRIM(cCurrentModuleObject)//' =
            //      '//TRIM(state.dataIPShortCut->cAlphaArgs(1))) ErrorsFound=.TRUE.
            //    END IF
            highTempRadSys.SurfaceName.allocate(highTempRadSys.TotSurfToDistrib);
            highTempRadSys.SurfacePtr.allocate(highTempRadSys.TotSurfToDistrib);
            highTempRadSys.FracDistribToSurf.allocate(highTempRadSys.TotSurfToDistrib);

            AllFracsSummed = highTempRadSys.FracDistribPerson;
            for (int SurfNum = 1; SurfNum <= highTempRadSys.TotSurfToDistrib; ++SurfNum) {
                highTempRadSys.SurfaceName(SurfNum) = state.dataIPShortCut->cAlphaArgs(SurfNum + 7);
                highTempRadSys.SurfacePtr(SurfNum) = HeatBalanceIntRadExchange::GetRadiantSystemSurface(
                    state, cCurrentModuleObject, highTempRadSys.Name, highTempRadSys.ZonePtr, highTempRadSys.SurfaceName(SurfNum), ErrorsFound);
                highTempRadSys.FracDistribToSurf(SurfNum) = state.dataIPShortCut->rNumericArgs(SurfNum + 9);
                // Error trap for fractions that are out of range
                if (highTempRadSys.FracDistribToSurf(SurfNum) < MinFraction) {
                    highTempRadSys.FracDistribToSurf(SurfNum) = MinFraction;
                    ShowWarningError(state,
                                     format("{} was less than the allowable minimum, reset to minimum value.",
                                            state.dataIPShortCut->cNumericFieldNames(SurfNum + 9)));
                    ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                }
                if (highTempRadSys.FracDistribToSurf(SurfNum) > MaxFraction) {
                    highTempRadSys.FracDistribToSurf(SurfNum) = MaxFraction;
                    ShowWarningError(state,
                                     format("{} was greater than the allowable maximum, reset to maximum value.",
                                            state.dataIPShortCut->cNumericFieldNames(SurfNum + 9)));
                    ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                }

                if (highTempRadSys.SurfacePtr(SurfNum) != 0) {
                    state.dataSurface->surfIntConv(highTempRadSys.SurfacePtr(SurfNum)).getsRadiantHeat = true;
                    state.dataSurface->allGetsRadiantHeatSurfaceList.emplace_back(highTempRadSys.SurfacePtr(SurfNum));
                }

                AllFracsSummed += highTempRadSys.FracDistribToSurf(SurfNum);

            } // ...end of DO loop through surfaces that the heater radiates to.

            // Error trap if the fractions add up to greater than 1.0
            if (AllFracsSummed > (MaxFraction + 0.01)) {
                ShowSevereError(
                    state,
                    format("Fraction of radiation distributed to surfaces sums up to greater than 1 for {}", state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            if (AllFracsSummed < (MaxFraction - 0.01)) { // User didn't distribute all of the radiation warn that some will be lost
                Real64 TotalFracToSurfs = AllFracsSummed - highTempRadSys.FracDistribPerson;
                FracOfRadPotentiallyLost = 1.0 - AllFracsSummed;
                ShowSevereError(state,
                                format("Fraction of radiation distributed to surfaces and people sums up to less than 1 for {}",
                                       state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "This would result in some of the radiant energy delivered by the high temp radiant heater being lost.");
                ShowContinueError(state, format("The sum of all radiation fractions to surfaces = {:.5T}", TotalFracToSurfs));
                ShowContinueError(state, format("The radiant fraction to people = {:.5T}", highTempRadSys.FracDistribPerson));
                ShowContinueError(state, format("So, all radiant fractions including surfaces and people = {:.5T}", AllFracsSummed));
                ShowContinueError(state,
                                  format("This means that the fraction of radiant energy that would be lost from the high temperature radiant heater "
                                         "would be = {:.5T}",
                                         FracOfRadPotentiallyLost));
                ShowContinueError(state,
                                  format("Please check and correct this so that all radiant energy is accounted for in {} = {}",
                                         cCurrentModuleObject,
                                         state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

        } // ...end of DO loop through all of the high temperature radiant heaters

        // Set up the output variables for high temperature radiant heaters
        // cCurrentModuleObject = "ZoneHVAC:HighTemperatureRadiant"
        for (int Item = 1; Item <= state.dataHighTempRadSys->NumOfHighTempRadSys; ++Item) {
            auto &highTempRadSys = state.dataHighTempRadSys->HighTempRadSys(Item);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Rate",
                                Constant::Units::W,
                                highTempRadSys.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                highTempRadSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Energy",
                                Constant::Units::J,
                                highTempRadSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                highTempRadSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            if (highTempRadSys.HeaterType == Constant::eResource::NaturalGas) {
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC NaturalGas Rate",
                                    Constant::Units::W,
                                    highTempRadSys.GasPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    highTempRadSys.Name);
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC NaturalGas Energy",
                                    Constant::Units::J,
                                    highTempRadSys.GasEnergy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    highTempRadSys.Name,
                                    Constant::eResource::NaturalGas,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Heating);
            } else if (highTempRadSys.HeaterType == Constant::eResource::Electricity) {
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC Electricity Rate",
                                    Constant::Units::W,
                                    highTempRadSys.ElecPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    highTempRadSys.Name);
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC Electricity Energy",
                                    Constant::Units::J,
                                    highTempRadSys.ElecEnergy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    highTempRadSys.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Heating);
            }
        }
    }

    void InitHighTempRadiantSystem(EnergyPlusData &state,
                                   bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                   int const RadSysNum // Index for the low temperature radiant system under consideration within the derived types
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables relating to high temperature
        // radiant heating systems.

        // METHODOLOGY EMPLOYED:
        // Simply initializes whatever needs initializing.

        // Using/Aliasing
        using DataZoneEquipment::CheckZoneEquipmentList;

        if (state.dataHighTempRadSys->firstTime) {
            state.dataHighTempRadSys->MySizeFlag.dimension(state.dataHighTempRadSys->NumOfHighTempRadSys, true);
            state.dataHighTempRadSys->firstTime = false;
        }

        // need to check all units to see if they are on Zone Equipment List or issue warning
        if (!state.dataHighTempRadSys->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataHighTempRadSys->ZoneEquipmentListChecked = true;
            for (auto &thisHTRSys : state.dataHighTempRadSys->HighTempRadSys) {
                if (CheckZoneEquipmentList(state, "ZoneHVAC:HighTemperatureRadiant", thisHTRSys.Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitHighTempRadiantSystem: Unit=[ZoneHVAC:HighTemperatureRadiant,{}] is not on any ZoneHVAC:EquipmentList.  "
                                       "It will not be simulated.",
                                       thisHTRSys.Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHighTempRadSys->MySizeFlag(RadSysNum)) {
            // for each radiant system do the sizing once.
            SizeHighTempRadiantSystem(state, RadSysNum);
            state.dataHighTempRadSys->MySizeFlag(RadSysNum) = false;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataHighTempRadSys->MyEnvrnFlag) {
            for (auto &thisHTR : state.dataHighTempRadSys->HighTempRadSys) {
                thisHTR.ZeroHTRSourceSumHATsurf = 0.0;
                thisHTR.QHTRRadSource = 0.0;
                thisHTR.QHTRRadSrcAvg = 0.0;
                thisHTR.LastQHTRRadSrc = 0.0;
                thisHTR.LastSysTimeElapsed = 0.0;
                thisHTR.LastTimeStepSys = 0.0;
            }
            state.dataHighTempRadSys->MyEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHighTempRadSys->MyEnvrnFlag = true;
        }

        if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) { // This is the first pass through in a particular time step
            auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);
            thisHTR.ZeroHTRSourceSumHATsurf =
                state.dataHeatBal->Zone(thisHTR.ZonePtr).sumHATsurf(state); // Set this to figure out what part of the load the radiant system meets
            thisHTR.QHTRRadSource = 0.0;                                    // Initialize this variable to zero (radiant system defaults to off)
            thisHTR.QHTRRadSrcAvg = 0.0;                                    // Initialize this variable to zero (radiant system defaults to off)
            thisHTR.LastQHTRRadSrc = 0.0;     // At the beginning of a time step, reset to zero so average calculation can start again
            thisHTR.LastSysTimeElapsed = 0.0; // At the beginning of a time step, reset to zero so average calculation can start again
            thisHTR.LastTimeStepSys = 0.0;    // At the beginning of a time step, reset to zero so average calculation can start again
        }
    }

    void SizeHighTempRadiantSystem(EnergyPlusData &state, int const RadSysNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing high temperature radiant components for which max power input has not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains design heating load from the zone sizing arrays

        // Using/Aliasing
        auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS
        Real64 TempSize; // autosized value of coil input field
        state.dataSize->DataScalableCapSizingON = false;

        int const curZoneEqNum = state.dataSize->CurZoneEqNum;

        if (curZoneEqNum > 0) {
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(curZoneEqNum);

            state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
            state.dataSize->DataZoneNumber = thisHTR.ZonePtr;
            // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingCapacitySizing, etc.)
            int SizingMethod = HVAC::HeatingCapacitySizing;
            int FieldNum = 1;
            std::string const SizingString = format("{} [W]", state.dataHighTempRadSys->HighTempRadSysNumericFields(RadSysNum).FieldNames(FieldNum));
            // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
            // FractionOfAutosizedHeatingCapacity )
            int CapSizingMethod = static_cast<int>(thisHTR.HeatingCapMethod);
            zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                std::string_view const CompType = "ZoneHVAC:HighTemperatureRadiant";
                std::string_view const CompName = thisHTR.Name;

                if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                    if (thisHTR.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        CheckZoneSizing(state, CompType, CompName);
                        zoneEqSizing.DesHeatingLoad =
                            state.dataSize->FinalZoneSizing(curZoneEqNum).NonAirSysDesHeatLoad / (thisHTR.FracRadiant + thisHTR.FracConvect);
                    } else {
                        zoneEqSizing.DesHeatingLoad = thisHTR.ScaledHeatingCapacity;
                    }
                    zoneEqSizing.HeatingCapacity = true;
                    TempSize = zoneEqSizing.DesHeatingLoad;
                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                    zoneEqSizing.HeatingCapacity = true;
                    zoneEqSizing.DesHeatingLoad = thisHTR.ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                    TempSize = zoneEqSizing.DesHeatingLoad;
                    state.dataSize->DataScalableCapSizingON = true;
                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                    CheckZoneSizing(state, CompType, CompName);
                    zoneEqSizing.HeatingCapacity = true;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = thisHTR.ScaledHeatingCapacity;
                    zoneEqSizing.DesHeatingLoad =
                        state.dataSize->FinalZoneSizing(curZoneEqNum).NonAirSysDesHeatLoad / (thisHTR.FracRadiant + thisHTR.FracConvect);
                    TempSize = DataSizing::AutoSize;
                    state.dataSize->DataScalableCapSizingON = true;
                } else {
                    TempSize = thisHTR.ScaledHeatingCapacity;
                }
                bool PrintFlag = true;
                bool errorsFound = false;
                constexpr std::string_view RoutineName = "SizeHighTempRadiantSystem";
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                thisHTR.MaxPowerCapac = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataScalableCapSizingON = false;
            }
        }
    }

    void CalcHighTempRadiantSystem(EnergyPlusData &state, int const RadSysNum) // name of the low temperature radiant system
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a high temperature radiant heating system.

        // METHODOLOGY EMPLOYED:
        // Follows the methods used by many other pieces of zone equipment except
        // that we are controlling the input to the heater element.  Note that
        // cooling is not allowed for such a system.  Controls are very basic at
        // this point using just a linear interpolation between being off at
        // one end of the throttling range, fully on at the other end, and varying
        // linearly in between.

        // REFERENCES:
        // Other EnergyPlus modules
        // Building Systems Laboratory, BLAST User's Guide/Reference.
        // Fanger, P.O. "Analysis and Applications in Environmental Engineering",
        //   Danish Technical Press, 1970.
        // Maloney, Dan. 1987. "Development of a radiant heater model and the
        //   incorporation of thermal comfort considerations into the BLAST
        //   energy analysis program", M.S. thesis, University of Illinois at
        //   Urbana-Champaign (Dept. of Mechanical and Industrial Engineering).

        // Using/Aliasing
        auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);

        // initialize local variables
        int ZoneNum = thisHTR.ZonePtr;
        Real64 HeatFrac = 0.0; // fraction of maximum energy input to radiant system [dimensionless]

        if (thisHTR.availSched->getCurrentVal() <= 0) {

            // Unit is off or has no load upon it; set the flow rates to zero and then
            // simulate the components with the no flow conditions
            thisHTR.QHTRRadSource = 0.0;

        } else { // Unit might be on-->this section is intended to control the output of the
            // high temperature radiant heater (temperature controlled)

            // Determine the current setpoint temperature and the temperature at which the unit should be completely off
            Real64 SetPtTemp = thisHTR.setptSched->getCurrentVal();
            Real64 OffTemp = SetPtTemp + 0.5 * thisHTR.ThrottlRange;
            auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
            Real64 OpTemp = (thisZoneHB.MAT + thisZoneHB.MRT) / 2.0; // Approximate the "operative" temperature

            // Determine the fraction of maximum power to the unit (limiting the fraction range from zero to unity)
            switch (thisHTR.ControlType) {
            case RadControlType::MATControl: {
                HeatFrac = (OffTemp - thisZoneHB.MAT) / thisHTR.ThrottlRange;
            } break;
            case RadControlType::MRTControl: {
                HeatFrac = (OffTemp - thisZoneHB.MRT) / thisHTR.ThrottlRange;
            } break;
            case RadControlType::OperativeControl: {
                OpTemp = 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
                HeatFrac = (OffTemp - OpTemp) / thisHTR.ThrottlRange;
            } break;
            default:
                break;
            }
            if (HeatFrac < 0.0) {
                HeatFrac = 0.0;
            }
            if (HeatFrac > 1.0) {
                HeatFrac = 1.0;
            }

            // Set the heat source for the high temperature electric radiant system
            thisHTR.QHTRRadSource = HeatFrac * thisHTR.MaxPowerCapac;
        }
    }

    void CalcHighTempRadiantSystemSP(
        EnergyPlusData &state,
        [[maybe_unused]] bool const FirstHVACIteration, // true if this is the first HVAC iteration at this system time step !unused1208
        int const RadSysNum                             // name of the low temperature radiant system
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2008
        //       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a high temperature radiant heating system using setpoint temperature control.

        // METHODOLOGY EMPLOYED:
        // Follows the methods used by many other pieces of zone equipment except
        // that we are controlling the input to the heater element.  Note that
        // cooling is not allowed for such a system.  Controls are very basic and
        // use an iterative approach to get close to what we need.

        // REFERENCES:
        // Other EnergyPlus modules
        // Building Systems Laboratory, BLAST User's Guide/Reference.
        // Fanger, P.O. "Analysis and Applications in Environmental Engineering",
        //   Danish Technical Press, 1970.
        // Maloney, Dan. 1987. "Development of a radiant heater model and the
        //   incorporation of thermal comfort considerations into the BLAST
        //   energy analysis program", M.S. thesis, University of Illinois at
        //   Urbana-Champaign (Dept. of Mechanical and Industrial Engineering).

        // Using/Aliasing
        auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);

        // SUBROUTINE PARAMETER DEFINITIONS:
        float const TempConvToler(0.1f); // Temperature controller tries to converge to within 0.1C
        int constexpr MaxIterations(10); // Maximum number of iterations to achieve temperature control
        // (10 interval halvings achieves control to 0.1% of capacity)
        // These two parameters are intended to achieve reasonable control
        // without excessive run times.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneTemp(0.0); // zone temperature (MAT, MRT, or Operative Temperature, depending on control type) [C]

        // initialize local variables
        int ZoneNum = thisHTR.ZonePtr;
        thisHTR.QHTRRadSource = 0.0;

        if (thisHTR.availSched->getCurrentVal() > 0) {

            // Unit is scheduled on-->this section is intended to control the output of the
            // high temperature radiant heater (temperature controlled)

            // Determine the current setpoint temperature and the temperature at which the unit should be completely off
            Real64 SetPtTemp = thisHTR.setptSched->getCurrentVal();

            // Now, distribute the radiant energy of all systems to the appropriate
            // surfaces, to people, and the air; determine the latent portion
            DistributeHTRadGains(state);

            // Now "simulate" the system by recalculating the heat balances
            HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
            HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

            // First determine whether or not the unit should be on
            // Determine the proper temperature on which to control
            auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
            switch (thisHTR.ControlType) {
            case RadControlType::MATSPControl: {
                ZoneTemp = thisZoneHB.MAT;
            } break;
            case RadControlType::MRTSPControl: {
                ZoneTemp = thisZoneHB.MRT;
            } break;
            case RadControlType::OperativeSPControl: {
                ZoneTemp = 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
            } break;
            default: {
                assert(false);
            } break;
            }

            if (ZoneTemp < (SetPtTemp - TempConvToler)) {

                // Use simple interval halving to find the best operating fraction to achieve proper temperature control
                int IterNum = 0;
                bool ConvergFlag = false;
                float HeatFrac; // fraction of maximum energy input to radiant system [dimensionless]
                float HeatFracMax = 1.0;
                float HeatFracMin = 0.0;

                while ((IterNum <= MaxIterations) && (!ConvergFlag)) {

                    // In the first iteration (IterNum=0), try full capacity and see if that is the best solution
                    if (IterNum == 0) {
                        HeatFrac = 1.0;
                    } else {
                        HeatFrac = (HeatFracMin + HeatFracMax) / 2.0;
                    }

                    // Set the heat source for the high temperature radiant system
                    thisHTR.QHTRRadSource = HeatFrac * thisHTR.MaxPowerCapac;

                    // Now, distribute the radiant energy of all systems to the appropriate
                    // surfaces, to people, and the air; determine the latent portion
                    DistributeHTRadGains(state);

                    // Now "simulate" the system by recalculating the heat balances
                    HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                    HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

                    // Redetermine the current value of the controlling temperature
                    auto const &thisZoneHBMod = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
                    switch (thisHTR.ControlType) {
                    case RadControlType::MATControl: {
                        ZoneTemp = thisZoneHBMod.MAT;
                    } break;
                    case RadControlType::MRTControl: {
                        ZoneTemp = thisZoneHBMod.MRT;
                    } break;
                    case RadControlType::OperativeControl: {
                        ZoneTemp = 0.5 * (thisZoneHBMod.MAT + thisZoneHBMod.MRT);
                    } break;
                    default:
                        break;
                    }

                    if ((std::abs(ZoneTemp - SetPtTemp)) <= TempConvToler) {
                        // The radiant heater has controlled the zone temperature to the appropriate level--stop iterating
                        ConvergFlag = true;
                    } else if (ZoneTemp < SetPtTemp) {
                        // The zone temperature is too low--try increasing the radiant heater output
                        if (IterNum == 0) {
                            // Heater already at capacity--this is the best that we can do
                            ConvergFlag = true;
                        } else {
                            HeatFracMin = HeatFrac;
                        }
                    } else { // (ZoneTemp > SetPtTemp)
                        // The zone temperature is too high--try decreasing the radiant heater output
                        if (IterNum > 0) {
                            HeatFracMax = HeatFrac;
                        }
                    }

                    ++IterNum;
                }
            }
        }
    }

    void UpdateHighTempRadiantSystem(EnergyPlusData &state,
                                     int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                     Real64 &LoadMet      // load met by the radiant system, in Watts
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does any updating that needs to be done for high
        // temperature radiant heating systems.  This routine has two functions.
        // First, it needs to keep track of the average high temperature
        // radiant source.  The method for doing this is similar to low
        // temperature systems except that heat input is kept locally on
        // a system basis rather than a surface basis.  This is because a high
        // temperature system affects many surfaces while a low temperature
        // system directly affects only one surface.  This leads to the second
        // function of this subroutine which is to account for the affect of
        // all high temperature radiant systems on each surface.  This
        // distribution must be "redone" every time to be sure that we have
        // properly accounted for all of the systems.

        // METHODOLOGY EMPLOYED:
        // For the source average update, if the system time step elapsed is
        // still what it used to be, then either we are still iterating or we
        // had to go back and shorten the time step.  As a result, we have to
        // subtract out the previous value that we added.  If the system time
        // step elapsed is different, then we just need to add the new values
        // to the running average.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneNum; // Zone index number for the current radiant system
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);

        // First, update the running average if necessary...
        if (thisHTR.LastSysTimeElapsed == SysTimeElapsed) {
            // Still iterating or reducing system time step, so subtract old values which were
            // not valid
            thisHTR.QHTRRadSrcAvg -= thisHTR.LastQHTRRadSrc * thisHTR.LastTimeStepSys / state.dataGlobal->TimeStepZone;
        }

        // Update the running average and the "last" values with the current values of the appropriate variables
        thisHTR.QHTRRadSrcAvg += thisHTR.QHTRRadSource * TimeStepSys / state.dataGlobal->TimeStepZone;

        thisHTR.LastQHTRRadSrc = thisHTR.QHTRRadSource;
        thisHTR.LastSysTimeElapsed = SysTimeElapsed;
        thisHTR.LastTimeStepSys = TimeStepSys;

        switch (thisHTR.ControlType) {
        case RadControlType::MATControl:
        case RadControlType::MRTControl:
        case RadControlType::OperativeControl: {
            // Only need to do this for the non-SP controls (SP has already done this enough)
            // Now, distribute the radiant energy of all systems to the appropriate
            // surfaces, to people, and the air; determine the latent portion
            DistributeHTRadGains(state);

            // Now "simulate" the system by recalculating the heat balances
            ZoneNum = thisHTR.ZonePtr;
            HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
            HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);
        } break;
        default:
            break;
        }

        if (thisHTR.QHTRRadSource <= 0.0) {
            LoadMet = 0.0; // System wasn't running so it can't meet a load
        } else {
            ZoneNum = thisHTR.ZonePtr;
            LoadMet = (state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - thisHTR.ZeroHTRSourceSumHATsurf) +
                      state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum);
        }
    }

    void UpdateHTRadSourceValAvg(EnergyPlusData &state, bool &HighTempRadSysOn) // .TRUE. if the radiant system has run this zone time step
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // To transfer the average value of the heat source over the entire
        // zone time step back to the heat balance routines so that the heat
        // balance algorithms can simulate one last time with the average source
        // to maintain some reasonable amount of continuity and energy balance
        // in the temperature and flux histories.

        // METHODOLOGY EMPLOYED:
        // All of the record keeping for the average term is done in the Update
        // routine so the only other thing that this subroutine does is check to
        // see if the system was even on.  If any average term is non-zero, then
        // one or more of the radiant systems was running.

        HighTempRadSysOn = false;

        // If this was never allocated, then there are no radiant systems in this input file (just RETURN)
        if (state.dataHighTempRadSys->NumOfHighTempRadSys == 0) {
            return;
        }

        // If it was allocated, then we have to check to see if this was running at all...
        for (auto &thisHTR : state.dataHighTempRadSys->HighTempRadSys) {
            thisHTR.QHTRRadSource = thisHTR.QHTRRadSrcAvg;
            if (thisHTR.QHTRRadSrcAvg != 0.0) {
                HighTempRadSysOn = true;
            }
        }

        DistributeHTRadGains(
            state); // state.dataHighTempRadSys->HighTempRadSys(RadSysNum).QHTRRadSource has been modified so we need to redistribute gains
    }

    void DistributeHTRadGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001
        //       MODIFIED       April 2010 Brent Griffith, max limit to protect surface temperature calcs

        // PURPOSE OF THIS SUBROUTINE:
        // To distribute the gains from the high temperature radiant heater
        // as specified in the user input file.  This includes distribution
        // of long wavelength radiant gains to surfaces and "people" as well
        // as latent, lost, and convective portions of the total gain.

        // METHODOLOGY EMPLOYED:
        // We must cycle through all of the radiant systems because each
        // surface could feel the effect of more than one radiant system.
        // Note that the energy radiated to people is assumed to affect them
        // but them it is assumed to be convected to the air.  This is why
        // the convective portion shown below has two parts to it.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr SmallestArea = 0.001; // Smallest area in meters squared (to avoid a divide by zero)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ThisSurfIntensity; // temporary for W/m2 term for rad on a surface

        // Using/Aliasing
        auto &dataHBFS = state.dataHeatBalFanSys;

        // Initialize arrays
        dataHBFS->SumConvHTRadSys = 0.0;
        dataHBFS->SumLatentHTRadSys = 0.0;
        for (auto &thisHTR : state.dataHighTempRadSys->HighTempRadSys) {
            for (int radSurfNum = 1; radSurfNum <= thisHTR.TotSurfToDistrib; ++radSurfNum) {
                int surfNum = thisHTR.SurfacePtr(radSurfNum);
                state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum).HTRadSys = 0.0;
            }
        }
        dataHBFS->ZoneQHTRadSysToPerson = 0.0;

        for (auto &thisHTR : state.dataHighTempRadSys->HighTempRadSys) {
            int ZoneNum = thisHTR.ZonePtr;

            dataHBFS->ZoneQHTRadSysToPerson(ZoneNum) = thisHTR.QHTRRadSource * thisHTR.FracRadiant * thisHTR.FracDistribPerson;
            dataHBFS->SumConvHTRadSys(ZoneNum) += thisHTR.QHTRRadSource * thisHTR.FracConvect;
            dataHBFS->SumLatentHTRadSys(ZoneNum) += thisHTR.QHTRRadSource * thisHTR.FracLatent;

            for (int RadSurfNum = 1; RadSurfNum <= thisHTR.TotSurfToDistrib; ++RadSurfNum) {
                int SurfNum = thisHTR.SurfacePtr(RadSurfNum);
                if (state.dataSurface->Surface(SurfNum).Area > SmallestArea) {
                    ThisSurfIntensity = (thisHTR.QHTRRadSource * thisHTR.FracRadiant * thisHTR.FracDistribToSurf(RadSurfNum) /
                                         state.dataSurface->Surface(SurfNum).Area);
                    state.dataHeatBalFanSys->surfQRadFromHVAC(SurfNum).HTRadSys += ThisSurfIntensity;

                    if (ThisSurfIntensity > DataHeatBalFanSys::MaxRadHeatFlux) { // CR 8074, trap excessive intensity (throws off surface balance )
                        ShowSevereError(state, "DistributeHTRadGains:  excessive thermal radiation heat flux intensity detected");
                        ShowContinueError(state, format("Surface = {}", state.dataSurface->Surface(SurfNum).Name));
                        ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                        ShowContinueError(state, format("Occurs in ZoneHVAC:HighTemperatureRadiant = {}", thisHTR.Name));
                        ShowContinueError(state, format("Radiation intensity = {:.2R} [W/m2]", ThisSurfIntensity));
                        ShowContinueError(state, "Assign a larger surface area or more surfaces in ZoneHVAC:HighTemperatureRadiant");
                        ShowFatalError(state, "DistributeHTRadGains:  excessive thermal radiation heat flux intensity detected");
                    }
                } else { // small surface
                    ShowSevereError(state, "DistributeHTRadGains:  surface not large enough to receive thermal radiation heat flux");
                    ShowContinueError(state, format("Surface = {}", state.dataSurface->Surface(SurfNum).Name));
                    ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                    ShowContinueError(state, format("Occurs in ZoneHVAC:HighTemperatureRadiant = {}", thisHTR.Name));
                    ShowContinueError(state, "Assign a larger surface area or more surfaces in ZoneHVAC:HighTemperatureRadiant");
                    ShowFatalError(state, "DistributeHTRadGains:  surface not large enough to receive thermal radiation heat flux");
                }
            }
        }

        // Here an assumption is made regarding radiant heat transfer to people.
        // While the ZoneQHTRadSysToPerson array will be used by the thermal comfort
        // routines, the energy transfer to people would get lost from the perspective
        // of the heat balance.  So, to avoid this net loss of energy which clearly
        // gets added to the zones, we must account for it somehow.  This assumption
        // that all energy radiated to people is converted to convective energy is
        // not very precise, but at least it conserves energy.
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            dataHBFS->SumConvHTRadSys(ZoneNum) += dataHBFS->ZoneQHTRadSysToPerson(ZoneNum);
        }
    }

    void ReportHighTempRadiantSystem(EnergyPlusData &state,
                                     int const RadSysNum) // Index for the low temperature radiant system under consideration within the derived types
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simply produces output for the high temperature radiant system.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &thisHTR = state.dataHighTempRadSys->HighTempRadSys(RadSysNum);

        if (thisHTR.HeaterType == Constant::eResource::NaturalGas) {
            thisHTR.GasPower = thisHTR.QHTRRadSource / thisHTR.CombustionEffic;
            thisHTR.GasEnergy = thisHTR.GasPower * TimeStepSysSec;
            thisHTR.ElecPower = 0.0;
            thisHTR.ElecEnergy = 0.0;
        } else if (thisHTR.HeaterType == Constant::eResource::Electricity) {
            thisHTR.GasPower = 0.0;
            thisHTR.GasEnergy = 0.0;
            thisHTR.ElecPower = thisHTR.QHTRRadSource;
            thisHTR.ElecEnergy = thisHTR.ElecPower * TimeStepSysSec;
        } else {
            ShowWarningError(state, "Someone forgot to add a high temperature radiant heater type to the reporting subroutine");
        }
        thisHTR.HeatPower = thisHTR.QHTRRadSource;
        thisHTR.HeatEnergy = thisHTR.HeatPower * TimeStepSysSec;
    }

} // namespace HighTempRadiantSystem

} // namespace EnergyPlus
