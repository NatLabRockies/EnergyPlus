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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace ElectricBaseboardRadiator {

    // Module ElectricBaseboardRadiator -- (ref: Object: ZoneHVAC:Baseboard:RadiantConvective:Electric)

    // MODULE INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   Feb 2010

    // PURPOSE OF THIS MODULE:
    // This module is to calculate the actual convective heat addition that an electrical baseboard heater
    // delivers to a space.

    // METHODOLOGY EMPLOYED:
    // Based on the convective-only electric baseboard module (Object: ZoneHVAC:Baseboard:Convective:Electric)
    // written by Richard Liesen in Nov 2001, this new electric baseboard module is to add the existing calculation
    // algorithm of radiant heat transfer in the high temperature radiant system module.

    // REFERENCES:
    // HighTempRadiantSystem module (ZoneHVAC:HighTemperatureRadiant)
    // Convective electric baseboard module (ZoneHVAC:Baseboard:Convective:Electric)

    void SimElecBaseboard(EnergyPlusData &state,
                          std::string const &EquipName,
                          int const ControlledZoneNum,
                          bool const FirstHVACIteration,
                          Real64 &PowerMet,
                          int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       Feb 2010 Daeho Kang for radiant component

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the Electric Baseboard units.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int BaseboardNum; // Index of unit in baseboard array
        int NumElecBaseboards = state.dataElectBaseboardRad->NumElecBaseboards;

        if (state.dataElectBaseboardRad->GetInputFlag) {
            GetElectricBaseboardInput(state);
            state.dataElectBaseboardRad->GetInputFlag = false;
        }

        // Find the correct Baseboard Equipment
        if (CompIndex == 0) {
            BaseboardNum = Util::FindItemInList(EquipName, state.dataElectBaseboardRad->ElecBaseboard, &ElecBaseboardParams::EquipName);
            if (BaseboardNum == 0) {
                ShowFatalError(state, "SimElectricBaseboard: Unit not found=" + EquipName);
            }
            CompIndex = BaseboardNum;
        } else {
            BaseboardNum = CompIndex;
            if (BaseboardNum > NumElecBaseboards || BaseboardNum < 1) {
                ShowFatalError(state,
                               format("SimElectricBaseboard:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      BaseboardNum,
                                      NumElecBaseboards,
                                      EquipName));
            }
            if (state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum).CheckEquipName) {
                if (EquipName != state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum).EquipName) {
                    ShowFatalError(state,
                                   format("SimElectricBaseboard: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          BaseboardNum,
                                          EquipName,
                                          state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum).EquipName));
                }
                state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum).CheckEquipName = false;
            }
        }

        InitElectricBaseboard(state, BaseboardNum, ControlledZoneNum, FirstHVACIteration);
        CalcElectricBaseboard(state, BaseboardNum, ControlledZoneNum);

        PowerMet = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum).TotPower;

        UpdateElectricBaseboard(state, BaseboardNum);
        ReportElectricBaseboard(state, BaseboardNum);
    }

    void GetElectricBaseboardInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       Feb 2010 Daeho Kang for radiant component

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the Baseboard units.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetElectricBaseboardInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetElectricBaseboardInput";  // include trailing blank space

        Real64 constexpr MaxFraction(1.0); // Maximum limit of fractional values
        Real64 constexpr MinFraction(0.0); // Minimum limit of fractional values
        //    INTEGER,PARAMETER :: MaxDistribSurfaces   = 20      ! Maximum number of surfaces that a baseboard heater can radiate to
        int constexpr MinDistribSurfaces(1);                  // Minimum number of surfaces that a baseboard heater can radiate to
        int constexpr iHeatCAPMAlphaNum(3);                   // get input index to HW baseboard heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum(1);       // get input index to HW baseboard heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum(2); // get input index to HW baseboard heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum(
            3); // get input index to HW baseboard heating capacity sizing as fraction of autosized heating capacity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNumbers;
        int IOStat;
        bool ErrorsFound(false); // If errors detected in input

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = state.dataElectBaseboardRad->cCMO_BBRadiator_Electric;

        // Update Num in state and make local convenience copy
        int NumElecBaseboards = state.dataElectBaseboardRad->NumElecBaseboards =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        // object is extensible, no max args needed as IPShortCuts being used
        auto &ElecBaseboardNumericFields = state.dataElectBaseboardRad->ElecBaseboardNumericFields;

        state.dataElectBaseboardRad->ElecBaseboard.allocate(NumElecBaseboards);
        ElecBaseboardNumericFields.allocate(NumElecBaseboards);

        for (int BaseboardNum = 1; BaseboardNum <= NumElecBaseboards; ++BaseboardNum) {
            auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     BaseboardNum,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            ElecBaseboardNumericFields(BaseboardNum).FieldNames.allocate(NumNumbers);
            ElecBaseboardNumericFields(BaseboardNum).FieldNames = "";
            ElecBaseboardNumericFields(BaseboardNum).FieldNames = s_ipsc->cNumericFieldNames;

            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueBaseboardName(
                state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject + " Name");

            elecBaseboard.EquipName = s_ipsc->cAlphaArgs(1); // name of this baseboard
            elecBaseboard.Schedule = s_ipsc->cAlphaArgs(2);
            if (s_ipsc->lAlphaFieldBlanks(2)) {
                elecBaseboard.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((elecBaseboard.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            // Determine HW radiant baseboard heating design capacity sizing method
            if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "HeatingDesignCapacity")) {
                elecBaseboard.HeatingCapMethod = DataSizing::HeatingDesignCapacity;

                if (!s_ipsc->lNumericFieldBlanks(iHeatDesignCapacityNumericNum)) {
                    elecBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum);
                    if (elecBaseboard.ScaledHeatingCapacity < 0.0 && elecBaseboard.ScaledHeatingCapacity != DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", s_ipsc->cCurrentModuleObject, elecBaseboard.EquipName));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum),
                                                 s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", s_ipsc->cCurrentModuleObject, elecBaseboard.EquipName));
                    ShowContinueError(state,
                                      "Input for " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                    ShowContinueError(state, "Blank field not allowed for " + s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "CapacityPerFloorArea")) {
                elecBaseboard.HeatingCapMethod = DataSizing::CapacityPerFloorArea;
                if (!s_ipsc->lNumericFieldBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                    elecBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum);
                    if (elecBaseboard.ScaledHeatingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", s_ipsc->cCurrentModuleObject, elecBaseboard.EquipName));
                        ShowContinueError(state,
                                          "Input for " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum),
                                                 s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    } else if (elecBaseboard.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", s_ipsc->cCurrentModuleObject, elecBaseboard.EquipName));
                        ShowContinueError(state,
                                          "Input for " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                        ShowContinueError(state, "Illegal " + s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum) + " = Autosize");
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format(s_ipsc->cCurrentModuleObject, elecBaseboard.EquipName));
                    ShowContinueError(state,
                                      "Input for " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                    ShowContinueError(state, "Blank field not allowed for " + s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "FractionOfAutosizedHeatingCapacity")) {
                elecBaseboard.HeatingCapMethod = DataSizing::FractionOfAutosizedHeatingCapacity;
                if (!s_ipsc->lNumericFieldBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                    elecBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum);
                    if (elecBaseboard.ScaledHeatingCapacity < 0.0) {
                        ShowSevereError(state, s_ipsc->cCurrentModuleObject + " = " + elecBaseboard.EquipName);
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum),
                                                 s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, s_ipsc->cCurrentModuleObject + " = " + elecBaseboard.EquipName);
                    ShowContinueError(state,
                                      "Input for " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                    ShowContinueError(state, "Blank field not allowed for " + s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, s_ipsc->cCurrentModuleObject + " = " + elecBaseboard.EquipName);
                ShowContinueError(state, "Illegal " + s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum) + " = " + s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum));
                ErrorsFound = true;
            }

            elecBaseboard.BaseboardEfficiency = s_ipsc->rNumericArgs(4);
            elecBaseboard.FracRadiant = s_ipsc->rNumericArgs(5);
            if (elecBaseboard.FracRadiant < MinFraction) {
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                     s_ipsc->cNumericFieldNames(5) + " was lower than the allowable minimum.");
                ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinFraction));
                elecBaseboard.FracRadiant = MinFraction;
            }
            if (elecBaseboard.FracRadiant > MaxFraction) {
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                     s_ipsc->cNumericFieldNames(5) + " was higher than the allowable maximum.");
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                elecBaseboard.FracRadiant = MaxFraction;
            }

            // Remaining fraction is added to the zone as convective heat transfer
            if (elecBaseboard.FracRadiant > MaxFraction) {
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) +
                                     "\", Fraction Radiant was higher than the allowable maximum.");
                elecBaseboard.FracRadiant = MaxFraction;
                elecBaseboard.FracConvect = 0.0;
            } else {
                elecBaseboard.FracConvect = 1.0 - elecBaseboard.FracRadiant;
            }

            elecBaseboard.FracDistribPerson = s_ipsc->rNumericArgs(6);
            if (elecBaseboard.FracDistribPerson < MinFraction) {
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                     s_ipsc->cNumericFieldNames(6) + " was lower than the allowable minimum.");
                ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinFraction));
                elecBaseboard.FracDistribPerson = MinFraction;
            }
            if (elecBaseboard.FracDistribPerson > MaxFraction) {
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                     s_ipsc->cNumericFieldNames(6) + " was higher than the allowable maximum.");
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                elecBaseboard.FracDistribPerson = MaxFraction;
            }

            elecBaseboard.TotSurfToDistrib = NumNumbers - 6;

            if ((elecBaseboard.TotSurfToDistrib < MinDistribSurfaces) && (elecBaseboard.FracRadiant > MinFraction)) {
                ShowSevereError(state,
                                std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) +
                                    "\", the number of surface/radiant fraction groups entered was less than the allowable minimum.");
                ShowContinueError(state, format("...the minimum that must be entered=[{}].", MinDistribSurfaces));
                ErrorsFound = true;
                elecBaseboard.TotSurfToDistrib = 0; // error
            }

            elecBaseboard.SurfaceName.allocate(elecBaseboard.TotSurfToDistrib);
            elecBaseboard.SurfaceName = "";
            elecBaseboard.SurfacePtr.allocate(elecBaseboard.TotSurfToDistrib);
            elecBaseboard.SurfacePtr = 0;
            elecBaseboard.FracDistribToSurf.allocate(elecBaseboard.TotSurfToDistrib);
            elecBaseboard.FracDistribToSurf = 0.0;

            elecBaseboard.ZonePtr =
                DataZoneEquipment::GetZoneEquipControlledZoneNum(state, DataZoneEquipment::ZoneEquipType::BaseboardElectric, elecBaseboard.EquipName);

            Real64 AllFracsSummed = elecBaseboard.FracDistribPerson;
            for (int SurfNum = 1; SurfNum <= elecBaseboard.TotSurfToDistrib; ++SurfNum) {
                elecBaseboard.SurfaceName(SurfNum) = s_ipsc->cAlphaArgs(SurfNum + 3);
                elecBaseboard.SurfacePtr(SurfNum) = HeatBalanceIntRadExchange::GetRadiantSystemSurface(state,
                                                                                                       s_ipsc->cCurrentModuleObject,
                                                                                                       elecBaseboard.EquipName,
                                                                                                       elecBaseboard.ZonePtr,
                                                                                                       elecBaseboard.SurfaceName(SurfNum),
                                                                                                       ErrorsFound);
                elecBaseboard.FracDistribToSurf(SurfNum) = s_ipsc->rNumericArgs(SurfNum + 6);
                if (elecBaseboard.FracDistribToSurf(SurfNum) > MaxFraction) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                         s_ipsc->cNumericFieldNames(SurfNum + 6) + "was greater than the allowable maximum.");
                    ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                    elecBaseboard.TotSurfToDistrib = MaxFraction;
                }
                if (elecBaseboard.FracDistribToSurf(SurfNum) < MinFraction) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) + "\", " +
                                         s_ipsc->cNumericFieldNames(SurfNum + 6) + "was less than the allowable minimum.");
                    ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MinFraction));
                    elecBaseboard.TotSurfToDistrib = MinFraction;
                }
                if (elecBaseboard.SurfacePtr(SurfNum) != 0) {
                    state.dataSurface->surfIntConv(elecBaseboard.SurfacePtr(SurfNum)).getsRadiantHeat = true;
                    state.dataSurface->allGetsRadiantHeatSurfaceList.emplace_back(elecBaseboard.SurfacePtr(SurfNum));
                }

                AllFracsSummed += elecBaseboard.FracDistribToSurf(SurfNum);
            } // Surfaces

            if (AllFracsSummed > (MaxFraction + 0.01)) {
                ShowSevereError(state,
                                std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) +
                                    "\", Summed radiant fractions for people + surface groups > 1.0");
                ErrorsFound = true;
            }
            if ((AllFracsSummed < (MaxFraction - 0.01)) &&
                (elecBaseboard.FracRadiant > MinFraction)) { // User didn't distribute all of the | radiation warn that some will be lost
                ShowWarningError(state,
                                 std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "=\"" + s_ipsc->cAlphaArgs(1) +
                                     "\", Summed radiant fractions for people + surface groups < 1.0");
                ShowContinueError(state, "The rest of the radiant energy delivered by the baseboard heater will be lost");
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, std::string{RoutineName} + s_ipsc->cCurrentModuleObject + "Errors found getting input. Program terminates.");
        }

        for (auto &elecBaseboard : state.dataElectBaseboardRad->ElecBaseboard) {
            // Setup Report variables for the Electric Baseboards
            // CurrentModuleObject='ZoneHVAC:Baseboard:RadiantConvective:Electric'
            SetupOutputVariable(state,
                                "Baseboard Total Heating Rate",
                                Constant::Units::W,
                                elecBaseboard.TotPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                elecBaseboard.EquipName);

            SetupOutputVariable(state,
                                "Baseboard Convective Heating Rate",
                                Constant::Units::W,
                                elecBaseboard.ConvPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                elecBaseboard.EquipName);
            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Rate",
                                Constant::Units::W,
                                elecBaseboard.RadPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                elecBaseboard.EquipName);

            SetupOutputVariable(state,
                                "Baseboard Electricity Energy",
                                Constant::Units::J,
                                elecBaseboard.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                elecBaseboard.EquipName,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Baseboard Electricity Rate",
                                Constant::Units::W,
                                elecBaseboard.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                elecBaseboard.EquipName);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Energy",
                                Constant::Units::J,
                                elecBaseboard.TotEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                elecBaseboard.EquipName,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);

            SetupOutputVariable(state,
                                "Baseboard Convective Heating Energy",
                                Constant::Units::J,
                                elecBaseboard.ConvEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                elecBaseboard.EquipName);
            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Energy",
                                Constant::Units::J,
                                elecBaseboard.RadEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                elecBaseboard.EquipName);
        }
    }

    void InitElectricBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       Feb 2010 Daeho Kang for radiant component

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the Baseboard units during simulation.

        auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);

        if (!state.dataGlobal->SysSizingCalc && elecBaseboard.MySizeFlag) {
            // for each coil, do the sizing once.
            SizeElectricBaseboard(state, BaseboardNum);
            elecBaseboard.MySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && elecBaseboard.MyEnvrnFlag) {
            // Initialize
            elecBaseboard.ZeroBBSourceSumHATsurf = 0.0;
            elecBaseboard.QBBElecRadSource = 0.0;
            elecBaseboard.QBBElecRadSrcAvg = 0.0;
            elecBaseboard.LastQBBElecRadSrc = 0.0;
            elecBaseboard.LastSysTimeElapsed = 0.0;
            elecBaseboard.LastTimeStepSys = 0.0;

            elecBaseboard.MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            elecBaseboard.MyEnvrnFlag = true;
        }

        if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) {
            elecBaseboard.ZeroBBSourceSumHATsurf = state.dataHeatBal->Zone(ControlledZoneNum).sumHATsurf(state);
            elecBaseboard.QBBElecRadSrcAvg = 0.0;
            elecBaseboard.LastQBBElecRadSrc = 0.0;
            elecBaseboard.LastSysTimeElapsed = 0.0;
            elecBaseboard.LastTimeStepSys = 0.0;
        }

        // Do the every time step initializations
        int ZoneNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
        elecBaseboard.AirInletTemp = state.dataLoopNodes->Node(ZoneNode).Temp;
        elecBaseboard.AirInletHumRat = state.dataLoopNodes->Node(ZoneNode).HumRat;

        // Set the reporting variables to zero at each timestep.
        elecBaseboard.TotPower = 0.0;
        elecBaseboard.Power = 0.0;
        elecBaseboard.ConvPower = 0.0;
        elecBaseboard.RadPower = 0.0;
        elecBaseboard.TotEnergy = 0.0;
        elecBaseboard.Energy = 0.0;
        elecBaseboard.ConvEnergy = 0.0;
        elecBaseboard.RadEnergy = 0.0;
        elecBaseboard.ElecUseLoad = 0.0;
        elecBaseboard.ElecUseRate = 0.0;
    }

    void SizeElectricBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing electric baseboard components for which nominal capacities have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data. UAs are
        // calculated by numerically inverting the baseboard calculation routine.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeElectricBaseboard");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TempSize; // autosized value of coil input field

        if (state.dataSize->CurZoneEqNum > 0) {
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
            auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);
            state.dataSize->DataScalableCapSizingON = false;

            std::string_view const CompType = state.dataElectBaseboardRad->cCMO_BBRadiator_Electric;
            std::string_view const CompName = elecBaseboard.EquipName;
            state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
            state.dataSize->DataZoneNumber = elecBaseboard.ZonePtr;
            int SizingMethod = HVAC::HeatingCapacitySizing; // Integer representation of sizing method name (e.g., CoolingAirflowSizing)
            int FieldNum = 1;                               // IDD numeric field number where input field description is found
            std::string const SizingString =
                format("{} [W]", state.dataElectBaseboardRad->ElecBaseboardNumericFields(BaseboardNum).FieldNames(FieldNum));
            // capacity sizing methods (e.g., HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity)
            int CapSizingMethod = elecBaseboard.HeatingCapMethod;
            zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                bool PrintFlag = true; // TRUE when sizing information is reported in the eio file
                if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                    if (elecBaseboard.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        CheckZoneSizing(state, CompType, CompName);
                        zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    } else {
                        zoneEqSizing.DesHeatingLoad = elecBaseboard.ScaledHeatingCapacity;
                    }
                    zoneEqSizing.HeatingCapacity = true;
                    TempSize = elecBaseboard.ScaledHeatingCapacity;
                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                    if (state.dataSize->ZoneSizingRunDone) {
                        zoneEqSizing.HeatingCapacity = true;
                        zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    }
                    TempSize = elecBaseboard.ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                    state.dataSize->DataScalableCapSizingON = true;
                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                    CheckZoneSizing(state, CompType, CompName);
                    zoneEqSizing.HeatingCapacity = true;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = elecBaseboard.ScaledHeatingCapacity;
                    zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    Real64 FracOfAutoSzCap = DataSizing::AutoSize;
                    bool ErrorsFound = false;
                    HeatingCapacitySizer sizerHeatingCapacity;
                    sizerHeatingCapacity.overrideSizingString(SizingString);
                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    FracOfAutoSzCap = sizerHeatingCapacity.size(state, FracOfAutoSzCap, ErrorsFound);
                    TempSize = FracOfAutoSzCap;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
                    state.dataSize->DataScalableCapSizingON = true;
                } else {
                    TempSize = elecBaseboard.ScaledHeatingCapacity;
                }
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                elecBaseboard.NominalCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataScalableCapSizingON = false;
            }
        }
    }

    void CalcElectricBaseboard(EnergyPlusData &state, int const BaseboardNum, [[maybe_unused]] int const ControlledZoneNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       Feb 2010 Daeho Kang for radiant component
        //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the heat exchange rate in a Electric baseboard heater.
        // It includes radiant heat transfer to people and surfaces in a space, and the actual convective
        // system impact of a electric baseboard heater is determined after the radiant heat distribution.

        // METHODOLOGY EMPLOYED:
        // This is primarily modified from Convective Electric Baseboard. An existing algorithm of radiant
        // heat transfer calculation in the High Temperature Radiant System module is implemented.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr SimpConvAirFlowSpeed(0.5); // m/s

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QBBCap;
        Real64 RadHeat;
        Real64 LoadMet;
        auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);

        int ZoneNum = elecBaseboard.ZonePtr;
        Real64 QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        Real64 AirInletTemp = elecBaseboard.AirInletTemp;
        Real64 AirOutletTemp = AirInletTemp;
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(elecBaseboard.AirInletHumRat);
        Real64 AirMassFlowRate = SimpConvAirFlowSpeed;
        Real64 CapacitanceAir = CpAir * AirMassFlowRate;

        // Currently only the efficiency is used to calculate the electric consumption.  There could be some
        // thermal loss that could be accounted for with this efficiency input.
        Real64 Effic = elecBaseboard.BaseboardEfficiency;

        if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) &&
            elecBaseboard.availSched->getCurrentVal() > 0.0) {

            // If the load exceeds the capacity than the capacity is set to the BB limit.
            if (QZnReq > elecBaseboard.NominalCapacity) {
                QBBCap = elecBaseboard.NominalCapacity;
            } else {
                QBBCap = QZnReq;
            }
            RadHeat = QBBCap * elecBaseboard.FracRadiant;
            elecBaseboard.QBBElecRadSource = RadHeat;

            if (elecBaseboard.FracRadiant > 0.0) { // User defines radiant heat addition
                // Now, distribute the radiant energy of all systems to the appropriate surfaces, to people, and the air
                DistributeBBElecRadGains(state);
                // Now "simulate" the system by recalculating the heat balances
                HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);
                // Here an assumption is made regarding radiant heat transfer to people.
                // While the radiant heat transfer to people array will be used by the thermal comfort
                // routines, the energy transfer to people would get lost from the perspective
                // of the heat balance.  So, to avoid this net loss of energy which clearly
                // gets added to the zones, we must account for it somehow.  This assumption
                // that all energy radiated to people is converted to convective energy is
                // not very precise, but at least it conserves energy. The system impact to heat balance
                // should include this.
                LoadMet = (state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - elecBaseboard.ZeroBBSourceSumHATsurf) +
                          (QBBCap * elecBaseboard.FracConvect) + (RadHeat * elecBaseboard.FracDistribPerson);

                if (LoadMet < 0.0) {
                    // This basically means that SumHATsurf is LESS than ZeroBBSourceSumHATsurf which
                    // should not happen unless something unusual is happening like a fast change
                    // in temperature or some sort of change in internal load.  This is not a problem
                    // normally, but when LoadMet goes negative the choice is to either zero out
                    // the baseboard or give it another shot at getting an accurate reading on
                    // what is happening in the zone.  If it is still predicting a negative heating
                    // load, then zero everything out.
                    // First, turn off the baseboard:
                    elecBaseboard.QBBElecRadSource = 0.0;
                    DistributeBBElecRadGains(state);
                    HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                    HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);
                    Real64 TempZeroBBSourceSumHATsurf = state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state);
                    // Now, turn it back on:
                    elecBaseboard.QBBElecRadSource = RadHeat;
                    DistributeBBElecRadGains(state);
                    HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                    HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);
                    // Recalculate LoadMet with new ZeroBBSource... term and see if it is positive now.  If not, shut it down.
                    LoadMet = (state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - TempZeroBBSourceSumHATsurf) +
                              (QBBCap * elecBaseboard.FracConvect) + (RadHeat * elecBaseboard.FracDistribPerson);
                    if (LoadMet < 0.0) {
                        // LoadMet is still less than zero so shut everything down
                        UpdateElectricBaseboardOff(
                            LoadMet, QBBCap, RadHeat, elecBaseboard.QBBElecRadSource, elecBaseboard.ElecUseRate, AirOutletTemp, AirInletTemp);
                    } else {
                        // Corrected LoadMet is now positive so use this and move forward with system operating
                        UpdateElectricBaseboardOn(AirOutletTemp, elecBaseboard.ElecUseRate, AirInletTemp, QBBCap, CapacitanceAir, Effic);
                    }
                } else {

                    UpdateElectricBaseboardOn(AirOutletTemp, elecBaseboard.ElecUseRate, AirInletTemp, QBBCap, CapacitanceAir, Effic);
                }

            } else { // zero radiant fraction, no need of recalculation of heat balances

                LoadMet = QBBCap;
                UpdateElectricBaseboardOn(AirOutletTemp, elecBaseboard.ElecUseRate, AirInletTemp, QBBCap, CapacitanceAir, Effic);
            }

        } else { // If there is an off condition the BB does nothing.

            UpdateElectricBaseboardOff(
                LoadMet, QBBCap, RadHeat, elecBaseboard.QBBElecRadSource, elecBaseboard.ElecUseRate, AirOutletTemp, AirInletTemp);
        }

        // Assign calculated ones
        elecBaseboard.AirOutletTemp = AirOutletTemp;
        elecBaseboard.Power = QBBCap;
        elecBaseboard.TotPower = LoadMet;
        elecBaseboard.RadPower = RadHeat;
        elecBaseboard.ConvPower = QBBCap - RadHeat;
    }

    void UpdateElectricBaseboardOff(Real64 &LoadMet,
                                    Real64 &QBBCap,
                                    Real64 &RadHeat,
                                    Real64 &QBBElecRadSrc,
                                    Real64 &ElecUseRate,
                                    Real64 &AirOutletTemp,
                                    Real64 const AirInletTemp)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   August 2017

        // PURPOSE OF THIS SUBROUTINE: Zero out appropriate system variables when it is off

        QBBCap = 0.0;
        LoadMet = 0.0;
        RadHeat = 0.0;
        AirOutletTemp = AirInletTemp;
        QBBElecRadSrc = 0.0;
        ElecUseRate = 0.0;
    }

    void UpdateElectricBaseboardOn(
        Real64 &AirOutletTemp, Real64 &ElecUseRate, Real64 const AirInletTemp, Real64 const QBBCap, Real64 const CapacitanceAir, Real64 const Effic)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   August 2017

        // PURPOSE OF THIS SUBROUTINE: System is on, so calculate some of the result variables

        AirOutletTemp = AirInletTemp + QBBCap / CapacitanceAir;
        // This could be utilized somehow or even reported so the data structures are left in place
        // The Baseboard electric Load is calculated using the efficiency
        ElecUseRate = QBBCap / Effic;
    }

    void UpdateElectricBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //                      Rick Strand
        //       DATE WRITTEN   Nov 1997
        //                      February 2001
        //       MODIFIED       Feb 2010 Daeho Kang for radiant component

        // Using/Aliasing
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);

        // First, update the running average if necessary...
        if (elecBaseboard.LastSysTimeElapsed == SysTimeElapsed) {
            elecBaseboard.QBBElecRadSrcAvg -= elecBaseboard.LastQBBElecRadSrc * elecBaseboard.LastTimeStepSys / state.dataGlobal->TimeStepZone;
        }
        // Update the running average and the "last" values with the current values of the appropriate variables
        elecBaseboard.QBBElecRadSrcAvg += elecBaseboard.QBBElecRadSource * TimeStepSys / state.dataGlobal->TimeStepZone;

        elecBaseboard.LastQBBElecRadSrc = elecBaseboard.QBBElecRadSource;
        elecBaseboard.LastSysTimeElapsed = SysTimeElapsed;
        elecBaseboard.LastTimeStepSys = TimeStepSys;
    }

    void UpdateBBElecRadSourceValAvg(EnergyPlusData &state, bool &ElecBaseboardSysOn) // .TRUE. if the radiant system has run this zone time step
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001
        //       MODIFIED       Feb 2010 Daeho Kang for baseboard

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

        ElecBaseboardSysOn = false;

        // If this was never allocated, then there are no radiant systems in this input file (just RETURN)
        if (state.dataElectBaseboardRad->NumElecBaseboards == 0) {
            return;
        }

        // If it was allocated, then we have to check to see if this was running at all...
        for (auto &elecBaseboard : state.dataElectBaseboardRad->ElecBaseboard) {
            elecBaseboard.QBBElecRadSource = elecBaseboard.QBBElecRadSrcAvg;
            if (elecBaseboard.QBBElecRadSrcAvg != 0.0) {
                ElecBaseboardSysOn = true;
            }
        }

        // QBBElecRadSource has been modified so we need to redistribute gains
        DistributeBBElecRadGains(state);
    }

    void DistributeBBElecRadGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001
        //       MODIFIED       Feb 2010 Daeho Kang for baseboard
        //                      April 2010 Brent Griffith, max limit to protect surface temperature calcs

        // PURPOSE OF THIS SUBROUTINE:
        // To distribute the gains from the electric baseboard heater
        // as specified in the user input file.  This includes distribution
        // of long wavelength radiant gains to surfaces and "people."

        // METHODOLOGY EMPLOYED:
        // We must cycle through all of the radiant systems because each
        // surface could feel the effect of more than one radiant system.
        // Note that the energy radiated to people is assumed to affect them
        // but them it is assumed to be convected to the air.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr SmallestArea(0.001); // Smallest area in meters squared (to avoid a divide by zero)

        // Initialize arrays
        for (auto &elecBaseboard : state.dataElectBaseboardRad->ElecBaseboard) {
            for (int radSurfNum = 1; radSurfNum <= elecBaseboard.TotSurfToDistrib; ++radSurfNum) {
                int surfNum = elecBaseboard.SurfacePtr(radSurfNum);
                state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum).ElecBaseboard = 0.0;
            }
        }
        state.dataHeatBalFanSys->ZoneQElecBaseboardToPerson = 0.0;

        for (auto &elecBaseboard : state.dataElectBaseboardRad->ElecBaseboard) {
            if (elecBaseboard.ZonePtr > 0) { // issue 5806 can be zero during first calls to baseboards, will be set after all are modeled
                int ZoneNum = elecBaseboard.ZonePtr;
                state.dataHeatBalFanSys->ZoneQElecBaseboardToPerson(ZoneNum) += elecBaseboard.QBBElecRadSource * elecBaseboard.FracDistribPerson;

                for (int RadSurfNum = 1; RadSurfNum <= elecBaseboard.TotSurfToDistrib; ++RadSurfNum) {
                    int SurfNum = elecBaseboard.SurfacePtr(RadSurfNum);
                    if (state.dataSurface->Surface(SurfNum).Area > SmallestArea) {
                        Real64 ThisSurfIntensity =
                            (elecBaseboard.QBBElecRadSource * elecBaseboard.FracDistribToSurf(RadSurfNum) / state.dataSurface->Surface(SurfNum).Area);
                        state.dataHeatBalFanSys->surfQRadFromHVAC(SurfNum).ElecBaseboard += ThisSurfIntensity;
                        if (ThisSurfIntensity > DataHeatBalFanSys::MaxRadHeatFlux) {
                            ShowSevereError(state, "DistributeBBElecRadGains:  excessive thermal radiation heat flux intensity detected");
                            ShowContinueError(state, "Surface = " + state.dataSurface->Surface(SurfNum).Name);
                            ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                            ShowContinueError(state,
                                              "Occurs in " + state.dataElectBaseboardRad->cCMO_BBRadiator_Electric + " = " + elecBaseboard.EquipName);
                            ShowContinueError(state, format("Radiation intensity = {:.2R} [W/m2]", ThisSurfIntensity));
                            ShowContinueError(
                                state, "Assign a larger surface area or more surfaces in " + state.dataElectBaseboardRad->cCMO_BBRadiator_Electric);
                            ShowFatalError(state, "DistributeBBElecRadGains:  excessive thermal radiation heat flux intensity detected");
                        }
                    } else {
                        ShowSevereError(state, "DistributeBBElecRadGains:  surface not large enough to receive thermal radiation heat flux");
                        ShowContinueError(state, "Surface = " + state.dataSurface->Surface(SurfNum).Name);
                        ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                        ShowContinueError(state,
                                          "Occurs in " + state.dataElectBaseboardRad->cCMO_BBRadiator_Electric + " = " + elecBaseboard.EquipName);
                        ShowContinueError(
                            state, "Assign a larger surface area or more surfaces in " + state.dataElectBaseboardRad->cCMO_BBRadiator_Electric);
                        ShowFatalError(state, "DistributeBBElecRadGains:  surface not large enough to receive thermal radiation heat flux");
                    }
                }
            }
        }
    }

    void ReportElectricBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Feb 2010

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &elecBaseboard = state.dataElectBaseboardRad->ElecBaseboard(BaseboardNum);
        elecBaseboard.ElecUseLoad = elecBaseboard.ElecUseRate * TimeStepSysSec;
        elecBaseboard.TotEnergy = elecBaseboard.TotPower * TimeStepSysSec;
        elecBaseboard.Energy = elecBaseboard.Power * TimeStepSysSec;
        elecBaseboard.ConvEnergy = elecBaseboard.ConvPower * TimeStepSysSec;
        elecBaseboard.RadEnergy = elecBaseboard.RadPower * TimeStepSysSec;
    }

} // namespace ElectricBaseboardRadiator

} // namespace EnergyPlus
